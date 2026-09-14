#include "StaticPointCloudProvider.h"

#include "geometry/StaticPointCloudFactory.h"
#include "io/CSVLoadingDialog.h"

namespace sahara::rendering
{

StaticPointCloudProvider::StaticPointCloudProvider(const std::filesystem::path& filepath, const io::CSVColumnSettings& columnSettings, rendering::OpenGLContext* opengl_context) noexcept
	: AbstractPointCloudProvider(filepath.filename().string().c_str(), opengl_context, 1)
{
	m_pointcloud = geometry::StaticPointCloudFactory::loadFromPointCloudFile(filepath, columnSettings);

	m_opengl_context->makeCurrent();
	for (const auto& attribute : m_pointcloud->attributes())
	{
		const auto semantic = attribute.first;
		m_gpu_buffers.emplace(semantic, QOpenGLBuffer::Type::VertexBuffer);
		m_gpu_buffers[semantic].create();
		m_gpu_buffers[semantic].setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);

		m_gpu_buffers[semantic].bind();
		m_gpu_buffers[semantic].allocate(m_pointcloud->attributeData(semantic)->rawData(), static_cast<int>(m_pointcloud->attributeData(semantic)->sizeInBytes()));
		m_gpu_buffers[semantic].release();

		m_available_attributes.emplace(semantic, attribute.second.metadata.get());
	}

	m_draw_command_buffer.updateDrawCommand(0, m_pointcloud->numberOfPoints(), 0, 0, true);
	m_draw_command_buffer.updateOnGPU();
	m_opengl_context->doneCurrent();

	m_is_valid = true;
}

StaticPointCloudProvider::~StaticPointCloudProvider()
{
}

size_t StaticPointCloudProvider::numberOfPoints() const
{
	return m_pointcloud->numberOfPoints();
}

int StaticPointCloudProvider::pointBudget() const
{
	return m_pointcloud->numberOfPoints();
}

const geometry::BoundingBox& StaticPointCloudProvider::boundingBox() const
{
	return m_pointcloud->boundingBox();
}

PointCloudProviderType StaticPointCloudProvider::type() const noexcept
{
	return PointCloudProviderType::StaticPointCloudProvider;
}

std::vector<RasterizerType> StaticPointCloudProvider::supportedRasterizers() const noexcept
{
	return { RasterizerType::PointPrimitiveRasterizer, RasterizerType::PointComputeRasterizer, RasterizerType::HQComputeRasterizer };
}

void StaticPointCloudProvider::update()
{
	// Nothing to do here. The point cloud was already loaded in full.
}

void StaticPointCloudProvider::bindGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	m_gpu_buffers[semantic].bind();
}

GLuint StaticPointCloudProvider::getGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	return m_gpu_buffers[semantic].bufferId();
}

void StaticPointCloudProvider::releaseGPUBuffer(geometry::AttributeSemantic semantic)
{
	assert(hasAttribute(semantic));
	m_gpu_buffers[semantic].release();
}

}