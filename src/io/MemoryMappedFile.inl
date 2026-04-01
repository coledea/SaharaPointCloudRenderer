#include "MemoryMappedFile.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace sahara::io
{
template <typename T>
inline void MemoryMappedFile::read(size_t offset, size_t count, T* buffer) const
{
	assert(offset + count <= m_size / sizeof(T));
	std::copy_n(static_cast<T*>(m_data) + offset, count, buffer);
}

}
