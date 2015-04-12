#include "InstanceScript.h"
#include "AIScript.h"
#include "Instance.h"
#include "CommonTypes.h"
#include "StringList.h"
#include <stdlib.h>

extern char GAuxBuf[];

namespace InstanceScript
{

enum InstanceScriptExtOpCodes
{
	OP_NOP = OP_MAXCOREOPCODE,  //The first instruction must begin where the default ScriptCore opcodes leave off.

	// Implemenation-Specific named commands.
	OP_SPAWN,         //spawn <propid>  force a spawnpoint generate a spawn
	OP_SPAWNAT,       //spawnat <propid> <creatureDef>  generate an arbitrary creature at a known spawnpoint.
	OP_SPAWNFLAG,       //spawnflag <propid> <creatureDef> <flags> generate an arbitrary creature at a known spawnpoint with specified flags.
	OP_COUNTALIVE,    //
	OP_SPAWNLOC,      //spawnloc <creaturedefid> <x> <y> <z>  Spawn a creature at exact coordinates. NOTE: x,y,z are pushed onto stack.
	OP_GETNPCID,      //get_npc_id <int_creaturedefID> <var_output>     Get the first creature matching a DefID and place it into VAR.
	OP_SETTARGET,     //set_target <var_sourceCID> <var_targetCID>   Set the CreatureID from VAR1 to target the CreatureID from VAR2

	OP_SCAN_NPC_CID,         //scan_mobs_cid <str:location> <iarray:dest>
	OP_SCAN_NPC_CID_FOR,     //scan_mobs_cid_for <str:location> <iarray:dest> <creatureDef>
	OP_GET_CDEF,
	OP_GET_HEALTH_PERCENT,   //get_health_percent <var:CID>
	OP_ORDER_WALK,           //order_walk <var:CID> <int:destX> <int:destZ>
	OP_GET_TARGET,           //get_target <var:CID> <var:dest>     
	OP_AI_SCRIPT_JUMP,      //ai_script_jump <var:CID> <str:label>
	OP_INFO,                //info <str:message> <int:globalBroadcast>
	OP_CHAT,
	OP_DESPAWN         // despawn <propid> force a despawn of a spawnpoint
};

OpCodeInfo extCoreOpCode[] = {
	// Implementation-Specific commands.
	{ "spawn",            OP_SPAWN,           1, {OPT_INT,  OPT_NONE,    OPT_NONE}},
	{ "spawn_at",         OP_SPAWNAT,         2, {OPT_INT,  OPT_INT,     OPT_NONE}},
	{ "spawn_flag",       OP_SPAWNFLAG,       3, {OPT_INT,  OPT_INT,     OPT_INT}},
	{ "countalive",       OP_COUNTALIVE,      2, {OPT_INT,  OPT_VAR,     OPT_NONE}},
	{ "spawnloc",         OP_SPAWNLOC,        1, {OPT_INT,  OPT_NONE,    OPT_NONE}},
	{ "get_npc_id",       OP_GETNPCID,        2, {OPT_INT,  OPT_VAR,     OPT_NONE}},
	{ "set_target",       OP_SETTARGET,       2, {OPT_VAR,  OPT_VAR,     OPT_NONE}},

	{ "scan_npc_cid",     OP_SCAN_NPC_CID,    2, {OPT_STR,  OPT_INTARR,  OPT_NONE}},
	{ "scan_npc_cid_for", OP_SCAN_NPC_CID_FOR,3, {OPT_STR,  OPT_INTARR,  OPT_INT}},
	{ "get_cdef",         OP_GET_CDEF,        2, {OPT_VAR,  OPT_VAR,     OPT_NONE}},
	{ "get_health_percent",     OP_GET_HEALTH_PERCENT,    2, {OPT_VAR,  OPT_VAR,  OPT_NONE}},
	{ "order_walk",       OP_ORDER_WALK,      3, {OPT_VAR,  OPT_INT,     OPT_INT}},
	{ "get_target",       OP_GET_TARGET,      2, {OPT_VAR,  OPT_VAR,     OPT_NONE}},
	{ "ai_script_jump",   OP_AI_SCRIPT_JUMP,  2, {OPT_VAR,  OPT_STR,     OPT_NONE}},

	{ "info",             OP_INFO,            1, {OPT_STR,  OPT_NONE,    OPT_NONE}},
	{ "chat",             OP_CHAT,            3, {OPT_STR,  OPT_STR,     OPT_STR}},
	{ "despawn",          OP_DESPAWN,         1, {OPT_INT,  OPT_NONE,    OPT_NONE}},
};
const int maxExtOpCode = COUNT_ARRAY_ELEMENTS(extCoreOpCode);

void InstanceScriptDef::GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize)
{
	*arrayStart = InstanceScript::extCoreOpCode;
	arraySize = InstanceScript::maxExtOpCode;
}

void InstanceScriptDef :: SetMetaDataDerived(const char *opname, ScriptCore::ScriptCompiler &compileData)
{
	STRINGLIST &tokens = compileData.mTokens;
	if(strcmp(opname, "#location") == 0)
	{
		//#location name x1 y1 x2 y2
		if(compileData.ExpectTokens(6, "#location", "str:name int:x1 int:z1 int:x2 int:z2") == true)
		{
			InstanceLocation &loc = mLocationDef[tokens[1]];
			loc.mX1 = Util::GetInteger(tokens, 2);
			loc.mY1 = Util::GetInteger(tokens, 3);
			loc.mX2 = Util::GetInteger(tokens, 4);
			loc.mY2 = Util::GetInteger(tokens, 5);
		}
	}
	else if(strcmp(opname, "#location_br") == 0)
	{
		if(compileData.ExpectTokens(5, "location_br", "str:name int:x int:z int:radius") == true)
		{
			InstanceLocation &loc = mLocationDef[tokens[1]];
			int x = Util::GetInteger(tokens, 2);
			int y = Util::GetInteger(tokens, 3);
			int r = Util::GetInteger(tokens, 4);
			loc.mX1 = x - r;
			loc.mY1 = y - r;
			loc.mX2 = x + r;
			loc.mY2 = y + r;
		}
	}
}

bool InstanceScriptDef::HandleAdvancedCommand(const char *commandToken, ScriptCore::ScriptCompiler &compileData)
{
	STRINGLIST &tokens = compileData.mTokens;
	bool retVal = false;
	if(strcmp(commandToken, "spawnloc") == 0)
	{
		retVal = true;
		if(tokens.size() < 5)
		{
			ScriptCore::PrintMessage("Syntax error: not enough operands for SPAWNLOC statement (%s line %d", compileData.mSourceFile, compileData.mLineNumber);
		}
		else
		{
			int CDefID = atoi(tokens[1].c_str());
			int x = atoi(tokens[2].c_str());
			int y = atoi(tokens[3].c_str());
			int z = atoi(tokens[4].c_str());

			//Need the arguments on the stack first, backwards so they can be read back in order.
			PushOpCode(OP_PUSHINT, z, 0);
			PushOpCode(OP_PUSHINT, y, 0);
			PushOpCode(OP_PUSHINT, x, 0);
			PushOpCode(OP_SPAWNLOC, CDefID, 0);
		}
	}
	return retVal;
}

InstanceLocation* InstanceScriptDef::GetLocationByName(const char *location)
{
	std::map<std::string, InstanceLocation>::iterator it;
	it = mLocationDef.find(location);
	if(it == mLocationDef.end())
		return NULL;
	return &it->second;
}


void InstanceScriptPlayer::RunImplementationCommands(int opcode)
{
	ScriptCore::OpData *instr = &def->instr[curInst];
	switch(opcode)
	{
	case OP_DESPAWN:
		actInst->spawnsys.RemoveSpawnPoint(instr->param1);
		break;
	case OP_SPAWN:
		actInst->spawnsys.TriggerSpawn(instr->param1, 0, 0);
		//g_Log.AddMessageFormat("Fired spawn: %d", def->instr[curInst].param1);
		break;
	case OP_SPAWNAT:
		actInst->spawnsys.TriggerSpawn(instr->param1, instr->param2, 0);
		//g_Log.AddMessageFormat("Fired spawn: %d, creature: %d", def->instr[curInst].param1, def->instr[curInst].param2);
		break;
	case OP_SPAWNFLAG:
		actInst->spawnsys.TriggerSpawn(instr->param1, instr->param2, instr->param3);
		//g_Log.AddMessageFormat("Fired spawn: %d, creature: %d", def->instr[curInst].param1, def->instr[curInst].param2);
		break;
	case OP_SPAWNLOC:
		{
			//Pop in reverse order
			int x = PopVarStack();
			int y = PopVarStack();
			int z = PopVarStack();
			if(x != 0 && y != 0 && z != 0)
				actInst->SpawnGeneric(instr->param1, x, y, z, 0);
		}
		break;
	case OP_COUNTALIVE:
		vars[instr->param2] = actInst->CountAlive(instr->param1);
		break;
	case OP_GETNPCID:
		{
			int creatureID = 0;
			CreatureInstance *search = actInst->GetNPCInstanceByCDefID(instr->param1);
			if(search != NULL)
				creatureID = search->CreatureID;
			vars[instr->param2] = creatureID;
		}
		break;
	case OP_SETTARGET:
		{
			CreatureInstance *source = actInst->GetInstanceByCID(GetVarValue(instr->param1));
			CreatureInstance *target = actInst->GetInstanceByCID(GetVarValue(instr->param2));
			if(source != NULL && target != NULL)
				source->SelectTarget(target);
		}
		break;

	case OP_SCAN_NPC_CID:
		{
			InstanceLocation *loc = GetLocationByName(GetStringPtr(instr->param1));
			int index = VerifyIntArrayIndex(instr->param2);
			if(index >= 0)
				ScanNPCCID(loc, intArray[index].arrayData);
		}
		break;
	case OP_SCAN_NPC_CID_FOR:
		{
			InstanceLocation *loc = GetLocationByName(GetStringPtr(instr->param1));
			int index = VerifyIntArrayIndex(instr->param2);
			if(index >= 0)
				ScanNPCCIDFor(loc, instr->param3, intArray[index].arrayData);
		}
		break;
	case OP_GET_CDEF:
		{
			int cdef = 0;
			CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
			if(ci)
				cdef = ci->CreatureDefID;
			SetVar(instr->param2, cdef);
		}
		break;
	case OP_GET_HEALTH_PERCENT:
		{
			CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
			int health = 0;
			if(ci)
				health = static_cast<int>(ci->GetHealthRatio() * 100.0F);
			SetVar(instr->param2, health);
		}
		break;
	case OP_ORDER_WALK:
		{
			CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
			if(ci)
			{
				ci->SetServerFlag(ServerFlags::ScriptMovement, true);
				ci->previousPathNode = 0;   //Disable any path links.
				ci->nextPathNode = 0;
				ci->tetherNodeX = instr->param2;
				ci->tetherNodeZ = instr->param3;
				ci->CurrentTarget.DesLocX = instr->param2;
				ci->CurrentTarget.DesLocZ = instr->param3;
				ci->CurrentTarget.desiredRange = 30;
				ci->Speed = 20;
			}
		}
		break;
	case OP_GET_TARGET:
		{
			int targetID = 0;
			CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
			if(ci)
			{
				if(ci->CurrentTarget.targ != NULL)
					targetID = ci->CurrentTarget.targ->CreatureID;
			}
			SetVar(instr->param2, targetID);
		}
		break;
	case OP_AI_SCRIPT_JUMP:
		{
			CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
			if(ci)
			{
				if(ci->aiScript)
					ci->aiScript->JumpToLabel(GetStringPtr(instr->param2));
			}
		}
		break;
	case OP_INFO:
		{
			char buffer[4096];
			int wpos = PrepExt_SendInfoMessage(buffer, GetStringPtr(instr->param1), INFOMSG_INFO);
			actInst->LSendToAllSimulator(buffer, wpos, -1);
		}
		break;
	case OP_CHAT:
		{
			char buffer[4096];
			int wpos = PrepExt_GenericChatMessage(buffer, 0, GetStringPtr(instr->param1), GetStringPtr(instr->param2), GetStringPtr(instr->param3));
			actInst->LSendToAllSimulator(buffer, wpos, -1);
		}
		break;
	default:
		g_Log.AddMessageFormat("Unidentified InstanceScriptPlayer OpCode: %d", instr->opCode);
		break;
	}
}

void InstanceScriptPlayer::SetInstancePointer(ActiveInstance *parent)
{
	actInst = parent;
}

InstanceLocation* InstanceScriptPlayer::GetLocationByName(const char *name)
{
	if(name == NULL)
		return NULL;
	InstanceScriptDef *thisDef = dynamic_cast<InstanceScriptDef*>(def);
	return thisDef->GetLocationByName(name);
}

void InstanceScriptPlayer::ScanNPCCIDFor(InstanceLocation *location, int CDefID, std::vector<int>& destResult)
{
	destResult.clear();
	if(actInst == NULL || location == NULL)
		return;
	for(size_t i = 0; i < actInst->NPCListPtr.size(); i++)
	{
		CreatureInstance *ci = actInst->NPCListPtr[i];
		if(ci->CreatureDefID != CDefID)
			continue;
		if(ci->CurrentX < location->mX1)
			continue;
		if(ci->CurrentX > location->mX2)
			continue;
		if(ci->CurrentZ < location->mY1)
			continue;
		if(ci->CurrentZ > location->mY2)
			continue;
		destResult.push_back(ci->CreatureID);
	}
}

void InstanceScriptPlayer::ScanNPCCID(InstanceLocation *location, std::vector<int>& destResult)
{
	destResult.clear();
	if(actInst == NULL || location == NULL)
		return;
	for(size_t i = 0; i < actInst->NPCListPtr.size(); i++)
	{
		CreatureInstance *ci = actInst->NPCListPtr[i];
		if(ci->CurrentX < location->mX1)
			continue;
		if(ci->CurrentX > location->mX2)
			continue;
		if(ci->CurrentZ < location->mY1)
			continue;
		if(ci->CurrentZ > location->mY2)
			continue;
		destResult.push_back(ci->CreatureID);
	}
}

CreatureInstance* InstanceScriptPlayer::GetNPCPtr(int CID)
{
	if(actInst == NULL)
		return NULL;
	return actInst->GetNPCInstanceByCID(CID);
}


} //namespace InstanceScript


