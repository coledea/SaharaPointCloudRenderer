#include "ChangeHighlightingRasterizerPass.h"

#include <ui/RenderWindow.h>

namespace sahara::rendering
{

ChangeHighlightingRasterizerPass::ChangeHighlightingRasterizerPass(rendering::OpenGLContext* opengl_context, navigation::Camera* camera, bool use_precomputed_changes) noexcept
	: m_opengl_context(opengl_context)
	, m_camera(camera)
	, m_standard_framebuffer(nullptr)
	, m_framebuffer1(opengl_context, camera->viewportWidth(), camera->viewportHeight())
	, m_framebuffer2(opengl_context, camera->viewportWidth(), camera->viewportHeight())
	, m_fullscreen_geometry(opengl_context)
	, m_lens_active(false)
	, m_lens_timestamp(0)
	, m_use_precomputed_changes(use_precomputed_changes)
{
	if (m_use_precomputed_changes)
	{
		m_framebuffer1.setRequiredAttachments({ FramebufferAttachmentTypes::CUSTOM_FLOAT });
		m_framebuffer2.setRequiredAttachments({ FramebufferAttachmentTypes::CUSTOM_FLOAT });
	}

	recompileShaders();

	m_opengl_context->makeCurrent();
	m_lens_vao.create();
	m_opengl_context->doneCurrent();
}

ChangeHighlightingRasterizerPass::~ChangeHighlightingRasterizerPass()
{
	m_opengl_context->makeCurrent();
	if (m_lens_vao.isCreated())
	{
		m_lens_vao.destroy();
	}

	m_framebuffer1.destroy();
	m_framebuffer2.destroy();
	m_opengl_context->doneCurrent();
}

void ChangeHighlightingRasterizerPass::setBaseShaderProgram(QOpenGLShaderProgram* shader_program)
{
	m_base_shader_program = shader_program;
}

void ChangeHighlightingRasterizerPass::recompileShaders()
{
	m_opengl_context->makeCurrent();

	utils::ShaderStringsFactory::compileVertexFragmentShader(&m_lens_shader_program, "./data/shaders/postprocessing/Lens.vert", "./data/shaders/postprocessing/Lens.frag");
	utils::ShaderStringsFactory::compileVertexFragmentShader(&m_compositing_shader_program, "./data/shaders/Differences.vert", m_use_precomputed_changes ? "./data/shaders/PrecomputedDifferences.frag" : "./data/shaders/Differences.frag");

	m_compositing_shader_program.bind();
	m_fullscreen_geometry.bind();
	m_compositing_shader_program.enableAttributeArray(0);
	m_compositing_shader_program.setAttributeBuffer(0, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
	m_fullscreen_geometry.release();
	m_compositing_shader_program.release();

	m_opengl_context->doneCurrent();

	utils::setUniformValue(m_opengl_context, m_compositing_shader_program, "u_lens_active", m_lens_active ? static_cast<GLfloat>(1.0) : static_cast<GLfloat>(0.0));
	utils::setUniformValue(m_opengl_context, m_compositing_shader_program, "u_lens_timestamp", static_cast<float>(m_lens_timestamp));
	utils::setUniformValue(m_opengl_context, m_lens_shader_program, "u_lens_timestamp", static_cast<float>(m_lens_timestamp));
}

void ChangeHighlightingRasterizerPass::setFramebuffer(Framebuffer* framebuffer)
{
	m_standard_framebuffer = framebuffer;
}

void ChangeHighlightingRasterizerPass::setAddedColor(const QColor& color)
{
	utils::setUniformValue(m_opengl_context, m_compositing_shader_program, "u_added_tint", color);
}

void ChangeHighlightingRasterizerPass::setRemovedColor(const QColor& color)
{
	utils::setUniformValue(m_opengl_context, m_compositing_shader_program, "u_removed_tint", color);
}

void ChangeHighlightingRasterizerPass::setLensRadius(float radius)
{
	utils::setUniformValue(m_opengl_context, m_compositing_shader_program, "u_lens_radius", radius);
	utils::setUniformValue(m_opengl_context, m_lens_shader_program, "u_lens_radius", radius);
}

void ChangeHighlightingRasterizerPass::setDifferenceThreshold(float epsilon)
{
	utils::setUniformValue(m_opengl_context, m_compositing_shader_program, "u_epsilon", epsilon);
}

void ChangeHighlightingRasterizerPass::run()
{
	handleUserInput();

	m_base_shader_program->bind();
	m_base_shader_program->setUniformValue("u_mvp", m_camera->viewProjectionMatrix());

	// RENDER PASS 1
	m_framebuffer1.bind();
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (0 * sizeof(DrawArraysIndirectCommand)));

	// RENDER PASS 2
	m_framebuffer2.bind();
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (1 * sizeof(DrawArraysIndirectCommand)));

	m_base_shader_program->release();

	// RENDER PASS 3
	m_standard_framebuffer->bind();
	m_compositing_shader_program.bind();

	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set textures and uniforms
	const float width = static_cast<float>(m_camera->viewportWidth());
	const float height = static_cast<float>(m_camera->viewportHeight());

	m_compositing_shader_program.setUniformValue("u_far_plane", m_camera->cameraSpecifications().far_plane);
	m_compositing_shader_program.setUniformValue("u_near_plane", m_camera->cameraSpecifications().near_plane);
	m_compositing_shader_program.setUniformValue("u_aspect_ratio", width / height);

	const QPointF current_mouse_position = ui::RenderWindow::s_event_cache.getMousePosition();
	QPointF mouse_position_normalized = QPointF(current_mouse_position.x() / width, current_mouse_position.y() / height) * 2.0f - QPointF(1.0, 1.0);
	mouse_position_normalized.setY(-mouse_position_normalized.y());
	m_compositing_shader_program.setUniformValue("u_lens_position", mouse_position_normalized);

	m_opengl_context->gl()->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_framebuffer1.textureID(FramebufferAttachmentTypes::DEPTH, m_framebuffer1.currentFBOIndex()));
	m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_compositing_shader_program.programId(), "u_depth_texture_1"), 0);

	m_opengl_context->gl()->glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_framebuffer1.textureID(FramebufferAttachmentTypes::COLOR, m_framebuffer1.currentFBOIndex()));
	m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_compositing_shader_program.programId(), "u_color_texture_1"), 1);

	m_opengl_context->gl()->glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_framebuffer2.textureID(FramebufferAttachmentTypes::DEPTH, m_framebuffer2.currentFBOIndex()));
	m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_compositing_shader_program.programId(), "u_depth_texture_2"), 2);

	m_opengl_context->gl()->glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_framebuffer2.textureID(FramebufferAttachmentTypes::COLOR, m_framebuffer2.currentFBOIndex()));
	m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_compositing_shader_program.programId(), "u_color_texture_2"), 3);

	if (m_use_precomputed_changes)
	{
		m_opengl_context->gl()->glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_framebuffer1.textureID(FramebufferAttachmentTypes::CUSTOM_FLOAT, m_framebuffer1.currentFBOIndex()));
		m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_compositing_shader_program.programId(), "u_change_texture_1"), 4);

		m_opengl_context->gl()->glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_framebuffer2.textureID(FramebufferAttachmentTypes::CUSTOM_FLOAT, m_framebuffer2.currentFBOIndex()));
		m_opengl_context->gl()->glUniform1i(m_opengl_context->gl()->glGetUniformLocation(m_compositing_shader_program.programId(), "u_change_texture_2"), 5);
	}

	glEnable(GL_DEPTH_TEST);
	m_fullscreen_geometry.draw();
	m_compositing_shader_program.release();

	if (m_lens_active)
	{
		renderLens();
	}
}

void ChangeHighlightingRasterizerPass::renderLens()
{
	m_lens_shader_program.bind();
	m_lens_vao.bind();

	const float width = static_cast<float>(m_camera->viewportWidth());
	const float height = static_cast<float>(m_camera->viewportHeight());

	const QPointF current_mouse_position = ui::RenderWindow::s_event_cache.getMousePosition();
	QPointF mouse_position_normalized = QPointF(current_mouse_position.x() / width, current_mouse_position.y() / height) * 2.0f - QPointF(1.0f, 1.0f);
	mouse_position_normalized.setY(-mouse_position_normalized.y());

	m_lens_shader_program.setUniformValue("u_lens_position", mouse_position_normalized);
	m_lens_shader_program.setUniformValue("u_aspect_ratio", width / height);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	m_lens_shader_program.release();
	m_lens_vao.release();
}

void ChangeHighlightingRasterizerPass::handleUserInput()
{
	auto& events = ui::RenderWindow::s_event_cache;

	bool last_lens_state = m_lens_active;
	m_lens_active = events.isKeyPressed(Qt::Key_Control);

	if (last_lens_state != m_lens_active)
	{
		m_compositing_shader_program.bind();
		m_compositing_shader_program.setUniformValue("u_lens_active", static_cast<GLfloat>(m_lens_active ? 1.0f : 0.0f));
		m_compositing_shader_program.release();
	}

	if (m_lens_active && events.didMouseReleaseOccur())
	{
		m_lens_timestamp = (m_lens_timestamp + 1) % 2;

		m_compositing_shader_program.bind();
		m_compositing_shader_program.setUniformValue("u_lens_timestamp", static_cast<float>(m_lens_timestamp));
		m_compositing_shader_program.release();

		m_lens_shader_program.bind();
		m_lens_shader_program.setUniformValue("u_lens_timestamp", static_cast<float>(m_lens_timestamp));
		m_lens_shader_program.release();
	}

	if (events.didResizeOccur())
	{
		const QPoint newSize = events.getFramebufferSize();
		m_framebuffer1.resize(newSize.x(), newSize.y());
		m_framebuffer2.resize(newSize.x(), newSize.y());
	}
}

}