#pragma once

#include <cstdint>
#include <istream>
#include <vector>

namespace sahara::utils
{

// Helper class to read from a byte buffer in memory using std::istream.
class MemoryStream : private std::streambuf, public std::istream
{
public:
	MemoryStream(std::vector<uint8_t>&& byte_buffer);

	MemoryStream(MemoryStream&&) = delete;
	MemoryStream& operator=(MemoryStream&&) = delete;

private:
	std::vector<uint8_t> m_buffer;

	std::streampos seekoff(std::streamoff off, std::ios_base::seekdir dir, std::ios_base::openmode which) override;
	std::streampos seekpos(std::streampos pos, std::ios_base::openmode which) override;
};
}
