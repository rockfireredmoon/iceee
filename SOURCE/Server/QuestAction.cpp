#include "QuestAction.h"
#include "StringList.h"
#include "Util.h"
#include "Stats.h"
#include "Item.h"
#include "Simulator.h"

namespace QuestCommand
{
ExtendedQuestAction :: ExtendedQuestAction()
{
	opCode = 0;
	param[0] = 0;
	param[1] = 0;
	param[2] = 0;
}

const QuestScriptCommandDef* ExtendedQuestAction :: GetCommandDef(const std::string &name)
{
	static const QuestScriptCommandDef commands[] = {
		//Conditions
		{"heroism",     CONDITION_HEROISM, 2, CommandParam::COMPARE, CommandParam::INTEGER, CommandParam::NONE },
		{"has_item",    CONDITION_HAS_ITEM, 2, CommandParam::INTEGER, CommandParam::INTEGER, CommandParam::NONE },
		
		//Actions
		{"change_heroism", ACTION_CHANGE_HEROISM, 1, CommandParam::INTEGER, CommandParam::NONE, CommandParam::NONE },
		{"remove_item",    ACTION_REMOVE_ITEM, 2, CommandParam::INTEGER, CommandParam::INTEGER, CommandParam::NONE },
		{"send_text",      ACTION_SEND_TEXT, 1, CommandParam::STRING, CommandParam::NONE, CommandParam::NONE },
		{"play_sound",     ACTION_PLAY_SOUND, 1, CommandParam::STRING, CommandParam::NONE, CommandParam::NONE },
	};

	static const int count = COUNT_ARRAY_ELEMENTS(commands);
	for(int i = 0; i < count; i++)
		if(name.compare(commands[i].name) == 0)
			return &commands[i];
	return NULL;
}

bool ExtendedQuestAction :: ResolveOperand(const std::string &token, int paramType, int &resolved)
{
	switch(paramType)
	{
	case CommandParam::NONE: resolved = 0; return true;
	case CommandParam::INTEGER: resolved = Util::GetInteger(token); return true;
	case CommandParam::STATNAME:
		resolved = GetStatIDByName(token);
		if(resolved == -1)
			return false;
		return true;
	case CommandParam::COMPARE:
		resolved = GetComparator(token);
		if(resolved == COMP_NONE)
			return false;
		return true;
	case CommandParam::STRING:
		resolved = 0;
		paramStr = token;
		return true;
	default:
		g_Log.AddMessageFormat("Unknown paramType:%d", paramType);
	}
	return false;
}
		
int ExtendedQuestAction :: GetComparator(const std::string &name)
{
	static const SimpleProperty dataArr[] = {
		{"=", COMP_EQUAL},
		{"!=", COMP_NOTEQUAL},
		{">", COMP_GT},
		{">=", COMP_GTE},
		{"<", COMP_LT},
		{"<=", COMP_LTE},
	};
	const int dataCount = COUNT_ARRAY_ELEMENTS(dataArr);
	for(int i = 0; i < dataCount; i++)
		if(name.compare(dataArr[i].name) == 0)
			return dataArr[i].value;

	g_Log.AddMessageFormat("Quest script comparator [%s] not recognized", name.c_str());
	return COMP_NONE;
}

int ExtendedQuestAction :: GetStatIDByName(const std::string &name)
{
	int r = GetStatIndexByName(name.c_str());
	if(r == -1)
		return -1;
	return StatList[r].ID;
}

bool ExtendedQuestAction :: ExpectTokens(int has, int required)
{
	if(has >= required)
		return true;
	return false;
}

bool ExtendedQuestAction :: InitCommand(const STRINGLIST &tokenList)
{
	if(tokenList.size() < 1)
		return false;
	const QuestScriptCommandDef *cmd = GetCommandDef(tokenList[0]);
	if(cmd == 0)
	{
		g_Log.AddMessageFormat("Quest command not recognized [%s]", tokenList[0].c_str());
		return false;
	}
	size_t tokenCount = tokenList.size();
	if(tokenCount != cmd->numParams + 1)  //First token is command itself
	{
		g_Log.AddMessageFormat("Expected [%d] arguments for command [%s]", cmd->numParams, cmd->name);
		return false;
	}

	opCode = cmd->opCode;
	for(int i = 0; i < cmd->numParams; i++)
	{
		bool valid = false;
		int result = 0;
		valid = ResolveOperand(tokenList[1 + i], cmd->paramType[i], result);
		if(valid == false)
			return false;
		param[i] = result;
	}
	return true;

	/*
	int opCode = GetCondition(tokenList[0]);
	size_t tokenCount = tokenList.size();
	int p1 = 0;
	int p2 = 0;
	int p3 = 0;
	switch(opCode)
	{
	case CONDITION_HASSTAT:
		if(ExpectTokens(tokenCount, 4) == false) return false;
		p1 = GetStatIDByName(tokenList[1]);
		p2 = GetComparator(tokenList[2]);
		p3 = Util::GetFloat(tokenList[3]);
		if(p1 == -1 || p2 == COMP_NONE)
			return false;
		break;
	case CONDITION_HASITEM:
		if(ExpectTokens(tokenCount, 3) == false) return false;
		p1 = Util::GetInteger(tokenList[1]);
		p2 = Util::GetInteger(tokenList[2]);
		break;
	}
	commandType = opCode;
	param1 = p1;
	param2 = p2;
	param3 = p3;
	return true;
	*/
}

void QuestActionContainer :: AddCommand(const std::string &command)
{
	STRINGLIST tokens;
	//Util::Split(command, " ", tokens);
	Util::TokenizeByWhitespace(command, tokens);
	if(tokens.size() == 0)
	{
		g_Log.AddMessageFormat("No tokens in command.");
		return;
	}
	ExtendedQuestAction inst;
	if(inst.InitCommand(tokens) == true)
	{
		//Debug disassembly
		//g_Log.AddMessageFormat("%s = [%d]=%d,%d,%d", command.c_str(), inst.opCode, inst.param[0], inst.param[1], inst.param[2]);

		mInstList.push_back(inst);
	}
}

void QuestActionContainer :: AddLine(const char *statement)
{
	STRINGLIST commands;
	Util::Split(statement, ";", commands);
	for(size_t i = 0; i < commands.size(); i++)
		AddCommand(commands[i]);
}

int QuestActionContainer :: ExecuteSingleCommand(SimulatorThread *caller, ExtendedQuestAction &e)
{
	CreatureInstance *cInst = caller->creatureInst;
	switch(e.opCode)
	{
	case COMMAND_NONE: return 0;
	case CONDITION_HEROISM:
		{
			int value = cInst->css.heroism;
			if(Compare(value, e.param[0], e.param[1]) == true)
				return 0;
			caller->SendInfoMessage("You don't meet the heroism requirement.", INFOMSG_ERROR);
			return -1;
		}
		break;
	case CONDITION_HAS_ITEM:
		{
			int itemID = e.param[0];
			int itemCount = e.param[1];
			int count = caller->pld.charPtr->inventory.GetItemCount(INV_CONTAINER, itemID);
			if(count >= itemCount)
				return 0;
			caller->SendInfoMessage("You don't have the required items in your backpack inventory.", INFOMSG_ERROR);
			return -1;
		}
		break;
	case ACTION_CHANGE_HEROISM:
		cInst->css.heroism += e.param[0];
		cInst->OnHeroismChange();
		break;
	case ACTION_REMOVE_ITEM:
		{
			int itemID = e.param[0];
			int itemCount = e.param[1];
			char buffer[2048];
			int len = caller->pld.charPtr->inventory.RemoveItemsAndUpdate(INV_CONTAINER, itemID, itemCount, buffer);
			if(len > 0)
				caller->AttemptSend(buffer, len);
		}
		break;
	case ACTION_SEND_TEXT:
		caller->SendInfoMessage(e.paramStr.c_str(), INFOMSG_INFO);
		break;
	case ACTION_PLAY_SOUND:
		{
			STRINGLIST sub;
			Util::Split(e.paramStr, "|", sub);
			while(sub.size() < 2)
			{
				sub.push_back("");
			}
			caller->SendPlaySound(sub[0].c_str(), sub[1].c_str());
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int QuestActionContainer :: ExecuteAllCommands(SimulatorThread *caller)
{
	if(mInstList.size() == 0)
		return 0;

	for(size_t i = 0; i < mInstList.size(); i++)
	{
		int r = ExecuteSingleCommand(caller, mInstList[i]);
		if(r < 0)
			return -1;
	}
	return 0;
}

bool QuestActionContainer :: Compare(int left, int comparator, int right)
{
	switch(comparator)
	{
	case COMP_NONE: return true;
	case COMP_EQUAL: return left == right;
	case COMP_NOTEQUAL: return left != right;
	case COMP_GT: return left > right;
	case COMP_GTE: return left >= right;
	case COMP_LT: return left < right;
	case COMP_LTE: return left <= right;
	default:
		return false;
	}
	return false;
}

void QuestActionContainer :: Clear(void)
{
	mInstList.clear();
	mStringList.clear();
}

void QuestActionContainer :: CopyFrom(const QuestActionContainer &other)
{
	mInstList.assign(other.mInstList.begin(), other.mInstList.end());
	mStringList = other.mStringList;
}

}//namespace QuestCommand
