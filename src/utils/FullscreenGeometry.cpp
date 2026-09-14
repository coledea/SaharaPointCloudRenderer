#include "FullscreenGeometry.h"

namespace sahara::utils
{

FullscreenGeometry::FullscreenGeometry(rendering::OpenGLContext* opengl_context)
	: triangle_vbo(std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer))
	, triangle_vao(std::make_unique<QOpenGLVertexArrayObject>())
{
	createFullscreenGeometry(opengl_context);
}

void FullscreenGeometry::createFullscreenGeometry(rendering::OpenGLContext* opengl_context) const
{
	const GLfloat triangle_vertices[] = { -1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f };

	opengl_context->makeCurrent();

	triangle_vao->create();
	triangle_vbo->create();

	triangle_vao->bind();
	triangle_vbo->bind();

	triangle_vbo->allocate(triangle_vertices, sizeof(triangle_vertices));

	triangle_vao->release();
	triangle_vbo->release();

	opengl_context->doneCurrent();
}

void FullscreenGeometry::bind() const
{
	triangle_vao->bind();
	triangle_vbo->bind();
}

void FullscreenGeometry::release() const
{
	triangle_vao->release();
	triangle_vbo->release();
}

void FullscreenGeometry::draw() const
{
	bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	release();
}

}