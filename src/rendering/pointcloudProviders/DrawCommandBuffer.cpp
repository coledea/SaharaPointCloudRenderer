#include "DrawCommandBuffer.h"

#include <iostream>

namespace sahara::rendering
{
DrawCommandBuffer::DrawCommandBuffer(OpenGLContext* opengl_context, size_t max_capacity)
	: m_opengl_context(opengl_context)
	, m_buffer(opengl_context, GL_WRITE_ONLY)
	, m_number_of_commands(0)
{
	m_opengl_context->makeCurrent();
	m_buffer.resize(max_capacity * sizeof(DrawArraysIndirectCommand));
	resetCommands(max_capacity);
	m_opengl_context->doneCurrent();
}

DrawCommandBuffer::~DrawCommandBuffer()
{
}

size_t DrawCommandBuffer::numberOfCommands() const noexcept
{
	return m_number_of_commands;
}

void DrawCommandBuffer::setCapacity(size_t max_capacity)
{
	m_opengl_context->makeCurrent();
	m_buffer.resize(max_capacity * sizeof(DrawArraysIndirectCommand));
	m_opengl_context->doneCurrent();
}

void DrawCommandBuffer::resetCommands(size_t new_size)
{
	if (new_size == 0)
	{
		return;
	}
	m_commands.resize(new_size, DrawArraysIndirectCommand{ 0, 0, 0, 0 });
}

void DrawCommandBuffer::updateDrawCommand(size_t index, uint number_of_points, uint buffer_offset, uint node_index, bool active)
{
	assert(index < m_commands.size());
	m_commands[index].number_of_points = number_of_points;
	m_commands[index].number_of_instances = active ? 1 : 0;
	m_commands[index].first_point = buffer_offset;
	m_commands[index].node_index = node_index;
}

void DrawCommandBuffer::updateOnGPU(int number_of_commands)
{
	m_number_of_commands = number_of_commands;
	if (number_of_commands == 0)
	{
		return;
	}
	m_buffer.write(0, sizeof(DrawArraysIndirectCommand) * number_of_commands, static_cast<const void*>(m_commands.data()));
}

void DrawCommandBuffer::updateOnGPU()
{
	updateOnGPU(m_commands.size());
}

void DrawCommandBuffer::bind()
{
	m_opengl_context->gl()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_buffer.handle());
}

void DrawCommandBuffer::release()
{
	m_opengl_context->gl()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

}
