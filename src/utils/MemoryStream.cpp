#include "MemoryStream.h"

namespace sahara::utils
{

MemoryStream::MemoryStream(std::vector<uint8_t>&& byte_buffer)
	: m_buffer(std::move(byte_buffer))
	, std::istream(this)
{
	char* begin = reinterpret_cast<char*>(const_cast<uint8_t*>(m_buffer.data()));
	setg(begin, begin, begin + m_buffer.size());
}

std::streampos MemoryStream::seekoff(std::streamoff off, std::ios_base::seekdir dir, std::ios_base::openmode which)
{
	if (dir == std::ios_base::cur)
	{
		gbump(static_cast<int>(off));
	}
	else if (dir == std::ios_base::beg)
	{
		setg(eback(), eback() + off, egptr());
	}
	else
	{
		setg(eback(), egptr() + off, egptr());
	}
	return gptr() - eback();
}

std::streampos MemoryStream::seekpos(std::streampos pos, std::ios_base::openmode which)
{
	return seekoff(pos, std::ios_base::beg, which);
}

}