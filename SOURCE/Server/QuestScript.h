#pragma once
#ifndef QUESTSCRIPT_H
#define QUESTSCRIPT_H

#include "ScriptCore.h"
#include "InstanceScript.h"
#include "Creature.h"
#include "Components.h"
#include "Quest.h"
#include <vector>
#include <string>
#include <list>
#include "util/SquirrelObjects.h"
#include "sqrat.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

class ActiveInstance;
class SimulatorThread;

namespace QuestScript
{

//
// Squirrel script system
//


class QuestNutDef: public ScriptCore::NutDef {

public:
	QuestNutDef(int questID);
	int mQuestID;
	fs::path GetQuestNutScriptPath();
	virtual ~QuestNutDef();
};

class QuestNutPlayer: public InstanceScript::InstanceNutPlayer {
public:
	//Variables used by the script.
	unsigned short RunFlags;
	static const unsigned short FLAG_FINISHED = 1;
	CreatureInstance *source;
	CreatureDefinition *sourceDef;
//	CreatureInstance *target;
	ScriptCore::NutScriptEvent *activateEvent;
//	int CurrentQuestAct;
	Squirrel::Vector3I activate;
	QuestNutPlayer();
	virtual ~QuestNutPlayer();

	void Initialize(ActiveInstance *actInst, QuestNutDef *defPtr, string &errors);
	virtual void RegisterFunctions();
	void RegisterQuestFunctions(NutPlayer *instance, Sqrat::DerivedClass<QuestNutPlayer, NutPlayer> *instanceClass);
	virtual void HaltDerivedExecution();
	virtual void HaltedDerived();
	int GetQuestID();
	void InterruptInteraction();

	// Exposed to scripts

	InstanceScript::InstanceNutPlayer *GetInstance();
	int GetTarget();
	int GetSource();
	bool ResetObjective(int objective);
	bool Abandon();
	int AddQuest(QuestDefinition questDefinition);
	bool Invite(int questID);
	QuestObjective KillObjective(string description, Sqrat::Array &cdefIds, int amount, string completeText, string markerLocations);
	QuestObjective TalkObjective(string description, int creatureDefId, string markerLocations);

	void ScatterSidekicks();
	void CallSidekicks();
	void SidekicksAttack();
	void SidekicksDefend();
	int RecruitSidekick(int cid, int type, int param, int hate);
	int AddSidekick(int cdefID, int type, int param, int hate);
	int RemoveSidekick(int sidekickID);
	bool Join(int questID);
	void Say(int cid, const char *message);
	void Chat(const char *name, const char *channel, const char *message);
	void Info(const char *message);
	void UInfo(const char *message);
	void EffectNPC(int CID, const char *effect);
	void Effect(const char *effect);
	void TriggerDelete(int targetCID, unsigned long delay);
	void Despawn(int targetCID);
	int ThisZone();
	int SpawnAt(int propID, int cdefID, unsigned long duration, int elevation);
	void WarpZone(int zoneID);
	bool IsInteracting(int cdefID);
	void Emote(const char *emotion);
	void EmoteNPC(int cid, const char *emotion);
	int Heroism();
	bool HasItem(int itemID);
	bool HasQuest(int questID);
	int GetTransformed();
	void ChangeHeroism(int amount);
	void RemoveItem(int itemID, int itemCount);
	void Transform(int cdefID);
	void Untransform();
	void JoinGuild(int guildDefID);
};

//class WarpToZoneCallback : public ScriptCore::NutCallback
//{
//public:
//	QuestNutPlayer *mNut;
//	int mZone;
//	WarpToZoneCallback(QuestNutPlayer *nut, int zone);
//	~WarpToZoneCallback();
//	bool Execute();
//};

class QuestNutManager
{
public:
	QuestNutManager();
	~QuestNutManager();
	map<int, QuestNutDef*> questDef;
	map<int, list<QuestNutPlayer*> > questAct;
	list<QuestNutPlayer*> GetActiveScripts(int CID);
	list<QuestNutPlayer*> GetActiveQuestScripts(int questID);
	Platform_CriticalSection cs;
	QuestNutPlayer * GetOrAddActiveScript(CreatureInstance *creature,  int questID);
	QuestNutPlayer * GetActiveScript(int CID, int questID);
	QuestNutPlayer * AddActiveScript(CreatureInstance *creature, int questID);
	void RemoveActiveScript(QuestNutPlayer *registeredPtr);
	void RemoveActiveScripts(int CID);
private:
	QuestNutDef* GetScriptByID(int questID);
};

//
// Old script system
//

enum QuestScriptExtOpCodes
{
	OP_NOP = OP_MAXCOREOPCODE,  //The first instruction must begin where the default ScriptCore opcodes leave off.

	// Implemenation-Specific commands.
	OP_INFO,		  //info <text> (like uinfo but sends to whole party)
	OP_UINFO,		  //uinfo <text> (like info, but only send to the player, not whole party)
	OP_EFFECT,        //Plays an effect for the target object.
	OP_WAITFINISH,    //Waits for the script "Finished" flag to be true.
	OP_NPCUNUSABLE,   //Disables creature usability for the target creature linked to this script.
	OP_NPCREMOVE,     //Removes the target creature linked to this script instance.
	OP_REQUIRECDEF,   //Abort the script if the activating definition does not match.
	OP_SPAWN,         //spawn <propid>  force a spawnpoint generate a spawn
	OP_SPAWNAT,       //spawn_at <creaturedef> <propid> spawn an arbitrary creature at the location of a spawnpoint
	OP_WARPZONE,      //Force the player to warp to a zone.
	OP_JMPCDEF,       //jmp_cdef <creaturedef> <label> jump to label if the activating creature matches.
	OP_SETVAR,        //setvar <index> <value> Set a runtime variable.
	OP_EMOTE          //emote <str> run an emote
};

class QuestScriptDef : public ScriptCore::ScriptDef
{
private:
	virtual void GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize);
};

class QuestScriptPlayer : public ScriptCore::ScriptPlayer
{
public:
	QuestScriptPlayer();

	//Variables used by the script.
	unsigned short RunFlags;
	static const unsigned short FLAG_FINISHED = 1;
	int sourceID;
	int targetID;
	int targetCDef;
	int QuestID;
	int QuestAct;
	int RunTimeVar[2];
	static const int MAX_VAR = 2;
	ActiveInstance *actInst;
	SimulatorThread *simCall;

	int activateX;
	int activateY;
	int activateZ;

	void ClearDerived();
	void Clear(void);
	void TriggerFinished(void);          //Signals the script that a quest interaction has finished successfully.
	void TriggerAbort(void);             //Triggers that a script should be canceled.  

private:
	virtual void RunImplementationCommands(int opcode);
};

void LoadQuestScripts(string filename);
void ClearQuestScripts(void);

//namespace QuestScript
}


extern QuestScript::QuestNutManager g_QuestNutManager;
extern QuestScript::QuestScriptDef g_QuestScript;

#endif //#define QUESTSCRIPT_H
