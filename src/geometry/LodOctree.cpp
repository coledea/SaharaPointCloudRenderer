#include "LodOctree.h"

#include "SerializedOctreeNode.h"

#include <QFile>
#include <algorithm>
#include <bitset>
#include <execution>

namespace sahara::geometry
{

LodOctree::LodOctree(const std::filesystem::path& filepath, int num_nodes, const BoundingBox& bbox, rendering::OpenGLContext* context)
	: AbstractLodOctree(context)
	, m_use_hierarchy_priority(false)
	, m_hierarchy_priority_factor(1.0f)
{
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Could not open file " << filepath.c_str();
		return;
	}

	std::vector<SerializedOctreeNode> serialized_nodes(num_nodes);
	file.read(reinterpret_cast<char*>(serialized_nodes.data()), num_nodes * sizeof(SerializedOctreeNode));
	file.close();

	m_root = std::make_unique<LodOctreeNode>();
	m_root->m_bounding_box = bbox;
	m_root->m_index = 0;
	m_root->m_data_index = serialized_nodes[0].data_index;
	m_root->m_number_of_points = serialized_nodes[0].number_of_points;
	m_root->m_priority = -1;
	m_root->m_level = 0;
	m_root->m_parent = nullptr;
	m_root->m_attribute_data = nullptr;
	m_root->m_is_loading = false;

	m_nodes.reserve(serialized_nodes.size());

	std::vector<NodeGPUData> node_gpu_data;
	node_gpu_data.reserve(serialized_nodes.size());
	std::vector<uint32_t> level_starts;
	level_starts.push_back(0);

	deserializeOctree(m_root.get(), serialized_nodes, node_gpu_data, level_starts);
	m_priority_computation = std::make_unique<NodePriorityComputation>(m_opengl_context, node_gpu_data, level_starts);
}

const BoundingBox& LodOctree::boundingBox() const
{
	return m_root->m_bounding_box;
}

void LodOctree::configureHierarchyPriority(bool use_hierarchy, float factor) noexcept
{
	m_use_hierarchy_priority = use_hierarchy;
	m_hierarchy_priority_factor = factor;
}

void LodOctree::setPriorityProjectionFunction(NodePriorityProjectionFunction f)
{
	dynamic_cast<NodePriorityComputation*>(m_priority_computation.get())->setPriorityProjectionFunction(f);
}

void LodOctree::setPriorityDistanceFunction(NodePriorityDistanceFunction f)
{
	dynamic_cast<NodePriorityComputation*>(m_priority_computation.get())->setPriorityDistanceFunction(f);
}

void LodOctree::setPriorityProjectionFactor(float factor)
{
	dynamic_cast<NodePriorityComputation*>(m_priority_computation.get())->setPriorityProjectionFactor(factor);
}

void LodOctree::setPriorityDistanceFactor(float factor)
{
	dynamic_cast<NodePriorityComputation*>(m_priority_computation.get())->setPriorityDistanceFactor(factor);
}

void LodOctree::setPriorityCentralityFactor(float factor)
{
	dynamic_cast<NodePriorityComputation*>(m_priority_computation.get())->setPriorityCentralityFactor(factor);
}

void LodOctree::setPriorityUseRecency(bool use_recency)
{
	dynamic_cast<NodePriorityComputation*>(m_priority_computation.get())->setUseRecency(use_recency);
}

// The priority hierarchy is 1xx for nodes that get rendered, 0xx for nodes that won't get rendered but might be important soon, and 0 for nodes that are not important.
void LodOctree::updatePrioritiesAndSort(const navigation::Camera& camera)
{
	const auto priorities = m_priority_computation->priorities();
	if (m_use_hierarchy_priority)
	{
		std::for_each(std::execution::par_unseq, m_nodes.begin(), m_nodes.end(), [priorities, this](LodOctreeNode* node) {
			updatePriorityForNode(node, priorities[node->m_index], m_hierarchy_priority_factor);
		});
	}
	else
	{
		std::for_each(std::execution::par_unseq, m_nodes.begin(), m_nodes.end(), [priorities](LodOctreeNode* node) {
			node->m_priority = priorities[node->m_index];
		});
	}

	std::stable_sort(m_nodes.begin(), m_nodes.end(), [](const LodOctreeNode* a, const LodOctreeNode* b) { return a->m_priority > b->m_priority; });
}

}
