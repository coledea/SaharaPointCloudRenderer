#pragma once

#include "rendering/OpenGLContext.h"

namespace sahara::rendering
{

class MappedOpenGLBuffer
{
public:
	MappedOpenGLBuffer(OpenGLContext* opengl_context, GLenum access); // access can be GL_WRITE_ONLY, GL_READ_ONLY, or GL_READ_WRITE
	~MappedOpenGLBuffer();

	GLuint handle() const noexcept;
	void* memoryPtr() const noexcept;

	void resize(size_t size_in_bytes);
	void bind(GLenum target);
	void release(GLenum target);

	void write(uint32_t byte_offset, uint32_t size_in_bytes, const void* data);
	void waitForGPUWrite();

private:
	OpenGLContext* m_opengl_context;
	GLenum m_access;
	GLuint m_buffer;
	uint8_t* m_mapped_buffer;
};

}