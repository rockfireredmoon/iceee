#include "Guilds.h"
#include "FileReader.h"
#include "Util.h"
#include "StringList.h"
#include "Character.h"

GuildManager g_GuildManager;

void GuildDefinition :: Clear(void)
{
	memset(defName, 0, sizeof(defName));
	guildDefinitionID = 0;
	guildType = 0;
}


void GuildDefinition:: RunLoadDefaults(void)
{
	//Prepare the minimap quest marker information by extracting from the data string.
	STRINGLIST locData;
	Util::Split(sGuildHall, ",", locData);
	if(locData.size() < 4)
	{
		g_Log.AddMessageFormat("[WARNING] GuildDef:%d has incomplete sGuildHall string", guildDefinitionID);
	}
	else
	{
		guildHallX = static_cast<int>(strtod(locData[0].c_str(), NULL));
		guildHallY = static_cast<int>(strtod(locData[1].c_str(), NULL));
		guildHallZ = static_cast<int>(strtod(locData[2].c_str(), NULL));
		guildHallZone = static_cast<int>(strtol(locData[3].c_str(), NULL, 10));
	}
}

GuildManager :: GuildManager()
{
}

GuildManager :: ~GuildManager()
{
	defList.clear();
}

void GuildManager :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open file [%s]", filename);
		return;
	}
	GuildDefinition newItem;
	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	while(lfr.FileOpen() == true)
	{
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if(r > 0)
		{
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.guildDefinitionID != 0)
				{
					newItem.RunLoadDefaults();
					defList.push_back(newItem);
					newItem.Clear();
				}
			}
			else if(strcmp(lfr.SecBuffer, "NAME") == 0)
				Util::SafeCopy(newItem.defName, lfr.BlockToStringC(1, 0), sizeof(newItem.defName));
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
				newItem.guildDefinitionID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "GUILDTYPE") == 0)
				newItem.guildType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "GUILDHALL") == 0)
				newItem.sGuildHall = lfr.BlockToStringC(1, 0);
			else
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s]", lfr.SecBuffer, filename);
		}
	}
	if(newItem.guildDefinitionID != 0)
	{
		newItem.RunLoadDefaults();
		defList.push_back(newItem);
	}
	lfr.CloseCurrent();
}


int GuildManager :: GetStandardCount(void)
{
	return (int)defList.size();
}

GuildDefinition * GuildManager :: GetGuildDefinition(int guildDefID)
{
	size_t i = 0;
	for(i = 0; i < defList.size(); i++)
	{
		if(defList[i].guildDefinitionID == guildDefID)
			return &defList[i];
	}
	return NULL;
}

GuildDefinition * GuildManager :: GetGuildDefinitionForGuildHallZoneID(int zoneID)
{
	size_t i = 0;
	for(i = 0; i < defList.size(); i++)
	{
		if(defList[i].guildHallZone == zoneID)
			return &defList[i];
	}
	return NULL;
}


bool GuildManager :: IsMutualGuild(int selfDefID, int otherDefID)
{
	CharacterData *cdata = g_CharacterManager.GetPointerByID(selfDefID);
	CharacterData *otherCdata = g_CharacterManager.GetPointerByID(otherDefID);
	for(uint i = 0 ; i < cdata->guildList.size(); i++)
		for(uint j = 0 ; j < otherCdata->guildList.size(); j++)
			if(cdata->guildList[i].GuildDefID == otherCdata->guildList[j].GuildDefID)
				return true;
	return false;
}