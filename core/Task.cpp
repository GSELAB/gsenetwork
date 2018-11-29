
#include "core/Task.h"

#include <chrono>
#include <thread>
#include "core/Log.h"

using namespace std;

namespace core {

Task::Task(std::string const& name, unsigned idleWaitMs): m_name(name), m_idleWaitMs(idleWaitMs)
{
    // CINFO << "Task:" << name;
}

Task::Task(Task&& m)
{
    std::swap(m_name, m.m_name);
}

Task& Task::operator=(Task&& m)
{
    assert(&m != this);
    std::swap(m_name, m.m_name);
    return *this;
}

void Task::startWorking()
{
    CINFO << "Task::startWorking for thread:" << m_name;
	std::unique_lock<std::mutex> l(x_task);
	if (m_task) {
		TaskState ex = TaskState::Stopped;
		m_state.compare_exchange_strong(ex, TaskState::Starting);
		m_state_notifier.notify_all();
	} else {
		m_state = TaskState::Starting;
		m_state_notifier.notify_all();
		m_task.reset(new thread([&]() {
			setThreadName(m_name.c_str());
			while (m_state != TaskState::Killing) {
				TaskState ex = TaskState::Starting;
				{
				    // the condition variable-related lock
                    unique_lock<mutex> l(x_task);
                    m_state = TaskState::Started;
				}

				m_state_notifier.notify_all();

				try {
					startedWorking();
					workLoop();
					doneWorking();
				} catch (std::exception const& _e) {
				    CWARN << "Task:" << m_name << " Exception thrown in Task thread: " << _e.what();
				}

                {
                    // the condition variable-related lock
                    unique_lock<mutex> l(x_task);
                    ex = m_state.exchange(TaskState::Stopped);
                    CINFO << "Task:" << m_name << " State: Stopped: Thread was" << (unsigned)ex;
                    if (ex == TaskState::Killing || ex == TaskState::Starting)
                        m_state.exchange(ex);
                }

				m_state_notifier.notify_all();
                {
                    unique_lock<mutex> l(x_task);
                    DEV_TIMED_ABOVE("Task stopping", 100)
                        while (m_state == TaskState::Stopped)
                            m_state_notifier.wait(l);
                }

			}
		}));
	}

	DEV_TIMED_ABOVE("Start Task", 100)
		while (m_state == TaskState::Starting)
			m_state_notifier.wait(l);
}

void Task::stopWorking()
{
	std::unique_lock<Mutex> l(x_task);
	if (m_task) {
		TaskState ex = TaskState::Started;
		if (!m_state.compare_exchange_strong(ex, TaskState::Stopping))
			return;
		m_state_notifier.notify_all();

		DEV_TIMED_ABOVE("Stop Task", 100)
			while (m_state != TaskState::Stopped)
				m_state_notifier.wait(l); // but yes who can wake this up, when the mutex is taken.
	}
}

void Task::terminate()
{
	CINFO << "stopWorking for thread" << m_name;
	std::unique_lock<Mutex> l(x_task);
	if (m_task) {
		if (m_state.exchange(TaskState::Killing) == TaskState::Killing) return; // Somebody else is doing this
		l.unlock();
		m_state_notifier.notify_all();
		DEV_TIMED_ABOVE("Terminate task", 100)
			m_task->join();

		l.lock();
		m_task.reset();
	}
}

void Task::workLoop()
{
	while (m_state == TaskState::Started) {
		if (m_idleWaitMs)
			this_thread::sleep_for(chrono::milliseconds(m_idleWaitMs));
		doWork();
	}
}

}