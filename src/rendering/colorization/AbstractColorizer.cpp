#include "AbstractColorizer.h"

#include "AttributeBasedColorizer.h"
#include "SingleColorColorizer.h"

namespace sahara::rendering
{

AbstractColorizer::AbstractColorizer(OpenGLContext* opengl_context) noexcept
	: m_opengl_context(opengl_context)
	, m_shader_program(nullptr)
{
}

const QString& AbstractColorizer::finalShaderCode() const noexcept
{
	return m_final_shader_code;
}

std::vector<AbstractParameter*>& AbstractColorizer::parameters() noexcept
{
	return m_parameters;
}

}
