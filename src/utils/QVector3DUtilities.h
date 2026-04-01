#pragma once

#include <QString>
#include <QVector3D>
#include <cmath>

constexpr QVector2D min(const QVector2D& first, const QVector2D& second) noexcept
{
	return QVector2D(
		std::min(first.x(), second.x()),
		std::min(first.y(), second.y()));
}

constexpr QVector2D max(const QVector2D& first, const QVector2D& second) noexcept
{
	return QVector2D(
		std::max(first.x(), second.x()),
		std::max(first.y(), second.y()));
}

constexpr QVector3D min(const QVector3D& first, const QVector3D& second) noexcept
{
	return QVector3D(
		std::min(first.x(), second.x()),
		std::min(first.y(), second.y()),
		std::min(first.z(), second.z()));
}

constexpr QVector3D max(const QVector3D& first, const QVector3D& second) noexcept
{
	return QVector3D(
		std::max(first.x(), second.x()),
		std::max(first.y(), second.y()),
		std::max(first.z(), second.z()));
}

constexpr QVector3D floor(const QVector3D& v)
{
	return QVector3D(std::floor(v.x()), std::floor(v.y()), std::floor(v.z()));
}

constexpr QVector3D round(const QVector3D& v)
{
	return QVector3D(std::round(v.x()), std::round(v.y()), std::round(v.z()));
}

constexpr QVector3D abs(const QVector3D& v)
{
	return QVector3D(std::abs(v.x()), std::abs(v.y()), std::abs(v.z()));
}

constexpr QVector3D lerp(const QVector3D& first, const QVector3D& second, float t) noexcept
{
	return first + t * (second - first);
}

inline QString vectorToString(const QVector3D& value)
{
	return "[" + QString::number(value.x()) + ", " + QString::number(value.y()) + ", " + QString::number(value.z()) + "]";
}