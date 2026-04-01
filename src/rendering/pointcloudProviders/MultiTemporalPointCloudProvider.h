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

	void update() override;
	void bindGPUBuffer(geometry::AttributeSemantic semantic) override;
	void releaseGPUBuffer(geometry::AttributeSemantic semantic) override;

private:
	std::vector<std::unique_ptr<geometry::StaticPointCloud>> m_pointclouds;
	std::vector<uint> m_buffer_offsets;
	int m_number_of_points;
	geometry::BoundingBox m_bounding_box;
	std::unordered_map<geometry::AttributeSemantic, QOpenGLBuffer> m_gpu_buffers;

	std::unique_ptr<RangeParameter<int>> m_epoch_parameter;

	void loadEpochsConcurrently(const std::filesystem::path& filepath);
	void createGPUBuffers();
	void updateCommandBuffer();
};

}