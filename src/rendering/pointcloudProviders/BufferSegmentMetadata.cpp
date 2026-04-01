#include "BufferSegmentMetadata.h"

namespace sahara::rendering
{

bool BufferSegmentMetadata::operator==(const BufferSegmentMetadata& other) const
{
	return other.offset == offset;
}

bool BufferSegmentMetadata::operator<(const BufferSegmentMetadata& other) const
{
	return size < other.size;
}

unsigned int BufferSegmentMetadata::end() const
{
	return offset + size;
}

bool BufferOffsetComparator::operator()(const BufferSegmentMetadata& l, const BufferSegmentMetadata& r) const
{
	return l.offset < r.offset;
}

bool BufferIteratorSizeComparator::operator()(const BufferSegmentsIterator& l, const BufferSegmentsIterator& r) const
{
	return l->size == r->size ? l->offset < r->offset : l->size < r->size;
}

bool BufferIteratorSizeComparator::operator()(const BufferSegmentMetadata& l, const BufferSegmentsIterator& r) const
{
	return l.size == r->size ? l.offset < r->offset : l.size < r->size;
}

bool BufferIteratorSizeComparator::operator()(const BufferSegmentsIterator& l, const BufferSegmentMetadata& r) const
{
	return l->size == r.size ? l->offset < r.offset : l->size < r.size;
}

}

std::size_t std::hash<sahara::rendering::BufferSegmentMetadata>::operator()(const sahara::rendering::BufferSegmentMetadata& s) const noexcept
{
	std::size_t h1 = std::hash<unsigned int>{}(s.size);
	std::size_t h2 = std::hash<unsigned int>{}(s.offset);
	return h1 ^ (h2 << 1);
}