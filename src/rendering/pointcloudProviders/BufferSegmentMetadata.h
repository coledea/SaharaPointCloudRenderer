#pragma once

#include <memory>
#include <set>

namespace sahara::rendering
{

struct BufferSegmentMetadata
{
	unsigned int offset;
	unsigned int size;

	bool operator==(const BufferSegmentMetadata& other) const;
	bool operator<(const BufferSegmentMetadata& other) const;

	unsigned int end() const;
};

struct BufferOffsetComparator // comparison of buffer segments according to their offset
{
	bool operator()(const BufferSegmentMetadata& l, const BufferSegmentMetadata& r) const;
};

typedef std::set<BufferSegmentMetadata, BufferOffsetComparator>::iterator BufferSegmentsIterator;

// comparison of buffer segments according to their size
// This comparator allows for heterogeneous comparison (i.e., comparing iterators with actual segments)
// We need this to be able to use lower_bound() with a dummy segment to find a minimal segment for an allocation.
// We also consider the offset in case of equal sizes to ensure unique elements (otherwise multiset::erase(key) would erase all elements with the same size)
struct BufferIteratorSizeComparator
{
	using is_transparent = void;

	bool operator()(const BufferSegmentsIterator& l, const BufferSegmentsIterator& r) const;
	bool operator()(const BufferSegmentMetadata& l, const BufferSegmentsIterator& r) const;
	bool operator()(const BufferSegmentsIterator& l, const BufferSegmentMetadata& r) const;
};

}

template <>
struct std::hash<sahara::rendering::BufferSegmentMetadata> // for being able to use BufferSegmentMetadata in hash-based data structures
{
	std::size_t operator()(const sahara::rendering::BufferSegmentMetadata& s) const noexcept;
};
