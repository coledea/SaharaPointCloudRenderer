#pragma once

#include <QOpenGLContext>
#include <QOpenGLFunctions_4_5_Core>
#include <QSurface>

namespace sahara::rendering
{

class OpenGLContext
{
public:
	explicit OpenGLContext(QSurface* surface);

	bool makeCurrent();
	void doneCurrent();
	void swapBuffers();

	QOpenGLFunctions_4_5_Core* gl();
	GLuint defaultFramebufferID();

private:
	QOpenGLContext m_context;
	QOpenGLFunctions_4_5_Core* m_functions;
	QSurface* m_surface;
};

}