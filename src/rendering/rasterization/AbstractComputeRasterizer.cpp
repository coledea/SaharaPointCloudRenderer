#include "AbstractComputeRasterizer.h"

#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <QOpenGLBuffer>

namespace sahara::rendering
{

AbstractComputeRasterizer::AbstractComputeRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: AbstractRasterizer(opengl_context, provider, camera)
	, m_fullscreen_geometry(std::make_unique<utils::FullscreenGeometry>(opengl_context))
{
	m_point_size_parameter = std::make_unique<RangeParameter<int>>("Point Size", 1, 1, 50, 1);
	connect(m_point_size_parameter.get(), &RangeParameter<int>::valueChanged, this, &AbstractComputeRasterizer::onPointSizeChanged);
	m_parameters.push_back(m_point_size_parameter.get());
}

AbstractComputeRasterizer::~AbstractComputeRasterizer()
{
}

void AbstractComputeRasterizer::recompileShaders(const std::set<geometry::AttributeSpecification>& required_outputs)
{
	m_colorizer->reloadShaderSpecificationsFromDisk();

	m_attributes = requiredAttributes(required_outputs);
	auto factory = ShaderProgramFactory(m_opengl_context);

	createShaderPrograms(factory, m_colorizer->finalShaderCode(), required_outputs);

	m_opengl_context->makeCurrent();
	m_resolve_program->bind();
	m_fullscreen_geometry->bind();
	m_resolve_program->enableAttributeArray(0);
	m_resolve_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
	m_fullscreen_geometry->release();
	m_resolve_program->release();
	m_opengl_context->doneCurrent();
}

void AbstractComputeRasterizer::runClearProgram(Framebuffer* framebuffer, int read_fbo_index)
{
	GLuint dispatch_groups = std::ceil(static_cast<float>(framebuffer->width() * framebuffer->height()) / 1024.0);

	m_clear_program->bind();

	m_opengl_context->gl()->glDispatchCompute(dispatch_groups, 1, 1);
	m_opengl_context->gl()->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	m_clear_program->release();
}

void AbstractComputeRasterizer::runResolveProgram(Framebuffer* framebuffer, int read_fbo_index)
{
	m_resolve_program->bind();
	m_opengl_context->gl()->glUniform2i(m_resolve_program->uniformLocation("u_FramebufferSize"), framebuffer->width(), framebuffer->height());

	m_fullscreen_geometry->draw();

	m_resolve_program->release();
}

void AbstractComputeRasterizer::run(Framebuffer* framebuffer, int read_fbo_index)
{
#ifdef PROFILER_ENABLED
	GLuint gl_timer_query;
	m_opengl_context->gl()->glGenQueries(1, &gl_timer_query);
	m_opengl_context->gl()->glBeginQuery(GL_TIME_ELAPSED, gl_timer_query);
#endif

	updateIntermediateBuffers(framebuffer->width(), framebuffer->height());
	bindGPUBuffers();
	runPrograms(framebuffer, read_fbo_index);

#ifdef PROFILER_ENABLED
	m_opengl_context->gl()->glEndQuery(GL_TIME_ELAPSED);
	GLuint elapsed_time;
	m_opengl_context->gl()->glGetQueryObjectuiv(gl_timer_query, GL_QUERY_RESULT, &elapsed_time);
	m_opengl_context->gl()->glDeleteQueries(1, &gl_timer_query);
	utils::global_profiler.addMeasurement("Rasterization", static_cast<int>(elapsed_time / 1000u));
#endif
}

void AbstractComputeRasterizer::bindGPUBuffers()
{
	for (const auto& attribute : m_attributes)
	{
		bindGPUBuffer(attribute);
	}
}

void AbstractComputeRasterizer::bindGPUBuffer(const geometry::AttributeSpecification& attribute)
{
	uint offset = getFirstAttributeSSBOBindingLocation();
	switch (attribute.semantic)
	{
		case geometry::AttributeSemantic::Position:
			offset += 0;
			break;
		case geometry::AttributeSemantic::Color:
			offset += 1;
			break;
		case geometry::AttributeSemantic::Normal:
			offset += 2;
			break;
		case geometry::AttributeSemantic::ID:
			offset += 3;
			break;
		case geometry::AttributeSemantic::SegmentID:
			offset += 4;
			break;
		default:
			offset += 5;
			break;
	}

	m_pointcloud_provider->bindGPUBuffer(attribute.semantic);
	m_opengl_context->gl()->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, offset, m_pointcloud_provider->getGPUBuffer(attribute.semantic));
	m_pointcloud_provider->releaseGPUBuffer(attribute.semantic);
}

void AbstractComputeRasterizer::onPointSizeChanged()
{
	utils::setUniformValue(m_opengl_context, *m_resolve_program.get(), "u_PointSize", m_point_size_parameter->value());
}

}
