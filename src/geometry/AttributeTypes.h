#pragma once

#include "Color.h"

#include <QVector3D>

namespace sahara::geometry
{

enum class AttributeType
{
	Int,
	Uint,
	Float,
	Vector3D,
	Color,
	Undefined
};

template <AttributeType T>
struct AttributeTypeEnumToType
{
	using type = void;
};

template <>
struct AttributeTypeEnumToType<AttributeType::Int>
{
	using type = int;
};

template <>
struct AttributeTypeEnumToType<AttributeType::Float>
{
	using type = float;
};

template <>
struct AttributeTypeEnumToType<AttributeType::Uint>
{
	using type = uint;
};

template <>
struct AttributeTypeEnumToType<AttributeType::Vector3D>
{
	using type = QVector3D;
};

template <>
struct AttributeTypeEnumToType<AttributeType::Color>
{
	using type = Color;
};

// #####################################################

template <typename T>
struct TypeToAttributeTypeEnum
{
	static constexpr AttributeType type = AttributeType::Undefined;
};

template <>
struct TypeToAttributeTypeEnum<int>
{
	static constexpr AttributeType type = AttributeType::Int;
};

template <>
struct TypeToAttributeTypeEnum<float>
{
	static constexpr AttributeType type = AttributeType::Float;
};

template <>
struct TypeToAttributeTypeEnum<uint>
{
	static constexpr AttributeType type = AttributeType::Uint;
};

template <>
struct TypeToAttributeTypeEnum<QVector3D>
{
	static constexpr AttributeType type = AttributeType::Vector3D;
};

template <>
struct TypeToAttributeTypeEnum<Color>
{
	static constexpr AttributeType type = AttributeType::Color;
};

constexpr int numberOfComponents(AttributeType type)
{
	if (type == AttributeType::Vector3D || type == AttributeType::Color)
	{
		return 3;
	}
	else
	{
		return 1;
	}
}

}