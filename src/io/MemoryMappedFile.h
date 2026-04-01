#pragma once

#include <cstddef>
#include <string>

namespace sahara::io
{

class MemoryMappedFile
{
public:
	MemoryMappedFile(const std::string& filepath);
	~MemoryMappedFile();

	template <typename T>
	void read(size_t offset, size_t count, T* buffer) const;

private:
	size_t m_size;
	void* m_data;

#ifdef _WIN32
	void* m_file_handle;
	void* m_mapping_handle;
#else
	int m_file_descriptor;
#endif
};

}

#include "MemoryMappedFile.inl"