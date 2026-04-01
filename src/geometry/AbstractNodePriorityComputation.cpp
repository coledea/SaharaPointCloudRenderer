#include "AbstractNodePriorityComputation.h"

#include "utils/ShaderUtilities.h"

#include <QtMath>

namespace sahara::geometry
{

AbstractNodePriorityComputation::AbstractNodePriorityComputation(rendering::OpenGLContext* context, uint32_t num_nodes, const std::string& shader_path)
	: m_opengl_context(context)
	, m_result_buffer(context, GL_READ_ONLY)
	, m_workgroup_size(static_cast<GLuint>(std::ceil(static_cast<float>(num_nodes) / 1024.0)))
{
	m_opengl_context->makeCurrent();

	if (!m_shader_program.addShaderFromSourceCode(QOpenGLShader::Compute, utils::ShaderStringsFactory::readShaderFile(shader_path.c_str())))
	{
		qDebug() << "Compute shader compilation error:" << m_shader_program.log();
	}

	if (!m_shader_program.link())
	{
		qDebug() << "Program linking error!" << m_shader_program.log();
	}

	m_result_buffer.resize(num_nodes * sizeof(float));
	m_opengl_context->doneCurrent();
}

AbstractNodePriorityComputation::~AbstractNodePriorityComputation()
{
	m_opengl_context->makeCurrent();
	if (m_bbox_buffer != 0)
	{
		m_opengl_context->gl()->glDeleteBuffers(1, &m_bbox_buffer);
	}
	m_opengl_context->doneCurrent();
}

const float* AbstractNodePriorityComputation::priorities() const noexcept
{
	return static_cast<float*>(m_result_buffer.memoryPtr());
}

void AbstractNodePriorityComputation::setProjectionSizeRenderThreshold(int threshold)
{
	utils::setUniformValue(m_opengl_context, m_shader_program, "u_projection_size_render_threshold", threshold);
}

GLuint AbstractNodePriorityComputation::nodeMetadataBuffer() const noexcept
{
	return m_bbox_buffer;
}

}
