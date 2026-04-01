#pragma once

#include "AbstractNodePriorityComputation.h"
#include "NodePriorityComputationHelper.h"

namespace sahara::geometry
{

class NodePriorityComputationMultiTemporal : public AbstractNodePriorityComputation
{
public:
	NodePriorityComputationMultiTemporal(rendering::OpenGLContext* context, const std::vector<NodeGPUData>& node_data, const std::vector<std::vector<uint32_t>>& level_starts);
	~NodePriorityComputationMultiTemporal();

	void computePriorities(const navigation::Camera& camera) override;

	void setTimestamp(uint timestamp);
	void setTimestampDistanceToConsider(uint distance);
	void setTimestampPriorityFalloff(float falloff);

private:
	uint m_number_of_timestamps;
	uint m_selected_timestamp;

	std::vector<std::vector<LevelDispatchData>> m_level_bounds;
	std::vector<float> m_temporal_priorities;
	uint m_max_levels;

	uint m_timestamp_distance_to_consider;
	float m_timestamp_priority_falloff;

	void computeTemporalPriorities();
};

}