#include "PointComputeRasterizer.h"

#include "rendering/ShaderProgramFactory.h"
#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLBuffer>
#include <iostream>

namespace sahara::rendering
{

PointComputeRasterizer::PointComputeRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: AbstractComputeRasterizer(opengl_context, provider, camera)
	, m_intermediate_buffer_size(0)
{
}

PointComputeRasterizer::~PointComputeRasterizer()
{
}

RasterizerType PointComputeRasterizer::type() const noexcept
{
	return RasterizerType::PointComputeRasterizer;
}

void PointComputeRasterizer::createShaderPrograms(ShaderProgramFactory& factory, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification>& required_outputs)
{
	ShaderPaths clear_program_paths;
	clear_program_paths[QOpenGLShader::Compute] = "./data/shaders/compute-rasterizer/ClearStep.comp";
	m_clear_program = factory.createShaderProgram(clear_program_paths, colorization_shader_code, m_attributes, required_outputs);

	ShaderPaths render_program_paths;
	render_program_paths[QOpenGLShader::Compute] = "./data/shaders/compute-rasterizer/RenderStep.comp";
	m_render_program = factory.createShaderProgram(render_program_paths, colorization_shader_code, m_attributes, required_outputs);

	ShaderPaths resolve_program_paths;
	resolve_program_paths[QOpenGLShader::Vertex] = "./data/shaders/compute-rasterizer/ResolveStep.vert";
	resolve_program_paths[QOpenGLShader::Fragment] = "./data/shaders/compute-rasterizer/ResolveStep.frag";
	m_resolve_program = factory.createShaderProgram(resolve_program_paths, colorization_shader_code, m_attributes, required_outputs);

	// To set initial point size
	onPointSizeChanged();

	m_colorizer->setCompiledShaderProgram(m_resolve_program.get());
}

void PointComputeRasterizer::runRenderProgram(Framebuffer* framebuffer, int read_fbo_index)
{
	GLuint dispatch_groups_x = std::ceil(static_cast<float>(m_pointcloud_provider->numberOfPoints()) / 1024.0);

	m_render_program->bind();
	m_opengl_context->gl()->glUniform2i(m_render_program->uniformLocation("u_FramebufferSize"), framebuffer->width(), framebuffer->height());
	m_render_program->setUniformValue("u_ModelViewProjectionMatrix", m_camera->viewProjectionMatrix());

	m_opengl_context->gl()->glDispatchCompute(dispatch_groups_x, 1, 1);
	m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	m_render_program->release();
}

void PointComputeRasterizer::runPrograms(Framebuffer* framebuffer, int read_fbo_index)
{
	runClearProgram(framebuffer, read_fbo_index);
	runRenderProgram(framebuffer, read_fbo_index);
	runResolveProgram(framebuffer, read_fbo_index);
}

void PointComputeRasterizer::updateIntermediateBuffers(int width, int height)
{
	if (m_intermediate_buffer_size == width * height)
	{
		return;
	}

	m_intermediate_buffer_size = width * height;

	if (m_intermediate_buffer.isCreated())
	{
		m_intermediate_buffer.destroy();
	}

	m_intermediate_buffer.create();
	m_intermediate_buffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_intermediate_buffer.bind();
	m_intermediate_buffer.allocate(nullptr, sizeof(uint64_t) * m_intermediate_buffer_size);
	m_intermediate_buffer.release();
}

void PointComputeRasterizer::bindGPUBuffers()
{
	AbstractComputeRasterizer::bindGPUBuffers();
	m_intermediate_buffer.bind();
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_intermediate_buffer.bufferId());
	m_intermediate_buffer.release();
}

inline int PointComputeRasterizer::getFirstAttributeSSBOBindingLocation() const noexcept
{
	return 1;
}

}
