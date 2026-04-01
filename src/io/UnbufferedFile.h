#pragma once

#include <cstddef>
#include <string>

namespace sahara::io
{

class UnbufferedFile
{
public:
	UnbufferedFile(const std::string& filepath);
	~UnbufferedFile();

	template <typename T>
	void read(size_t offset, size_t count, T* buffer) const;

private:
	size_t m_size;
	size_t m_sector_size;

#ifdef _WIN32
	void* m_file_handle;
#else
	int m_file_descriptor;
#endif
};

}

#include "UnbufferedFile.inl"