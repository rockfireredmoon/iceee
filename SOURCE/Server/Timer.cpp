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

#include "Timer.h"
#include <algorithm>


TimerManager g_TimerManager;

//
// TimerTask
//

TimerTask::TimerTask() {
	mWhen = 0;
	mTaskId = 0;
}

TimerTask::~TimerTask() {
}

void TimerTask::cancel() {

}

//
// TimerManager
//

TimerManager::TimerManager() {
	mNextRun = 0;
	mNextTaskId = 1;
}

TimerManager::~TimerManager() {
}

bool TaskSort(const TimerTask *l1, TimerTask *l2) {
	return l1->mWhen < l2->mWhen;
}

void TimerManager::RunTasks() {
	if(mTasks.size() > 0) {
		unsigned long now = time(NULL);
		if(now > mNextRun) {
			cs.Enter("TimerManager::RunTasks");
			TimerTask *t = mTasks[0];
			mTasks.erase(mTasks.begin());
			cs.Leave();
			t->run();
			delete t;

			// Update next run time
			cs.Enter("TimerManager::RunTasks");
			if(mTasks.size() > 0) {
				mNextRun = mTasks[0]->mWhen;
			}
			else {
				mNextRun = 0;
			}
			cs.Leave();
		}
	}
}

void TimerManager::AddTask(TimerTask *task) {
	cs.Enter("TimerManager::AddTask");
	task->mTaskId = mNextTaskId++;
	mTasks.push_back(task);
	sort(mTasks.begin(), mTasks.end(), TaskSort);
	cs.Leave();
	mNextRun = mTasks[0]->mWhen;
}

