#include "QuestScript.h"
#include "InstanceScript.h"
#include "StringList.h"
#include "Simulator.h"
#include "Util.h"
#include "Instance.h"
#include "Item.h"
#include "CommonTypes.h"
#include "ByteBuffer.h"
#include <algorithm>

QuestScript::QuestScriptDef g_QuestScript;
QuestScript::QuestNutManager g_QuestNutManager;

namespace QuestScript
{

//
// New script system
//

QuestNutDef::QuestNutDef(int questID)
{
	mQuestID = questID;
	char strBuf[100];
	Util::SafeFormat(strBuf, sizeof(strBuf), "QuestScripts\\%d.nut", mQuestID);
	Platform::FixPaths(strBuf);
	mSourceFile = string(strBuf);
	scriptName = std::string(Platform::Basename(mSourceFile.c_str()));
}

QuestNutDef::~QuestNutDef()
{

}
std::string QuestNutDef::GetQuestNutScriptPath() {
	return mSourceFile;
}

QuestNutManager::QuestNutManager() {
}

QuestNutManager::~QuestNutManager() {
	questDef.clear();
	questAct.clear();
}

QuestNutDef * QuestNutManager::GetScriptByID(int questID) {
	std::map<int, QuestNutDef*>::iterator it = questDef.find(questID);
	if(it == questDef.end()) {
		// Create script def
		QuestNutDef *d = new QuestNutDef(questID);
		if(Platform::FileExists(d->GetQuestNutScriptPath())) {
			questDef[questID] = d;
			return d;
		}
		else {
			/* No script for this quest, but insert a NULL map entry so
			 * we don't create a new def next time this is called. Def's
			 * can live the life of the server (or until they are
			 * edited when this feature is added)			 */

			questDef[questID] = NULL;
			return NULL;
		}

	}
	return it->second;
}

std::list<QuestNutPlayer*> QuestNutManager::GetActiveScripts(int CID)
{
	return questAct.find(CID) == questAct.end() ? std::list<QuestNutPlayer*>() : questAct[CID];
}

std::list<QuestNutPlayer*> QuestNutManager::GetActiveQuestScripts(int questID)
{
	// TODO make this faster, maintain a map keyed by quest ID
	std::list<QuestNutPlayer*> l;
	std::map<int, std::list<QuestNutPlayer*> >::iterator it;
	it = questAct.begin();
	std::list<QuestNutPlayer*>::iterator eit;
	for(; it != questAct.end(); ++ it) {
		std::list<QuestNutPlayer*> p;
		p = it->second;
		eit = p.begin();
		for(; eit != p.end(); ++ eit) {
			QuestNutPlayer *player = *eit;
			if(player->GetQuestID() == questID) {
				l.push_back(player);
			}
		}
	}
	return l;
}

QuestNutPlayer * QuestNutManager::GetActiveScript(int CID, int questID)
{

	std::list<QuestNutPlayer*> l = questAct[CID];
	list<QuestNutPlayer*>::iterator it;
	for (it = l.begin(); it != l.end(); ++it) {
		QuestNutPlayer *pl = *it;
		if(pl != NULL && pl->GetQuestID() == questID) {
			return *it;
		}
	}
	return NULL;

}

QuestNutPlayer * QuestNutManager::GetOrAddActiveScript(CreatureInstance *creature, int questID) {
	QuestNutPlayer * player = GetActiveScript(creature->CreatureID, questID);
	if(player == NULL) {
		return AddActiveScript(creature, questID);
	}
	return player;
}

QuestNutPlayer * QuestNutManager::AddActiveScript(CreatureInstance *creature, int questID) {
	QuestNutDef *def = GetScriptByID(questID);
	if (def == NULL)
		return NULL;
	cs.Enter("QuestNutManager::AddActiveScript");

	g_Log.AddMessageFormat("Compiling quest script %d", questID);
	QuestNutPlayer * player = new QuestNutPlayer();
	std::string errors;
	player->source = creature;
	player->sourceDef = &creature->charPtr->cdef;
	creature->actInst->questNutScriptList.push_back(player);

	player->Initialize(creature->actInst, def, errors);
	if (errors.length() > 0)
		g_Log.AddMessageFormat("Failed to compile %s. %s",
				def->scriptName.c_str(), errors.c_str());

	if(questAct.find(creature->CreatureID) == questAct.end()) {
		std::list<QuestNutPlayer*> l;
		l.push_back(player);
		questAct[creature->CreatureID] = l;
	}
	else {
		questAct[creature->CreatureID].push_back(player);
	}

	cs.Leave();
	return player;
}

void QuestNutManager::RemoveActiveScripts(int CID) {
	cs.Enter("QuestNutManager::RemoveActiveScript");
	g_Log.AddMessageFormat("Removing active scripts for %d", CID);
	std::list<QuestNutPlayer*> l = questAct[CID];
	for (list<QuestNutPlayer*>::iterator it = l.begin(); it != l.end(); ++it) {
		QuestNutPlayer* player = *it;
		if(player->source != NULL && player->source->actInst != NULL)
			player->source->actInst->questNutScriptList.erase(
					std::remove(player->source->actInst->questNutScriptList.begin(), player->source->actInst->questNutScriptList.end(), player), player->source->actInst->questNutScriptList.end());
		if(player->mActive)
			player->HaltExecution();
		delete (*it);
	}
	l.clear();
	questAct.erase(questAct.find(CID));
	cs.Leave();
}

void QuestNutManager::RemoveActiveScript(QuestNutPlayer *registeredPtr) {
	cs.Enter("QuestNutManager::RemoveActiveScript");
	std::list<QuestNutPlayer*> l = questAct[registeredPtr->source->CreatureID];
	list<QuestNutPlayer*>::iterator it;
	for (it = l.begin(); it != l.end(); ++it) {
		if (*it == registeredPtr) {
			QuestNutPlayer* player = *it;
			if(player->source != NULL && player->source->actInst != NULL)
				player->source->actInst->questNutScriptList.erase(
						std::remove(player->source->actInst->questNutScriptList.begin(), player->source->actInst->questNutScriptList.end(), player), player->source->actInst->questNutScriptList.end());
			g_Log.AddMessageFormat("[REMOVEME] Deleting player");
			if(player->mActive) {
				g_Log.AddMessageFormat("[WARNING!] Player is active, cannot delete");
			}
			else {
				// TODO
				//g_Log.AddMessageFormat("[WARNING!] TODO actually delete player");
				delete player;
			}
			l.erase(it);
			break;
		}
	}
	if(l.size() == 0) {
		questAct.erase(questAct.find(registeredPtr->source->CreatureID));
	}
	cs.Leave();
}

QuestNutPlayer::QuestNutPlayer()
{
	activate = Squirrel::Vector3I(0,0,0);
	source = NULL;
	sourceDef = NULL;
//	target = NULL;
	activateEvent = NULL;
	RunFlags = 0;
//	CurrentQuestAct = 0;
}

QuestNutPlayer::~QuestNutPlayer()
{
}

void QuestNutPlayer::HaltDerivedExecution(){}

void QuestNutPlayer::HaltedDerived() {
}

void QuestNutPlayer::RegisterFunctions() {

	InstanceNutPlayer::RegisterFunctions();

//	Sqrat::Class<NutPlayer> nutClass(vm, _SC("Core"), true);
//	Sqrat::RootTable(vm).Bind(_SC("Core"), nutClass);
//	RegisterCoreFunctions(this, &nutClass);

	Sqrat::DerivedClass<QuestNutPlayer, NutPlayer> questClass(vm, _SC("Quest"));
	Sqrat::RootTable(vm).Bind(_SC("Quest"), questClass);
	RegisterQuestFunctions(this, &questClass);
	Sqrat::RootTable(vm).SetInstance(_SC("quest"), this);

	Sqrat::Class<QuestObjective> questObjectiveClass(vm, "QuestObjective", true);
	questObjectiveClass.Ctor();
	questObjectiveClass.Ctor<int, std::string>();
	Sqrat::RootTable(vm).Bind(_SC("QuestObjective"), questObjectiveClass);


//	static const int OBJECTIVE_TYPE_NONE = 0;
//	static const int OBJECTIVE_TYPE_TRAVEL = 1;
//	static const int OBJECTIVE_TYPE_KILL = 2;
//	static const int OBJECTIVE_TYPE_ACTIVATE = 3;
//	static const int OBJECTIVE_TYPE_GATHER = 4;
//	static const int OBJECTIVE_TYPE_TALK = 5;
//	static const int OBJECTIVE_TYPE_EMOTE = 6;

	questObjectiveClass.Var(_SC("type"), &QuestObjective::type);
	questObjectiveClass.Var(_SC("data2"), &QuestObjective::data2);
	questObjectiveClass.Var(_SC("activate_time"), &QuestObjective::ActivateTime);
	questObjectiveClass.Var(_SC("activate_text"), &QuestObjective::ActivateText);
	questObjectiveClass.Var(_SC("description"), &QuestObjective::description);
	questObjectiveClass.Var(_SC("complete"), &QuestObjective::complete);
	questObjectiveClass.Var(_SC("my_creature_def_id"), &QuestObjective::myCreatureDefID);
	questObjectiveClass.Var(_SC("my_item_id"), &QuestObjective::myItemID);
	questObjectiveClass.Var(_SC("complete_text"), &QuestObjective::completeText);
	questObjectiveClass.Var(_SC("marker_locations"), &QuestObjective::markerLocations);
	questObjectiveClass.Var(_SC("gather"), &QuestObjective::gather);
	questObjectiveClass.Func(_SC("add_data_1"), &QuestObjective::AddData1);

	Sqrat::Class<QuestAct> questActClass(vm, "QuestAct", true);
	questActClass.Ctor();
	questActClass.Ctor<std::string, QuestObjective*>();
	Sqrat::RootTable(vm).Bind(_SC("QuestAct"), questActClass);
	questActClass.Var(_SC("body_text"), &QuestAct::BodyText);
	questActClass.Func(_SC("set_objective"), &QuestAct::AddObjective);

//	questActClass.Var("objective", &QuestAct::objective);

	// Point Object, X/Z location


	Sqrat::Class<QuestOutcome> questOutcomeClass(vm, "QuestOutcome", true);
	questOutcomeClass.Ctor();
	Sqrat::RootTable(vm).Bind(_SC("QuestOutcome"), questOutcomeClass);

	Sqrat::Class<QuestDefinition> questDefinitionClass(vm, "QuestDefinition", true);
	questDefinitionClass.Ctor();
	Sqrat::RootTable(vm).Bind(_SC("QuestDefinition"), questDefinitionClass);


	questDefinitionClass.Var(_SC("profession"), &QuestDefinition::profession);
	questDefinitionClass.Var(_SC("quest_id"), &QuestDefinition::questID);
	questDefinitionClass.Var(_SC("title"), &QuestDefinition::title);
	questDefinitionClass.Var(_SC("body_text"), &QuestDefinition::bodyText);
	questDefinitionClass.Var(_SC("level_suggested"), &QuestDefinition::levelSuggested);
	questDefinitionClass.Var(_SC("party_size"), &QuestDefinition::partySize);
	questDefinitionClass.Var(_SC("coin"), &QuestDefinition::coin);
	questDefinitionClass.Var(_SC("unabandon"), &QuestDefinition::unabandon);
	questDefinitionClass.Var(_SC("s_giver"), &QuestDefinition::sGiver);
	questDefinitionClass.Var(_SC("s_ender"), &QuestDefinition::sEnder);
	questDefinitionClass.Var(_SC("level_min"), &QuestDefinition::levelMin);
	questDefinitionClass.Var(_SC("level_max"), &QuestDefinition::levelMax);
	questDefinitionClass.Var(_SC("requires"), &QuestDefinition::Requires);
	questDefinitionClass.Var(_SC("quest_giver_id"), &QuestDefinition::QuestGiverID);
	questDefinitionClass.Var(_SC("quest_ender_id"), &QuestDefinition::QuestEnderID);
	questDefinitionClass.Var(_SC("repeat"), &QuestDefinition::Repeat);
	questDefinitionClass.Var(_SC("repeat_minute_delay"), &QuestDefinition::RepeatMinuteDelay);
	questDefinitionClass.Var(_SC("guild_start"), &QuestDefinition::guildStart);
	questDefinitionClass.Var(_SC("guild_id"), &QuestDefinition::guildId);
	questDefinitionClass.Var(_SC("valour_required"), &QuestDefinition::valourRequired);
	questDefinitionClass.Var(_SC("account_quest"), &QuestDefinition::accountQuest);
	questDefinitionClass.Func(_SC("add_act"), &QuestDefinition::AddAct);
	questDefinitionClass.Func(_SC("add_outcome"), &QuestDefinition::AddOutcome);

	questOutcomeClass.Var(_SC("num_rewards"), &QuestOutcome::numRewards);
	questOutcomeClass.Var(_SC("experience"), &QuestOutcome::experience);
	questOutcomeClass.Var(_SC("comp_text"), &QuestOutcome::compText);
	questOutcomeClass.Var(_SC("heroism"), &QuestOutcome::heroism);
	questOutcomeClass.Var(_SC("valour_given"), &QuestOutcome::valourGiven);


//	Sqrat::DerivedClass<InstanceScript::AbstractInstanceNutPlayer, NutPlayer> abstractInstanceClass(vm, _SC("AbstractInstance"));
//	Sqrat::DerivedClass<InstanceScript::InstanceNutPlayer, InstanceScript::AbstractInstanceNutPlayer> instanceClass(vm, _SC("Instance"));
//	Sqrat::RootTable(vm).Bind(_SC("Instance"), instanceClass);
//	InstanceScript::InstanceNutPlayer::RegisterInstanceFunctions(vm, &instanceClass);

//	//Note: numbers in brackets (ex: [0]) indicate which row of the outgoing query data this field occupies.
//
//		//QuestObjective objective[3];
//		//Three sections  (for(i = 12; i < 25; i += 6)
//		// [12]   i+0  string description.  If not empty, get the rest.
//		// [13]   i+1  bool complete ("true", "false" ?)
//		// [14]   i+2  int myCreatureDefID
//		// [15]   i+3  int myItemID
//		// [16]   i+4  string completeText
//		// [17]   i+5  string markerLocations  "x,y,z,zone;x,y,z,zone;..."
//		// { [18] [19] [20] [21] [22] [23] }
//		// { [24] [25] [26] [27] [28] [29] }
//
//		//[30], [31], [32], [33]
//		static const int MAXREWARDS = 4;
//		QuestItemReward rewardItem[MAXREWARDS];  //"id:# count:# required:false"
//
//		//This data is used internally by the server
//		std::vector<QuestAct> actList;
//		int actCount;
//
//
//		// This stuff is used internally for determining quest marker information.  It is extracted from the "sGiver" field and converted to numerical types here for faster processing.
//		int giverX;
//		int giverY;
//		int giverZ;
//		int giverZone;


}

void QuestNutPlayer::Initialize(ActiveInstance *actInst, QuestNutDef *defPtr, std::string &errors) {
	SetInstancePointer(actInst);
	NutPlayer::Initialize(defPtr, errors);
}

void QuestNutPlayer::RegisterQuestFunctions(NutPlayer *instance, Sqrat::DerivedClass<QuestNutPlayer, NutPlayer> *instanceClass)
{
	instanceClass->Func(_SC("get_instance"), &QuestNutPlayer::GetInstance);
	instanceClass->Func(_SC("say"), &QuestNutPlayer::Say);
	instanceClass->Func(_SC("add_quest"), &QuestNutPlayer::AddQuest);
	instanceClass->Func(_SC("talk_objective"), &QuestNutPlayer::TalkObjective);
	instanceClass->Func(_SC("kill_objective"), &QuestNutPlayer::KillObjective);
	instanceClass->Func(_SC("invite"), &QuestNutPlayer::Invite);
	instanceClass->Func(_SC("recruit_sidekick"), &QuestNutPlayer::RecruitSidekick);
	instanceClass->Func(_SC("add_sidekick"), &QuestNutPlayer::AddSidekick);
	instanceClass->Func(_SC("remove_sidekick"), &QuestNutPlayer::RemoveSidekick);
	instanceClass->Func(_SC("abandon"), &QuestNutPlayer::Abandon);
	instanceClass->Func(_SC("reset_objective"), &QuestNutPlayer::ResetObjective);
	instanceClass->Func(_SC("join"), &QuestNutPlayer::Join);
	instanceClass->Func(_SC("scatter_sidekicks"), &QuestNutPlayer::ScatterSidekicks);
	instanceClass->Func(_SC("call_sidekicks"), &QuestNutPlayer::CallSidekicks);
	instanceClass->Func(_SC("sidekicks_attack"), &QuestNutPlayer::SidekicksAttack);
	instanceClass->Func(_SC("sidekicks_defend"), &QuestNutPlayer::SidekicksDefend);
	instanceClass->Func(_SC("this_zone"), &QuestNutPlayer::ThisZone);
	instanceClass->Func(_SC("info"), &QuestNutPlayer::Info);
	instanceClass->Func(_SC("uinfo"), &QuestNutPlayer::Info);
	instanceClass->Func(_SC("effect"), &QuestNutPlayer::Effect);
	instanceClass->Func(_SC("effect_npc"), &QuestNutPlayer::EffectNPC);
	instanceClass->Func(_SC("trigger_delete"), &QuestNutPlayer::TriggerDelete);
	instanceClass->Func(_SC("despawn"), &QuestNutPlayer::Despawn);
	instanceClass->Func(_SC("get_target"), &QuestNutPlayer::GetTarget);
	instanceClass->Func(_SC("get_source"), &QuestNutPlayer::GetSource);
	instanceClass->Func(_SC("spawn_prop"), &QuestNutPlayer::SpawnProp);
	instanceClass->Func(_SC("spawn"), &QuestNutPlayer::Spawn);
	instanceClass->Func(_SC("spawn_at"), &QuestNutPlayer::SpawnAt);
	instanceClass->Func(_SC("warp_zone"), &QuestNutPlayer::WarpZone);
	instanceClass->Func(_SC("is_interacting"), &QuestNutPlayer::IsInteracting);
	instanceClass->Func(_SC("emote"), &QuestNutPlayer::Emote);
	instanceClass->Func(_SC("emote_npc"), &QuestNutPlayer::EmoteNPC);
	instanceClass->Func(_SC("heroism"), &QuestNutPlayer::Heroism);
	instanceClass->Func(_SC("has_item"), &QuestNutPlayer::HasItem);
	instanceClass->Func(_SC("has_quest"), &QuestNutPlayer::HasQuest);
	instanceClass->Func(_SC("get_transformed"), &QuestNutPlayer::GetTransformed);
	instanceClass->Func(_SC("change_heroism"), &QuestNutPlayer::ChangeHeroism);
	instanceClass->Func(_SC("remove_item"), &QuestNutPlayer::RemoveItem);
	instanceClass->Func(_SC("transform"), &QuestNutPlayer::Transform);
	instanceClass->Func(_SC("untransform"), &QuestNutPlayer::Untransform);
	instanceClass->Func(_SC("join_guild"), &QuestNutPlayer::JoinGuild);
	instanceClass->Func(_SC("chat"), &QuestNutPlayer::Chat);

}

InstanceScript::InstanceNutPlayer* QuestNutPlayer::GetInstance() {
	return source->actInst->nutScriptPlayer;
}

int QuestNutPlayer::ThisZone() {
	return source->actInst->mZone;
}

int QuestNutPlayer::Heroism() {
	return source->css.heroism;;
}

bool QuestNutPlayer::HasItem(int itemID) {
	return source->charPtr->inventory.GetItemCount(INV_CONTAINER, itemID);;
}

bool QuestNutPlayer::HasQuest(int questID) {
	return source->charPtr->questJournal.activeQuests.HasQuestID(questID) > -1;
}

int QuestNutPlayer::GetTransformed() {
	return source->IsTransformed() ? source->transformCreatureId : 0;
}

void QuestNutPlayer::Say(int CID, const char *message) {
	CreatureInstance *creature = source->actInst->GetNPCorSidekickInstanceByCID(CID);
	if(creature != NULL) {
		creature->SendSay(message);
	}
}

void QuestNutPlayer::Chat(const char *name, const char *channel, const char *message) {
	char buffer[4096];
	int wpos = PrepExt_GenericChatMessage(buffer, 0, name, channel, message);
	source->actInst->LSendToAllSimulator(buffer, wpos, -1);
}

void QuestNutPlayer::ChangeHeroism(int amount) {
	source->css.heroism += amount;
	source->OnHeroismChange();
}
void QuestNutPlayer::RemoveItem(int itemID, int itemCount) {
	char buffer[2048];
	int len = source->charPtr->inventory.RemoveItemsAndUpdate(INV_CONTAINER, itemID, itemCount, buffer);
	if(len > 0)
		source->simulatorPtr->AttemptSend(buffer, len);
}

void QuestNutPlayer::Transform(int cdefID) {
	g_Log.AddMessageFormat("Transform: %d", cdefID);
	source->CAF_Transform(cdefID, 0, -1);
}

void QuestNutPlayer::Untransform() {
	g_Log.AddMessageFormat("Untransform");
	source->CAF_Untransform();
}

void QuestNutPlayer::JoinGuild(int guildDefID) {
	GuildDefinition *gDef = g_GuildManager.GetGuildDefinition(guildDefID);
	if(gDef == NULL)
		source->simulatorPtr->SendInfoMessage("Hrmph. This guild does not exist, please report a bug!", INFOMSG_INFO);
	else {
		source->simulatorPtr->SendInfoMessage("Joining guild ..", INFOMSG_INFO);
		source->simulatorPtr->JoinGuild(gDef, 0);
		char buffer[64];
		Util::SafeFormat(buffer, sizeof(buffer), "You have joined %s", gDef->defName.c_str());
		source->simulatorPtr->SendInfoMessage(buffer, INFOMSG_INFO);
	}
}

void QuestNutPlayer::InterruptInteraction()
{
	char buf[128];
	if(activateEvent != NULL) {
		QueueRemove(activateEvent);
		activateEvent = NULL;
	}
	Util::SafeFormat(buf, sizeof(buf), "on_interrupt");
	JumpToLabel(buf);
}

void QuestNutPlayer::Info(const char *message)
{
	char Buffer[1024];
	int size = PrepExt_SendInfoMessage(Buffer, message, INFOMSG_INFO);

	if(source->PartyID == 0)
		// Just sent to player same as OP_INFO
		source->simulatorPtr->AttemptSend(Buffer, size);
	else
	{
		ActiveParty * party = g_PartyManager.GetPartyByID(source->PartyID);
		if(party == NULL)
			source->simulatorPtr->AttemptSend(Buffer, size);
		else
			for(unsigned int i = 0 ; i < party->mMemberList.size(); i++)
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(Buffer, size, party->mMemberList[i].mCreaturePtr->simulatorPtr);

	}
}

void QuestNutPlayer::UInfo(const char *message)
{
	char Buffer[1024];
	int size = PrepExt_SendInfoMessage(Buffer, message, INFOMSG_INFO);
	source->simulatorPtr->AttemptSend(Buffer, size);
}

void QuestNutPlayer::Effect(const char *effect) {
	if(source->CurrentTarget.targ != NULL) {
		char Buffer[1024];
		int size = PrepExt_SendEffect(Buffer, source->CurrentTarget.targ->CreatureID, effect, 0);
		source->actInst->LSendToAllSimulator(Buffer, size, -1);
	}
}

void QuestNutPlayer::EffectNPC(int CID, const char *effect) {
	CreatureInstance *instance = source->actInst->GetNPCInstanceByCID(CID);
	if(instance  != NULL) {
		char Buffer[1024];
		int size = PrepExt_SendEffect(Buffer, instance ->CreatureID, effect, 0);
		source->actInst->LSendToAllSimulator(Buffer, size, -1);
	}
}
void QuestNutPlayer::TriggerDelete(int CID, unsigned long ms) {
	CreatureInstance *targ;
	targ = source->actInst->GetNPCInstanceByCID(CID);
	if(targ != NULL)
	{
		targ->SetServerFlag(ServerFlags::IsUnusable, true);
		//Trigger for deletion
		if(ms != 0)
		{
			//targ->_AddStatusList(StatusEffects::DEAD, -1);
			targ->SetServerFlag(ServerFlags::TriggerDelete, true);
			targ->deathTime = g_ServerTime + ms;
		}
	}
}

QuestObjective QuestNutPlayer::TalkObjective(std::string description, int creatureDefId, std::string markerLocations) {
	QuestObjective o;
	o.type = QuestObjective::OBJECTIVE_TYPE_TALK;
	o.description = description;
	o.AddData1(0);
	o.data2 = 0;
	o.myCreatureDefID = creatureDefId;
	o.markerLocations = markerLocations;
	return o;
}

QuestObjective QuestNutPlayer::KillObjective(std::string description, Sqrat::Array &cdefIds, int amount, std::string completeText, std::string markerLocations) {
	QuestObjective o;
	o.type = QuestObjective::OBJECTIVE_TYPE_KILL;
	o.description = description;
	for (int i = 0; i < cdefIds.GetSize(); i++) {
		Sqrat::Object obj = cdefIds.GetSlot(SQInteger(i));
		o.AddData1((int)obj.Cast<unsigned long>());
	}
	o.data2 = amount;
	o.completeText = completeText;
	o.markerLocations = markerLocations;
	return o;
}

//	"Kill 6 Anubian forces");
//		obj.add_data_1(3132);
//		obj.add_data_1(3133);
//		obj.add_data_1(3140);
//		obj.add_data_1(3141);
//		obj.data2 = 6;
//		obj.complete_text = "0 of 6";
//		obj.marker_locations = "11144,501.824,11989.8,92";


bool QuestNutPlayer::Invite(int questID) {
	if(questID == GetQuestID())
		return false;
	return source->simulatorPtr->QuestInvite(questID);
}

int QuestNutPlayer::AddQuest(QuestDefinition questDefinition) {
	unsigned long questID = QuestDef.mVirtualQuestID++;
	questDefinition.questID = questID;
	SessionVarsChangeData.AddChange();
	QuestDef.AddIfValid(questDefinition);
	return questID;
}

void QuestNutPlayer::SidekicksDefend() {
	source->RemoveNoncombatantStatus("skattack");
	source->simulatorPtr->AddMessage((long) source, 0, BCM_SidekickDefend);
	source->simulatorPtr->PendingSend = true;
}

void QuestNutPlayer::SidekicksAttack() {
	source->RemoveNoncombatantStatus("skattack");
	source->simulatorPtr->AddMessage((long) source, 0, BCM_SidekickAttack);
	source->simulatorPtr->PendingSend = true;
}

void QuestNutPlayer::CallSidekicks() {
	source->simulatorPtr->AddMessage((long) source, 0, BCM_SidekickCall);
	source->simulatorPtr->PendingSend = true;
}

void QuestNutPlayer::ScatterSidekicks() {
	source->simulatorPtr->AddMessage((long) source, 0, BCM_SidekickScatter);
	source->simulatorPtr->PendingSend = true;
}

int QuestNutPlayer::RecruitSidekick(int CID, int type, int param, int hate) {

	CreatureInstance *instance = source->actInst->GetNPCInstanceByCID(CID);
	if(instance  != NULL) {
		int exist = source->charPtr->CountSidekick(type);

		SidekickObject skobj(instance->CreatureDefID);
		skobj.summonType = type;
		skobj.hateType = hate;
		skobj.summonParam = param;

	//	source->charPtr->AddSidekick(skobj);
		int r = source->actInst->CreateSidekick(source, skobj);
		if(r == -1) {
			g_Log.AddMessageFormat("Failed to add sidekick %d", instance->CreatureDefID);
			return -1;
		}
		return r;
	}
	return -1;
}

int QuestNutPlayer::AddSidekick(int cdefID, int type, int param, int hate) {
	int exist = source->charPtr->CountSidekick(type);

	SidekickObject skobj(cdefID);
	skobj.summonType = type;
	skobj.hateType = hate;

	// Parameter might be ability group ID in case of ABILITY summon type (e.g. 589)
	skobj.summonParam = param;

//	source->charPtr->AddSidekick(skobj);
	int r = source->actInst->CreateSidekick(source, skobj);
	if(r == -1) {
		g_Log.AddMessageFormat("Failed to add sidekick %d", cdefID);
		return -1;
	}
	return r;
}

int QuestNutPlayer::RemoveSidekick(int sidekickID) {
	if(sidekickID < 1 || source->actInst->SidekickRemove(source, &source->charPtr->SidekickList, sidekickID) == 1)
		return 0;
	return sidekickID;
}

bool QuestNutPlayer::Join(int questID) {
	if(questID == GetQuestID())
		return false;
	return source->simulatorPtr->QuestJoin(questID);
}

bool QuestNutPlayer::ResetObjective(int objective) {
	return source->simulatorPtr->QuestResetObjectives(((QuestNutDef*)def)->mQuestID, objective);
}

bool QuestNutPlayer::Abandon() {
	return source->simulatorPtr->QuestClear(((QuestNutDef*)def)->mQuestID);
}

int QuestNutPlayer::GetQuestID() {
	return ((QuestNutDef*)def)->mQuestID;
}

int QuestNutPlayer::GetTarget() {
	return source->CurrentTarget.targ == NULL ? -1 : source->CurrentTarget.targ->CreatureID;
}

int QuestNutPlayer::GetSource() {
	return source == NULL ? -1 : source->CreatureID;
}

void QuestNutPlayer::Despawn(int CID) {
	source->actInst->RemoveNPCInstance(CID);
}

int QuestNutPlayer::SpawnAt(int propID, int cdefID, unsigned long duration, int elevation) {
	CreatureInstance *c = source->actInst->SpawnAtProp(propID, cdefID, duration, elevation);
	return c == NULL ? -1 : c->CreatureID;
}

void QuestNutPlayer::WarpZone(int zoneID) {
	/*HaltExecution();
	source->simulatorPtr->MainCallSetZone(zoneID, 0, true);
	*/
	ClearQueue();
	source->simulatorPtr->MainCallSetZone(zoneID, 0, true);
}

bool QuestNutPlayer::IsInteracting(int cdefID) {
	return source->CurrentTarget.targ != NULL && source->CurrentTarget.targ->CreatureDefID == cdefID;
}

void QuestNutPlayer::Emote(const char *emotion) {
	char Buffer[1024];
	int size = PrepExt_GenericChatMessage(Buffer, source->CreatureID, "", "emote", emotion);
	source->actInst->LSendToAllSimulator(Buffer, size, -1);
}

void QuestNutPlayer::EmoteNPC(int CID, const char *emotion) {
	char Buffer[1024];
	int size = PrepExt_GenericChatMessage(Buffer, CID, "", "emote", emotion);
	source->actInst->LSendToAllSimulator(Buffer, size, -1);
}

//
// Old script system
//

OpCodeInfo extCoreOpCode[] = {
	// Implemenation-Specific commands.
	{ "info",          OP_INFO,         1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
	{ "uinfo",         OP_UINFO,        1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
	{ "effect",        OP_EFFECT,       1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
	{ "wait_finish",   OP_WAITFINISH,   0, {OPT_NONE,  OPT_NONE,  OPT_NONE }},
	{ "npcunusable",   OP_NPCUNUSABLE,  1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "npcremove",     OP_NPCREMOVE,    0, {OPT_NONE,  OPT_NONE,  OPT_NONE }},
	{ "require_cdef",  OP_REQUIRECDEF,  1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "spawn",         OP_SPAWN,        1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "spawn_at",      OP_SPAWNAT,      2, {OPT_INT,   OPT_INT,   OPT_NONE }},
	{ "warp_zone",     OP_WARPZONE,     1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "jmp_cdef",      OP_JMPCDEF,      2, {OPT_INT,   OPT_LABEL, OPT_NONE }},
	{ "setvar",        OP_SETVAR,       2, {OPT_INT,   OPT_INT,   OPT_NONE }},
	{ "emote",         OP_EMOTE,        1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
};
const int maxExtOpCode = COUNT_ARRAY_ELEMENTS(extCoreOpCode);

void QuestScriptDef::GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize)
{
	*arrayStart = QuestScript::extCoreOpCode;
	arraySize = QuestScript::maxExtOpCode;
}



QuestScriptPlayer::QuestScriptPlayer()
{
	Clear();
}

void QuestScriptPlayer::Clear(void)
{
	RunFlags = 0;
	sourceID = 0;
	targetID = 0;
	QuestID = 0;
	QuestAct = 0;
	targetCDef = 0;
	actInst = NULL;
	simCall = NULL;
	memset(RunTimeVar, 0, sizeof(RunTimeVar));

	activateX = 0;
	activateY = 0;
	activateZ = 0;
}

void QuestScriptPlayer::RunImplementationCommands(int opcode)
{
	ScriptCore::OpData *instr = &def->instr[curInst];
	switch(instr->opCode)
	{
	case OP_INFO:
		{
			char Buffer[1024];
			int size = PrepExt_SendInfoMessage(Buffer, def->stringList[instr->param1].c_str(), INFOMSG_INFO);

			CreatureInstance * cInst = actInst->GetInstanceByCID(sourceID);
			if(cInst == NULL || cInst->PartyID == 0)
				// Just sent to player same as OP_INFO
				simCall->AttemptSend(Buffer, size);
			else
			{
				ActiveParty * party = g_PartyManager.GetPartyByID(cInst->PartyID);
				if(party == NULL)
					simCall->AttemptSend(Buffer, size);
				else
					for(unsigned int i = 0 ; i < party->mMemberList.size(); i++)
						party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(Buffer, size, party->mMemberList[i].mCreaturePtr->simulatorPtr);

			}
		}
		break;
	case OP_UINFO:
		{
		char Buffer[1024];
		int size = PrepExt_SendInfoMessage(Buffer, def->stringList[instr->param1].c_str(), INFOMSG_INFO);
		simCall->AttemptSend(Buffer, size);
		}
		break;
	case OP_EFFECT:
		{
		char Buffer[1024];
		int size = PrepExt_SendEffect(Buffer, targetID, def->stringList[instr->param1].c_str(), 0);
		actInst->LSendToAllSimulator(Buffer, size, -1);
		}
		break;
	case OP_WAITFINISH:
		if(!(RunFlags & FLAG_FINISHED))
			advance = 0;
		break;
	case OP_NPCUNUSABLE:
		CreatureInstance *targ;
		targ = actInst->GetNPCInstanceByCID(targetID);
		if(targ != NULL)
		{
			targ->SetServerFlag(ServerFlags::IsUnusable, true);
			//Trigger for deletion
			if(def->instr[curInst].param1 != 0)
			{
				//targ->_AddStatusList(StatusEffects::DEAD, -1);
				targ->SetServerFlag(ServerFlags::TriggerDelete, true);
				targ->deathTime = g_ServerTime + def->instr[curInst].param1;
			}
		}
		break;
	case OP_NPCREMOVE:
		actInst->RemoveNPCInstance(targetID);
		break;
	case OP_REQUIRECDEF:
		if(targetCDef != instr->param1)
			mExecuting = false;
		break;
	case OP_SPAWN:
		actInst->spawnsys.TriggerSpawn(instr->param1, 0, 0);
		break;
	case OP_SPAWNAT:
		actInst->SpawnAtProp(instr->param1, instr->param2, RunTimeVar[0], RunTimeVar[1]);
		break;
	case OP_WARPZONE:
		simCall->MainCallSetZone(instr->param1, 0, true);
		break;
	case OP_JMPCDEF:
		if(targetCDef == instr->param1)
		{
			curInst = instr->param2;
			advance = 0;
		}
		break;
	case OP_SETVAR:
		int index;
		index = Util::ClipInt(instr->param1, 0, MAX_VAR - 1);
		RunTimeVar[index] = instr->param2;
		break;
	case OP_EMOTE:
		{
		char Buffer[1024];
		int size = PrepExt_GenericChatMessage(Buffer, sourceID, "", "emote", def->stringList[instr->param1].c_str());
		actInst->LSendToAllSimulator(Buffer, size, -1);
		}
		break;
	default:
		g_Log.AddMessageFormat("Unidentified op type: %d", instr->opCode);
		break;
	}
}

void QuestScriptPlayer::TriggerFinished(void)
{
	RunFlags |= FLAG_FINISHED;
}

void QuestScriptPlayer::TriggerAbort(void)
{
	if(!(RunFlags & FLAG_FINISHED))
		mExecuting = false;
}

//WarpToZoneCallback :: WarpToZoneCallback(QuestNutPlayer *nut, int zone) {
//	mNut = nut;
//	mZone = zone;
//}
//
//WarpToZoneCallback :: ~WarpToZoneCallback() {
//	mNut = NULL;
//}
//
//bool WarpToZoneCallback :: Execute() {
//	if(mNut->source != NULL && mNut->source->simulatorPtr != NULL) {
//		mNut->source->simulatorPtr->MainCallSetZone(mZone, 0, true);
//	}
//	return true;
//}


void LoadQuestScripts(const char *filename)
{
	g_QuestScript.CompileFromSource(filename);
}

void ClearQuestScripts(void)
{
	g_QuestScript.ClearBase();
}


//namespace QuestScript
}
