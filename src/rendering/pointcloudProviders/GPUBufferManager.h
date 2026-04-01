#pragma once

#include "BufferSegmentMetadata.h"
#include "MappedOpenGLBuffer.h"
#include "geometry/AttributeMetadata.h"
#include "geometry/LodOctree.h"

#include <future>
#include <list>
#include <queue>
#include <unordered_set>

namespace sahara::rendering
{

class GPUBufferManager
{
public:
	GPUBufferManager(OpenGLContext* opengl_context);

	GLuint positionsBuffer() const noexcept;
	int bufferHits() const noexcept;
	int successfullAllocations() const noexcept;
	int unsuccessfullAllocations() const noexcept;
	size_t availableBufferStorage() const;

	void setRequiredAttributes(std::vector<const geometry::AttributeMetadata*>& required_attributes);
	void resize(size_t new_capacity);
	void setBufferUpdateLimitPerCycle(int max_updates);

	void freeSegments(const std::vector<bool>& nodes_to_keep);
	std::optional<uint32_t> transferToGPU(geometry::LodOctreeNode* const node); // transfers attribute data to GPU if necessary and returns the buffer offset
	void bindBuffer(geometry::AttributeSemantic semantic);

	void startNewCycle(); // indicate that the next call to transferToGPU() is part of a new frame (-> resets the buffer update metrics)

private:
	OpenGLContext* m_opengl_context;
	std::unordered_map<geometry::AttributeSemantic, MappedOpenGLBuffer> m_buffers;
	std::vector<const geometry::AttributeMetadata*> m_required_attributes;

	// We use sets, as they provide logarithmic search and amortized constant addition/removal for a known iterator
	std::set<BufferSegmentMetadata, BufferOffsetComparator> m_available_segments;					 // offset-ordered
	std::multiset<BufferSegmentsIterator, BufferIteratorSizeComparator> m_available_segments_sorted; // size-ordered
	std::unordered_map<uint32_t, BufferSegmentMetadata> m_allocated_segments;

	std::future<void> m_free_segments_future;

	int m_max_buffer_updates;
	int m_buffer_updates;
	int m_buffer_hits;
	int m_successfull_allocations;
	int m_unsuccessfull_allocations;

	bool nodeHasBufferSegment(geometry::LodOctreeNode* const node);
	uint32_t getSegment(geometry::LodOctreeNode* const node);
	std::optional<uint32_t> allocateSegment(geometry::LodOctreeNode* const node);
	void writeNodeDataToBuffers(geometry::LodOctreeNode* const node, uint32_t buffer_offset);
	std::unordered_map<uint32_t, BufferSegmentMetadata>::iterator freeSegment(uint32_t node_index);

	void incrementBufferHits() noexcept;
	void incrementSuccessfullAllocations() noexcept;
	void incrementUnsuccessfullAllocations() noexcept;
};

}