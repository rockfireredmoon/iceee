#include "FriendStatus.h"
#include "FileReader.h"

#include "Util.h"
#include "DirectoryAccess.h"
#include "Config.h"
#include "Cluster.h"
#include "StringUtil.h"
#include "util/Log.h"

FriendListManager g_FriendListManager;

SocialWindowEntry::SocialWindowEntry(void) {
	Clear();
}

void SocialWindowEntry::Clear(void) {
	creatureDefID = 0;
	name.clear();
	level = 0;
	profession = 0;
	online = false;
	status.clear();
	shard.clear();
}

void SocialWindowEntry::CopyFrom(SocialWindowEntry& source) {
	creatureDefID = source.creatureDefID;
	name = source.name;
	level = source.level;
	profession = source.profession;
	online = source.online;
	status = source.status;
	shard = source.shard;
}

bool SocialWindowEntry::WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_FRIEND_LIST, StringUtil::Format("%d", creatureDefID));
	writer->Value("Level", level);
	writer->Value("Profession", profession);
	writer->Value("Online", online);
	writer->Value("Status", status);
	writer->Value("Shard", shard);
	return true;
}

bool SocialWindowEntry::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_FRIEND_LIST, StringUtil::Format("%d", creatureDefID));
	return true;
}

bool SocialWindowEntry::ReadEntity(AbstractEntityReader *reader) {
	level = reader->ValueInt("Level");
	profession = reader->ValueInt("Profession");
	online = reader->ValueBool("Online");
	status = reader->Value("Status");
	shard = reader->Value("Shard");
	return true;
}

void FriendListManager::UpdateSocialEntry(SocialWindowEntry &data) {
	if (!g_ClusterManager.WriteEntity(&data, false)) {
		g_Logs.data->info("Failed to save social entry [%v] to the cluster.",
				data.creatureDefID);
	}
}

void FriendListManager::EnumerateFriends(SEARCH_INPUT& inPlayers,
		SEARCH_OUTPUT& outResults) {
	SOCIAL_MAP::iterator it;
	for (size_t i = 0; i < inPlayers.size(); i++) {
		SocialWindowEntry data;
		data.creatureDefID = inPlayers[i].first;
		if (g_ClusterManager.ReadEntity(&data)) {
			outResults.push_back(data);
		}
	}
}

void FriendListManager::UpdateNetworkEntry(int CreatureDefID,
		std::vector<int>& FriendDefIDs) {
	STRINGLIST l;
	for (auto a = FriendDefIDs.begin(); a != FriendDefIDs.end(); ++a) {
		l.push_back(StringUtil::Format("%d", *a));
	}
	g_ClusterManager.ListSet(
			StringUtil::Format("%s:%d", LISTPREFIX_FRIEND_NETWORK.c_str(),
					CreatureDefID), l);
}

bool FriendListManager::IsMutualFriendship(int selfDefID, int otherDefID) {
	if (HasPlayerInNetwork(selfDefID, otherDefID) == false)
		return false;

	//Search the reverse.  If we get to this point and it returns true, the friendship is mutual.
	return HasPlayerInNetwork(otherDefID, selfDefID);
}

bool FriendListManager::HasPlayerInNetwork(int firstDefID, int otherDefID) {
	STRINGLIST friends = g_ClusterManager.GetList(
			StringUtil::Format("%s:%d", LISTPREFIX_FRIEND_NETWORK.c_str(),
					firstDefID));
	return std::find(friends.begin(), friends.end(),
			StringUtil::Format("%d", otherDefID)) != friends.end();
}
