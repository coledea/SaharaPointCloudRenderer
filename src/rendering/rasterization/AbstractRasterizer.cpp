#include "AbstractRasterizer.h"

namespace sahara::rendering
{

AbstractRasterizer::AbstractRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: m_opengl_context(opengl_context)
	, m_pointcloud_provider(provider)
	, m_camera(camera)
	, m_shader_program(nullptr)
{
}

const RasterizerShaderSpecifications& AbstractRasterizer::shaderSpecifications() const noexcept
{
	return m_shader_specifications;
}

std::vector<AbstractParameter*>& AbstractRasterizer::parameters() noexcept
{
	return m_parameters;
}

}