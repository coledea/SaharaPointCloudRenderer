#pragma once

#include <QString>
#include <unordered_map>
#include <vector>

namespace sahara::rendering
{

struct DrawArraysIndirectCommand
{
	unsigned int number_of_points;
	unsigned int number_of_instances; // for points this is equal to number_of_points. If this is set to 0, the draw command is ignored.
	unsigned int first_point;		  // vertex buffer index offset
	unsigned int node_index;		  // actually, this is the base_instance. However, as we don't use instancing, we can use it freely. In the OOC case, we store the node index.
};

}