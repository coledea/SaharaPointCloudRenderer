#include "NodePriorityComputation.h"

#include "LodOctree.h"
#include "NodePriorityComputationHelper.h"
#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <QtMath>

namespace sahara::geometry
{

NodePriorityComputation::NodePriorityComputation(rendering::OpenGLContext* context, const std::vector<NodeGPUData>& node_data, const std::vector<uint32_t>& level_starts)
	: AbstractNodePriorityComputation(context, node_data.size(), "./data/shaders/processing/ComputePriorities.comp")
{
	m_opengl_context->makeCurrent();

	for (int i = 0; i < level_starts.size() - 1; i++)
	{
		auto workgroup_size = static_cast<GLuint>(std::ceil(static_cast<float>(level_starts[i + 1] - level_starts[i]) / 1024.0f));
		m_level_bounds.emplace_back(level_starts[i], level_starts[i + 1], workgroup_size);
	}
	m_level_bounds.emplace_back(level_starts.back(), node_data.size(), static_cast<GLuint>(std::ceil(static_cast<float>(node_data.size() - level_starts.back()) / 1024.0f)));

	m_opengl_context->gl()->glGenBuffers(1, &m_bbox_buffer);
	m_opengl_context->gl()->glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bbox_buffer);
	m_opengl_context->gl()->glBufferData(GL_SHADER_STORAGE_BUFFER, node_data.size() * sizeof(NodeGPUData), node_data.data(), GL_STATIC_DRAW);

	m_opengl_context->doneCurrent();
}

NodePriorityComputation::~NodePriorityComputation()
{
}

void NodePriorityComputation::computePriorities(const navigation::Camera& camera)
{
	// Also adapt the shader code
	m_shader_program.bind();

	auto mvp = camera.viewProjectionMatrix();
	m_shader_program.setUniformValue("u_mvp", mvp);
	m_shader_program.setUniformValue("u_camera_position", camera.cameraSpecifications().eye);
	m_shader_program.setUniformValue("u_screen_width", camera.viewportWidth());
	m_shader_program.setUniformValue("u_screen_height", camera.viewportHeight());

	std::array<QVector4D, 6> view_frustum = {
		mvp.row(3) + mvp.row(0),
		mvp.row(3) - mvp.row(0),
		mvp.row(3) + mvp.row(1),
		mvp.row(3) - mvp.row(1),
		mvp.row(3) + mvp.row(2),
		mvp.row(3) - mvp.row(2)
	};
	m_shader_program.setUniformValueArray("u_camera_frustum", view_frustum.data(), 6);

	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bbox_buffer);
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_result_buffer.handle());

	for (const auto& level : m_level_bounds)
	{
		m_opengl_context->gl()->glUniform1ui(m_shader_program.uniformLocation("u_level_start"), level.start); // no support for uint in QOpenGLShaderProgram::setUniformValue
		m_opengl_context->gl()->glUniform1ui(m_shader_program.uniformLocation("u_level_end"), level.end);
		m_opengl_context->gl()->glDispatchCompute(level.workgroup_size, 1, 1);
		m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	m_shader_program.release();

	m_result_buffer.waitForGPUWrite();
}

}
