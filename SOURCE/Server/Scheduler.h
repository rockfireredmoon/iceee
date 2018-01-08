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

typedef std::function<void()> TaskType;

template <typename T>
class Queue {
public:
    void Push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
        }
        this->d_condition.notify_one();
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return rc;
    }
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
};

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

class PooledWorker {
public:
	PooledWorker(int workerID);
	~PooledWorker();
	//Thread delegation for loading scenery.
	static void ThreadProc(PooledWorker *object);

	void Work();

	int mWorkerID;
};

class Scheduler {
public:
	Scheduler();
	void Init();
	void RunProcessingCycle();
	int Pool(const TaskType& task);
	int Schedule(const TaskType& task, unsigned long when);
	int Submit(const TaskType& task);
	void Cancel(int id);
	TaskType PopPoolTask();
private:
	unsigned long mNextRun;
	int mNextTaskId;
	Queue<TaskType> mQueue;
	std::recursive_mutex mMutex;
	std::vector<ScheduledTimerTask> scheduled;
	std::vector<PooledWorker*> workers;
};

extern Scheduler g_Scheduler;

#endif //#define SCHEDULER_H
