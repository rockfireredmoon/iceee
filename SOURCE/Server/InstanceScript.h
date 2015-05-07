#pragma once
#ifndef INSTANCESCRIPT_H
#define INSTANCESCRIPT_H

#include "ScriptCore.h"
#include <vector>

class ActiveInstance;
class CreatureInstance;

struct InstanceLocation {
	int mX1;
	int mX2;
	int mY1;
	int mY2;
	InstanceLocation() {
		mX1 = 0;
		mX2 = 0;
		mY1 = 0;
		mY2 = 0;
	}
};

namespace InstanceScript {

class InstanceNutDef: public ScriptCore::NutDef {

public:
	virtual ~InstanceNutDef();
	static std::string GetInstanceScriptPath(int zoneID, bool pathIfNotExists, bool grove);

private:
	std::map<std::string, InstanceLocation> mLocationDef;
};

class InstanceNutPlayer: public ScriptCore::NutPlayer {
public:
	std::vector<int> spawned;
	InstanceNutPlayer();
	virtual ~InstanceNutPlayer();

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
	std::vector<int> GetAllCIDForCDefID(int CDefID);
	void Chat(const char *name, const char *channel, const char *message);
	void CreatureChat(int cid, const char *channel, const char *message);
	int CDefIDForCID(int cid);
	bool Despawn(int CID);
	int DespawnAll(int CDefID);
	int GetHealthPercent(int cid);
	void OrderWalk(int CID, float destX, float destY, int speed, int range);
	int Spawn(int propID, int creatureID, int flags);
	int SpawnAt(int creatureID, float x, float y, float z, int facing, int flags);
	int GetTarget(int CDefID);
	bool SetTarget(int CDefID, int targetCDefID);
	std::vector<int> ScanForNPCByCDefID(InstanceLocation *location, int CDefID);

private:
	ActiveInstance *actInst;
	CreatureInstance* GetNPCPtr(int CID);

};

class InstanceScriptDef: public ScriptCore::ScriptDef {
public:
	InstanceLocation *GetLocationByName(const char *location);

private:
	std::map<std::string, InstanceLocation> mLocationDef;
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

	InstanceLocation* GetLocationByName(const char *name);

	//Script helper functions, often utilizing the Instance for lookups.
	void ScanNPCCID(InstanceLocation *location, std::vector<int>& destResult);
	void ScanNPCCIDFor(InstanceLocation *location, int CDefId,
			std::vector<int>& destResult);
	CreatureInstance* GetNPCPtr(int CID);
};

}  //namespace InstanceScript

#endif //#define INSTANCESCRIPT_H
