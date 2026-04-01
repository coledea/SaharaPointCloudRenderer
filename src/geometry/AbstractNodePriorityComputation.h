#pragma once

#include "LodOctreeNode.h"
#include "navigation/Camera.h"
#include "rendering/OpenGLContext.h"
#include "rendering/pointcloudProviders/MappedOpenGLBuffer.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

namespace sahara::geometry
{

class AbstractNodePriorityComputation
{
public:
	AbstractNodePriorityComputation(rendering::OpenGLContext* context, uint32_t num_nodes, const std::string& shader_path);
	~AbstractNodePriorityComputation();

	const float* priorities() const noexcept;

	virtual void computePriorities(const navigation::Camera& camera) = 0;

	void setProjectionSizeRenderThreshold(int threshold);

	GLuint nodeMetadataBuffer() const noexcept;

protected:
	QOpenGLShaderProgram m_shader_program;
	rendering::MappedOpenGLBuffer m_result_buffer;
	GLuint m_bbox_buffer; // TODO: instead of managing the node metadata buffer here, we could also keep it in the lod octree, since it is not only required for priority computation, but also for rasterization
	rendering::OpenGLContext* m_opengl_context;
	GLuint m_workgroup_size;
};

}