#include <string.h>

#include "Interact.h"
#include "StringList.h"
#include "FileReader.h"
#include "util/Log.h"

InteractObjectContainer g_InteractObjectContainer;

InteractObject :: InteractObject()
{
	Clear();
}

InteractObject :: ~InteractObject()
{
}

void InteractObject :: Clear(void)
{
	memset(internalName, 0, sizeof(internalName));
	memset(useMessage, 0, sizeof(useMessage));
	memset(scriptFunction, 0, sizeof(scriptFunction));

	useTime = DEFAULT_USE_TIME;
	CreatureDefID = 0;

	opType = 0;
	questReq = 0;
	questComp = false;
	zoneReq = 0;
	facing = 0;

	WarpX = 0;
	WarpY = 0;
	WarpZ = 0;
	WarpID = 0;

	cost = 0;
}

void InteractObject :: SetName(char *str)
{
	unsigned int len = strlen(str);
	if(len > sizeof(internalName) - 1)
	{
		g_Log.AddMessageFormat("[WARNING] InteractObject::SetName string size is too long [%s]", str);
		len = sizeof(internalName) - 1;
	}
	strncpy(internalName, str, len);
}

void InteractObject :: SetType(char *str)
{
	static const int TypeID[5] = {TYPE_NONE, TYPE_WARP, TYPE_HENGE, TYPE_LOCATIONRETURN, TYPE_SCRIPT};
	static const char *TypeName[5] = {"none", "warp", "henge", "locationreturn", "script"};
	opType = TYPE_NONE;
	for(size_t i = 0; i < 5; i++)
	{
		if(strcmp(str, TypeName[i]) == 0)
		{
			opType = TypeID[i];
			break;
		}
	}

	if(opType == TYPE_NONE)
		g_Log.AddMessageFormat("[WARNING] InteractObject::SetType unknown type [%s]", str);
}

void InteractObject :: SetMessage(char *str)
{
	unsigned int len = strlen(str);
	if(len > sizeof(useMessage) - 1)
	{
		g_Log.AddMessageFormat("[WARNING] InteractObject::SetMessage string size is too long [%s]", str);
		len = sizeof(useMessage) - 1;
	}
	strncpy(useMessage, str, len);
}

void InteractObject :: SetScriptFunction(char *str)
{
	unsigned int len = strlen(str);
	if(len > sizeof(scriptFunction) - 1)
	{
		g_Log.AddMessageFormat("[WARNING] InteractObject::SetScriptFunction string size is too long [%s]", str);
		len = sizeof(scriptFunction) - 1;
	}
	strncpy(scriptFunction, str, len);
}

InteractObjectContainer :: InteractObjectContainer()
{
}

InteractObjectContainer :: ~InteractObjectContainer()
{
	objList.clear();
}

int InteractObjectContainer :: FilterByName(char *className, std::vector<InteractObject*> &resultPtrList)
{
	for(size_t i = 0; i < objList.size(); i++)
		if(strcmp(objList[i].internalName, className) != 0)
			resultPtrList.push_back(&objList[i]);

	return resultPtrList.size();
}

int InteractObjectContainer :: FilterByCDefID(int CDefID, std::vector<InteractObject*> &resultPtrList)
{
	for(size_t i = 0; i < objList.size(); i++)
		if(objList[i].CreatureDefID == CDefID)
			resultPtrList.push_back(&objList[i]);

	return resultPtrList.size();
}

InteractObject * InteractObjectContainer :: GetObjectByID(int CDefID, int zoneID)
{
	for(size_t i = 0; i < objList.size(); i++)
	{
		if(objList[i].zoneReq == 0 || objList[i].zoneReq == zoneID)
			if(objList[i].CreatureDefID == CDefID)
				return &objList[i];
	}
	return NULL;
}

InteractObject * InteractObjectContainer :: GetHengeByDefID(int CDefID)
{
	for(size_t i = 0; i < objList.size(); i++)
		if(objList[i].opType == InteractObject::TYPE_HENGE)
			if(objList[i].CreatureDefID == CDefID)
				return &objList[i];
	return NULL;
}

InteractObject * InteractObjectContainer :: GetHengeByTargetName(const char* name)
{
	for(size_t i = 0; i < objList.size(); i++)
		if(objList[i].opType == InteractObject::TYPE_HENGE)
			if(strcmp(objList[i].useMessage, name) == 0)
				return &objList[i];
	return NULL;
}

void InteractObjectContainer :: LoadFromFile(char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.data->error("Could not open file [%v]", filename);
		return;
	}

	InteractObject newItem;

	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				AddItem(newItem);
				newItem.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "Name") == 0)
				newItem.SetName(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "Type") == 0)
				newItem.SetType(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "Message") == 0)
				newItem.SetMessage(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "UseTime") == 0)
				newItem.useTime = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "ObjectID") == 0)
				newItem.CreatureDefID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Facing") == 0)
				newItem.facing = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "WarpTo") == 0)
			{
				newItem.WarpX = lfr.BlockToIntC(1);
				newItem.WarpY = lfr.BlockToIntC(2);
				newItem.WarpZ = lfr.BlockToIntC(3);
				newItem.WarpID = lfr.BlockToIntC(4);
			}
			else if(strcmp(lfr.SecBuffer, "Quest") == 0)
			{
				newItem.questReq = lfr.BlockToIntC(1);
				if(r >= 3)
					newItem.questComp = lfr.BlockToBoolC(2);
			}
			else if(strcmp(lfr.SecBuffer, "Cost") == 0)
				newItem.cost = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Zone") == 0)
				newItem.zoneReq = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "ScriptFunction") == 0)
				newItem.SetScriptFunction(lfr.BlockToStringC(1, 0));
			else
				g_Log.AddMessageFormat("[WARNING] Unknown identifier [%s] in file [%s] on line [%d]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	AddItem(newItem);
	lfr.CloseCurrent();
}

void InteractObjectContainer :: AddItem(InteractObject &newObject)
{
	if(newObject.opType != InteractObject::TYPE_NONE)
		objList.push_back(newObject);
}
