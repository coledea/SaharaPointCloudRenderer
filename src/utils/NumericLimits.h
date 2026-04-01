#pragma once
#include "geometry/Color.h"

#include <QVector3D>

namespace sahara::utils
{

template <typename T>
struct numeric_limits
{
	static constexpr T min() noexcept
	{
		return std::numeric_limits<T>::min();
	};

	static constexpr T max() noexcept
	{
		return std::numeric_limits<T>::max();
	};

	static constexpr T lowest() noexcept
	{
		return std::numeric_limits<T>::lowest();
	};
};

template <>
struct numeric_limits<sahara::geometry::Color>
{
	static constexpr sahara::geometry::Color min() noexcept
	{
		return sahara::geometry::Color(0, 0, 0);
	}

	static constexpr sahara::geometry::Color max() noexcept
	{
		return sahara::geometry::Color(255, 255, 255);
	}

	static constexpr sahara::geometry::Color lowest() noexcept
	{
		return sahara::geometry::Color(0, 0, 0);
	}
};

template <>
struct numeric_limits<QVector3D>
{
	static constexpr QVector3D min() noexcept
	{
		return QVector3D(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
	}

	static constexpr QVector3D max() noexcept
	{
		return QVector3D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	}

	static constexpr QVector3D lowest() noexcept
	{
		return QVector3D(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	}
};

}