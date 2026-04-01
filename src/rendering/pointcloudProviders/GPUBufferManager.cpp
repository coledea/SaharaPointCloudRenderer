#include "GPUBufferManager.h"

#include <QDebug>

namespace sahara::rendering
{
GPUBufferManager::GPUBufferManager(OpenGLContext* opengl_context)
	: m_opengl_context(opengl_context)
	, m_max_buffer_updates(10000)
	, m_buffer_updates(0)
	, m_buffer_hits(0)
	, m_successfull_allocations(0)
	, m_unsuccessfull_allocations(0)
{
}

void GPUBufferManager::resize(size_t new_capacity)
{
	m_available_segments.clear();
	m_allocated_segments.clear();
	m_available_segments_sorted.clear();

	auto result = m_available_segments.emplace(BufferSegmentMetadata{ 0, static_cast<uint>(new_capacity) });
	m_available_segments_sorted.insert(result.first);

	m_opengl_context->makeCurrent();
	for (const auto attribute : m_required_attributes)
	{
		m_buffers.at(attribute->semantic).resize(new_capacity * attribute->singleEntrySizeInBytes());
	}
	m_opengl_context->doneCurrent();
}

void GPUBufferManager::setBufferUpdateLimitPerCycle(int max_updates)
{
	m_max_buffer_updates = max_updates;
}

void GPUBufferManager::startNewCycle()
{
	m_buffer_hits = 0;
	m_successfull_allocations = 0;
	m_unsuccessfull_allocations = 0;
	m_buffer_updates = 0;
	m_free_segments_future.wait();
}

void GPUBufferManager::setRequiredAttributes(std::vector<const geometry::AttributeMetadata*>& required_attributes)
{
	m_required_attributes = required_attributes;
	m_buffers.clear();
	for (const auto attribute : m_required_attributes)
	{
		m_buffers.emplace(std::piecewise_construct, std::forward_as_tuple(attribute->semantic), std::forward_as_tuple(m_opengl_context, static_cast<GLenum>(GL_WRITE_ONLY)));
	}
}

int GPUBufferManager::bufferHits() const noexcept
{
	return m_buffer_hits;
}

int GPUBufferManager::successfullAllocations() const noexcept
{
	return m_successfull_allocations;
}

int GPUBufferManager::unsuccessfullAllocations() const noexcept
{
	return m_unsuccessfull_allocations;
}

GLuint GPUBufferManager::positionsBuffer() const noexcept
{
	return m_buffers.at(geometry::AttributeSemantic::Position).handle();
}

size_t GPUBufferManager::availableBufferStorage() const
{
	return std::accumulate(m_available_segments.begin(), m_available_segments.end(), 0, [](size_t sum, const BufferSegmentMetadata& segment) {
		return sum + segment.size;
	});
}

void GPUBufferManager::bindBuffer(geometry::AttributeSemantic semantic)
{
	m_opengl_context->gl()->glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(semantic).handle());
}

std::optional<uint32_t> GPUBufferManager::transferToGPU(geometry::LodOctreeNode* const node)
{
	if (nodeHasBufferSegment(node))
	{
		incrementBufferHits();
		return getSegment(node);
	}

	if (m_buffer_updates == m_max_buffer_updates)
	{
		return std::nullopt;
	}

	auto allocation_result = allocateSegment(node);
	if (allocation_result == std::nullopt)
	{
		incrementUnsuccessfullAllocations();
		return std::nullopt;
	}

	incrementSuccessfullAllocations();
	uint32_t buffer_offset = allocation_result.value();
	writeNodeDataToBuffers(node, buffer_offset);
	m_buffer_updates++;
	return buffer_offset;
}

void GPUBufferManager::freeSegments(const std::vector<bool>& nodes_to_keep)
{
	m_free_segments_future = std::async(std::launch::async, [this, nodes_to_keep]() {
		for (auto it = m_allocated_segments.begin(); it != m_allocated_segments.end();)
		{
			if (nodes_to_keep[it->first] == false)
			{
				it = freeSegment(it->first);
			}
			else
			{
				it++;
			}
		}
	});
}

bool GPUBufferManager::nodeHasBufferSegment(geometry::LodOctreeNode* const node)
{
	return m_allocated_segments.find(node->m_index) != m_allocated_segments.end();
}

uint32_t GPUBufferManager::getSegment(geometry::LodOctreeNode* const node)
{
	assert(nodeHasBufferSegment(node));
	return m_allocated_segments[node->m_index].offset;
}

std::optional<uint32_t> GPUBufferManager::allocateSegment(geometry::LodOctreeNode* const node)
{
	// find suitable segment and set it as allocated
	uint32_t requested_size = node->m_number_of_points; // for voxels, we also assume vec3 for position. While this wastes a lot of space, it is faster and way easier.
	auto size_sorted_iterator = m_available_segments_sorted.lower_bound(BufferSegmentMetadata{ 0, requested_size });
	if (size_sorted_iterator == m_available_segments_sorted.end())
	{
		return std::nullopt; // no segment with enough capacity available
	}
	auto available_segment_it = *size_sorted_iterator;
	m_allocated_segments.emplace(node->m_index, BufferSegmentMetadata{ available_segment_it->offset, requested_size });

	// adjust the available segments by removing the segment and inserting a new one with adjusted offset and size if the requested size differed from the actual size of the available segment
	auto original_segment = *available_segment_it;
	m_available_segments_sorted.erase(size_sorted_iterator);
	auto offset_sorted_iterator = m_available_segments.erase(available_segment_it);

	if (original_segment.size != requested_size)
	{
		original_segment.offset += requested_size;
		original_segment.size -= requested_size;
		auto inserted_iterator = m_available_segments.insert(offset_sorted_iterator, original_segment);
		m_available_segments_sorted.insert(inserted_iterator);
	}

	assert(m_available_segments.size() == m_available_segments_sorted.size());
	return m_allocated_segments[node->m_index].offset;
}

void GPUBufferManager::writeNodeDataToBuffers(geometry::LodOctreeNode* const node, uint32_t buffer_offset)
{
	for (const auto attribute : m_required_attributes)
	{
		const auto semantic = attribute->semantic;
		auto attribute_data = node->m_attribute_data->find(semantic);
		if (attribute_data != node->m_attribute_data->end())
		{
			m_buffers.at(semantic).write(buffer_offset * attribute->singleEntrySizeInBytes(), attribute_data->second->sizeInBytes(), attribute_data->second->rawData());
		}
	}
}

std::unordered_map<uint32_t, BufferSegmentMetadata>::iterator GPUBufferManager::freeSegment(uint32_t node_index)
{
	assert(m_allocated_segments.find(node_index) != m_allocated_segments.end());
	assert(m_available_segments.size() == m_available_segments_sorted.size());

	auto segment = m_allocated_segments[node_index];
	auto return_iterator = m_allocated_segments.erase(m_allocated_segments.find(node_index));

	auto insertion_pos = m_available_segments.lower_bound(segment);

	// If freed segment is directly adjacent to other free segments: merge them
	if (insertion_pos != m_available_segments.begin())
	{
		auto previous = std::prev(insertion_pos);
		if (previous->end() == segment.offset)
		{
			segment.offset = previous->offset;
			segment.size += previous->size;
			m_available_segments_sorted.erase(previous);
			insertion_pos = m_available_segments.erase(previous);
			assert(m_available_segments.size() == m_available_segments_sorted.size());
		}
	}

	if (insertion_pos != m_available_segments.end())
	{
		if (segment.end() == insertion_pos->offset)
		{
			segment.size += insertion_pos->size;
			m_available_segments_sorted.erase(insertion_pos);
			insertion_pos = m_available_segments.erase(insertion_pos);
			assert(m_available_segments.size() == m_available_segments_sorted.size());
		}
	}

	insertion_pos = m_available_segments.insert(insertion_pos, segment);
	m_available_segments_sorted.insert(insertion_pos);

	assert(m_available_segments.size() == m_available_segments_sorted.size());
	return return_iterator;
}

void GPUBufferManager::incrementBufferHits() noexcept
{
#ifdef PROFILER_ENABLED
	m_buffer_hits++;
#endif
}

void GPUBufferManager::incrementSuccessfullAllocations() noexcept
{
#ifdef PROFILER_ENABLED
	m_successfull_allocations++;
#endif
}

void GPUBufferManager::incrementUnsuccessfullAllocations() noexcept
{
#ifdef PROFILER_ENABLED
	m_unsuccessfull_allocations++;
#endif
}

}