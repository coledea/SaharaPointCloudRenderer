#include "AbstractRasterizer.h"

#include "RasterizerSpecifications.h"
#include "rendering/colorization/ColorizerSpecifications.h"

namespace sahara::rendering
{

AbstractRasterizer::AbstractRasterizer(rendering::OpenGLContext* opengl_context, AbstractPointCloudProvider* provider, navigation::Camera* camera) noexcept
	: m_opengl_context(opengl_context)
	, m_pointcloud_provider(provider)
	, m_camera(camera)
	, m_colorizer(nullptr)
{
}

AbstractColorizer* AbstractRasterizer::setColorizerType(ColorizerType colorizer_type)
{
	if (m_colorizer == nullptr || m_colorizer->type() != colorizer_type)
	{
		m_colorizer = ColorizerSpecifications::Specifications.at(colorizer_type).createColorizer(m_opengl_context, m_pointcloud_provider, m_camera);
	}

	return m_colorizer.get();
}

std::vector<AbstractParameter*>& AbstractRasterizer::parameters() noexcept
{
	return m_parameters;
}

std::set<geometry::AttributeSpecification> AbstractRasterizer::generatableAttributes() noexcept
{
	return {};
}

std::set<geometry::AttributeSpecification> AbstractRasterizer::availableAttributes()
{
	auto attributes = std::set<geometry::AttributeSpecification>();
	for (const auto& attribute : generatableAttributes())
	{
		attributes.insert(attribute);
	}
	for (const auto& attributes_metadata_pair : m_pointcloud_provider->attributesMetadata())
	{
		attributes.emplace(attributes_metadata_pair.second->type, attributes_metadata_pair.first);
	}

	return attributes;
}

std::set<geometry::AttributeSpecification> AbstractRasterizer::requiredAttributes(const std::set<geometry::AttributeSpecification>& requested_outputs) const
{
	auto inputs = std::set<geometry::AttributeSpecification>();
	for (const auto& attribute : requested_outputs)
	{
		inputs.insert(attribute);
	}
	for (const auto& attribute : RasterizerSpecifications::Specifications.at(type()).necessaryAttributes())
	{
		inputs.insert(attribute);
	}
	for (const auto& attribute : m_colorizer->requestedAttributes())
	{
		inputs.insert(attribute);
	}

	return inputs;
}

}
