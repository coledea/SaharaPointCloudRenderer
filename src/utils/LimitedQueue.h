#pragma once

#include <vector>

namespace sahara::utils
{

// A queue with a fixed capacity that does not take any items when full. Faster than std::queue with a dequeue for many insertions.
template <typename T>
class LimitedQueue
{
public:
	explicit LimitedQueue(size_t capacity);

	void push(const T& item);
	void pop();
	T& front();
	bool empty() const;

	void clear();

private:
	std::vector<T> buffer;
	size_t start;
	size_t end;
	bool full;
};
}

#include "LimitedQueue.inl"