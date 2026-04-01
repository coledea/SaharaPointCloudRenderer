#include "PriorityMutex.h"

namespace sahara::utils
{
PriorityMutex::PriorityMutex()
	: m_is_locked(false)
	, m_high_priority_threads_waiting(0)
{
}

void PriorityMutex::lock(bool high_priority)
{
	m_mutex.lock();

	if (high_priority)
	{
		m_high_priority_threads_waiting++;
		while (m_is_locked)
		{
			m_internal_wait_condition.wait(&m_mutex);
		}
		m_high_priority_threads_waiting--;
	}
	else
	{
		while (m_is_locked || m_high_priority_threads_waiting > 0)
		{
			m_internal_wait_condition.wait(&m_mutex);
		}
	}

	m_is_locked = true;
	m_mutex.unlock();
}

void PriorityMutex::unlock()
{
	m_mutex.lock();
	m_is_locked = false;
	m_internal_wait_condition.wakeAll();
	m_mutex.unlock();
}

void PriorityMutex::wait(QWaitCondition* condition)
{
	assert(m_is_locked);
	m_mutex.lock();
	m_is_locked = false;
	m_internal_wait_condition.wakeAll();
	condition->wait(&m_mutex);
	m_is_locked = true;
	m_mutex.unlock();
}

}