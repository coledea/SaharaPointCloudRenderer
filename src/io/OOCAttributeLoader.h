#pragma once

#ifdef BYPASS_OS_FILE_CACHE
#	include "UnbufferedFile.h"
using StreamingFile = sahara::io::UnbufferedFile;
#else
#	include "MemoryMappedFile.h"
using StreamingFile = sahara::io::MemoryMappedFile;
#endif

#include "geometry/AttributeData.h"
#include "geometry/AttributeMetadata.h"

#include <QMutex>
#include <filesystem>

namespace sahara::io
{

class AbstractOOCAttributeLoader
{
public:
	virtual ~AbstractOOCAttributeLoader() = default;
	virtual std::unique_ptr<geometry::AbstractAttributeData> loadAttributeData(size_t point_offset, int count) = 0;
};

template <typename T>
class OOCAttributeLoader : public AbstractOOCAttributeLoader
{
public:
	OOCAttributeLoader(const std::filesystem::path& path);

	std::unique_ptr<geometry::AbstractAttributeData> loadAttributeData(size_t point_offset, int count) override;

private:
	StreamingFile m_attribute_file;
};

}

#include "OOCAttributeLoader.inl"