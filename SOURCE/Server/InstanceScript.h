#pragma once
#ifndef INSTANCESCRIPT_H
#define INSTANCESCRIPT_H

#include "ScriptCore.h"
#include "ScriptObjects.h"
#include <vector>
#include "Callback.h"
#include "Scenery2.h"

class ActiveInstance;
class CreatureInstance;

namespace InstanceScript {

class InstanceNutDef: public ScriptCore::NutDef {

public:
	virtual ~InstanceNutDef();

	static std::string GetInstanceNutScriptPath(int zoneID, bool grove);
	static std::string GetInstanceScriptPath(int zoneID, bool pathIfNotExists, bool grove);

private:
	std::map<std::string, ScriptObjects::Area> mLocationDef;
};

class InstanceNutPlayer: public ScriptCore::NutPlayer {
public:
	std::vector<int> spawned;
	InstanceNutPlayer();
	virtual ~InstanceNutPlayer();

	static SQInteger CIDs(HSQUIRRELVM v);

	void SetInstancePointer(ActiveInstance *parent);
	virtual void RegisterFunctions();
	void RegisterInstanceFunctions(NutPlayer *instance, Sqrat::DerivedClass<InstanceNutPlayer, NutPlayer> *instanceClass);
	virtual void HaltDerivedExecution();

	// Exposed to scripts
//	void Queue(Sqrat::Function function, int fireDelay);
	void UnremoveProps();
	void UnremoveProp(int propID);
	bool RemoveProp(int propID);
	void PlaySound(const char *name);
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
	void Walk(int CID, ScriptObjects::Point point, int speed, int range);
	void WalkThen(int CID, ScriptObjects::Point point, int speed, int range, Sqrat::Function onArrival);
	int Spawn(int propID, int creatureID, int flags);
	int SpawnAt(int creatureID, float x, float y, float z, int facing, int flags);
	int GetTarget(int CDefID);
	bool SetTarget(int CDefID, int targetCDefID);
	std::vector<int> ScanForNPCByCDefID(ScriptObjects::Area *location, int CDefID);

private:
	ActiveInstance *actInst;
	std::vector<SceneryEffect> activeEffects;
	CreatureInstance* GetNPCPtr(int CID);
	void DoUnremoveProp(int propID);

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
	ScriptObjects::Area *GetLocationByName(const char *location);

private:
	std::map<std::string, ScriptObjects::Area> mLocationDef;
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

	ScriptObjects::Area* GetLocationByName(const char *name);

	//Script helper functions, often utilizing the Instance for lookups.
	void ScanNPCCID(ScriptObjects::Area *location, std::vector<int>& destResult);
	void ScanNPCCIDFor(ScriptObjects::Area *location, int CDefId,
			std::vector<int>& destResult);
	CreatureInstance* GetNPCPtr(int CID);
};


}  //namespace InstanceScript

#endif //#define INSTANCESCRIPT_H
