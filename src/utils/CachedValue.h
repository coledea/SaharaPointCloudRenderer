#pragma once

#include <functional>

// adapted from: https://github.com/cginternals/gloperate/blob/master/source/gloperate/include/gloperate/base/CachedValue.h

namespace sahara::utils
{

template <typename T>
class CachedValue
{
public:
	CachedValue(std::function<void(T&)> update_Function);

	inline T& value();
	inline const T& value() const;
	inline void invalidate() noexcept;

private:
	inline void validate() const;

	mutable T m_value;
	mutable bool m_valid;
	std::function<void(T&)> m_update_function;
};

}

#include "CachedValue.inl"