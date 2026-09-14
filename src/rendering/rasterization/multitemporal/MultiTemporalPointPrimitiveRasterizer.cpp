#include "MultiTemporalPointPrimitiveRasterizer.h"

#include "rendering/Framebuffer.h"
#include "rendering/pointcloudProviders/MultiTemporalPointCloudProvider.h"
#include "ui/RenderWindow.h"
#include "utils/Profiler.h"
#include "utils/ShaderUtilities.h"

#include <iostream>

namespace sahara::rendering
{

const char* PRECOMPUTED_DISTANCE_FIELD_NAME = "scalar_significant_change"; // this is the attribute name assigned by CloudCompare for M3C2 differences. If such an attribute is present, we enable the Differences_P mode.

MultiTemporalPointPrimitiveRasterizer::MultiTemporalPointPrimitiveRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: AbstractRasterizer(opengl_context, provider, camera)
	, m_side_by_side_pass(opengl_context, camera)
	, m_split_view_pass(opengl_context, camera)
	, m_change_highlighting_pass(opengl_context, camera, false)
	, m_precomputed_change_highlighting_pass(opengl_context, camera, true)
{
	m_point_size_parameter = std::make_unique<RangeParameter<int>>("Point Size", 1, 1, 50, 1);
	connect(m_point_size_parameter.get(), &RangeParameter<int>::valueChanged, this, &MultiTemporalPointPrimitiveRasterizer::onPointSizeChanged);
	m_parameters.push_back(m_point_size_parameter.get());

	auto comparison_modes = std::vector<QString>{ "Standard", "Side-to-Side", "Split View", "Differences_R" };
	if (provider->hasAttribute(PRECOMPUTED_DISTANCE_FIELD_NAME)) // this is the attribute name assigned by CloudCompare for M3C2 differences
	{
		comparison_modes.push_back("Differences_P");
	}
	m_comparison_mode_parameter = std::make_unique<EnumParameter>("Comparison Mode", comparison_modes, 0u);
	m_parameters.push_back(m_comparison_mode_parameter.get());
	connect(m_comparison_mode_parameter.get(), &EnumParameter::valueChanged, this, &MultiTemporalPointPrimitiveRasterizer::onComparisonModeChanged);

	m_added_color_parameter = std::make_unique<Parameter<QColor>>("Added Segments Color", QColor::fromRgbF(0.0, 1.0, 1.0, 1.0f));
	connect(m_added_color_parameter.get(), &Parameter<QColor>::valueChanged, this, &MultiTemporalPointPrimitiveRasterizer::onAddedColorChanged);
	m_parameters.push_back(m_added_color_parameter.get());

	m_removed_color_parameter = std::make_unique<Parameter<QColor>>("Removed Segments Color", QColor::fromRgbF(1.0, 0.0, 1.0, 1.0f));
	connect(m_removed_color_parameter.get(), &Parameter<QColor>::valueChanged, this, &MultiTemporalPointPrimitiveRasterizer::onRemovedColorChanged);
	m_parameters.push_back(m_removed_color_parameter.get());

	m_epsilon_parameter = std::make_unique<RangeParameter<float>>("Epsilon", 0.003, 0.0, 0.1, 0.0005);
	connect(m_epsilon_parameter.get(), &RangeParameter<float>::valueChanged, this, &MultiTemporalPointPrimitiveRasterizer::onEpsilonChanged);
	m_parameters.push_back(m_epsilon_parameter.get());

	m_lens_size_parameter = std::make_unique<RangeParameter<float>>("Focus Lens Size", 0.2, 0.01, 1.0, 0.01);
	connect(m_lens_size_parameter.get(), &RangeParameter<float>::valueChanged, this, &MultiTemporalPointPrimitiveRasterizer::onLensSizeChanged);
	m_parameters.push_back(m_lens_size_parameter.get());

	connect(m_pointcloud_provider, &AbstractPointCloudProvider::pointBudgetChanged, [this]() { m_opengl_context->makeCurrent(); initializeVAO(); m_opengl_context->doneCurrent(); });
}

MultiTemporalPointPrimitiveRasterizer::~MultiTemporalPointPrimitiveRasterizer()
{
}

ShaderPaths MultiTemporalPointPrimitiveRasterizer::getShaderPaths() const noexcept
{
	ShaderPaths paths;
	paths[QOpenGLShader::Vertex] = "./data/shaders/PointPrimitiveRasterizer.vert";
	paths[QOpenGLShader::Fragment] = "./data/shaders/PointPrimitiveRasterizer.frag";
	return paths;
}

void MultiTemporalPointPrimitiveRasterizer::recompileShaders(const std::set<geometry::AttributeSpecification>& required_outputs)
{
	m_side_by_side_pass.recompileShaders();
	m_split_view_pass.recompileShaders();
	m_change_highlighting_pass.recompileShaders();
	m_precomputed_change_highlighting_pass.recompileShaders();

	auto factory = ShaderProgramFactory(m_opengl_context);

	m_colorizer->reloadShaderSpecificationsFromDisk();

	auto inputs = requiredAttributes(required_outputs);

	auto final_required_outputs = required_outputs;

	// In the case of precomputed M3C2 distances, we have to write them into a framebuffer attachment
	if (m_comparison_mode_parameter->selectedEntry() == "Differences_P")
	{
		auto metadata = m_pointcloud_provider->attributeMetadata(PRECOMPUTED_DISTANCE_FIELD_NAME);
		final_required_outputs.insert({ metadata->type, metadata->semantic });
	}

	m_shader_program = factory.createShaderProgram(getShaderPaths(), m_colorizer->finalShaderCode(), inputs, final_required_outputs);

	m_opengl_context->makeCurrent();
	m_shader_program->bind();

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_DEPTH_TEST);

	// set uniform values of new program according to current parameter values
	m_shader_program->setUniformValue("u_point_size", static_cast<float>(m_point_size_parameter->value()));

	if (!geometry::attributeSpecificationsEqual(inputs, m_required_input_attributes))
	{
		m_required_input_attributes = inputs;
		initializeVAO();
	}

	m_shader_program->release();
	m_opengl_context->doneCurrent();

	m_colorizer->setCompiledShaderProgram(m_shader_program.get());
	m_side_by_side_pass.setBaseShaderProgram(m_shader_program.get());
	m_split_view_pass.setBaseShaderProgram(m_shader_program.get());
	m_change_highlighting_pass.setBaseShaderProgram(m_shader_program.get());
	m_precomputed_change_highlighting_pass.setBaseShaderProgram(m_shader_program.get());

	onAddedColorChanged();
	onRemovedColorChanged();
	onEpsilonChanged();
	onLensSizeChanged();
}

void MultiTemporalPointPrimitiveRasterizer::run(Framebuffer* framebuffer, int read_fbo_index)
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

	m_vao.bind();
	draw_command_buffer.bind();

	if (m_comparison_mode_parameter->selectedEntry() == "Standard")
	{
		drawStandardView();
	}

	else if (m_comparison_mode_parameter->selectedEntry() == "Side-to-Side")
	{
		m_side_by_side_pass.run();
	}

	else if (m_comparison_mode_parameter->selectedEntry() == "Split View")
	{
		m_split_view_pass.run();
	}

	else if (m_comparison_mode_parameter->selectedEntry() == "Differences_R")
	{
		m_change_highlighting_pass.run();
	}

	else if (m_comparison_mode_parameter->selectedEntry() == "Differences_P")
	{
		m_precomputed_change_highlighting_pass.run();
	}

	draw_command_buffer.release();
	m_vao.release();

#ifdef PROFILER_ENABLED
	m_opengl_context->gl()->glEndQuery(GL_TIME_ELAPSED);
	GLuint elapsed_time;
	m_opengl_context->gl()->glGetQueryObjectuiv(gl_timer_query, GL_QUERY_RESULT, &elapsed_time);
	m_opengl_context->gl()->glDeleteQueries(1, &gl_timer_query);
	utils::global_profiler.addMeasurement("Rasterization", static_cast<int>(elapsed_time / 1000u));
#endif
}

void MultiTemporalPointPrimitiveRasterizer::setFramebuffer(Framebuffer* framebuffer)
{
	m_change_highlighting_pass.setFramebuffer(framebuffer);
	m_precomputed_change_highlighting_pass.setFramebuffer(framebuffer);
}

std::set<geometry::AttributeSpecification> MultiTemporalPointPrimitiveRasterizer::requiredAttributes(const std::set<geometry::AttributeSpecification>& required_outputs) const
{
	auto required_attributes = AbstractRasterizer::requiredAttributes(required_outputs);

	if (m_comparison_mode_parameter->selectedEntry() == "Differences_P")
	{
		auto metadata = m_pointcloud_provider->attributeMetadata(PRECOMPUTED_DISTANCE_FIELD_NAME);
		required_attributes.insert({ metadata->type, metadata->semantic });
	}

	return required_attributes;
}

RasterizerType MultiTemporalPointPrimitiveRasterizer::type() const noexcept
{
	return RasterizerType::MultiTemporalPointPrimitiveRasterizer;
}

std::vector<AbstractRasterizer::AnnotationViewport> MultiTemporalPointPrimitiveRasterizer::annotationViewports() const
{
	if (m_comparison_mode_parameter->selectedEntry() != "Side-to-Side")
	{
		return AbstractRasterizer::annotationViewports();
	}

	const int width = static_cast<int>(m_camera->viewportWidth());
	const int height = static_cast<int>(m_camera->viewportHeight());
	const int split_position = width / 2;
	if (split_position <= 0 || height <= 0)
	{
		return AbstractRasterizer::annotationViewports();
	}

	const auto& camera_specifications = m_camera->cameraSpecifications();
	QMatrix4x4 half_projection;
	half_projection.setToIdentity();
	half_projection.perspective(camera_specifications.fov,
								static_cast<float>(split_position) / static_cast<float>(height),
								camera_specifications.near_plane,
								camera_specifications.far_plane);
	const QMatrix4x4 half_view_projection = half_projection * m_camera->viewMatrix();

	return { { QRect(0, 0, split_position, height), half_view_projection },
			 { QRect(split_position, 0, width - split_position, height), half_view_projection } };
}

void MultiTemporalPointPrimitiveRasterizer::drawStandardView()
{
	if (ui::RenderWindow::s_event_cache.isKeyPressed(Qt::Key_Control) && ui::RenderWindow::s_event_cache.didMouseReleaseOccur())
	{
		if (ui::RenderWindow::s_event_cache.getPressedButton() == Qt::MouseButton::LeftButton)
		{
			static_cast<MultiTemporalPointCloudProvider*>(m_pointcloud_provider)->decreaseTimestamp();
		}

		if (ui::RenderWindow::s_event_cache.getPressedButton() == Qt::MouseButton::RightButton)
		{
			static_cast<MultiTemporalPointCloudProvider*>(m_pointcloud_provider)->increaseTimestamp();
		}
	}

	m_shader_program->bind();
	m_shader_program->setUniformValue("u_mvp", m_camera->viewProjectionMatrix());
	m_opengl_context->gl()->glDrawArraysIndirect(GL_POINTS, (void*) (0 * sizeof(DrawArraysIndirectCommand)));
	m_shader_program->release();
}

void MultiTemporalPointPrimitiveRasterizer::initializeVAO()
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

void MultiTemporalPointPrimitiveRasterizer::setVertexAttribute(const geometry::AttributeSpecification& attribute)
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

void MultiTemporalPointPrimitiveRasterizer::onPointSizeChanged()
{
	m_opengl_context->makeCurrent();
	m_shader_program->bind();
	m_shader_program->setUniformValue("u_point_size", static_cast<float>(m_point_size_parameter->value()));
	m_shader_program->release();
	m_opengl_context->doneCurrent();
}

void MultiTemporalPointPrimitiveRasterizer::onComparisonModeChanged()
{
	if (m_comparison_mode_parameter->selectedEntry() == "Differences_P")
	{
		emit requiredAttributesChanged();
	}
}

void MultiTemporalPointPrimitiveRasterizer::onAddedColorChanged()
{
	m_change_highlighting_pass.setAddedColor(m_added_color_parameter->value());
	m_precomputed_change_highlighting_pass.setAddedColor(m_added_color_parameter->value());
}

void MultiTemporalPointPrimitiveRasterizer::onRemovedColorChanged()
{
	m_change_highlighting_pass.setRemovedColor(m_removed_color_parameter->value());
	m_precomputed_change_highlighting_pass.setRemovedColor(m_removed_color_parameter->value());
}

void MultiTemporalPointPrimitiveRasterizer::onEpsilonChanged()
{
	m_change_highlighting_pass.setDifferenceThreshold(m_epsilon_parameter->value());

	if (m_pointcloud_provider->hasAttribute(PRECOMPUTED_DISTANCE_FIELD_NAME))
	{
		auto typed_distance_attribute = dynamic_cast<const geometry::TypedAttributeMetadata<float>*>(m_pointcloud_provider->attributeMetadata(PRECOMPUTED_DISTANCE_FIELD_NAME));
		m_precomputed_change_highlighting_pass.setDifferenceThreshold(m_epsilon_parameter->value() * typed_distance_attribute->maximum);
	}
}

void MultiTemporalPointPrimitiveRasterizer::onLensSizeChanged()
{
	m_change_highlighting_pass.setLensRadius(m_lens_size_parameter->value());
	m_precomputed_change_highlighting_pass.setLensRadius(m_lens_size_parameter->value());
}
}
