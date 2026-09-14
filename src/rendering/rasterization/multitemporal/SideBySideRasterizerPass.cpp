#include "SideBySideRasterizerPass.h"

namespace sahara::rendering
{

SideBySideRasterizerPass::SideBySideRasterizerPass(rendering::OpenGLContext* opengl_context, navigation::Camera* camera) noexcept
	: m_opengl_context(opengl_context)
	, m_camera(camera)
{
	recompileShaders();

	m_opengl_context->makeCurrent();
	m_line_vao.create();
	m_opengl_context->doneCurrent();
}

SideBySideRasterizerPass::~SideBySideRasterizerPass()
{
	m_opengl_context->makeCurrent();
	if (m_line_vao.isCreated())
	{
		m_line_vao.destroy();
	}
	m_opengl_context->doneCurrent();
}

void SideBySideRasterizerPass::setBaseShaderProgram(QOpenGLShaderProgram* shader_program)
{
	m_base_shader_program = shader_program;
}

void SideBySideRasterizerPass::recompileShaders()
{
	m_opengl_context->makeCurrent();

	utils::ShaderStringsFactory::compileVertexFragmentShader(&m_line_shader_program, "./data/shaders/postprocessing/Line.vert", "./data/shaders/postprocessing/Line.frag");
	m_line_shader_program.bind();
	m_line_shader_program.setUniformValue("u_xposition", 0.0f);
	m_line_shader_program.release();

	m_opengl_context->doneCurrent();
}

void SideBySideRasterizerPass::run()
{
	const float width = m_camera->viewportWidth();
	const float height = m_camera->viewportHeight();
	const float split_position = width * 0.5;

	m_camera->setViewport(split_position, height);
	const QMatrix4x4 tempMVPMatrix = m_camera->projectionMatrix() * m_camera->viewMatrix();

	m_base_shader_program->bind();
	m_base_shader_program->setUniformValue("u_mvp", tempMVPMatrix);

	//  RENDER PASS 1
	glViewport(0, 0, split_position, height);
	m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (0 * sizeof(DrawArraysIndirectCommand)));

	// RENDER PASS 2
	glViewport(split_position, 0, split_position, height);
	m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (1 * sizeof(DrawArraysIndirectCommand)));

	// reset Viewport
	glViewport(0, 0, width, height);
	m_camera->setViewport(width, height);

	m_base_shader_program->release();

	// RENDER PASS 3
	m_line_shader_program.bind();
	m_line_vao.bind();

	m_line_shader_program.setUniformValue("u_thickness", 4.0f / width);

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	m_line_shader_program.release();
}

}
