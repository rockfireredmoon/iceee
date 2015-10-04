#include "Util.h"
#include "StringList.h"
#include "Config.h"
#include "Globals.h"
#define OUTPUT   g_Log.AddMessageFormat

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
			OUTPUT("Not enough data (at: %d, has: %d", curPos, buflen);
			break;
		}
		type = GetByte(&buffer[curPos], curPos);
		size = GetShort(&buffer[curPos], curPos);
		if(buflen - curPos < size)
		{
			OUTPUT("Not enough data for payload");
			break;
		}
		switch(type)
		{
		case 1: decode_handleQueryResultMsg(&buffer[curPos]); break;
		case 5: decode_handleCreatureUpdateMsg(&buffer[curPos]); break;
		case 42: decode_handleEnvironmentUpdateMsg(&buffer[curPos]); break;
		default:
			OUTPUT("Unhandled message type: %d (%d bytes)", type, size);
		}
		curPos += size;
	}
}

void decode_handleCreatureUpdateMsg(const char *buffer)
{
	OUTPUT("[_handleCreatureUpdateMsg]");
	char extbuf[4096];
	int rpos = 0;
	int cid = GetInteger(&buffer[rpos], rpos);
	short mask;
	if(g_ProtocolVersion < 22)
		mask = GetByte(&buffer[rpos], rpos);
	else
		mask = GetShort(&buffer[rpos], rpos);

	OUTPUT("CreatureID=%d", cid);
	OUTPUT("mask=%d", mask);

	if(mask & CREATURE_UPDATE_TYPE)
		OUTPUT("CreatureDefID=%d", GetInteger(&buffer[rpos], rpos));
	if(mask & CREATURE_UPDATE_ZONE)
	{
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		OUTPUT("CurrentZone=%s", extbuf);
		OUTPUT("CurrentX=%d", GetInteger(&buffer[rpos], rpos));
		OUTPUT("CurrentZ=%d", GetInteger(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_ELEVATION)
	{
		OUTPUT("ypos=%d", GetShort(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_POSITION_INC)
	{
		OUTPUT("xpos=%d", GetShort(&buffer[rpos], rpos));
		OUTPUT("zpos=%d", GetShort(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_VELOCITY)
	{
		OUTPUT("heading=%d", GetByte(&buffer[rpos], rpos));
		OUTPUT("rotation=%d", GetByte(&buffer[rpos], rpos));
		OUTPUT("speed=%d", GetByte(&buffer[rpos], rpos));
	}
	if(mask & CREATURE_UPDATE_MOD)
	{
		short modCount = GetShort(&buffer[rpos], rpos);
		for(int a = 0; a < modCount; a++)
		{
			if(g_ProtocolVersion > 15)
			{
				int priority = GetInteger(&buffer[rpos], rpos);
				OUTPUT("priority=%d", priority);
				OUTPUT("ID=%d", GetShort(&buffer[rpos], rpos));
				OUTPUT("AbilityID=%d", GetShort(&buffer[rpos], rpos));
				if(priority == 1)
					OUTPUT("amount=%g", GetFloat(&buffer[rpos], rpos));
				else
					OUTPUT("amount=%d", GetShort(&buffer[rpos], rpos));
			}
			else
			{
				OUTPUT("ID=%d", GetShort(&buffer[rpos], rpos));
				OUTPUT("AbilityID=%d", GetShort(&buffer[rpos], rpos));
				OUTPUT("amount=%d", GetShort(&buffer[rpos], rpos));
			}
			OUTPUT("duration=%d", GetInteger(&buffer[rpos], rpos));
			if(g_ProtocolVersion >= 24)
			{
				GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
				OUTPUT("description=%s", extbuf);
			}
		}

		short numStatusEffects = GetShort(&buffer[rpos], rpos);
		OUTPUT("numStatusEffects=%d", numStatusEffects);
		for(int a = 0; a < numStatusEffects; a++)
			OUTPUT("statID=%d", GetShort(&buffer[rpos], rpos));

	}
	if(mask & CREATURE_UPDATE_STAT)
	{
		short numStats = GetShort(&buffer[rpos], rpos);
		OUTPUT("numStats=%d", numStats);
		for(int a = 0; a < numStats; a++)
		{
			short statID = GetShort(&buffer[rpos], rpos);
			int r = GetStatIndex(statID);
			if(r == -1)
			{
				OUTPUT("Stat ID not found: %d", statID);
			}
			else
			{
				if(strcmp(StatList[r].type, "string") == 0)
				{
					GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
					OUTPUT("%s=%s", StatList[r].name, extbuf);
				}
				else if(strcmp(StatList[r].type, "short") == 0)
					OUTPUT("%s=%d", StatList[r].name, GetShort(&buffer[rpos], rpos));
				else if(strcmp(StatList[r].type, "int") == 0)
					OUTPUT("%s=%d", StatList[r].name, GetInteger(&buffer[rpos], rpos));
				else if(strcmp(StatList[r].type, "integer") == 0)
					OUTPUT("%s=%d", StatList[r].name, GetInteger(&buffer[rpos], rpos));
				else if(strcmp(StatList[r].type, "float") == 0)
					OUTPUT("%s=%g", StatList[r].name, GetFloat(&buffer[rpos], rpos));
			}
		}
	}
	OUTPUT("[/_handleCreatureUpdateMsg]");
}

void decode_handleEnvironmentUpdateMsg(const char *buffer)
{
	OUTPUT("[_handleEnvironmentUpdateMsg]");

	char extbuf[4096];
	int rpos = 0;
	unsigned char mask = GetByte(&buffer[rpos], rpos);
	if(mask == 0)
	{
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		OUTPUT("instanceZoneString=%s", extbuf);

		OUTPUT("zoneDefID=%d", GetInteger(&buffer[rpos], rpos));
		OUTPUT("PageSize=%d", GetShort(&buffer[rpos], rpos));

		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		OUTPUT("TerrainConfig=%s", extbuf);
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		OUTPUT("EnvironmentType=%s", extbuf);
		GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
		OUTPUT("MapName=%s", extbuf);
	}
	OUTPUT("[/_handleEnvironmentUpdateMsg]");
}

void decode_handleQueryResultMsg(const char *buffer)
{
	int rpos = 0;
	char extbuf[4096];
	OUTPUT("QueryID=%d", GetInteger(&buffer[rpos], rpos));
	short rows = GetShort(&buffer[rpos], rpos);
	OUTPUT("Rows=%d", rows);
	for(int a = 0; a < rows; a++)
	{
		unsigned char strings = GetByte(&buffer[rpos], rpos);
		OUTPUT("Strings=%d", rows);
		for(int b = 0; b < strings; b++)
		{
			GetStringUTF(&buffer[rpos], extbuf, sizeof(extbuf), rpos);
			OUTPUT("[%d][%d]=%s", a, b, extbuf);
		}
	}
}