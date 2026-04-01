#pragma once

#include <algorithm>
#include <cstdint>

namespace sahara::geometry
{

struct Color
{
	constexpr Color() noexcept
		: r(0)
		, g(0)
		, b(0)
	{
	}

	constexpr Color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept
		: r(r)
		, g(g)
		, b(b)
	{
	}

	uint8_t r;
	uint8_t g;
	uint8_t b;
};
}

constexpr sahara::geometry::Color min(const sahara::geometry::Color& first, const sahara::geometry::Color& second) noexcept
{
	return { std::min(first.r, second.r), std::min(first.g, second.g), std::min(first.b, second.b) };
}

constexpr sahara::geometry::Color max(const sahara::geometry::Color& first, const sahara::geometry::Color& second) noexcept
{
	return { std::max(first.r, second.r), std::max(first.g, second.g), std::max(first.b, second.b) };
}