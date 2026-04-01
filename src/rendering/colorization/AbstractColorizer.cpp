#include "AbstractColorizer.h"

namespace sahara::rendering
{

AbstractColorizer::AbstractColorizer(OpenGLContext* opengl_context) noexcept
	: m_opengl_context(opengl_context)
	, m_shader_program(nullptr)
{
}

const ColorizerShaderSpecifications& AbstractColorizer::shaderSpecifications() const noexcept
{
	return m_shader_specifications;
}

std::vector<AbstractParameter*>& AbstractColorizer::parameters() noexcept
{
	return m_parameters;
}

}