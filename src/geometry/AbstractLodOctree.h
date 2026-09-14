#pragma once

#include "AbstractNodePriorityComputation.h"
#include "LodOctreeNode.h"
#include "NodePriorityComputationHelper.h"
#include "navigation/Camera.h"
#include "rendering/OpenGLContext.h"

namespace sahara::geometry
{

struct SerializedOctreeNode;

class AbstractLodOctree
{
public:
	AbstractLodOctree(rendering::OpenGLContext* context);
	virtual ~AbstractLodOctree() {};

	virtual const BoundingBox& boundingBox() const = 0;
	virtual const std::vector<LodOctreeNode*>& nodes() const;
	GLuint nodeMetadataBuffer() const noexcept;
	void computePrioritiesAndSort(const navigation::Camera& camera);

	void setProjectionSizeRenderThreshold(int threshold);

protected:
	rendering::OpenGLContext* m_opengl_context;
	std::vector<LodOctreeNode*> m_nodes;
	std::unique_ptr<geometry::AbstractNodePriorityComputation> m_priority_computation;

	void deserializeOctree(LodOctreeNode* root, const std::vector<SerializedOctreeNode>& serialized_nodes, std::vector<NodeGPUData>& node_gpu_data, std::vector<uint32_t>& level_starts);
	void updatePriorityForNode(LodOctreeNode* node, float priority, float parent_child_factor);

	virtual void updatePrioritiesAndSort(const navigation::Camera& camera) = 0;
};

}