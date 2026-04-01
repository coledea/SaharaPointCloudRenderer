#include "AttributeMetadata.h"
#include "utils/NumericLimits.h"

namespace sahara::geometry
{

inline AttributeMetadata::AttributeMetadata(const QString& name, AttributeSemantic semantic, AttributeType type)
	: name(name)
	, semantic(semantic)
	, type(type)
{
}

inline size_t AttributeMetadata::singleEntrySizeInBytes() const noexcept
{
	return 0;
}

template <typename T>
inline TypedAttributeMetadata<T>::TypedAttributeMetadata(const QString& name, AttributeSemantic semantic, const AttributeDataVector<T>& data)
	: AttributeMetadata(name, semantic, TypeToAttributeTypeEnum<T>::type)
	, minimum(utils::numeric_limits<T>::max())
	, maximum(utils::numeric_limits<T>::lowest())
{
	using namespace std; // Necessary for the compiler to find overloads for native datatypes
	// TODO: could do this multi-threaded
	for (const auto& entry : data)
	{
		this->minimum = min(this->minimum, entry);
		this->maximum = max(this->maximum, entry);
	}
}

template <typename T>
inline TypedAttributeMetadata<T>::TypedAttributeMetadata(const QString& name, AttributeSemantic semantic, const T& minimum, const T& maximum)
	: AttributeMetadata(name, semantic, TypeToAttributeTypeEnum<T>::type)
	, minimum(minimum)
	, maximum(maximum)
{
}

template <typename T>
inline size_t TypedAttributeMetadata<T>::singleEntrySizeInBytes() const noexcept
{
	return sizeof(T);
}

}
