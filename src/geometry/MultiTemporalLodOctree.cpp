#include "MultiTemporalLodOctree.h"

#include "NodePriorityComputationMultiTemporal.h"
#include "SerializedOctreeNode.h"

#include <QFile>
#include <algorithm>
#include <execution>
#include <iostream>

namespace sahara::geometry
{

MultiTemporalLodOctree::MultiTemporalLodOctree(const std::filesystem::path& filepath, std::vector<int>& num_nodes, std::vector<BoundingBox>& bounding_boxes, rendering::OpenGLContext* context)
	: AbstractLodOctree(context)
	, m_selected_timestamp(0)
	, m_timestamp_range(0)
	, m_num_nodes_per_timestamp(num_nodes)
	, m_max_chunks_per_timestamp(0)
{
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Could not open file " << filepath.c_str();
		return;
	}

	m_timestamp_offsets.push_back(0);
	auto overall_num_nodes = std::accumulate(num_nodes.begin(), num_nodes.end(), 0);
	std::vector<SerializedOctreeNode> serialized_nodes(overall_num_nodes);
	file.read(reinterpret_cast<char*>(serialized_nodes.data()), overall_num_nodes * sizeof(SerializedOctreeNode));

	// We read and keep the metadata of all nodes. For 10M nodes (able to organize a maximum of 500 billion points+voxels) this roughly amounts to 1.3 GB of RAM
	// If necessary, we could signifcantly reduce the memory usage by omitting certain information (e.g., level or bbox) that is not required in main memory anymore after having been transferred to VRAM.
	// We could also replace the pointers to children with indices, to save memory on the cost of an additional indirection.
	m_nodes.reserve(serialized_nodes.size());

	std::vector<NodeGPUData> node_gpu_data;
	node_gpu_data.reserve(serialized_nodes.size());
	std::vector<std::vector<uint32_t>> level_starts;

	for (int timestamp = 0; timestamp < num_nodes.size(); timestamp++)
	{
		level_starts.push_back({ m_timestamp_offsets.back() });

		m_octrees.emplace_back(std::make_unique<LodOctreeNode>());
		m_octrees[timestamp]->m_bounding_box = bounding_boxes[timestamp];
		m_octrees[timestamp]->m_index = m_timestamp_offsets.back();
		m_octrees[timestamp]->m_data_index = serialized_nodes[m_timestamp_offsets.back()].data_index;
		m_octrees[timestamp]->m_number_of_points = serialized_nodes[m_timestamp_offsets.back()].number_of_points;
		m_octrees[timestamp]->m_priority = -1;
		m_octrees[timestamp]->m_level = 0;
		m_octrees[timestamp]->m_parent = nullptr;
		m_octrees[timestamp]->m_attribute_data = nullptr;
		m_octrees[timestamp]->m_is_loading = false;
		deserializeOctree(m_octrees[timestamp].get(), serialized_nodes, node_gpu_data, level_starts.back());

		m_timestamp_offsets.push_back(m_timestamp_offsets.back() + num_nodes[timestamp]);
		m_max_chunks_per_timestamp = std::max(m_max_chunks_per_timestamp, num_nodes[timestamp]);
	}
	file.close();

	m_considered_nodes.reserve((2 * m_timestamp_range + 1) * m_max_chunks_per_timestamp);
	rebuildConsideredNodes();

	m_priority_computation = std::make_unique<geometry::NodePriorityComputationMultiTemporal>(m_opengl_context, node_gpu_data, level_starts);
}

uint MultiTemporalLodOctree::numberOfTimestamps() const noexcept
{
	return m_octrees.size();
}

const BoundingBox& MultiTemporalLodOctree::boundingBox() const
{
	return m_octrees.front()->m_bounding_box;
}

const std::vector<LodOctreeNode*>& MultiTemporalLodOctree::nodes() const
{
	return m_considered_nodes;
}

const std::vector<LodOctreeNode*>& MultiTemporalLodOctree::allNodes() const
{
	return m_nodes;
}

uint MultiTemporalLodOctree::overallNumberOfNodes() const noexcept
{
	return m_nodes.size();
}

void MultiTemporalLodOctree::setTimestamp(uint timestamp) noexcept
{
	auto current_first = std::max(0, static_cast<int>(m_selected_timestamp) - m_timestamp_range);
	auto new_first = std::max(0, static_cast<int>(timestamp) - m_timestamp_range);
	auto current_last = std::min(m_selected_timestamp + m_timestamp_range + 1, numberOfTimestamps());
	auto new_last = std::min(timestamp + m_timestamp_range + 1, numberOfTimestamps());

	m_selected_timestamp = timestamp;
	dynamic_cast<geometry::NodePriorityComputationMultiTemporal*>(m_priority_computation.get())->setTimestamp(timestamp);

	if (current_first == new_first && current_last == new_last)
	{
		return; // no change in considered nodes, no need to rebuild the vector
	}

	rebuildConsideredNodes();
}

void MultiTemporalLodOctree::setTimestampDistanceToConsider(int range)
{
	if (range == m_timestamp_range)
	{
		return;
	}

	m_timestamp_range = range;

	m_considered_nodes.reserve((2 * m_timestamp_range + 1) * m_max_chunks_per_timestamp);
	rebuildConsideredNodes();
	dynamic_cast<geometry::NodePriorityComputationMultiTemporal*>(m_priority_computation.get())->setTimestampDistanceToConsider(range);
}

void MultiTemporalLodOctree::setTimestampPriorityFalloff(float falloff)
{
	dynamic_cast<geometry::NodePriorityComputationMultiTemporal*>(m_priority_computation.get())->setTimestampPriorityFalloff(falloff);
}

// The priority hierarchy is 1xx for nodes that get rendered, 0xx for nodes that won't get rendered but might be important soon, and 0 for nodes that are not important.
// We use a priority function that is a combination of the screen-space size of the AABB of a node and the relevancy of parent/child nodes.
void MultiTemporalLodOctree::updatePrioritiesAndSort(const navigation::Camera& camera)
{
	const auto priorities = m_priority_computation->priorities();
	std::for_each(std::execution::par_unseq, m_considered_nodes.begin(), m_considered_nodes.end(), [priorities, this](LodOctreeNode* node) {
		updatePriorityForNode(node, priorities[node->m_index], 1.0f);
	});

	std::stable_sort(m_considered_nodes.begin(), m_considered_nodes.end(), [](const LodOctreeNode* a, const LodOctreeNode* b) { return a->m_priority > b->m_priority; });
}

int MultiTemporalLodOctree::timestampOfNode(const LodOctreeNode* node) const noexcept
{
	for (int i = 0; i < m_timestamp_offsets.size(); i++)
	{
		if (i == m_timestamp_offsets.size() - 1)
		{
			return i;
		}

		if (node->m_index < m_timestamp_offsets[i + 1])
		{
			return i;
		}
	}
}

int MultiTemporalLodOctree::numNodesInTimestamp(uint timestamp) const noexcept
{
	return m_num_nodes_per_timestamp[timestamp];
}

void MultiTemporalLodOctree::rebuildConsideredNodes()
{
	auto first = m_nodes.begin() + m_timestamp_offsets[std::max(0, static_cast<int>(m_selected_timestamp) - m_timestamp_range)];
	auto last = m_nodes.begin() + m_timestamp_offsets[std::min(m_selected_timestamp + m_timestamp_range + 1, numberOfTimestamps())];
	m_considered_nodes.assign(first, last);
}

}