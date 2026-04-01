#pragma once

#include "AbstractPointCloudProvider.h"
#include "GPUBufferManager.h"
#include "MappedOpenGLBuffer.h"
#include "geometry/AbstractLodOctree.h"
#include "io/OOCPointCloudLoader.h"
#include "rendering/parameters/EnumParameter.h"
#include "rendering/parameters/RangeParameter.h"

#include <bitset>
#include <filesystem>

namespace sahara::rendering
{

// this point cloud provider stores an LoD data structure and maintains a buffer of fixed size for the currently relevant relevant point cloud chunks.
// the chunks are loaded from disk based on the virtual viewpoint
class AbstractOOCPointCloudProvider : public AbstractPointCloudProvider
{
public:
	AbstractOOCPointCloudProvider(const std::string& name, rendering::OpenGLContext* opengl_context, navigation::Camera* camera);
	virtual ~AbstractOOCPointCloudProvider();

	size_t numberOfPoints() const override;
	int pointBudget() const override;
	const geometry::BoundingBox& boundingBox() const override;
	virtual PointCloudProviderType type() const noexcept = 0;
	std::vector<RasterizerType> supportedRasterizers() const noexcept override;

	void setRequiredAttributes(const std::set<geometry::AttributeSpecification>& attributes) override;
	void update() override;

	GLuint nodeMetadataBuffer() const noexcept;
	GLuint positionsBuffer() const noexcept;
	GLuint positionOffsetsBuffer() const noexcept;

	void bindGPUBuffer(geometry::AttributeSemantic semantic) override;
	void releaseGPUBuffer(geometry::AttributeSemantic semantic) override;

	void setRenderFence(GLsync fence); // this is used to synchronize the rendering with the GPU buffer updates -> wait for rendering to finish before starting buffer updates

protected:
	navigation::Camera* m_camera;
	size_t m_number_of_points;
	GLsync m_render_fence;

	std::unique_ptr<geometry::AbstractLodOctree> m_octree;
	std::unique_ptr<io::OOCPointCloudLoader> m_chunk_loader;
	GPUBufferManager m_gpu_buffer_manager;

	GLuint m_position_offsets_buffer;

	uint m_render_budget;
	uint m_gpu_cache_budget;
	uint m_cpu_cache_budget;

	std::vector<bool> m_nodes_should_remain_on_gpu;
	std::vector<bool> m_nodes_should_remain_on_cpu;

	std::unordered_map<geometry::AttributeSemantic, std::unique_ptr<geometry::AttributeMetadata>> m_attribute_metadata;

	std::unique_ptr<RangeParameter<int>> m_point_budget_parameter;
	std::unique_ptr<RangeParameter<int>> m_cpu_cache_factor_parameter;
	std::unique_ptr<RangeParameter<int>> m_gpu_cache_factor_parameter;
	std::unique_ptr<RangeParameter<int>> m_buffer_update_limit_parameter;

	std::unique_ptr<RangeParameter<int>> m_projected_size_threshold_parameter;

	std::vector<uint> m_positions_offsets;

	bool m_budget_too_low;
	int m_cpu_buffer_hits;
	int m_cpu_buffer_misses;

	void initializeParameters();
	void onCPUCacheSizeChanged();
	void onGPUCacheSizeChanged();
	void onBufferUpdateLimitChanged();
	void onProjectedSizeThresholdChanged();

	void updateBufferSizes();
	void updateBuffers();
	uint updateLoaderAndFreeMemory();
	uint updateGPUBuffers(uint nodes_to_render);

	void resizePositionsOffsetsBuffer(uint nodes_to_render);

	void incrementCPUBufferHits() noexcept;
	void incrementCPUBufferMisses() noexcept;
	void setBudgetTooLow(bool budget_too_low) noexcept;
};
}