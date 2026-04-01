#pragma once

#include "AbstractNodePriorityComputation.h"
#include "NodePriorityComputationHelper.h"

namespace sahara::geometry
{

enum class NodePriorityProjectionFunction
{
	None,
	FullProjection,
	ViewDiagonalProjection,
	BoundingSphereProjection
};

enum class NodePriorityDistanceFunction
{
	None,
	Center,
	NearestCorner
};

class NodePriorityComputation : public AbstractNodePriorityComputation
{
public:
	NodePriorityComputation(rendering::OpenGLContext* context, const std::vector<NodeGPUData>& node_data, const std::vector<uint32_t>& level_starts);
	~NodePriorityComputation();

	void computePriorities(const navigation::Camera& camera) override;

	void setPriorityProjectionFunction(NodePriorityProjectionFunction f);
	void setPriorityDistanceFunction(NodePriorityDistanceFunction f);
	void setPriorityProjectionFactor(float factor);
	void setPriorityDistanceFactor(float factor);
	void setPriorityCentralityFactor(float factor);
	void setUseRecency(bool use_recency);

private:
	GLuint m_recency_buffer; // only used for the recency-based priority term
	std::vector<LevelDispatchData> m_level_bounds;
};

}