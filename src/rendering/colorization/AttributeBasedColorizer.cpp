#include "AttributeBasedColorizer.h"

#include "ColorizerSpecifications.h"

namespace sahara::rendering
{

inline std::vector<QString> retrieveAvailableAttributes(AbstractPointCloudProvider* pointcloud_provider)
{
	std::vector<QString> available_attributes{ "depth" };
	for (const auto attribute : pointcloud_provider->attributesMetadata())
	{
		available_attributes.push_back(attribute.second->name);
	}
	return available_attributes;
}

// These defines are used in shaders to mark whether certain vertex attributes are available.
inline const std::unordered_map<geometry::AttributeSemantic, QString> SHADER_DEFINE_STRINGS = {
	{ geometry::AttributeSemantic::ID, "#define USE_ID" },
	{ geometry::AttributeSemantic::Color, "#define USE_COLOR" },
	{ geometry::AttributeSemantic::Position, "#define USE_POSITION" },
	{ geometry::AttributeSemantic::Normal, "#define USE_NORMAL" },
	{ geometry::AttributeSemantic::SegmentID, "#define USE_SEGMENT" },
	{ geometry::AttributeSemantic::Custom0, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom1, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom2, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom3, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom4, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom5, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom6, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom7, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom8, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom9, "#define USE_CUSTOM" },
	{ geometry::AttributeSemantic::Custom10, "#define USE_CUSTOM" }
};

AttributeBasedColorizer::AttributeBasedColorizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* pointcloud_provider, navigation::Camera* camera) noexcept
	: AbstractColorizer(opengl_context)
	, m_pointcloud_provider(pointcloud_provider)
	, m_camera(camera)
{
	auto available_attributes = retrieveAvailableAttributes(pointcloud_provider);
	auto color_iterator = std::find(available_attributes.begin(), available_attributes.end(), "color");
	int initial_index = (color_iterator == available_attributes.end()) ? 0 : color_iterator - available_attributes.begin();

	m_attribute_selection_parameter = std::make_unique<EnumParameter>("Attribute", available_attributes, initial_index);
	connect(m_attribute_selection_parameter.get(), &EnumParameter::valueChanged, this, &AttributeBasedColorizer::onAttributeSelectionChanged);
	m_parameters.push_back(m_attribute_selection_parameter.get());

	m_color_scale_parameter = std::make_unique<EnumParameter>("Color Scale", std::initializer_list<std::string>({ "Sequential", "Qualitative" }), 0u);
	connect(m_color_scale_parameter.get(), &EnumParameter::valueChanged, this, &AttributeBasedColorizer::onColorScaleChanged);
	m_parameters.push_back(m_color_scale_parameter.get());

	m_original_color_factor_parameter = std::make_unique<RangeParameter<float>>("Original Color", 0.0, 0.0, 1.0, 0.001);
	connect(m_original_color_factor_parameter.get(), &RangeParameter<float>::valueChanged, this, &AttributeBasedColorizer::onOriginalColorFactorChanged);
	m_parameters.push_back(m_original_color_factor_parameter.get());

	connect(m_camera, &navigation::Camera::farPlaneChanged, this, &AttributeBasedColorizer::onCameraFarPlaneChanged);
	connect(m_camera, &navigation::Camera::nearPlaneChanged, this, &AttributeBasedColorizer::onCameraNearPlaneChanged);

	reloadShaderSpecificationsFromDisk();
}

AttributeBasedColorizer::~AttributeBasedColorizer()
{
}

void AttributeBasedColorizer::reloadShaderSpecificationsFromDisk()
{
	m_raw_shader_code = utils::ShaderStringsFactory::readShaderFile("./data/shaders/AttributeBasedColorizer.glsl");
	refreshFinalShaderCode();
}

void AttributeBasedColorizer::setCompiledShaderProgram(QOpenGLShaderProgram* shader_program)
{
	m_shader_program = shader_program;

	m_opengl_context->makeCurrent();
	m_shader_program->bind();

	const auto attribute_selection = m_attribute_selection_parameter->selectedEntry();
	if (attribute_selection == "position")
	{
		m_shader_program->setUniformValue("u_bbox_min", m_pointcloud_provider->boundingBox().minimum());
		m_shader_program->setUniformValue("u_bbox_max", m_pointcloud_provider->boundingBox().maximum());
	}
	else if (attribute_selection == "depth")
	{
		m_shader_program->setUniformValue("u_far_plane", m_camera->cameraSpecifications().far_plane);
		m_shader_program->setUniformValue("u_near_plane", m_camera->cameraSpecifications().near_plane);
	}
	else if (attribute_selection == "id")
	{
		m_shader_program->setUniformValue("u_number_of_points", static_cast<uint>(m_pointcloud_provider->numberOfPoints()));
	}
	else
	{
		const auto attribute_semantic = m_pointcloud_provider->attributeMetadata(attribute_selection)->semantic;
		if (attributeIsCustom(attribute_semantic))
		{
			const auto attribute_type = m_pointcloud_provider->attributeMetadata(attribute_semantic)->type;
			switch (attribute_type)
			{
				case geometry::AttributeType::Float:
				{
					const auto attribute = m_pointcloud_provider->typedAttributeMetadata<float>(attribute_semantic);
					m_shader_program->setUniformValue("u_custom_value_min", attribute->minimum);
					m_shader_program->setUniformValue("u_custom_value_max", attribute->maximum);
					break;
				}
				case geometry::AttributeType::Int:
				{
					const auto attribute = m_pointcloud_provider->typedAttributeMetadata<int>(attribute_semantic);
					m_shader_program->setUniformValue("u_custom_value_min", attribute->minimum);
					m_shader_program->setUniformValue("u_custom_value_max", attribute->maximum);
					break;
				}
				case geometry::AttributeType::Uint:
				{
					// QOpenGLShaderProgram does not support passing of unsigned int uniform values (they are internally casted to int). So we have to do it via raw OpenGL here.
					const auto attribute = m_pointcloud_provider->typedAttributeMetadata<uint>(attribute_semantic);
					m_opengl_context->gl()->glUniform1ui(m_shader_program->uniformLocation("u_custom_value_min"), attribute->minimum);
					m_opengl_context->gl()->glUniform1ui(m_shader_program->uniformLocation("u_custom_value_max"), attribute->maximum);
					break;
				}
				case geometry::AttributeType::Vector3D:
				{
					const auto attribute = m_pointcloud_provider->typedAttributeMetadata<QVector3D>(attribute_semantic);
					m_shader_program->setUniformValue("u_custom_value_min", attribute->minimum);
					m_shader_program->setUniformValue("u_custom_value_max", attribute->maximum);
					break;
				}
				default:
					break;
			}
		}
	}

	m_shader_program->setUniformValue("u_use_linear_color_scale", m_color_scale_parameter->value() == 0u);
	m_shader_program->setUniformValue("u_original_color_factor", m_original_color_factor_parameter->value());
	m_shader_program->release();
	m_opengl_context->doneCurrent();
}

ColorizerType AttributeBasedColorizer::type() const noexcept
{
	return ColorizerType::AttributeBased;
}

std::set<geometry::AttributeSpecification> AttributeBasedColorizer::necessaryAttributes()
{
	return {};
}

void AttributeBasedColorizer::onAttributeSelectionChanged()
{
	refreshFinalShaderCode();

	emit AbstractColorizer::shaderRequiresRecompile();
}

void AttributeBasedColorizer::refreshFinalShaderCode()
{
	m_final_shader_code = m_raw_shader_code;

	// if color is available, we always use it for mixing it with the color derived from the selected attribute
	if (m_pointcloud_provider->hasAttribute(geometry::AttributeSemantic::Color))
	{
		m_final_shader_code.insert(0, "#define USE_COLOR\n");
	}

	const auto attribute_selection = m_attribute_selection_parameter->selectedEntry();

	if (attribute_selection == "depth")
	{
		m_final_shader_code.insert(0, "#define USE_DEPTH\n");
	}
	else if (attribute_selection != "color")
	{
		const auto attribute_metadata = m_pointcloud_provider->attributeMetadata(attribute_selection);
		QString shader_define = SHADER_DEFINE_STRINGS.at(attribute_metadata->semantic);
		if (attributeIsCustom(attribute_metadata->semantic))
		{
			shader_define += utils::ShaderStringsFactory::ATTRIBUTE_TYPE_SUFFIXES.at(attribute_metadata->type);
		}
		shader_define += "\n";

		m_final_shader_code.insert(0, shader_define);
	}
}

std::set<geometry::AttributeSpecification> AttributeBasedColorizer::requestedAttributes() const
{
	auto requested_attributes = ColorizerSpecifications::Specifications.at(type()).necessaryAttributes();

	// if color is available, we always use it for mixing it with the color derived from the selected attribute
	if (m_pointcloud_provider->hasAttribute(geometry::AttributeSemantic::Color))
	{
		requested_attributes.insert({ geometry::AttributeType::Color, geometry::AttributeSemantic::Color });
	}

	const auto attribute_selection = m_attribute_selection_parameter->selectedEntry();
	if (attribute_selection != "depth" && attribute_selection != "color")
	{
		const auto attribute_metadata = m_pointcloud_provider->attributeMetadata(attribute_selection);
		requested_attributes.insert({ attribute_metadata->type, attribute_metadata->semantic });
	}

	return requested_attributes;
}

void AttributeBasedColorizer::onColorScaleChanged()
{
	utils::setUniformValue(m_opengl_context, *m_shader_program, "u_use_linear_color_scale", m_color_scale_parameter->value() == 0u);
}

void AttributeBasedColorizer::onOriginalColorFactorChanged()
{
	utils::setUniformValue(m_opengl_context, *m_shader_program, "u_original_color_factor", m_original_color_factor_parameter->value());
}

void AttributeBasedColorizer::onCameraFarPlaneChanged(float far_plane)
{
	if (m_attribute_selection_parameter->selectedEntry() == "depth")
	{
		utils::setUniformValue(m_opengl_context, *m_shader_program, "u_far_plane", far_plane);
	}
}

void AttributeBasedColorizer::onCameraNearPlaneChanged(float near_plane)
{
	if (m_attribute_selection_parameter->selectedEntry() == "depth")
	{
		utils::setUniformValue(m_opengl_context, *m_shader_program, "u_near_plane", near_plane);
	}
}

}