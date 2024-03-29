#include "InstanceScript.h"
#include "AIScript.h"
#include "AIScript2.h"
#include "Ability2.h"
#include "Instance.h"
#include "CommonTypes.h"

#include "Simulator.h"
#include "Creature.h"
#include "Config.h"
#include <stdlib.h>
#include "sqrat.h"
#include <algorithm>    // remove

#include "util/SquirrelObjects.h"
#include "util/Log.h"

namespace InstanceScript {

enum InstanceScriptExtOpCodes {
	OP_NOP = OP_MAXCOREOPCODE, //The first instruction must begin where the default ScriptCore opcodes leave off.

	// Implemenation-Specific named commands.
	OP_SPAWN,         //spawn <propid>  force a spawnpoint generate a spawn
	OP_SPAWNAT, //spawnat <propid> <creatureDef>  generate an arbitrary creature at a known spawnpoint.
	OP_SPAWNFLAG, //spawnflag <propid> <creatureDef> <flags> generate an arbitrary creature at a known spawnpoint with specified flags.
	OP_PARTICLE_ATTACH, //particleattach <propid> <effectName> <scale> attach a particle effect to a prop. A 'tag' will be pushed onto the stack that must be popped
	OP_PARTICLE_DETACH, //particledetach <propid> <tag>  detech a particle effect from a prop. The 'tag' should have been popped from the stack when attaching
	OP_COUNTALIVE,    //
	OP_SPAWNLOC, //spawnloc <creaturedefid> <x> <y> <z>  Spawn a creature at exact coordinates. NOTE: x,y,z are pushed onto stack.
	OP_GETNPCID, //get_npc_id <int_creaturedefID> <var_output>     Get the first creature matching a DefID and place it into VAR.
	OP_SETTARGET, //set_target <var_sourceCID> <var_targetCID>   Set the CreatureID from VAR1 to target the CreatureID from VAR2

	OP_SCAN_NPC_CID,         //scan_mobs_cid <str:location> <iarray:dest>
	OP_SCAN_NPC_CID_FOR, //scan_mobs_cid_for <str:location> <iarray:dest> <creatureDef>
	OP_GET_CDEF,
	OP_GET_HEALTH_PERCENT,   //get_health_percent <var:CID>
	OP_ORDER_WALK,           //order_walk <var:CID> <int:destX> <int:destZ>
	OP_GET_TARGET,           //get_target <var:CID> <var:dest>     
	OP_AI_SCRIPT_JUMP,      //ai_script_jump <var:CID> <str:label>
	OP_INFO,                //info <str:message> <int:globalBroadcast>
	OP_CHAT,
	OP_BROADCAST, // broadcast <message> send info message locally or broadcast to server in instance
	OP_DESPAWN,             // despawn <propid> force a despawn of a spawnpoint
	OP_DESPAWN_ALL, // despawn_all <creatureDefID> despawn all creatures of a type
};

OpCodeInfo extCoreOpCode[] =
		{
				// Implementation-Specific commands.
				{ "spawn", OP_SPAWN, 1, { OPT_INT, OPT_NONE, OPT_NONE } }, {
						"spawn_at", OP_SPAWNAT, 2,
						{ OPT_INT, OPT_INT, OPT_NONE } }, { "spawn_flag",
						OP_SPAWNFLAG, 3, { OPT_INT, OPT_INT, OPT_INT } }, {
						"particle_attach", OP_PARTICLE_ATTACH, 3, { OPT_INT,
								OPT_STR, OPT_INT } }, { "particle_detach",
						OP_PARTICLE_DETACH, 2, { OPT_INT, OPT_VAR, OPT_NONE } },
				{ "countalive", OP_COUNTALIVE, 2, { OPT_INT, OPT_VAR, OPT_NONE } },
				{ "spawnloc", OP_SPAWNLOC, 1, { OPT_INT, OPT_NONE, OPT_NONE } },
				{ "get_npc_id", OP_GETNPCID, 2, { OPT_INT, OPT_VAR, OPT_NONE } },
				{ "set_target", OP_SETTARGET, 2, { OPT_VAR, OPT_VAR, OPT_NONE } },

				{ "scan_npc_cid", OP_SCAN_NPC_CID, 2, { OPT_STR, OPT_INTARR,
						OPT_NONE } }, { "scan_npc_cid_for", OP_SCAN_NPC_CID_FOR,
						3, { OPT_STR, OPT_INTARR, OPT_INT } }, { "get_cdef",
						OP_GET_CDEF, 2, { OPT_VAR, OPT_VAR, OPT_NONE } }, {
						"get_health_percent", OP_GET_HEALTH_PERCENT, 2, {
								OPT_VAR, OPT_VAR, OPT_NONE } }, { "order_walk",
						OP_ORDER_WALK, 3, { OPT_VAR, OPT_INT, OPT_INT } }, {
						"get_target", OP_GET_TARGET, 2, { OPT_VAR, OPT_VAR,
								OPT_NONE } }, { "ai_script_jump",
						OP_AI_SCRIPT_JUMP, 2, { OPT_VAR, OPT_STR, OPT_NONE } },

				{ "info", OP_INFO, 1, { OPT_STR, OPT_NONE, OPT_NONE } }, {
						"chat", OP_CHAT, 3, { OPT_STR, OPT_STR, OPT_STR } }, {
						"broadcast", OP_BROADCAST, 1, { OPT_STR, OPT_NONE,
								OPT_STR } }, { "despawn", OP_DESPAWN, 1, {
						OPT_VAR, OPT_NONE, OPT_NONE } }, { "despawn_all",
						OP_DESPAWN_ALL, 1, { OPT_VAR, OPT_NONE, OPT_NONE } }, };
const int maxExtOpCode = COUNT_ARRAY_ELEMENTS(extCoreOpCode);

InstanceNutDef::~InstanceNutDef() {
}

void InstanceNutDef::Reload() {
	if(IsFromCluster()) {
		LoadFromCluster(mZoneID);
	}
	else {
		NutDef::Reload();
	}
}

fs::path InstanceNutDef::GetInstanceNutScriptPath(int zoneID) {
	return g_Config.ResolveVariableDataPath() / "Instance" / Util::Format("%d", zoneID) / "Script.nut";
}
fs::path InstanceNutDef::GetInstanceScriptPath(int zoneID,
		bool pathIfNotExists) {
	auto p = GetInstanceNutScriptPath(zoneID);
	if (!fs::exists(p)) {
		auto t = g_Config.ResolveVariableDataPath() / "Instance" / Util::Format("%d", zoneID) / "Script.txt";
		if (!fs::exists(t) && !pathIfNotExists) {
			return "";
		}
		else if (pathIfNotExists) {
			return p;
		}
		else
			return t;
	}
	return p;
}

bool InstanceNutDef::LoadFromCluster(int zoneID) {
	mZoneID = zoneID;
	mSourceFile = Util::Format("Grove/%d.nut", zoneID);
	scriptName = Util::Format("%d", zoneID);
	return g_ClusterManager.ReadEntity(this);
}

ActiveInteraction::ActiveInteraction(CreatureInstance *creature,
		ScriptCore::NutScriptEvent *event) {
	mCreature = creature;
	mEvent = event;
}

ActiveInteraction::~ActiveInteraction() {
}

//
// Abstract player class for scripts that run in instances. This includes instance scripts
// and AI scripts
//
AbstractInstanceNutPlayer::AbstractInstanceNutPlayer() {
	actInst = NULL;
}

AbstractInstanceNutPlayer::~AbstractInstanceNutPlayer() {

}

void AbstractInstanceNutPlayer::RegisterAbstractInstanceFunctions(
		NutPlayer *instance,
		Sqrat::DerivedClass<AbstractInstanceNutPlayer, NutPlayer> *instanceClass) {

	// Vector3F Object, floating point X/Y/Z location
	Sqrat::Class<SceneryObject> sceneryObjectClass(vm, "SceneryObject", true);
	Sqrat::RootTable(vm).Bind(_SC("SceneryObject"), sceneryObjectClass);
	sceneryObjectClass.Var("x", &SceneryObject::LocationX);
	sceneryObjectClass.Var("y", &SceneryObject::LocationY);
	sceneryObjectClass.Var("z", &SceneryObject::LocationZ);
	sceneryObjectClass.Var("id", &SceneryObject::ID);
	sceneryObjectClass.Var("asset", &SceneryObject::Asset);
	sceneryObjectClass.Var("name", &SceneryObject::Name);
	sceneryObjectClass.Var("layer", &SceneryObject::Layer);
	sceneryObjectClass.Var("sx", &SceneryObject::ScaleX);
	sceneryObjectClass.Var("sy", &SceneryObject::ScaleY);
	sceneryObjectClass.Var("sz", &SceneryObject::ScaleZ);
	sceneryObjectClass.Var("qx", &SceneryObject::QuatX);
	sceneryObjectClass.Var("qy", &SceneryObject::QuatY);
	sceneryObjectClass.Var("qz", &SceneryObject::QuatZ);
	sceneryObjectClass.Var("qw", &SceneryObject::QuatW);
	sceneryObjectClass.Func(_SC("is_spawn_point"), &SceneryObject::IsSpawnPoint);

	// Functions that return arrays or tables have to be dealt with differently
	instanceClass->SquirrelFunc(_SC("cids"), &InstanceNutPlayer::CIDs);
	instanceClass->SquirrelFunc(_SC("all_cids"), &InstanceNutPlayer::AllCIDs);
	instanceClass->SquirrelFunc(_SC("all_players"),
			&InstanceNutPlayer::AllPlayers);

	// Common instance functions (TODO register in abstract class somehow)
	instanceClass->Func(_SC("props"), &InstanceNutPlayer::Props);
	instanceClass->Func(_SC("get_creature_distance"),
			&AbstractInstanceNutPlayer::GetCreatureDistance);
	instanceClass->Func(_SC("creature_spawn_prop"),
			&AbstractInstanceNutPlayer::GetCreatureSpawnProp);
	instanceClass->Func(_SC("get_npc_id"),
			&AbstractInstanceNutPlayer::GetNPCID);
	instanceClass->Func(_SC("shake"), &AbstractInstanceNutPlayer::Shake);
	instanceClass->Func(_SC("creature_use"),
			&AbstractInstanceNutPlayer::CreatureUse);
	instanceClass->Func(_SC("creature_use_once"),
			&AbstractInstanceNutPlayer::CreatureUseNoRetry);
	instanceClass->Func(_SC("broadcast"),
			&AbstractInstanceNutPlayer::Broadcast);
	instanceClass->Func(_SC("local_broadcast"),
			&AbstractInstanceNutPlayer::LocalBroadcast);
	instanceClass->Func(_SC("info"), &AbstractInstanceNutPlayer::Info);
	instanceClass->Func(_SC("unhate"), &AbstractInstanceNutPlayer::Unhate);
	instanceClass->Func(_SC("interrupt"),
			&AbstractInstanceNutPlayer::Interrupt);
	instanceClass->Func(_SC("message"), &AbstractInstanceNutPlayer::Message);
	instanceClass->Func(_SC("untarget"), &AbstractInstanceNutPlayer::Untarget);
	instanceClass->Func(_SC("error"), &AbstractInstanceNutPlayer::Error);
	instanceClass->Func(_SC("get_cid_for_prop"),
			&AbstractInstanceNutPlayer::GetCIDForPropID);
	instanceClass->Func(_SC("chat"), &AbstractInstanceNutPlayer::Chat);
	instanceClass->Func(_SC("creature_chat"),
			&AbstractInstanceNutPlayer::CreatureChat);
	instanceClass->Func(_SC("rotate_creature"),
			&AbstractInstanceNutPlayer::RotateCreature);
	instanceClass->Func(_SC("set_flags"),
			&AbstractInstanceNutPlayer::SetServerFlags);
	instanceClass->Func(_SC("set_flag"),
			&AbstractInstanceNutPlayer::SetServerFlag);
	instanceClass->Func(_SC("get_flags"),
			&AbstractInstanceNutPlayer::GetServerFlags);
	instanceClass->Func(_SC("stop_ai"), &AbstractInstanceNutPlayer::StopAI);
	instanceClass->Func(_SC("clear_ai_queue"),
			&AbstractInstanceNutPlayer::ClearAIQueue);
	instanceClass->Func(_SC("is_at_tether"),
			&AbstractInstanceNutPlayer::IsAtTether);
	instanceClass->Func(_SC("target_self"),
			&AbstractInstanceNutPlayer::TargetSelf);
	instanceClass->Func(_SC("set_timeofday"),
			&AbstractInstanceNutPlayer::SetTimeOfDay);
	instanceClass->Func(_SC("get_timeofday"),
			&AbstractInstanceNutPlayer::GetTimeOfDay);
	instanceClass->Func(_SC("set_env"),
			&AbstractInstanceNutPlayer::SetEnvironment);
	instanceClass->Func(_SC("get_env"),
			&AbstractInstanceNutPlayer::GetEnvironment);

	// Functions that return arrays or tables have to be dealt with differently
	instanceClass->SquirrelFunc(_SC("get_nearby_creature"),
			&AbstractInstanceNutPlayer::GetCreaturesNearCreature);

	Sqrat::RootTable(vm).Func(_SC("_AB"), &InstanceNutPlayer::GetAbilityID);

	// Some constants
	Sqrat::ConstTable(vm).Const(_SC("SPAWN_TILE_SIZE"),
			SpawnTile::SPAWN_TILE_SIZE);
	Sqrat::ConstTable(vm).Const(_SC("SPAWN_TILE_RANGE"),
			SpawnTile::SPAWN_TILE_RANGE);

	Sqrat::ConstTable(vm).Const(_SC("INFOMSG_INFO"), INFOMSG_INFO);
	Sqrat::ConstTable(vm).Const(_SC("INFOMSG_ERROR"), INFOMSG_ERROR);
	Sqrat::ConstTable(vm).Const(_SC("INFOMSG_SYSNOTIFY"), INFOMSG_SYSNOTIFY);
	Sqrat::ConstTable(vm).Const(_SC("INFOMSG_SHARD"), INFOMSG_SHARD);
	Sqrat::ConstTable(vm).Const(_SC("INFOMSG_MAPNAME"), INFOMSG_MAPNAME);

	Sqrat::ConstTable(vm).Const(_SC("CREATURE_WALK_SPEED"),
			CREATURE_WALK_SPEED);
	Sqrat::ConstTable(vm).Const(_SC("CREATURE_JOG_SPEED"), CREATURE_JOG_SPEED);
	Sqrat::ConstTable(vm).Const(_SC("CREATURE_RUN_SPEED"), CREATURE_RUN_SPEED);

	Sqrat::ConstTable(vm).Const(_SC("RES_OK"), ScriptCore::Result::OK);
	Sqrat::ConstTable(vm).Const(_SC("RES_INTR"),
			ScriptCore::Result::INTERRUPTED);
	Sqrat::ConstTable(vm).Const(_SC("RES_FAILED"), ScriptCore::Result::FAILED);

	Sqrat::ConstTable(vm).Const(_SC("TS_NONE"), TargetStatus::None);
	Sqrat::ConstTable(vm).Const(_SC("TS_ALIVE"), TargetStatus::Alive);
	Sqrat::ConstTable(vm).Const(_SC("TS_DEAD"), TargetStatus::Dead);
	Sqrat::ConstTable(vm).Const(_SC("TS_ENEMY"), TargetStatus::Enemy);
	Sqrat::ConstTable(vm).Const(_SC("TS_ENEMY_ALIVE"), TargetStatus::Enemy_Alive);
	Sqrat::ConstTable(vm).Const(_SC("TS_FRIEND"), TargetStatus::Friend);
	Sqrat::ConstTable(vm).Const(_SC("TS_FRIEND_ALIVE"),
			TargetStatus::Friend_Alive);
	Sqrat::ConstTable(vm).Const(_SC("TS_FRIEND_DEAD"),
			TargetStatus::Friend_Dead);

	Sqrat::ConstTable(vm).Const(_SC("HATE_SIDEKICK"),
			SidekickObject::HATE_SIDEKICK);
	Sqrat::ConstTable(vm).Const(_SC("HATE_OFFICER"),
			SidekickObject::HATE_OFFICER);
	Sqrat::ConstTable(vm).Const(_SC("HATE_BOTH"), SidekickObject::HATE_BOTH);
	Sqrat::ConstTable(vm).Const(_SC("HATE_SIDEKICK_MORE"),
			SidekickObject::HATE_SIDEKICK_MORE);
	Sqrat::ConstTable(vm).Const(_SC("HATE_OFFICER_MORE"),
			SidekickObject::HATE_OFFICER_MORE);
	Sqrat::ConstTable(vm).Const(_SC("HATE_NEITHER"),
			SidekickObject::HATE_NEITHER);

	Sqrat::ConstTable(vm).Const(_SC("SIDEKICK_GENERIC"),
			SidekickObject::GENERIC);
	Sqrat::ConstTable(vm).Const(_SC("SIDEKICK_ABILITY"),
			SidekickObject::ABILITY);
	Sqrat::ConstTable(vm).Const(_SC("SIDEKICK_PET"), SidekickObject::PET);
	Sqrat::ConstTable(vm).Const(_SC("SIDEKICK_QUEST"), SidekickObject::QUEST);

	Sqrat::ConstTable(vm).Const(_SC("STATUS_EFFECT_INVINCIBLE"),
			StatusEffects::INVINCIBLE);
	Sqrat::ConstTable(vm).Const(_SC("STATUS_EFFECT_USABLE_BY_COMBATANT"),
			StatusEffects::USABLE_BY_COMBATANT);
	Sqrat::ConstTable(vm).Const(_SC("STATUS_EFFECT_IS_USABLE"),
			StatusEffects::IS_USABLE);
	Sqrat::ConstTable(vm).Const(_SC("STATUS_EFFECT_UNATTACKABLE"),
			StatusEffects::UNATTACKABLE);

	/* NOTE: Not all */
	Sqrat::ConstTable(vm).Const(_SC("SF_LOCAL_ACTIVE"),
			ServerFlags::LocalActive);
	Sqrat::ConstTable(vm).Const(_SC("SF_IS_PLAYER"), ServerFlags::IsPlayer);
	Sqrat::ConstTable(vm).Const(_SC("SF_IS_NPC"), ServerFlags::IsNPC);
	Sqrat::ConstTable(vm).Const(_SC("SF_IS_SIDEKICK"), ServerFlags::IsSidekick);
	Sqrat::ConstTable(vm).Const(_SC("SF_INIT_ATTACK"), ServerFlags::InitAttack);
	Sqrat::ConstTable(vm).Const(_SC("SF_AUTO_TARGET"), ServerFlags::AutoTarget);
	Sqrat::ConstTable(vm).Const(_SC("SF_CALLED_BACK"), ServerFlags::CalledBack);
	Sqrat::ConstTable(vm).Const(_SC("SF_PATH_NODE"), ServerFlags::PathNode);
	Sqrat::ConstTable(vm).Const(_SC("SF_LEASH_RECALL"),
			ServerFlags::LeashRecall);
	Sqrat::ConstTable(vm).Const(_SC("SF_IS_TRANSFORMED"),
			ServerFlags::IsTransformed);
	Sqrat::ConstTable(vm).Const(_SC("SF_NEUTRAL_INACTIVE"),
			ServerFlags::NeutralInactive);
	Sqrat::ConstTable(vm).Const(_SC("SF_STATIONARY"), ServerFlags::Stationary);
	Sqrat::ConstTable(vm).Const(_SC("SF_HAS_MELEE_WEAPON"),
			ServerFlags::HasMeleeWeapon);
	Sqrat::ConstTable(vm).Const(_SC("SF_HAS_SHIELD"), ServerFlags::HasShield);
	Sqrat::ConstTable(vm).Const(_SC("SF_NON_COMBATANT"),
			ServerFlags::Noncombatant);
	Sqrat::ConstTable(vm).Const(_SC("SF_TAUNTED"), ServerFlags::Taunted);
	Sqrat::ConstTable(vm).Const(_SC("SF_SCRIPT_MOVEMENT"),
			ServerFlags::ScriptMovement);


	Sqrat::ConstTable(vm).Const(_SC("FLAG_FRIENDLY"),
			SpawnPackageDef::FLAG_FRIENDLY);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_HIDEMAP"),
			SpawnPackageDef::FLAG_HIDEMAP);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_NEUTRAL"),
			SpawnPackageDef::FLAG_NEUTRAL);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_FRIENDLY_ATTACK"),
			SpawnPackageDef::FLAG_FRIENDLY_ATTACK);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_ENEMY"),
			SpawnPackageDef::FLAG_ENEMY);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_VISWEAPON_MELEE"),
			SpawnPackageDef::FLAG_VISWEAPON_MELEE);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_VISWEAPON_RANGED"),
			SpawnPackageDef::FLAG_VISWEAPON_RANGED);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_ALLBITS"),
			SpawnPackageDef::FLAG_ALLBITS);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_VISWEAPON_MELEE"),
			SpawnPackageDef::FLAG_VISWEAPON_MELEE);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_USABLE"),
			SpawnPackageDef::FLAG_USABLE);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_USABLE_BY_COMBATANT"),
			SpawnPackageDef::FLAG_USABLE_BY_COMBATANT);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_VISWEAPON_MELEE"),
			SpawnPackageDef::FLAG_VISWEAPON_MELEE);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_HIDE_NAMEBOARD"),
			SpawnPackageDef::FLAG_HIDE_NAMEBOARD);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_HIDE_NAMEBOARD"),
			SpawnPackageDef::FLAG_HIDE_NAMEBOARD);
	Sqrat::ConstTable(vm).Const(_SC("FLAG_KILLABLE"),
			SpawnPackageDef::FLAG_KILLABLE);
}

void AbstractInstanceNutPlayer::ClearAIQueue(int CID) {
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci != NULL && ci->aiNut != NULL)
		ci->aiNut->QueueClear();
	if (ci != NULL && ci->aiScript != NULL)
		ci->aiScript->ClearQueue();
}

bool AbstractInstanceNutPlayer::IsAtTether(int CID) {
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci != NULL)
		return ci->IsAtTether();
	return false;
}

bool AbstractInstanceNutPlayer::TargetSelf(int CID) {
	CreatureInstance *source = actInst->GetInstanceByCID(CID);
	if (source != NULL) {
		source->SelectTarget(source);
		return true;
	}
	return false;
}

bool AbstractInstanceNutPlayer::SetEnvironment(const char *environment) {
	actInst->SetEnvironment(environment);
	return true;
}

string AbstractInstanceNutPlayer::GetTimeOfDay() {
	return actInst->GetTimeOfDay();
}

void AbstractInstanceNutPlayer::SetTimeOfDay(string timeOfDay) {
	actInst->SetTimeOfDay(timeOfDay);
}

string AbstractInstanceNutPlayer::GetEnvironment(int x, int y) {
	return actInst->GetEnvironment(x, y);
}

void AbstractInstanceNutPlayer::StopAI(int CID) {
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci->aiScript != NULL)
		ci->aiScript->EndExecution();
	if (ci->aiNut != NULL)
		ci->aiNut->Halt();
}

void AbstractInstanceNutPlayer::Info(const char *message) {
	Message(message, INFOMSG_INFO);
}

void AbstractInstanceNutPlayer::CreatureChat(int CID, const char *channel,
		const char *message) {
	char buffer[4096];
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci != NULL)
		actInst->LSendToAllSimulator(buffer,
				PrepExt_GenericChatMessage(buffer, CID, ci->css.display_name,
						channel, message), -1);
	else
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to communicate.",
				CID);
}

void AbstractInstanceNutPlayer::RotateCreature(int CID, int rotation) {
	CreatureInstance *ci = actInst->GetInstanceByCID(CID);
	if (ci != NULL) {
		ci->Rotation = rotation;
		ci->Heading = rotation;
		ci->SetServerFlag(ServerFlags::ScriptMovement, true);
	} else
		g_Logs.script->error(
				"Could not find creature with ID %d in this instance to rotate.",
				CID);
}

void AbstractInstanceNutPlayer::Chat(const char *name, const char *channel,
		const char *message) {
	char buffer[4096];
	int wpos = PrepExt_GenericChatMessage(buffer, 0, name, channel, message);
	actInst->LSendToAllSimulator(buffer, wpos, -1);
}

void AbstractInstanceNutPlayer::Error(const char *message) {
	Message(message, INFOMSG_ERROR);
}

void AbstractInstanceNutPlayer::Message(const char *message, int type) {

	char buffer[4096];
	int wpos = PrepExt_SendInfoMessage(buffer, message, type);
	actInst->LSendToAllSimulator(buffer, wpos, -1);
}

void AbstractInstanceNutPlayer::Broadcast(const char *message) {
	char buffer[4096];
	if (actInst->mZoneDefPtr->mGrove) {
		int wpos = PrepExt_SendInfoMessage(buffer, message, INFOMSG_INFO);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
	} else
		NutPlayer::Broadcast(message);
}

void AbstractInstanceNutPlayer::LocalBroadcast(const char *message) {
	actInst->BroadcastMessage(message);
}

int AbstractInstanceNutPlayer::GetAbilityID(const char *name) {
	return g_AbilityManager.GetAbilityIDByName(name);
}

void AbstractInstanceNutPlayer::Shake(float amount, float time, float range) {
	char buffer[256];
	int wpos = actInst->Shake(buffer, amount, time, range);
	actInst->LSendToAllSimulator(buffer, wpos, -1);
}

void AbstractInstanceNutPlayer::Unhate(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL)
		ci->UnHate();
	else
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to unhate.",
				CID);
}

bool AbstractInstanceNutPlayer::Untarget(int CID) {
	CreatureInstance *source = actInst->GetInstanceByCID(CID);
	if (source != NULL) {
		source->SelectTarget(NULL);
		return true;
	}
	return false;
}
bool AbstractInstanceNutPlayer::CreatureUse(int CID, int abilityID) {
	return DoCreatureUse(CID, abilityID, true);
}
bool AbstractInstanceNutPlayer::CreatureUseNoRetry(int CID, int abilityID) {
	return DoCreatureUse(CID, abilityID, false);
}

void AbstractInstanceNutPlayer::Interrupt(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL)
		ci->Interrupt();
	else
		g_Logs.script->error(
				"Could not find creature with ID %d in this instance to interrupt.",
				CID);
}

bool AbstractInstanceNutPlayer::DoCreatureUse(int CID, int abilityID,
		bool retry) {
	CreatureInstance *attachedCreature = actInst->GetInstanceByCID(CID);
	if (attachedCreature != NULL && attachedCreature->ab[0].bPending == false) {
		//DEBUG OUTPUT
		if (g_Config.DebugLogAIScriptUse == true) {
			const Ability2::AbilityEntry2* abptr =
					g_AbilityManager.GetAbilityPtrByID(abilityID);
			g_Logs.script->debug("Using: %s",
					abptr->GetRowAsCString(Ability2::ABROW::NAME));
		}
		//END DEBUG OUTPUT

		/* We use 'force' so Noncombatant doesn't affect scripts (used heavily by Valkal, Boss Hawg etc) */
		int r = attachedCreature->CallAbilityEvent(abilityID,
				EventType::onRequest, true);

		if (r != 0) {
			//Notify the creature we failed, may need a distance check.
			//The script should wait and retry soon.
			attachedCreature->AICheckAbilityFailure(r);

			if (g_Config.DebugLogAIScriptUse == true) {
				const Ability2::AbilityEntry2* abptr =
						g_AbilityManager.GetAbilityPtrByID(abilityID);
				g_Logs.script->debug("Using: %s   Failed: %d",
						abptr->GetRowAsCString(Ability2::ABROW::NAME),
						g_AbilityManager.GetAbilityErrorCode(r));
			}

			if (retry
					&& attachedCreature->AIAbilityFailureAllowRetry(r) == true)
				QueueAdd(
						new ScriptCore::NutScriptEvent(
								new ScriptCore::TimeCondition(USE_FAIL_DELAY),
								new InstanceUseCallback(this, CID, abilityID)));
		} else
			return true;
	}
	return false;
}

int AbstractInstanceNutPlayer::GetCreatureSpawnProp(int CID) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature != NULL && creature->spawnGen != NULL
			&& creature->spawnGen->spawnPoint != NULL) {
		return creature->spawnGen->spawnPoint->ID;
	}
	return 0;
}

int AbstractInstanceNutPlayer::GetCreatureDistance(int CID, int CID2) {
	CreatureInstance *targ1 = actInst->GetInstanceByCID(CID);
	CreatureInstance *targ2 = actInst->GetInstanceByCID(CID2);
	if (targ1 == NULL || targ2 == NULL)
		return -1;
	return targ1->GetDistance(targ2, SANE_DISTANCE);
}

int AbstractInstanceNutPlayer::GetCIDForPropID(int propID) {
	return actInst->spawnsys.GetCIDForProp(propID);
}

SQInteger AbstractInstanceNutPlayer::GetCreaturesNearCreature(HSQUIRRELVM v) {
	if (sq_gettop(v) == 6) {
		Sqrat::Var<AbstractInstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			Sqrat::Var<int> range(v, 2);
			Sqrat::Var<int> cid(v, 3);
			Sqrat::Var<int> playerAbilityRestrict(v, 4);
			Sqrat::Var<int> npcAbilityRestrict(v, 5);
			Sqrat::Var<int> sidekickAbilityRestrict(v, 6);
			//vector<CreatureInstance*> vv;
			CreatureInstance::CREATURE_PTR_SEARCH vv;
			CreatureInstance* target = left.value.actInst->GetNPCInstanceByCID(
					cid.value);
			if (target != NULL) {
				float x = (float) target->CurrentX;
				float z = (float) target->CurrentZ;
				target->AIFillCreaturesNear(range.value, x, z,
						playerAbilityRestrict.value, npcAbilityRestrict.value,
						sidekickAbilityRestrict.value, vv);
			}
			sq_newarray(v, 0);
			for (size_t i = 0; i < vv.size(); ++i) {
				sq_pushinteger(v, vv[i]->CreatureID);
				sq_arrayappend(v, -2);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

int AbstractInstanceNutPlayer::GetNPCID(int CDefID) {
	CreatureInstance *targ = actInst->GetNPCInstanceByCDefID(CDefID);
	return targ == NULL ? 0 : targ->CreatureID;
}

void AbstractInstanceNutPlayer::MonitorArea(string name,
		Squirrel::Area area) {
	InstanceScript::MonitorArea a;
	a.name = name;
	a.area = area;
	monitorAreas.push_back(a);

	for (vector<CreatureInstance*>::iterator it =
			actInst->PlayerListPtr.begin(); it != actInst->PlayerListPtr.end();
			++it) {
		PlayerMovement(*it);
	}
}

void AbstractInstanceNutPlayer::UnmonitorArea(string name) {
	for (vector<InstanceScript::MonitorArea>::iterator it =
			monitorAreas.begin(); it != monitorAreas.end(); ++it) {
		if ((*it).name.compare(name) == 0) {
			monitorAreas.erase(it);
			return;
		}
	}
}

void AbstractInstanceNutPlayer::PlayerLeft(CreatureInstance *creature) {
	bool contain;
	for (vector<InstanceScript::MonitorArea>::iterator it =
			monitorAreas.begin(); it != monitorAreas.end(); ++it) {
		InstanceScript::MonitorArea mon = *it;
		contain = mon.Contains(creature->CreatureID);
		if (contain) {
			// Player leaves monitored area
			mon.Remove(creature->CreatureID);
			vector<ScriptCore::ScriptParam> p;
			p.push_back(ScriptCore::ScriptParam(creature->CreatureID));
			char ConvBuf[256];
			Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_player_exit_%s",
					mon.name.c_str());
			RunFunction(ConvBuf, p, false);
		}
	}
}

void AbstractInstanceNutPlayer::PlayerMovement(CreatureInstance *creature) {
	bool inside;
	bool contain;

	for (size_t i = 0 ; i < monitorAreas.size(); i++) {
		InstanceScript::MonitorArea area = monitorAreas[i];
		inside = area.area.Inside(creature->CurrentX, creature->CurrentZ);
		contain = area.Contains(creature->CreatureID);
		if (inside && !contain) {
			area.creatureIds.push_back(creature->CreatureID);
			monitorAreas[i] = area;
			// Player enters monitored area
			vector<ScriptCore::ScriptParam> p;
			p.push_back(ScriptCore::ScriptParam(area.name));
			p.push_back(ScriptCore::ScriptParam(creature->CreatureID));
			char ConvBuf[256];
			Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_player_enter");
			JumpToLabel(ConvBuf, p);
		} else if (!inside && contain) {
			// Player leaves monitored area
			area.Remove(creature->CreatureID);
			monitorAreas[i] = area;
			vector<ScriptCore::ScriptParam> p;
			p.push_back(ScriptCore::ScriptParam(area.name));
			p.push_back(ScriptCore::ScriptParam(creature->CreatureID));
			char ConvBuf[256];
			Util::SafeFormat(ConvBuf, sizeof(ConvBuf), "on_player_exit");
			JumpToLabel(ConvBuf, p);
		}
	}
}

vector<int> AbstractInstanceNutPlayer::ScanForNPCs(
		Squirrel::Area *location, int CDefID) {

	vector<int> vv;

	if (actInst != NULL) {
		for (size_t i = 0; i < actInst->NPCListPtr.size(); i++) {
			CreatureInstance *ci = actInst->NPCListPtr[i];
			if (CDefID != -1 && ci->CreatureDefID != CDefID)
				continue;
			if (ci->CurrentX < location->mX1)
				continue;
			if (ci->CurrentX > location->mX2)
				continue;
			if (ci->CurrentZ < location->mY1)
				continue;
			if (ci->CurrentZ > location->mY2)
				continue;
			vv.push_back(ci->CreatureID);
		}
	}
	return vv;
}

void InstanceNutPlayer::SetCreatureGTAE(int CID) {
	CreatureInstance *source = actInst->GetInstanceByCID(CID);
	if (source != NULL)
		source->AISetGTAE();
}

void InstanceNutPlayer::SetCreatureGTAETo(int CID, Squirrel::Vector3I loc) {
	CreatureInstance *source = actInst->GetInstanceByCID(CID);
	if (source != NULL)
		source->AISetGTAETo(loc.mX, loc.mY, loc.mZ);
}

int AbstractInstanceNutPlayer::ScanNPC(Squirrel::Area *location, int CDefID) {
	vector<int> v = ScanForNPCs(location, CDefID);
	return v.size() == 0 ? 0 : v[0];
}

void AbstractInstanceNutPlayer::SetServerFlag(int CID, unsigned long flag,
		bool state) {
	CreatureInstance *ci = actInst->GetInstanceByCID(CID);
	if (ci != NULL) {
		ci->SetServerFlag(flag, state);
	} else
		g_Logs.script->error(
				"Could not find creature with ID %d in this instance to set server flag.",
				CID);
}

void AbstractInstanceNutPlayer::SetServerFlags(int CID, unsigned long flags) {
	CreatureInstance *ci = actInst->GetInstanceByCID(CID);
	if (ci != NULL) {
		ci->serverFlags = flags;
	} else
		g_Logs.script->error(
				"Could not find creature with ID %d in this instance to set server flags.",
				CID);
}

unsigned long AbstractInstanceNutPlayer::GetServerFlags(int CID) {
	CreatureInstance *ci = actInst->GetInstanceByCID(CID);
	if (ci != NULL) {
		return ci->serverFlags;
	} else
		g_Logs.script->error(
				"Could not find creature with ID %d in this instance to set server flags.",
				CID);
	return -1;
}

SQInteger AbstractInstanceNutPlayer::ScanNPCs(HSQUIRRELVM v) {
	if (sq_gettop(v) == 3) {
		Sqrat::Var<AbstractInstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			Sqrat::Var<Squirrel::Area> right(v, 2);
			Sqrat::Var<int> end(v, 3);
			Squirrel::Area *location = &right.value;
			int CDefID = end.value;
			vector<int> vv = left.value.ScanForNPCs(location, CDefID);
			sq_newarray(v, vv.size());
			for (size_t i = 0; i < vv.size(); ++i) {
				Sqrat::PushVar(v, i);
				Sqrat::PushVar(v, vv[i]);
				sq_rawset(v, -3);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

SQInteger AbstractInstanceNutPlayer::Scan(HSQUIRRELVM v) {
	if (sq_gettop(v) == 2) {
		Sqrat::Var<AbstractInstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			Sqrat::Var<Squirrel::Area> right(v, 2);
			Squirrel::Area *location = &right.value;
			vector<int> vv = left.value.ScanForNPCs(location, -1);
			sq_newarray(v, vv.size());
			for (size_t i = 0; i < vv.size(); ++i) {
				Sqrat::PushVar(v, i);
				Sqrat::PushVar(v, vv[i]);
				sq_rawset(v, -3);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

SQInteger AbstractInstanceNutPlayer::GetHated(HSQUIRRELVM v) {
	if (sq_gettop(v) == 2) {
		Sqrat::Var<AbstractInstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			Sqrat::Var<int> right(v, 2);
			vector<int> vv;
			CreatureInstance *creature = left.value.GetNPCPtr(right.value);
			if (creature != NULL && creature->hateProfilePtr != NULL) {
				vector<HateCreatureData>::iterator it;
				for (it = creature->hateProfilePtr->hateList.begin();
						it < creature->hateProfilePtr->hateList.end(); ++it) {
					vv.push_back(it->CID);
				}
			}
			sq_newarray(v, vv.size());
			for (size_t i = 0; i < vv.size(); ++i) {
				Sqrat::PushVar(v, i);
				Sqrat::PushVar(v, vv[i]);
				sq_rawset(v, -3);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

SQInteger AbstractInstanceNutPlayer::AllCIDs(HSQUIRRELVM v) {
	if (sq_gettop(v) == 1) {
		Sqrat::Var<InstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			sq_newarray(v, 0);
			vector<CreatureInstance*>::iterator it;
			for (it = left.value.actInst->NPCListPtr.begin();
					it != left.value.actInst->NPCListPtr.end(); ++it) {
				sq_pushinteger(v, (*it)->CreatureID);
				sq_arrayappend(v, -2);
			}
			for (it = left.value.actInst->PlayerListPtr.begin();
					it != left.value.actInst->PlayerListPtr.end(); ++it) {
				sq_pushinteger(v, (*it)->CreatureID);
				sq_arrayappend(v, -2);
			}
			for (it = left.value.actInst->SidekickListPtr.begin();
					it != left.value.actInst->SidekickListPtr.end(); ++it) {
				sq_pushinteger(v, (*it)->CreatureID);
				sq_arrayappend(v, -2);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

SQInteger AbstractInstanceNutPlayer::AllPlayers(HSQUIRRELVM v) {
	if (sq_gettop(v) == 1) {
		Sqrat::Var<InstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			sq_newarray(v, 0);
			vector<CreatureInstance*>::iterator it;
			for (it = left.value.actInst->PlayerListPtr.begin();
					it != left.value.actInst->PlayerListPtr.end(); ++it) {
				sq_pushinteger(v, (*it)->CreatureID);
				sq_arrayappend(v, -2);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

Sqrat::Object AbstractInstanceNutPlayer::Props(Squirrel::Vector3I location) {

	vector<SceneryObject> vv;
	Sqrat::Array arr(vm);
	actInst->GetProps(location.mX, location.mZ, &vv);
	for(vector<SceneryObject>::iterator it = vv.begin(); it != vv.end(); it++) {
		arr.Append(*it);
	}

	return arr;
}

SQInteger AbstractInstanceNutPlayer::CIDs(HSQUIRRELVM v) {
	if (sq_gettop(v) == 2) {
		Sqrat::Var<InstanceNutPlayer&> left(v, 1);
		if (!Sqrat::Error::Occurred(v)) {
			Sqrat::Var<int> right(v, 2);
			vector<int> vv;
			left.value.actInst->GetNPCInstancesByCDefID(right.value, &vv);
			sq_newarray(v, vv.size());
			for (size_t i = 0; i < vv.size(); ++i) {
				Sqrat::PushVar(v, i);
				Sqrat::PushVar(v, vv[i]);
				sq_rawset(v, -3);
			}
			return 1;
		}
		return sq_throwerror(v, Sqrat::Error::Message(v).c_str());
	}
	return sq_throwerror(v, _SC("wrong number of parameters"));
}

void AbstractInstanceNutPlayer::SetInstancePointer(ActiveInstance *parent) {
	actInst = parent;
}

CreatureInstance* AbstractInstanceNutPlayer::GetNPCPtr(int CID) {
	if (actInst == NULL)
		return NULL;
	return actInst->GetNPCInstanceByCID(CID);
}

CreatureInstance* AbstractInstanceNutPlayer::GetCreaturePtr(int CID) {
	if (actInst == NULL)
		return NULL;
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci == NULL) {
		ci = actInst->GetPlayerByID(CID);
	}
	return ci;
}

//
// Main player class for instance scripts
//

InstanceNutPlayer::InstanceNutPlayer() {
	spawned.clear();
	genericSpawned.clear();
	openedForms.clear();
}

InstanceNutPlayer::~InstanceNutPlayer() {
//	sqbind_method(def->v, "broadcast"). )
}

void InstanceNutPlayer::HaltedDerived() {
}

void InstanceNutPlayer::HaltDerivedExecution() {
	UnremoveProps();

	vector<SceneryEffect> effectsToRemove;
	effectsToRemove.insert(effectsToRemove.begin(), activeEffects.begin(),
			activeEffects.end());
	vector<SceneryEffect>::iterator eit;
	for (eit = effectsToRemove.begin(); eit != effectsToRemove.end(); ++eit)
		DetachSceneryEffect(eit->propID, eit->tag);

	vector<int>::iterator it;
	for (it = spawned.begin(); it != spawned.end(); ++it) {
		CreatureInstance *source = actInst->GetInstanceByCID(*it);
		if (source != NULL)
			actInst->spawnsys.Despawn(*it);
	}
	spawned.clear();

	for (it = genericSpawned.begin(); it != genericSpawned.end(); ++it) {
		CreatureInstance *source = actInst->GetInstanceByCID(*it);
		if (source != NULL)
			actInst->spawnsys.Despawn(*it);
	}
	genericSpawned.clear();

	map<int, int>::iterator it2;
	char buf[32];
	for (it2 = openedForms.begin(); it2 != openedForms.end(); ++it2) {
		CreatureInstance *creature = actInst->GetInstanceByCID(it2->second);
		if (creature != NULL && creature->simulatorPtr != NULL) {
			creature->simulatorPtr->AttemptSend(buf,
					PrepExt_SendFormClose(buf, it2->first));
		}
	}
	openedForms.clear();

	actInst->SetEnvironment("");
	actInst->SetTimeOfDay("");
}

void InstanceNutPlayer::RegisterInstanceFunctions(HSQUIRRELVM vm,
		Sqrat::DerivedClass<InstanceNutPlayer, AbstractInstanceNutPlayer> *instanceClass) {
	instanceClass->Func(_SC("interact"), &InstanceNutPlayer::Interact);
	instanceClass->Func(_SC("transform"), &InstanceNutPlayer::Transform);
	instanceClass->Func(_SC("set_size"), &InstanceNutPlayer::SetSize);
	instanceClass->Func(_SC("pvp_goal"), &InstanceNutPlayer::PVPGoal);
	instanceClass->Func(_SC("has_status_effect"),
			&InstanceNutPlayer::HasStatusEffect);
	instanceClass->Func(_SC("set_status_effect"),
			&InstanceNutPlayer::SetStatusEffect);
	instanceClass->Func(_SC("remove_status_effect"),
			&InstanceNutPlayer::RemoveStatusEffect);
	instanceClass->Func(_SC("pvp_start"), &InstanceNutPlayer::PVPStart);
	instanceClass->Func(_SC("pvp_stop"), &InstanceNutPlayer::PVPStop);
	instanceClass->Func(_SC("create_party"), &InstanceNutPlayer::CreateParty);
	instanceClass->Func(_SC("create_team"), &InstanceNutPlayer::CreateTeam);
	instanceClass->Func(_SC("disband_party"),
			&InstanceNutPlayer::DisbandVirtualParty);
	instanceClass->Func(_SC("add_to_party"),
			&InstanceNutPlayer::AddToVirtualParty);
	instanceClass->Func(_SC("quit_party"), &InstanceNutPlayer::QuitParty);
	instanceClass->Func(_SC("get_party_members"),
			&InstanceNutPlayer::GetVirtualPartyMembers);
	instanceClass->Func(_SC("get_party_size"),
			&InstanceNutPlayer::GetVirtualPartySize);
	instanceClass->Func(_SC("get_leader"),
			&InstanceNutPlayer::GetVirtualPartyLeader);
	instanceClass->Func(_SC("attach_item"), &InstanceNutPlayer::AttachItem);
	instanceClass->Func(_SC("detach_item"), &InstanceNutPlayer::DetachItem);
	instanceClass->Func(_SC("get_ai"), &InstanceNutPlayer::GetAI);
	instanceClass->Func(_SC("clear_target"), &InstanceNutPlayer::ClearTarget);
	instanceClass->Func(_SC("spawn_prop"), &InstanceNutPlayer::SpawnProp);
	instanceClass->Func(_SC("spawn"), &InstanceNutPlayer::Spawn);
	instanceClass->Func(_SC("play_sound"), &InstanceNutPlayer::PlaySound);
	instanceClass->Func(_SC("invite_quest"), &InstanceNutPlayer::InviteQuest);
	instanceClass->Func(_SC("join_quest"), &InstanceNutPlayer::JoinQuest);
	instanceClass->Func(_SC("attach_sidekick"),
			&InstanceNutPlayer::AttachSidekick);
	instanceClass->Func(_SC("get_display_name"),
			&InstanceNutPlayer::GetDisplayName);
	instanceClass->Func(_SC("load_spawn_tile"),
			&InstanceNutPlayer::LoadSpawnTile);
	instanceClass->Func(_SC("load_spawn_tile_for"),
			&InstanceNutPlayer::LoadSpawnTileFor);
	instanceClass->Func(_SC("spawn_at"), &InstanceNutPlayer::SpawnAt);
	instanceClass->Func(_SC("cdef"), &InstanceNutPlayer::CDefIDForCID);
	instanceClass->Func(_SC("getTarget"), &InstanceNutPlayer::GetTarget);
	instanceClass->Func(_SC("get_target"), &InstanceNutPlayer::GetTarget);
	instanceClass->Func(_SC("get_location"), &InstanceNutPlayer::GetLocation);
	instanceClass->Func(_SC("get_party_id"), &InstanceNutPlayer::GetPartyID);
	instanceClass->Func(_SC("set_target"), &InstanceNutPlayer::SetTarget);
	instanceClass->Func(_SC("set_creature_gtae"),
			&InstanceNutPlayer::SetCreatureGTAE);
	instanceClass->Func(_SC("set_creature_gtae_to"),
			&InstanceNutPlayer::SetCreatureGTAETo);
	instanceClass->Func(_SC("ai"), &InstanceNutPlayer::AI);
	instanceClass->Func(_SC("count_alive"), &InstanceNutPlayer::CountAlive);
	instanceClass->Func(_SC("get_health_pc"),
			&InstanceNutPlayer::GetHealthPercent);
	instanceClass->Func(_SC("get_spawn_prop"),
			&InstanceNutPlayer::GetPropIDForSpawn);
	instanceClass->Func(_SC("walk"), &InstanceNutPlayer::Walk);
	instanceClass->Func(_SC("walk_then"), &InstanceNutPlayer::WalkThen);
	instanceClass->Func(_SC("despawn"), &InstanceNutPlayer::Despawn);
	instanceClass->Func(_SC("despawn_all"), &InstanceNutPlayer::DespawnAll);
	instanceClass->Func(_SC("has_item"), &InstanceNutPlayer::HasItem);
	instanceClass->Func(_SC("open_book"), &InstanceNutPlayer::OpenBook);
	instanceClass->Func(_SC("open_form"), &InstanceNutPlayer::OpenForm);
	instanceClass->Func(_SC("close_form"), &InstanceNutPlayer::CloseForm);
	instanceClass->Func(_SC("give_item"), &InstanceNutPlayer::GiveItem);
	instanceClass->Func(_SC("warp_player"), &InstanceNutPlayer::WarpPlayer);
	// TODO deprecated
	instanceClass->Func(_SC("effect"), &InstanceNutPlayer::ParticleAttach);
	instanceClass->Func(_SC("restore"),
			&InstanceNutPlayer::DetachSceneryEffect);
	instanceClass->Func(_SC("asset"), &InstanceNutPlayer::Asset);
	instanceClass->Func(_SC("emote"), &InstanceNutPlayer::Emote);
	instanceClass->Func(_SC("remove_prop"), &InstanceNutPlayer::RemoveProp);
	instanceClass->Func(_SC("unremove_prop"), &InstanceNutPlayer::UnremoveProp);
	instanceClass->Func(_SC("unremove_props"),
			&InstanceNutPlayer::UnremoveProps);
	instanceClass->Func(_SC("scan_npc"), &InstanceNutPlayer::ScanNPC);
	instanceClass->Func(_SC("monitor_area"), &InstanceNutPlayer::MonitorArea);
	instanceClass->Func(_SC("unmonitor_area"),
			&InstanceNutPlayer::UnmonitorArea);
	instanceClass->Func(_SC("quest_advance"), &InstanceNutPlayer::AdvanceQuest);

	// Functions that return arrays or tables have to be dealt with differently
	instanceClass->SquirrelFunc(_SC("get_hated"), &InstanceNutPlayer::GetHated);
	instanceClass->SquirrelFunc(_SC("scan_npcs"),
			&AbstractInstanceNutPlayer::ScanNPCs);
	instanceClass->SquirrelFunc(_SC("scan"), &AbstractInstanceNutPlayer::Scan);

}

bool InstanceNutPlayer::HasStatusEffect(int CID, const char *effect) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci) {
		int EffectID = GetStatusIDByName(effect);
		if (EffectID != -1) {
			return ci->HasStatus(EffectID);
		}

	}
	return false;
}

void InstanceNutPlayer::RegisterFunctions() {
	Sqrat::Class<NutPlayer> nutClass(vm, _SC("Core"), true);
	Sqrat::RootTable(vm).Bind(_SC("Core"), nutClass);
	RegisterCoreFunctions(this, &nutClass);

	Sqrat::DerivedClass<AbstractInstanceNutPlayer, NutPlayer> abstractInstanceClass(
			vm, _SC("AbstractInstance"));
	RegisterAbstractInstanceFunctions(this, &abstractInstanceClass);
	Sqrat::DerivedClass<InstanceNutPlayer, AbstractInstanceNutPlayer> instanceClass(
			vm, _SC("Instance"));
	Sqrat::RootTable(vm).Bind(_SC("Instance"), instanceClass);

	Sqrat::DerivedClass<AINutPlayer, InstanceScript::InstanceNutPlayer> aiClass(
			vm, _SC("AI"));
	Sqrat::RootTable(vm).Bind(_SC("AI"), aiClass);
	RegisterInstanceFunctions(vm, &instanceClass);

//	Sqrat::Class<SceneryObject> sceneryObjectClass(vm, _SC("SceneryObject"), true);
//	Sqrat::RootTable(vm).Bind(_SC("SceneryObject"), sceneryObjectClass);
//	sceneryObjectClass.Var(_SC("asset"), &SceneryObject::Asset);
//	sceneryObjectClass.Var(_SC("name"), &SceneryObject::Name);
//	sceneryObjectClass.Var(_SC("id"), &SceneryObject::ID);

	Sqrat::RootTable(vm).SetInstance(_SC("inst"), this);

	// Form objects
	Sqrat::Class<FormDefinition> formClass(vm, "Form", true);
	formClass.Ctor<string>();
	formClass.Ctor<string, string>();
	Sqrat::RootTable(vm).Bind(_SC("Form"), formClass);
	formClass.Var("title", &FormDefinition::mTitle);
	formClass.Var("description", &FormDefinition::mDescription);
	formClass.Func("add_row", &FormDefinition::AddRow);

	Sqrat::Class<FormRow> formRowClass(vm, "FormRow", true);
	formRowClass.Ctor();
	formRowClass.Ctor<string>();
	Sqrat::RootTable(vm).Bind(_SC("FormRow"), formRowClass);
	formRowClass.Var("group", &FormRow::mGroup);
	formRowClass.Var("height", &FormRow::mHeight);
	formRowClass.Func("add_item", &FormRow::AddItem);

	Sqrat::Class<FormItem> formItemClass(vm, "FormItem", true);
	formItemClass.Ctor<string, int>();
	formItemClass.Ctor<string, int, string>();
	formItemClass.Ctor<string, int, string, int>();
	Sqrat::RootTable(vm).Bind(_SC("FormItem"), formItemClass);
	formItemClass.Var("name", &FormItem::mName);
	formItemClass.Var("type", &FormItem::mType);
	formItemClass.Var("value", &FormItem::mValue);
	formItemClass.Var("cells", &FormItem::mCells);
	formItemClass.Var("width", &FormItem::mWidth);
	formItemClass.Var("style", &FormItem::mStyle);

	Sqrat::ConstTable(vm).Const(_SC("FORM_BLANK"), BLANK);
	Sqrat::ConstTable(vm).Const(_SC("FORM_LABEL"), LABEL);
	Sqrat::ConstTable(vm).Const(_SC("FORM_TEXTFIELD"), TEXTFIELD);
	Sqrat::ConstTable(vm).Const(_SC("FORM_CHECKBOX"), CHECKBOX);
	Sqrat::ConstTable(vm).Const(_SC("FORM_BUTTON"), BUTTON);
}

bool InstanceNutPlayer::DisbandVirtualParty(int partyID) {
	ActiveParty *party = g_PartyManager.GetPartyByID(partyID);
	if (party != NULL) {
		g_PartyManager.DoDisband(partyID);
		PartyMember *leader = party->GetMemberByID(party->mLeaderID);
		if (leader != NULL) {
			g_PartyManager.DoQuit(leader->mCreaturePtr);
		}
		return true;
	}
	return false;
}

bool InstanceNutPlayer::AddToVirtualParty(int partyID, int CID) {
	ActiveParty * party = g_PartyManager.GetPartyByID(partyID);
	if (party == NULL)
		g_Logs.script->error(
				"Cannot add %v to party with ID %v because it doesn't exist.",
				CID, partyID);
	else {
		CreatureInstance *ci = GetCreaturePtr(CID);
		if (ci == NULL)
			g_Logs.script->error(
					"Cannot add %v to party with ID %v because they CID doesn't exist.",
					CID, partyID);
		else {
			char WriteBuf[512];
			party->AddMember(ci);
			party->RebroadCastMemberList(WriteBuf);
			return true;
		}
	}
	return false;
}

int InstanceNutPlayer::GetVirtualPartySize(int partyID) {
	ActiveParty * party = g_PartyManager.GetPartyByID(partyID);
	return party == NULL ? 0 : party->mMemberList.size();
}

vector<int> InstanceNutPlayer::GetVirtualPartyMembers(int partyID) {
	ActiveParty * party = g_PartyManager.GetPartyByID(partyID);
	vector<int> p;
	if (party == NULL)
		g_Logs.script->error(
				"Cannot get party members for ID %v because it doesn't exist.",
				partyID);
	else
		for (size_t i = 0; i < party->mMemberList.size(); i++)
			p.push_back(party->mMemberList[i].mCreatureID);
	return p;
}

int InstanceNutPlayer::GetVirtualPartyLeader(int partyID) {
	ActiveParty * party = g_PartyManager.GetPartyByID(partyID);
	return party == NULL ? 0 : party->mLeaderID;
}

bool InstanceNutPlayer::QuitParty(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci == NULL)
		g_Logs.script->error(
				"Cannot add quit %v from party as the CID doesn't exist.", CID);
	else {
		g_PartyManager.DoQuit(ci);
		return true;
	}
	return false;
}

int InstanceNutPlayer::PVPStart(int type) {
	PVP::PVPGame * game = actInst->StartPVP(type);
	return game != NULL ? game->mId : 0;
}

bool InstanceNutPlayer::PVPGoal(int cid) {
	CreatureInstance *ci = GetCreaturePtr(cid);
	if (ci != NULL) {
		ci->ProcessPVPGoal();
		return true;
	}
	g_Logs.script->error(
			"Failed to increase goal for %v, creature instance not found.",
			cid);
	return false;
}

bool InstanceNutPlayer::PVPStop() {
	return actInst->StopPVP();
}

int InstanceNutPlayer::CreateParty(int leaderCID) {
	ActiveParty * party = DoCreateParty(leaderCID, -1);
	return party == NULL ? -1 : party->mPartyID;
}

int InstanceNutPlayer::CreateTeam(int leaderCID, int team) {
	if (actInst->pvpGame == NULL) {
		g_Logs.script->error(
				"Request to create team when there is no PVP game opened.");
		return -1;
	}
	ActiveParty * party = DoCreateParty(leaderCID, team);
	if (party == NULL)
		return -1;
	actInst->pvpGame->mTeams[team] = party;
	return party->mPartyID;
}

ActiveParty * InstanceNutPlayer::DoCreateParty(int leaderCID, int team) {
	CreatureInstance *ci = GetCreaturePtr(leaderCID);
	if (ci != NULL) {
		if (ci->PartyID != 0) {
			g_Logs.script->error("%v is already in party %v.", leaderCID,
					ci->PartyID);
			return NULL;
		}
		ActiveParty * party = g_PartyManager.CreateParty(ci);
		if (party != NULL) {
			if (team > -1)
				party->mPVPTeam = team;
			party->mPartyID = g_PartyManager.GetNextPartyID(); //leader->CreatureDefID;
			char WriteBuf[512];
			party->AddMember(ci);
			party->RebroadCastMemberList(WriteBuf);
			return party;
		}
	}
	g_Logs.script->error("Failed to create party %v.", leaderCID);
	return NULL;
}

//int PartyManager :: AcceptInvite(CreatureInstance* member, CreatureInstance* leader)
//{
//	ActiveParty *party = GetPartyByLeader(leader->CreatureDefID);
//	if(party == NULL)
//	{
//		party = CreateParty(leader);
//		if(party != NULL)
//		{
//			party->mPartyID = GetNextPartyID();  //leader->CreatureDefID;
//			party->AddMember(leader);
//			party->AddMember(member);
//			party->RebroadCastMemberList(WriteBuf);
//			return 1;
//		}
//	}
//	else
//	{
//		party->AddMember(member);
//		return 2;
//	}
//	return 0;
//
//	/*
//	//A player has accepted a party invitation.
//	ActiveParty *party = GetPartyByLeader(leader->CreatureDefID);
//	if(party == NULL)
//		return 0;
//	party->AddMember(member);
//	return party->mPartyID;
//	*/
//}
void InstanceNutPlayer::ClearTarget(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL) {
		ci->SelectTarget(NULL);
		ci->SetAutoAttack(NULL, 0);
	} else
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to clear targets from.",
				CID);
}

void InstanceNutPlayer::DetachItem(int CID, const char *type,
		const char *node) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL) {
		ci->DetachItem(type, node);
	} else
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to detach item %v (%v) from.",
				CID, type, node);
}

void InstanceNutPlayer::AttachItem(int CID, const char *type,
		const char *node) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL) {
		ci->AttachItem(type, node);
	} else
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to attach item %v (%v) to.",
				CID, type, node);
}

void InstanceNutPlayer::UnremoveProps() {
	list<int>::iterator it;
	for (it = actInst->RemovedProps.begin(); it != actInst->RemovedProps.end();
			++it)
		DoUnremoveProp(*it);
	actInst->RemovedProps.clear();
}

void InstanceNutPlayer::UnremoveProp(int propID) {
	list<int>::iterator found = find(actInst->RemovedProps.begin(),
			actInst->RemovedProps.end(), propID);
	if (found != actInst->RemovedProps.end()) {
		DoUnremoveProp(propID);
		actInst->RemovedProps.erase(found);
	}
}

void InstanceNutPlayer::DoUnremoveProp(int propID) {
	SceneryObject *propPtr = NULL;
	g_SceneryManager.GetThread("InstanceNutPlayer::DoUnremoveProp");
	propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, propID, NULL);
	g_SceneryManager.ReleaseThread();
	if (propPtr == NULL)
		return;

	bool isSpawnPoint = propPtr->IsSpawnPoint();

	if (isSpawnPoint == true)
		actInst->spawnsys.UpdateSpawnPoint(propPtr);

	char SendBuf[512];
	int wpos = PrepExt_UpdateScenery(SendBuf, propPtr);
	actInst->LSendToLocalSimulator(SendBuf, wpos, propPtr->LocationX,
			propPtr->LocationZ);
}

bool InstanceNutPlayer::RemoveProp(int propID) {
	/* This doesn't physically delete the prop, but will send delete events to all local simulators.
	 * It also adds the prop to a list that is maintained in the instance of props that have been
	 * deleted, to prevent the prop from being sent to the client again until the instance is
	 * removed or the script stopped (at which point the prop will be re-added)
	 */

	SceneryObject *propPtr = NULL;
	g_SceneryManager.GetThread("InstanceNutPlayer::RemoveProp");
	propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, propID, NULL);
	g_SceneryManager.ReleaseThread();
	if (propPtr == NULL)
		return false;

	//Spawn point must be deactivated before it is deleted, otherwise pointers
	//will be invalidated and it may crash.
	actInst->spawnsys.RemoveSpawnPoint(propID);

	//Generate a bare prop that only has the necessary data for a delete
	//operation.
	SceneryObject prop;
	prop.ID = propID;
	prop.Asset.clear();
	char SendBuf[512];
	int wpos = PrepExt_UpdateScenery(SendBuf, &prop);
	actInst->LSendToLocalSimulator(SendBuf, wpos, propPtr->LocationX,
			propPtr->LocationZ);

	/* Mark the prop as removed in the instance so it doesn't get sent again until either
	 * the prop(s) are 'unremoved' or the script is stopped.
	 */
	actInst->RemovedProps.push_back(propID);

	return true;
}

void InstanceNutPlayer::PlaySound(const char *name) {
	STRINGLIST sub;
	Util::Split(name, "|", sub);
	while (sub.size() < 2) {
		sub.push_back("");
	}
	actInst->SendPlaySound(sub[0].c_str(), sub[1].c_str());
}

bool InstanceNutPlayer::InviteQuest(int CID, int questID, bool inviteParty) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	bool ok = false;
	if (ci != NULL && ci->simulatorPtr != NULL) {
		if (!ci->simulatorPtr->QuestInvite(questID)) {
			g_Logs.script->error("%v could not be invited the quest %v.", CID,
					questID);
		} else
			ok = true;

		if (inviteParty && ci->PartyID > 0) {
			ActiveParty* party = g_PartyManager.GetPartyByID(ci->PartyID);
			if (party != NULL) {
				vector<PartyMember>::iterator it;
				for (it = party->mMemberList.begin();
						it != party->mMemberList.end(); ++it) {
					PartyMember m = *it;
					if (m.IsOnlineAndValid())
						m.mCreaturePtr->simulatorPtr->QuestInvite(questID);
				}
				ok = true;
			}
		}

	} else {
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to invite quest %v.",
				CID, questID);
	}
	return ok;
}

bool InstanceNutPlayer::AttachSidekick(int playerCID, int sidekickCID,
		int summonType) {
	CreatureInstance *ci = GetCreaturePtr(playerCID);
	CreatureInstance *si = GetCreaturePtr(sidekickCID);
	if (ci == NULL || si == NULL)
		return false;

	// TODO check if sidekick is already attached

	SidekickObject skobj(si->CreatureDefID);
	skobj.summonType = summonType;

	ci->charPtr->AddSidekick(skobj);

	si->AnchorObject = ci;
	si->css.aggro_players = 0;
	si->SetServerFlag(ServerFlags::IsSidekick, true);
	if (skobj.summonType == SidekickObject::ABILITY)
		si->CAF_RunSidekickStatFilter(skobj.summonParam);
	else if (skobj.summonType == SidekickObject::QUEST)
		si->CAF_RunSidekickStatFilter(skobj.summonParam);
	else if (skobj.summonType == SidekickObject::PET)
		si->SetServerFlag(ServerFlags::Noncombatant, true);

	skobj.CID = si->CreatureID;
	si->_AddStatusList(StatusEffects::INVINCIBLE, -1);
	si->_AddStatusList(StatusEffects::UNATTACKABLE, -1);

	return true;
}

bool InstanceNutPlayer::AdvanceQuest(int CID, int questID, int act,
		int objective, int outcome) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	bool ok = false;
	if (ci != NULL && ci->simulatorPtr != NULL) {
		QuestReference *ref = ci->charPtr->questJournal.activeQuests.GetItem(
				questID);
		if (ref != NULL && ref->CurAct == act) {
			QuestAct act = ref->DefPtr->actList[ref->CurAct];
			if (objective >= 0 && objective < MAXOBJECTIVES) {
				char buffer[256];
				int wpos = ref->CheckQuestObjective(CID, buffer,
						act.objective[objective].type, ci->CreatureDefID,
						ci->CreatureID);
				ci->simulatorPtr->AttemptSend(buffer, wpos);
				ok = true;
			}
		}
	}
	return ok;

}

bool InstanceNutPlayer::JoinQuest(int CID, int questID, bool joinParty) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	bool ok = false;
	if (ci != NULL && ci->simulatorPtr != NULL) {
		if (!ci->simulatorPtr->QuestJoin(questID)) {
			g_Logs.script->error("%v could not be invited the quest %v.", CID,
					questID);
		} else
			ok = true;

		if (joinParty && ci->PartyID > 0) {
			ActiveParty* party = g_PartyManager.GetPartyByID(ci->PartyID);
			if (party != NULL) {
				vector<PartyMember>::iterator it;
				for (it = party->mMemberList.begin();
						it != party->mMemberList.end(); ++it) {
					PartyMember m = *it;
					if (m.IsOnlineAndValid())
						m.mCreaturePtr->simulatorPtr->QuestJoin(questID);
				}
				ok = true;
			}
		}

	} else {
		g_Logs.script->error(
				"Could not find creature with ID %v in this instance to invite quest %v.",
				CID, questID);
	}
	return ok;
}


void InstanceNutPlayer::SetSize(int CID, float scale) {
	CreatureInstance *ci = GetNPCPtr(CID);
	if(ci != NULL)
	{
		char buf[20];
		Util::SafeFormat(buf,sizeof(buf),"%f",scale);
		CreatureAttributeModifier *cam = new CreatureAttributeModifier("sz", buf);
		ci->PushAppearanceModifier(cam);
	}
	else
		g_Logs.script->warn("Request to size creature that doesn't exist, [%v] to %v", CID, scale);
}

int InstanceNutPlayer::Transform(int propID, Sqrat::Table transformation) {
	char buffer[256];
	SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone,
			propID, NULL);
	if (propPtr != NULL) {
		SceneryEffectList *effectList = actInst->GetSceneryEffectList(propID);
		SceneryEffect l;
		l.type = TRANSFORMATION;
		l.tag = ++actInst->mNextEffectTag;
		l.propID = propID;

		Squirrel::Printer printer;
		string transformString;
		printer.PrintTable(&transformString, transformation);

		l.effect = transformString.c_str();

		effectList->push_back(l);
		activeEffects.push_back(l);
		int wpos = actInst->AddSceneryEffect(buffer, &l);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
		g_Logs.script->debug("Create effect tag %v for prop %v.", l.tag,
				propID);
		return l.tag;
	} else
		g_Logs.script->error("Could not find prop with ID %v in this instance.",
				propID);
	return -1;
}

int InstanceNutPlayer::Asset(int propID, const char *newAsset, float scale) {
	char buffer[256];
	SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone,
			propID, NULL);
	if (propPtr != NULL) {
		SceneryEffectList *effectList = actInst->GetSceneryEffectList(propID);
		SceneryEffect l;
		l.type = ASSET_UPDATE;
		l.tag = ++actInst->mNextEffectTag;
		l.propID = propID;
		l.effect = newAsset;
		l.scale = scale;
		effectList->push_back(l);
		activeEffects.push_back(l);
		int wpos = actInst->AddSceneryEffect(buffer, &l);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
		g_Logs.script->debug("Create effect tag %v for prop %v.", l.tag,
				propID);
		return l.tag;
	} else
		g_Logs.script->error("Could not find prop with ID %v in this instance.",
				propID);
	return -1;
}

void InstanceNutPlayer::DetachSceneryEffect(int propID, int tag) {
	char buffer[256];
	SceneryEffect *tagObj = actInst->RemoveSceneryEffect(propID, tag);
	if (tagObj != NULL) {
		int wpos = actInst->DetachSceneryEffect(buffer, tagObj->propID,
				tagObj->type, tag);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
		for (vector<SceneryEffect>::iterator it = activeEffects.begin();
				it != activeEffects.end(); ++it) {
			if (it->propID == propID && it->tag == tag) {
				activeEffects.erase(it);
				return;
			}
		}
	}
}

int InstanceNutPlayer::ParticleAttach(int propID, const char *effect,
		float scale, float offsetX, float offsetY, float offsetZ) {
	char buffer[256];
	SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(actInst->mZone,
			propID, NULL);
	if (propPtr != NULL) {
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
		activeEffects.push_back(l);
		int wpos = actInst->AddSceneryEffect(buffer, &l);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
		g_Logs.script->debug("Create effect tag %v for prop %v.", l.tag,
				propID);
		return l.tag;
	} else
		g_Logs.script->error("Could not find prop with ID %v in this instance.",
				propID);
	return -1;
}

void InstanceNutPlayer::Emote(int CID, const char *emotion) {
	char buffer[4096];
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci != NULL)
		actInst->LSendToAllSimulator(buffer,
				PrepExt_GenericChatMessage(buffer, CID, ci->css.display_name,
						"emote", emotion), -1);
	else
		g_Logs.script->info(
				"Could not find creature with ID %v in this instance to emote.",
				CID);
}

AINutPlayer* InstanceNutPlayer::GetAI(int CID) {
	CreatureInstance *ci = GetNPCPtr(CID);
	return ci != NULL ? ci->aiNut : NULL;
}

int InstanceNutPlayer::GetPartyID(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	return ci != NULL ? ci->PartyID : 0;
}

int InstanceNutPlayer::GetPropIDForSpawn(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL && ci->spawnGen != NULL && ci->spawnGen->spawnPoint != NULL) {
		return ci->spawnGen->spawnPoint->ID;
	}
	return -1;
}

Squirrel::Vector3I InstanceNutPlayer::GetLocation(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL) {
		return Squirrel::Vector3I(ci->CurrentX, ci->CurrentY, ci->CurrentZ);
	}
	return Squirrel::Vector3I(0, 0, 0);

}

const char * InstanceNutPlayer::GetDisplayName(int CID) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci != NULL)
		return ci->css.display_name;
	else
		return "Unknown";
}

WalkCondition::WalkCondition(CreatureInstance *c) {
	cInst = c;
}
bool WalkCondition::CheckCondition() {
	if (cInst->CurrentTarget.DesLocX == 0 && cInst->CurrentTarget.DesLocZ == 0
			&& cInst->CurrentTarget.desiredRange == 0) {
		return true;
	}
	return false;
}

long InstanceNutPlayer::TempWalkThen(int CID, Squirrel::Point point, int facing,
		int speed, int range, Sqrat::Function onArrival) {

	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci) {
		ci->StopMovement(ScriptCore::Result::INTERRUPTED);
		return ci->scriptMoveEvent = QueueAdd(
				new ScriptCore::NutScriptEvent(new ScriptCore::NeverCondition(),
						new WalkCallback(this, ci, point, facing, speed, range,
								onArrival, true)));
	}
	return -1;
}

long InstanceNutPlayer::WalkThen(int CID, Squirrel::Point point, int facing,
		int speed, int range, Sqrat::Function onArrival) {

	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci) {
		if(mPreventReentry)
			g_Logs.script->warn("Attempt to re-enter WalkThen, ignoring.");
		else
			ci->StopScriptMovement(ScriptCore::Result::INTERRUPTED);
		return ci->scriptMoveEvent = QueueAdd(
				new ScriptCore::NutScriptEvent(new ScriptCore::NeverCondition(),
						new WalkCallback(this, ci, point, facing, speed, range,
								onArrival, false)));
	}
	return -1;
}

//void InstanceNutPlayer::Queue(Sqrat::Function function, int fireDelay)
//{
//
//	DoQueue(new ScriptCore::NutScriptEvent(
//				new ScriptCore::TimeCondition(fireDelay),
//				new ScriptCore::SquirrelFunctionCallback(this, function)));
//}

void InstanceNutPlayer::WarpPlayer(int CID, int zoneID) {
	CreatureInstance *ci = actInst->GetPlayerByID(CID);
	if (ci) {
		ci->simulatorPtr->MainCallSetZone(zoneID, 0, true);
	}
	/*HaltExecution();
	 source->simulatorPtr->MainCallSetZone(zoneID, 0, true);
	 */
}

long InstanceNutPlayer::Walk(int CID, Squirrel::Point point, int facing,
		int speed, int range) {

	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci) {
		ci->StopMovement(ScriptCore::Result::INTERRUPTED);
		return ci->scriptMoveEvent = QueueAdd(
				new ScriptCore::NutScriptEvent(new ScriptCore::NeverCondition(),
						new WalkCallback(this, ci, point, facing, speed, range,
								false)));
	}
	return -1;
}

bool InstanceNutPlayer::SetStatusEffect(int CID, const char *effect,
		long durationMS) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci) {
		int EffectID = GetStatusIDByName(effect);
		if (EffectID != -1) {
			ci->_AddStatusList(EffectID, durationMS);
			return true;
		}

	}
	return false;
}

bool InstanceNutPlayer::RemoveStatusEffect(int CID, const char *effect) {
	CreatureInstance *ci = GetCreaturePtr(CID);
	if (ci) {
		int EffectID = GetStatusIDByName(effect);
		if (EffectID != -1) {
			ci->_RemoveStatusList(EffectID);
			return true;
		}

	}
	return false;
}

bool InstanceNutPlayer::Despawn(int CID) {
	g_Logs.script->debug("Despawning %v", CID);
	CreatureInstance *source = actInst->GetNPCInstanceByCID(CID);
	if (source == NULL) {
		g_Logs.script->debug("No spawn %v", CID);
		return false;
	}
	spawned.erase(remove(spawned.begin(), spawned.end(), CID),
			spawned.end());
	actInst->spawnsys.Despawn(CID);
	return true;
}

int InstanceNutPlayer::DespawnAll(int CDefID) {
	int despawned = 0;
	vector<int> v;
	actInst->GetNPCInstancesByCDefID(CDefID, &v);
	for (vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
		actInst->spawnsys.Despawn(*it);
		spawned.erase(remove(spawned.begin(), spawned.end(), *it),
				spawned.end());
		despawned++;
	}
	return despawned;
}

int InstanceNutPlayer::GetHealthPercent(int cid) {
	CreatureInstance *ci = GetNPCPtr(cid);
	if (ci)
		return static_cast<int>(ci->GetHealthRatio() * 100.0F);
	return -1;
}

int InstanceNutPlayer::CDefIDForCID(int cid) {
	CreatureInstance *ci = GetNPCPtr(cid);
	if (ci)
		return ci->CreatureDefID;
	return -1;
}

int InstanceNutPlayer::CountAlive(int CDefID) {
	return actInst->CountAlive(CDefID);
}

bool InstanceNutPlayer::AI(int CID, const char *label) {
	CreatureInstance *ci = GetNPCPtr(CID);
	return ci
			&& ((ci->aiScript && ci->aiScript->JumpToLabel(label))
					|| (ci->aiNut && ci->aiNut->JumpToLabel(label)));
}

int InstanceNutPlayer::GetTarget(int CID) {
	int targetID = 0;
	CreatureInstance *ci = GetNPCPtr(CID);
	if (ci) {
		if (ci->CurrentTarget.targ != NULL)
			targetID = ci->CurrentTarget.targ->CreatureID;
	}
	return targetID;
}

bool InstanceNutPlayer::SetTarget(int CID, int targetCID) {
	CreatureInstance *source = actInst->GetInstanceByCID(CID);
	CreatureInstance *target = actInst->GetInstanceByCID(targetCID);
	if (source != NULL && target != NULL) {
		source->SelectTarget(target);
		return true;
	}
	return false;
}

int InstanceNutPlayer::LoadSpawnTile(Squirrel::Point location) {
	actInst->spawnsys.GenerateTile(location.mX, location.mZ);
	return 1;
}

int InstanceNutPlayer::LoadSpawnTileFor(Squirrel::Point location) {
	return LoadSpawnTile(
			Squirrel::Point(location.mX / SpawnTile::SPAWN_TILE_SIZE,
					location.mZ / SpawnTile::SPAWN_TILE_SIZE));
}

int InstanceNutPlayer::SpawnProp(int propID) {
	return Spawn(propID, 0, 0);
}

int InstanceNutPlayer::Spawn(int propID, int creatureID, int flags) {
	int res = actInst->spawnsys.TriggerSpawn(propID, creatureID, flags);
	if (res > -1) {
		spawned.push_back(res);
	}
	return res;
}

int InstanceNutPlayer::SpawnAt(int creatureID, Squirrel::Vector3I location,
		int facing, int flags) {
	CreatureInstance *creature = actInst->SpawnGeneric(creatureID, location.mX,
			location.mY, location.mZ, facing, flags);
	if (creature == NULL)
		return -1;
	else {
		genericSpawned.push_back(creature->CreatureID);
		g_Logs.script->debug("Spawn at returned temp creature ID of %v",
				creature->CreatureID);
		return creature->CreatureID;
	}
}

int InstanceNutPlayer::OLDSpawnAt(int creatureID, float x, float y, float z,
		int facing, int flags) {
	CreatureInstance *creature = actInst->SpawnGeneric(creatureID, x, y, z,
			facing, flags);
	if (creature == NULL)
		return -1;
	else {
		genericSpawned.push_back(creature->CreatureID);
		g_Logs.script->debug("Spawn at returned temp creature ID of %v",
				creature->CreatureID);
		return creature->CreatureID;
	}
}

//
//
//

fs::path InstanceScriptDef::GetInstanceTslScriptPath(int zoneID) {
	return g_Config.ResolveVariableDataPath() / "Instance" / Util::Format("%d", zoneID) / "Script.txt";
}

void InstanceScriptDef::GetExtendedOpCodeTable(OpCodeInfo **arrayStart,
		size_t &arraySize) {
	*arrayStart = InstanceScript::extCoreOpCode;
	arraySize = InstanceScript::maxExtOpCode;
}

void InstanceScriptDef::SetMetaDataDerived(const char *opname,
		ScriptCore::ScriptCompiler &compileData) {
	STRINGLIST &tokens = compileData.mTokens;
	if (strcmp(opname, "#location") == 0) {
		//#location name x1 y1 x2 y2
		if (compileData.ExpectTokens(6, "#location",
				"str:name int:x1 int:z1 int:x2 int:z2") == true) {
			Squirrel::Area &loc = mLocationDef[tokens[1]];
			loc.mX1 = Util::GetInteger(tokens, 2);
			loc.mY1 = Util::GetInteger(tokens, 3);
			loc.mX2 = Util::GetInteger(tokens, 4);
			loc.mY2 = Util::GetInteger(tokens, 5);
		}
	} else if (strcmp(opname, "#location_br") == 0) {
		if (compileData.ExpectTokens(5, "location_br",
				"str:name int:x int:z int:radius") == true) {
			Squirrel::Area &loc = mLocationDef[tokens[1]];
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

bool InstanceScriptDef::HandleAdvancedCommand(const char *commandToken,
		ScriptCore::ScriptCompiler &compileData) {
	STRINGLIST &tokens = compileData.mTokens;
	bool retVal = false;
	if (strcmp(commandToken, "spawnloc") == 0) {
		retVal = true;
		if (tokens.size() < 5) {
			g_Logs.script->error(
					"Syntax error: not enough operands for SPAWNLOC statement (%v line %v",
					compileData.mSourceFile, compileData.mLineNumber);
		} else {
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

Squirrel::Area* InstanceScriptDef::GetLocationByName(const char *location) {
	map<string, Squirrel::Area>::iterator it;
	it = mLocationDef.find(location);
	if (it == mLocationDef.end())
		return NULL;
	return &it->second;
}

void InstanceScriptPlayer::RunImplementationCommands(int opcode) {
	ScriptCore::OpData *instr = &def->instr[curInst];
	switch (opcode) {
	case OP_PARTICLE_ATTACH: {
		char buffer[256];
		SceneryObject *propPtr = g_SceneryManager.GlobalGetPropPtr(
				actInst->mZone, instr->param1, NULL);
		if (propPtr != NULL) {
			SceneryEffectList *effectList = actInst->GetSceneryEffectList(
					instr->param1);
			SceneryEffect l;
			l.tag = ++actInst->mNextEffectTag;
			l.propID = instr->param1;
			l.effect = GetStringPtr(instr->param2);
			l.scale = (float) instr->param3 / 100.0;
			effectList->push_back(l);
			int wpos = actInst->AddSceneryEffect(buffer, &l);
			actInst->LSendToAllSimulator(buffer, wpos, -1);
			PushVarStack(l.tag);
			g_Logs.script->debug("Create effect tag %v for prop %v.", l.tag,
					instr->param1);
		} else
			g_Logs.script->error(
					"Could not find prop with ID %v in this instance.",
					instr->param1);
		break;
	}
	case OP_PARTICLE_DETACH: {
		char buffer[256];
		SceneryEffect *tag = actInst->RemoveSceneryEffect(instr->param1,
				GetVarValue(instr->param2));
		if (tag != NULL) {
			int wpos = actInst->DetachSceneryEffect(buffer, tag->propID,
					tag->type, tag->tag);
			actInst->LSendToAllSimulator(buffer, wpos, -1);
		} else {
			Util::SafeFormat(buffer, sizeof(buffer),
					"Could not find effect tag with ID %d in this instance.",
					instr->param1);
			int wpos = PrepExt_SendInfoMessage(buffer,
					GetStringPtr(instr->param1), INFOMSG_INFO);
			actInst->LSendToAllSimulator(buffer, wpos, -1);
		}
		break;
	}
	case OP_DESPAWN_ALL: {
		while (true) {
			CreatureInstance *source = actInst->GetNPCInstanceByCDefID(
					GetVarValue(instr->param1));
			if (source == NULL)
				break;
			else {
				g_Logs.script->debug("Despawn: %v (%v)",
						GetVarValue(instr->param1), source->CreatureID);
				actInst->spawnsys.Despawn(source->CreatureID);
			}
		}
		break;
	}
	case OP_DESPAWN: {
		CreatureInstance *source = actInst->GetInstanceByCID(
				GetVarValue(instr->param1));
		g_Logs.script->debug("Despawn: %v (%v)", GetVarValue(instr->param1),
				source->CreatureDefID);
		if (source == NULL)
			g_Logs.script->error("Despawn failed, %v does not exist.",
					GetVarValue(instr->param1));
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
		actInst->spawnsys.TriggerSpawn(instr->param1, instr->param2,
				instr->param3);
		//g_Log.AddMessageFormat("Fired spawn: %d, creature: %d", def->instr[curInst].param1, def->instr[curInst].param2);
		break;
	case OP_SPAWNLOC: {
		//Pop in reverse order
		int x = PopVarStack();
		int y = PopVarStack();
		int z = PopVarStack();
		if (x != 0 && y != 0 && z != 0)
			actInst->SpawnGeneric(instr->param1, x, y, z, 0, 0);
	}
		break;
	case OP_COUNTALIVE:
		vars[instr->param2] = actInst->CountAlive(instr->param1);
		break;
	case OP_GETNPCID: {
		int creatureID = 0;
		CreatureInstance *search = actInst->GetNPCInstanceByCDefID(
				instr->param1);
		if (search != NULL)
			creatureID = search->CreatureID;
		vars[instr->param2] = creatureID;
	}
		break;
	case OP_SETTARGET: {
		CreatureInstance *source = actInst->GetInstanceByCID(
				GetVarValue(instr->param1));
		CreatureInstance *target = actInst->GetInstanceByCID(
				GetVarValue(instr->param2));
		if (source != NULL && target != NULL)
			source->SelectTarget(target);
	}
		break;

	case OP_SCAN_NPC_CID: {
		Squirrel::Area *loc = GetLocationByName(GetStringPtr(instr->param1));
		int index = VerifyIntArrayIndex(instr->param2);
		if (index >= 0)
			ScanNPCCID(loc, intArray[index].arrayData);
	}
		break;
	case OP_SCAN_NPC_CID_FOR: {
		Squirrel::Area *loc = GetLocationByName(GetStringPtr(instr->param1));
		int index = VerifyIntArrayIndex(instr->param2);
		if (index >= 0)
			ScanNPCCIDFor(loc, instr->param3, intArray[index].arrayData);
	}
		break;
	case OP_GET_CDEF: {
		int cdef = 0;
		CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
		if (ci)
			cdef = ci->CreatureDefID;
		SetVar(instr->param2, cdef);
	}
		break;
	case OP_GET_HEALTH_PERCENT: {
		CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
		int health = 0;
		if (ci)
			health = static_cast<int>(ci->GetHealthRatio() * 100.0F);
		SetVar(instr->param2, health);
	}
		break;
	case OP_ORDER_WALK: {
		CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
		if (ci) {
			ci->SetServerFlag(ServerFlags::ScriptMovement, true);
			ci->previousPathNode = 0;   //Disable any path links.
			ci->nextPathNode = 0;
			ci->tetherNodeX = instr->param2;
			ci->tetherNodeZ = instr->param3;
			ci->tetherFacing = 0;
			ci->CurrentTarget.DesLocX = instr->param2;
			ci->CurrentTarget.DesLocZ = instr->param3;
			ci->CurrentTarget.desiredRange = 30;
			ci->Speed = ci->CurrentTarget.desiredSpeed = 20;
		}
	}
		break;
	case OP_GET_TARGET: {
		int targetID = 0;
		CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
		if (ci) {
			if (ci->CurrentTarget.targ != NULL)
				targetID = ci->CurrentTarget.targ->CreatureID;
		}
		SetVar(instr->param2, targetID);
	}
		break;
	case OP_AI_SCRIPT_JUMP: {
		CreatureInstance *ci = GetNPCPtr(GetVarValue(instr->param1));
		if (ci) {
			if (ci->aiScript)
				ci->aiScript->JumpToLabel(GetStringPtr(instr->param2));
		}
	}
		break;
	case OP_INFO: {
		char buffer[4096];
		int wpos = PrepExt_SendInfoMessage(buffer, GetStringPtr(instr->param1),
				INFOMSG_INFO);
		actInst->LSendToAllSimulator(buffer, wpos, -1);
	}
		break;
	case OP_CHAT: {
		char buffer[4096];
		int wpos = PrepExt_GenericChatMessage(buffer, 0,
				GetStringPtr(instr->param1), GetStringPtr(instr->param2),
				GetStringPtr(instr->param3));
		actInst->LSendToAllSimulator(buffer, wpos, -1);
	}
		break;
	case OP_BROADCAST: {
		char buffer[4096];
		if (actInst->mZoneDefPtr->mGrove) {
			int wpos = PrepExt_SendInfoMessage(buffer,
					GetStringPtr(instr->param1), INFOMSG_INFO);
			actInst->LSendToAllSimulator(buffer, wpos, -1);
		} else {
			int wpos = PrepExt_Broadcast(buffer, GetStringPtr(instr->param1));
			actInst->LSendToAllSimulator(buffer, wpos, -1);
		}
	}
		break;
	default:
		g_Logs.script->error("Unidentified InstanceScriptPlayer OpCode: %v",
				instr->opCode);
		break;
	}
}

void InstanceScriptPlayer::SetInstancePointer(ActiveInstance *parent) {
	actInst = parent;
}

Squirrel::Area* InstanceScriptPlayer::GetLocationByName(const char *name) {
	if (name == NULL)
		return NULL;
	InstanceScriptDef *thisDef = dynamic_cast<InstanceScriptDef*>(def);
	return thisDef->GetLocationByName(name);
}

void InstanceScriptPlayer::ScanNPCCIDFor(Squirrel::Area *location, int CDefID,
		vector<int>& destResult) {
	destResult.clear();
	if (actInst == NULL || location == NULL)
		return;
	for (size_t i = 0; i < actInst->NPCListPtr.size(); i++) {
		CreatureInstance *ci = actInst->NPCListPtr[i];
		if (ci->CreatureDefID != CDefID)
			continue;
		if (ci->CurrentX < location->mX1)
			continue;
		if (ci->CurrentX > location->mX2)
			continue;
		if (ci->CurrentZ < location->mY1)
			continue;
		if (ci->CurrentZ > location->mY2)
			continue;
		destResult.push_back(ci->CreatureID);
	}
}

void InstanceScriptPlayer::ScanNPCCID(Squirrel::Area *location,
		vector<int>& destResult) {
	destResult.clear();
	if (actInst == NULL || location == NULL)
		return;
	for (size_t i = 0; i < actInst->NPCListPtr.size(); i++) {
		CreatureInstance *ci = actInst->NPCListPtr[i];
		if (ci->CurrentX < location->mX1)
			continue;
		if (ci->CurrentX > location->mX2)
			continue;
		if (ci->CurrentZ < location->mY1)
			continue;
		if (ci->CurrentZ > location->mY2)
			continue;
		destResult.push_back(ci->CreatureID);
	}
}

CreatureInstance* InstanceScriptPlayer::GetNPCPtr(int CID) {
	if (actInst == NULL)
		return NULL;
	return actInst->GetNPCInstanceByCID(CID);
}

void InstanceNutPlayer::AddInteraction(CreatureInstance *creature, ScriptCore::NutScriptEvent *evt) {
	interactions.push_back(ActiveInteraction(creature, evt));
}

bool InstanceNutPlayer::Interact(int CID, const char *text, int time,
		bool gather, Sqrat::Function function) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature == NULL)
		return false;
	else {
		char strBuf[1024];
		int wpos = creature->QuestInteractObject(strBuf, text, time, gather);
		creature->simulatorPtr->AttemptSend(strBuf, wpos);
		ScriptCore::NutScriptEvent *evt = new ScriptCore::NutScriptEvent(
				new ScriptCore::TimeCondition(time),
				new InteractCallback(this, function, CID));
		AddInteraction(creature, evt);
		QueueAdd(evt);
		return true;
	}
}

void InstanceNutPlayer::RemoveInteraction(int CID) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature != NULL) {
		vector<ActiveInteraction>::iterator eit;
		for (eit = interactions.begin(); eit != interactions.end(); ++eit) {
			if (creature == eit->mCreature) {
				interactions.erase(eit);
				return;
			}
		}
	}
}

void InstanceNutPlayer::CloseForm(int CID, int formId) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature == NULL || creature->simulatorPtr == NULL)
		return;

	map<int, int>::iterator it = openedForms.find(formId);
	if (it != openedForms.end()) {
		openedForms.erase(it);
	}
	char buf[32];
	creature->simulatorPtr->AttemptSend(buf,
			PrepExt_SendFormClose(buf, formId));
}

void InstanceNutPlayer::OpenForm(int CID, FormDefinition form) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature == NULL || creature->simulatorPtr == NULL)
		return;

	openedForms.insert(map<int, int>::value_type(form.mId, CID));
	char buf[4096];
	creature->simulatorPtr->AttemptSend(buf, PrepExt_SendFormOpen(buf, form));
}

bool InstanceNutPlayer::OpenBook(int CID, int id, int page, bool refresh) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature == NULL || creature->simulatorPtr == NULL)
		return false;

	char buf[64];
	creature->simulatorPtr->AttemptSend(buf,
			PrepExt_SendBookOpen(buf, id, page - 1, refresh ? 2 : 1));
	return false;
}

bool InstanceNutPlayer::HasItem(int CID, int itemID) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature == NULL || creature->simulatorPtr == NULL)
		return false;

	return creature->charPtr->inventory.GetItemPtrByID(itemID) != NULL;
}

bool InstanceNutPlayer::GiveItem(int CID, int itemID) {
	if (actInst->mZoneDefPtr->mGrove)
		return false;

	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature == NULL || creature->simulatorPtr == NULL)
		return false;

	ItemDef *item = g_ItemManager.GetSafePointerByID(itemID);
	if (item->mID == 0)
		return false;

	int slot = creature->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1)
		return false;

	InventorySlot *sendSlot = creature->charPtr->inventory.AddItem_Ex(
			INV_CONTAINER, item->mID, 1);
	if (sendSlot != NULL) {
		creature->charPtr->pendingChanges++;
		creature->simulatorPtr->ActivateActionAbilities(sendSlot);
		char buf[128];
		char buf2[64];
		int wpos = AddItemUpdate(buf, buf2, sendSlot);
		creature->simulatorPtr->AttemptSend(buf, wpos);
	}

	return true;
}

void InstanceNutPlayer::InterruptInteraction(int CID) {
	CreatureInstance *creature = actInst->GetInstanceByCID(CID);
	if (creature != NULL) {
		vector<ActiveInteraction>::iterator eit;
		for (eit = interactions.begin(); eit != interactions.end(); ++eit) {
			if (creature == eit->mCreature) {
				interactions.erase(eit);
				QueueRemove(eit->mEvent);
				break;
			}
		}
	} else {
		g_Logs.server->warn("Interrupt on a creature that doesn't exist. %v",
				CID);
	}
	vector<ScriptCore::ScriptParam> p;
	p.push_back(ScriptCore::ScriptParam(CID));
	RunFunction("on_interrupt", p, false);
	return;
}

//
// InteractCallback
//

InteractCallback::InteractCallback(InstanceNutPlayer *nut,
		Sqrat::Function function, int CID) {
	mNut = nut;
	mFunction = function;
	mCID = CID;
}

InteractCallback::~InteractCallback() {
}

bool InteractCallback::Execute() {
	mNut->RemoveInteraction(mCID);
	Sqrat::SharedPtr<bool> ptr = mFunction.Evaluate<bool>();
	return ptr.Get() == NULL || ptr.Get();
}

//
// WallCallback
//

WalkCallback::WalkCallback(ScriptCore::NutPlayer *nut,
		CreatureInstance *creature, Squirrel::Point point, int facing,
		int speed, int range, bool reset) {
	Init(nut, creature, point, facing, speed, range, reset);
}

WalkCallback::WalkCallback(ScriptCore::NutPlayer *nut,
		CreatureInstance *creature, Squirrel::Point point, int facing,
		int speed, int range, Sqrat::Function onArrival, bool reset) {
	mFunction = onArrival;
	Init(nut, creature, point, facing, speed, range, reset);
}

void WalkCallback::Init(ScriptCore::NutPlayer *nut, CreatureInstance *creature,
		Squirrel::Point point, int facing, int speed, int range, bool reset) {
	mNut = nut;
	mCreature = creature;
	mReset = reset;

	sPreviousPathNode = creature->previousPathNode;
	sNextPathNode = creature->nextPathNode;
	sTetherNodeX = creature->tetherNodeX;
	sTetherNodeZ = creature->tetherNodeZ;
	sTetherNodeFacing = creature->tetherFacing;

	creature->previousPathNode = 0;   //Disable any path links.
	creature->nextPathNode = 0;

	creature->tetherNodeX = point.mX;
	creature->tetherNodeZ = point.mZ;
	creature->tetherFacing = facing;

	creature->MoveTo(point.mX, point.mZ, range, speed);
	creature->SetServerFlag(ServerFlags::ScriptMovement, true);

}

WalkCallback::~WalkCallback() {
}

bool WalkCallback::Execute() {
	bool ok = false;
	if (!mFunction.IsNull()) {
		try {
			mNut->mPreventReentry = true;
			Sqrat::SharedPtr<bool> ptr = mFunction.Evaluate<bool>(mResult);
			ok = ptr.Get() == NULL || ptr.Get();
		} catch (Sqrat::Exception &e) {
			g_Logs.script->error("Exception while execute script function.");
		}
		mNut->mPreventReentry = false;

	}

	mCreature->SetServerFlag(ServerFlags::ScriptMovement, false);

//	mCreature->CurrentTarget.desiredRange = 0;
//	mCreature->CurrentTarget.desiredSpeed = 0;
//	mCreature->CurrentTarget.DesLocX = 0;
//	mCreature->CurrentTarget.DesLocZ = 0;
//	mCreature->SetServerFlag(ServerFlags::ScriptMovement, false);
//
	if (mReset) {
		mCreature->previousPathNode = sPreviousPathNode;
		mCreature->nextPathNode = sNextPathNode;
		mCreature->tetherFacing = sTetherNodeFacing;
		mCreature->tetherNodeX = sTetherNodeX;
		mCreature->tetherNodeZ = sTetherNodeZ;
	}

	mCreature ->scriptMoveEvent = -1;
	return ok;
}

//
// InstanceUseCallback
//

InstanceUseCallback::InstanceUseCallback(AbstractInstanceNutPlayer *instanceNut,
		int CID, int abilityID) {
	mInstanceNut = instanceNut;
	mAbilityID = abilityID;
	mCID = CID;
}
InstanceUseCallback::~InstanceUseCallback() {
}

bool InstanceUseCallback::Execute() {
	mInstanceNut->CreatureUse(mCID, mAbilityID);
	return true;
}

} //namespace InstanceScript

