#include "Guilds.h"
#include "FileReader.h"
#include "Util.h"

#include "Character.h"
#include "util/Log.h"

GuildManager g_GuildManager;

void GuildDefinition::Clear(void) {
	defName = "";
	guildDefinitionID = 0;
	guildType = 0;
}

void GuildDefinition::WriteToJSON(Json::Value &value) {
	value["id"] = guildDefinitionID;
	value["type"] = guildType;
	value["name"] = defName;
	value["motto"] = motto;
	Json::Value g;
	g["zone"] = guildHallZone;
	g["x"] = guildHallX;
	g["y"] = guildHallY;
	g["z"] = guildHallZ;
	value["hall"] = g;
}

void GuildDefinition::RunLoadDefaults(void) {
	//Prepare the minimap quest marker information by extracting from the data string.
	STRINGLIST locData;
	Util::Split(sGuildHall, ",", locData);
	if (locData.size() < 4) {
		g_Logs.data->warn("GuildDef:%v has incomplete sGuildHall string",
				guildDefinitionID);
	} else {
		guildHallX = static_cast<int>(strtod(locData[0].c_str(), NULL));
		guildHallY = static_cast<int>(strtod(locData[1].c_str(), NULL));
		guildHallZ = static_cast<int>(strtod(locData[2].c_str(), NULL));
		guildHallZone = static_cast<int>(strtol(locData[3].c_str(), NULL, 10));
	}

	for (unsigned int i = 0; i < ranks.size(); i++) {
		ranks[i].RunLoadDefaults();
	}
}

void GuildRankObject::Clear(void) {
	valour = 0;
	rank = 0;
}

void GuildRankObject::RunLoadDefaults() {
	STRINGLIST rankData;
	Util::Split(_data, ",", rankData);
	if (rankData.size() < 2) {
		g_Logs.data->warn("Rank:%v has incomplete rank string", rank);
	} else {
		valour = static_cast<int>(strtod(rankData[0].c_str(), NULL));
		title = rankData[1].c_str();
	}
}

GuildManager::GuildManager() {
}

GuildManager::~GuildManager() {
	defList.clear();
}

void GuildManager::LoadFile(std::string filename) {
	FileReader lfr;
	if (lfr.OpenText(filename.c_str()) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", filename);
		return;
	}
	GuildDefinition newItem;
	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (newItem.guildDefinitionID != 0) {
					newItem.RunLoadDefaults();
					defList.push_back(newItem);
					newItem.Clear();
				}
			} else if (strcmp(lfr.SecBuffer, "NAME") == 0)
				newItem.defName = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "ID") == 0)
				newItem.guildDefinitionID = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "GUILDTYPE") == 0)
				newItem.guildType = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "GUILDHALL") == 0)
				newItem.sGuildHall = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "MOTTO") == 0)
				newItem.motto = lfr.BlockToStringC(1, 0);
			else {
				// Assume to be rank
				unsigned int rank = lfr.BlockToIntC(0);
				if (rank != newItem.ranks.size() + 1) {
					if (newItem.ranks.size() > 0) {
						g_Logs.data->warn(
								"Unknown identifier [%v] in file [%v], expected <rank>=<valour>",
								lfr.SecBuffer, filename);
					} else {
						g_Logs.data->warn(
								"Unknown identifier [%v] in file [%v]",
								lfr.SecBuffer, filename);
					}
				} else {
					GuildRankObject newObject;
					newObject.rank = newItem.ranks.size() + 1;
					newObject._data = lfr.BlockToStringC(1, 0);
					newItem.ranks.push_back(newObject);
				}

			}
		}
	}
	if (newItem.guildDefinitionID != 0) {
		newItem.RunLoadDefaults();
		defList.push_back(newItem);
	}
	lfr.CloseCurrent();
}

int GuildManager::GetStandardCount(void) {
	return (int) defList.size();
}

GuildDefinition * GuildManager::FindGuildDefinition(std::string name) {
	for (int i = 0; i < defList.size(); i++) {
		g_Logs.data->debug("Looking for guild .. %v against %v", name.c_str(), defList[i].defName.c_str());
		if (defList[i].defName.compare(name) == 0)
			return &defList[i];
	}
	return NULL;
}

GuildDefinition * GuildManager::GetGuildDefinition(int guildDefID) {
	size_t i = 0;

	for (i = 0; i < defList.size(); i++) {
		if (defList[i].guildDefinitionID == guildDefID)
			return &defList[i];
	}
	return NULL;
}

GuildDefinition * GuildManager::GetGuildDefinitionForGuildHallZoneID(
		int zoneID) {
	size_t i = 0;
	for (i = 0; i < defList.size(); i++) {
		if (defList[i].guildHallZone == zoneID)
			return &defList[i];
	}
	return NULL;
}

GuildRankObject * GuildManager::GetRank(int CDefID, int guildDefId) {
	CharacterData *cdata = g_CharacterManager.GetPointerByID(CDefID);
	int val = cdata->GetValour(guildDefId);
	GuildDefinition *def = GetGuildDefinition(guildDefId);
	for (int i = def->ranks.size() - 1; i >= 0; i--) {
		if (val >= def->ranks[i].valour) {
			return &def->ranks[i];
		}
	}
	return NULL;
}

bool GuildManager::IsMutualGuild(int selfDefID, int otherDefID) {
	CharacterData *cdata = g_CharacterManager.GetPointerByID(selfDefID);
	CharacterData *otherCdata = g_CharacterManager.GetPointerByID(otherDefID);
	for (unsigned int i = 0; i < cdata->guildList.size(); i++)
		for (unsigned int j = 0; j < otherCdata->guildList.size(); j++)
			if (cdata->guildList[i].GuildDefID
					== otherCdata->guildList[j].GuildDefID)
				return true;
	return false;
}
