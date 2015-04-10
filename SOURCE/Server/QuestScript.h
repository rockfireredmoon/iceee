#pragma once
#ifndef QUESTSCRIPT_H
#define QUESTSCRIPT_H

#include "ScriptCore.h"
#include <vector>
#include <string>

class ActiveInstance;
class SimulatorThread;

namespace QuestScript
{

enum QuestScriptExtOpCodes
{
	OP_NOP = OP_MAXCOREOPCODE,  //The first instruction must begin where the default ScriptCore opcodes leave off.

	// Implemenation-Specific commands.
	OP_INFO,		  //info <text>
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

	void Clear(void);
	void TriggerFinished(void);          //Signals the script that a quest interaction has finished successfully.
	void TriggerAbort(void);             //Triggers that a script should be canceled.  

private:
	virtual void RunImplementationCommands(int opcode);
};

void LoadQuestScripts(const char *filename);
void ClearQuestScripts(void);

//namespace QuestScript
}

extern QuestScript::QuestScriptDef g_QuestScript;

#endif //#define QUESTSCRIPT_H
