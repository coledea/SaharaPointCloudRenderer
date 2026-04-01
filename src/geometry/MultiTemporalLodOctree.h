#pragma once

#include "AbstractLodOctree.h"

#include <filesystem>

namespace sahara::geometry
{

class MultiTemporalLodOctree : public AbstractLodOctree
{
public:
	MultiTemporalLodOctree(const std::filesystem::path& filepath, std::vector<int>& num_nodes, std::vector<BoundingBox>& bounding_boxes, rendering::OpenGLContext* context);
	~MultiTemporalLodOctree() = default;

	uint numberOfTimestamps() const noexcept;
	const BoundingBox& boundingBox() const override;
	const std::vector<LodOctreeNode*>& nodes() const override;

	const std::vector<LodOctreeNode*>& allNodes() const;
	uint overallNumberOfNodes() const noexcept;

	void setTimestamp(uint timestamp) noexcept;
	void setTimestampDistanceToConsider(int range);
	void setTimestampPriorityFalloff(float falloff);

	int timestampOfNode(const LodOctreeNode* node) const noexcept;
	int numNodesInTimestamp(uint timestamp) const noexcept;

private:
	std::vector<std::unique_ptr<LodOctreeNode>> m_octrees;
	std::vector<LodOctreeNode*> m_considered_nodes;
	std::vector<int> m_num_nodes_per_timestamp;
	std::vector<uint32_t> m_timestamp_offsets;
	uint m_selected_timestamp;
	int m_timestamp_range;

	int m_max_chunks_per_timestamp;

	void rebuildConsideredNodes();
	void updatePrioritiesAndSort(const navigation::Camera& camera) override;
};

}