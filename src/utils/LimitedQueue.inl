#include "LimitedQueue.h"

namespace sahara::utils
{

template <typename T>
inline LimitedQueue<T>::LimitedQueue(size_t capacity)
	: start(0)
	, end(0)
	, full(false)
	, buffer(capacity)
{
}

template <typename T>
inline void LimitedQueue<T>::push(const T& item)
{
	if (full)
	{
		return;
	}

	buffer[end] = item;
	end = (end + 1) % buffer.size();
	full = (start == end);
}

template <typename T>
inline void LimitedQueue<T>::pop()
{
	assert(!empty() && "Tried to pop from empty queue");
	start = (start + 1) % buffer.size();
	full = false;
}

template <typename T>
inline T& LimitedQueue<T>::front()
{
	return buffer[start];
}

template <typename T>
inline bool LimitedQueue<T>::empty() const
{
	return (!full && (start == end));
}

template <typename T>
inline void LimitedQueue<T>::clear()
{
	start = 0;
	end = 0;
	full = false;
}

}