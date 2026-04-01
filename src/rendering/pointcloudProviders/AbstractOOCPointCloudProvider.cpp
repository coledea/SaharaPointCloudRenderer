#include "AbstractOOCPointCloudProvider.h"

#include "geometry/MultiTemporalLodOctree.h"
#include "utils/Profiler.h"

#include <QFile>
#include <QtConcurrent>

namespace sahara::rendering
{

AbstractOOCPointCloudProvider::AbstractOOCPointCloudProvider(const std::string& name, rendering::OpenGLContext* opengl_context, navigation::Camera* camera)
	: AbstractPointCloudProvider(name.c_str(), opengl_context, 1)
	, m_camera(camera)
	, m_number_of_points(0)
	, m_gpu_buffer_manager(opengl_context)
	, m_render_budget(0)
	, m_gpu_cache_budget(0)
	, m_cpu_cache_budget(0)
	, m_position_offsets_buffer(0)
	, m_cpu_buffer_hits(0)
	, m_cpu_buffer_misses(0)
	, m_budget_too_low(false)
	, m_render_fence(nullptr)
{
	m_is_valid = true;
}

AbstractOOCPointCloudProvider::~AbstractOOCPointCloudProvider()
{
	if (m_position_offsets_buffer != 0)
	{
		m_opengl_context->gl()->glDeleteBuffers(1, &m_position_offsets_buffer);
	}
}

size_t AbstractOOCPointCloudProvider::numberOfPoints() const
{
	return m_number_of_points;
}

int AbstractOOCPointCloudProvider::pointBudget() const
{
	return m_point_budget_parameter->value();
}

const geometry::BoundingBox& AbstractOOCPointCloudProvider::boundingBox() const
{
	return m_octree->boundingBox();
}

std::vector<RasterizerType> AbstractOOCPointCloudProvider::supportedRasterizers() const noexcept
{
	return { RasterizerType::OOCPointPrimitiveRasterizer };
}

void AbstractOOCPointCloudProvider::initializeParameters()
{
	m_point_budget_parameter = std::make_unique<RangeParameter<int>>("Point Budget", 35000000, 100000, 100000000, 10000);
	connect(m_point_budget_parameter.get(), &RangeParameter<int>::valueChanged, [this]() {
		updateBufferSizes();
		emit pointBudgetChanged(m_render_budget);
	});
	m_parameters.push_back(m_point_budget_parameter.get());

	m_cpu_cache_factor_parameter = std::make_unique<RangeParameter<int>>("CPU Cache Size Factor", 4, 1, 10, 1);
	connect(m_cpu_cache_factor_parameter.get(), &RangeParameter<int>::valueChanged, this, &AbstractOOCPointCloudProvider::onCPUCacheSizeChanged);
	m_parameters.push_back(m_cpu_cache_factor_parameter.get());

	m_gpu_cache_factor_parameter = std::make_unique<RangeParameter<int>>("GPU Cache Size Factor", 2, 1, 10, 1);
	connect(m_gpu_cache_factor_parameter.get(), &RangeParameter<int>::valueChanged, this, &AbstractOOCPointCloudProvider::onGPUCacheSizeChanged);
	m_parameters.push_back(m_gpu_cache_factor_parameter.get());

	m_buffer_update_limit_parameter = std::make_unique<RangeParameter<int>>("Max Buffer Updates per Frame", 10000, 1, 10000, 1);
	connect(m_buffer_update_limit_parameter.get(), &RangeParameter<int>::valueChanged, this, &AbstractOOCPointCloudProvider::onBufferUpdateLimitChanged);
	m_parameters.push_back(m_buffer_update_limit_parameter.get());
	m_gpu_buffer_manager.setBufferUpdateLimitPerCycle(m_buffer_update_limit_parameter->value());

	m_projected_size_threshold_parameter = std::make_unique<RangeParameter<int>>("Projected Size Pixel Threshold", 220, 0, 2000, 1);
	connect(m_projected_size_threshold_parameter.get(), &RangeParameter<int>::valueChanged, this, &AbstractOOCPointCloudProvider::onProjectedSizeThresholdChanged);
	m_parameters.push_back(m_projected_size_threshold_parameter.get());
	onProjectedSizeThresholdChanged();
}

void AbstractOOCPointCloudProvider::setRequiredAttributes(const std::set<geometry::AttributeSpecification>& attributes)
{
	AbstractPointCloudProvider::setRequiredAttributes(attributes);
	m_chunk_loader->setRequiredAttributes(m_required_attributes);
	m_gpu_buffer_manager.setRequiredAttributes(m_required_attributes);
	updateBufferSizes();
}

void AbstractOOCPointCloudProvider::update()
{
	m_octree->computePrioritiesAndSort(*m_camera);

	utils::global_profiler.startTimer("Update Buffers");
	updateBuffers();
	utils::global_profiler.stopTimer("Update Buffers");
}

GLuint AbstractOOCPointCloudProvider::nodeMetadataBuffer() const noexcept
{
	return m_octree->nodeMetadataBuffer();
}

GLuint AbstractOOCPointCloudProvider::positionsBuffer() const noexcept
{
	return m_gpu_buffer_manager.positionsBuffer();
}

GLuint AbstractOOCPointCloudProvider::positionOffsetsBuffer() const noexcept
{
	return m_position_offsets_buffer;
}

void AbstractOOCPointCloudProvider::bindGPUBuffer(geometry::AttributeSemantic semantic)
{
	m_gpu_buffer_manager.bindBuffer(semantic);
}

void AbstractOOCPointCloudProvider::releaseGPUBuffer(geometry::AttributeSemantic semantic)
{
	m_opengl_context->gl()->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void AbstractOOCPointCloudProvider::setRenderFence(GLsync fence)
{
	m_render_fence = fence;
}

void AbstractOOCPointCloudProvider::updateBuffers()
{
	m_budget_too_low = false;
	m_cpu_buffer_hits = 0;
	m_cpu_buffer_misses = 0;
	m_nodes_should_remain_on_cpu.assign(m_nodes_should_remain_on_cpu.size(), false);
	m_nodes_should_remain_on_gpu.assign(m_nodes_should_remain_on_gpu.size(), false);

	m_chunk_loader->resetLoadingQueueAndPauseLoading(); // we pause the chunk loader for the whole update.

	utils::global_profiler.startTimer("Update Buffers CPU");
	uint nodes_to_render = updateLoaderAndFreeMemory();
	utils::global_profiler.stopTimer("Update Buffers CPU");

	utils::global_profiler.startTimer("Update Buffers GPU");
	m_draw_command_buffer.resetCommands(nodes_to_render);
	resizePositionsOffsetsBuffer(nodes_to_render);

	if (m_render_fence != nullptr)
	{
		m_opengl_context->gl()->glClientWaitSync(m_render_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000); // 1s timeout
		m_opengl_context->gl()->glDeleteSync(m_render_fence);
		m_render_fence = nullptr;
	}

	nodes_to_render = updateGPUBuffers(nodes_to_render);

	m_chunk_loader->resumeLoading();

	m_draw_command_buffer.updateOnGPU(nodes_to_render);
	if (!m_positions_offsets.empty())
	{
		m_opengl_context->gl()->glNamedBufferSubData(m_position_offsets_buffer, 0, sizeof(uint) * m_positions_offsets.size(), static_cast<const void*>(m_positions_offsets.data()));
	}

	utils::global_profiler.stopTimer("Update Buffers GPU");
	utils::global_profiler.addMeasurement("GPU_buffer_hits", m_gpu_buffer_manager.bufferHits());
	utils::global_profiler.addMeasurement("GPU_buffer_allocation_successfull", m_gpu_buffer_manager.successfullAllocations());
	utils::global_profiler.addMeasurement("GPU_buffer_allocation_unsuccessfull", m_gpu_buffer_manager.unsuccessfullAllocations());
}

uint AbstractOOCPointCloudProvider::updateLoaderAndFreeMemory()
{
	const auto& chunks = m_octree->nodes();
	uint nodes_to_render = 0;
	uint number_of_considered_points = 0;
	int chunk_index = 0;

	// traverse chunks within the render budget and queue for loading if necessary
	for (; chunk_index < chunks.size(); chunk_index++)
	{
		auto node = chunks[chunk_index];

		if (number_of_considered_points + node->m_number_of_points > m_render_budget)
		{
			setBudgetTooLow(node->m_priority >= 100.0f);
			break;
		}

		if (node->m_priority < 100.0f)
		{
			break;
		}

		if (node->m_attribute_data != nullptr)
		{
			m_nodes_should_remain_on_gpu[node->m_index] = true;
			m_nodes_should_remain_on_cpu[node->m_index] = true;
			nodes_to_render++;
			incrementCPUBufferHits();
		}
		else
		{
			m_chunk_loader->addToLoadingQueue(node);
			incrementCPUBufferMisses();
		}
		number_of_considered_points += node->m_number_of_points;
	}

	// travese further chunks within the GPU buffer budget and queue for loading if necessary
	for (; chunk_index < chunks.size(); chunk_index++)
	{
		auto node = chunks[chunk_index];
		if (number_of_considered_points + node->m_number_of_points >= m_gpu_cache_budget)
		{
			break;
		}

		// we prefetch only nodes with priority > 0, but cache all nodes within the budget
		if (node->m_attribute_data == nullptr && node->m_priority > 0.0f)
		{
			m_chunk_loader->addToLoadingQueue(node);
		}

		m_nodes_should_remain_on_cpu[node->m_index] = true;
		m_nodes_should_remain_on_gpu[node->m_index] = true;
		number_of_considered_points += node->m_number_of_points;
	}

	m_gpu_buffer_manager.freeSegments(m_nodes_should_remain_on_gpu);

	// traverse further chunks within the main memory budget and queue for loading if necessary
	for (; chunk_index < chunks.size(); chunk_index++)
	{
		auto node = chunks[chunk_index];
		if (number_of_considered_points + node->m_number_of_points > m_cpu_cache_budget)
		{
			break;
		}

		// we prefetch only nodes with priority > 0, but cache all nodes within the budget
		if (node->m_attribute_data == nullptr && node->m_priority > 0.0f)
		{
			m_chunk_loader->addToLoadingQueue(node);
		}
		number_of_considered_points += node->m_number_of_points;
		m_nodes_should_remain_on_cpu[node->m_index] = true;
	}

	m_chunk_loader->pinNodes(m_nodes_should_remain_on_cpu);

	utils::global_profiler.addMeasurement("Budget_too_low", m_budget_too_low ? 1 : 0);
	utils::global_profiler.addMeasurement("CPU_buffer_hits", m_cpu_buffer_hits);
	utils::global_profiler.addMeasurement("CPU_buffer_misses", m_cpu_buffer_misses);

	return nodes_to_render;
}

uint AbstractOOCPointCloudProvider::updateGPUBuffers(uint nodes_to_render)
{
	m_gpu_buffer_manager.startNewCycle();
	int processed_nodes = 0;
	const auto& chunks = m_octree->nodes();
	for (int i = 0; i < chunks.size(); i++)
	{
		auto node = chunks[i];
		if (processed_nodes >= nodes_to_render || node->m_priority < 100.0f)
		{
			break;
		}

		if (node->m_attribute_data == nullptr) // skip nodes for which the attribute data is not yet loaded
		{
			continue;
		}

		auto buffer_offset = m_gpu_buffer_manager.transferToGPU(node);
		if (buffer_offset == std::nullopt)
		{
			nodes_to_render--;
			continue;
		}

		m_draw_command_buffer.updateDrawCommand(processed_nodes, node->m_number_of_points, buffer_offset.value(), node->m_index, node->m_priority >= 100.0f); // nodes with priority >= 100.0f should be rendered
		m_positions_offsets[processed_nodes] = buffer_offset.value();
		processed_nodes++;
	}
	return processed_nodes;
}

void AbstractOOCPointCloudProvider::resizePositionsOffsetsBuffer(uint nodes_to_render)
{
	if (nodes_to_render == 0)
	{
		return;
	}
	m_positions_offsets.resize(nodes_to_render, 0);

	if (nodes_to_render == 0)
	{
		return;
	}

	if (m_position_offsets_buffer != 0)
	{
		m_opengl_context->gl()->glDeleteBuffers(1, &m_position_offsets_buffer);
	}
	m_opengl_context->gl()->glCreateBuffers(1, &m_position_offsets_buffer);
	m_opengl_context->gl()->glNamedBufferStorage(m_position_offsets_buffer, nodes_to_render * sizeof(uint), nullptr, GL_DYNAMIC_STORAGE_BIT);
}

void AbstractOOCPointCloudProvider::incrementCPUBufferHits() noexcept
{
#ifdef PROFILER_ENABLED
	m_cpu_buffer_hits++;
#endif
}

void AbstractOOCPointCloudProvider::incrementCPUBufferMisses() noexcept
{
#ifdef PROFILER_ENABLED
	m_cpu_buffer_misses++;
#endif
}

void AbstractOOCPointCloudProvider::setBudgetTooLow(bool budget_too_low) noexcept
{
#ifdef PROFILER_ENABLED
	m_budget_too_low = budget_too_low;
#endif
}

void AbstractOOCPointCloudProvider::updateBufferSizes()
{
	m_render_budget = m_point_budget_parameter->value();
	onCPUCacheSizeChanged();
	onGPUCacheSizeChanged();
	m_gpu_buffer_manager.resize(m_gpu_cache_budget);
}

void AbstractOOCPointCloudProvider::onCPUCacheSizeChanged()
{
	m_cpu_cache_budget = m_cpu_cache_factor_parameter->value() * m_point_budget_parameter->value();
}

void AbstractOOCPointCloudProvider::onGPUCacheSizeChanged()
{
	m_gpu_cache_budget = m_gpu_cache_factor_parameter->value() * m_point_budget_parameter->value();

	// we increase the gpu cache size slightly. Otherwise, very important, large chunks can sometimes not be loaded.
	// The reason for this is that all chunks fitting into the gpu budget are pinned (regardless of their priority). However, buffer fragmentation may decrease the effective budget.
	m_gpu_cache_budget = m_gpu_cache_budget + m_gpu_cache_budget / 5;
}

void AbstractOOCPointCloudProvider::onBufferUpdateLimitChanged()
{
	m_gpu_buffer_manager.setBufferUpdateLimitPerCycle(m_buffer_update_limit_parameter->value());
}

void AbstractOOCPointCloudProvider::onProjectedSizeThresholdChanged()
{
	m_octree->setProjectionSizeRenderThreshold(m_projected_size_threshold_parameter->value());
}

}