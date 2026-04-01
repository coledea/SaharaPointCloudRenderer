#include "OOCPointPrimitiveRasterizer.h"

#include "rendering/pointcloudProviders/AbstractOOCPointCloudProvider.h"
#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <iostream>

namespace sahara::rendering
{

OOCPointPrimitiveRasterizer::OOCPointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: PointPrimitiveRasterizer(opengl_context, provider, camera)
{
}

OOCPointPrimitiveRasterizer::~OOCPointPrimitiveRasterizer()
{
}

void OOCPointPrimitiveRasterizer::reloadShaderSpecificationsFromDisk()
{
	m_shader_specifications.vertex_shader_path = "./data/shaders/OOCPointPrimitiveRasterizer.vert";
	m_shader_specifications.fragment_shader_path = "./data/shaders/PointPrimitiveRasterizer.frag";
}

RasterizerType OOCPointPrimitiveRasterizer::type() const noexcept
{
	return RasterizerType::OOCPointPrimitiveRasterizer;
}

void OOCPointPrimitiveRasterizer::run()
{
	auto& draw_command_buffer = m_pointcloud_provider->drawCommandBuffer();
	if (draw_command_buffer.numberOfCommands() == 0)
	{
		return;
	}

#ifdef PROFILER_ENABLED
	GLuint gl_timer_query;
	m_opengl_context->gl()->glGenQueries(1, &gl_timer_query);
	m_opengl_context->gl()->glBeginQuery(GL_TIME_ELAPSED, gl_timer_query);
#endif

	m_shader_program->bind();
	m_vao.bind();
	draw_command_buffer.bind();

	auto provider = dynamic_cast<AbstractOOCPointCloudProvider*>(m_pointcloud_provider);
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, provider->nodeMetadataBuffer());
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, provider->positionOffsetsBuffer());
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, provider->positionsBuffer());

	m_shader_program->setUniformValue("u_mvp", m_camera->viewProjectionMatrix());

	m_opengl_context->gl()->glMultiDrawArraysIndirect(GL_POINTS, 0, static_cast<GLsizei>(draw_command_buffer.numberOfCommands()), 0);

	draw_command_buffer.release();
	m_vao.release();
	m_shader_program->release();

#ifdef PROFILER_ENABLED
	m_opengl_context->gl()->glEndQuery(GL_TIME_ELAPSED);
	GLuint elapsed_time;
	m_opengl_context->gl()->glGetQueryObjectuiv(gl_timer_query, GL_QUERY_RESULT, &elapsed_time);
	m_opengl_context->gl()->glDeleteQueries(1, &gl_timer_query);
	utils::global_profiler.addMeasurement("Rasterization", static_cast<int>(elapsed_time / 1000u));
#endif

	provider->setRenderFence(m_opengl_context->gl()->glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
}

void OOCPointPrimitiveRasterizer::initializeVAO()
{
	// remove position attribute. We manage it separately, as points and voxels store positions differently.
	auto position_it = m_required_input_attributes.find(geometry::AttributeSpecification{ geometry::AttributeType::Vector3D, geometry::AttributeSemantic::Position });
	if (position_it != m_required_input_attributes.end())
	{
		m_required_input_attributes.erase(position_it);
	}
	PointPrimitiveRasterizer::initializeVAO();
}

}
