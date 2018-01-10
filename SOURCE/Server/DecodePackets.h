#include "Util.h"

#include "Config.h"
#include "Globals.h"

void decode_handleQueryResultMsg(const char *buffer);
void decode_handleCreatureUpdateMsg(const char *buffer);
void decode_handleEnvironmentUpdateMsg(const char *buffer);

void DecodePacketBuffer(const char *buffer, int buflen)
{
	int curPos = 0;
	unsigned char type;
	unsigned short size;
	while(curPos < buflen)
	{
		if(curPos + 3 > buflen)
		{
			g_Logs.server->error("Not enough data (at: %v, has: %v", curPos, buflen);
			break;
		}
		type = GetByte(&buffer[curPos], curPos);
		size = GetShort(&buffer[curPos], curPos);
		if(buflen - curPos < size)
		{
			g_Logs.server->error("Not enough data for payload");
			break;
		}
		switch(type)
		{
		case 1: decode_handleQueryResultMsg(&buffer[curPos]); break;
		case 5: decode_handleCreatureUpdateMsg(&buffer[curPos]); break;
		case 42: decode_handleEnvironmentUpdateMsg(&buffer[curPos]); break;
		default:
			g_Logs.server->warn("Unhandled message type: %v (%v bytes)", type, size);
		}
		curPos += size;
	}
}

void decode_handleCreatureUpdateMsg(const char *buffer)
{
	g_Logs.server->debug("[_handleCreatureUpdateMsg]");
	char extbuf[4096];
	int rpos = 0;
	int cid = GetInteger(&buffer[rpos], rpos);
	short mask;
	if(g_ProtocolVersion < 22)
		mask = GetByte(&buffer[rpos], rpos);
	else
		mask = GetShort(&buffer[rpos], rpos);

	g_Logs.server->debug("CreatureID=%v", cid);
	g_Logs.server->debug("mask=%v", mask);

	if(mask & CREATURE_UPDATE_TYPE)
		g_Logs.server->debug("CreatureDefID=%v", GetInteger(&buffer[rpos], rpos));
	if(mask & CREATURE_UPDATE_ZONE)
	{
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		g_Logs.server->debug("CurrentZone=%v", extbuf);
		g_Logs.server->debug("CurrentX=%v", GetInteger(&buffer[rpos], rpos));
		g_Logs.server->debug("CurrentZ=%v", GetInteger(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_ELEVATION)
	{
		g_Logs.server->debug("ypos=%v", GetShort(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_POSITION_INC)
	{
		g_Logs.server->debug("xpos=%v", GetShort(&buffer[rpos], rpos));
		g_Logs.server->debug("zpos=%v", GetShort(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_VELOCITY)
	{
		g_Logs.server->debug("heading=%v", GetByte(&buffer[rpos], rpos));
		g_Logs.server->debug("rotation=%v", GetByte(&buffer[rpos], rpos));
		g_Logs.server->debug("speed=%v", GetByte(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_MOD)
	{
		short modCount = GetShort(&buffer[rpos], rpos);
		for(int a = 0; a < modCount; a++)
		{
			if(g_ProtocolVersion > 15)
			{
				int priority = GetInteger(&buffer[rpos], rpos);
				g_Logs.server->debug("priority=%v", priority);
				g_Logs.server->debug("ID=%v", GetShort(&buffer[rpos], rpos));
				g_Logs.server->debug("AbilityID=%v", GetShort(&buffer[rpos], rpos));
				if(priority == 1)
					g_Logs.server->debug("amount=%v", GetFloat(&buffer[rpos], rpos));
				else
					g_Logs.server->debug("amount=%v", GetShort(&buffer[rpos], rpos));
			}
			else
			{
				g_Logs.server->debug("ID=%v", GetShort(&buffer[rpos], rpos));
				g_Logs.server->debug("AbilityID=%v", GetShort(&buffer[rpos], rpos));
				g_Logs.server->debug("amount=%v", GetShort(&buffer[rpos], rpos));
			}
			g_Logs.server->debug("duration=%v", GetInteger(&buffer[rpos], rpos));
			if(g_ProtocolVersion >= 24)
			{
				GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
				g_Logs.server->debug("description=%v", extbuf);
			}
		}

		short numStatusEffects = GetShort(&buffer[rpos], rpos);
		g_Logs.server->debug("numStatusEffects=%v", numStatusEffects);
		for(int a = 0; a < numStatusEffects; a++)
			g_Logs.server->debug("statID=%v", GetShort(&buffer[rpos], rpos));

	}
	if(mask & CREATURE_UPDATE_STAT)
	{
		short numStats = GetShort(&buffer[rpos], rpos);
		g_Logs.server->debug("numStats=%v", numStats);
		for(int a = 0; a < numStats; a++)
		{
			short statID = GetShort(&buffer[rpos], rpos);
			int r = GetStatIndex(statID);
			if(r == -1)
			{
				g_Logs.server->debug("Stat ID not found: %v", statID);
			}
			else
			{
				if(strcmp(StatList[r].type, "string") == 0)
				{
					GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
					g_Logs.server->debug("%v=%v", StatList[r].name, extbuf);
				}
				else if(strcmp(StatList[r].type, "short") == 0)
					g_Logs.server->debug("%v=%v", StatList[r].name, GetShort(&buffer[rpos], rpos));
				else if(strcmp(StatList[r].type, "int") == 0)
					g_Logs.server->debug("%v=%v", StatList[r].name, GetInteger(&buffer[rpos], rpos));
				else if(strcmp(StatList[r].type, "integer") == 0)
					g_Logs.server->debug("%v=%v", StatList[r].name, GetInteger(&buffer[rpos], rpos));
				else if(strcmp(StatList[r].type, "float") == 0)
					g_Logs.server->debug("%v=%v", StatList[r].name, GetFloat(&buffer[rpos], rpos));
			}
		}
	}
	g_Logs.server->debug("[/_handleCreatureUpdateMsg]");
}

void decode_handleEnvironmentUpdateMsg(const char *buffer)
{
	g_Logs.server->debug("[_handleEnvironmentUpdateMsg]");

	char extbuf[4096];
	int rpos = 0;
	unsigned char mask = GetByte(&buffer[rpos], rpos);
	if(mask == 0)
	{
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		g_Logs.server->debug("instanceZoneString=%v", extbuf);

		g_Logs.server->debug("zoneDefID=%v", GetInteger(&buffer[rpos], rpos));
		g_Logs.server->debug("PageSize=%v", GetShort(&buffer[rpos], rpos));

		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		g_Logs.server->debug("TerrainConfig=%v", extbuf);
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		g_Logs.server->debug("EnvironmentType=%v", extbuf);
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		g_Logs.server->debug("MapName=%v", extbuf);
	}
	g_Logs.server->debug("[/_handleEnvironmentUpdateMsg]");
}

void decode_handleQueryResultMsg(const char *buffer)
{
	int rpos = 0;
	char extbuf[4096];
	g_Logs.server->debug("QueryID=%v", GetInteger(&buffer[rpos], rpos));
	short rows = GetShort(&buffer[rpos], rpos);
	g_Logs.server->debug("Rows=%v", rows);
	for(int a = 0; a < rows; a++)
	{
		unsigned char strings = GetByte(&buffer[rpos], rpos);
		g_Logs.server->debug("Strings=%v", rows);
		for(int b = 0; b < strings; b++)
		{
			GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
			g_Logs.server->debug("[%v][%v]=%v", a, b, extbuf);
		}
	}
}
