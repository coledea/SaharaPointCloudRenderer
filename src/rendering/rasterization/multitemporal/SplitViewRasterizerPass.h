#pragma once

#include "rendering/ShaderProgramFactory.h"

#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class SplitViewRasterizerPass
{
public:
	SplitViewRasterizerPass(OpenGLContext* opengl_context, navigation::Camera* camera) noexcept;
	~SplitViewRasterizerPass();

	void run();
	void setBaseShaderProgram(QOpenGLShaderProgram* shader_program);
	void recompileShaders();

private:
	OpenGLContext* m_opengl_context;
	navigation::Camera* m_camera;
	QOpenGLShaderProgram* m_base_shader_program;

	QOpenGLVertexArrayObject m_line_vao;
	QOpenGLShaderProgram m_line_shader_program;

	float m_split_line_position;

	void updateSplitLinePosition();
};

}