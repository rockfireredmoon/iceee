#include "CreditShop.h"
#include "Util.h"
#include "Clan.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "StringList.h"
#include <string.h>
#include <stdlib.h>
#include <vector>

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
	nextClanID = 1;
}

ClanManager::~ClanManager() {
}

std::string ClanManager::GetPath(int id) {
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "Clan/%d.txt", id);
	Platform::FixPaths(buf);
	return buf;
}

void ClanManager::CreateClan(Clan &clan) {
	clan.mId = nextClanID++;
	clan.mCreated = time(NULL);
	mClans[clan.mId] = clan;
	SaveClan(clan);
	SessionVarsChangeData.AddChange();
}

bool ClanManager::LoadClan(int id, Clan &clan) {
	const char * buf = GetPath(id).c_str();
	if (!Platform::FileExists(buf)) {
		g_Log.AddMessageFormat("No file for CS item [%s]", buf);
		return NULL;
	}

	FileReader lfr;
	if (lfr.OpenText(buf) != Err_OK) {
		g_Log.AddMessageFormat("Could not open file [%s]", buf);
		return false;
	}

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	long amt = -1;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (clan.mId != 0) {
					g_Log.AddMessageFormat(
							"[WARNING] %s contains multiple entries. CS items have one entry per file",
							buf);
					break;
				}
				clan.mId = id;
			} else if (strcmp(lfr.SecBuffer, "NAME") == 0)
				clan.mName = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "MOTD") == 0)
				clan.mMOTD = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "MEMBER") == 0) {
				std::vector<std::string> l;
				Util::Split(lfr.BlockToStringC(1, 0), ",", l);
				ClanMember mem;
				if (l.size() > 1) {
					mem.mID = atoi(l[0].c_str());
					mem.mRank = atoi(l[1].c_str());
					clan.mMembers.push_back(mem);
				} else {
					g_Log.AddMessageFormat(
							"Incomplete clan memober information [%s] in file [%s]",
							lfr.SecBuffer, buf);
				}
			} else
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s]",
						lfr.SecBuffer, buf);
		}
	}
	lfr.CloseCurrent();
	return true;
}

bool ClanManager::HasClan(int clanID) {
	return mClans.find(clanID) != mClans.end();
}

int ClanManager::FindClanID(std::string clanName) {
	for(std::map<int, Clan>::iterator it = mClans.begin(); it != mClans.end(); ++it) {
		if(it->second.mName.compare(clanName) == 0)
			return it->first;
	}
	return -1;
}

bool ClanManager::RemoveClan(Clan &clan) {

	const char * path = GetPath(clan.mId).c_str();
	if (!Platform::FileExists(path)) {
		g_Log.AddMessageFormat("No file for clan [%s] to remove", path);
		return false;
	}
	cs.Enter("ClanManager::RemoveClan");
	mClans.erase(mClans.find(clan.mId));
	cs.Leave();
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "Clan/%d.del", clan.mId);
	Platform::FixPaths(buf);
	if(!Platform::FileExists(buf) || remove(buf) == 0) {
		if(!rename(path, buf) == 0) {
			g_Log.AddMessageFormat("Failed to remove clan %d", clan.mId);
			return false;
		}
	}
	return true;
}

int ClanManager::LoadClans(void) {
	mClans.clear();

	Platform_DirectoryReader r;
	std::string dir = r.GetDirectory();
	r.SetDirectory("Clan");
	r.ReadFiles();
	r.SetDirectory(dir);

	std::vector<std::string>::iterator it;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			Clan c;
			if (LoadClan(atoi(Platform::Basename(p.c_str()).c_str()), c)) {
				mClans[c.mId] = c;
			}
		}
	}

	return 0;
}


bool ClanManager::SaveClan(Clan &clan) {
	std::string path = GetPath(clan.mId);
	g_Log.AddMessageFormat("Saving clan to %s.", path.c_str());
	FILE *output = fopen(path.c_str(), "wb");
	if (output == NULL) {
		g_Log.AddMessageFormat("[ERROR] Saving clan could not open: %s",
				path.c_str());
		return false;
	}

	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "ID=%d\r\n", clan.mId);
	fprintf(output, "Name=%s\r\n", clan.mName.c_str());
	fprintf(output, "MOTD=%s\r\n", clan.mMOTD.c_str());
	for(std::vector<ClanMember>::iterator it = clan.mMembers.begin(); it != clan.mMembers.end(); ++it) {
		fprintf(output, "Member=%d,%d\r\n", (*it).mID, (*it).mRank);
	}
	fprintf(output, "\r\n");
	fflush(output);
	fclose(output);
	mClans[clan.mId] = clan;
	return true;
}

}
