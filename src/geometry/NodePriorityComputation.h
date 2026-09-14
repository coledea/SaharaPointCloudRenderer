#pragma once

#include "AbstractNodePriorityComputation.h"
#include "NodePriorityComputationHelper.h"

namespace sahara::geometry
{

class NodePriorityComputation : public AbstractNodePriorityComputation
{
public:
	NodePriorityComputation(rendering::OpenGLContext* context, const std::vector<NodeGPUData>& node_data, const std::vector<uint32_t>& level_starts);
	~NodePriorityComputation();

	void computePriorities(const navigation::Camera& camera) override;

private:
	std::vector<LevelDispatchData> m_level_bounds;
};

}