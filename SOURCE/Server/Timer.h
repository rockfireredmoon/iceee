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
#ifndef TIMER_H
#define TIMER_H

#include <vector>
#include "Components.h"

class TimerTask
{
public:
	TimerTask();
	virtual ~TimerTask();

	unsigned long mWhen;
	int mTaskId;

	virtual void run() =0;
};

class TimerManager
{
public:
	TimerManager();
	~TimerManager();
	Platform_CriticalSection cs;
	std::vector<TimerTask*> mTasks;
	void AddTask(TimerTask *task);
	void RunTasks();
private:
	unsigned long mNextRun;
	int mNextTaskId;
};


extern TimerManager g_TimerManager;

#endif
