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

#include "Leaderboard.h"
#include "Components.h"

#include "Util.h"
#include "util/Log.h"

PLATFORM_THREADRETURN LeaderboardThreadProc(PLATFORM_THREADARGS lpParam)
{
	LeaderboardManager *controller = (LeaderboardManager*)lpParam;
	controller->mIsExist = true;
	controller->mIsActive = true;
	AdjustComponentCount(1);
	controller->RunMainLoop();
	g_Logs.leaderboard->info("LeaderboardManager Thread shut down.");
	controller->mIsExist = false;
	AdjustComponentCount(-1);
	PLATFORM_CLOSETHREAD(0);
	return 0;
}

LeaderboardManager g_LeaderboardManager;

//
// Leader
//

Leader::Leader() {
	mId = 0;
	mName = "";
}

void Leader::WriteToJSON(Json::Value &value) {
	value["id"] = mId;
	value["name"] = mName;
	Json::Value stats;
	mStats.WriteToJSON(stats);
	value["stats"] = stats;
}

//
// LeaderBoard
//
Leaderboard::Leaderboard() {
	mCollected = 0;
	mName = "";
	cs.Init();
	cs.SetDebugName("Leaderboard");
}

Leaderboard::~Leaderboard() {
}

void Leaderboard::SetName(std::string name) {
	mName = name;
	cs.SetDebugName(name.c_str());
}

void Leaderboard::Build() {
	std::vector<Leader> l;
	OnBuild(&l);
	cs.Enter("Leaderboard::Build");
	mLeaders.clear();
	mLeaders.reserve(l.size());
	copy(l.begin(), l.end(), back_inserter(mLeaders));
	mCollected = time(NULL);
	cs.Leave();
}

//
// LeaderBoardManager
//

LeaderboardManager::LeaderboardManager() {
	mBoards.clear();
	mThreadID = 0;
	mGlobalThreadID = 0;
	mIsActive = false;
	mIsExist = false;
}

LeaderboardManager::~LeaderboardManager() {
}

Leaderboard* LeaderboardManager::GetBoard(std::string name) {
	for (std::vector<Leaderboard*>::iterator it = mBoards.begin();
			it != mBoards.end(); ++it) {
		if ((*it)->mName.compare(name) == 0) {
			return *it;
		}
	}
	return NULL;
}

int LeaderboardManager::InitThread(int globalThreadID) {
	mGlobalThreadID = globalThreadID;
	int r = Platform_CreateThread(0, (void*) LeaderboardThreadProc, this,
			&mThreadID);
	if (r == 0) {
		mIsActive = false;
		g_Logs.leaderboard->error(
				"LeaderboardManager %v could not create thread.",
				globalThreadID);
		return 1;
	}
	return 0;
}

void LeaderboardManager::Shutdown() {
	mIsActive = false;
}

void LeaderboardManager::RunMainLoop() {
	unsigned seconds = 0;
	while (mIsActive == true) {
		if (seconds % 60 == 0) {
			for (std::vector<Leaderboard*>::iterator it = mBoards.begin();
					it != mBoards.end(); ++it) {
				(*it)->Build();
			}
		}
		seconds++;
		PLATFORM_SLEEP(1000);
	}
}
