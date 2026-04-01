#include "MappedOpenGLBuffer.h"

#include <algorithm>

namespace sahara::rendering
{
MappedOpenGLBuffer::MappedOpenGLBuffer(OpenGLContext* opengl_context, GLenum access)
	: m_opengl_context(opengl_context)
	, m_access(access)
	, m_buffer(0)
	, m_mapped_buffer(nullptr)
{
}

MappedOpenGLBuffer::~MappedOpenGLBuffer()
{
	if (m_buffer != 0)
	{
		m_opengl_context->gl()->glDeleteBuffers(1, &m_buffer);
	}
}

GLuint MappedOpenGLBuffer::handle() const noexcept
{
	return m_buffer;
}

void* MappedOpenGLBuffer::memoryPtr() const noexcept
{
	return m_mapped_buffer;
}

void MappedOpenGLBuffer::resize(size_t size_in_bytes)
{
	if (size_in_bytes == 0)
	{
		return;
	}

	if (m_buffer != 0)
	{
		m_opengl_context->gl()->glDeleteBuffers(1, &m_buffer);
	}
	m_opengl_context->gl()->glCreateBuffers(1, &m_buffer);

	GLenum storage_access_flag = 0;
	if (m_access == GL_READ_ONLY || m_access == GL_READ_WRITE)
	{
		storage_access_flag |= GL_MAP_READ_BIT;
	}
	if (m_access == GL_WRITE_ONLY || m_access == GL_READ_WRITE)
	{
		storage_access_flag |= GL_MAP_WRITE_BIT;
	}
	storage_access_flag |= GL_MAP_PERSISTENT_BIT;
	m_opengl_context->gl()->glNamedBufferStorage(m_buffer, size_in_bytes, nullptr, storage_access_flag);

	if (m_access == GL_WRITE_ONLY || m_access == GL_READ_WRITE)
	{
		storage_access_flag |= GL_MAP_FLUSH_EXPLICIT_BIT; // flushing explicitly gives us more interpretable performance measurements. We did not observe any performance difference between explicit and driver-controlled flushing
	}
	m_mapped_buffer = static_cast<uint8_t*>(m_opengl_context->gl()->glMapNamedBufferRange(m_buffer, 0, size_in_bytes, storage_access_flag));
}

void MappedOpenGLBuffer::bind(GLenum target)
{
	m_opengl_context->gl()->glBindBuffer(target, m_buffer);
}

void MappedOpenGLBuffer::release(GLenum target)
{
	m_opengl_context->gl()->glBindBuffer(target, 0);
}

void MappedOpenGLBuffer::write(uint32_t byte_offset, uint32_t size_in_bytes, const void* data)
{
	std::copy_n(static_cast<const uint8_t*>(data), size_in_bytes, m_mapped_buffer + byte_offset);
	m_opengl_context->gl()->glFlushMappedNamedBufferRange(m_buffer, byte_offset, size_in_bytes);
}

void MappedOpenGLBuffer::waitForGPUWrite()
{
	m_opengl_context->gl()->glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
	GLsync fence = m_opengl_context->gl()->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	m_opengl_context->gl()->glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, GLuint(1e12));
	m_opengl_context->gl()->glDeleteSync(fence);
}
}
