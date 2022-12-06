#include "Scheduler.h"
#include "Components.h"
#include "Config.h"
#include "util/Log.h"
#include "StringUtil.h"

#define MAX_TASKS_PER_CYCLE 5
#define POOL_SIZE 5

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
	mRunning = true;
	mNextTaskId = 0;
	mNextRun = 0;
}

void Scheduler::Init() {
	/* Setup thread pool */
	for(int i = 0 ; i < POOL_SIZE; i++) {
		PooledWorker *worker = new PooledWorker(i);
		if(Platform_CreateThread(0, (void*)worker->ThreadProc, worker, NULL) == 0) {
			g_Logs.server->error("Failed to create worker thread %v", i);
		}
		else {
			workers.push_back(worker);
		}
	}
}

void Scheduler::Shutdown() {
	mRunning = false;
	g_Logs.server->info("Shutting down scheduler, %v tasks to clear", scheduled.size());
	mMutex.lock();
	scheduled.clear();
	mQueue.Clear();
	mMutex.unlock();
	g_Logs.server->info("Shut down scheduler");
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
			if(g_Logs.server->enabled(el::Level::Debug))
				g_Logs.server->debug("Scheduled task running %v", t.mTaskId);
			mMutex.unlock();
			t.mTask();
			mMutex.lock();

			// Update next run time
			if(scheduled.size() > 0) {
				mNextRun = scheduled[0].mWhen;
				if(g_Logs.server->enabled(el::Level::Debug))
					g_Logs.server->debug("Next scheduler task will run in %v", StringUtil::FormatTimeHHMMSSmm(mNextRun - g_ServerTime));
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
}

int Scheduler::Pool(const TaskType& task) {
	mQueue.Push(task);
	return 0;
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
			g_Logs.server->debug("This scheduler task will run in %v", StringUtil::FormatTimeHHMMSSmm(when - g_ServerTime));
	mMutex.lock();
	taskWrapper.mTaskId = mNextTaskId++;
	scheduled.push_back(taskWrapper);
	sort(scheduled.begin(), scheduled.end(), ScheduledTaskSort);
	mNextRun = scheduled[0].mWhen;
	if(g_Logs.server->enabled(el::Level::Debug))
			g_Logs.server->debug("Next scheduler task will run in %v", StringUtil::FormatTimeHHMMSSmm(mNextRun - g_ServerTime));
	mMutex.unlock();
	return taskWrapper.mTaskId;
}

bool Scheduler::IsRunning() {
	return mRunning;
}

int Scheduler::Submit(const TaskType& task) {
	return Schedule(task, g_ServerTime);
}

bool Scheduler::PopPoolTask(TaskType &task) {
	if(g_Logs.server->enabled(el::Level::Debug))
		g_Logs.server->debug("Popping pool task");
	bool r = mQueue.Pop(task);
	if(g_Logs.server->enabled(el::Level::Debug))
		g_Logs.server->debug("Popped pool task");
	return r;
}

PooledWorker::PooledWorker(int workerID) {
	mWorkerID = workerID;
}

PooledWorker::~PooledWorker() {

}

void PooledWorker::ThreadProc(PooledWorker *object) {
	object->Work();
}

void PooledWorker::Work() {
	g_Logs.server->info("Starting pooled thread worker %v", mWorkerID);
	while(g_Scheduler.IsRunning()) {
		if(g_Logs.server->enabled(el::Level::Debug))
			g_Logs.server->debug("Waiting for work on thread %v", mWorkerID);
		TaskType t;
		if(g_Scheduler.PopPoolTask(t)) {
			t();
			PLATFORM_SLEEP(g_MainSleep);
		}
		else
			break;
	}
	g_Logs.server->info("Left scheduler worker loop");
}
