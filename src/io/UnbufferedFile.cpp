#include "UnbufferedFile.h"

#ifdef _WIN32
#	include <windows.h>
#	include <io.h>
#else
#	include <sys/mman.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#endif

#include <QDebug>

namespace sahara::io
{

UnbufferedFile::UnbufferedFile(const std::string& filepath)
	: m_size(0)
	, m_sector_size(4096)
{
#ifdef _WIN32
	m_file_handle = CreateFileA(
		filepath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr);

	if (m_file_handle == INVALID_HANDLE_VALUE)
	{
		qWarning() << "Could not open file:" << filepath.c_str();
		return;
	}

	LARGE_INTEGER size;
	if (!GetFileSizeEx(m_file_handle, &size))
	{
		CloseHandle(m_file_handle);
		qWarning() << "Could not get file size:" << filepath.c_str();
		return;
	}
	m_size = static_cast<size_t>(size.QuadPart);

	// Query physical sector size
	DWORD bytes_per_sector = 0;
	GetDiskFreeSpaceA(nullptr, nullptr, &bytes_per_sector, nullptr, nullptr);
	m_sector_size = bytes_per_sector ? bytes_per_sector : 4096;

#else
	m_file_descriptor = open(filepath.c_str(), O_RDONLY | O_DIRECT);
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

	// TODO: query sector size
#endif
}

UnbufferedFile::~UnbufferedFile()
{
#ifdef _WIN32
	if (m_file_handle && m_file_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_file_handle);
	}
#else
	if (m_file_descriptor >= 0)
	{
		close(m_file_descriptor);
	}
#endif
}
}