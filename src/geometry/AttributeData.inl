#include "AttributeData.h"
#include "utils/QVector3DUtilities.h"

namespace sahara::geometry
{

template <typename T>
inline AttributeData<T>::AttributeData(AttributeDataVector<T>&& data) noexcept
	: AbstractAttributeData()
	, m_data(std::move(data))
{
}

template <typename T>
inline AttributeData<T>::AttributeData(AttributeData&& attribute) noexcept
	: m_data(std::move(attribute.m_data))
{
}

template <typename T>
inline AttributeData<T>::AttributeData(const AttributeData& attribute) noexcept
	: m_data(attribute.m_data)
{
}

template <typename T>
inline AttributeData<T>& AttributeData<T>::operator=(AttributeData&& attribute)
{
	m_data = std::move(attribute.m_data);
	return *this;
}

template <typename T>
inline const void* AttributeData<T>::rawData() const noexcept
{
	return m_data.data();
}

template <typename T>
inline size_t AttributeData<T>::size() const noexcept
{
	return m_data.size();
}

template <typename T>
inline size_t AttributeData<T>::sizeInBytes() const noexcept
{
	return m_data.size() * sizeof(T);
}

}