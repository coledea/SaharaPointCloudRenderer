#include "NoInitializationAllocator.h"

namespace sahara::utils
{

template <typename T, typename A>
template <typename U>
inline void NoInitializationAllocator<T, A>::construct(U* ptr) noexcept(std::is_nothrow_default_constructible<U>::value)
{
	// skips default initialization
}

template <typename T, typename A>
template <typename U, typename... Args>
inline void NoInitializationAllocator<T, A>::construct(U* ptr, Args&&... args)
{
	a_t::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
}
}