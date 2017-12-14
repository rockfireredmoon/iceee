// Holds cached data for friend lists, so that characters don't have to be loaded
// to determine their last known level or shard

#ifndef FRIENDSTATUS_H
#define FRIENDSTATUS_H

#include <map>
#include <string>
#include <vector>

struct SocialWindowEntry
{
	int creatureDefID;
	std::string name;
	int level;
	char profession;
	bool online;
	std::string status;
	std::string shard;

	SocialWindowEntry();
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

	void LoadAllData(void);
	void SaveAllData(void);

	//Friend SocialWindowEntry functions
	void UpdateSocialEntry(SocialWindowEntry &data);
	void EnumerateFriends(SEARCH_INPUT& inPlayers, SEARCH_OUTPUT& outResults);
	bool HasFriendOf(int playerDefID, int searchDefID);
	
	//Network functions
	void UpdateNetworkEntry(int CreatureDefID, std::vector<int>& FriendDefIDs);
	bool IsMutualFriendship(int selfDefID, int otherDefID);

private:
	std::string socialDataFile;
	std::string networkDataFile;
	SOCIAL_MAP socialData;        //This controls a list of player social entries.  The information they hold is relevant to the friend list in-game.
	NETWORK_MAP networkData;      //This holds a network of friends.

	void LoadSocialData(void);
	void SaveSocialData(void);

	void LoadNetworkData(void);
	void SaveNetworkData(void);
	bool HasPlayerInNetwork(int firstDefID, int otherDefID);
};

extern FriendListManager g_FriendListManager;

#endif //#ifndef FRIENDSTATUS_H
