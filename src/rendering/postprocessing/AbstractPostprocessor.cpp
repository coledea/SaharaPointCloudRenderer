#include "AbstractPostprocessor.h"

namespace sahara::rendering
{

AbstractPostprocessor::AbstractPostprocessor(rendering::OpenGLContext* opengl_context) noexcept
	: m_opengl_context(opengl_context)
{
}

const PostprocessorShaderSpecifications& AbstractPostprocessor::shaderSpecifications() const noexcept
{
	return m_shader_specifications;
}

std::vector<AbstractParameter*>& AbstractPostprocessor::parameters() noexcept
{
	return m_parameters;
}

}