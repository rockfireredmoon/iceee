#pragma once

#ifndef ACTIVECHARACTER_H
#define ACTIVECHARACTER_H

#include <string.h>  //For memset
#include "Stats.h"
#include "Character.h"  //For IntContainer

class AccountData;
class ZoneDefInfo;
class QuestJournal;

class MultiVar
{
public:
	MultiVar() { data.lval = 0; }
	MultiVar(long value) { data.lval = value; }
	MultiVar(void *value) { data.pval = value; }
	long GetLong(void) {return data.lval; }
	void* GetPtr(void) {return data.pval; }
	~MultiVar() {}
	union
	{
		long lval;
		void *pval;
	} data;
};

struct CharacterServerData
{
	int CreatureDefID;  
	int CreatureID;
	AccountData *accPtr;     //Pointer to the account this simulator has logged into
	CharacterData *charPtr;  //Pointer to the character this simulator has logged into

	ZoneDefInfo *zoneDef;   //Zone that this character is placed into.
	int CurrentInstanceID;  //Instance ID that the character is logged into.
	int CurrentZoneID;      //Zone ID that the character is logged into.
	char CurrentZone[40];   //Constructed ZoneID "[InstanceID-Map-Unknown]".  The result shouldn't exceed this size even with maximum integer values with signs.

	int oldSpawnX;          //For movement among the spawning tiles, to determine if an update is necessary
	int oldSpawnZ;

	int PendingMovement;    //Movement steps that have been processed since the last frame
	int MovementStep;
	unsigned long MovementTime;
	unsigned long MovementBlockTime;  //Block movement until this time is reached.
	unsigned long ResendSpeedTime;
	unsigned long LastAbilityErrorMessageTime;  //A timer to help prevent spam of ability error messages
	unsigned long NextPartyLocationBroadcastTime;   //Determines when location should next be broadcasted to the party.
	int LastAbilityErrorMessagePriority;
	bool IgnoreNextMovement;

	long TotalDistanceMoved;         //Total distance moved since logging in.
	long LastCheckDistanceMoved;     //The last distance check.
	unsigned long NextIdleCheckTime; //Time to perform the next idle check. 
	int LastCastX;                   //Location of last ability activation.
	int LastCastZ;                   //Location of last ability activation.
	int IdleCastCount;               //Number of consecutive abilities cast from the same location.

	short CurrentMapInt;    //Internal Mapdef index
	short LastMapTick;      //Current tick of the map changing system
	char PortalRequestDest[32];
	char CurrentEnv[40]; //Internal current environment

	int DeltaY;
	bool bFalling;

	bool mItemUseInProgress;
	unsigned int mItemUseCCSID;

	int DebugPingServerNotifyTime;
	int DebugPingServerLowest;
	int DebugPingServerHighest;
	int DebugPingServerTotalTime;
	int DebugPingServerTotalReceived;
	int DebugPingServerSent;
	int DebugPingServerLastMsgReceived;
	int DebugPingServerTotalMsgReceived;

	int DebugPingClientID;
	int DebugPingClientSuccessCount;
	int DebugPingClientFailCount;
	int DebugPingClientTimeoutCount;
	int DebugPingClientLowestTime;
	int DebugPingClientHighestTime;
	int DebugPingClientTotalTime;
	int DebugPingClientReceivedCount;
	
	CharacterServerData();
	void Reset(void);
	void SetPortalRequestDest(const char *locationName);
	void ClearPortalRequestDest(void);
	void UpdateNextIdleCheckTime(void);
	bool VerifyIdle(void);
	bool NotifyCast(int locationX, int locationZ, int abilityID);

	QuestJournal* GetQuestJournal(void);
};


int PrepExt_SetMap(char *buffer, CharacterServerData *pldata, int x, int z);

#endif //ACTIVECHARACTER_H
