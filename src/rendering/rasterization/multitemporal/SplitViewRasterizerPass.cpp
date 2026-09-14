#include "SplitViewRasterizerPass.h"

#include <ui/RenderWindow.h>

namespace sahara::rendering
{

SplitViewRasterizerPass::SplitViewRasterizerPass(rendering::OpenGLContext* opengl_context, navigation::Camera* camera) noexcept
	: m_opengl_context(opengl_context)
	, m_camera(camera)
	, m_split_line_position(0.5f * static_cast<float>(camera->viewportWidth()))
{
	recompileShaders();

	m_opengl_context->makeCurrent();
	m_line_vao.create();
	m_opengl_context->doneCurrent();
}

SplitViewRasterizerPass::~SplitViewRasterizerPass()
{
	m_opengl_context->makeCurrent();
	if (m_line_vao.isCreated())
	{
		m_line_vao.destroy();
	}
	m_opengl_context->doneCurrent();
}

void SplitViewRasterizerPass::setBaseShaderProgram(QOpenGLShaderProgram* shader_program)
{
	m_base_shader_program = shader_program;
}

void SplitViewRasterizerPass::recompileShaders()
{
	m_opengl_context->makeCurrent();
	utils::ShaderStringsFactory::compileVertexFragmentShader(&m_line_shader_program, "./data/shaders/postprocessing/Line.vert", "./data/shaders/postprocessing/Line.frag");
	m_opengl_context->doneCurrent();
}

void SplitViewRasterizerPass::updateSplitLinePosition()
{
	auto& events = ui::RenderWindow::s_event_cache;

	if (events.didResizeOccur())
	{
		m_split_line_position = 0.5f * static_cast<float>(m_camera->viewportWidth());
	}

	if (events.isKeyPressed(Qt::Key_Control) && events.isMouseButtonPressed(Qt::MouseButton::LeftButton))
	{
		m_split_line_position = events.getMousePosition().x();
	}
}

void SplitViewRasterizerPass::run()
{
	updateSplitLinePosition();

	const float width = m_camera->viewportWidth();
	const float height = m_camera->viewportHeight();

	m_base_shader_program->bind();
	m_base_shader_program->setUniformValue("u_mvp", m_camera->viewProjectionMatrix());

	// RENDER PASS 1
	if (m_split_line_position > 0.0f)
	{
		glScissor(0, 0, m_split_line_position, height);
		glEnable(GL_SCISSOR_TEST);
		m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (0 * sizeof(DrawArraysIndirectCommand)));
	}

	// RENDER PASS 2
	if (m_split_line_position < width)
	{
		glScissor(m_split_line_position, 0, width - m_split_line_position, height);
		m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (1 * sizeof(DrawArraysIndirectCommand)));
		glDisable(GL_SCISSOR_TEST);
	}

	m_base_shader_program->release();

	// RENDER PASS 3
	m_line_shader_program.bind();
	m_line_vao.bind();

	const float split_line_position_normalized = m_split_line_position / width * 2.0f - 1.0f;

	m_line_shader_program.setUniformValue("u_xposition", split_line_position_normalized);
	m_line_shader_program.setUniformValue("u_thickness", 4.0f / width);

	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	m_line_vao.release();
	m_line_shader_program.release();
}

}
