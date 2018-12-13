#ifndef PARTYMANAGER_H
#define PARTYMANAGER_H 

#include <string>
#include <vector>
#include "Creature.h"

class PartyMember
{
public:
	int mCreatureDefID;
	int mCreatureID;
	std::string mDisplayName;
	CreatureInstance *mCreaturePtr;
	CharacterData *mCharPtr;
	int mSocket;

	// PVP scores (a team is a party)
	int mPVPKills;
	int mPVPDeaths;
	int mPVPGoals;

	bool IsOnlineAndValid();
	bool IsOnline();

};


//this.LootModes <- {
//	FREE_FOR_ALL = 0,
//	ROUND_ROBIN = 1,
//	LOOT_MASTER = 2
//};
//this.LootFlags <- {
//	NEED_B4_GREED = 1,
//	MUNDANE = 2
//};


enum LootFlags
{
	NEED_B4_GREED = (1 << 0),
	MUNDANE = (1 << 1)
};
enum LootMode
{
	FREE_FOR_ALL  = 0,   //The default
	ROUND_ROBIN = 1,	 //Offer to each player in turn
	LOOT_MASTER = 2      //Leader keeps all loot
};

//Holds details about a tagged loot, i.e. a party loot. Each item for each player
//gets a tag, and this used in the handshaking when deciding the window of the
//roll
class LootTag
{
public:
	int lootTag;
	int mItemId;
	int mCreatureId;
	int mLootCreatureId;
	int mSlotIndex;
	bool needed;

	LootTag();
	LootTag(const LootTag *tag);
	LootTag(int tag, int itemId, int creatureId, int lootCreatureId);

	bool Valid(void);
	void Clear(void);
};

class ActiveInstance;

class ActiveParty
{
public:
	int mPartyID;
	int mLeaderDefID;
	int mLeaderID;
	LootMode mLootMode;
	int mLootFlags;
	unsigned int mNextToGetLoot;
	std::string mLeaderName;
	std::vector<PartyMember> mMemberList;
	std::map<int, LootTag> lootTags;

	// PVP scores (a team is a party)
	int mPVPTeam;
	int mPVPKills;
	int mPVPDeaths;
	int mPVPGoals;

	ActiveParty();
	~ActiveParty();
	LootTag GetTag(int itemId, int creatureId);
	void RemoveTagsForLootCreatureId(int lootCreatureId, int itemId, int creatureId);
	void RemoveCreatureTags(int itemId, int creatureId);
	LootTag TagItem(int itemId, int creatureId, int lootCreatureId, int slot);
	bool HasTags(int lootCreatureId, int itemId);
	void Dump();
	void AddMember(CreatureInstance* member);
	bool HasMember(int memberDefID);
	void RemoveMember(int memberDefID);
	bool UpdateLeaderDropped(int memberID);
	bool SetLeader(int newLeaderDefID);
	PartyMember* GetNextLooter();
	PartyMember* GetMemberByID(int memberID);
	PartyMember* GetMemberByDefID(int memberDefID);
	bool UpdatePlayerReferences(CreatureInstance* member);
	int GetOnlineMemberCount(void);
	bool RemovePlayerReferences(int memberDefID, bool disconnect);
	void RebroadCastMemberList(char *buffer);
	void DebugDestroyParty(const char *buffer, int length);
	void Disband(char *buffer);
	int GetMaxPlayerLevel(void);
	void BroadcastInfoMessageToAllMembers(const char *buffer);

	void BroadCast(const char *buffer, int length);
	void BroadCastExcept(const char *buffer, int length, int excludeDefID);
	void BroadCastTo(const char *buffer, int length, int creatureDefID);
	void AttemptSend(int socket, const char *buffer, int length);
};

struct PartyUpdateOpTypes
{
	enum
	{
		INVITE					= 0,
		PROPOSE_INVITE			= 1,
		INVITE_REJECTED			= 2,
		ADD_MEMBER				= 3,
		REMOVE_MEMBER			= 4,
		JOINED_PARTY			= 5,
		LEFT_PARTY				= 6,
		IN_CHARGE				= 7,
		STRATEGY_CHANGE			= 8,
		STRATEGYFLAGS_CHANGE	= 9,
		OFFER_LOOT				= 10,
		LOOT_ROLL 				= 11,
		LOOT_WIN 				= 12,
		QUEST_INVITE			= 13,
	};
};

struct PartyManager
{
	std::vector<ActiveParty> mPartyList;
	int nextPartyID;

	PartyManager();
	ActiveParty* GetPartyByLeader(int leaderDefID);
	ActiveParty* GetPartyByID(int partyID);
	ActiveParty* GetPartyWithMember(int memberDefID);
	ActiveParty* CreateParty(CreatureInstance* leader);
	int GetNextPartyID(void);

	int AcceptInvite(CreatureInstance* member, CreatureInstance* leader);

	void BroadcastAddMember(CreatureInstance* member);
	bool DoDisband(int partyID);
	bool DoQuit(CreatureInstance* member);
	void DoRejectInvite(int leaderDefID, const char* nameDenied);
	void DoSetLeader(CreatureInstance *callMember, int newLeaderID);
	void DoKick(CreatureInstance *caller, int memberID);
	void DoQuestInvite(CreatureInstance *caller, const char *questName, int questID);
	void DeletePartyByID(int partyID);
	void UpdatePlayerReferences(CreatureInstance* member);
	void RemovePlayerReferences(int memberDefID, bool disconnect);
	void BroadCastPacket(int partyID, int callDefID, const char *buffer, int buflen);
	void CheckMemberLogin(CreatureInstance* member);
	int PrepMemberList(char *outbuf, int partyID, int memberID);
	void DebugDestroyParties(void);
	void DebugForceRemove(CreatureInstance *caller);

	static int StrategyFlagsChange(char *outbuf, int newFlags);
	static int StrategyChange(char *outbuf, LootMode newLootMode);
	static int OfferLoot(char *outbuf, int itemDefID, const char *lootTag, bool needed);

	static int WriteLootRoll(char *outbuf, const char *itemDefName, char roll, const char *bidder);
	static int WriteLootWin(char *outbuf, const char *lootTag, const char *originalTag, const char *winner, int creatureId, int slotIndex);
	static int WriteInvite(char *outbuf, int leaderId, const char *leaderName);
	static int WriteProposeInvite(char *outbuf, int proposeeId, const char *proposeeName, int proposerId, const char *proposerName);
	static int WriteMemberList(char *outbuf, ActiveParty *party, int memberID);
	static int WriteLeftParty(char *outbuf);
	static int WriteRemoveMember(char *outbuf, int memberID);
	static int WriteInCharge(char *outbuf, ActiveParty *party);
	static int WriteQuestInvite(char *outbuf, const char* questName, int questID);
	static int WriteRejectInvite(char *outbuf, const char *memberDenied);

	char WriteBuf[512];
};

extern PartyManager g_PartyManager;

#endif //#ifndef PARTYMANAGER_H
