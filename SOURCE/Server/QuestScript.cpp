#include "QuestScript.h"
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
		if(Platform::FileExists(d->GetQuestNutScriptPath().c_str())) {
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
	return questAct[CID];
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
	creature->actInst->questNutScriptList.push_back(player);

	player->Initialize(def, errors);
	if (errors.length() > 0)
		g_Log.AddMessageFormat("Failed to compile %s. %s",
				def->scriptName.c_str(), errors.c_str());

	std::list<QuestNutPlayer*> l;
	l.push_back(player);

	questAct[creature->CreatureID] = l;

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
			delete player;
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
	target = NULL;
	activateEvent = NULL;
	RunFlags = 0;
	QuestAct = 0;
}

QuestNutPlayer::~QuestNutPlayer()
{
}

void QuestNutPlayer::HaltDerivedExecution(){}

void QuestNutPlayer::HaltedDerived() {
}

void QuestNutPlayer::RegisterFunctions() {
	Sqrat::Class<NutPlayer> nutClass(vm, _SC("Core"), true);
	Sqrat::RootTable(vm).Bind(_SC("Core"), nutClass);
	RegisterCoreFunctions(this, &nutClass);
	Sqrat::DerivedClass<QuestNutPlayer, NutPlayer> questClass(vm, _SC("Quest"));
	Sqrat::RootTable(vm).Bind(_SC("Quest"), questClass);
	RegisterQuestFunctions(this, &questClass);
	Sqrat::RootTable(vm).SetInstance(_SC("quest"), this);
}

void QuestNutPlayer::RegisterQuestFunctions(NutPlayer *instance, Sqrat::DerivedClass<QuestNutPlayer, NutPlayer> *instanceClass)
{
	instanceClass->Func(_SC("add_sidekick"), &QuestNutPlayer::AddSidekick);
	instanceClass->Func(_SC("remove_sidekick"), &QuestNutPlayer::RemoveSidekick);
	instanceClass->Func(_SC("abandon"), &QuestNutPlayer::Abandon);
	instanceClass->Func(_SC("reset_objective"), &QuestNutPlayer::ResetObjective);
	instanceClass->Func(_SC("join"), &QuestNutPlayer::Join);
	instanceClass->Func(_SC("this_zone"), &QuestNutPlayer::ThisZone);
	instanceClass->Func(_SC("info"), &QuestNutPlayer::Info);
	instanceClass->Func(_SC("info"), &QuestNutPlayer::Info);
	instanceClass->Func(_SC("uinfo"), &QuestNutPlayer::Info);
	instanceClass->Func(_SC("effect"), &QuestNutPlayer::Effect);
	instanceClass->Func(_SC("trigger_delete"), &QuestNutPlayer::TriggerDelete);
	instanceClass->Func(_SC("despawn"), &QuestNutPlayer::Despawn);
	instanceClass->Func(_SC("get_target"), &QuestNutPlayer::GetTarget);
	instanceClass->Func(_SC("get_source"), &QuestNutPlayer::GetSource);
	instanceClass->Func(_SC("spawn"), &QuestNutPlayer::Spawn);
	instanceClass->Func(_SC("spawn_at"), &QuestNutPlayer::SpawnAt);
	instanceClass->Func(_SC("warp_zone"), &QuestNutPlayer::WarpZone);
	instanceClass->Func(_SC("is_interacting"), &QuestNutPlayer::IsInteracting);
	instanceClass->Func(_SC("emote"), &QuestNutPlayer::Emote);
	instanceClass->Func(_SC("heroism"), &QuestNutPlayer::Heroism);
	instanceClass->Func(_SC("has_item"), &QuestNutPlayer::HasItem);
	instanceClass->Func(_SC("has_quest"), &QuestNutPlayer::HasQuest);
	instanceClass->Func(_SC("get_transformed"), &QuestNutPlayer::GetTransformed);
	instanceClass->Func(_SC("change_heroism"), &QuestNutPlayer::ChangeHeroism);
	instanceClass->Func(_SC("remove_item"), &QuestNutPlayer::RemoveItem);
	instanceClass->Func(_SC("transform"), &QuestNutPlayer::Transform);
	instanceClass->Func(_SC("untransform"), &QuestNutPlayer::Untransform);
	instanceClass->Func(_SC("join_guild"), &QuestNutPlayer::JoinGuild);

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
	Util::SafeFormat(buf, sizeof(buf), "on_interrupt_%d", QuestAct);
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
	char Buffer[1024];
	int size = PrepExt_SendEffect(Buffer, target->CreatureID, effect, 0);
	source->actInst->LSendToAllSimulator(Buffer, size, -1);
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


int QuestNutPlayer::AddSidekick(int cdefID, bool pet) {
	int type = pet ? SidekickObject::PET : SidekickObject::QUEST;
	int exist = source->charPtr->CountSidekick(type);

	SidekickObject skobj(cdefID);
	skobj.summonType = type;
	skobj.summonParam = SIDEKICK_ABILITY_GROUP_ID;

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
	source->simulatorPtr->QuestJoin(questID);
	return true;}

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
	return target == NULL ? -1 : target->CreatureID;
}

int QuestNutPlayer::GetSource() {
	return source == NULL ? -1 : source->CreatureID;
}

void QuestNutPlayer::Despawn(int CID) {
	source->actInst->RemoveNPCInstance(CID);
}

int QuestNutPlayer::Spawn(int propID) {
	return source->actInst->spawnsys.TriggerSpawn(propID, 0, 0);
}

int QuestNutPlayer::SpawnAt(int propID, int cdefID, unsigned long duration, int elevation) {
	CreatureInstance *c = source->actInst->SpawnAtProp(propID, cdefID, duration, elevation);
	return c == NULL ? -1 : c->CreatureID;
}

void QuestNutPlayer::WarpZone(int zoneID) {
	source->simulatorPtr->MainCallSetZone(zoneID, 0, true);
}

bool QuestNutPlayer::IsInteracting(int cdefID) {
	return target != NULL && target->CreatureDefID == cdefID;
}

void QuestNutPlayer::Emote(const char *emotion) {
	char Buffer[1024];
	int size = PrepExt_GenericChatMessage(Buffer, source->CreatureID, "", "emote", emotion);
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
