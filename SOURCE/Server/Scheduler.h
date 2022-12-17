/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <deque>
#include "CommonTypes.h"

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread.hpp>


//
//
//	mWork = std::unique_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(mLogicService));
//
//#include <boost/asio.hpp>
//#include <boost/asio/post.hpp>
//#include <boost/thread.hpp>
//	void RunMainLoop();
//	void RunOnLogicThread(CompletionToken task);
//	boost::asio::io_service mLogicService; // The main logic service. Queues jobs to run on main thread
//	std::unique_ptr<boost::asio::io_service::work> mWork;
//
//void SimulatorManager::RunMainLoop(void) {
//	mLogicService.run();
//}
//
//void SimulatorManager::RunOnLogicThread(CompletionToken task) {
//	boost::asio::post(mLogicService, task);
////	mLogicService.post(task);
//}
//	mWork.reset();


#define POOL_SIZE 5

typedef boost::function<void()> TaskType;

class ScheduledTimerTask
{
public:
	ScheduledTimerTask();
	ScheduledTimerTask(const TaskType &task, unsigned long when);
	~ScheduledTimerTask();

	unsigned long mWhen;
	int mTaskId;
	TaskType mTask;
};

class Scheduler {
public:
	Scheduler();
	~Scheduler();
	void Init();
	void RunProcessingCycle();
	void Pool(const TaskType& task);
	int ScheduleIn(const TaskType& task, unsigned long wait);
	int Schedule(const TaskType& task, unsigned long when);
	int Schedule(const TaskType& task);
	void Submit(const TaskType& task);
	void Cancel(int id);
	void Shutdown();
	bool IsRunning();
private:
	bool mRunning;
	unsigned long mNextRun;
	int mNextTaskId;
	boost::asio::io_service mQueue;
	boost::asio::thread_pool *mPool;
	std::recursive_mutex mMutex;
	std::vector<ScheduledTimerTask> scheduled;
};

extern Scheduler g_Scheduler;

#endif //#define SCHEDULER_H
