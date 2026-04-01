#pragma once

#include "AbstractAttributeData.h"
#include "utils/NoInitializationAllocator.h"

#include <vector>

namespace sahara::geometry
{

template <typename T>
using AttributeDataVector = std::vector<T, utils::NoInitializationAllocator<T>>;

template <typename T>
class AttributeData : public AbstractAttributeData
{
public:
	AttributeData(AttributeDataVector<T>&& data) noexcept;

	AttributeData(AttributeData&& attribute) noexcept;
	AttributeData(const AttributeData& attribute) noexcept;

	AttributeData& operator=(AttributeData&& attribute);

	size_t size() const noexcept override;
	size_t sizeInBytes() const noexcept override;
	const void* rawData() const noexcept override;

private:
	AttributeDataVector<T> m_data;
};

}

#include "AttributeData.inl"