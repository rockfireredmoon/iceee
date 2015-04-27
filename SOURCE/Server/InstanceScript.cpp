#include "InstanceScript.h"
#include "AIScript.h"
#include "Instance.h"
#include "CommonTypes.h"
#include "StringList.h"
#include "Simulator.h"
#include <stdlib.h>
#include <sqrat.h>
#include <sqrat.h>
#include <algorithm>    // std::remove
//#include <sqbind.h>

//extern char GAuxBuf[];

namespace InstanceScript
{


enum InstanceScriptExtOpCodes
{
	OP_NOP = OP_MAXCOREOPCODE,  //The first instruction must begin where the default ScriptCore opcodes leave off.

	// Implemenation-Specific named commands.
	OP_SPAWN,         //spawn <propid>  force a spawnpoint generate a spawn
	OP_SPAWNAT,       //spawnat <propid> <creatureDef>  generate an arbitrary creature at a known spawnpoint.
	OP_SPAWNFLAG,       //spawnflag <propid> <creatureDef> <flags> generate an arbitrary creature at a known spawnpoint with specified flags.
	OP_PARTICLE_ATTACH, //particleattach <propid> <effectName> <scale> attach a particle effect to a prop. A 'tag' will be pushed onto the stack that must be popped
	OP_PARTICLE_DETACH, //particledetach <propid> <tag>  detech a particle effect from a prop. The 'tag' should have been popped from the stack when attaching
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
	OP_BROADCAST,           // broadcast <message> send info message locally or broadcast to server in instance
	OP_DESPAWN,             // despawn <propid> force a despawn of a spawnpoint
	OP_DESPAWN_ALL,			// despawn_all <creatureDefID> despawn all creatures of a type
};

OpCodeInfo extCoreOpCode[] = {
	// Implementation-Specific commands.
	{ "spawn",            OP_SPAWN,           1, {OPT_INT,  OPT_NONE,    OPT_NONE}},
	{ "spawn_at",         OP_SPAWNAT,         2, {OPT_INT,  OPT_INT,     OPT_NONE}},
	{ "spawn_flag",       OP_SPAWNFLAG,       3, {OPT_INT,  OPT_INT,     OPT_INT}},
	{ "particle_attach",  OP_PARTICLE_ATTACH, 3, {OPT_INT,  OPT_STR,     OPT_INT}},
	{ "particle_detach",  OP_PARTICLE_DETACH, 2, {OPT_INT,  OPT_VAR,     OPT_NONE}},
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
	{ "broadcast",        OP_BROADCAST,       1, {OPT_STR,  OPT_NONE,    OPT_STR}},
	{ "despawn",          OP_DESPAWN,         1, {OPT_VAR,  OPT_NONE,    OPT_NONE}},
	{ "despawn_all",      OP_DESPAWN_ALL,     1, {OPT_VAR,  OPT_NONE,    OPT_NONE}},
};
const int maxExtOpCode = COUNT_ARRAY_ELEMENTS(extCoreOpCode);



InstanceNutDef::~InstanceNutDef()
{
}


std::string InstanceNutDef::GetInstanceScriptPath(int zoneID, bool pathIfNotExists) {
	char strBuf[100];
	Util::SafeFormat(strBuf, sizeof(strBuf), "Instance\\%d\\Script.nut", zoneID);
	Platform::FixPaths(strBuf);
	if(!Platform::FileExists(strBuf)) {
		Util::SafeFormat(strBuf, sizeof(strBuf), "Instance\\%d\\Script.txt", zoneID);
		Platform::FixPaths(strBuf);
		if(!Platform::FileExists(strBuf) && !pathIfNotExists) {
			return "";
		}
		if(pathIfNotExists) {
			Util::SafeFormat(strBuf, sizeof(strBuf), "Instance\\%d\\Script.nut", zoneID);
			Platform::FixPaths(strBuf);
		}
	}
	return strBuf;
}

InstanceNutPlayer::InstanceNutPlayer()
{
	actInst = NULL;
	spawned.clear();
}

InstanceNutPlayer::~InstanceNutPlayer()
{
//	sqbind_method(def->v, "broadcast"). )
}

void InstanceNutPlayer::HaltDerivedExecution()
{
	std::vector<int>::iterator it;
	for(it = spawned.begin(); it != spawned.end(); ++it)
	{
		CreatureInstance *source = actInst->GetInstanceByCID(*it);
		if(source != NULL)
			actInst->spawnsys.Despawn(*it);
	}
	spawned.clear();
}

void InstanceNutPlayer::RegisterDerivedFunctions()
{
	Sqrat::DefaultVM::Set(vm);

	Sqrat::Class<InstanceLocation> instanceLocation;
	Sqrat::RootTable().Bind(_SC("InstanceLocation"), instanceLocation);

	Sqrat::Class<InstanceNutPlayer> instance;
	Sqrat::RootTable().Bind(_SC("Instance"), instance);

	instance.Func(_SC("broadcast"), &InstanceNutPlayer::Broadcast);
	instance.Func(_SC("info"), &InstanceNutPlayer::Info);
	instance.Func(_SC("spawn"), &InstanceNutPlayer::Spawn);
	instance.Func(_SC("spawnAt"), &InstanceNutPlayer::SpawnAt);
	instance.Func(_SC("cdef"), &InstanceNutPlayer::CDefIDForCID);
	instance.Func(_SC("cids"), &InstanceNutPlayer::GetAllCIDForCDefID);
	instance.Func(_SC("scanForNPCByCDefID"), &InstanceNutPlayer::ScanForNPCByCDefID);
	instance.Func(_SC("getTarget"), &InstanceNutPlayer::GetTarget);
	instance.Func(_SC("setTarget"), &InstanceNutPlayer::SetTarget);
	instance.Func(_SC("ai"), &InstanceNutPlayer::AI);
	instance.Func(_SC("countAlive"), &InstanceNutPlayer::CountAlive);
	instance.Func(_SC("healthPercent"), &InstanceNutPlayer::GetHealthPercent);
	instance.Func(_SC("orderWalk"), &InstanceNutPlayer::OrderWalk);
	instance.Func(_SC("chat"), &InstanceNutPlayer::Chat);
	instance.Func(_SC("despawn"), &InstanceNutPlayer::Despawn);
	instance.Func(_SC("despawnAll"), &InstanceNutPlayer::DespawnAll);
	instance.Func(_SC("particleAttach"), &InstanceNutPlayer::ParticleAttach);
	instance.Func(_SC("particleDetach"), &InstanceNutPlayer::DetachSceneryEffect);
	instance.Func(_SC("asset"), &InstanceNutPlayer::Asset);

	Sqrat::RootTable().SetInstance(_SC("inst"), this);
}

void InstanceNutPlayer::SetInstancePointer(ActiveInstance *parent)
{
	actInst = parent;
}

int InstanceNutPlayer::Asset(int propID, const char *newAsset, float scale) {
	char buffer[256];
	SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, propID, NULL);
	if(propPtr != NULL)
	{
		SceneryEffectList *effectList = actInst->GetSceneryEffectList(propID);
		SceneryEffect l;
		l.type = ASSET_UPDATE;
		l.tag = ++actInst->mNextEffectTag;
		l.propID = propID;
		l.effect = newAsset;
		l.scale = scale;
		effectList->push_back(l);
		int wpos = actInst->AddSceneryEffect(buffer, &l);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
		g_Log.AddMessageFormat("Create effect tag %d for prop %d.", l.tag, propID);
		return l.tag;
	}
	else
		g_Log.AddMessageFormat("Could not find prop with ID %d in this instance.", propID);
	return -1;
}

void InstanceNutPlayer::DetachSceneryEffect(int propID, int tag) {
	char buffer[256];
	SceneryEffect *tagObj = actInst->RemoveSceneryEffect(propID, tag);
	if(tagObj != NULL)
	{
		int wpos = actInst->DetachSceneryEffect(buffer, tagObj->propID, tagObj->type, tag);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
	}
}

int InstanceNutPlayer::ParticleAttach(int propID, const char *effect, float scale, float offsetX, float offsetY, float offsetZ) {
	char buffer[256];
	SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, propID, NULL);
	if(propPtr != NULL)
	{
		SceneryEffectList *effectList = actInst->GetSceneryEffectList(propID);
		SceneryEffect l;
		l.tag = ++actInst->mNextEffectTag;
		l.type = PARTICLE_EFFECT;
		l.propID = propID;
		l.effect = effect;
		l.scale = scale;
		l.offsetX = offsetX;
		l.offsetY = offsetY;
		l.offsetZ = offsetZ;
		effectList->push_back(l);
		int wpos = actInst->AddSceneryEffect(buffer, &l);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
		g_Log.AddMessageFormat("Create effect tag %d for prop %d.", l.tag, propID);
		return l.tag;
	}
	else
		g_Log.AddMessageFormat("Could not find prop with ID %d in this instance.", propID);
	return -1;
}

void InstanceNutPlayer::Chat(const char *name, const char *channel, const char *message) {
	char buffer[4096];
	int wpos = PrepExt_GenericChatMessage(buffer, 0, name, channel, message);
	actInst->LSendToAllSimulator(buffer, wpos, -1);
}

void InstanceNutPlayer::OrderWalk(int CID, float destX, float destY, int speed, int range) {

	CreatureInstance *ci = GetNPCPtr(CID);
	if(ci)
	{
		ci->SetServerFlag(ServerFlags::ScriptMovement, true);
		ci->previousPathNode = 0;   //Disable any path links.
		ci->nextPathNode = 0;
		ci->tetherNodeX = destX;
		ci->tetherNodeZ = destY;
		ci->CurrentTarget.DesLocX = destX;
		ci->CurrentTarget.DesLocZ = destY;
		ci->CurrentTarget.desiredRange = range;
		ci->Speed = speed;
	}
}

vector<int> InstanceNutPlayer::GetAllCIDForCDefID(int CDefID) {
	vector<int> v;
	actInst->GetNPCInstancesByCDefID(CDefID, v);
	return v;
}

bool InstanceNutPlayer::Despawn(int CID)
{
	CreatureInstance *source = actInst->GetNPCInstanceByCID(CID);
	if(source == NULL)
		return false;
	spawned.erase(std::remove(spawned.begin(), spawned.end(), CID), spawned.end());
	actInst->spawnsys.Despawn(CID);
	return true;
}

int InstanceNutPlayer::DespawnAll(int CDefID)
{
	int despawned = 0;
	while(true) {
		CreatureInstance *source = actInst->GetNPCInstanceByCDefID(CDefID);
		if(source == NULL)
			break;
		else {
			actInst->spawnsys.Despawn(source->CreatureID);
			spawned.erase(std::remove(spawned.begin(), spawned.end(), source->CreatureID), spawned.end());
			despawned++;
		}
	}
	return despawned;
}

int InstanceNutPlayer::GetHealthPercent(int cid)
{
	CreatureInstance *ci = GetNPCPtr(cid);
	if(ci)
		return static_cast<int>(ci->GetHealthRatio() * 100.0F);
	return 0;
}

int InstanceNutPlayer::CDefIDForCID(int cid)
{
	CreatureInstance *ci = GetNPCPtr(cid);
	if(ci)
		return ci->CreatureDefID;
	return -1;
}

int InstanceNutPlayer::CountAlive(int CDefID)
{
	return actInst->CountAlive(CDefID);
}

bool InstanceNutPlayer::AI(int CID, const char *label)
{
	CreatureInstance *ci = GetNPCPtr(CID);
	return ci && ci->aiScript && ci->aiScript->JumpToLabel(label);
}

int InstanceNutPlayer::GetTarget(int CDefID)
{
	int targetID = 0;
	CreatureInstance *ci = GetNPCPtr(CDefID);
	if(ci)
	{
		if(ci->CurrentTarget.targ != NULL)
			targetID = ci->CurrentTarget.targ->CreatureID;
	}
	return targetID;
}

bool InstanceNutPlayer::SetTarget(int CDefID, int targetCDefID)
{
	CreatureInstance *source = actInst->GetInstanceByCID(CDefID);
	CreatureInstance *target = actInst->GetInstanceByCID(targetCDefID);
	if(source != NULL && target != NULL)
	{
		source->SelectTarget(target);
		return true;
	}
	return false;
}

vector<int> InstanceNutPlayer::ScanForNPCByCDefID(InstanceLocation *location, int CDefID) {
	vector<int> v;
	v.clear();
	if(actInst == NULL || location == NULL)
		return v;
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
		v.push_back(ci->CreatureID);
	}
	return v;
}

int InstanceNutPlayer::Spawn(int propID, int creatureID, int flags)
{
	int res =actInst->spawnsys.TriggerSpawn(propID, creatureID, flags);
	if(res > -1) {
		spawned.push_back(res);
	}
	return res;
}

int InstanceNutPlayer::SpawnAt(int creatureID, float x, float y, float z, int facing, int flags)
{
	CreatureInstance *creature = actInst->SpawnGeneric(creatureID, x, y, z, facing, flags);
	if(def == NULL )
		return -1;
	else {
		spawned.push_back(creature->CreatureID);
		return creature->CreatureID;
	}
}

void InstanceNutPlayer::Info(const char *message)
{
	char buffer[4096];
	int wpos = PrepExt_SendInfoMessage(buffer, message, INFOMSG_INFO);
	actInst->LSendToAllSimulator(buffer, wpos, -1);
}

void InstanceNutPlayer::Broadcast(const char *message)
{
	char buffer[4096];
	if(actInst->mZoneDefPtr->mGrove)
	{
		int wpos = PrepExt_SendInfoMessage(buffer, message, INFOMSG_INFO);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
	}
	else
		g_SimulatorManager.BroadcastMessage(message);
}

CreatureInstance* InstanceNutPlayer::GetNPCPtr(int CID)
{
	if(actInst == NULL)
		return NULL;
	return actInst->GetNPCInstanceByCID(CID);
}

//
//
//

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
	case OP_PARTICLE_ATTACH:
	{
		char buffer[256];
		SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, instr->param1, NULL);
		if(propPtr != NULL)
		{
			SceneryEffectList *effectList = actInst->GetSceneryEffectList(instr->param1);
			SceneryEffect l;
			l.tag = ++actInst->mNextEffectTag;
			l.propID = instr->param1;
			l.effect = GetStringPtr(instr->param2);
			l.scale = (float)instr->param3 / 100.0;
			effectList->push_back(l);
			int wpos = actInst->AddSceneryEffect(buffer, &l);
			actInst->LSendToAllSimulator(buffer, wpos, -1);
			PushVarStack(l.tag);
			g_Log.AddMessageFormat("Create effect tag %d for prop %d.", l.tag, instr->param1);
		}
		else
			g_Log.AddMessageFormat("Could not find prop with ID %d in this instance.", instr->param1);
		break;
	}
	case OP_PARTICLE_DETACH:
		{
			char buffer[256];
			SceneryEffect *tag = actInst->RemoveSceneryEffect(instr->param1, GetVarValue(instr->param2));
			if(tag != NULL)
			{
				int wpos = actInst->DetachSceneryEffect(buffer, tag->propID, tag->type, tag->tag);
				actInst->LSendToAllSimulator(buffer, wpos, -1);
			}
			else
			{
				Util::SafeFormat(buffer, sizeof(buffer), "Could not find effect tag with ID %d in this instance.", instr->param1);
				int wpos = PrepExt_SendInfoMessage(buffer, GetStringPtr(instr->param1), INFOMSG_INFO);
				actInst->LSendToAllSimulator(buffer, wpos, -1);
			}
			break;
		}
	case OP_DESPAWN_ALL:
	{
		while(true) {
			CreatureInstance *source = actInst->GetNPCInstanceByCDefID(GetVarValue(instr->param1));
			if(source == NULL)
				break;
			else {
				g_Log.AddMessageFormat("Despawn: %d (%d)", GetVarValue(instr->param1), source->CreatureID);
				actInst->spawnsys.Despawn(source->CreatureID);
			}
		}
		break;
	}
	case OP_DESPAWN:
	{
		CreatureInstance *source = actInst->GetInstanceByCID(GetVarValue(instr->param1));
		g_Log.AddMessageFormat("Despawn: %d (%d)", GetVarValue(instr->param1), source->CreatureDefID);
		if(source == NULL)
			g_Log.AddMessageFormat("Despawn failed, %d does not exist.", GetVarValue(instr->param1));
		else
			actInst->spawnsys.Despawn(GetVarValue(instr->param1));
		break;
	}
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
				actInst->SpawnGeneric(instr->param1, x, y, z, 0, 0);
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
	case OP_BROADCAST:
		{
			char buffer[4096];
			if(actInst->mZoneDefPtr->mGrove)
			{
				int wpos = PrepExt_SendInfoMessage(buffer, GetStringPtr(instr->param1), INFOMSG_INFO);
				actInst->LSendToAllSimulator(buffer, wpos, -1);
			}
			else
			{
				int wpos = PrepExt_Broadcast(buffer, GetStringPtr(instr->param1));
				actInst->LSendToAllSimulator(buffer, wpos, -1);
			}
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


