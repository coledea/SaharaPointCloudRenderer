#include "ShaderProgramFactory.h"

#include "utils/ShaderUtilities.h"

namespace sahara::rendering
{

ShaderProgramFactory::ShaderProgramFactory(rendering::OpenGLContext* opengl_context) noexcept
	: m_opengl_context(opengl_context)
	, m_vertex_shader(nullptr)
	, m_fragment_shader(nullptr)
{
	m_opengl_context->makeCurrent();
	m_vertex_shader = std::make_unique<QOpenGLShader>(QOpenGLShader::Vertex);
	m_fragment_shader = std::make_unique<QOpenGLShader>(QOpenGLShader::Fragment);
	m_opengl_context->doneCurrent();
}

std::unique_ptr<QOpenGLShaderProgram> ShaderProgramFactory::createShaderProgram(const AbstractRasterizer* rasterizer, const AbstractColorizer* colorizer, const PostprocessorShaderSpecifications& postprocessor_specifications)
{
	configure(rasterizer, colorizer, postprocessor_specifications);
	return createShaderProgram();
}

void ShaderProgramFactory::configure(const AbstractRasterizer* rasterizer, const AbstractColorizer* colorizer, const PostprocessorShaderSpecifications& postprocessor_specifications)
{
	m_shader_defines_string.clear();

	m_vertex_shader_path = rasterizer->shaderSpecifications().vertex_shader_path;
	m_fragment_shader_path = rasterizer->shaderSpecifications().fragment_shader_path;
	m_colorization_shader_code = colorizer->shaderSpecifications().colorization_shader_code;

	// find set of required vertex shader inputs and outputs
	std::set<geometry::AttributeSpecification> vertex_shader_outputs;
	for (const auto attribute : colorizer->shaderSpecifications().vertex_shader_outputs)
	{
		vertex_shader_outputs.insert(attribute);
	}
	for (const auto attribute : postprocessor_specifications.required_vertex_attributes)
	{
		vertex_shader_outputs.insert(attribute);
	}

	// add corresponding defines in the shader code
	for (const auto attribute : vertex_shader_outputs)
	{
		m_shader_defines_string += utils::ShaderStringsFactory::vertexShaderOutputDefineString(attribute.semantic, attribute.type);
	}
	for (const auto attribute : postprocessor_specifications.required_vertex_attributes)
	{
		m_shader_defines_string += utils::ShaderStringsFactory::vertexShaderOutputDefineString(attribute.semantic, attribute.type);
	}
}

std::unique_ptr<QOpenGLShaderProgram> ShaderProgramFactory::createShaderProgram()
{
	m_opengl_context->makeCurrent();

	auto shader_program = std::make_unique<QOpenGLShaderProgram>();

	if (!compileVertexShader())
	{
		qDebug() << "Vertex shader compilation error:" << m_vertex_shader->log();
	}
	shader_program->addShader(m_vertex_shader.get());

	if (!compileFragmentShader())
	{
		qDebug() << "Fragment shader compilation error:" << m_fragment_shader->log();
	}
	shader_program->addShader(m_fragment_shader.get());

	if (!shader_program->link())
	{
		qDebug() << "Program linking error!";
	}

	m_opengl_context->doneCurrent();

	return shader_program;
}

bool ShaderProgramFactory::compileVertexShader()
{
	QString vertex_shader_code = utils::ShaderStringsFactory::readShaderFile(m_vertex_shader_path);

	// Insert vertex shader outputs/output writes
	vertex_shader_code.insert(vertex_shader_code.indexOf("#SHADER_DEFINES"), m_shader_defines_string);
	vertex_shader_code.remove("#SHADER_DEFINES");
	return m_vertex_shader->compileSourceCode(vertex_shader_code);
}

bool ShaderProgramFactory::compileFragmentShader()
{
	QString fragment_shader_code = utils::ShaderStringsFactory::readShaderFile(m_fragment_shader_path);

	// Insert fragment shader inputs
	fragment_shader_code.insert(fragment_shader_code.indexOf("#SHADER_DEFINES"), m_shader_defines_string);
	fragment_shader_code.insert(fragment_shader_code.indexOf("#COLORIZATION"), m_colorization_shader_code);
	fragment_shader_code.remove("#SHADER_DEFINES");
	fragment_shader_code.remove("#COLORIZATION");
	return m_fragment_shader->compileSourceCode(fragment_shader_code);
}
}