#pragma once

#include "rendering/InputEventCache.h"
#include "rendering/ShaderProgramFactory.h"
#include "utils/FullscreenGeometry.h"

#include <QOpenGLVertexArrayObject>

namespace sahara::rendering
{

class ChangeHighlightingRasterizerPass
{
public:
	ChangeHighlightingRasterizerPass(OpenGLContext* opengl_context, navigation::Camera* camera, bool use_precomputed_changes) noexcept;
	~ChangeHighlightingRasterizerPass();

	void run();
	void setBaseShaderProgram(QOpenGLShaderProgram* shader_program);
	void recompileShaders();

	void setFramebuffer(Framebuffer* framebuffer);

	void setAddedColor(const QColor& color);
	void setRemovedColor(const QColor& color);
	void setLensRadius(float radius);
	void setDifferenceThreshold(float epsilon);

private:
	OpenGLContext* m_opengl_context;
	navigation::Camera* m_camera;
	Framebuffer* m_standard_framebuffer;
	QOpenGLShaderProgram* m_base_shader_program;

	utils::FullscreenGeometry m_fullscreen_geometry;
	QOpenGLShaderProgram m_compositing_shader_program;
	Framebuffer m_framebuffer1;
	Framebuffer m_framebuffer2;

	QOpenGLVertexArrayObject m_lens_vao;
	QOpenGLShaderProgram m_lens_shader_program;

	bool m_use_precomputed_changes;
	bool m_lens_active;
	int m_lens_timestamp;

	void renderLens();
	void handleUserInput();
};

}