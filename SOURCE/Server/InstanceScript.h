#pragma once
#ifndef INSTANCESCRIPT_H
#define INSTANCESCRIPT_H

#include "ScriptCore.h"
#include <vector>
#include <algorithm>
#include "Callback.h"
#include "Scenery2.h"
#include "PartyManager.h"
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

class AbstractInstanceNutPlayer: public ScriptCore::NutPlayer {
public:
	AbstractInstanceNutPlayer();
	virtual ~AbstractInstanceNutPlayer();
	void SetInstancePointer(ActiveInstance *parent);
	static SQInteger CIDs(HSQUIRRELVM v);
	static SQInteger GetHated(HSQUIRRELVM v);
	int GetNPCID(int CDefID);
	void MonitorArea(std::string name, Squirrel::Area area);
	void UnmonitorArea(std::string name);
	int GetCIDForPropID(int propID);
	void PlayerMovement(CreatureInstance *creature);
	void PlayerLeft(CreatureInstance *creature);
	void Info(const char *message);
	void Message(const char *message, int type);
	void LocalBroadcast(const char *message);
	void Broadcast(const char *message);
	void Error(const char *message);
	void Chat(const char *name, const char *channel, const char *message);
	void CreatureChat(int cid, const char *channel, const char *message);
	static SQInteger Scan(HSQUIRRELVM v);
	static SQInteger ScanNPCs(HSQUIRRELVM v);
	int ScanNPC(Squirrel::Area *location, int CDefID);
	std::vector<int> ScanForNPCs(Squirrel::Area *location, int CDefID);
	std::vector<InstanceScript::MonitorArea> monitorAreas;
protected:
	CreatureInstance* GetNPCPtr(int CID);
	CreatureInstance* GetCreaturePtr(int CID);
	ActiveInstance *actInst;
};

class InstanceNutPlayer: public AbstractInstanceNutPlayer {
public:
	std::vector<int> spawned;
	std::vector<int> genericSpawned;
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
	void UnremoveProp(int propID);
	bool RemoveProp(int propID);
	void PlaySound(const char *name);
	void Unhate(int CID);
	void ClearTarget(int CID);
	bool AI(int CID, const char *label);
	int GetPartyID(int CID);
	Squirrel::Vector3I GetLocation(int CID);
	const char *GetDisplayName(int CID);
	int Transform(int propID, Sqrat::Table transformation);
	int Asset(int propID, const char *newAsset, float scale);
	int CountAlive(int CDefID);
	bool AttachSidekick(int playerCID, int sidekickCID, int summonType);
	bool InviteQuest(int CID, int questID, bool inviteParty);
	bool JoinQuest(int CID, int questID, bool joinParty);
	bool AdvanceQuest(int CID, int questID, int act, int objective, int outcome);
	void DetachSceneryEffect(int propID, int tag);
	int GetPropIDForSpawn(int CID);
	int ParticleAttach(int propID, const char *effect, float scale, float offsetX, float offsetY, float offsetZ);
	void Emote(int cid, const char *emotion);
	int CDefIDForCID(int cid);
	bool Despawn(int CID);
	int LoadSpawnTileFor(Squirrel::Point location);
	int LoadSpawnTile(Squirrel::Point location);
	int DespawnAll(int CDefID);
	int GetHealthPercent(int cid);
	void Walk(int CID, Squirrel::Point point, int speed, int range);
	void WalkThen(int CID, Squirrel::Point point, int speed, int range, Sqrat::Function onArrival);
	int Spawn(int propID, int creatureID, int flags);
	int SpawnAt(int creatureID, Squirrel::Vector3I location, int facing, int flags);
	int OLDSpawnAt(int creatureID, float x, float y, float z, int facing, int flags);
	int GetTarget(int CID);
	bool SetTarget(int CID, int targetCID);

private:
	std::vector<SceneryEffect> activeEffects;
	void DoUnremoveProp(int propID);
	ActiveParty * DoCreateParty(int leaderCID, int team);

};

class WalkCondition : public ScriptCore::NutCondition
{
public:
	CreatureInstance *cInst;
	WalkCondition(CreatureInstance *c);
	virtual bool CheckCondition();
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
