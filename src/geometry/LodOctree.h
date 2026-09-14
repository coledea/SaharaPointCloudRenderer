#pragma once

#include "AbstractLodOctree.h"
#include "NodePriorityComputation.h"

#include <filesystem>

namespace sahara::geometry
{

class LodOctree : public AbstractLodOctree
{
public:
	LodOctree(const std::filesystem::path& filepath, int num_nodes, const BoundingBox& bbox, rendering::OpenGLContext* context);

	const BoundingBox& boundingBox() const override;
	void configureHierarchyPriority(bool use_hierarchy, float factor) noexcept;

private:
	std::unique_ptr<LodOctreeNode> m_root;
	bool m_use_hierarchy_priority;
	float m_hierarchy_priority_factor;

	void updatePrioritiesAndSort(const navigation::Camera& camera) override;
};

}