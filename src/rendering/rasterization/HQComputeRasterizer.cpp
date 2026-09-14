#include "HQComputeRasterizer.h"

#include "rendering/ShaderProgramFactory.h"
#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLBuffer>
#include <iostream>

namespace sahara::rendering
{

HQComputeRasterizer::HQComputeRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: AbstractComputeRasterizer(opengl_context, provider, camera)
	, m_intermediate_buffers_size(0)
{
	m_threshold_parameter = std::make_unique<RangeParameter<float>>("HQ Threshold", 0.0001f, 0.0f, 0.1f, 0.0001f);
	connect(m_threshold_parameter.get(), &RangeParameter<float>::valueChanged, this, &HQComputeRasterizer::onThresholdChanged);
	m_parameters.push_back(m_threshold_parameter.get());
}

HQComputeRasterizer::~HQComputeRasterizer()
{
}

RasterizerType HQComputeRasterizer::type() const noexcept
{
	return RasterizerType::HQComputeRasterizer;
}

void HQComputeRasterizer::createShaderPrograms(ShaderProgramFactory& factory, const QString& colorization_shader_code, const std::set<geometry::AttributeSpecification>& required_outputs)
{
	ShaderPaths clear_program_paths;
	clear_program_paths[QOpenGLShader::Compute] = "./data/shaders/hq-compute-rasterizer/ClearStep.comp";
	m_clear_program = factory.createShaderProgram(clear_program_paths, colorization_shader_code, m_attributes, required_outputs);

	ShaderPaths depth_program_paths;
	depth_program_paths[QOpenGLShader::Compute] = "./data/shaders/hq-compute-rasterizer/DepthStep.comp";
	m_depth_program = factory.createShaderProgram(depth_program_paths, colorization_shader_code, m_attributes, required_outputs);

	ShaderPaths color_program_paths;
	color_program_paths[QOpenGLShader::Compute] = "./data/shaders/hq-compute-rasterizer/ColorStep.comp";
	m_color_program = factory.createShaderProgram(color_program_paths, colorization_shader_code, m_attributes, required_outputs);

	ShaderPaths resolve_program_paths;
	resolve_program_paths[QOpenGLShader::Vertex] = "./data/shaders/hq-compute-rasterizer/ResolveStep.vert";
	resolve_program_paths[QOpenGLShader::Fragment] = "./data/shaders/hq-compute-rasterizer/ResolveStep.frag";
	m_resolve_program = factory.createShaderProgram(resolve_program_paths, colorization_shader_code, m_attributes, required_outputs);

	// To set initial point size and threshold
	onPointSizeChanged();
	onThresholdChanged();

	m_colorizer->setCompiledShaderProgram(m_color_program.get());
}

void HQComputeRasterizer::runDepthProgram(Framebuffer* framebuffer, int read_fbo_index)
{
	GLuint dispatch_groups_x = std::ceil(static_cast<float>(m_pointcloud_provider->numberOfPoints()) / 1024.0);

	m_depth_program->bind();
	m_depth_program->setUniformValue("u_FramebufferSize", QVector2D(framebuffer->width(), framebuffer->height()));
	m_depth_program->setUniformValue("u_ModelViewProjectionMatrix", m_camera->viewProjectionMatrix());

	m_opengl_context->gl()->glDispatchCompute(dispatch_groups_x, 1, 1);
	m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	m_depth_program->release();
}

void HQComputeRasterizer::runColorProgram(Framebuffer* framebuffer, int read_fbo_index)
{
	GLuint dispatch_groups_x = std::ceil(static_cast<float>(m_pointcloud_provider->numberOfPoints()) / 1024.0);

	m_color_program->bind();
	m_color_program->setUniformValue("u_FramebufferSize", QVector2D(framebuffer->width(), framebuffer->height()));
	m_color_program->setUniformValue("u_ModelViewProjectionMatrix", m_camera->viewProjectionMatrix());

	m_opengl_context->gl()->glDispatchCompute(dispatch_groups_x, 1, 1);
	m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	m_color_program->release();
}

void HQComputeRasterizer::runPrograms(Framebuffer* framebuffer, int read_fbo_index)
{
	runClearProgram(framebuffer, read_fbo_index);
	runDepthProgram(framebuffer, read_fbo_index);
	runColorProgram(framebuffer, read_fbo_index);
	runResolveProgram(framebuffer, read_fbo_index);
}

void HQComputeRasterizer::updateIntermediateBuffers(int width, int height)
{
	if (m_intermediate_buffers_size == width * height)
	{
		return;
	}

	m_intermediate_buffers_size = width * height;

	if (m_intermediate_color_buffer.isCreated())
	{
		m_intermediate_color_buffer.destroy();
	}

	m_intermediate_color_buffer.create();
	m_intermediate_color_buffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_intermediate_color_buffer.bind();
	m_intermediate_color_buffer.allocate(nullptr, sizeof(uint32_t) * m_intermediate_buffers_size * 4);
	m_intermediate_color_buffer.release();

	if (m_intermediate_depth_buffer.isCreated())
	{
		m_intermediate_depth_buffer.destroy();
	}

	m_intermediate_depth_buffer.create();
	m_intermediate_depth_buffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	m_intermediate_depth_buffer.bind();
	m_intermediate_depth_buffer.allocate(nullptr, sizeof(uint32_t) * m_intermediate_buffers_size);
	m_intermediate_depth_buffer.release();
}

void HQComputeRasterizer::bindGPUBuffers()
{
	AbstractComputeRasterizer::bindGPUBuffers();
	m_intermediate_depth_buffer.bind();
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_intermediate_depth_buffer.bufferId());
	m_intermediate_depth_buffer.release();
	m_intermediate_color_buffer.bind();
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_intermediate_color_buffer.bufferId());
	m_intermediate_color_buffer.release();
}

inline int HQComputeRasterizer::getFirstAttributeSSBOBindingLocation() const noexcept
{
	return 2;
}

void HQComputeRasterizer::onThresholdChanged()
{
	utils::setUniformValue(m_opengl_context, *m_color_program.get(), "u_threshold", m_threshold_parameter->value());
}

}
