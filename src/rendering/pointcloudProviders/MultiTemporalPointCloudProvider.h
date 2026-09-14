#pragma once

#include "AbstractPointCloudProvider.h"
#include "geometry/StaticPointCloud.h"
#include "rendering/parameters/RangeParameter.h"

#include <filesystem>

namespace sahara::rendering
{

// the multi-temporal point cloud provider stores multiple static pointcloud that are loaded into memory once in the beginning
class MultiTemporalPointCloudProvider : public AbstractPointCloudProvider
{
public:
	MultiTemporalPointCloudProvider(const std::filesystem::path& filepath, rendering::OpenGLContext* opengl_context);
	~MultiTemporalPointCloudProvider();

	size_t numberOfPoints() const override;
	int pointBudget() const override;
	const geometry::BoundingBox& boundingBox() const override;
	PointCloudProviderType type() const noexcept override;
	std::vector<RasterizerType> supportedRasterizers() const noexcept override;
	void setProvideMultipleTimestamps(bool provide_multiple_timestamps);
	void decreaseTimestamp();
	void increaseTimestamp();

	void update() override;

	void bindGPUBuffer(geometry::AttributeSemantic semantic) override;
	GLuint getGPUBuffer(geometry::AttributeSemantic semantic) override;
	void releaseGPUBuffer(geometry::AttributeSemantic semantic) override;

private:
	std::vector<std::unique_ptr<geometry::StaticPointCloud>> m_pointclouds;
	std::vector<uint> m_buffer_offsets;
	int m_number_of_points;
	geometry::BoundingBox m_bounding_box;
	std::unordered_map<geometry::AttributeSemantic, QOpenGLBuffer> m_gpu_buffers;
	std::vector<std::unique_ptr<geometry::AttributeMetadata>> m_combined_attribute_metadata;

	bool m_provide_multiple_timestamps;
	std::unique_ptr<RangeParameter<int>> m_epoch_parameter;

	void loadEpochsConcurrently(const std::filesystem::path& filepath);
	void createCombinedAttributeMetadata();
	void createGPUBuffers();
	void updateCommandBuffer();
};

}
