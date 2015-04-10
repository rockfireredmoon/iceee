#pragma once
#ifndef INSTANCESCRIPT_H
#define INSTANCESCRIPT_H

#include "ScriptCore.h"

class ActiveInstance;
class CreatureInstance;

struct InstanceLocation
{
	int mX1;
	int mX2;
	int mY1;
	int mY2;
	InstanceLocation()
	{
		mX1 = 0; mX2 = 0; mY1 = 0; mY2 = 0;
	}
};

namespace InstanceScript
{

class InstanceScriptDef : public ScriptCore::ScriptDef
{
public:
	InstanceLocation *GetLocationByName(const char *location);

private:
	std::map<std::string, InstanceLocation> mLocationDef;
	virtual void GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize);
	virtual void SetMetaDataDerived(const char *opname, ScriptCore::ScriptCompiler &compileData);
	virtual bool HandleAdvancedCommand(const char *commandToken, ScriptCore::ScriptCompiler &compileData);
};

class InstanceScriptPlayer : public ScriptCore::ScriptPlayer
{
public:
	void SetInstancePointer(ActiveInstance *parent);

private:
	ActiveInstance *actInst;
	virtual void RunImplementationCommands(int opcode);

	InstanceLocation* GetLocationByName(const char *name);

	//Script helper functions, often utilizing the Instance for lookups.
	void ScanNPCCID(InstanceLocation *location, std::vector<int>& destResult);
	CreatureInstance* GetNPCPtr(int CID);
};

}  //namespace InstanceScript

#endif //#define INSTANCESCRIPT_H
