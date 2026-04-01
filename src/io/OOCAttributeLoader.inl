#include "OOCAttributeLoader.h"
#include "utils/NoInitializationAllocator.h"

namespace sahara::io
{

template <typename T>
inline OOCAttributeLoader<T>::OOCAttributeLoader(const std::filesystem::path& path)
	: m_attribute_file(path.string())
{
}

template <typename T>
inline std::unique_ptr<geometry::AbstractAttributeData> OOCAttributeLoader<T>::loadAttributeData(size_t point_offset, int count)
{
	auto raw_data = geometry::AttributeDataVector<T>(count);
	m_attribute_file.read(point_offset, count, raw_data.data());
	return std::make_unique<geometry::AttributeData<T>>(std::move(raw_data));
}

}