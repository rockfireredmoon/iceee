#include "QuestScript.h"
#include "StringList.h"
#include "Simulator.h"
#include "Util.h"
#include "Instance.h"
#include "CommonTypes.h"

QuestScript::QuestScriptDef g_QuestScript;

namespace QuestScript
{

OpCodeInfo extCoreOpCode[] = {
	// Implemenation-Specific commands.
	{ "info",          OP_INFO,         1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
	{ "uinfo",         OP_UINFO,        1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
	{ "effect",        OP_EFFECT,       1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
	{ "wait_finish",   OP_WAITFINISH,   0, {OPT_NONE,  OPT_NONE,  OPT_NONE }},
	{ "npcunusable",   OP_NPCUNUSABLE,  1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "npcremove",     OP_NPCREMOVE,    0, {OPT_NONE,  OPT_NONE,  OPT_NONE }},
	{ "require_cdef",  OP_REQUIRECDEF,  1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "spawn",         OP_SPAWN,        1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "spawn_at",      OP_SPAWNAT,      2, {OPT_INT,   OPT_INT,   OPT_NONE }},
	{ "warp_zone",     OP_WARPZONE,     1, {OPT_INT,   OPT_NONE,  OPT_NONE }},
	{ "jmp_cdef",      OP_JMPCDEF,      2, {OPT_INT,   OPT_LABEL, OPT_NONE }},
	{ "setvar",        OP_SETVAR,       2, {OPT_INT,   OPT_INT,   OPT_NONE }},
	{ "emote",         OP_EMOTE,        1, {OPT_STR,   OPT_NONE,  OPT_NONE }},
};
const int maxExtOpCode = COUNT_ARRAY_ELEMENTS(extCoreOpCode);

void QuestScriptDef::GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize)
{
	*arrayStart = QuestScript::extCoreOpCode;
	arraySize = QuestScript::maxExtOpCode;
}



QuestScriptPlayer::QuestScriptPlayer()
{
	Clear();
}

void QuestScriptPlayer::Clear(void)
{
	RunFlags = 0;
	sourceID = 0;
	targetID = 0;
	QuestID = 0;
	QuestAct = 0;
	targetCDef = 0;
	actInst = NULL;
	simCall = NULL;
	memset(RunTimeVar, 0, sizeof(RunTimeVar));

	activateX = 0;
	activateY = 0;
	activateZ = 0;
}

void QuestScriptPlayer::RunImplementationCommands(int opcode)
{
	ScriptCore::OpData *instr = &def->instr[curInst];
	switch(instr->opCode)
	{
	case OP_INFO:
		{
			char Buffer[1024];
			int size = PrepExt_SendInfoMessage(Buffer, def->stringList[instr->param1].c_str(), INFOMSG_INFO);

			CreatureInstance * cInst = actInst->GetInstanceByCID(sourceID);
			if(cInst == NULL || cInst->PartyID == 0)
				// Just sent to player same as OP_INFO
				simCall->AttemptSend(Buffer, size);
			else
			{
				ActiveParty * party = g_PartyManager.GetPartyByID(cInst->PartyID);
				if(party == NULL)
					simCall->AttemptSend(Buffer, size);
				else
					for(uint i = 0 ; i < party->mMemberList.size(); i++)
						party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(Buffer, size, party->mMemberList[i].mCreaturePtr->simulatorPtr);

			}
		}
		break;
	case OP_UINFO:
		{
		char Buffer[1024];
		int size = PrepExt_SendInfoMessage(Buffer, def->stringList[instr->param1].c_str(), INFOMSG_INFO);
		simCall->AttemptSend(Buffer, size);
		}
		break;
	case OP_EFFECT:
		{
		char Buffer[1024];
		int size = PrepExt_SendEffect(Buffer, targetID, def->stringList[instr->param1].c_str(), 0);
		actInst->LSendToAllSimulator(Buffer, size, -1);
		}
		break;
	case OP_WAITFINISH:
		if(!(RunFlags & FLAG_FINISHED))
			advance = 0;
		break;
	case OP_NPCUNUSABLE:
		CreatureInstance *targ;
		targ = actInst->GetNPCInstanceByCID(targetID);
		if(targ != NULL)
		{
			targ->SetServerFlag(ServerFlags::IsUnusable, true);
			//Trigger for deletion
			if(def->instr[curInst].param1 != 0)
			{
				//targ->_AddStatusList(StatusEffects::DEAD, -1);
				targ->SetServerFlag(ServerFlags::TriggerDelete, true);
				targ->deathTime = g_ServerTime + def->instr[curInst].param1;
			}
		}
		break;
	case OP_NPCREMOVE:
		actInst->RemoveNPCInstance(targetID);
		break;
	case OP_REQUIRECDEF:
		if(targetCDef != instr->param1)
			active = false;
		break;
	case OP_SPAWN:
		actInst->spawnsys.TriggerSpawn(instr->param1, 0, 0);
		break;
	case OP_SPAWNAT:
		actInst->SpawnAtProp(instr->param1, instr->param2, RunTimeVar[0], RunTimeVar[1]);
		break;
	case OP_WARPZONE:
		simCall->MainCallSetZone(instr->param1, 0, true);
		break;
	case OP_JMPCDEF:
		if(targetCDef == instr->param1)
		{
			curInst = instr->param2;
			advance = 0;
		}
		break;
	case OP_SETVAR:
		int index;
		index = Util::ClipInt(instr->param1, 0, MAX_VAR - 1);
		RunTimeVar[index] = instr->param2;
		break;
	case OP_EMOTE:
		{
		char Buffer[1024];
		int size = PrepExt_GenericChatMessage(Buffer, sourceID, "", "emote", def->stringList[instr->param1].c_str());
		actInst->LSendToAllSimulator(Buffer, size, -1);
		}
		break;
	default:
		g_Log.AddMessageFormat("Unidentified op type: %d", instr->opCode);
		break;
	}
}

void QuestScriptPlayer::TriggerFinished(void)
{
	RunFlags |= FLAG_FINISHED;
}

void QuestScriptPlayer::TriggerAbort(void)
{
	if(!(RunFlags & FLAG_FINISHED))
		active = false;
}


void LoadQuestScripts(const char *filename)
{
	g_QuestScript.CompileFromSource(filename);
}

void ClearQuestScripts(void)
{
	g_QuestScript.ClearBase();
}


//namespace QuestScript
}
