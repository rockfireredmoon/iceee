#ifndef AISCRIPT_H
#define AISCRIPT_H
#include <list>

#include "ScriptCore.h"

class CreatureInstance;
class ReportBuffer;

enum AIScriptExtOpCodes
{
	OP_USE = OP_MAXCOREOPCODE,
	OP_GETWILL,
	OP_GETWILLCHARGE,
	OP_GETMIGHT,
	OP_GETMIGHTCHARGE,
	OP_HASTARGET,
	OP_GETLEVEL,
	OP_DEBUGPRINT,
	OP_GETCOOLDOWN,
	OP_ISBUSY,
	OP_COUNTENEMYNEAR,
	OP_COUNTENEMYAT,
	OP_HEALTHPERCENT,
	OP_TARGETHEALTHPERCENT,
	OP_SETELAPSEDTIME,
	OP_TIMEOFFSET,
	OP_VISUALEFFECT,
	OP_VISUALEFFECTT,
	OP_SAY,
	OP_INSTANCECALL,
	OP_GETIDLEMOB,
	OP_GETTARGET,
	OP_GETSELF,
	OP_SETOTHERTARGET,
	OP_AISCRIPTCALL,
	OP_ISTARGETENEMY,
	OP_ISTARGETFRIENDLY,
	OP_SETSPEED,
	OP_GETTARGETCDEF,
	OP_GETPROPERTY,
	OP_GETTARGETPROPERTY,
	OP_DISPELTARGETPROPERTY,
	OP_RANDOMIZE,
	OP_FINDCDEF,
	OP_PLAYSOUND,
	OP_GETBUFFTIER,
	OP_GETTARGETBUFFTIER,
	OP_TARGETINRANGE,
	OP_GETTARGETRANGE,
	OP_SETGTAE,
	OP_GETSPEED,
	OP_CIDISBUSY,
};

class AIScriptDef : public ScriptCore::ScriptDef
{
private:
	virtual void GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize);
/*
	virtual OpCodeInfo* GetInstructionData(int opcode);
	virtual OpCodeInfo* GetInstructionDataByName(const char *name);
*/
};

class AIScriptPlayer : public ScriptCore::ScriptPlayer
{
	virtual void RunImplementationCommands(int opcode);
	CreatureInstance *ResolveCreatureInstance(int CreatureInstanceID);

public:
	CreatureInstance *attachedCreature;

	void DebugGenerateReport(ReportBuffer &report);
};


class AIScriptManager
{
public:
	AIScriptManager();
	~AIScriptManager();
	std::list<AIScriptDef> aiDef;
	std::list<AIScriptPlayer> aiAct;
	int LoadScripts(void);
	AIScriptDef* GetScriptByName(const char *name);
	AIScriptPlayer *AddActiveScript(const char *name);
	void RemoveActiveScript(AIScriptPlayer *registeredPtr);
};

extern AIScriptManager aiScriptManager;
#endif
