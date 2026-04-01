#include "CachedValue.h"

namespace sahara::utils
{

template <typename T>
inline CachedValue<T>::CachedValue(std::function<void(T&)> update_function)
	: m_update_function(update_function)
	, m_valid(false)
{
}

template <typename T>
inline T& CachedValue<T>::value()
{
	validate();
	return m_value;
}

template <typename T>
inline const T& CachedValue<T>::value() const
{
	validate();
	return m_value;
}

template <typename T>
inline void CachedValue<T>::invalidate() noexcept
{
	m_valid = false;
}

template <typename T>
inline void CachedValue<T>::validate() const
{
	if (!m_valid)
	{
		m_update_function(m_value);
		m_valid = true;
	}
}

}
