#include "ShaderProgramFactory.h"

#include "utils/ShaderUtilities.h"

namespace sahara::rendering
{

ShaderProgramFactory::ShaderProgramFactory(rendering::OpenGLContext* opengl_context) noexcept
	: m_opengl_context(opengl_context)
{
	m_opengl_context->makeCurrent();
	m_opengl_context->doneCurrent();
}

std::unique_ptr<QOpenGLShaderProgram> ShaderProgramFactory::createShaderProgram(const ShaderPaths& shader_paths, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification> attribute_inputs, const std::set<geometry::AttributeSpecification> attribute_outputs)
{
	m_shader_paths = shader_paths;
	m_colorization_shader_code = colorization_shader_code;

	m_shader_defines_string.clear();
	for (const auto attribute : attribute_inputs)
	{
		m_shader_defines_string += utils::ShaderStringsFactory::attributeInputDefineString(attribute.semantic, attribute.type);
	}
	for (const auto attribute : attribute_outputs)
	{
		m_shader_defines_string += utils::ShaderStringsFactory::attributeOutputDefineString(attribute.semantic, attribute.type);
	}

	return createShaderProgram();
}

std::unique_ptr<QOpenGLShaderProgram> ShaderProgramFactory::createShaderProgram()
{
	m_opengl_context->makeCurrent();

	auto shader_program = std::make_unique<QOpenGLShaderProgram>();

	for (const auto& [shaderType, shaderPath] : m_shader_paths)
	{
		addShader(shader_program.get(), shaderType, shaderPath);
	}

	if (!shader_program->link())
	{
		qDebug() << "Program linking error: " << shader_program->log();
	}

	m_opengl_context->doneCurrent();

	return shader_program;
}

void insertIfAvailable(QString& shader_code, const QString& insert_string, const QString& to_insert)
{
	auto index = shader_code.indexOf(insert_string);
	if (index >= 0)
	{
		shader_code.insert(index, to_insert);
		shader_code.remove(insert_string);
	}
}

void ShaderProgramFactory::addShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderTypeBit shaderType, QString shaderPath)
{
	QString shaderCode = utils::ShaderStringsFactory::readShaderFile(shaderPath);

	const QString definesString = "#SHADER_DEFINES";
	insertIfAvailable(shaderCode, definesString, m_shader_defines_string);

	const QString colorizationString = "#COLORIZATION";
	insertIfAvailable(shaderCode, colorizationString, m_colorization_shader_code);

	if (!program->addShaderFromSourceCode(shaderType, shaderCode))
	{
		qDebug() << "Shader compilation error:" << program->log();
	}
}

}