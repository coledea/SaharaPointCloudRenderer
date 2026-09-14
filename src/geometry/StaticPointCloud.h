#pragma once

#include "AttributeData.h"
#include "AttributeMetadata.h"
#include "BoundingBox.h"
#include "rendering/OpenGLContext.h"

#include <QOpenGLBuffer>
#include <unordered_map>
#include <variant>

namespace sahara::geometry
{

struct StaticPointCloudAttribute
{
	std::unique_ptr<AbstractAttributeData> data;
	std::unique_ptr<AttributeMetadata> metadata;
};

class StaticPointCloud
{
public:
	StaticPointCloud();

	StaticPointCloud(const StaticPointCloud&) = delete;
	StaticPointCloud& operator=(const StaticPointCloud&) = delete;

	size_t numberOfPoints() const;
	const BoundingBox& boundingBox() const noexcept;

	// attributes can either be queried by their semantic, or by a reference metadata object (which is useful for custom attributes with no fixed semantic)
	bool hasAttribute(AttributeSemantic semantic) const;
	bool hasAttribute(const AttributeMetadata& metadata) const;
	const AttributeMetadata* attributeMetadata(AttributeSemantic semantic) const;
	const AttributeMetadata* attributeMetadata(const AttributeMetadata& reference_metadata) const;
	const AbstractAttributeData* attributeData(AttributeSemantic semantic) const;
	const AbstractAttributeData* attributeData(const AttributeMetadata& reference_metadata) const;

	const std::unordered_map<AttributeSemantic, StaticPointCloudAttribute>& attributes() const;

	// Note: overrides any previous data for an attribute with the same semantic
	void addAttribute(std::unique_ptr<AbstractAttributeData>&& attribute_data, std::unique_ptr<AttributeMetadata>&& metadata);

private:
	std::unordered_map<AttributeSemantic, StaticPointCloudAttribute> m_attributes;
	BoundingBox m_bounding_box;
};

}