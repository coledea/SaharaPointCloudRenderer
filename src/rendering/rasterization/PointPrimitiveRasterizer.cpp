#include "PointPrimitiveRasterizer.h"

#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <iostream>

namespace sahara::rendering
{

PointPrimitiveRasterizer::PointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: AbstractRasterizer(opengl_context, provider, camera)
{
	m_point_size_parameter = std::make_unique<RangeParameter<int>>("Point Size", 1, 1, 50, 1);
	connect(m_point_size_parameter.get(), &RangeParameter<int>::valueChanged, this, &PointPrimitiveRasterizer::onPointSizeChanged);
	m_parameters.push_back(m_point_size_parameter.get());

	connect(m_pointcloud_provider, &AbstractPointCloudProvider::pointBudgetChanged, [this]() { m_opengl_context->makeCurrent(); initializeVAO(); m_opengl_context->doneCurrent(); });
}

PointPrimitiveRasterizer::~PointPrimitiveRasterizer()
{
}

void PointPrimitiveRasterizer::reloadShaderSpecificationsFromDisk()
{
	m_shader_specifications.vertex_shader_path = "./data/shaders/PointPrimitiveRasterizer.vert";
	m_shader_specifications.fragment_shader_path = "./data/shaders/PointPrimitiveRasterizer.frag";
}

RasterizerType PointPrimitiveRasterizer::type() const noexcept
{
	return RasterizerType::PointPrimitiveRasterizer;
}

void PointPrimitiveRasterizer::setCompiledShaderProgram(QOpenGLShaderProgram* shader_program, const std::set<geometry::AttributeSpecification>& input_attributes)
{
	m_opengl_context->makeCurrent();
	m_shader_program = shader_program;
	m_shader_program->bind();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);

	// set uniform values of new program according to current parameter values
	m_shader_program->setUniformValue("u_point_size", static_cast<float>(m_point_size_parameter->value()));

	if (!geometry::attributeSpecificationsEqual(input_attributes, m_required_input_attributes))
	{
		m_required_input_attributes = input_attributes;
		initializeVAO();
	}

	m_shader_program->release();
	m_opengl_context->doneCurrent();
}

void PointPrimitiveRasterizer::run()
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
}

void PointPrimitiveRasterizer::initializeVAO()
{
	if (m_vao.isCreated())
	{
		m_vao.destroy();
	}

	m_vao.create();
	QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

	for (const auto attribute : m_required_input_attributes)
	{
		if (m_pointcloud_provider->hasAttribute(attribute.semantic))
		{
			setVertexAttribute(attribute);
		}
		else
		{
			std::cout << "Required attribute not available: " << utils::ShaderStringsFactory::vertexShaderInputName(attribute.semantic) << std::endl;
		}
	}
}

void PointPrimitiveRasterizer::setVertexAttribute(const geometry::AttributeSpecification& attribute)
{
	int attribute_components = geometry::numberOfComponents(attribute.type);
	m_pointcloud_provider->bindGPUBuffer(attribute.semantic);
	auto attribute_location = m_opengl_context->gl()->glGetAttribLocation(m_shader_program->programId(), utils::ShaderStringsFactory::vertexShaderInputName(attribute.semantic).c_str());
	m_opengl_context->gl()->glEnableVertexAttribArray(attribute_location);

	if (attribute.type == geometry::AttributeType::Int || attribute.type == geometry::AttributeType::Uint)
	{
		m_opengl_context->gl()->glVertexAttribIPointer(attribute_location, attribute_components, utils::ATTRIBUTE_GL_TYPES.at(attribute.type), 0, nullptr);
	}
	else
	{
		bool normalize = attribute.type == geometry::AttributeType::Color; // colors are stored as uint8_t and have to be normalized
		m_opengl_context->gl()->glVertexAttribPointer(attribute_location, attribute_components, utils::ATTRIBUTE_GL_TYPES.at(attribute.type), normalize, 0, nullptr);
	}

	m_pointcloud_provider->releaseGPUBuffer(attribute.semantic);
}

void PointPrimitiveRasterizer::onPointSizeChanged()
{
	m_opengl_context->makeCurrent();
	m_shader_program->bind();
	m_shader_program->setUniformValue("u_point_size", static_cast<float>(m_point_size_parameter->value()));
	m_shader_program->release();
	m_opengl_context->doneCurrent();
}

}
