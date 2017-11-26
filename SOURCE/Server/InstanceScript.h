#pragma once
#ifndef INSTANCESCRIPT_H
#define INSTANCESCRIPT_H

#include "ScriptCore.h"
#include <vector>
#include <map>
#include <algorithm>
#include "Callback.h"
#include "Scenery2.h"
#include "PartyManager.h"
#include "Forms.h"
#include "util/SquirrelObjects.h"

class ActiveInstance;
class CreatureInstance;

namespace InstanceScript {

class MonitorArea {
public:
	Squirrel::Area area;
	std::vector<int> creatureIds;
	std::string name;

	MonitorArea() {
		Clear();
	}

	~MonitorArea() {
		Clear();
	}

	void Remove(int creatureId) {
		creatureIds.erase(std::find(creatureIds.begin(), creatureIds.end(), creatureId));
	}

	bool Contains(int creatureId) {
		return std::find(creatureIds.begin(), creatureIds.end(), creatureId) != creatureIds.end();
	}

	void Clear() {
		creatureIds.clear();
	}

};

class InstanceNutDef: public ScriptCore::NutDef {

public:
	virtual ~InstanceNutDef();

	static std::string GetInstanceNutScriptPath(int zoneID, bool grove);
	static std::string GetInstanceScriptPath(int zoneID, bool pathIfNotExists, bool grove);

private:
	std::map<std::string, Squirrel::Area> mLocationDef;
};

class ActiveInteraction {
public:
	CreatureInstance *mCreature;
	ScriptCore::NutScriptEvent *mEvent;
	~ActiveInteraction();
	ActiveInteraction(CreatureInstance *mCreature, ScriptCore::NutScriptEvent *mEvent);
};

class AbstractInstanceNutPlayer: public ScriptCore::NutPlayer {
public:

	AbstractInstanceNutPlayer();
	virtual ~AbstractInstanceNutPlayer();
	void SetInstancePointer(ActiveInstance *parent);
	void RegisterAbstractInstanceFunctions(NutPlayer *instance, Sqrat::DerivedClass<AbstractInstanceNutPlayer, NutPlayer> *instanceClass);
	static SQInteger GetCreaturesNearCreature(HSQUIRRELVM v);
	static SQInteger CIDs(HSQUIRRELVM v);
	static SQInteger AllCIDs(HSQUIRRELVM v);
	static SQInteger GetHated(HSQUIRRELVM v);
	static SQInteger AllPlayers(HSQUIRRELVM v);
	int GetNPCID(int CDefID);
	void MonitorArea(std::string name, Squirrel::Area area);
	void UnmonitorArea(std::string name);
	int GetCIDForPropID(int propID);
	int GetCreatureDistance(int CID, int CID2);
	std::string GetCreatureSpawnProp(int CID);
	void PlayerMovement(CreatureInstance *creature);
	void PlayerLeft(CreatureInstance *creature);
	static int GetAbilityID(const char *name);
	void Shake(float amount, float time, float range);
	void RotateCreature(int CID, int rotation);
	bool Untarget(int CID);
	bool CreatureUse(int CID, int abilityID);
	bool CreatureUseNoRetry(int CID, int abilityID);
	bool DoCreatureUse(int CID, int abilityID, bool retry);
	void Info(const char *message);
	void Message(const char *message, int type);
	void LocalBroadcast(const char *message);
	void Broadcast(const char *message);
	void Error(const char *message);
	void Chat(const char *name, const char *channel, const char *message);
	void CreatureChat(int cid, const char *channel, const char *message);
	void SetServerFlags(int CID, unsigned long flags);
	void SetServerFlag(int CID, unsigned long flag, bool state);
	void StopAI(int CID);
	void Unhate(int CID);
	void Interrupt(int CID);
	void ClearAIQueue(int CID);
	bool IsAtTether(int CID);
	bool TargetSelf(int CID);
	bool SetEnvironment(const char *environment);
	std::string GetTimeOfDay();
	std::string GetEnvironment(int x, int y);
	void SetTimeOfDay(std::string timeOfDay);
	unsigned long GetServerFlags(int CID);
	static SQInteger Scan(HSQUIRRELVM v);
	static SQInteger ScanNPCs(HSQUIRRELVM v);
	int ScanNPC(Squirrel::Area *location, int CDefID);
	std::vector<int> ScanForNPCs(Squirrel::Area *location, int CDefID);
	std::vector<InstanceScript::MonitorArea> monitorAreas;
protected:
	CreatureInstance* GetNPCPtr(int CID);
	CreatureInstance* GetCreaturePtr(int CID);
	ActiveInstance *actInst;
	std::vector<ActiveInteraction> interactions;
};

class InstanceNutPlayer: public AbstractInstanceNutPlayer {
public:
	std::vector<int> spawned;
	std::vector<int> genericSpawned;
	std::map<int, int> openedForms;
	InstanceNutPlayer();
	virtual ~InstanceNutPlayer();
	virtual void RegisterFunctions();
	static void RegisterInstanceFunctions(HSQUIRRELVM vm, Sqrat::DerivedClass<InstanceNutPlayer, AbstractInstanceNutPlayer> *instanceClass);
	virtual void HaltDerivedExecution();
	virtual void HaltedDerived();

	// Exposed to scripts
//	void Queue(Sqrat::Function function, int fireDelay);
	int PVPStart(int type);
	bool PVPGoal(int cid);
	bool PVPStop();
	AINutPlayer* GetAI(int CID);
	bool HasStatusEffect(int CID, const char *effect);
	bool SetStatusEffect(int CID, const char *effect, long durationMS);
	bool RemoveStatusEffect(int CID, const char *effect);
	void RestoreOriginalAppearance(int CID);
	int CreateParty(int leaderCID);
	int CreateTeam(int leaderCID, int team);
	bool DisbandVirtualParty(int partyID);
	bool AddToVirtualParty(int partyID, int CID);
	int GetVirtualPartySize(int partyID);
	bool QuitParty(int CID);
	std::vector<int> GetVirtualPartyMembers(int partyID);
	int GetVirtualPartyLeader(int partyID);
	void DetachItem(int CID, const char *type, const char *node);
	void AttachItem(int CID, const char *type, const char *node);
	void UnremoveProps();
	void UnremoveProp(std::string propID);
	bool RemoveProp(std::string propID);
	void PlaySound(const char *name);
	void ClearTarget(int CID);
	bool AI(int CID, const char *label);
	int GetPartyID(int CID);
	Squirrel::Vector3I GetLocation(int CID);
	const char *GetDisplayName(int CID);
	int Transform(std::string propID, Sqrat::Table transformation);
	int Asset(std::string propID, const char *newAsset, float scale);
	int CountAlive(int CDefID);
	bool AttachSidekick(int playerCID, int sidekickCID, int summonType);
	bool InviteQuest(int CID, int questID, bool inviteParty);
	bool JoinQuest(int CID, int questID, bool joinParty);
	bool AdvanceQuest(int CID, int questID, int act, int objective, int outcome);
	void DetachSceneryEffect(int propID, int tag);
	std::string GetPropIDForSpawn(int CID);
	int ParticleAttach(std::string propID, const char *effect, float scale, float offsetX, float offsetY, float offsetZ);
	void Emote(int cid, const char *emotion);
	int CDefIDForCID(int cid);
	void SetCreatureGTAE(int CID);
	void SetCreatureGTAETo(int CID, Squirrel::Vector3I loc);
	void SetSize(int CID, float size);
	bool Despawn(int CID);
	int LoadSpawnTileFor(Squirrel::Point location);
	int LoadSpawnTile(Squirrel::Point location);
	int DespawnAll(int CDefID);
	int GetHealthPercent(int cid);
	void WarpPlayer(int CID, int zoneID);
	long Walk(int CID, Squirrel::Point point, int facing, int speed, int range);
	long WalkThen(int CID, Squirrel::Point point, int facing, int speed, int range, Sqrat::Function onArrival);
	long TempWalkThen(int CID, Squirrel::Point point, int facing, int speed, int range, Sqrat::Function onArrival);
	int SpawnProp(int propID);
	int Spawn(int propID, int creatureID, int flags);
	int SpawnAt(int creatureID, Squirrel::Vector3I location, int facing, int flags);
	int OLDSpawnAt(int creatureID, float x, float y, float z, int facing, int flags);
	int GetTarget(int CID);
	bool SetTarget(int CID, int targetCID);
	bool SetEnvironment(const char *environment);
	std::string GetEnvironment(int x, int y);
	std::string PopEnvironment();
	std::string GetTimeOfDay();
	void SetTimeOfDay(std::string timeOfDay);
	bool Interact(int CID, const char *text, int time, bool gather, Sqrat::Function function);
	void AddInteraction(CreatureInstance *creature, ScriptCore::NutScriptEvent *evt);
	void RemoveInteraction(int CID);
	void InterruptInteraction(int CID);
	bool HasItem(int CID, int itemID);
	bool GiveItem(int CID, int itemID);
	bool OpenBook(int CID, int id, int page, bool refresh);
	void OpenForm(int CID, FormDefinition form);
	void CloseForm(int CID, int formId);

private:
	std::vector<SceneryEffect> activeEffects;
	void DoUnremoveProp(std::string propID);
	ActiveParty * DoCreateParty(int leaderCID, int team);

};

class InteractCallback : public ScriptCore::NutCallback
{
public:
	InstanceNutPlayer* mNut;
	Sqrat::Function mFunction;		//Function to jump to
	int mCID;
	InteractCallback(InstanceNutPlayer *nut, Sqrat::Function function, int CID);
	~InteractCallback();
	bool Execute();
};

class InstanceUseCallback : public ScriptCore::NutCallback
{
public:
	int mAbilityID;
	int mCID;
	AbstractInstanceNutPlayer* mInstanceNut;

	InstanceUseCallback(AbstractInstanceNutPlayer *instanceNut, int CID, int abilityID);
	~InstanceUseCallback();
	bool Execute();
};

class WalkCondition : public ScriptCore::NutCondition
{
public:
	CreatureInstance *cInst;
	WalkCondition(CreatureInstance *c);
	virtual bool CheckCondition();
	void Init(ScriptCore::NutPlayer *nut, CreatureInstance *creature, Squirrel::Point point, int facing, int speed, int range, bool reset);
};


class WalkCallback : public ScriptCore::NutCallback
{
public:
	CreatureInstance *mCreature;
	std::string sPreviousPathNode;
	std::string sNextPathNode;
	int sTetherNodeX;
	int sTetherNodeZ;
	int sTetherNodeFacing;

	bool mReset;
	ScriptCore::NutPlayer* mNut;
	Sqrat::Function mFunction;		//Function to jump to

	WalkCallback(ScriptCore::NutPlayer *nut, CreatureInstance *creature, Squirrel::Point point, int facing, int speed, int range, bool mReset);
	WalkCallback(ScriptCore::NutPlayer *nut, CreatureInstance *creature, Squirrel::Point point, int facing, int speed, int range, Sqrat::Function onArrival, bool mReset);
	~WalkCallback ();
	bool Execute();
	void Reset();
private:
	void Init(ScriptCore::NutPlayer *nut, CreatureInstance *creature, Squirrel::Point point, int facing, int speed, int range, bool reset);
};

class InstanceScriptDef: public ScriptCore::ScriptDef {
public:
	static std::string GetInstanceTslScriptPath(int zoneID, bool grove);
	Squirrel::Area *GetLocationByName(const char *location);

private:
	std::map<std::string, Squirrel::Area> mLocationDef;
	virtual void GetExtendedOpCodeTable(OpCodeInfo **arrayStart,
			size_t &arraySize);
	virtual void SetMetaDataDerived(const char *opname,
			ScriptCore::ScriptCompiler &compileData);
	virtual bool HandleAdvancedCommand(const char *commandToken,
			ScriptCore::ScriptCompiler &compileData);
};

class InstanceScriptPlayer: public ScriptCore::ScriptPlayer {
public:
	void SetInstancePointer(ActiveInstance *parent);

private:
	ActiveInstance *actInst;
	virtual void RunImplementationCommands(int opcode);

	Squirrel::Area* GetLocationByName(const char *name);

	//Script helper functions, often utilizing the Instance for lookups.
	void ScanNPCCID(Squirrel::Area *location, std::vector<int>& destResult);
	void ScanNPCCIDFor(Squirrel::Area *location, int CDefId,
			std::vector<int>& destResult);
	CreatureInstance* GetNPCPtr(int CID);
};


}  //namespace InstanceScript

#endif //#define INSTANCESCRIPT_H
