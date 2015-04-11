#pragma once
#ifndef GUILDS_H
#define GUILDS_H


#include <vector>
#include <string>

struct GuildDefinition
{
	char defName[20];
	int guildDefinitionID;
	int guildType;
	std::string sGuildHall;

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
};

extern GuildManager g_GuildManager;


#endif /* GUILDS_H */
