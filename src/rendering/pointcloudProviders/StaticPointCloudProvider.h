#pragma once

#include "AbstractPointCloudProvider.h"
#include "geometry/StaticPointCloud.h"
#include "io/CSVColumnSettings.h"

#include <filesystem>

namespace sahara::rendering
{

class StaticPointCloudProvider : public AbstractPointCloudProvider
{
public:
	StaticPointCloudProvider(const std::filesystem::path& filepath, const io::CSVColumnSettings& columnSettings, rendering::OpenGLContext* opengl_context) noexcept;

	~StaticPointCloudProvider();

	size_t numberOfPoints() const override;
	int pointBudget() const override;
	const geometry::BoundingBox& boundingBox() const override;
	PointCloudProviderType type() const noexcept override;
	std::vector<RasterizerType> supportedRasterizers() const noexcept override;

	void update() override;

	void bindGPUBuffer(geometry::AttributeSemantic semantic) override;
	GLuint getGPUBuffer(geometry::AttributeSemantic semantic) override;
	void releaseGPUBuffer(geometry::AttributeSemantic semantic) override;

private:
	std::unique_ptr<geometry::StaticPointCloud> m_pointcloud; // the static point cloud provider only stores one static pointcloud that is loaded into memory once in the beginning
	std::unordered_map<geometry::AttributeSemantic, QOpenGLBuffer> m_gpu_buffers;
};

}