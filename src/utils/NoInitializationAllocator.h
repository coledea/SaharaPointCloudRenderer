#pragma once

#include <memory>

namespace sahara::utils
{
// Adapted from: https://stackoverflow.com/questions/21028299/is-this-behavior-of-vectorresizesize-type-n-under-c11-and-boost-container/21028742#21028742
// An allocator that does not initialize its members. Can be used for resizing containers without initialization, which is useful if the contents are overwritten anyways.
template <typename T, typename A = std::allocator<T>>
class NoInitializationAllocator : public A
{
	typedef std::allocator_traits<A> a_t;

public:
	template <typename U>
	struct rebind
	{
		using other = NoInitializationAllocator<U, typename a_t::template rebind_alloc<U>>;
	};

	using A::A;

	template <typename U>
	void construct(U* ptr) noexcept(std::is_nothrow_default_constructible<U>::value);

	template <typename U, typename... Args>
	void construct(U* ptr, Args&&... args);
};
}

#include "NoInitializationAllocator.inl"