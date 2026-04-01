#include "StaticPointCloud.h"

namespace sahara::geometry
{

StaticPointCloud::StaticPointCloud()
{
}

void StaticPointCloud::addAttribute(std::unique_ptr<AbstractAttributeData>&& attribute_data, std::unique_ptr<AttributeMetadata>&& metadata)
{
	const auto semantic = metadata->semantic;
	m_attributes.emplace(semantic, StaticPointCloudAttribute{ std::move(attribute_data), std::move(metadata) });

	if (semantic == AttributeSemantic::Position)
	{
		assert(dynamic_cast<TypedAttributeMetadata<QVector3D>*>(m_attributes[semantic].metadata.get()));
		const auto position_metadata = dynamic_cast<TypedAttributeMetadata<QVector3D>*>(m_attributes[semantic].metadata.get());
		m_bounding_box.setBounds(position_metadata->minimum, position_metadata->maximum);
	}
}

size_t StaticPointCloud::numberOfPoints() const
{
	if (!hasAttribute(AttributeSemantic::Position))
	{
		return 0;
	}

	return m_attributes.at(AttributeSemantic::Position).data->size();
}

const BoundingBox& StaticPointCloud::boundingBox() const noexcept
{
	return m_bounding_box;
}

bool StaticPointCloud::hasAttribute(AttributeSemantic semantic) const
{
	return m_attributes.find(semantic) != m_attributes.end();
}

const AbstractAttributeData* StaticPointCloud::attributeData(AttributeSemantic semantic) const
{
	assert(hasAttribute(semantic) && "Requested attribute does not exist");
	return m_attributes.at(semantic).data.get();
}

const std::unordered_map<AttributeSemantic, StaticPointCloudAttribute>& StaticPointCloud::attributes() const
{
	return m_attributes;
}

}