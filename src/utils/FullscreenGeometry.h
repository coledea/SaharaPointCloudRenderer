#pragma once

#include "rendering/OpenGLContext.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

namespace sahara::utils
{

class FullscreenGeometry
{
public:
	FullscreenGeometry(rendering::OpenGLContext* opengl_context);

	void bind() const;
	void release() const;

	void draw() const;

private:
	inline void createFullscreenGeometry(rendering::OpenGLContext* opengl_context) const;

	std::unique_ptr<QOpenGLBuffer> triangle_vbo;
	std::unique_ptr<QOpenGLVertexArrayObject> triangle_vao;

public:
	static void createFullscreenGeometry(rendering::OpenGLContext* opengl_context, QOpenGLBuffer& triangle_vbo, QOpenGLVertexArrayObject& triangle_vao);
};

}
