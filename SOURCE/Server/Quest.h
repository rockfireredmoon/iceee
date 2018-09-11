/*  Quest activation procedure:

Creature must have the IS_USABLE status effect enabled.  Typically these creatures
also have the UNATTACKABLE and INVINCIBLE flags.  Otherwise they'll be considered
an enemy type, and right clicking them will issue an autoattack instead of an
interaction request.

When the client requests a creature with the IS_USABLE flag, it sends two
queries to the client:
- "creature.isusable"  which returns a fairly simple response like "Y", "N", or "D"
- "quest.indicator" which returns which kind of quest marker appears over its head.

When a creature is marked as having an available quest, you may click on it in-game.
This sends the "quest.getquestoffer" with a parameter of the creature ID that was
clicked on.  The server then sends back the quest ID associated with this creature.

The client sends the "quest.genericdata" query with the quest ID as a parameter.  The
server now returns a more complex data set, including everything you see in the
quest accept/decline screen (title, description, objectives, exp, money, rewards).

When you accept the quest, the client sends several queries.
- "quest.join" with two parameters, the Quest ID and Creature ID
- "quest.list" which attempts to fetch the current objective status for all quests.
- "quest.indicator" requesting an update for the floating quest icon.
- "creature.isusable" requesting an update for the usability status.

*/

#pragma once
#ifndef QUEST_H
#define QUEST_H

#include <string>
#include <vector>
#include <map>
#include "QuestAction.h"
#include "CommonTypes.h"
#include "Creature.h"

class QuestDefinitionContainer;
class QuestDefinition;
class QuestJournal;
struct QuestAct;
struct QuestObjective;

class QuestItemReward
{
public:
	QuestItemReward();
	~QuestItemReward();

	int itemID;      //ID of the item given for this reward.
	int itemCount;   //Number of items of this type as a reward (for stackable items like potions)
	bool required;   //If true, the item is "required" to be granted to the player.  If false, the player can select it as an optional item. For optional cases, be sure to appropriately set the "NumRewards" quest data field.

	void Clear(void);
	void CopyFrom(const QuestItemReward &other);
	char * Print(char *buffer);
};

//Entry as returned by the "quest.data" query.
//class QuestData
//{
//public:
//	QuestData();
//	~QuestData();
//
//	int questID;      //[0]
//	std::string title;     //[1]
//	std::string bodyText;  //[2]
//	std::string compText;  //[3] Completion text
//	int level;        //[4]
//	int experience;   //[5]
//	int partySize;    //[6]
//	int numRewards;   //[7]
//	int coin;         //[8]
//	bool unabandon;   //[9] Unabandonable (written as "true" or "false")
//
//	std::string sGiver;
//	std::string sEnder;
//
//	//Three sections  (for(i = 12; i < 25; i += 6)
//	// [12]   i+0  string description.  If not empty, get the rest.
//	// [13]   i+1  bool complete ("true", "false" ?)
//	// [14]   i+2  int myCreatureDefID
//	// [15]   i+3  int myItemID
//	// [16]   i+4  string completeText
//	// [17]   i+5  string markerLocations  "x,y,z,zone;x,y,z,zone;..."
//	// { [18] [19] [20] [21] [22] [23] }
//	// { [24] [25] [26] [27] [28] [29] }
//
//	//[30], [31], [32], [33]
//	QuestItemReward rewardItem[4];  //"id:# count:# required:false"
//	
//
//	/*
//	int questID;      //[0]
//	string title;     //[1]
//	string bodyText;  //[2]
//	string compText;  //[3] Completion text
//	int level;        //[4]
//	int experience;   //[5]
//	int partySize;    //[6]
//	int numRewards;   //[7]
//	int coin;         //[8]
//
//
//	//Two sections
//	// [11]   i+0  description.  If not empty, get the rest.
//	// [12]   i+1  complete ("true", "false" ?)
//	// [13]   i+2  myCreatureDefID
//	// [14]   i+3  myItemID
//	// [15]   i+4  completeText
//	// [16]   i+5  markerLocations  "x,y,z,zone;x,y,z,zone;..."
//
//
//	//[29], [30], [31], [32]
//	ItemEntry rewardItem[4];  "id:# count:# required:false
//	*/
//};

struct QuestRepeatDelay
{
	int QuestID;
	unsigned long StartTimeMinutes;
	unsigned long WaitTimeMinutes;

	QuestRepeatDelay(int questID, unsigned long startTimeMinutes, unsigned long waitTimeMinutes);
	bool IsAvailable(void);
};

struct QuestReference
{
	int QuestID;
	int CreatureDefID;  //Typically the quest giver ID
	QuestDefinition *DefPtr;
	short ObjCounter[MAXOBJECTIVES];    // Number of objects killed/gathered/activated.
	char ObjComplete[MAXOBJECTIVES];    // Objective completed.
	char Complete;
	short CurAct;
	int Outcome;
	QuestReference()
	{
		QuestID = 0;
		CreatureDefID = 0;
		DefPtr = NULL;
		Reset();
	}
	QuestReference(int nQuestID, QuestDefinition *questDef)
	{
		QuestID = nQuestID;
		CreatureDefID = 0;
		DefPtr = questDef;
		Reset();
	}
	QuestReference(int nQuestID, QuestDefinition *questDef, int nCreatureDefID)
	{
		QuestID = nQuestID;
		CreatureDefID = nCreatureDefID;
		DefPtr = questDef;
		Reset();
	}

	void RunObjectiveCompleteScripts(int CID, int act, int obj);
	int CheckQuestObjective(int CID, char *buffer, int type, int targetCDefID, int targetCID);
	int CheckCompletedAct(QuestAct *defAct);
	int CheckTravelLocation(int x, int y, int z, int zone);
	void ClearObjectiveData(void);
	int AdvanceAct(int CID, char *buffer, QuestDefinition *questDef);
	void ResetObjectives(void);
	void Reset(void);
	QuestDefinition* GetQuestPointer(void); 
	bool operator < (const QuestReference &compare) const { return QuestID < compare.QuestID; }
	bool TestInvalid(void);
	int CompleteQuest(int CID, char *buffer, QuestDefinition *qdef);
	int QuestJournal(int CID, char *buffer, QuestDefinition *qdef);

	/* DISABLED, NEVER FINISHED
	bool HasItemObjective(void);
	void QueryItemObjectives(std::vector<int> &resultList);
	*/

	static bool testEquivalenceByQuestID(QuestReference &a, QuestReference &b) { return (a.QuestID == b.QuestID); }
};

class QuestReferenceContainer
{
public:
	QuestReferenceContainer();
	~QuestReferenceContainer();
	void Free(void);

	std::vector<QuestReference> itemList;

	void AddItem(int newQuestID, QuestDefinition *qdef);
	void AddItem(QuestReference &newItem);
	void Sort(void);
	void StartScript(CreatureInstance *creature);
	int HasQuestID(int searchVal);
	int HasCreatureDef(int searchVal);
	void RemoveIndex(size_t index);
	void ResolveIDs(void);
	QuestReference* GetItem(int questID);
	int HasCreatureReturn(int searchVal);
	int HasObjectInteraction(int CreatureDefID);

	int GetAvailableQuestFor(int CreatureDefID, QuestJournal *questJournal);

	void RemoveInvalidEntries(void);
	int GetInvalidEntry(void);
};

class QuestJournal
{
public:
	QuestJournal();
	~QuestJournal();
	static const int QUEST_SOON_TOLERANCE = 2;

	static const int SHARE_SUCCESS_QUALIFIES = 0;
	static const int SHARE_FAILED_COMPLETED = -1;
	static const int SHARE_FAILED_ACTIVE = -2;
	static const int SHARE_FAILED_INVALIDQUEST = -3;
	static const int SHARE_FAILED_REQUIRE = -4;
	static const int SHARE_FAILED_AVAILABLE = -5;

	void Clear(void);
	QuestReferenceContainer availableQuests;
	QuestReferenceContainer completedQuests;
	QuestReferenceContainer activeQuests;
	QuestReferenceContainer availableSoonQuests;
	
	std::vector<QuestRepeatDelay> delayedRepeat;

	bool IsCompleted(int QuestID);
	void AddPendingQuest(QuestReference &newItem);
	int QuestJoin_Helper(int questID);
	int CheckQuestShare(int questID);
	static const char * GetQuestShareErrorString(int errCode);

	int CheckQuestObjective(int CID, char *buffer, int type, int targetCDefID, int targetCID);
	int GetCurrentAct(int questID);

	/* DISABLED, NEVER FINISHED
	bool HasItemObjectives(void);
	void QueryItemObjectives(std::vector<int> &resultList);
	int NotifyItemCount(int itemID, int itemCount, char *writeBuf);
	*/
	
	//Experimental migratory functions
	const char * CreatureIsusable(int CreatureDefID);
	const char * QuestIndicator(int CreatureDefID);
	char * QuestGetQuestOffer(int CreatureDefID, char *convBuf);
	int QuestGenericData(char *buffer, int bufsize, char *convBuf, int QuestID, int QueryIndex);
	int QuestData(char *buffer, char *convBuf, int QuestID, int QueryIndex);
	int QuestJoin(char *buffer, int QuestID, int QueryIndex);
	int QuestList(char *buffer, char *convBuf, int QueryID);
	int QuestGetCompleteQuest(char *buffer, char *convBuf, int CreatureDefID, int QueryIndex);
	int QuestGetCompleteQuest_Helper(int creatureDefID);
	int QuestComplete(int QuestID);
	QuestObjective* CreatureUse(int CreatureDefID, int &QuestID, int &CurrentAct);
	int CreatureUse_Confirmed(int CID, char *buffer, int targetCreatureDefID, int targetCreatureID);
	int CheckTravelLocations(int CID, char *buffer, int x, int y, int z, int zone);
	int CheckQuestTalk(char *buffer, int CreatureDefID, int CreatureInstID, int PlayerCID);
	int ForceComplete(int CID, int QuestID, char *buffer);
	int ForceAllComplete(int CID, char *buffer);
	void QuestResetObjectives(int CID, int QuestID);
	void QuestClear(int CID, int QuestID);
	void QuestLeave(int CID, int QuestID);
	int FilterEmote(int CID, char *outbuf, const char *message, int xpos, int zpos, int zoneID);

	static int WriteQuestJoin(char *buffer, int questID);

	void ResolveLoadedQuests(void);
	bool IsQuestRedeemable(QuestDefinition* questDef, int QuestID, int QuestEnderCreatureID);

	void AddQuestRepeatDelay(int questID, unsigned long startTimeMinutes, unsigned long waitTimeMinutes);
	void CheckRepeatDelay(void);
	void RemoveOldRepeatQuests(void);
	int GetRepeatDelayIndex(int questID);
};

class QuestOutcome
{
public:
	std::string compText;  //[3]    The speech text that is displayed to the player when they redeem the quest and accept rewards.
	int experience;        //[5]    Experience points awarded to the player when the quest is redeemed.
	int numRewards;        //[7]    If the player is given a choice of which reward items to choose, this is the number of items they are required to select from the reward box.  Usually set to 1 when multiple rewards are offered.  Should not be set if the quest has only one reward, because the player implicitly accepts it, and if this field is set then the player must explicitly select it even though there are no other options.
	int coin;              //[8]    Copper granted to the player when the quest is redeemed.
	//[30], [31], [32], [33]
	static const int MAXREWARDS = 4;
	QuestItemReward rewardItem[MAXREWARDS];  //"id:# count:# required:false"

	int heroism;         //This is a quest completion bonus new to this server.

	int valourGiven;	// The amount of valour given on completion

	QuestOutcome() {
		Clear();
	}

	~QuestOutcome()	{
		Clear();
	}

	void CopyFrom(const QuestOutcome &other)
	{
		valourGiven = other.valourGiven;
		heroism = other.heroism;
		compText = other.compText;
		experience = other.experience;
		numRewards = other.numRewards;
		for(size_t i = 0; i < MAXREWARDS; i++)
			rewardItem[i].CopyFrom(other.rewardItem[i]);
	}

	void Clear(void) {
		valourGiven = 0;
		heroism = 0;
		compText.clear();
		experience = 0;
		numRewards = 0;
		for(int i = 0; i < MAXREWARDS; i++)
			rewardItem[i].Clear();
	}
};

class QuestObjective
{
public:
	static const int OBJECTIVE_TYPE_NONE = 0;
	static const int OBJECTIVE_TYPE_TRAVEL = 1;
	static const int OBJECTIVE_TYPE_KILL = 2;
	static const int OBJECTIVE_TYPE_ACTIVATE = 3;
	static const int OBJECTIVE_TYPE_GATHER = 4;
	static const int OBJECTIVE_TYPE_TALK = 5;
	static const int OBJECTIVE_TYPE_EMOTE = 6;
	static const int OBJECTIVE_TYPE_GENERIC = 7;

	static const int EVENTMSG_STATUS = 0;
	static const int EVENTMSG_ACTCOMPLETED = 1;
	static const int EVENTMSG_QUESTCOMPLETED = 2;
	static const int EVENTMSG_JOURNAL = 3;
	static const int EVENTMSG_ABANDONED = 4;
	static const int EVENTMSG_JOINED = 5;
	static const int EVENTMSG_TURNEDIN = 6;  //Doesn't exist in the 0.6.0 client.

	static const int DEFAULT_ACTIVATE_TIME = 2000;
	static const int DEFAULT_TRAVEL_RANGE = 150;

	//This data is used internally by the server to process objectives.
	unsigned char type;
	bool gather;
	std::vector<int> data1;  //Data points.  See documentation.
	int data2;
	int ActivateTime;            //Determines how much time must be spent interacting with an object, ex: needing 2 seconds to gather.
	std::string ActivateText;    //The text string to send to the client when interacting, ex: "Opening crate..."

	//Travel:
	//  Data1: always 3 elements [x, y, z];
	//  Data2: required distance to target (box range)
	//Kill:
	//  Data1: list of creature IDs that trigger the objective
	//  Data2: number of kills required
	//Activate:
	//  Data1: list of creature IDs that trigger the objective
	//  Data2: number of activations/gathers required
	//Talk:
	//  Data1: Unused
	//  Data2: Unused.  The return Creature ID is stored in "myCreatureDefID"

	//This data is sent to the client as part of the quest information.
	std::string description;       //[offset] + 0
	int complete;             //[offset] + 1
	int myCreatureDefID;      //[offset] + 2
	int myItemID;             //[offset] + 3
	std::string completeText;      //[offset] + 4   //Shows in parenthesis after the objective.
	std::string markerLocations;   //[offset] + 5
	int outcome;

	QuestObjective()
	{
		Clear();
	}

	QuestObjective(int pType, std::string pDescription)
	{
		Clear();
		type = pType;
		description = pDescription;
	}

	static int GetTypeByName(char *name)
	{
		const static char *TypeName[8] = {
			"NONE", "KILL", "TRAVEL", "ACTIVATE", "GATHER", "TALK", "EMOTE", "GENERIC"
		};
		const static int TypeVar[8] = {
			QuestObjective::OBJECTIVE_TYPE_NONE,
			QuestObjective::OBJECTIVE_TYPE_KILL,
			QuestObjective::OBJECTIVE_TYPE_TRAVEL,
			QuestObjective::OBJECTIVE_TYPE_ACTIVATE,
			QuestObjective::OBJECTIVE_TYPE_GATHER,
			QuestObjective::OBJECTIVE_TYPE_TALK,
			QuestObjective::OBJECTIVE_TYPE_EMOTE,
			QuestObjective::OBJECTIVE_TYPE_GENERIC
		};

		for(int a = 1; a < 8; a++)
			if(strcmp(name, TypeName[a]) == 0)
				return TypeVar[a];

		return TypeVar[0];
	}

	void Clear(void)
	{
		outcome = -1;
		type = 0;
		data1.clear();
		data2 = 0;
		ActivateTime = 0;
		ActivateText = "";
		description = "";
		complete = 0;
		myCreatureDefID = 0;
		myItemID = 0;
		completeText = "";
		markerLocations = "";
		gather = false;
	}

	void AddData1(int data)
	{
		data1.push_back(data);
	}

	void CopyFrom(QuestObjective *objective)
	{
		outcome = objective->outcome;
		type = objective->type;
		data1.reserve(objective->data1.size());
		copy(objective->data1.begin(),objective->data1.end(),back_inserter(data1));
		data2 = objective->data2;
		ActivateTime = objective->ActivateTime;
		ActivateText = objective->ActivateText;
		description = objective->description;
		complete = objective->complete;
		myCreatureDefID = objective->myCreatureDefID;
		myItemID = objective->myItemID;
		completeText = objective->completeText;
		markerLocations = objective->markerLocations;
		gather = objective->gather;
	}
	int HasObjectiveCDef(int objType, int objCDef)
	{
		if(objType == -1 || objType == OBJECTIVE_TYPE_KILL)
		{
			for(size_t i = 0; i < data1.size(); i++)
				if(data1[i] == objCDef)
					return static_cast<int>(i);
		}
		if(objType == -1 || objType == OBJECTIVE_TYPE_ACTIVATE)
		{
			for(size_t i = 0; i < data1.size(); i++)
				if(data1[i] == objCDef)
					return static_cast<int>(i);
		}
		if(objType == -1 || objType == OBJECTIVE_TYPE_TALK)
		{
			if(myCreatureDefID == objCDef)
				return 0;
		}
		return -1;
	}
};


class QuestAct
{
public:
	QuestObjective objective[MAXOBJECTIVES];
	std::string BodyText;  //Each act has custom body text that differs from the "genericdata" text.

	QuestAct() {
		Clear();
	}
	QuestAct(std::string pBodyText, QuestObjective *obj) {
		Clear();
		BodyText = pBodyText;
		AddObjective(0, obj);
	}

	~QuestAct()
	{
		Clear();
	}
	void AddObjective(int index, QuestObjective *obj)
	{
		objective[index].CopyFrom(obj);
	}

	void Clear(void)
	{
		for(int a = 0; a < MAXOBJECTIVES; a++)
			objective[a].Clear();
		BodyText.clear();
	}
};

class QuestDefinition
{
public:
	QuestDefinition();
	~QuestDefinition();

	//Note: numbers in brackets (ex: [0]) indicate which row of the outgoing query data this field occupies.
	char profession;    //Determines a class restriction, if applicable (0=all, 1=knight, 2=rogue, 3=mage, 4=druid)
	int questID;           //[0]    Quest identification, for server lookups and client info.
	std::string title;     //[1]    Title name.
	std::string bodyText;  //[2]    Speech text that is provided to the player for them to read before they accept the quest.
	int levelSuggested;    //[4]    Recommended player level, displayed on the quest accept screen.
	int partySize;         //[6]    Recommended party size, displayed on the quest accept screen.
	int coin;              //[8]    Copper granted to the player when the quest is redeemed.
	bool unabandon;        //[9]    Unabandonable (written as "true" or "false")

	std::string sGiver;    //[10]   Location of the quest-giver NPC.  String as "x,y,z,zone"
	std::string sEnder;    //[11]   Location of the quest-ender NPC.  String as "x,y,z,zone"

	//QuestObjective objective[3];
	//Three sections  (for(i = 12; i < 25; i += 6)
	// [12]   i+0  string description.  If not empty, get the rest.
	// [13]   i+1  bool complete ("true", "false" ?)
	// [14]   i+2  int myCreatureDefID
	// [15]   i+3  int myItemID
	// [16]   i+4  string completeText
	// [17]   i+5  string markerLocations  "x,y,z,zone;x,y,z,zone;..."
	// { [18] [19] [20] [21] [22] [23] }
	// { [24] [25] [26] [27] [28] [29] }

	//This data is used internally by the server
	int levelMin;        //Minimum player level required to accept this quest.  Used for internal processing.
	int levelMax;        //Maximum player level required to accept this quest.  Used for internal processing.
	int Requires;        //A single quest ID for a quest that must be completed (leave zero for no prerequisite quest).
	int QuestGiverID;    //Creature Definition ID of the quest-giver NPC.
	int QuestEnderID;    //Creature Definition ID of the quest-ender NPC.
	std::vector<QuestAct> actList;
	int actCount;
	bool Repeat;         //Quest is repeatable and is not logged into the completed quest list. (such as bounty boards).
	unsigned long RepeatMinuteDelay;  //Number of minutes that must pass before the quest is reactivated for another one-time completion.  Specifically used for event quests so the ID does not remain forever in the completed list.  Note this is a special case and not related to <Repeat>, which must remain false for this work correctly.

	//	IceEE additions
	bool guildStart;	// This quest starts a guild (and so the player must not be in that guild for it to be available)
	int guildId;		// This quest requires the player is part of this guid
	int	valourRequired;	// The amount of valour required to activate the quest
	bool accountQuest;  // The quest may only be done once per account

	// This stuff is used internally for determining quest marker information.  It is extracted from the "sGiver" field and converted to numerical types here for faster processing.
	int giverX;
	int giverY;
	int giverZ;
	int giverZone;
	
	// Outcomes
	std::vector<QuestOutcome> outcomes;
	int outcomeCount;

	//These are extentions to allow a very basic form of command scripting within the game.
	 
	QuestCommand::QuestActionContainer mScriptAcceptCondition;    //Additional conditions to accept the quest.
	QuestCommand::QuestActionContainer mScriptAcceptAction;       //Actions to perform when the quest is accepted.
	QuestCommand::QuestActionContainer mScriptCompleteCondition;  //Additional conditions to perform to redeem the quest.
	QuestCommand::QuestActionContainer mScriptCompleteAction;       //Actions to perform when the quest is accepted.

	void Clear(void);
	QuestOutcome* GetOutcome(int index);
	QuestOutcome* GetLastOutcome();
	void AddOutcome(QuestOutcome outcome);
	void AddAct(QuestAct &act);
	void CopyFrom(const QuestDefinition &other);
	int GetObjective(int act, int type, int CDefID);
	QuestAct* GetActPtrByIndex(int index);
	bool FilterSelectedRewards(int outcomeIndex, const std::vector<size_t>& selectedIndexes, std::vector<QuestItemReward>& outputRewardList);
	void RunLoadDefaults(void);
	void RunLoadValidation(void);
	void SetRepeatTime(const char *format);

private:
	unsigned long CalculateMinutesFromString(const char *format);
};

class QuestDefinitionContainer
{
public:
	QuestDefinitionContainer();
	~QuestDefinitionContainer();

	unsigned long mVirtualQuestID;
	std::map<int, QuestDefinition> mQuests;
	typedef std::map<int, QuestDefinition>::iterator ITERATOR;

	void Clear(void);
	QuestDefinition* GetQuestDefPtrByID(int id);
	QuestDefinition* GetQuestDefPtrByName(const char *name);
	void LoadQuestPackages(std::string filename);
	void ResolveQuestMarkers(void);
	void AddIfValid(QuestDefinition &newItem);

private:
	void LoadFromFile(std::string filename);
	bool LimitIndex(int &value, int max);
	void AppendString(std::string &value, char *appendStr);
};


struct QuestIndicator
{
	static const char *QueryResponse[];
	static const int NONE = 0;
	static const int WILL_HAVE_QUEST = 1;
	static const int HAVE_QUEST = 2;
	static const int PLAYER_ON_QUEST = 3;
	static const int QUEST_COMPLETED = 4;
	static const int QUEST_INTERACT = 5;
};


extern QuestDefinitionContainer QuestDef;

int PrepExt_QuestStatusMessage(char *buffer, int questID, int objectiveIndex, bool complete, std::string message);

#endif //QUEST_H
