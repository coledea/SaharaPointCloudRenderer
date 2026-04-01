#pragma once

#include <cstdint>

namespace sahara::geometry
{

#if defined(_MSC_VER)
__pragma(pack(push, 1)) struct SerializedOctreeNode
{
	uint64_t data_index;
	uint32_t number_of_points;
	uint8_t child_mask;
};
__pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
struct __attribute__((packed)) SerializedOctreeNode
{
	uint64_t data_index;
	uint32_t number_of_points;
	uint8_t child_mask;
};
#endif

}