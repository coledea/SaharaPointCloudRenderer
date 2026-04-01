#pragma once

#include "AbstractAttributeData.h"
#include "AttributeSemantics.h"
#include "BoundingBox.h"

#include <array>
#include <filesystem>

namespace sahara::geometry
{

typedef std::unordered_map<AttributeSemantic, std::unique_ptr<AbstractAttributeData>> NodeAttributeData;

struct LodOctreeNode
{
	uint32_t m_index;
	uint32_t m_number_of_points;
	uint64_t m_data_index;
	BoundingBox m_bounding_box;

	std::array<std::unique_ptr<LodOctreeNode>, 8> m_children;

	float m_priority;
	uint m_level;
	LodOctreeNode* m_parent;
	bool m_is_leaf;
	bool m_is_loading;

	std::unique_ptr<NodeAttributeData> m_attribute_data;
};

struct priorityGreater
{
	bool operator()(const LodOctreeNode* l, const LodOctreeNode* r) const
	{
		return l->m_priority > r->m_priority;
	};
};

}