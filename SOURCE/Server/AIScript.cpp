#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "AIScript.h"
#include "FileReader.h"
#include "StringList.h"
#include "Instance.h"
#include "Creature.h"
#include "Ability2.h"
#include "Util.h"
#include "DirectoryAccess.h"
#include "Report.h"
#include "Config.h"
#include "util/Log.h"

const int USE_FAIL_DELAY = 250;  //Milliseconds to wait before retrying a failed script "use" command.


AIScriptManager aiScriptManager;

OpCodeInfo extCoreOpCode[] = {
	{ "use",               OP_USE,            1, {OPT_INT,  OPT_NONE, OPT_NONE }},
	{ "getwill",           OP_GETWILL,        1, {OPT_VAR,  OPT_NONE, OPT_NONE }},
	{ "getwillcharge",     OP_GETWILLCHARGE,  1, {OPT_VAR,  OPT_NONE, OPT_NONE }},
	{ "getmight",          OP_GETMIGHT,       1, {OPT_VAR,  OPT_NONE, OPT_NONE }},
	{ "getmightcharge",    OP_GETMIGHTCHARGE, 1, {OPT_VAR,  OPT_NONE, OPT_NONE }},
	{ "has_target",        OP_HASTARGET,      1, {OPT_VAR,  OPT_NONE, OPT_NONE }},
	{ "getlevel",          OP_GETLEVEL,       1, {OPT_VAR,  OPT_NONE, OPT_NONE }},
	{ "debugprint",        OP_DEBUGPRINT,     1, {OPT_STR,  OPT_NONE, OPT_NONE }},
	{ "getcooldown",       OP_GETCOOLDOWN,    2, {OPT_STR,  OPT_VAR,  OPT_NONE  }},  //Check cooldown category STR and place nonzero in VAR if cooldown is still active
	{ "is_busy",           OP_ISBUSY,         1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //If casting, channeling, or otherwise using a primary skill, set VAR to 1, otherwise 0
	{ "count_enemy_near",  OP_COUNTENEMYNEAR, 2, {OPT_INT,  OPT_VAR,  OPT_NONE  }},  //Count the enemy targets within INT game units, place the number of targets in VAR
	{ "count_enemy_at",    OP_COUNTENEMYAT,   2, {OPT_INT,  OPT_VAR,  OPT_NONE  }},  //Count the number of enemies within INT game units of the current target, place the number of targets in VAR.
	{ "health_percent",    OP_HEALTHPERCENT,  1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //Calculate the mob's health percent (0 to 100) and store the result in VAR.
	{ "target_health_percent", OP_TARGETHEALTHPERCENT, 1, {OPT_VAR, OPT_NONE, OPT_NONE }},  //Calculate the mob's target health percent (0 to 100) and store the result in VAR.
	{ "set_elapsed_time",  OP_SETELAPSEDTIME, 1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //Set the current elapsed time (milliseconds) into VAR
	{ "time_offset",       OP_TIMEOFFSET,     2, {OPT_VAR,  OPT_VAR,  OPT_NONE  }},  //Take the time offset between the current time and VAR1, and store in VAR2
	{ "visual_effect",     OP_VISUALEFFECT,   1, {OPT_STR,  OPT_NONE, OPT_NONE }},  //Perform a visual effect like a skill animation by this creature in the client.
	{ "visual_effect_t",   OP_VISUALEFFECTT,  1, {OPT_STR,  OPT_NONE, OPT_NONE }},  //Perform a visual effect like a skill animation by this creature in the client.
	{ "say",               OP_SAY,            1, {OPT_STR,  OPT_NONE, OPT_NONE }},  //Peform a /say message by this creature.
	{ "instance_call",     OP_INSTANCECALL,   1, {OPT_STR,  OPT_NONE, OPT_NONE }},  //Call an instance script block by its label.
	{ "get_idle_mob",      OP_GETIDLEMOB,     2, {OPT_INT,  OPT_VAR,  OPT_NONE }},  //Get an idle ally of the creatureDef INT, and store its creatureID in VAR
	{ "get_target",        OP_GETTARGET,      1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //Get the creatureID of the current target, and store in VAR
	{ "get_self",          OP_GETSELF,        1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //Get the creatureID of self, and store in VAR
	{ "set_other_target",  OP_SETOTHERTARGET, 2, {OPT_VAR,  OPT_VAR,  OPT_NONE  }},  //Set the creatureID of VAR1 to select the target creature ID of VAR2
	{ "aiscript_call",     OP_AISCRIPTCALL,   2, {OPT_VAR,  OPT_STR,  OPT_NONE  }},  //Set the creatureID of VAR to jump to its own script label of STR
	{ "is_target_enemy",   OP_ISTARGETENEMY,  1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //If the target is an enemy (and alive), set VAR to 1.
	{ "is_target_friendly",OP_ISTARGETFRIENDLY,1, {OPT_VAR, OPT_NONE, OPT_NONE }},  //If the target is a friend (and alive), set VAR to 1.
	{ "set_speed",         OP_SETSPEED,       1, {OPT_INT,  OPT_NONE, OPT_NONE }},  //Force the movement speed to a certain amount.
	{ "get_target_cdef",   OP_GETTARGETCDEF,  1, {OPT_VAR,  OPT_NONE, OPT_NONE }},  //Place the target's creatureDefID into VAR (zero if no target)
	{ "get_property",      OP_GETPROPERTY,    2, {OPT_STR,  OPT_VAR, OPT_NONE }},   //Get the value of the creatures's current property stat name STR, load into VAR.
	{ "get_target_property",OP_GETTARGETPROPERTY,2, {OPT_STR, OPT_VAR, OPT_NONE }},  //Get the value of the target's current property stat name STR, load into VAR.
	{ "dispel_target_property",OP_DISPELTARGETPROPERTY, 2, {OPT_STR, OPT_INT, OPT_NONE }},  //Dispel abilities from the target that buff a certain property.
	{ "randomize",         OP_RANDOMIZE,      2, {OPT_INT,  OPT_VAR,  OPT_NONE }},  //Generate a random number from 1 to INT, store the result in VAR
	{ "find_cdef",         OP_FINDCDEF,       2, {OPT_INT,  OPT_VAR,  OPT_NONE }},  //Find a creature with the CDef in INT, save CreatureID to VAR
	{ "play_sound",        OP_PLAYSOUND,      1, {OPT_STR,  OPT_NONE, OPT_NONE }}, //Send a sound effect trigger to the client of STR "package|filename.ogg"
	{ "get_buff_tier",     OP_GETBUFFTIER,    2, {OPT_INT,  OPT_VAR,  OPT_NONE }},  //Look up a buff containing a certain Ability Group ID, and store the resulting tier 
	{ "get_target_buff_tier",OP_GETTARGETBUFFTIER,    2, {OPT_INT, OPT_VAR, OPT_NONE }},  //Look up a buff containing a certain Ability Group ID, and store the resulting tier 
	{ "target_in_range",  OP_TARGETINRANGE,   2, {OPT_INT,  OPT_VAR,  OPT_NONE }},  //Check if the target is within range (in game units).  Return true if a target exists, and is in range.
	{ "get_target_range", OP_GETTARGETRANGE,  1, {OPT_VAR,  OPT_NONE, OPT_NONE }}, //Get the distance from the current target (in game units, 10 = 1 meter) and store in Var.  Return 0 if no target.
	{ "set_gtae",         OP_SETGTAE,         0, {OPT_NONE, OPT_NONE, OPT_NONE }},  //Set the ground target for GTAE abilities to the current target, or the caster if no target.
	{ "get_speed",        OP_GETSPEED,        2, {OPT_VAR,  OPT_VAR,  OPT_NONE }},   //Get the target's current movement speed (first VAR is CreatureInstanceID) and store the result in the second VAR.  Nonzero means moving.
	{ "cid_is_busy",      OP_CIDISBUSY,       2, {OPT_VAR,  OPT_VAR,  OPT_NONE }},   //First VAR is Creature ID to check.  Second VAR is result.  If casting, channeling, or otherwise using a primary skill, set VAR to 1, otherwise 0
};

const int maxExtOpCode = sizeof(extCoreOpCode) / sizeof(extCoreOpCode[0]);

void AIScriptDef::GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize)
{
	*arrayStart = extCoreOpCode;
	arraySize = maxExtOpCode;
}

/*
OpCodeInfo* AIScriptDef :: GetInstructionData(int opcode)
{
	for(int i = 0; i < ScriptCore::maxCoreOpCode; i++)
		if(ScriptCore::defCoreOpCode[i].opCode == opcode)
			return &ScriptCore::defCoreOpCode[i];

	for(int i = 0; i < maxExtOpCode; i++)
		if(extCoreOpCode[i].opCode == opcode)
			return &extCoreOpCode[i];

	ScriptCore::PrintMessage("Unidentified opcode: %d", opcode);
	return &ScriptCore::defCoreOpCode[0];
}

OpCodeInfo* AIScriptDef :: GetInstructionDataByName(const char *name)
{
	for(int i = 0; i < ScriptCore::maxCoreOpCode; i++)
		if(strcmp(ScriptCore::defCoreOpCode[i].name, name) == 0)
			return &ScriptCore::defCoreOpCode[i];

	for(int i = 0; i < maxExtOpCode; i++)
		if(strcmp(extCoreOpCode[i].name, name) == 0)
			return &extCoreOpCode[i];

	return &ScriptCore::defCoreOpCode[0];
}
*/

void AIScriptPlayer :: RunImplementationCommands(int opcode)
{
	ScriptCore::OpData *in = &def->instr[curInst];
	switch(opcode)
	{
	case OP_USE:
		if(attachedCreature->ab[0].bPending == false)
		{
			//DEBUG OUTPUT
			if(g_Config.DebugLogAIScriptUse == true)
			{
				const Ability2::AbilityEntry2* abptr = g_AbilityManager.GetAbilityPtrByID(in->param1);
				g_Log.AddMessageFormat("Using: %s", abptr->GetRowAsCString(Ability2::ABROW::NAME));
			}
			//END DEBUG OUTPUT

			int r = attachedCreature->CallAbilityEvent(in->param1, EventType::onRequest);
			if(r != 0)
			{
				//Notify the creature we failed, may need a distance check.
				//The script should wait and retry soon.
				attachedCreature->AICheckAbilityFailure(r);
				nextFire = g_ServerTime + USE_FAIL_DELAY;


				if(g_Config.DebugLogAIScriptUse == true)
				{
					const Ability2::AbilityEntry2* abptr = g_AbilityManager.GetAbilityPtrByID(in->param1);
					g_Log.AddMessageFormat("Using: %s   Failed: %d", abptr->GetRowAsCString(Ability2::ABROW::NAME), g_AbilityManager.GetAbilityErrorCode(r));
				}

				if(attachedCreature->AIAbilityFailureAllowRetry(r) == true)
				{
					advance = 0;  //Don't advance the instruction so that we can retry this command.
				}
			}
		}
		else
		{
			advance = 0;
			nextFire = g_ServerTime + USE_FAIL_DELAY;
		}
		break;
	case OP_GETWILL:
		SetVar(def->instr[curInst].param1, attachedCreature->css.will);
		break;
	case OP_GETWILLCHARGE:
		SetVar(def->instr[curInst].param1, attachedCreature->css.will_charges);
		break;
	case OP_GETMIGHT:
		SetVar(def->instr[curInst].param1, attachedCreature->css.might);
		break;
	case OP_GETMIGHTCHARGE:
		SetVar(def->instr[curInst].param1, attachedCreature->css.might_charges);
		break;
	case OP_HASTARGET:
		SetVar(def->instr[curInst].param1, (attachedCreature->CurrentTarget.targ != NULL) ? 1 : 0);
		break;
	case OP_GETLEVEL:
		SetVar(def->instr[curInst].param1, attachedCreature->css.level);
		break;
	case OP_DEBUGPRINT:
		g_Logs.script->debug("%v", def->stringList[def->instr[curInst].param1].c_str());
		break;
	case OP_GETCOOLDOWN:
		{
			const char *cooldownName = GetStringTableEntry(def->instr[curInst].param1);
			int cooldownID = g_AbilityManager.ResolveCooldownCategoryID(cooldownName);
			int result = (attachedCreature->HasCooldown(cooldownID) == true) ? 1 : 0;
			SetVar(in->param2, result);
		}
		break;
	case OP_ISBUSY:
		{
			int result = (attachedCreature->AICheckIfAbilityBusy() == true) ? 1 : 0;
			SetVar(in->param1, result);
		}
		break;
	case OP_COUNTENEMYNEAR:
		{
			float x = (float)attachedCreature->CurrentX;
			float z = (float)attachedCreature->CurrentZ;
			SetVar(in->param2, attachedCreature->AICountEnemyNear(in->param1, x, z));
		}
		break;
	case OP_COUNTENEMYAT:
		{
			float x = (float)attachedCreature->CurrentX;
			float z = (float)attachedCreature->CurrentZ;
			if(attachedCreature->CurrentTarget.targ != NULL)
			{
				x = (float)attachedCreature->CurrentTarget.targ->CurrentX;
				z = (float)attachedCreature->CurrentTarget.targ->CurrentZ;
			}
			SetVar(in->param2, attachedCreature->AICountEnemyNear(in->param1, x, z));
		}
		break;
	case OP_HEALTHPERCENT:
		SetVar(in->param1, static_cast<int>(attachedCreature->GetHealthRatio() * 100.0F));
		break;
	case OP_TARGETHEALTHPERCENT:
		{
			int health = 0;
			if(attachedCreature->CurrentTarget.targ != NULL)
				health = static_cast<int>(attachedCreature->CurrentTarget.targ->GetHealthRatio() * 100.0F);
			SetVar(in->param1, health);
		}
		break;
	case OP_SETELAPSEDTIME:
		SetVar(in->param1, static_cast<int>(g_PlatformTime.getElapsedMilliseconds()));
		break;
	case OP_TIMEOFFSET:
		{
			unsigned long offset = g_PlatformTime.getElapsedMilliseconds() - static_cast<unsigned long>(GetVarValue(in->param1));
			SetVar(in->param2, static_cast<int>(offset));
		}
		break;
	case OP_VISUALEFFECT:
		attachedCreature->SendEffect(GetStringTableEntry(in->param1), 0);
		break;
	case OP_VISUALEFFECTT:
		{
		int targID = 0;
		if(attachedCreature->CurrentTarget.targ != NULL)
			targID = attachedCreature->CurrentTarget.targ->CreatureID;
		attachedCreature->SendEffect(GetStringTableEntry(in->param1), targID);
		}
		break;
	case OP_SAY:
		attachedCreature->SendSay(GetStringTableEntry(in->param1));
		break;
	case OP_INSTANCECALL:
		attachedCreature->actInst->ScriptCall(GetStringTableEntry(in->param1));
		break;
	case OP_GETIDLEMOB:
		{
			int creatureDefID = in->param1;
			int creatureID = attachedCreature->AIGetIdleMob(creatureDefID);
			SetVar(in->param2, creatureID);
		}
		break;
	case OP_GETTARGET:
		{
			int creatureID = 0;
			if(attachedCreature->CurrentTarget.targ != NULL)
				creatureID = attachedCreature->CurrentTarget.targ->CreatureID;
			SetVar(in->param1, creatureID);
		}
		break;
	case OP_GETSELF:
		SetVar(in->param1, attachedCreature->CreatureID);
		break;
	case OP_SETOTHERTARGET:
		{
			int creatureID = GetVarValue(in->param1);
			int creatureIDTarg = GetVarValue(in->param2);
			attachedCreature->AIOtherSetTarget(creatureID, creatureIDTarg);
		}
		break;
	case OP_AISCRIPTCALL:
		{
			int creatureID = GetVarValue(in->param1);
			attachedCreature->AIOtherCallLabel(creatureID, GetStringTableEntry(in->param2));
		}
		break;
	case OP_ISTARGETENEMY:
		{
			int result = (attachedCreature->AIIsTargetEnemy() == true) ? 1 : 0;
			SetVar(in->param1, result);
		}
		break;
	case OP_ISTARGETFRIENDLY:
		{
			int result = (attachedCreature->AIIsTargetFriend() == true) ? 1 : 0;
			SetVar(in->param1, result);
		}
		break;
	case OP_SETSPEED:
		attachedCreature->Speed = in->param1;
		break;
	case OP_GETTARGETCDEF:
		{
			int targCDef = 0;
			if(attachedCreature->CurrentTarget.targ != NULL)
				targCDef = attachedCreature->CurrentTarget.targ->CreatureDefID;
			SetVar(in->param1, targCDef);
		}
		break;
	case OP_GETPROPERTY:
		{
			const char *propName = GetStringTableEntry(in->param1);
			SetVar(in->param2, static_cast<int>(attachedCreature->AIGetProperty(propName, false)));
		}
		break;
	case OP_GETTARGETPROPERTY:
		{
			const char *propName = GetStringTableEntry(in->param1);
			SetVar(in->param2, static_cast<int>(attachedCreature->AIGetProperty(propName, true)));
		}
		break;
	case OP_DISPELTARGETPROPERTY:
		{
			const char *propName = GetStringTableEntry(in->param1);
			int sign = in->param2;
			attachedCreature->AIDispelTargetProperty(propName, sign);
		}
		break;
	case OP_RANDOMIZE:
		SetVar(in->param2, randint(1, in->param1));
		break;
	case OP_FINDCDEF:
		{
			int creatureID = 0;
			CreatureInstance *targ = attachedCreature->actInst->GetNPCInstanceByCDefID(in->param1);
			if(targ != NULL)
				creatureID = targ->CreatureID;
			SetVar(in->param2, creatureID);
		}
		break;
	case OP_PLAYSOUND:
		{
			STRINGLIST sub;
			Util::Split(GetStringTableEntry(in->param1), "|", sub);
			while(sub.size() < 2)
			{
				sub.push_back("");
			}
			attachedCreature->SendPlaySound(sub[0].c_str(), sub[1].c_str());
		}
		break;
	case OP_GETBUFFTIER:
		SetVar(in->param2, attachedCreature->AIGetBuffTier(in->param1, false));
		break;
	case OP_GETTARGETBUFFTIER:
		SetVar(in->param2, attachedCreature->AIGetBuffTier(in->param1, true));
		break;
	case OP_TARGETINRANGE:
		SetVar(in->param2, (attachedCreature->InRange_Target((float)in->param1) == true) ? 1 : 0);
		break;
	case OP_GETTARGETRANGE:
		SetVar(in->param1, attachedCreature->AIGetTargetRange());
		break;
	case OP_SETGTAE:
		attachedCreature->AISetGTAE();
		break;
	case OP_GETSPEED:
		{
			int result = 0;
			int creatureID = GetVarValue(in->param1);
			CreatureInstance *targ = ResolveCreatureInstance(creatureID);
			if(targ != NULL)
			{
				result = targ->Speed;
			}
			SetVar(in->param2, result);
		}
		break;
	case OP_CIDISBUSY:
		{
			int result = 0;
			CreatureInstance *targ = ResolveCreatureInstance(GetVarValue(in->param1));
			if(targ != NULL)
			{
				result = (targ->AICheckIfAbilityBusy() == true) ? 1 : 0;
			}
			SetVar(in->param2, result);
		}
		break;
	default:
		g_Logs.script->error("Unidentified op type: %v", in->opCode);
		break;
	}
}

CreatureInstance* AIScriptPlayer :: ResolveCreatureInstance(int CreatureInstanceID)
{
	if(attachedCreature == NULL)
		return NULL;
	if(attachedCreature->actInst == NULL)
		return NULL;
	return attachedCreature->actInst->GetInstanceByCID(CreatureInstanceID);
}

void AIScriptPlayer :: DebugGenerateReport(ReportBuffer &report)
{
	if(def != NULL)
	{
		report.AddLine("Name:%s", def->scriptName.c_str());
	}
	report.AddLine("active:%d", static_cast<int>(mActive));
	report.AddLine("executing:%d", static_cast<int>(mExecuting));
	report.AddLine("curInst:%d", curInst);
	report.AddLine("nextFire:%lu", nextFire);
	report.AddLine(NULL);
	report.AddLine("vars:%d", vars.size());
	for(size_t i = 0; i < vars.size(); i++)
		report.AddLine("vars[%d]=%d", i, vars[i]);

	report.AddLine(NULL);
	report.AddLine("varStack:%d", varStack.size());
	for(size_t i = 0; i < varStack.size(); i++)
		report.AddLine("varStack[%d]=%d", i, varStack[i]);

	report.AddLine(NULL);
	report.AddLine("callStack:%d", callStack.size());
	for(size_t i = 0; i < callStack.size(); i++)
		report.AddLine("callStack[%d]=%d", i, callStack[i]);
}

AIScriptManager :: AIScriptManager()
{
}

AIScriptManager :: ~AIScriptManager()
{
	aiDef.clear();
	aiAct.clear();
}

int AIScriptManager :: LoadScripts(void)
{
	char FileName[256];
	Platform::GenerateFilePath(FileName, "AIScript", "script_list.txt");
	FileReader lfr;
	if(lfr.OpenText(FileName) != Err_OK)
	{
		g_Log.AddMessageFormat("Error opening master script list [%s]", FileName);
		return -1;
	}

	AIScriptDef newItem;
	lfr.CommentStyle = Comment_Semi;
	string LoadName;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			aiDef.push_back(newItem);
			Platform::GenerateFilePath(LoadName, "AIScript", lfr.DataBuffer);
			aiDef.back().CompileFromSource(LoadName.c_str());
		}
	}
	lfr.CloseCurrent();
	g_Log.AddMessageFormat("Loaded %d AI Scripts", aiDef.size());

	return 0;
}

AIScriptDef * AIScriptManager :: GetScriptByName(const char *name)
{
	if(aiDef.size() == 0)
		return NULL;
	list<AIScriptDef>::iterator it;
	for(it = aiDef.begin(); it != aiDef.end(); ++it)
	{
		if(it->scriptName.compare(name) == 0)
			return &*it;
	}
	return NULL;
}

AIScriptPlayer * AIScriptManager :: AddActiveScript(const char *name)
{
	AIScriptDef *def = GetScriptByName(name);
	if(def == NULL)
		return NULL;
	aiAct.push_back(AIScriptPlayer());
	aiAct.back().Initialize(def);
	return &aiAct.back();
}

void AIScriptManager :: RemoveActiveScript(AIScriptPlayer *registeredPtr)
{
	list<AIScriptPlayer>::iterator it;
	for(it = aiAct.begin(); it != aiAct.end(); ++it)
	{
		if(&*it == registeredPtr)
		{
			aiAct.erase(it);
			break;
		}
	}
}

