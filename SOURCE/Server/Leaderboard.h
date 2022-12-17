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
#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <list>
#include <vector>
#include <string>
#include "Components.h"
#include "PlayerStats.h"
#include "Components.h"
#include "json/json.h"

#include <boost/thread.hpp>

class Leader {
public:
	int mId;
	std::string mName;
	PlayerStatSet mStats;
	Leader();
	void WriteToJSON(Json::Value &value);
};

class Leaderboard
{
public:
	Leaderboard();
	virtual ~Leaderboard();

	Platform_CriticalSection cs;  //Needed for external account management since the HTTP threads might be accessing this concurrently.
	std::string mName;
	unsigned long mCollected;
	std::vector<Leader> mLeaders;

	void Build();
	void SetName(std::string name);
	virtual void OnBuild(std::vector<Leader> *leaders) =0;
};

class LeaderboardManager
{
public:
	LeaderboardManager();
	~LeaderboardManager();

	int InitThread(int globalThreadID);
	void Shutdown();
	void AddBoard(Leaderboard* board);
	Leaderboard* GetBoard(std::string name);

private:
	std::vector<Leaderboard*> mBoards;
	void RunMain();
	bool mIsActive;
	bool mIsExist;
	boost::thread *mThread;
	unsigned long mThreadID;
	int mGlobalThreadID;
};


extern LeaderboardManager g_LeaderboardManager;

#endif //#ifndef LEADERBOARD_H
