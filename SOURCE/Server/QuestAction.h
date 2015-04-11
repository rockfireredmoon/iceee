#ifndef QUESTACTION_H
#define QUESTACTION_H

#include "CommonTypes.h"

class SimulatorThread;

namespace QuestCommand
{

struct CommandParam
{
	enum
	{
		NONE = 0,
		INTEGER,
		STATNAME,
		COMPARE,
		STRING,
	};
};

enum Conditions
{
	COMMAND_NONE = 0,
	CONDITION_HEROISM,
	CONDITION_HAS_ITEM,
	ACTION_CHANGE_HEROISM,
	ACTION_REMOVE_ITEM,
	ACTION_SEND_TEXT,
	ACTION_PLAY_SOUND,
	ACTION_JOIN_GUILD
};
enum Comparator
{
	COMP_NONE = 0,
	COMP_EQUAL,
	COMP_NOTEQUAL,
	COMP_GT,
	COMP_GTE,
	COMP_LT,
	COMP_LTE
};

struct SimpleProperty
{
	const char *name;
	int value;
};

struct QuestScriptCommandDef
{
	const char *name;
	int opCode;
	int numParams;
	int paramType[3];
};

struct ExtendedQuestAction
{
	int opCode;
	int param[3];
	std::string paramStr;

	ExtendedQuestAction();
	const QuestScriptCommandDef *GetCommandDef(const std::string &name);
	bool ResolveOperand(const std::string &token, int paramType, int &resolved);
	bool ExpectTokens(int has, int required);
	int GetComparator(const std::string &name);
	int GetStatIDByName(const std::string &name);
	bool InitCommand(const STRINGLIST &tokenList);
};

class QuestActionContainer
{
public:
	std::vector<ExtendedQuestAction> mInstList;
	STRINGLIST mStringList;

	void AddLine(const char *statement);
	void AddCommand(const std::string &command);
	int	ExecuteSingleCommand(SimulatorThread *caller, ExtendedQuestAction &e);
	int ExecuteAllCommands(SimulatorThread *caller);
	bool Compare(int left, int comparator, int right);
	void Clear(void);
	void CopyFrom(const QuestActionContainer &other);
};

}
//namespace QuestCommand


#endif //#ifndef QUESTACTION_H
