#include "Util.h"
#include "Clan.h"
#include "Config.h"
#include "StringUtil.h"
#include "Cluster.h"
#include "DirectoryAccess.h"

#include <string.h>
#include <stdlib.h>
#include <vector>
#include "util/Log.h"

Clans::ClanManager g_ClanManager;

namespace Clans {


namespace Rank {

const char *GetNameByID(int id) {
	switch (id) {
	case LEADER:
		return "Leader";
	case OFFICER:
		return "Officer";
	case MEMBER:
		return "Member";
	}
	return "Initiate";
}

int GetIDByName(const std::string &name) {
	if (name.compare("Leader") == 0)
		return LEADER;
	if (name.compare("Officer") == 0)
		return OFFICER;
	if (name.compare("Member") == 0)
		return MEMBER;
	return INITIATE;
}
}

//
// ClanMember
//
ClanMember::ClanMember() {
	mID = 0;
	mRank = Rank::MEMBER;
}

void ClanMember::WriteToJSON(Json::Value &value) {
	value["id"] = mID;
	value["rank"] = mRank;
}

void ClanMember::CopyFrom(ClanMember &original) {
	mID = original.mID;
	mRank = original.mRank;
}

//
// Clan
//

Clan::Clan() {
	mId = 0;
	mName = "";
	mMOTD = "";
	mCreated = 0;
}
Clan::~Clan() {
}

bool Clan :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_CLAN, StringUtil::Format("%d", mId));
	writer->Value("Name", mName);
	writer->Value("MOTD", mMOTD);
	writer->Value("Created", mCreated);
	STRINGLIST l;
	for(auto a = mMembers.begin(); a != mMembers.end(); ++a) {
		l.push_back(StringUtil::Format("%d:%d", (*a).mID, (*a).mRank));
	}
	writer->ListValue("Member", l);
	l.clear();
	for(auto a = mPendingMembers.begin(); a != mPendingMembers.end(); ++a) {
		l.push_back(StringUtil::Format("%d", *a));
	}
	writer->ListValue("PendingMember", l);
	return true;
}

bool Clan :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_CLAN, StringUtil::Format("%d", mId), true);
	return true;
}

bool Clan :: ReadEntity(AbstractEntityReader *reader) {
	mName = reader->Value("Name");
	mMOTD = reader->Value("MOTD");
	mCreated = reader->ValueULong("Created");
	STRINGLIST m = reader->ListValue("Member");
	for(auto a = m.begin(); a != m.end(); ++a) {
		STRINGLIST l;
		Util::Split(*a, ",", l);
		ClanMember mem;
		if (l.size() > 1) {
			mem.mID = atoi(l[0].c_str());
			mem.mRank = atoi(l[1].c_str());
			mMembers.push_back(mem);
		}
	}
	m = reader->ListValue("PendingMember");
	for(auto a = m.begin(); a != m.end(); ++a) {
		mPendingMembers.push_back(atoi((*a).c_str()));
	}
	return true;
}


ClanMember Clan::GetFirstMemberOfRank(int rank) {
	for(std::vector<ClanMember>::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
		if((*it).mRank == rank)
			return *it;
	}
	return ClanMember();
}

bool Clan::HasMember(int id) {
	return GetMember(id).mID == id;
}

bool Clan::RemoveMember(ClanMember &member) {
	for(std::vector<ClanMember>::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
		if((*it).mID == member.mID) {
			mMembers.erase(it);
			return true;
		}
	}
	return false;
}

ClanMember Clan::GetMember(int id) {
	for(std::vector<ClanMember>::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
		if((*it).mID == id)
			return *it;
	}
	return ClanMember();
}

void Clan::UpdateMember(ClanMember &member) {
	for(std::vector<ClanMember>::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
		if((*it).mID == member.mID) {
			(*it).CopyFrom(member);
			return;
		}
	}
}

void Clan::WriteToJSON(Json::Value &value) {
	value["id"] = mId;
	value["name"] = mName;
	value["motd"] = mMOTD;
	value["createdTime"] = Json::UInt64(mCreated);
	char buf[12];
	Json::Value members;
	for(std::vector<ClanMember>::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
		Json::Value v;
		(*it).WriteToJSON(v);
		Util::SafeFormat(buf,sizeof(buf), "%d", (*it).mID);
		members[buf] = v;
	}
	value["members"] = members;
}

//
// ClanManager
//

ClanManager::ClanManager() {
}

ClanManager::~ClanManager() {
}

void ClanManager::CreateClan(Clan &clan) {
	clan.mId = g_ClusterManager.NextValue(ID_NEXT_CLAN_ID);
	clan.mCreated = time(NULL);
	SaveClan(clan);
}

Clan ClanManager::GetClan(int id) {
	Clan c;
	if(!LoadClan(id, c)) {
		g_Logs.server->info("Failed to load clan [%v] from cluster", id);
	}
	return c;
}

std::vector<Clan> ClanManager::GetClans() {
	std::vector<Clan> c;
	g_ClusterManager.Scan([this, &c](const std::string &key) {
		STRINGLIST l;
		Util::Split(key, ":", l);
		c.push_back(GetClan(atoi(l[1].c_str())));
	}, StringUtil::Format("%s:", KEYPREFIX_CLAN.c_str()));
	return c;
}

bool ClanManager::LoadClan(int id, Clan &clan) {
	clan.mId = id;
	return g_ClusterManager.ReadEntity(&clan);
}

bool ClanManager::HasClan(int clanID) {
	return g_ClusterManager.HasKey(StringUtil::Format("%s:%d", KEYPREFIX_CLAN.c_str(), clanID));
}

int ClanManager::FindClanID(const std::string &clanName) {
	return atoi(g_ClusterManager.GetKey(StringUtil::Format("%s:%s", KEYPREFIX_CLAN_NAME_TO_ID.c_str(), clanName.c_str()), "-1").c_str());
}

bool ClanManager::RemoveClan(Clan &clan) {
	g_ClusterManager.RemoveKey(StringUtil::Format("%s:%s", KEYPREFIX_CLAN_NAME_TO_ID.c_str(), clan.mName.c_str()));
	if (!g_ClusterManager.RemoveEntity(&clan)) {
		g_Logs.server->info("Failed to remove clan [%v] from cluster", clan.mName);
		return false;
	}
	return true;
}

bool ClanManager::SaveClan(Clan &clan) {
	g_ClusterManager.SetKey(StringUtil::Format("%s:%s", KEYPREFIX_CLAN_NAME_TO_ID.c_str(), clan.mName.c_str()), StringUtil::Format("%d", clan.mId));
	if(!g_ClusterManager.WriteEntity(&clan)) {
		g_Logs.data->warn("Failed to save clan %v (%v) to cluster.", clan.mId, clan.mName);
		return false;
	}
	return true;
}

}
