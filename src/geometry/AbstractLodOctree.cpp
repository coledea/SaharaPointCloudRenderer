#include "AbstractLodOctree.h"

#include "SerializedOctreeNode.h"
#include "utils/Profiler.h"

#include <queue>

namespace sahara::geometry
{

AbstractLodOctree::AbstractLodOctree(rendering::OpenGLContext* context)
	: m_opengl_context(context)
{
	static_assert(sizeof(SerializedOctreeNode) == 13, "SerializedOctreeNode must be 13 bytes in size");
}

const std::vector<LodOctreeNode*>& AbstractLodOctree::nodes() const
{
	return m_nodes;
}

GLuint AbstractLodOctree::nodeMetadataBuffer() const noexcept
{
	return m_priority_computation->nodeMetadataBuffer();
}

void AbstractLodOctree::setProjectionSizeRenderThreshold(int threshold)
{
	m_priority_computation->setProjectionSizeRenderThreshold(threshold);
}

void AbstractLodOctree::computePrioritiesAndSort(const navigation::Camera& camera)
{
	utils::global_profiler.startTimer("Compute Priorities GPU");
	m_priority_computation->computePriorities(camera);
	utils::global_profiler.stopTimer("Compute Priorities GPU");

	utils::global_profiler.startTimer("Update Priorities CPU and Sort");
	updatePrioritiesAndSort(camera);
	utils::global_profiler.stopTimer("Update Priorities CPU and Sort");
}

void AbstractLodOctree::deserializeOctree(LodOctreeNode* root, const std::vector<SerializedOctreeNode>& serialized_nodes, std::vector<NodeGPUData>& node_gpu_data, std::vector<uint32_t>& level_starts)
{

	// We deserialize breadth first as this is the order of the nodes
	std::queue<LodOctreeNode*> nodes_to_process;
	nodes_to_process.push(root);

	uint current_node_index = root->m_index;
	while (!nodes_to_process.empty())
	{
		LodOctreeNode* node = nodes_to_process.front();
		m_nodes.push_back(node);
		nodes_to_process.pop();

		// process children
		node->m_is_leaf = true;
		for (int i = 0; i < 8; i++)
		{
			bool child_exists = (serialized_nodes[node->m_index].child_mask >> i) & 1;
			if (child_exists && serialized_nodes[++current_node_index].number_of_points > 0)
			{
				node->m_is_leaf = false;
				node->m_children[i] = std::make_unique<LodOctreeNode>();
				node->m_children[i]->m_index = current_node_index;
				node->m_children[i]->m_data_index = serialized_nodes[current_node_index].data_index;
				node->m_children[i]->m_number_of_points = serialized_nodes[current_node_index].number_of_points;
				node->m_children[i]->m_priority = -1;
				node->m_children[i]->m_level = node->m_level + 1;
				node->m_children[i]->m_parent = node;
				node->m_children[i]->m_attribute_data = nullptr;
				node->m_children[i]->m_is_loading = false;

				const auto half = node->m_bounding_box.extent() / 2.0;
				QVector3D translation = QVector3D((i & 0b100) >> 2, (i & 0b010) >> 1, (i & 0b001)) * half;
				node->m_children[i]->m_bounding_box.setBounds(node->m_bounding_box.minimum() + translation, node->m_bounding_box.minimum() + half + translation);

				nodes_to_process.push(node->m_children[i].get());
			}
		}

		// fill buffers for GPU priority computation
		node_gpu_data.emplace_back(
			node->m_bounding_box.minimum(),
			node->m_is_leaf ? 1.0f : 0.0f,
			node->m_bounding_box.maximum(),
			node->m_parent == nullptr ? 0x7FFFFFFFu : node->m_parent->m_index // set highest possible index for root
		);

		if (node->m_level >= level_starts.size())
		{
			level_starts.push_back(m_nodes.size() - 1);
		}
	}
}

void AbstractLodOctree::updatePriorityForNode(LodOctreeNode* node, float priority, float parent_child_factor)
{
	if (node->m_parent != nullptr && node->m_parent->m_priority >= 100.0f)
	{
		node->m_priority = priority + parent_child_factor;
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		if (node->m_children[i] != nullptr && node->m_children[i]->m_priority >= 100.0f)
		{
			node->m_priority = priority + parent_child_factor;
			return;
		}
	}
	node->m_priority = priority;
}

}
