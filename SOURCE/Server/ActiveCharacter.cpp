#include "ActiveCharacter.h"
#include "Util.h"
#include "Config.h"
#include "ZoneDef.h"
#include "StringList.h"
#include <stdlib.h>

CharacterServerData :: CharacterServerData()
{
	Reset();
}

void CharacterServerData:: Reset(void)
{
	memset(this, 0, sizeof(CharacterServerData));
}

void CharacterServerData :: SetPortalRequestDest(const char *locationName)
{
	Util::SafeCopy(PortalRequestDest, locationName, sizeof(PortalRequestDest));
}


void CharacterServerData :: ClearPortalRequestDest(void)
{
	memset(PortalRequestDest, 0, sizeof(PortalRequestDest));
}

void CharacterServerData :: UpdateNextIdleCheckTime(void)
{
	NextIdleCheckTime = g_ServerTime + g_Config.IdleCheckFrequency;
	LastCheckDistanceMoved = TotalDistanceMoved;
}

bool CharacterServerData :: VerifyIdle(void)
{
	//Return true if the player has been determined to be idle or botting.
	if(g_Config.IdleCheckVerification == false)
		return false;
	if(zoneDef == NULL)
		return false;
	if(zoneDef->mGrove == true)
		return false;
	if(zoneDef->mGuildHall == true)
		return false;
	if(zoneDef->mInstance == true)
		return false;

	if((TotalDistanceMoved - LastCheckDistanceMoved) < g_Config.IdleCheckDistance)
		return true;

	return false;
}

bool CharacterServerData :: NotifyCast(int locationX, int locationZ, int abilityID)
{
	int xlen = abs(locationX - LastCastX);
	int zlen = abs(locationZ - LastCastZ);
	if(xlen <= g_Config.IdleCheckDistanceTolerance && zlen <= g_Config.IdleCheckDistanceTolerance)
	{
		IdleCastCount++;
		if(IdleCastCount >= g_Config.IdleCheckCast)
		{
			NextIdleCheckTime -= g_Config.IdleCheckCastInterval;
			IdleCastCount = 0;
			return true;
		}
	}
	else
	{
		IdleCastCount = 0;
		LastCastX = locationX;
		LastCastZ = locationZ;
	}
	return false;
}

QuestJournal* CharacterServerData :: GetQuestJournal(void)
{
	if(charPtr == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] GetQuestJournal() charPtr is NULL");
		return NULL;
	}
	return &charPtr->questJournal;
}



int PrepExt_SetMap(char *buffer, CharacterServerData *csd, int x, int z)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 42);   //_handleEnvironmentUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutByte(&buffer[wpos], 0);   //Mask

	wpos += PutStringUTF(&buffer[wpos], csd->CurrentZone);    //zoneID
	wpos += PutInteger(&buffer[wpos], csd->zoneDef->mID);      //zoneDefID
	wpos += PutShort(&buffer[wpos], csd->zoneDef->mPageSize);  //zonePageSize
	wpos += PutStringUTF(&buffer[wpos], csd->zoneDef->mTerrainConfig.c_str());   //Terrain
	wpos += PutStringUTF(&buffer[wpos], csd->zoneDef->GetTileEnvironment(x,z)->c_str());   //envtype
	wpos += PutStringUTF(&buffer[wpos], csd->zoneDef->mMapName.c_str());   //mapName

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}
