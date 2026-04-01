#pragma once

#include "DrawArraysIndirectCommand.h"
#include "MappedOpenGLBuffer.h"
#include "rendering/OpenGLContext.h"

namespace sahara::rendering
{

class DrawCommandBuffer
{
public:
	DrawCommandBuffer(OpenGLContext* opengl_context, size_t max_capacity);
	~DrawCommandBuffer();

	size_t numberOfCommands() const noexcept;

	void setCapacity(size_t max_capacity);
	void resetCommands(size_t new_size);
	void updateDrawCommand(size_t index, uint number_of_points, uint buffer_offset, uint node_index, bool active);
	void updateOnGPU(int number_of_commands);
	void updateOnGPU();
	void bind();
	void release();

private:
	OpenGLContext* m_opengl_context;
	std::vector<DrawArraysIndirectCommand> m_commands;
	MappedOpenGLBuffer m_buffer;
	int m_number_of_commands;
};

}