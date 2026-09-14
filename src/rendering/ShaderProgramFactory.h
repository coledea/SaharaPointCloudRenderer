#pragma once

#include "OpenGLContext.h"
#include "rendering/colorization/AbstractColorizer.h"
#include "rendering/postprocessing/AbstractPostprocessor.h"
#include "rendering/rasterization/AbstractRasterizer.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <set>

namespace sahara::rendering
{

using ShaderPaths = std::map<QOpenGLShader::ShaderTypeBit, QString>;

class ShaderProgramFactory
{
public:
	ShaderProgramFactory(rendering::OpenGLContext* opengl_context) noexcept;

	std::unique_ptr<QOpenGLShaderProgram> createShaderProgram(const ShaderPaths& shader_paths, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification> attribute_inputs, const std::set<geometry::AttributeSpecification> attribute_outputs);

private:
	rendering::OpenGLContext* m_opengl_context;

	ShaderPaths m_shader_paths;
	QString m_colorization_shader_code;
	QString m_shader_defines_string;

	std::unique_ptr<QOpenGLShaderProgram> createShaderProgram();
	void addShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderTypeBit shaderType, QString shaderPath);
};

}