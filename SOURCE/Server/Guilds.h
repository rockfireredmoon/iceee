#pragma once
#ifndef GUILDS_H
#define GUILDS_H


#include <vector>
#include <string>


struct GuildRankObject
{
	int rank;
	std::string title;
	int valour;
	std::string _data;

	GuildRankObject() { Clear(); };
	~GuildRankObject() { }
	void Clear(void);
	void RunLoadDefaults(void);
};

struct GuildDefinition
{
	char defName[20];
	char motto[128];
	int guildDefinitionID;
	int guildType;
	std::string sGuildHall;
	std::vector<GuildRankObject> ranks;

	// Used internally and extracted from sGuildHall
	int guildHallX;
	int guildHallY;
	int guildHallZ;
	int guildHallZone;

	GuildDefinition() { Clear(); }
	~GuildDefinition() { }
	void Clear(void);
	void RunLoadDefaults(void);
};

class GuildManager
{
public:
	GuildManager();
	~GuildManager();

	std::vector<GuildDefinition> defList;

	void LoadFile(const char *filename);
	int GetStandardCount(void);
	GuildDefinition *GetGuildDefinitionForGuildHallZoneID(int zoneID);
	GuildDefinition *GetGuildDefinition(int GuildDefID);
	bool IsMutualGuild(int selfDefID, int otherDefID);
	GuildRankObject *GetRank(int CDefId, int guildDefId);
};

extern GuildManager g_GuildManager;


#endif /* GUILDS_H */
