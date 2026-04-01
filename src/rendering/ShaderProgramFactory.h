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

class ShaderProgramFactory
{
public:
	ShaderProgramFactory(rendering::OpenGLContext* opengl_context) noexcept;

	std::unique_ptr<QOpenGLShaderProgram> createShaderProgram(const AbstractRasterizer* rasterizer, const AbstractColorizer* colorizer, const PostprocessorShaderSpecifications& postprocessor_specifications);

private:
	rendering::OpenGLContext* m_opengl_context;

	std::unique_ptr<QOpenGLShader> m_vertex_shader;
	QString m_vertex_shader_path;

	std::unique_ptr<QOpenGLShader> m_fragment_shader;
	QString m_fragment_shader_path;
	QString m_colorization_shader_code;
	QString m_shader_defines_string;

	void configure(const AbstractRasterizer* rasterizer, const AbstractColorizer* colorizer, const PostprocessorShaderSpecifications& postprocessor_specifications);
	std::unique_ptr<QOpenGLShaderProgram> createShaderProgram();
	bool compileVertexShader();
	bool compileFragmentShader();
};

}