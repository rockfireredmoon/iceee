#pragma once
#ifndef INSTANCESCRIPT_H
#define INSTANCESCRIPT_H

#include "ScriptCore.h"
#include <vector>
#include "ScriptObjects.h"
#include "Callback.h"

class ActiveInstance;
class CreatureInstance;

namespace InstanceScript {

class InstanceNutDef: public ScriptCore::NutDef {

public:
	virtual ~InstanceNutDef();
	static std::string GetInstanceScriptPath(int zoneID, bool pathIfNotExists, bool grove);

private:
	std::map<std::string, ScriptObjects::InstanceLocation> mLocationDef;
};

class InstanceNutPlayer: public ScriptCore::NutPlayer {
public:
	std::vector<int> spawned;
	InstanceNutPlayer();
	virtual ~InstanceNutPlayer();

	static SQInteger CIDs(HSQUIRRELVM v);

	void SetInstancePointer(ActiveInstance *parent);
	void RegisterFunctions();
	virtual void HaltDerivedExecution();
	virtual void RegisterDerivedFunctions();
	void Info(const char *message);
	bool AI(int CID, const char *label);
	void Broadcast(const char *message);
	int Asset(int propID, const char *newAsset, float scale);
	int CountAlive(int CDefID);
	void DetachSceneryEffect(int propID, int tag);
	int ParticleAttach(int propID, const char *effect, float scale, float offsetX, float offsetY, float offsetZ);
	void Emote(int cid, const char *emotion);
	void Chat(const char *name, const char *channel, const char *message);
	void CreatureChat(int cid, const char *channel, const char *message);
	int CDefIDForCID(int cid);
	bool Despawn(int CID);
	int DespawnAll(int CDefID);
	int GetHealthPercent(int cid);
	void OrderWalk(int CID, ScriptObjects::Point point, int speed, int range, const char *labelName);
	int Spawn(int propID, int creatureID, int flags);
	int SpawnAt(int creatureID, float x, float y, float z, int facing, int flags);
	int GetTarget(int CDefID);
	bool SetTarget(int CDefID, int targetCDefID);
	std::vector<int> ScanForNPCByCDefID(ScriptObjects::InstanceLocation *location, int CDefID);

private:
	ActiveInstance *actInst;
	CreatureInstance* GetNPCPtr(int CID);

};

class InstanceScriptDef: public ScriptCore::ScriptDef {
public:
	ScriptObjects::InstanceLocation *GetLocationByName(const char *location);

private:
	std::map<std::string, ScriptObjects::InstanceLocation> mLocationDef;
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

	ScriptObjects::InstanceLocation* GetLocationByName(const char *name);

	//Script helper functions, often utilizing the Instance for lookups.
	void ScanNPCCID(ScriptObjects::InstanceLocation *location, std::vector<int>& destResult);
	void ScanNPCCIDFor(ScriptObjects::InstanceLocation *location, int CDefId,
			std::vector<int>& destResult);
	CreatureInstance* GetNPCPtr(int CID);
};

class OrderWalkScriptCondition {
public:
	CreatureInstance *cInst;
	TCallback<OrderWalkScriptCondition> cb;
	OrderWalkScriptCondition(CreatureInstance *creatureInstance);
	bool Execute();
};



}  //namespace InstanceScript

#endif //#define INSTANCESCRIPT_H
