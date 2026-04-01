#include "OpenGLContext.h"

#include <QOpenGLVersionFunctionsFactory>

namespace sahara::rendering
{

// from https://www.khronos.org/opengl/wiki/Debug_Output
void GLMessageCallback(GLenum /*source*/,
					   GLenum type,
					   GLuint /*id*/,
					   GLenum severity,
					   GLsizei /*length*/,
					   const GLchar* message,
					   const void* /*userParam*/)
{
	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
	{
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
	}
}

OpenGLContext::OpenGLContext(QSurface* surface)
	: m_surface(surface)
{
	m_context.setFormat(QSurfaceFormat::defaultFormat());
	m_context.create();
	m_context.makeCurrent(m_surface);
	m_functions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_5_Core>(&m_context);

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	gl()->glDebugMessageCallback(GLMessageCallback, 0);
#endif

	m_context.doneCurrent();
}

bool OpenGLContext::makeCurrent()
{
	return m_context.makeCurrent(m_surface);
}

void OpenGLContext::doneCurrent()
{
	m_context.doneCurrent();
}

void OpenGLContext::swapBuffers()
{
	m_context.swapBuffers(m_surface);
}

QOpenGLFunctions_4_5_Core* OpenGLContext::gl()
{
	return m_functions;
}

GLuint OpenGLContext::defaultFramebufferID()
{
	return m_context.defaultFramebufferObject();
}

}