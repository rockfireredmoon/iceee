#include "Quest.h"
#include "QuestScript.h"
#include "StringList.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "Character.h"

#include <algorithm>
#include "Util.h"

//#pragma warning(disable:4996)

const char *QuestIndicator::QueryResponse[] = {
	"NONE",
	"WILL_HAVE_QUEST",
	"HAVE_QUEST",
	"PLAYER_ON_QUEST",
	"QUEST_COMPLETED",
	"QUEST_INTERACT",
};

QuestDefinitionContainer QuestDef;

//extern int g_ProtocolVersion;

QuestRepeatDelay :: QuestRepeatDelay(int questID, unsigned long startTimeMinutes, unsigned long waitTimeMinutes)
{
	QuestID = questID;
	StartTimeMinutes = startTimeMinutes;
	WaitTimeMinutes = waitTimeMinutes;
}

bool QuestRepeatDelay :: IsAvailable(void)
{
	unsigned long currentTime = g_PlatformTime.getAbsoluteMinutes();
	if(currentTime > (StartTimeMinutes + WaitTimeMinutes))
		return true;
	return false;
}

int QuestReference :: CheckQuestObjective(int CID, char *buffer, int type, int CDefID)
{
	static char ConvBuf[64];
	static char completeStr[] = "Complete";

	char *writeStr = &ConvBuf[0];

	int tsize = 0;
	int wpos = 0;
	if(DefPtr == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] CheckQuestObjective unresolved quest ID: %d", QuestID);
		return 0;
	}
	QuestDefinition *qd = DefPtr;   //We changed away from using a vector, so just assign as an alias.
	if(CurAct < 0 || CurAct >= (int)qd->actList.size())
	{
		g_Log.AddMessageFormat("[CRITICAL] CurAct out of range: %d (quest count: %d, quest ID: %d)", CurAct, qd->actList.size(), qd->questID);
		return 0;
	}

	//int obj = qd->GetKillObjective(CurAct, CDefID);
	int obj = qd->GetObjective(CurAct, type, CDefID);
	if(obj >= 0)
	{
		//TODO: debug check, can probably remove this later
		if(obj >= 3)
		{
			g_Log.AddMessageFormat("[CRITICAL] obj out of range: %d", obj);
			return 0;
		}
		
		//Already complete, no need for additional processing.
		if(ObjComplete[obj] == 1)
			return tsize;

		ObjCounter[obj]++;
		if(ObjCounter[obj] >= qd->actList[CurAct].objective[obj].data2)
		{
			ObjComplete[obj] = 1;
			writeStr = &completeStr[0];
		}
		else
		{
			sprintf(ConvBuf, "%d of %d", ObjCounter[obj], qd->actList[CurAct].objective[obj].data2);
			//g_Log.AddMessageFormat("[DEBUG] Objective: %s", writeStr);
			//writeStr already points to this
		}
		wpos = tsize;
		wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
		wpos += PutShort(&buffer[wpos], 0); //Size
		wpos += PutInteger(&buffer[wpos], QuestID ); //Quest ID
		wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_STATUS);
		wpos += PutByte(&buffer[wpos], obj);  //Objective Index.
		wpos += PutByte(&buffer[wpos], ObjComplete[obj]); //Set to 1 if this objective is completed.
		wpos += PutStringUTF(&buffer[wpos], writeStr); //Completed text ex: 1 of 5
		PutShort(&buffer[tsize + 1], wpos - tsize - 3);
		tsize += (wpos - tsize);

		QuestScript::QuestNutPlayer* player = g_QuestNutManager.GetActiveScript(CID, qd->questID);

		// Run on_objective_incr functions for the quest script
		if(player != NULL) {
			Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_objective_incr_%d_%d",CurAct,obj);
			player->JumpToLabel(ConvBuf);
		}

		if(ObjComplete[obj] == 1)
		{
			RunObjectiveCompleteScripts(CID, CurAct, obj);

			if(CheckCompletedAct(&qd->actList[CurAct]) == 1)
				tsize += AdvanceAct(CID, &buffer[tsize], qd);
		}
	}
	return tsize;
}

void QuestReference :: RunObjectiveCompleteScripts(int CID, int act, int obj)
{
	QuestScript::QuestNutPlayer* player = g_QuestNutManager.GetActiveScript(CID, QuestID);
	// Run on_objective_complete functions for the quest script
	char ConvBuf[256];
	if(player != NULL) {
		Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_objective_complete_%d_%d",act,obj);
		player->JumpToLabel(ConvBuf);
	}
}

int QuestReference :: CheckCompletedAct(QuestAct *defAct)
{
	//Check if all objectives are complete in the currect act.
	int reqCount = 0;
	int hasCount = 0;

	for(int b = 0; b < 3; b++)
	{
		if(defAct->objective[b].type != QuestObjective::OBJECTIVE_TYPE_NONE)
		{
			reqCount++;
			if(ObjComplete[b] != 0)
				hasCount++;
		}
	}
	if(hasCount >= reqCount)
		return 1;

	return 0;
}

int QuestReference :: CheckTravelLocation(int x, int y, int z, int zone)
{
	//If a location is in range for a travel objective, return the
	//objective index.
	QuestDefinition *qdef = GetQuestPointer();
	if(qdef == NULL)
		return -1;

	for(int i = 0; i < 3; i++)
	{
		QuestObjective &qobj = qdef->actList[CurAct].objective[i];
		if(ObjComplete[i] != 0)
			continue;

		if(qobj.type == QuestObjective::OBJECTIVE_TYPE_TRAVEL)
		{
			if(qobj.data1.size() < 4)
			{
				g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] CheckTravelLocation() QuestID [%d] act [%d] does not have 4 location points.", qdef->questID, CurAct);
				return -1;
			}

			int validTravelRange = qdef->actList[CurAct].objective[i].data2;

			if(zone != qobj.data1[3])
				continue;

			if(abs(x - qobj.data1[0]) > validTravelRange)
				continue;

			if(abs(y - qobj.data1[1]) > validTravelRange)
				continue;

			if(abs(z - qobj.data1[2]) > validTravelRange)
				continue;

			return i;
		}
	}
	return -1;
}

void QuestReference :: ClearObjectiveData(void)
{
	memset(ObjCounter, 0, sizeof(ObjCounter));
	memset(ObjComplete, 0, sizeof(ObjComplete));
}

int QuestReference :: AdvanceAct(int CID, char *buffer, QuestDefinition *questDef)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
	wpos += PutShort(&buffer[wpos], 0); //Size
	wpos += PutInteger(&buffer[wpos], QuestID); //Quest ID
	wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_ACTCOMPLETED);
	wpos += PutInteger(&buffer[wpos], CurAct); //act
	PutShort(&buffer[1], wpos - 3);

	CurAct++;
	if(CurAct >= questDef->actCount)
	{
		g_Log.AddMessageFormat("[ERROR] Current act exceeds count (QID: %d, Act: %d, ActCount: %d)", questDef->questID, CurAct, questDef->actCount);
		CurAct = questDef->actCount - 1;
	}
	else
	{
		ClearObjectiveData();
		//Objective[0] = 0;
		//Objective[1] = 0;
		//Objective[2] = 0;
		Complete = 0;
	}

	QuestScript::QuestNutPlayer* player = g_QuestNutManager.GetActiveScript(CID, questDef->questID);

	// Run on_advance_act functions for the quest script
	if(player != NULL) {
		char ConvBuf[128];
		Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_advance_act_%d",CurAct);
		player->JumpToLabel(ConvBuf);
	}


	//if(CurAct == questDef->actCount - 1)
	//Complete = 1;
	//g_Log.AddMessageFormat("[DEBUG] Objective set completed, new act: %d", CurAct);
	return wpos;
}

QuestDefinition* QuestReference :: GetQuestPointer(void)
{
	//Resolve a quest if it has not done so, and return a pointer to it.
	if(DefPtr == NULL)
	{
		DefPtr = QuestDef.GetQuestDefPtrByID(QuestID);
		if(DefPtr == NULL)
			g_Log.AddMessageFormat("[ERROR] GetQuestPointer quest ID [%d] not found", QuestID);
	}
	return DefPtr;
}

void QuestReference :: ResetObjectives(void)
{
	memset(ObjCounter, 0, sizeof(ObjCounter));
	memset(ObjComplete, 0, sizeof(ObjComplete));
}

void QuestReference :: Reset(void)
{
	ResetObjectives();
	Complete = 0;
	CurAct = 0;
}

bool QuestReference :: TestInvalid(void)
{
	//Check whether this reference is invalid.

	QuestDefinition *qdef = GetQuestPointer();
	if(qdef == NULL)
		return true;

	if(CurAct < 0)
		return true;
	if(CurAct >= (int)qdef->actList.size())
		return true;

	return false;
}

/* DISABLED, NEVER FINISHED
bool QuestReference :: HasItemObjective(void)
{
	for(int i = 0; i < 3; i++)
	{
		QuestObjective *qobj = &QuestDef.defList[DefIndex].actList[CurAct].objective[i];
		if(ObjComplete[i] != 0)
			continue;

		if(qobj->type == QuestObjective::OBJECT_TYPE_ITEMCOUNT)
			return true;
	}
	return false;
}
*/

/* DISABLED, NEVER FINISHED
// Search objectives for inventory item requirements, returning all items that count toward an objective.
void QueryItemObjectives(std::vector<int> &resultList)
{
	for(int i = 0; i < 3; i++)
	{
		QuestObjective *qobj = &QuestDef.defList[DefIndex].actList[CurAct].objective[i];
		if(ObjComplete[i] != 0)
			continue;
		if(qobj->type == QuestObjective::OBJECT_TYPE_ITEMCOUNT)
		{
			for(size_t di = 0; di < qobj->data1.size(); di++)
				resultList.push_back(qobj->data1[di]);
		}
	}
}
*/

QuestJournal :: QuestJournal()
{
}

QuestJournal :: ~QuestJournal()
{
	Clear();
}

void QuestJournal :: Clear(void)
{
	availableQuests.Free();
	completedQuests.Free();
	activeQuests.Free();
	availableSoonQuests.Free();
}

void QuestJournal :: AddPendingQuest(QuestReference &newItem)
{
	//Add a quest ID to the available list.  Make sure the quest isn't
	//completed, isn't active, and isn't already in the available list.
	//Because the lookups require sorted arrays, immediately sort the
	//available list after adding the new quest.

	if(completedQuests.HasQuestID(newItem.QuestID) >= 0)
		return;

	if(activeQuests.HasQuestID(newItem.QuestID) >= 0)
	{
		QuestDefinition *qdef = newItem.GetQuestPointer();
		if(qdef == NULL)
			return;

		//There is an exception for repeatable quests.
		if(qdef->Repeat == false)
			return;
	}

	if(availableQuests.HasQuestID(newItem.QuestID) >= 0)
		return;

	availableQuests.AddItem(newItem);
	availableQuests.Sort();
}

int QuestJournal :: QuestJoin_Helper(int questID)
{
	int r;

	//Must not have been completed
	//r = completedQuests.HasItem(questID);
	r = completedQuests.HasQuestID(questID);
	if(r >= 0)
		return -1;

	//Must not be active
	//r = HasQuestByQID(questID);
	r = activeQuests.HasQuestID(questID);
	if(r >= 0)
		return -1;

	//However, it must be in the available list.
	//r = availableQuests.HasItem(questID);
	r = availableQuests.HasQuestID(questID);
	if(r == -1)
		return -1;

	activeQuests.AddItem(availableQuests.itemList[r]);
	activeQuests.Sort();
	
	//Repeatable quests must not be removed from the available list.  If a
	//bounty board quest is removed from the available list, returning to
	//the bounty board will fetch a new indicator icon before the quest is
	//fully redeemed.
	bool preserve = false;
	QuestDefinition *qDef = QuestDef.GetQuestDefPtrByID(questID);
	if(qDef != NULL)
		if(qDef->Repeat == true)
			preserve = true;

	if(preserve == false)
		availableQuests.RemoveIndex(r);

	return 0;
}

int QuestJournal :: CheckQuestShare(int questID)
{
	//Check to see if the given quest can be shared with the player.

	if(completedQuests.HasQuestID(questID) >= 0)
		return SHARE_FAILED_COMPLETED;
	if(activeQuests.HasQuestID(questID) >= 0)
		return SHARE_FAILED_ACTIVE;

	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(questID);
	if(qdef == NULL)
		return SHARE_FAILED_INVALIDQUEST;

	int questReq = qdef->Requires;
	if(questReq > 0)
		if(completedQuests.HasQuestID(questReq) == -1)
			return SHARE_FAILED_REQUIRE;

	if(availableQuests.HasQuestID(questID) == -1)
		return SHARE_FAILED_AVAILABLE;

	return SHARE_SUCCESS_QUALIFIES;
}

const char * QuestJournal :: GetQuestShareErrorString(int errCode)
{
	switch(errCode)
	{
	case SHARE_FAILED_COMPLETED: return "You have already completed that quest.";
	case SHARE_FAILED_ACTIVE: return "You already have that quest.";
	case SHARE_FAILED_INVALIDQUEST: return "That quest does not exist.";
	case SHARE_FAILED_REQUIRE: return "You have not finished the requirements to unlock that quest.";
	case SHARE_FAILED_AVAILABLE: return "That quest is not available.";
	case SHARE_SUCCESS_QUALIFIES: return "That quest is available.";
	};
	return "Unknown error";
}

int QuestJournal :: CheckQuestObjective(int CID, char *buffer, int type, int CDefID)
{
	//This function is activated by the query.  Intercept an activation type
	//to determine if the character needs to process the activation.
	int tsize = 0;
	for(size_t a = 0; a < activeQuests.itemList.size(); a++)
		if(activeQuests.itemList[a].Complete == 0)
			tsize += activeQuests.itemList[a].CheckQuestObjective(CID, &buffer[tsize], type, CDefID);

	return tsize;
}

int QuestJournal :: GetCurrentAct(int questID)
{
	int r = activeQuests.HasQuestID(questID);
	if(r >= 0)
		return activeQuests.itemList[r].CurAct;

	return 0;
}

/* DISABLED, NEVER FINISHED
bool QuestJournal :: HasItemObjectives(void)
{
	for(size_t i = 0; i < activeQuests.itemList.size(); i++)
	{
		if(activeQuests.itemList[i].Complete == 0)
			if(activeQuests.itemList[i].HasItemObjective() == true)
				return true;
	}
	return false;
}
*/

/* DISABLED, NEVER FINISHED
void QuestJournal :: QueryItemObjectives(std::vector<int> &resultList)
{
	for(size_t i = 0; i < activeQuests.itemList.size(); i++)
	{
		if(activeQuests.itemList[i].Complete == 0)
		{
			if(activeQuests.itemList[i].HasItemObjective() == true)
			{
				activeQuests.itemList[i].QueryItemObjectives(resultList);
			}
		}
	}
}

int QuestJournal :: NotifyItemCount(int itemID, int itemCount, char *writeBuf)
{
}
*/



QuestItemReward :: QuestItemReward()
{
	Clear();
}
QuestItemReward :: ~QuestItemReward()
{
}

void QuestItemReward :: Clear(void)
{
	itemID = 0;
	itemCount = 0;
	required = 0;
}

void QuestItemReward :: CopyFrom(const QuestItemReward &other)
{
	itemID = other.itemID;
	itemCount = other.itemCount;
	required = other.required;
}

char * QuestItemReward :: Print(char *buffer)
{
	if(itemID == 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "id:%d count:%d required:%s", itemID, itemCount, (required == true) ? "true" : "false");
	return buffer;
}

QuestDefinition :: QuestDefinition()
{
	Clear();
}
QuestDefinition :: ~QuestDefinition()
{
	Clear();
}

void QuestDefinition :: Clear(void)
{
	profession = 0;
	questID = 0;
	title.clear();
	bodyText.clear();
	compText.clear();
	levelMin = 0;
	levelMax = 0;
	levelSuggested = 0;

	experience = 0;
	partySize = 0;
	numRewards = 0;
	coin = 0;
	unabandon = false;

	sGiver.clear();
	sEnder.clear();

	for(size_t i = 0; i < actList.size(); i++)
		actList[i].Clear();
	actCount = 0;
	actList.clear();

	for(int i = 0; i < 4; i++)
		rewardItem[i].Clear();

	Requires = 0;
	QuestGiverID = 0;
	QuestEnderID = 0;
	Repeat = false;
	RepeatMinuteDelay = 0;
	heroism = 0;

	giverX = 0;
	giverY = 0;
	giverZ = 0;
	giverZone = 0;

	mScriptAcceptCondition.Clear();
	mScriptAcceptAction.Clear();
	mScriptCompleteCondition.Clear();
	mScriptCompleteAction.Clear();

	valourGiven = 0;
	valourRequired = 0;
	guildId = 0;
	guildStart = false;
	accountQuest = false;
}

void QuestDefinition :: CopyFrom(const QuestDefinition &other)
{
	profession = other.profession;
	questID = other.questID;
	title = other.title;
	bodyText = other.bodyText;
	compText = other.compText;
	levelSuggested = other.levelSuggested;
	experience = other.experience;
	partySize = other.partySize;
	numRewards = other.numRewards;
	coin = other.coin;
	unabandon = other.unabandon;
	sGiver = other.sGiver;
	sEnder = other.sEnder;

	for(size_t i = 0; i < MAXREWARDS; i++)
		rewardItem[i].CopyFrom(other.rewardItem[i]);

	levelMin = other.levelMin;
	levelMax = other.levelMax;
	Requires = other.Requires;
	QuestGiverID = other.QuestGiverID;
	QuestEnderID = other.QuestEnderID;
	actList.assign(other.actList.begin(), other.actList.end());
	actCount = other.actCount;
	Repeat = other.Repeat;
	RepeatMinuteDelay = other.RepeatMinuteDelay;
	heroism = other.heroism;

	giverX = other.giverX;
	giverY = other.giverY;
	giverZ = other.giverZ;
	giverZone = other.giverZone;
	
	mScriptAcceptCondition.CopyFrom(other.mScriptAcceptCondition);
	mScriptAcceptAction.CopyFrom(other.mScriptAcceptAction);
	mScriptCompleteCondition.CopyFrom(other.mScriptCompleteCondition);
	mScriptCompleteAction.CopyFrom(other.mScriptCompleteAction);

	valourGiven = other.valourGiven;
	valourRequired = other.valourRequired;
	guildId = other.guildId;
	guildStart = other.guildStart;
	accountQuest = other.accountQuest;
}

int QuestDefinition :: GetObjective(unsigned int act, int type, int CDefID)
{
	//Search the objective list to see if any targets match the given ID.
	//Return a pointer to the objective definition.
	if(act >= actList.size())
	{
		g_Log.AddMessageFormat("[WARNING] GetKillObjective() act [%d] is out of range for Quest ID: %d", act, questID);
		return -1;
	}
	int a;
	for(a = 0; a < 3; a++)
	{
		int r = actList[act].objective[a].HasObjectiveCDef(type, CDefID);
		if(r >= 0)
			return a;
	}
	return -1;
}

QuestAct* QuestDefinition :: GetActPtrByIndex(int index)
{
	if(index < 0 || index >= (int)actList.size())
	{
		g_Log.AddMessageFormat("[WARNING] GetActPtrByIndex() act [%d] is out of range.", index);
		return NULL;
	}
	return &actList[index];
}

//Scan through the list of rewards, applying a list of chosen rewards by index and returning
//a list of rewards (but usually just one) that should be granted to the player.
//Return true if the filter was successful, otherwise an error occurred and the items should not
//be granted.
//Note that numRewards is only used for quests with multiple-choice reward options, and specifies
//exactly how many items must be selected by the player.
bool QuestDefinition :: FilterSelectedRewards(const std::vector<int>& selectedIndexes, std::vector<QuestItemReward>& outputRewardList)
{
	int possibleRewards = 0;
	int choiceCount = 0;
	bool add = false;

	for(size_t i = 0; i < MAXREWARDS; i++)
	{
		if(rewardItem[i].itemID == 0)
			continue;

		possibleRewards++;

		if(rewardItem[i].required == false)
		{
			for(size_t s = 0; s < selectedIndexes.size(); s++)
			{
				if(selectedIndexes[s] == i)
				{
					add = true;
					choiceCount++;
					break;
				}
			}
		}
		else
		{
			//Required, always give this reward
			add = true;
		}

		if(add == true)
		{
			outputRewardList.push_back(rewardItem[i]);
			add = false;
		}
	}

	if(numRewards > 0 && choiceCount != numRewards)
	{
		//Don't allow multiple rewards in this case, but a single reward should always be accepted,
		//otherwise the player would not be able to complete the quest.  Technically if the quest data was
		//properly set this wouldn't matter, but just in case...
		if(possibleRewards > 1)
			return false;
	}

	return true;
}

void QuestDefinition :: RunLoadDefaults(void)
{
	//Run some postprocessing defaults before the quests are loaded.
	for(size_t a = 0; a < actList.size(); a++)
	{
		for(int ob = 0; ob < 3; ob++)
		{
			if(actList[a].objective[ob].type == QuestObjective::OBJECTIVE_TYPE_ACTIVATE)
			{
				if(actList[a].objective[ob].ActivateTime == 0)
					actList[a].objective[ob].ActivateTime = QuestObjective::DEFAULT_ACTIVATE_TIME;
			}
			else if(actList[a].objective[ob].type == QuestObjective::OBJECTIVE_TYPE_TRAVEL)
			{
				if(actList[a].objective[ob].data2 == 0)
					actList[a].objective[ob].data2 = QuestObjective::DEFAULT_TRAVEL_RANGE;
			}
		}
	}

	//Prepare the minimap quest marker information by extracting from the data string.
	STRINGLIST locData;
	Util::Split(sGiver, ",", locData);
	if(locData.size() < 4)
	{
		g_Log.AddMessageFormat("[WARNING] Quest:%d has incomplete sGiver string", questID);
	}
	else
	{
		giverX = static_cast<int>(strtod(locData[0].c_str(), NULL));
		giverY = static_cast<int>(strtod(locData[1].c_str(), NULL));
		giverZ = static_cast<int>(strtod(locData[2].c_str(), NULL));
		giverZone = static_cast<int>(strtol(locData[3].c_str(), NULL, 10));
	}
}

void QuestDefinition :: RunLoadValidation(void)
{
	//Attempt to find some potential errors in the quest data that could cause problems in
	//operational logic.  Output warnings to notify which information needs fixing.

	//Check NPC giver/ender references to make sure the creature definitions exist 
	CreatureDefinition* cdef = NULL;
	cdef = CreatureDef.GetPointerByCDef(QuestGiverID);
	if(cdef == NULL)
		g_Log.AddMessageFormat("[WARNING] Quest:%d references unknown creature def [%d] for QuestGiverID", questID, QuestGiverID);
		
	cdef = CreatureDef.GetPointerByCDef(QuestEnderID);
	if(cdef == NULL)
		g_Log.AddMessageFormat("[WARNING] Quest:%d references unknown creature def [%d] for QuestEnderID", questID, QuestEnderID);
	

	//Check the rewards to make sure they're properly matched for correct player selection.
	int optionalRewards = 0;
	for(int i = 0; i < QuestDefinition::MAXREWARDS; i++)
	{
		if(rewardItem[i].itemID != 0)
		{
			if(rewardItem[i].required == false)
				optionalRewards++;
		}
	}

	if(numRewards > 0)
	{
		if(optionalRewards == 0)
			g_Log.AddMessageFormat("[WARNING] Quest:%d numRewards is set, but no optional items are defined.", questID);
		else if(optionalRewards == 1)
			g_Log.AddMessageFormat("[WARNING] Quest:%d numRewards is set, but only one item is defined (requires player selection for implicit reward)", questID);
	}
	else
	{
		if(optionalRewards > 0)
			g_Log.AddMessageFormat("[WARNING] Quest:%d has optional rewards but numRewards is not set.", questID);
	}

	
	//Verify whether the quest ender is correctly set with the objective.
	if(actList.size() > 0)
	{
		int talkTarget = 0;
		QuestAct *act = &actList[actList.size() - 1];
		for(int ob = 0; ob < 3; ob++)
		{
			if(act->objective[ob].type != QuestObjective::OBJECTIVE_TYPE_TALK)
				continue;
			talkTarget = act->objective[ob].myCreatureDefID;
			if(talkTarget != QuestEnderID)
				g_Log.AddMessageFormat("[WARNING] Quest:%d QuestEnderID does not match a talk objective", questID);
		}
		if(talkTarget == 0)
			g_Log.AddMessageFormat("[WARNING] Quest:%d does not have a final talk objective to redeem it.", questID);
	}
	else
	{
		g_Log.AddMessageFormat("[WARNING] Quest:%d no acts are defined.", questID);
	}
}

// Expects a string containing a format where the first character indicates the
// the time unit, and the remaining characters the amount of time.
// For example "m60" means 60 minutes, and "h1" means 1 hour, both returning the same
// amount of time.
// This has been put in a separate function for ease of use in case I want to make this
// a more generalized utility function in the future.
unsigned long QuestDefinition :: CalculateMinutesFromString(const char *format)
{
	if(format == NULL)
		return 0;

	size_t len = strlen(format);
	if(len == 0)
		return 0;
	
	size_t offset = 1;
	int multiplier = 1;
	switch(format[0])
	{
	case 'm': case 'M': multiplier = 1; break;
	case 'h': case 'H': multiplier = 60; break;
	case 'd': case 'D': multiplier = 1440; break;
	default: offset = 0; break;
	}
	if(offset >= len)
		return 0;
	return atoi(&format[offset]) * multiplier;
}

void QuestDefinition :: SetRepeatTime(const char *format)
{
	RepeatMinuteDelay = CalculateMinutesFromString(format);
}

QuestDefinitionContainer :: QuestDefinitionContainer()
{
}

QuestDefinitionContainer :: ~QuestDefinitionContainer()
{
	Clear();
}

void QuestDefinitionContainer :: Clear(void)
{
	mQuests.clear();
}

void QuestDefinitionContainer :: LoadQuestPackages(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open Quest list file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			Platform::FixPaths(lfr.DataBuffer);
			LoadFromFile(lfr.DataBuffer);
		}
	}
	lfr.CloseCurrent();
}

void QuestDefinitionContainer :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Could not open quest definition file [%s]", filename);
		return;
	}

	QuestDefinition newItem;
	QuestAct *curAct = NULL;
	bool firstAct = true;
	string *LastLoadString = NULL;
	lfr.CommentStyle = Comment_Slash;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		r = lfr.BreakUntil(".=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, Case_Upper);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				AddIfValid(newItem);
				newItem.actList.push_back(QuestAct());
				curAct = &newItem.actList.back();
				newItem.actCount = 1;
				firstAct = true;
				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "REQUIRES") == 0)
				newItem.Requires = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "PROFESSION") == 0)
				newItem.profession = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
				newItem.questID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "TITLE") == 0)
				AppendString(newItem.title, lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "BODYTEXT") == 0)
			{
				AppendString(newItem.bodyText, lfr.BlockToStringC(1, 0));
				LastLoadString = &newItem.bodyText;
			}
			else if(strcmp(lfr.SecBuffer, "COMPLETETEXT") == 0)
			{
				AppendString(newItem.compText, lfr.BlockToStringC(1, 0));
				LastLoadString = &newItem.compText;
			}
			else if(strcmp(lfr.SecBuffer, "LEVEL") == 0)
			{
				lfr.MultiBreak("=,");
				newItem.levelMin = lfr.BlockToIntC(1);
				newItem.levelMax = lfr.BlockToIntC(2);

				//Since the suggested level is used by the client, set a default if not
				//explicitly set.
				if(newItem.levelSuggested == 0)
					newItem.levelSuggested = newItem.levelMin;

				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "SUGGESTED") == 0)
			{
				newItem.levelSuggested = lfr.BlockToIntC(1);
				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "GUILDSTART") == 0)
			{
				newItem.guildStart = lfr.BlockToIntC(1) == 1;
				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "VALOURGIVEN") == 0)
			{
				newItem.valourGiven = lfr.BlockToIntC(1);
				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "VALOURREQUIRED") == 0)
			{
				newItem.valourRequired = lfr.BlockToIntC(1);
				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "GUILDID") == 0)
			{
				newItem.guildId = lfr.BlockToIntC(1);
				LastLoadString = NULL;
			}
			else if(strcmp(lfr.SecBuffer, "EXP") == 0)
				newItem.experience = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "PARTYSIZE") == 0)
				newItem.partySize = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "NUMREWARDS") == 0)
				newItem.numRewards = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "COIN") == 0)
				newItem.coin = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "UNABANDON") == 0)
				newItem.unabandon = lfr.BlockToBool(1);
			else if(strcmp(lfr.SecBuffer, "QUESTGIVERID") == 0)
				newItem.QuestGiverID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "QUESTENDERID") == 0)
				newItem.QuestEnderID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "REPEAT") == 0)
				newItem.Repeat = lfr.BlockToBoolC(1);
			else if(strcmp(lfr.SecBuffer, "ACCOUNTQUEST") == 0)
				newItem.accountQuest = lfr.BlockToBoolC(1);
			else if(strcmp(lfr.SecBuffer, "REPEATDELAY") == 0)
				newItem.SetRepeatTime(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "SGIVER") == 0)
				newItem.sGiver = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "SENDER") == 0)
				newItem.sEnder = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "HEROISM") == 0)
				newItem.heroism = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "[ACT]") == 0)
			{
				if(firstAct == true)
				{
					firstAct = false;
				}
				else
				{
					newItem.actList.push_back(QuestAct());
					curAct = &newItem.actList.back();
					newItem.actCount++;
				}
			}
			else if(strcmp(lfr.SecBuffer, "ACT") == 0)
			{
				lfr.BlockToStringC(1, Case_Upper);
				if(strcmp(lfr.SecBuffer, "BODYTEXT") == 0)
				{
					AppendString(curAct->BodyText, lfr.BlockToStringC(2, 0));
					LastLoadString = &curAct->BodyText;
				}
			}
			else if(strcmp(lfr.SecBuffer, "OBJ") == 0)
			{
				LastLoadString = NULL;
				int index = lfr.BlockToIntC(1);
				if(LimitIndex(index, 2) == true)
					g_Log.AddMessageFormat("[WARNING] Quest data Obj index is limited to 0-2 (line %d)", lfr.LineNumber);
				lfr.BlockToStringC(2, Case_Upper);
				if(strcmp(lfr.SecBuffer, "TYPE") == 0)
				{
					curAct->objective[index].type = GetTypeByName(lfr.BlockToStringC(3, Case_Upper));
					if(curAct->objective[index].type == 0)
						g_Log.AddMessageFormatW(MSG_DIAG, "[WARNING] Unidentified quest objective type [%s] (line %d)", lfr.BlockToStringC(3, 0), lfr.LineNumber);

					//Hack for gather types, after gather behavior was separated from
					//interact.  Gather and interact are functionally equivalent in
					//terms of objective verification and completion, but gathered
					//objects are deleted when used.  To avoid modifying the
					//rest of the code, just set a new flag here.
					if(curAct->objective[index].type == QuestObjective::OBJECTIVE_TYPE_GATHER)
					{
						curAct->objective[index].type = QuestObjective::OBJECTIVE_TYPE_ACTIVATE;
						curAct->objective[index].gather = true;
					}
				}
				else if(strcmp(lfr.SecBuffer, "DATA1") == 0)
				{
					int r = lfr.MultiBreak(".=,");
					for(int i = 3; i < r; i++)
						curAct->objective[index].data1.push_back(lfr.BlockToIntC(i));
				}
				else if(strcmp(lfr.SecBuffer, "DATA2") == 0)
					curAct->objective[index].data2 = lfr.BlockToIntC(3);
				else if(strcmp(lfr.SecBuffer, "ACTIVATETIME") == 0)
					curAct->objective[index].ActivateTime = lfr.BlockToIntC(3);
				else if(strcmp(lfr.SecBuffer, "ACTIVATETEXT") == 0)
					curAct->objective[index].ActivateText = lfr.BlockToStringC(3, 0);
				else if(strcmp(lfr.SecBuffer, "DESCRIPTION") == 0)
					curAct->objective[index].description = lfr.BlockToStringC(3, 0);
				else if(strcmp(lfr.SecBuffer, "COMPLETE") == 0)
					curAct->objective[index].complete = lfr.BlockToIntC(3);
				else if(strcmp(lfr.SecBuffer, "MYCREATUREDEFID") == 0)
					curAct->objective[index].myCreatureDefID = lfr.BlockToIntC(3);
				else if(strcmp(lfr.SecBuffer, "MYITEMID") == 0)
					curAct->objective[index].myItemID = lfr.BlockToIntC(3);
				else if(strcmp(lfr.SecBuffer, "COMPLETETEXT") == 0)
					curAct->objective[index].completeText = lfr.BlockToStringC(3, 0);
				else if(strcmp(lfr.SecBuffer, "MARKERLOCATIONS") == 0)
					curAct->objective[index].markerLocations = lfr.BlockToStringC(3, 0);
				else
					g_Log.AddMessageFormat("[WARNING] Unidentified quest Objective tag [%s] (line %d)", lfr.SecBuffer, lfr.LineNumber);
			}
			else if(strcmp(lfr.SecBuffer, "REWARDITEM") == 0)
			{
				int index = lfr.BlockToIntC(1);
				if(LimitIndex(index, 3) == true)
					g_Log.AddMessageFormat("[WARNING] Quest RewardItem index is limited to 0-3 (line %d)", lfr.LineNumber);
				lfr.MultiBreak(".=,");
				newItem.rewardItem[index].itemID = lfr.BlockToIntC(2);
				newItem.rewardItem[index].itemCount = lfr.BlockToIntC(3);
				newItem.rewardItem[index].required = lfr.BlockToBool(4);
			}
			else if(strcmp(lfr.SecBuffer, "SCRIPTACCEPTCONDITION") == 0)
				newItem.mScriptAcceptCondition.AddLine(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "SCRIPTACCEPTACTION") == 0)
				newItem.mScriptAcceptAction.AddLine(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "SCRIPTCOMPLETECONDITION") == 0)
				newItem.mScriptCompleteCondition.AddLine(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "SCRIPTCOMPLETEACTION") == 0)
				newItem.mScriptCompleteAction.AddLine(lfr.BlockToStringC(1, 0));
			else if(LastLoadString != NULL)
			{
				AppendString(*LastLoadString, lfr.DataBuffer);
			}
			else
				g_Log.AddMessageFormat("[WARNING] Unidentified quest information tag [%s] (line %d)", lfr.SecBuffer, lfr.LineNumber);
		}
	}
	AddIfValid(newItem);
	lfr.CloseCurrent();
}

void QuestDefinitionContainer :: AddIfValid(QuestDefinition &newItem)
{
	if(newItem.questID == 0)
		return;

	newItem.RunLoadDefaults();
	newItem.RunLoadValidation();

	mQuests[newItem.questID].CopyFrom(newItem);
	newItem.Clear();
}

bool QuestDefinitionContainer :: LimitIndex(int &value, int max)
{
	if(value < 0)
	{
		value = 0;
		return true;
	}
	if(value > max)
	{
		value = max;
		return true;
	}
	return false;
}

void QuestDefinitionContainer :: AppendString(string &value, char *appendStr)
{
	if(value.size() > 0)
		value.append("\r\n");
	value.append(appendStr);
}

int QuestDefinitionContainer :: GetTypeByName(char *name)
{
	const static char *TypeName[7] = {
		"NONE", "KILL", "TRAVEL", "ACTIVATE", "GATHER", "TALK", "EMOTE",
	};
	const static int TypeVar[7] = {
		QuestObjective::OBJECTIVE_TYPE_NONE,
		QuestObjective::OBJECTIVE_TYPE_KILL,
		QuestObjective::OBJECTIVE_TYPE_TRAVEL,
		QuestObjective::OBJECTIVE_TYPE_ACTIVATE,
		QuestObjective::OBJECTIVE_TYPE_GATHER,
		QuestObjective::OBJECTIVE_TYPE_TALK,
		QuestObjective::OBJECTIVE_TYPE_EMOTE
	};

	for(int a = 1; a < 7; a++)
		if(strcmp(name, TypeName[a]) == 0)
			return TypeVar[a];

	return TypeVar[0];
}

QuestDefinition* QuestDefinitionContainer :: GetQuestDefPtrByName(const char *name)
{
	//TODO: not thread safe?
	ITERATOR it;
	for(it = mQuests.begin(); it != mQuests.end(); ++it)
		if(strcmp(it->second.title.c_str(), name) == 0)
			return &it->second;
	return NULL;
}

QuestDefinition* QuestDefinitionContainer :: GetQuestDefPtrByID(int id)
{
	ITERATOR it = mQuests.find(id);
	if(it == mQuests.end())
		return NULL;
	return &it->second;
}

void QuestDefinitionContainer :: ResolveQuestMarkers(void)
{
	//QuestMarker marker;
	ITERATOR it;
	for(it = mQuests.begin(); it != mQuests.end(); ++it)
	{
		QuestDefinition *qd = &it->second;

		//Extra bit of verification we'll toss here since it's a convenient place to trap some logic errors in the quest data.
		int possibleRewards = 0;
		int optional = 0;
		for(int i = 0; i < QuestDefinition::MAXREWARDS; i++)
		{
			if(qd->rewardItem[i].itemID != 0)
			{
				possibleRewards++;
				if(qd->rewardItem[i].required == false)
					optional++;
			}
		}
		if(qd->numRewards > 0 && possibleRewards == 0)
			g_Log.AddMessageFormat("[WARNING] quest numRewards exists, but no items for Quest ID: %d", qd->questID);
		if(qd->numRewards > 0 && optional <= 1)
			g_Log.AddMessageFormat("[WARNING] quest numRewards mismatch for Quest ID: %d", qd->questID);

		if(qd->actCount > 0)
		{
			QuestAct *act = &qd->actList[qd->actList.size() - 1];
			for(int ob = 0; ob < 3; ob++)
			{
				if(act->objective[ob].type != QuestObjective::OBJECTIVE_TYPE_TALK)
					continue;
				if(act->objective[ob].myCreatureDefID != qd->QuestEnderID)
					g_Log.AddMessageFormat("[WARNING] quest return mismatch for Quest ID: %d", qd->questID);
			}
		}

		STRINGLIST locData;
		Util::Split(qd->sGiver, ",", locData);

		if(locData.size() < 4)
		{
			g_Log.AddMessageFormat("[WARNING] sGiver string incomplete for Quest ID: %d", qd->questID);
			continue;
		}
		CreatureDefinition* cdef = CreatureDef.GetPointerByCDef(qd->QuestGiverID);
		//int cindex = CreatureDef.GetIndex(qd->QuestGiverID);
		//if(cindex == -1)
		if(cdef == NULL)
		{
			g_Log.AddMessageFormat("[WARNING] QuestGiverID missing for Quest ID: %d", qd->questID);
			continue;
		}
		
		qd->giverX = (int)strtod(locData[0].c_str(), NULL);
		qd->giverY = (int)strtod(locData[1].c_str(), NULL);
		qd->giverZ = (int)strtod(locData[2].c_str(), NULL);
		qd->giverZone = strtol(locData[3].c_str(), NULL, 10);
	}
}

QuestReferenceContainer :: QuestReferenceContainer()
{
}

QuestReferenceContainer :: ~QuestReferenceContainer()
{
	Free();
}

void QuestReferenceContainer :: Free(void)
{
	itemList.clear();
}

void QuestReferenceContainer :: StartScript(CreatureInstance *instance)
{

	std::vector<QuestReference>::iterator it = itemList.begin();
	for(; it != itemList.end(); ++it) {
		g_QuestNutManager.AddActiveScript(instance, it->QuestID);
	}
}

void QuestReferenceContainer :: AddItem(int newQuestID, QuestDefinition *qdef)
{
	QuestReference newItem;
	newItem.QuestID = newQuestID;
	newItem.DefPtr = qdef;
	itemList.push_back(newItem);
	/*
	QuestReference newItem;
	newItem.QuestID = newQuestID;
	newItem.DefIndex = newDefIndex;
	itemList.push_back(newItem);
	*/
}

void QuestReferenceContainer :: AddItem(QuestReference &newItem)
{
	itemList.push_back(newItem);
}

void QuestReferenceContainer :: Sort(void)
{
	/* OBSOLETE
	std::sort(itemList.begin(), itemList.end());
	*/
}

int QuestReferenceContainer :: HasQuestID(int searchVal)
{
	for(size_t i = 0; i < itemList.size(); i++)
		if(itemList[i].QuestID == searchVal)
			return i;

	return -1;
}

int QuestReferenceContainer :: HasCreatureDef(int searchVal)
{
	for(size_t a = 0; a < itemList.size(); a++)
		if(itemList[a].CreatureDefID == searchVal)
			return a;

	return -1;
}

int QuestReferenceContainer :: HasCreatureReturn(int searchVal)
{
	//Check if the given CreatureDefID is a return point (for
	//an active quest.)
	for(size_t a = 0; a < itemList.size(); a++)
	{
		int act = itemList[a].CurAct;
		QuestDefinition *qd = itemList[a].GetQuestPointer();
		if(qd == NULL)
			continue;

		for(int b = 0; b < 3; b++)
		{
			if(qd->actList[act].objective[b].type == QuestObjective::OBJECTIVE_TYPE_TALK)
				if(qd->actList[act].objective[b].myCreatureDefID == searchVal)
					return a;
		}
	}
	return -1;
}

int QuestReferenceContainer :: HasObjectInteraction(int CreatureDefID)
{
	//Check active quests to see if the target creature is a return point
	//for a quest, or is marked for quest activation objective.
	for(size_t a = 0; a < itemList.size(); a++)
	{
		if(itemList[a].Complete == 0)
		{
			QuestDefinition *qdef = itemList[a].GetQuestPointer();
			if(qdef == NULL)
				continue;

			int act = itemList[a].CurAct;
			for(int b = 0; b < 3; b++)
			{
				if(itemList[a].ObjComplete[b] != 0)
					continue;

				if(qdef->actList[act].objective[b].type == QuestObjective::OBJECTIVE_TYPE_ACTIVATE)
				{
					int r = qdef->actList[act].objective[b].HasObjectiveCDef(QuestObjective::OBJECTIVE_TYPE_ACTIVATE, CreatureDefID);
					if(r >= 0)
						return a;
				}
			}
		}
	}
	
	return -1;
}

int QuestReferenceContainer :: GetAvailableQuestFor(int CreatureDefID, QuestJournal *questJournal)
{
	//Intended to operate on the available list only.
	//Needs to access the quest journal for completed quests.
	for(size_t i = 0; i < itemList.size(); i++)
	{
		if(itemList[i].CreatureDefID == CreatureDefID)
		{
			QuestDefinition *qdef = itemList[i].GetQuestPointer();
			if(qdef != NULL)
			{
				int reqID = qdef->Requires;
				if(reqID != 0)
				{
					if(questJournal->completedQuests.HasQuestID(reqID) >= 0)
						return i;
				}
				else
				{
					return i;
				}
			}
			else
			{
				g_Log.AddMessageFormat("[ERROR] GetAvailableQuestFor() Quest ID [%d] was not found.", itemList[i].QuestID);
			}
		}
	}
	return -1;
}

void QuestReferenceContainer :: RemoveInvalidEntries(void)
{
	int index = -1;
	do
	{
		if(index >= 0)
			RemoveIndex(index);
		index = GetInvalidEntry();
	} while(index >= 0);

	vector<QuestReference>::iterator it;
	it = std::unique(itemList.begin(), itemList.end(), QuestReference::testEquivalenceByQuestID);
	if(it != itemList.end())
	{
		int size = itemList.end() - it;
		g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] Deleted %d duplicate quest elements", size);
		itemList.erase(it, itemList.end());
	}
}


int QuestReferenceContainer :: GetInvalidEntry(void)
{
	for(size_t a = 0; a < itemList.size(); a++)
		if(itemList[a].TestInvalid() == true)
			return a;
	return -1;
}


void QuestReferenceContainer :: RemoveIndex(size_t index)
{
	if(index >= itemList.size())
		return;

	//This function assumes the list is already sorted.
	//Deleting an entry won't disrupt the order.
	itemList.erase(itemList.begin() + index);
}

void QuestReferenceContainer :: ResolveIDs(void)
{
	for(size_t i = 0; i < itemList.size(); i++)
	{
		QuestDefinition *qdef = itemList[i].GetQuestPointer();
		if(qdef != NULL)
			itemList[i].CreatureDefID = qdef->QuestGiverID;
	}
}


//TODO: move this up with the rest of the quest journal functions
const char * QuestJournal :: CreatureIsusable(int CreatureDefID)
{
	//Returns the string response for a "creature.isusable" query.
	//The simulator will enter this string into the outgoing data.
	static const char *responseStr[4] = {"N", "Y", "Q", "D"};
	//"Q" is quest-usable, and will display a shimmer effect.
	//"D" is default-usable, and will not have a shimmer effect.
	
	int r = activeQuests.HasCreatureReturn(CreatureDefID);
	if(r >= 0) {
		return responseStr[1];
	}

	r = activeQuests.HasObjectInteraction(CreatureDefID);
	if(r >= 0) {
		return responseStr[2];
	}

	return responseStr[0];
}

const char * QuestJournal :: QuestIndicator(int CreatureDefID)
{
	//Returns the string response for a "quest.indicator" query.
	//The simulator will enter this string into the outgoing data.

	int r = activeQuests.HasCreatureReturn(CreatureDefID);
	if(r >= 0)
		return QuestIndicator::QueryResponse[QuestIndicator::QUEST_INTERACT];

	//Active quests needs to be ahead of available quests because repeatable
	//quests must not be removed from the available list.  If a bounty board
	//quest is removed from the available list, returning to the bounty board
	//will fetch a new indicator icon before the quest is fully redeemed.
	r = activeQuests.HasCreatureDef(CreatureDefID);
	if(r >= 0)
		return QuestIndicator::QueryResponse[QuestIndicator::PLAYER_ON_QUEST];

	r = availableQuests.GetAvailableQuestFor(CreatureDefID, this);
	if(r >= 0)
		return QuestIndicator::QueryResponse[QuestIndicator::HAVE_QUEST];

	r = availableSoonQuests.GetAvailableQuestFor(CreatureDefID, this);
	if(r >= 0)
		return QuestIndicator::QueryResponse[QuestIndicator::WILL_HAVE_QUEST];

	return QuestIndicator::QueryResponse[QuestIndicator::NONE];
}

char * QuestJournal :: QuestGetQuestOffer(int CreatureDefID, char *convBuf)
{
	//Returns the string response for a "quest.getquestoffer" query.
	//The simulator will enter this string into the outgoing data.
	int QuestID = 0;

	for(size_t i = 0; i < availableQuests.itemList.size(); i++)
	{
		QuestDefinition *qdef = availableQuests.itemList[i].GetQuestPointer();
		if(qdef != NULL)
		{
			if(qdef->QuestGiverID != CreatureDefID)
				continue;

			//Need to check quest requirements when searching for available quests for
			//a creature.  If quest IDs did not match a sequential order, then later
			//quests in a quest line could be issued by an NPC before their prerequisite
			//quests were completed.
			if(qdef->Requires != 0)
				if(completedQuests.HasQuestID(qdef->Requires) == -1)
					continue;

			QuestID = qdef->questID;
			break;
		}
	}
	return StringFromInt(convBuf, QuestID);
}

int QuestJournal :: QuestGenericData(char *buffer, int bufsize, char *convBuf, int QuestID, int QueryIndex)
{
	//The client issues this query for the quest data after receiving the
	//quest ID from the "quest.getquestoffer" query.
	//Prepares a response buffer for the "quest.genericdata" query.
	//Returns the size of the buffer.

	//TODO: Very long quest data may cause a buffer overflow.  May want to handle
	//this more gracefully.
	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QuestID);
	if(qd == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] Quest ID [%d] not found", QuestID);
		return PrepExt_QueryResponseError(buffer, QueryIndex, "Server error: quest not found.");
	}

	int debug_check = qd->title.size() + qd->bodyText.size() + qd->compText.size();
	if(debug_check > bufsize - 100)
	{
		g_Log.AddMessageFormat("[ERROR] QUEST DATA TOO LARGE FOR BUFFER (Quest ID:%d)", QuestID);
		return PrepExt_QueryResponseError(buffer, QueryIndex, "Server error: too much data");
	}

	//Generic data has 23 rows.
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);           //Message size

	wpos += PutInteger(&buffer[wpos], QueryIndex); //Query response index

	wpos += PutShort(&buffer[wpos], 1);           //Array count
	wpos += PutByte(&buffer[wpos], 24);           //String count

	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->questID));   //[0] = Quest ID
	wpos += PutStringUTF(&buffer[wpos], qd->title.c_str());   //[1] = Title
	wpos += PutStringUTF(&buffer[wpos], qd->bodyText.c_str());   //[2] = body
	wpos += PutStringUTF(&buffer[wpos], qd->compText.c_str());   //[3] = completion text
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->levelSuggested));   //[4] = level
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->experience));   //[5] = experience
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->partySize));   //[6] = party size
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->numRewards));   //[7] = rewards
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->coin));   //[8] = coin
	wpos += PutStringUTF(&buffer[wpos], StringFromBool(convBuf, qd->unabandon));   //[9] = unabandon
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->valourGiven)); //[10] = valour

	//3 sets of data, 3 elements each
	//  [0] = Objective text
	//  [1] = Complete: either "true" or "false"
	//  [2] = myItemID
	//Spans Rows: {11, 12, 13}, {14, 15, 16}, {17, 18, 19}
	int a;
	for(a = 0; a < 3; a++)
	{
		wpos += PutStringUTF(&buffer[wpos], qd->actList[0].objective[a].description.c_str());
		wpos += PutStringUTF(&buffer[wpos], StringFromBool(convBuf, qd->actList[0].objective[a].complete));
		wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->actList[0].objective[a].myItemID));
	}

	for(a = 0; a < 4; a++)
		wpos += PutStringUTF(&buffer[wpos], qd->rewardItem[a].Print(convBuf));


	//Spans Rows: {20, 21, 22, 23}
	PutShort(&buffer[1], wpos - 3);               //Set message size
	return wpos;
}

int QuestJournal :: QuestData(char *buffer, char *convBuf, int QuestID, int QueryIndex)
{
	//The client issues the "quest.data" query when a quest is accepted, or when
	//objectives are refreshed.
	//Returns the size of the buffer.

	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QuestID);
	if(qd == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] Quest ID [%d] not found", QuestID);
		return PrepExt_QueryResponseError(buffer, QueryIndex, "Server error: quest not found.");
	}

	int act = GetCurrentAct(QuestID);
	if(act >= (int)qd->actList.size())
		return PrepExt_QueryResponseError(buffer, QueryIndex, "Server error: quest act does not exist");

	int QuestData = activeQuests.HasQuestID(QuestID);
	QuestReference *qref = NULL;
	if(QuestData >= 0)
		qref = &activeQuests.itemList[QuestData];

	//Full data has 34 rows.
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);           //Message size

	wpos += PutInteger(&buffer[wpos], QueryIndex);    //Query response index

	wpos += PutShort(&buffer[wpos], 1);           //Array count
	wpos += PutByte(&buffer[wpos], 35);           //String count

	/*
	if(r == -1)
	{
		g_Log.AddMessageFormat("[WARNING] Quest data not found for [%d]", questID);

		for(int a = 0; a < 34; a++)
			wpos += PutStringUTF(&buffer[wpos], "");

		PutShort(&buffer[1], wpos - 3);
		return wpos;
	}*/

	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->questID));   //[0] = Quest ID
	wpos += PutStringUTF(&buffer[wpos], qd->title.c_str());   //[1] = Title

	//Updated: The body text changes as you complete acts.
	//FIXED: it used to be a reference (string&) and was overwriting the body text.
	string *bodyText = &qd->bodyText;
	if(qd->actList[act].BodyText.size() > 0)
		bodyText = &qd->actList[act].BodyText;
	wpos += PutStringUTF(&buffer[wpos], bodyText->c_str());   //[2] = body

	wpos += PutStringUTF(&buffer[wpos], qd->compText.c_str());   //[3] = completion text
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->levelSuggested));   //[4] = level
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->experience));   //[5] = experience
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->partySize));   //[6] = party size
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->numRewards));   //[7] = rewards
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->coin));   //[8] = coin
	wpos += PutStringUTF(&buffer[wpos], StringFromBool(convBuf, qd->unabandon));   //[9] = unabandon
	wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->valourGiven));   //[10] = valour

	/*
	sprintf(ConvBuf, "%g,%g,%g,%d", qd->sGiver.x, qd->sGiver.y, qd->sGiver.z, qd->sGiver.zone);
	wpos += PutStringUTF(&buffer[wpos], ConvBuf);   //[10] = giver

	sprintf(ConvBuf, "%g,%g,%g,%d", qd->sEnder.x, qd->sEnder.y, qd->sEnder.z, qd->sEnder.zone);
	wpos += PutStringUTF(&buffer[wpos], ConvBuf);   //[11] = ender
	*/
	wpos += PutStringUTF(&buffer[wpos], qd->sGiver.c_str());  //[11]
	wpos += PutStringUTF(&buffer[wpos], qd->sEnder.c_str());  //[12]

	//3 sets of data, 6 elements each
	// [13]   i+0  description.  If not empty, get the rest.
	// [14]   i+1  complete ("true", "false" ?)
	// [15]   i+2  myCreatureDefID
	// [16]   i+3  myItemID
	// [17]   i+4  completeText
	// [18]   i+5  markerLocations  "x,y,z,zone;x,y,z,zone;..."

	//Spans Rows: {13, 14, 15, 16, 17, 18}
	//            {19, 20, 21, 22, 23, 24}
	//            {25, 26, 27, 28, 29, 30}

	int a;
	for(a = 0; a < 3; a++)
	{
		wpos += PutStringUTF(&buffer[wpos], qd->actList[act].objective[a].description.c_str());

		//TODO: probably need to enforce updated objectives at all time
		int complete = qd->actList[act].objective[a].complete;
		int count = 0;
		if(qref != NULL)
		{
			complete = qref->ObjComplete[a];
			count = qref->ObjCounter[a];
		}

		wpos += PutStringUTF(&buffer[wpos], StringFromBool(convBuf, complete));
		wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->actList[act].objective[a].myCreatureDefID));
		wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qd->actList[act].objective[a].myItemID));

		//Check for updated objectives.
		convBuf[0] = 0;
		if(complete == 0)
		{
			if(qref != NULL)
			{
				if(qd->actList[act].objective[a].completeText.find(" of ") != string::npos)
				{
					if(qd->actList[act].objective[a].type == QuestObjective::OBJECTIVE_TYPE_ACTIVATE || qd->actList[act].objective[a].type == QuestObjective::OBJECTIVE_TYPE_KILL)
					{
						int need = qd->actList[act].objective[a].data2;
						int have = qref->ObjCounter[a];
						sprintf(convBuf, "%d of %d", have, need);
					}
				}
			}
		}
		else
		{
			strcpy(convBuf, "Complete");
		}
		if(convBuf[0] == 0)
			wpos += PutStringUTF(&buffer[wpos], qd->actList[act].objective[a].completeText.c_str());
		else
			wpos += PutStringUTF(&buffer[wpos], convBuf);

		wpos += PutStringUTF(&buffer[wpos], qd->actList[act].objective[a].markerLocations.c_str());
	}

	// {31, 32, 33, 34}
	for(a = 0; a < 4; a++)
		wpos += PutStringUTF(&buffer[wpos], qd->rewardItem[a].Print(convBuf));

	PutShort(&buffer[1], wpos - 3);               //Set message size
	return wpos;
}

int QuestJournal :: QuestJoin(char *buffer, int QuestID, int QueryIndex)
{
	int r = QuestJoin_Helper(QuestID);

	/*
	// TODO: Hack for 0.8.9.  Something with the protocol isn't correct, since it
	// sends two join requests.  Click once and the window stays open, click a second
	// time and quest accept window closes, but the quest is already in the active list.
	if(r == -1 && g_ProtocolVersion >= 38)
		r = activeQuests.HasQuestID(QuestID);
		*/

	int wpos = PrepExt_QueryResponseString(buffer, QueryIndex, "OK");
	if(r == 0)
	{
		wpos += WriteQuestJoin(&buffer[wpos], QuestID);
	}
	else
	{
		wpos += PrepExt_SendInfoMessage(&buffer[wpos], "Unable to join quest.", INFOMSG_ERROR);
	}

	return wpos;
}

int QuestJournal :: WriteQuestJoin(char *buffer, int questID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
	wpos += PutShort(&buffer[wpos], 0); //Size
	wpos += PutInteger(&buffer[wpos], questID ); //Quest ID
	wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_JOINED);
	PutShort(&buffer[1], wpos - 3);
	return wpos;
}

int QuestJournal :: QuestList(char *buffer, char *convBuf, int QueryID)
{
	//Fill a "quest.list" query with the appropriate response data.
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);           //Message size

	wpos += PutInteger(&buffer[wpos], QueryID);

	int count = activeQuests.itemList.size();
	wpos += PutShort(&buffer[wpos], count);
	if(count > 0)
	{
		for(int a = 0; a < count; a++)
		{
			int qid = activeQuests.itemList[a].QuestID;
			QuestDefinition *qdef = activeQuests.itemList[a].GetQuestPointer();
			if(qdef == NULL)
			{
				g_Log.AddMessageFormat("[WARNING] QuestList() Unknown active quest ID [%d]", qid);
				wpos += PutByte(&buffer[wpos], 3);           //String count
				for(int b = 0; b < 3; b++)
					wpos += PutStringUTF(&buffer[wpos], "");
			}
			else
			{
				wpos += PutByte(&buffer[wpos], 3);           //String count
				wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qid));
				wpos += PutStringUTF(&buffer[wpos], qdef->title.c_str());
				wpos += PutStringUTF(&buffer[wpos], StringFromInt(convBuf, qdef->partySize));
			}
		}
	}

	PutShort(&buffer[1], wpos - 3);               //Set message size
	return wpos;
}

int QuestJournal :: QuestGetCompleteQuest_Helper(int creatureDefID)
{
	int r = activeQuests.HasCreatureReturn(creatureDefID);
	if(r == -1)
		return -1;

	int retval = activeQuests.itemList[r].QuestID;
	return retval;
}



int QuestJournal :: QuestGetCompleteQuest(char *buffer, char *convBuf, int CreatureDefID, int QueryIndex)
{
	int qid = QuestGetCompleteQuest_Helper(CreatureDefID);
	int wpos = 0;
	if(qid >= 0)
	{
		wpos += PrepExt_QueryResponseString(&buffer[wpos], QueryIndex, StringFromInt(convBuf, qid));
		//wpos += CheckQuestTalk(&buffer[wpos], CreatureDefID);
	}
	else
	{
		wpos = PrepExt_QueryResponseError(buffer, QueryIndex, "Unable to redeem quest.");
		//wpos = PrepExt_QueryResponseString(&buffer[wpos], QueryIndex, "0");
	}
	return wpos;
}

int QuestJournal :: QuestComplete(int QuestID)
{
	int r = activeQuests.HasQuestID(QuestID);
	if(r == -1)
		return -1;

	g_CharacterManager.GetThread("QuestJournal::_QuestComplete");

	QuestDefinition *qdef = activeQuests.itemList[r].GetQuestPointer();
	if(qdef == NULL)
		return -1;

	if(qdef->Repeat == false)
	{
		completedQuests.AddItem(activeQuests.itemList[r]);
		completedQuests.Sort();

		//Special case repeatable quests are sent to a repeat list where the quest is removed from the
		//completed list and put back in the available list after a certain amount of time has passed.
		//The player will not receive the quest in real-time when it is available, they will need to
		//relog.  This was the easiest way to add special event quests which should remain completed
		//for some time, but eventually freed so the quest can be completed again next year.
		//We don't mess with the explicit Repeat=true logic because that breaks stuff elsewhere, the
		//quest will still show as available to the player in game but they won't be able to activate it.
		if(qdef->RepeatMinuteDelay > 0)
		{
			AddQuestRepeatDelay(qdef->questID, 0, qdef->RepeatMinuteDelay);
		}
	}
	else
	{
		//Repeatable quests (typically bounty boards) remain in the available lists.
	}
	activeQuests.RemoveIndex(r);

	g_CharacterManager.ReleaseThread();

	return 1;
}

QuestObjective * QuestJournal :: CreatureUse(int CreatureDefID, int &QuestID, int &CurrentAct)
{
	size_t a, b;
	for(b = 0; b < activeQuests.itemList.size(); b++)
	{
		QuestDefinition *qdef = activeQuests.itemList[b].GetQuestPointer();
		if(qdef == NULL)
			continue;

		int act = activeQuests.itemList[b].CurAct;
		for(a = 0; a < 3; a++)
		{
			// "creature.use" is only sent for gather or quest redeeming, so
			// this should be safe from kill objectives.
			QuestObjective *qo = &qdef->actList[act].objective[a];
			if(qo->HasObjectiveCDef(qo->type, CreatureDefID) >= 0)
			{
				QuestID = qdef->questID;
				CurrentAct = act;
				return qo;
			}
		}
	}
	return NULL;
}

int QuestJournal :: CreatureUse_Confirmed(int CID, char *buffer, int CreatureDefID)
{
	return CheckQuestObjective(CID, buffer, QuestObjective::OBJECTIVE_TYPE_ACTIVATE, CreatureDefID);
}

int QuestJournal :: CheckTravelLocations(int CID, char *buffer, int x, int y, int z, int zone)
{
	int wpos = 0;
	for(size_t i = 0; i < activeQuests.itemList.size(); i++)
	{
		QuestReference &qr = activeQuests.itemList[i];
		QuestDefinition *qdef = activeQuests.itemList[i].GetQuestPointer();
		if(qdef == NULL)
			continue;

		int r = qr.CheckTravelLocation(x, y, z, zone);
		if(r >= 0)
		{
			qr.ObjComplete[r] = 1;
			qr.ObjCounter[r] = 1;

			std::string resultText;
			std::string *resPtr = &resultText;
			wpos += PrepExt_QuestStatusMessage(&buffer[wpos], qdef->questID, r, true, "Complete");
			if(qdef->actList[qr.CurAct].objective[r].ActivateText.size() > 0)
				resPtr = &qdef->actList[qr.CurAct].objective[r].ActivateText;
			else
			{
				resultText = "Objective complete: ";
				resultText.append(qdef->actList[qr.CurAct].objective[r].description);
				resPtr = &resultText;
			}
			wpos += PrepExt_SendInfoMessage(&buffer[wpos], resPtr->c_str(), INFOMSG_INFO);

			// Run on_objective_complete functions for the quest script
			std::list<QuestScript::QuestNutPlayer*> l = g_QuestNutManager.GetActiveQuestScripts(qr.QuestID);
			if(l.size() > 0) {
				char ConvBuf[256];
				Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_objective_complete_%d_%d",qr.CurAct,r);
				for(std::list<QuestScript::QuestNutPlayer*>::iterator it = l.begin() ; it != l.end(); ++it) {
					QuestScript::QuestNutPlayer *player = *it;
					player->JumpToLabel(ConvBuf);
				}
			}

			qr.RunObjectiveCompleteScripts(CID, qr.CurAct, r);

			if(qr.CheckCompletedAct(&qdef->actList[qr.CurAct]) == 1)
				wpos += qr.AdvanceAct(CID, &buffer[wpos], qdef);
		}
	}
	return wpos;
}

int QuestJournal :: CheckQuestTalk(char *buffer, int CreatureDefID, int CreatureInstID)
{
	//Resolve which quest has the talk objective.
	int activeIndex = activeQuests.HasCreatureReturn(CreatureDefID);
	if(activeIndex == -1)
	{
		g_Log.AddMessageFormat("[ERROR] CheckQuestTalk() No active quest has a return point for CDef [%d]", CreatureDefID);
		return 0;
	}
	QuestReference &questRef = activeQuests.itemList[activeIndex];

	//Resolve the quest definition.
	QuestDefinition *qdef = questRef.GetQuestPointer();
	if(qdef == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] CheckQuestTalk() Quest could not be resolved: (ID: %d).", questRef.QuestID);
		return 0;
	}
	//Resolve the objective.
	int objective = -1;
	for(int a = 0; a < 3; a++)
	{
		if(qdef->actList[questRef.CurAct].objective[a].type == QuestObjective::OBJECTIVE_TYPE_TALK)
		{
			if(qdef->actList[questRef.CurAct].objective[a].myCreatureDefID == CreatureDefID)
			{
				objective = a;
				break;
			}
		}
	}
	if(objective == -1)
	{
		g_Log.AddMessageFormat("[ERROR] CheckQuestTalk() Objective index could not be resolved.");
		return 0;
	}
	questRef.ObjComplete[objective] = 1;
	questRef.ObjCounter[objective] = 1;

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
	wpos += PutShort(&buffer[wpos], 0); //Size
	wpos += PutInteger(&buffer[wpos], qdef->questID); //Quest ID
	wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_STATUS);
	wpos += PutByte(&buffer[wpos], objective);
	wpos += PutByte(&buffer[wpos], 1);  //Completed, value in questRef.ObjComplete[objective]
	wpos += PutStringUTF(&buffer[wpos], "Complete");
	PutShort(&buffer[1], wpos - 3);

	if(questRef.CheckCompletedAct(&qdef->actList[questRef.CurAct]) == 1)
	{
		//Send a quest complete message instead of act complete.

		int tpos = wpos;
		wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
		wpos += PutShort(&buffer[wpos], 0); //Size
		wpos += PutInteger(&buffer[wpos], qdef->questID); //Quest ID
		wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_QUESTCOMPLETED);
		PutShort(&buffer[tpos + 1], wpos - tpos - 3);

		tpos = wpos;
		wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
		wpos += PutShort(&buffer[wpos], 0); //Size
		wpos += PutInteger(&buffer[wpos], qdef->questID); //Quest ID
		wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_JOURNAL);
		wpos += PutInteger(&buffer[wpos], CreatureInstID); //Quest ID
		PutShort(&buffer[tpos + 1], wpos - tpos - 3);

		//wpos += questRef.AdvanceAct(&buffer[wpos], &questDef);
	}

	//Just to make sure the act doesn't extend beyond the array size
	//By talking to an NPC, the quest is done anyway.
	if(questRef.CurAct > qdef->actCount - 1)
		questRef.CurAct = qdef->actCount - 1;

	return wpos;
}

int QuestJournal :: ForceComplete(int CID, int QuestID, char *buffer)
{
	//Cheat to instantly complete the current act of all quests.
	int wpos = 0;
	for(size_t a = 0; a < activeQuests.itemList.size(); a++)
	{
		QuestReference &qref = activeQuests.itemList[a];
		if(QuestID == -1 || qref.QuestID == QuestID) {
			QuestDefinition *qdef = qref.GetQuestPointer();
			if(qdef == NULL)
				continue;
			int act = qref.CurAct;
			for(int b = 0; b < 3; b++)
			{
				int count = 0;
				switch(qdef->actList[act].objective[b].type)
				{
				case QuestObjective::OBJECTIVE_TYPE_ACTIVATE:
				case QuestObjective::OBJECTIVE_TYPE_KILL:
					count = qdef->actList[act].objective[b].data2;
					break;
				case QuestObjective::OBJECTIVE_TYPE_TALK:
					count = 1;
					break;
				case QuestObjective::OBJECTIVE_TYPE_TRAVEL:
					count = 1;
					break;
				default:
					count = 0;
				}

				if(count > 0)
				{
					qref.ObjComplete[b] = 1;
					qref.ObjCounter[b] = count;

					qref.RunObjectiveCompleteScripts(CID, act, b);

					int tpos = wpos;
					wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
					wpos += PutShort(&buffer[wpos], 0); //Size
					wpos += PutInteger(&buffer[wpos], qdef->questID); //Quest ID
					wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_STATUS);
					wpos += PutByte(&buffer[wpos], b);
					wpos += PutByte(&buffer[wpos], 1);
					wpos += PutStringUTF(&buffer[wpos], "Complete");
					PutShort(&buffer[tpos + 1], wpos - tpos - 3);

					wpos += qref.AdvanceAct(CID, &buffer[wpos], qdef);
					if(qref.CurAct > qdef->actCount - 1)
						qref.CurAct = qdef->actCount - 1;
				}
			}
		}
	}
	return wpos;
}

int QuestJournal :: ForceAllComplete(int CID, char *buffer)
{
	return ForceComplete(CID, -1, buffer);
}

void QuestJournal :: QuestLeave(int CID, int QuestID)
{
	int r = activeQuests.HasQuestID(QuestID);
	if(r >= 0)
	{

		activeQuests.itemList[r].Reset();

		QuestDefinition *qDef = QuestDef.GetQuestDefPtrByID(QuestID);
		if(qDef != NULL)
		{
			if(qDef->Repeat == false)
			{
				availableQuests.AddItem(activeQuests.itemList[r]);
				availableQuests.Sort();
			}
		}
		activeQuests.RemoveIndex(r);

		QuestScript::QuestNutPlayer * player = g_QuestNutManager.GetActiveScript(CID, QuestID);
		if(player != NULL) {
			player->RunFunction("on_leave", std::vector<ScriptCore::ScriptParam>(), false);
			player->HaltExecution();
		}
	}
}

void QuestJournal :: QuestResetObjectives(int CID, int QuestID)
{
	int r = activeQuests.HasQuestID(QuestID);
	if(r >= 0)
	{
		activeQuests.itemList[r].ResetObjectives();
	}
}

void QuestJournal :: QuestClear(int CID, int QuestID)
{
	QuestLeave(CID, QuestID);
	int r = completedQuests.HasQuestID(QuestID);
	if(r >= 0)
	{
		completedQuests.itemList[r].Reset();
		QuestDefinition *qDef = QuestDef.GetQuestDefPtrByID(QuestID);
		if(qDef != NULL)
		{
			availableQuests.AddItem(completedQuests.itemList[r]);
			availableQuests.Sort();
		}
		completedQuests.RemoveIndex(r);
	}
}

int QuestJournal :: FilterEmote(int CID, char *outbuf, const char *message, int xpos, int zpos, int zoneID)
{
	int wpos = 0;
	for(size_t i = 0; i < activeQuests.itemList.size(); i++)
	{
		QuestReference *qr = &activeQuests.itemList[i];
		QuestDefinition *qdef = qr->GetQuestPointer();
		if(qdef == NULL)
			continue;

		int act = qr->CurAct;
		if(act < 0 || act >= qdef->actCount)
			continue;

		for(int obj = 0; obj < 3; obj++)
		{
			QuestObjective *qo = &qdef->actList[act].objective[obj];
			if(qo->type != QuestObjective::OBJECTIVE_TYPE_EMOTE)
				continue;

			if(Util::CaseInsensitiveStringCompare(qo->ActivateText, message))
				continue;
			if(qo->data1.size() < 4)
			{
				g_Log.AddMessageFormat("[WARNING] Not enough objective data for QID:%d, Act:%d, Obj:%d", qdef->questID, act, obj);
				continue;
			}
			if(zoneID != qo->data1[3])
				continue;
			if(abs(xpos - qo->data1[0]) > qo->data2)
				continue;
			if(abs(zpos - qo->data1[2]) > qo->data2)
				continue;

			qr->ObjComplete[obj] = 1;
			qr->ObjCounter[obj] = 1;

			qr->RunObjectiveCompleteScripts(CID, qr->CurAct,obj);

			std::string resultText;
			std::string *resPtr = &resultText;
			wpos += PrepExt_QuestStatusMessage(&outbuf[wpos], qdef->questID, obj, true, "Complete");
			resultText = "Objective complete: ";
			resultText.append(qdef->actList[act].objective[obj].description);

			wpos += PrepExt_SendInfoMessage(&outbuf[wpos], resPtr->c_str(), INFOMSG_INFO);

			if(qr->CheckCompletedAct(&qdef->actList[act]) == 1)
				wpos += qr->AdvanceAct(CID, &outbuf[wpos], qdef);
		}
	}
	return wpos;
}


void QuestJournal :: ResolveLoadedQuests(void)
{
	//Resolve the quest entries that were loaded into the Active and Completed
	//lists from the character file.
	//The goals:
	// -Make sure each quest ID exists in the master list.
	// -Make sure the current act is valid.
	// -Restore repeatable quests that are on hold from a time delay.

	activeQuests.Sort();
	completedQuests.Sort();

	activeQuests.RemoveInvalidEntries();
	completedQuests.RemoveInvalidEntries();

	//Moves completed quests back to the available list, if applicable.
	//Need to do this before the IDs are resolved or they need to relog a second time
	//for the quest to appear available again.
	CheckRepeatDelay();

	//Ideally this is not needed, and would not be if the server was new.  Check completed
	//quests and restore them if the new QuestDefinitions have repeat delays and if we're
	//missing current delay info.  That means that the formerly completed quest should be
	//restored back into the available list.
	RemoveOldRepeatQuests();

	activeQuests.ResolveIDs();
	availableQuests.ResolveIDs();
}

//Determine if the quest is complete and can be redeemed.
//The quest definition can be resolved by the calling function.  Otherwise if NULL, it will be
//resolved using the provided ID.
bool QuestJournal :: IsQuestRedeemable(QuestDefinition* questDef, int QuestID, int QuestEnderCreatureID)
{
	if(questDef == NULL)
		questDef = QuestDef.GetQuestDefPtrByID(QuestID);

	if(questDef == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] IsQuestComplete could not resolve quest: %d", QuestID);
		return false;
	}

	if(questDef->QuestEnderID != QuestEnderCreatureID)
		return false;

	int index = activeQuests.HasQuestID(QuestID);
	if(index == -1)
		return false;

	QuestReference &qr = activeQuests.itemList[index];
	QuestAct *act = questDef->GetActPtrByIndex(qr.CurAct);
	if(act == NULL)
		return false;

	if(qr.CheckCompletedAct(act) != 0)
		return true;

	return false;
}
	
// Add a new quest entry to to the delay list.  The delay list is responsible for holding when
// a completed quest should be returned to the active list.
// If the start time is zero, we create a new entry with the current server time.  Otherwise we're
// probably loading a value from a previously retrieved state.
void QuestJournal :: AddQuestRepeatDelay(int questID, unsigned long startTimeMinutes, unsigned long waitTimeMinutes)
{
	if(startTimeMinutes == 0)
		startTimeMinutes = g_PlatformTime.getAbsoluteMinutes();

	delayedRepeat.push_back(QuestRepeatDelay(questID, startTimeMinutes, waitTimeMinutes));
}

void QuestJournal :: CheckRepeatDelay(void)
{
	int questsRestored = 0;
	size_t pos = 0;
	while(pos < delayedRepeat.size())
	{
		if(delayedRepeat[pos].IsAvailable() == true)
		{
			int questID = delayedRepeat[pos].QuestID;
			QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(questID);
			if(qdef != NULL)
			{

				//If the quest is somehow already active, don't add it to the available list.
				//Otherwise add it to the available list if not already there.
				int acti = activeQuests.HasQuestID(questID);
				if(acti == -1)
				{
					int availi = availableQuests.HasQuestID(questID);
					if(availi == -1)
					{
						availableQuests.AddItem(questID, qdef);
						g_Log.AddMessageFormat("[QUEST] Resetting quest %d", questID);
					}
				}

				//Remove from completed list if there.
				int comi = completedQuests.HasQuestID(questID);
				if(comi >= 0)
					completedQuests.RemoveIndex(comi);

				questsRestored++;
			}

			delayedRepeat.erase(delayedRepeat.begin() + pos);
			//Don't increment index to scan the next.
		}
		else
		{
			pos++;
		}
	}
	//If anything was modified, sort the lists.
	if(questsRestored > 0)
	{
		availableQuests.Sort();
		completedQuests.Sort();
	}
}

// This function performs a brute force reactivation of all completed quests whose QuestDefinition has
// the new RepeatDelay enabled.  If the quest is not in the waiting repeat list, reactivate it.
void QuestJournal :: RemoveOldRepeatQuests(void)
{
	size_t pos = 0;
	while(pos < completedQuests.itemList.size())
	{
		bool delThis = false;
		
		int questID = completedQuests.itemList[pos].QuestID;
		QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(questID);
		if(qdef != NULL)
		{
			if(qdef->RepeatMinuteDelay > 0)
			{
				int repeatIndex = GetRepeatDelayIndex(questID);

				//This is a repeat quest but we have no completion data, which means it's an
				//old quest, completed before this new system, and must be reactivated.
				if(repeatIndex == -1)
					delThis = true;
			}
		}
		else
		{
			//Quest doesn't exist, might as well delete it while we're processing.
			delThis = true;
			g_Log.AddMessageFormat("[QUEST] Deleting unknown quest ID %d", questID);
		}

		if(delThis == true)
		{
			if(qdef != NULL)
			{
				//Restore the quest back into the available list.
				int acti = activeQuests.HasQuestID(questID);
				if(acti == -1)
				{
					int availi = availableQuests.HasQuestID(questID);
					if(availi == -1)
					{
						availableQuests.AddItem(questID, qdef);
						g_Log.AddMessageFormat("[QUEST] Restoring old completed quest %d", questID);
					}
				}
			}
			completedQuests.RemoveIndex(pos);
		}
		else
		{
			pos++;
		}
	}
}

// Return the index of a matching quest in the waiting repeat list.
int QuestJournal :: GetRepeatDelayIndex(int questID)
{
	for(size_t i = 0; i < delayedRepeat.size(); i++)
	{
		if(delayedRepeat[i].QuestID == questID)
			return i;
	}
	return -1;
}


int PrepExt_QuestStatusMessage(char *buffer, int questID, int objectiveIndex, bool complete, std::string message)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 7);  //_handleQuestEventMsg
	wpos += PutShort(&buffer[wpos], 0); //Size
	wpos += PutInteger(&buffer[wpos], questID); //Quest ID
	wpos += PutByte(&buffer[wpos], QuestObjective::EVENTMSG_STATUS);
	wpos += PutByte(&buffer[wpos], objectiveIndex);
	wpos += PutByte(&buffer[wpos], complete ? 1 : 0);
	wpos += PutStringUTF(&buffer[wpos], message.c_str());
	PutShort(&buffer[1], wpos - 3);
	return wpos;
}


