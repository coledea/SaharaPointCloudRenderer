#include "DefaultColorizer.h"

#include "utils/ShaderUtilities.h"

namespace sahara::rendering
{

DefaultColorizer::DefaultColorizer() noexcept
	: AbstractColorizer(nullptr)
{
}

DefaultColorizer::~DefaultColorizer()
{
}

void DefaultColorizer::reloadShaderSpecificationsFromDisk()
{
	m_shader_specifications.colorization_shader_code = utils::ShaderStringsFactory::readShaderFile("./data/shaders/DefaultColorizer.glsl");
	m_shader_specifications.vertex_shader_outputs = { { geometry::AttributeType::Color, geometry::AttributeSemantic::Color } };
}

void DefaultColorizer::setCompiledShaderProgram(QOpenGLShaderProgram* shader_program)
{
	// No parameters to set for the default colorizer
}

ColorizerType DefaultColorizer::type() const noexcept
{
	return ColorizerType::None;
}

}