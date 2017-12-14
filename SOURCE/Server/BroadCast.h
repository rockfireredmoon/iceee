/*
This class originally handled a queue of broadcast messages that were to be sent to all clients.
Originally the server was based on heavy multithreaded model which was horrendously buggy with
crashes and deadlocks, so these messages were required to route communications to each gameplay
simulator.

Now that all simulator processing is called by the main thread, these messages are technically
not required for network communication.

It is an old mechanism, still in use for backward compatibility with old functions (too lazy to
change what already worked) and is generally not required for any new code.  However, it does have
some specific uses for passing other notifications around.
*/

#ifndef BROADCAST_H
#define BROADCAST_H

#include <vector>
#include "Components.h"

class ActiveInstance;

enum BCMEnum
{
	BCM_None = 0,          //No message
	BCM_CreatureDefRequest, //A CreatureDef request.  The message is triggered by and operational for a single simulator query.
	BCM_UpdateCreatureDef, //Update all stats of a particular creature definition
	BCM_UpdateCreatureInstance,  //Update all stats of a particular creature instance.
	BCM_UpdateVelocity,    //A creature's velocity must be message.
	BCM_UpdatePosition,    //A creature's position must be explicitly set
	BCM_UpdateAppearance,  //A creature's appearance has been updated, probably by CreatureTweak
	BCM_UpdatePosInc,      //Incremental position update?
	BCM_UpdateElevation,   //Update entity elevation
	BCM_SendHealth,        //Send health
	BCM_RequestTarget,     //Param1 = (CreatureInstance*) to modify, Param2 = Instance ID of selected object
	BCM_AbilityRequest,    //Handle an ability request
	BCM_AbilityActivate2,  //Handle a forced ability activation
	BCM_ActorJump,         //Handle a jump ability
	BCM_Disconnect,        //Handle a simulator disconnect param1=SimulatorID, param2=unused

	BCM_RemoveCreature,    //Remove object from the creature list
	BCM_UpdateFullPosition, //Full update

	BCM_Notice_MOTD,       //No parameters
	BCM_SpawnCreateCreature,   //Param1 = Creature Instance Ptr of spawner, Param2 = CDef ID
	BCM_CreatureDelete,  //Param1 = Creature Instance Ptr of deleter, Param2 = Creature ID

	BCM_PlayerLogIn,          //Param1 = CharacterData*, Param2 = Simulator ignore index
	BCM_PlayerLogOut,         //Param1 = CharacterData*, Param2 = Simulator ignore index
	BCM_PlayerFriendLogState, //Param1 = SimulatorThread*, Param2 = LoginState (0 or 1)

	BCM_SidekickAttack,     //Order all sidekicks to attack the target.  Param1 = CreatureInstance *host
	BCM_SidekickCall,       //Cancel attacks and calls sidekicks to the host.  Param1 = CreatureInstance *host
	BCM_SidekickScatter,    //Order all sidekicks to scatter around either their host if inactive, or target.  Param1 = CreatureInstance *host
	BCM_SidekickWarp,       //Order all sidekicks to warp to the player regardless of current position.  To be used on instance changes.  Param1 = CreatureInstance *host

	BCM_RunObjectInteraction, //Triggers processing of an interaction by a character
	BCM_RunTranslocate,       //Triggers a simulator to return the player to its bind point, which may be outside an instance.
	BCM_RunPortalRequest,     //Triggers a request to jump to a henge interact object by name


	BCM_SidekickDefend,       //Calls sidekicks to defend the host.  Param1 = CreatureInstance *host
};

struct MessageComponent
{
	int SimulatorID;   //The calling simulator ID
	ActiveInstance *actInst;    //Pointer to the instance that this message is intended for
	long param1;       //The character index into the server's internal data array (not the actual Creature ID)
	long param2;       //
	short message;     //The broadcast message to send
	int x;             //Location of the player (for local broadcasting)
	int z;
};

class BroadcastMessage2
{
public:
	BroadcastMessage2();
	~BroadcastMessage2();
	std::vector<MessageComponent> mlog;

	int AddEventCopy(MessageComponent &msg);
	int AddEvent(int newSimID, long newParam1, int newMessage, ActiveInstance* newActInst);
	int AddEvent2(int newSimID, long newParam1, long newParam2, int newMessage, ActiveInstance* newActInst);
	int RemoveSimulator(int SimulatorID);
	
	void EnterCS(const char *request);
	void LeaveCS(void);
	void Init(void);
	void Free(void);

private:
	Platform_CriticalSection cs;
};

extern BroadcastMessage2 bcm;

#endif //BROADCAST_H
