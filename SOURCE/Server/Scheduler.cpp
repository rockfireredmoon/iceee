#include "Scheduler.h"
#include "Components.h"
#include "Config.h"
#include "util/Log.h"
#include "StringUtil.h"


#define MAX_TASKS_PER_CYCLE 5

Scheduler g_Scheduler;

bool ScheduledTaskSort(const ScheduledTimerTask &l1, ScheduledTimerTask &l2) {
	return l1.mWhen < l2.mWhen;
}

//
// ScheduledTimerTask
//

ScheduledTimerTask::ScheduledTimerTask() {
	mWhen = 0;
	mTaskId = 0;
}
ScheduledTimerTask::ScheduledTimerTask(const TaskType &task,
		unsigned long when) {
	mWhen = when;
	mTask = task;
	mTaskId = 0;
}

ScheduledTimerTask::~ScheduledTimerTask() {
}

//
// Scheduler
//

Scheduler::Scheduler() {
	mPool = new boost::asio::thread_pool(POOL_SIZE);
	mRunning = true;
	mNextTaskId = 0;
	mNextRun = 0;
}

Scheduler::~Scheduler() {
	Shutdown();
}

void Scheduler::Init() {
}

void Scheduler::Shutdown() {
	if(mRunning) {
		mMutex.lock();
		mRunning = false;
		g_Logs.server->info("Shutting down scheduler, %v tasks to clear", scheduled.size());
		g_Logs.server->info("REMOVEME Stopping POOL");
		mPool->stop();
		g_Logs.server->info("REMOVEME Stopping QUEUE");
		mQueue.stop();
		g_Logs.server->info("REMOVEME Stopping SCHED");
		scheduled.clear();
		g_Logs.server->info("REMOVEME Stopping DEL");
		delete mPool;
		g_Logs.server->info("REMOVEME Stopping UNL");
		mMutex.unlock();
		g_Logs.server->info("Shut down scheduler");
	}
}

void Scheduler::Cancel(int id) {
	mMutex.lock();
	for (auto it = scheduled.begin();
			it != scheduled.end(); ++it) {
		if (it->mTaskId == id) {
			mMutex.unlock();
			scheduled.erase(it);
			return;
		}
	}
	mMutex.unlock();
}

void Scheduler::RunProcessingCycle() {
	mMutex.lock();
	int c = 0;

//	g_Logs.server->info("Have %v tasks in scheduler, next run is %v", scheduled.size(), mNextRun);
	while(mRunning && mNextRun > 0 && c < MAX_TASKS_PER_CYCLE && scheduled.size() > 0) {
		unsigned long now = g_PlatformTime.getMilliseconds();
		if(now > mNextRun) {
			ScheduledTimerTask t = scheduled[0];
			scheduled.erase(scheduled.begin());
			if(g_Logs.server->enabled(el::Level::Trace))
				g_Logs.server->trace("Scheduled task running %v", t.mTaskId);

//			mMutex.unlock();
//			t.mTask();
//			mMutex.lock();

			mQueue.post(t.mTask);

			// Update next run time
			if(scheduled.size() > 0) {
				mNextRun = scheduled[0].mWhen;
				if(g_Logs.server->enabled(el::Level::Trace))
					g_Logs.server->trace("Next scheduler task will run in %v", StringUtil::FormatTimeHHMMSSmm(mNextRun - g_ServerTime));
			}
			else {
				mNextRun = 0;
			}
			c++;
		}
		else
			break;
	}
	mMutex.unlock();

	mQueue.run();
	mMutex.lock();
	mQueue.restart();
	mMutex.unlock();
}

void Scheduler::Pool(const TaskType& task) {
	boost::asio::post(*mPool, task);
}

int Scheduler::ScheduleIn(const TaskType& task, unsigned long when) {
	return Schedule(task, when + g_ServerTime);
}

int Scheduler::Schedule(const TaskType& task, unsigned long when) {
	if(when <= g_ServerTime) {
		when = g_ServerTime + g_MainSleep;
	}
	ScheduledTimerTask taskWrapper(task, when);
	if(g_Logs.server->enabled(el::Level::Debug))
			g_Logs.server->debug("This scheduler (id: %v) task will run in %v", mNextTaskId, StringUtil::FormatTimeHHMMSSmm(when - g_ServerTime));
	mMutex.lock();
	taskWrapper.mTaskId = mNextTaskId++;
	scheduled.push_back(taskWrapper);
	sort(scheduled.begin(), scheduled.end(), ScheduledTaskSort);
	mNextRun = scheduled[0].mWhen;
	if(g_Logs.server->enabled(el::Level::Trace))
			g_Logs.server->trace("Next scheduler task will run in %v", StringUtil::FormatTimeHHMMSSmm(mNextRun - g_ServerTime));
	mMutex.unlock();
	return taskWrapper.mTaskId;
}

bool Scheduler::IsRunning() {
	return mRunning;
}

int Scheduler::Schedule(const TaskType& task) {
	return Schedule(task, g_ServerTime);
}

void Scheduler::Submit(const TaskType& task) {
	mQueue.post(task);
	//Schedule(task, g_ServerTime);
}

