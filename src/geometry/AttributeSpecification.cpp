#include "AttributeSpecification.h"

namespace sahara::geometry
{

bool AttributeSpecification::operator==(const AttributeSpecification& other) const
{
	return other.semantic == semantic && other.type == type;
}

// this order has no inherent meaning but is required for certain STL containers
bool AttributeSpecification::operator<(const AttributeSpecification& other) const
{
	return (semantic == other.semantic) ? static_cast<int>(type) < static_cast<int>(other.type) : static_cast<int>(semantic) < static_cast<int>(other.semantic);
}

bool attributeSpecificationsEqual(const std::set<AttributeSpecification>& old_set, const std::set<AttributeSpecification>& new_set)
{
	for (const auto& element : old_set)
	{
		if (std::find(new_set.begin(), new_set.end(), element) == new_set.end())
		{
			return false;
		}
	}
	for (const auto& element : new_set)
	{
		if (std::find(old_set.begin(), old_set.end(), element) == old_set.end())
		{
			return false;
		}
	}
	return true;
}

}