#pragma once

#include <string>
#include <thread>
#include <atomic>

#include <core/Guards.h>

namespace core {


enum class IfRunning {
	Fail,
	Join,
	Detach
};

enum class TaskState {
	Starting,
	Started,
	Stopping,
	Stopped,
	Killing
};

class Task {
protected:
	Task(std::string const& name, unsigned idleWaitMs = 30);

	/// Move-constructor.
	Task(Task&& m);

	/// Move-assignment.
	Task& operator=(Task&& m);

	virtual ~Task() { terminate(); }

	/// Allows changing worker name if work is stopped.
	void setName(std::string name) { if (!isWorking()) m_name = name; }

	/// Starts worker thread; causes startedWorking() to be called.
	void startWorking();

	/// Stop worker thread; causes call to stopWorking().
	void stopWorking();

	/// Returns if worker thread is present.
	bool isWorking() const { Guard l(x_task); return m_state == TaskState::Started; }

	/// Called after thread is started from startWorking().
	virtual void startedWorking() {}

	/// Called continuously following sleep for m_idleWaitMs.
	virtual void doWork() {}

	/// Overrides doWork(); should call shouldStop() often and exit when true.
	virtual void workLoop();
	bool shouldStop() const { return m_state != TaskState::Started; }

	/// Called when is to be stopped, just prior to thread being joined.
	virtual void doneWorking() {}

	/// Blocks caller into worker thread has finished.
//	void join() const { Guard l(x_work); try { if (m_work) m_work->join(); } catch (...) {} }

	/// Stop and never start again.
	/// This has to be called in the destructor of any most derived class.  Otherwise the worker thread will try to lookup vptrs.
	/// It's OK to call terminate() in destructors of multiple derived classes.
	void terminate();

private:
	std::string m_name;

	unsigned m_idleWaitMs = 0;

	mutable Mutex x_task;						///< Lock for the network existance and m_state_notifier.
	std::unique_ptr<std::thread> m_task;		///< The network thread.
    mutable std::condition_variable m_state_notifier; //< Notification when m_state changes.
	std::atomic<TaskState> m_state = {TaskState::Starting};
};

}