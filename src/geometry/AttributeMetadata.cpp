#include "AttributeMetadata.h"

namespace sahara::geometry
{

AttributeMetadata::AttributeMetadata(const QString& name, AttributeSemantic semantic, AttributeType type)
	: name(name)
	, semantic(semantic)
	, type(type)
{
}

size_t AttributeMetadata::singleEntrySizeInBytes() const noexcept
{
	return 0;
}

// for custom attributes, we compare by name and type, for standard attributes we compare by semantic
bool AttributeMetadata::operator==(const AttributeMetadata& other) const
{
	if (attributeIsCustom(semantic) != attributeIsCustom(other.semantic))
	{
		return false;
	}

	if (!attributeIsCustom(semantic))
	{
		return semantic == other.semantic;
	}
	return name == other.name && type == other.type;
}

}
