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
#ifndef CLAN_H
#define CLAN_H

#include <map>
#include <vector>
#include "json/json.h"
#include "Components.h"
#include "Entities.h"

static std::string KEYPREFIX_CLAN = "Clan";
static std::string ID_NEXT_CLAN_ID = "NextClanID";
static std::string KEYPREFIX_CLAN_NAME_TO_ID = "ClanNameToID";

namespace Clans {

namespace Rank {
enum {
	UNKNOWN = 0, INITIATE = 1, MEMBER = 2, OFFICER = 3, LEADER = 4
};
const char *GetNameByID(int eventID);
int GetIDByName(const std::string &eventName);
}

class ClanMember {
public:
	ClanMember();
	int mID;
	int mRank;

	void CopyFrom(ClanMember &original);
	void WriteToJSON(Json::Value &value);
};

class Clan: public AbstractEntity {
public:
	Clan();
	~Clan();

	int mId;
	std::string mName;
	std::string mMOTD;
	std::vector<ClanMember> mMembers;
	std::vector<int> mPendingMembers;
	unsigned long mCreated;

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	ClanMember GetMember(int id);
	bool RemoveMember(ClanMember &member);
	bool HasMember(int id);
	void UpdateMember(ClanMember &member);
	ClanMember GetFirstMemberOfRank(int rank);
	void WriteToJSON(Json::Value &value);

};

class ClanManager {
public:
	Platform_CriticalSection cs;

	ClanManager();
	~ClanManager();

	bool HasClan(int id);
	void CreateClan(Clan &clan);
	Clan GetClan(int id);
	std::vector<Clan> GetClans();
	int FindClanID(const std::string &clanName);
	bool SaveClan(Clan &clan);
	bool RemoveClan(Clan &clan);
	bool LoadClan(int id, Clan &clan);

};

}

extern Clans::ClanManager g_ClanManager;

#endif //#ifndef CLAN_H
