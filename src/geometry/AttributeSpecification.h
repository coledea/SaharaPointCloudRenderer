#pragma once

#include "AttributeSemantics.h"
#include "AttributeTypes.h"

#include <set>

namespace sahara::geometry
{

// this is a helper struct for passing around required attributes in STL containers (mainly used for shader construction)
// we did not use AttributeMetdata, as this also contains a name and possibly more elements (minimum, maximum) that we don't want to copy
struct AttributeSpecification
{
	AttributeType type;
	AttributeSemantic semantic;

	bool operator==(const AttributeSpecification& other) const;

	// this order has no inherent meaning but is required for certain STL containers
	bool operator<(const AttributeSpecification& other) const;
};

// utility function for comparing the equality of two sets of attribute specifications
bool attributeSpecificationsEqual(const std::set<AttributeSpecification>& old_set, const std::set<AttributeSpecification>& new_set);

}

template <>
struct std::hash<sahara::geometry::AttributeSpecification>
{
	std::size_t operator()(const sahara::geometry::AttributeSpecification& s) const noexcept
	{
		std::size_t h1 = std::hash<int>{}(static_cast<int>(s.semantic));
		std::size_t h2 = std::hash<int>{}(static_cast<int>(s.type));
		return h1 ^ (h2 << 1);
	}
};