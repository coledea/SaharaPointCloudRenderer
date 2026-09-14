#pragma once

#include <optional>

namespace sahara::geometry
{

enum class AttributeSemantic
{
	Position,
	Color,
	Normal,
	ID,
	SegmentID,
	Custom0,
	Custom1,
	Custom2,
	Custom3,
	Custom4,
	Custom5,
	Custom6,
	Custom7,
	Custom8,
	Custom9,
	Custom10,
	Undefined
};

inline constexpr bool attributeIsCustom(geometry::AttributeSemantic semantic)
{
	return (semantic != geometry::AttributeSemantic::ID)
		   && (semantic != geometry::AttributeSemantic::Position)
		   && (semantic != geometry::AttributeSemantic::Color)
		   && (semantic != geometry::AttributeSemantic::Normal)
		   && (semantic != geometry::AttributeSemantic::SegmentID);
}

inline std::optional<geometry::AttributeSemantic> nextCustomSemantic(geometry::AttributeSemantic current)
{
	auto next = static_cast<geometry::AttributeSemantic>(static_cast<int>(current) + 1);
	if (current >= geometry::AttributeSemantic::Custom10)
	{
		return std::nullopt;
	}
	return next;
}

}