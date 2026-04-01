#pragma once

#include "AttributeData.h"
#include "AttributeSpecification.h"

#include <QString>

namespace sahara::geometry
{

struct AttributeMetadata
{
	QString name;
	AttributeType type;
	AttributeSemantic semantic;

	AttributeMetadata(const QString& name, AttributeSemantic semantic, AttributeType type);

	virtual ~AttributeMetadata() = default;
	virtual size_t singleEntrySizeInBytes() const noexcept;
};

template <typename T>
struct TypedAttributeMetadata : public AttributeMetadata
{
	TypedAttributeMetadata(const QString& name, AttributeSemantic semantic, const AttributeDataVector<T>& data);
	TypedAttributeMetadata(const QString& name, AttributeSemantic semantic, const T& minimum, const T& maximum);

	size_t singleEntrySizeInBytes() const noexcept override;

	T minimum;
	T maximum;
};

}

#include "AttributeMetadata.inl"
