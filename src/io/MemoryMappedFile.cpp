#include "MemoryMappedFile.h"

#ifdef _WIN32
#	include <windows.h>
#	include <io.h>
#else
#	include <sys/mman.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <unistd.h>
#endif

#include <QDebug>

namespace sahara::io
{

MemoryMappedFile::MemoryMappedFile(const std::string& filepath)
	: m_data(nullptr)
	, m_size(0)
{
#if defined(_WIN32)
	m_file_handle = CreateFileA(filepath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_file_handle == INVALID_HANDLE_VALUE)
	{
		qWarning() << "Could not open file: " << filepath.c_str();
		return;
	}

	LARGE_INTEGER size;
	if (!GetFileSizeEx(m_file_handle, &size))
	{
		CloseHandle(m_file_handle);
		qWarning() << "Could not get file size for: " << filepath.c_str();
		return;
	}
	m_size = static_cast<size_t>(size.QuadPart);

	m_mapping_handle = CreateFileMappingA(m_file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (m_mapping_handle == nullptr)
	{
		CloseHandle(m_file_handle);
		qWarning() << "Could not create file mapping for: " << filepath.c_str();
		return;
	}
	m_data = MapViewOfFile(m_mapping_handle, FILE_MAP_READ, 0, 0, 0);
#else
	m_file_descriptor = open(filepath.c_str(), O_RDONLY);
	if (m_file_descriptor < 0)
	{
		qWarning() << "Could not open file: " << filepath.c_str();
		return;
	}

	struct stat file_stat;
	if (fstat(m_file_descriptor, &file_stat) < 0)
	{
		close(m_file_descriptor);
		qWarning() << "Could not get file size for: " << filepath.c_str();
		return;
	}

	m_size = static_cast<size_t>(file_stat.st_size);

	m_data = mmap(nullptr, m_size, PROT_READ, MAP_PRIVATE, m_file_descriptor, 0);
	if (m_data == MAP_FAILED)
	{
		m_data = nullptr;
		close(m_file_descriptor);
		qWarning() << "Could not map file: " << filepath.c_str();
		return;
	}
#endif
}

MemoryMappedFile::~MemoryMappedFile()
{
#if defined(_WIN32)
	if (m_data)
	{
		UnmapViewOfFile(m_data);
	}
	if (m_mapping_handle)
	{
		CloseHandle(m_mapping_handle);
	}
	if (m_file_handle)
	{
		CloseHandle(m_file_handle);
	}
#else
	if (m_data)
	{
		munmap(m_data, m_size);
	}
	if (m_file_descriptor >= 0)
	{
		close(m_file_descriptor);
	}
#endif
}
}