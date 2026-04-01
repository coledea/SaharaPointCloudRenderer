#pragma once

#include <QMutex>
#include <QWaitCondition>

namespace sahara::utils
{

// A mutex that allows high priority threads to lock earlier than other threads, even if they are waiting already.
class PriorityMutex
{
public:
	PriorityMutex();

	void lock(bool high_priority = false);
	void unlock();

	void wait(QWaitCondition* condition);

private:
	QMutex m_mutex;
	QWaitCondition m_internal_wait_condition;
	bool m_is_locked;
	int m_high_priority_threads_waiting;

	QWaitCondition m_wait_condition;
};
}
