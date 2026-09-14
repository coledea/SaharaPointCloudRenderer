#include "SingleColorColorizer.h"

#include "ColorizerSpecifications.h"
#include "utils/ShaderUtilities.h"

#include <QColor>

namespace sahara::rendering
{

SingleColorColorizer::SingleColorColorizer(OpenGLContext* opengl_context, AbstractPointCloudProvider* provider) noexcept
	: AbstractColorizer(opengl_context)
	, m_pointcloud_provider(provider)
{
	m_original_color_factor_parameter = std::make_unique<RangeParameter<float>>("Original Color", 0.0, 0.0, 1.0, 0.001);
	connect(m_original_color_factor_parameter.get(), &RangeParameter<float>::valueChanged, this, &SingleColorColorizer::onOriginalColorFactorChanged);
	m_parameters.push_back(m_original_color_factor_parameter.get());

	m_color_parameter = std::make_unique<Parameter<QColor>>("Color", QColor::fromRgbF(0.5f, 0.5f, 0.5f, 1.0f));
	connect(m_color_parameter.get(), &Parameter<QColor>::valueChanged, this, &SingleColorColorizer::onColorChanged);
	m_parameters.push_back(m_color_parameter.get());

	reloadShaderSpecificationsFromDisk();
}

SingleColorColorizer::~SingleColorColorizer()
{
}

void SingleColorColorizer::reloadShaderSpecificationsFromDisk()
{
	m_final_shader_code = utils::ShaderStringsFactory::readShaderFile("./data/shaders/SingleColorColorizer.glsl");

	if (m_pointcloud_provider->hasAttribute(geometry::AttributeSemantic::Color))
	{
		m_final_shader_code.insert(0, "#define USE_COLOR\n");
	}
}

std::set<geometry::AttributeSpecification> SingleColorColorizer::requestedAttributes() const
{
	auto requested_attributes = ColorizerSpecifications::Specifications.at(type()).necessaryAttributes();
	if (m_pointcloud_provider->hasAttribute(geometry::AttributeSemantic::Color))
	{
		requested_attributes.insert({ geometry::AttributeType::Color, geometry::AttributeSemantic::Color });
	}
	return requested_attributes;
}

void SingleColorColorizer::setCompiledShaderProgram(QOpenGLShaderProgram* shader_program)
{
	m_shader_program = shader_program;

	// set uniform values of new shader program according to current parameter values
	onColorChanged();
	onOriginalColorFactorChanged();
}

ColorizerType SingleColorColorizer::type() const noexcept
{
	return ColorizerType::SingleColor;
}

void SingleColorColorizer::onColorChanged()
{
	utils::setUniformValue(m_opengl_context, *m_shader_program, "u_color", m_color_parameter->value());
}

void SingleColorColorizer::onOriginalColorFactorChanged()
{
	utils::setUniformValue(m_opengl_context, *m_shader_program, "u_original_color_factor", m_original_color_factor_parameter->value());
}

}