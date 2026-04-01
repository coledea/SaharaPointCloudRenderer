#include "AbstractPointCloudProvider.h"

namespace sahara::rendering
{

uint AbstractPointCloudProvider::m_next_id = 0;

AbstractPointCloudProvider::AbstractPointCloudProvider(const QString& name, rendering::OpenGLContext* opengl_context, size_t number_of_draw_commands) noexcept
	: m_id(m_next_id++)
	, m_name(name)
	, m_is_valid(false)
	, m_opengl_context(opengl_context)
	, m_draw_command_buffer(opengl_context, number_of_draw_commands)
{
}

uint AbstractPointCloudProvider::id() const noexcept
{
	return m_id;
}

const QString& AbstractPointCloudProvider::name() const noexcept
{
	return m_name;
}

bool AbstractPointCloudProvider::isValid() const noexcept
{
	return m_is_valid;
}

std::vector<AbstractParameter*>& AbstractPointCloudProvider::parameters() noexcept
{
	return m_parameters;
}

DrawCommandBuffer& AbstractPointCloudProvider::drawCommandBuffer() noexcept
{
	return m_draw_command_buffer;
}

bool AbstractPointCloudProvider::hasAttribute(geometry::AttributeSemantic semantic) const
{
	return m_available_attributes.find(semantic) != m_available_attributes.end();
}

const std::unordered_map<geometry::AttributeSemantic, geometry::AttributeMetadata*>& AbstractPointCloudProvider::attributesMetadata() const
{
	return m_available_attributes;
}

const geometry::AttributeMetadata* AbstractPointCloudProvider::attributeMetadata(const QString& attribute_name) const
{
	for (const auto& attribute : m_available_attributes)
	{
		if (attribute.second->name == attribute_name)
		{
			return attribute.second;
		}
	}

	throw std::invalid_argument("No attribute with name " + attribute_name.toStdString() + " exists");
}

const geometry::AttributeMetadata* AbstractPointCloudProvider::attributeMetadata(geometry::AttributeSemantic semantic) const
{
	assert(m_available_attributes.find(semantic) != m_available_attributes.end());
	return m_available_attributes.at(semantic);
}

void AbstractPointCloudProvider::setRequiredAttributes(const std::set<geometry::AttributeSpecification>& attributes)
{
	m_required_attributes.clear();
	for (const auto& attribute : attributes)
	{
		m_required_attributes.push_back(attributeMetadata(attribute.semantic));
	}
}

}