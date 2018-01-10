#include "Scheduler.h"
#include "Components.h"
#include "Config.h"
#include "util/Log.h"

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

void Scheduler::Cancel(int id) {
	SYNCHRONIZED(mMutex){
		for (auto it = scheduled.begin();
				it != scheduled.end(); ++it) {
			if (it->mTaskId == id) {
				scheduled.erase(it);
				return;
			}
		}
	}
}

void Scheduler::RunProcessingCycle() {
	SYNCHRONIZED(mMutex){
		int c = 0;

		while(mNextRun > 0 && c < MAX_TASKS_PER_CYCLE && scheduled.size() > 0) {
			unsigned long now = g_PlatformTime.getMilliseconds();
			if(now > mNextRun) {
				ScheduledTimerTask t = scheduled[0];
				scheduled.erase(scheduled.begin());
				t.mTask();

				// Update next run time
				if(scheduled.size() > 0) {
					mNextRun = scheduled[0].mWhen;
					g_Logs.server->debug("Next scheduler task will run at %v", mNextRun);
				}
				else {
					mNextRun = 0;
				}
				c++;
			}
			else
				break;
		}
	}
}

int Scheduler::Pool(const TaskType& task) {
	mQueue.Push(task);
	return 0;
}

int Scheduler::Schedule(const TaskType& task, unsigned long when) {
	ScheduledTimerTask taskWrapper(task, when);
	SYNCHRONIZED(mMutex){
		taskWrapper.mTaskId = mNextTaskId++;
		scheduled.push_back(taskWrapper);
		sort(scheduled.begin(), scheduled.end(), ScheduledTaskSort);
		mNextRun = scheduled[0].mWhen;
		g_Logs.server->debug("Next scheduler task will run at %v", mNextRun);
	}
	return taskWrapper.mTaskId;
}

int Scheduler::Submit(const TaskType& task) {
	return Schedule(task, g_ServerTime);
}

TaskType Scheduler::PopPoolTask() {
	return mQueue.Pop();
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
	while(true) {
		g_Logs.server->debug("Waiting for work on thread %v", mWorkerID);
		TaskType t = g_Scheduler.PopPoolTask();
		t();
		PLATFORM_SLEEP(g_MainSleep);
	}
}
