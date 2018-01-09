// Holds cached data for friend lists, so that characters don't have to be loaded
// to determine their last known level or shard

#ifndef FRIENDSTATUS_H
#define FRIENDSTATUS_H

#include <map>
#include <string>
#include <vector>
#include "Entities.h"

static std::string KEYPREFIX_FRIEND_LIST = "FriendList";
static std::string LISTPREFIX_FRIEND_NETWORK = "FriendNetwork";

class SocialWindowEntry: public AbstractEntity {
public:
	int creatureDefID;
	std::string name;
	int level;
	char profession;
	bool online;
	std::string status;
	std::string shard;

	SocialWindowEntry();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
	void Clear(void);
	void CopyFrom(SocialWindowEntry& source);
};

class FriendListManager
{
public:
	typedef std::map<int, SocialWindowEntry> SOCIAL_MAP;  //First = playerDefID
	typedef std::pair<int, SocialWindowEntry> SOCIAL_PAIR;
	typedef std::pair<int, std::string> SEARCH_PAIR;
	typedef std::vector<SEARCH_PAIR> SEARCH_INPUT;
	typedef std::vector<SocialWindowEntry> SEARCH_OUTPUT;

	typedef std::map<int, std::vector<int> > NETWORK_MAP;  //First = playerDefID, Second = Array of friended player defs.

	//Friend SocialWindowEntry functions
	void UpdateSocialEntry(SocialWindowEntry &data);
	void EnumerateFriends(SEARCH_INPUT& inPlayers, SEARCH_OUTPUT& outResults);
	bool HasFriendOf(int playerDefID, int searchDefID);
	bool DeleteCharacter(int CreatureDefID);
	
	//Network functions
	void UpdateNetworkEntry(int CreatureDefID, std::vector<int>& FriendDefIDs);
	bool IsMutualFriendship(int selfDefID, int otherDefID);

private:
	bool HasPlayerInNetwork(int firstDefID, int otherDefID);
};

extern FriendListManager g_FriendListManager;

#endif //#ifndef FRIENDSTATUS_H
