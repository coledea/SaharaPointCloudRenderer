#include "UnbufferedFile.h"

#include <algorithm>
#include <cassert>
#include <cstring>

#ifdef _WIN32
#	define NOMINMAX
#	include <windows.h>
#else
#	include <unistd.h>
#endif

namespace sahara::io
{

template <typename T>
void UnbufferedFile::read(size_t offset, size_t count, T* buffer) const
{
	const size_t element_size = sizeof(T);

	const size_t byte_offset = offset * element_size;
	const size_t byte_count = count * element_size;

	assert(byte_offset + byte_count <= m_size);

	// Align file offset down to sector boundary
	const size_t aligned_offset = (byte_offset / m_sector_size) * m_sector_size;

	// Offset inside the aligned buffer where real data begins
	const size_t offset_delta = byte_offset - aligned_offset;

	// Total bytes we must read (round up to sector size)
	const size_t aligned_size =
		((offset_delta + byte_count + m_sector_size - 1) / m_sector_size) * m_sector_size;

	// Allocate aligned temporary buffer
	void* aligned_buffer = nullptr;

#ifdef _WIN32
	aligned_buffer = _aligned_malloc(aligned_size, m_sector_size);
#else
	posix_memalign(&aligned_buffer, m_sector_size, aligned_size);
#endif

	// Perform aligned read
#ifdef _WIN32
	LARGE_INTEGER li;
	li.QuadPart = static_cast<LONGLONG>(aligned_offset);

	SetFilePointerEx(m_file_handle, li, nullptr, FILE_BEGIN);

	DWORD bytes_read = 0;
	ReadFile(
		m_file_handle,
		aligned_buffer,
		static_cast<DWORD>(aligned_size),
		&bytes_read,
		nullptr);
#else
	ssize_t bytes_read = pread(
		m_file_descriptor,
		aligned_buffer,
		aligned_size,
		static_cast<off_t>(aligned_offset));
#endif

	// Copy requested subrange to user buffer
	std::memcpy(
		buffer,
		static_cast<char*>(aligned_buffer) + offset_delta,
		byte_count);

	// Free staging buffer
#ifdef _WIN32
	_aligned_free(aligned_buffer);
#else
	free(aligned_buffer);
#endif
}
}