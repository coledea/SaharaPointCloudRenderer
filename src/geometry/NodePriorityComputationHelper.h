#pragma once

#include "rendering/OpenGLContext.h"

#include <QVector3D>

namespace sahara::geometry
{

// Helper struct for the GPU-side buffer that holds per-node metadata
struct NodeGPUData
{
	QVector3D bbox_min;
	float is_leaf;
	QVector3D bbox_max;
	uint parent_index_and_render_flag; // the highest order bit is used for marking whether the path this node is on already contains a rendered node

	NodeGPUData(const QVector3D& min, float leaf, const QVector3D& max, uint parent)
		: bbox_min(min)
		, is_leaf(leaf)
		, bbox_max(max)
		, parent_index_and_render_flag(parent)
	{
	}
};

// Helper struct for holding the dispatch data for each level of an LOD octree
struct LevelDispatchData
{
	uint start;
	uint end;
	GLuint workgroup_size;
};

}