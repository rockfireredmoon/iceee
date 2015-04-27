//Zone Definitions, Map Definitions and World Instances

#ifndef INSTANCE_H
#define INSTANCE_H

#define CREATUREMAP 1
#define CREATUREQAV 1

#include <vector>
#include <list>

#include "BroadCast.h"
#include "ActiveCharacter.h" //For CharacterServerData class definition
#include "Creature.h"
//#include "CreatureSpawner.h"
#include "CreatureSpawner2.h"
#include "NPC.h"
#include "EssenceShop.h"
#include "DropTable.h"
#include "Trade.h"
#include "ZoneDef.h"
#include "QuestScript.h"
#include "InstanceScript.h"
#include "Arena.h"
using namespace std;

class SimulatorThread;
class InstanceScaleProfile;

extern const int SANE_DISTANCE;
extern const int DISTANCE_FAILED;

extern int InstanceUpdateDelay;
extern unsigned long INSTANCE_DELETE_MAXTIME;
extern unsigned long INSTANCE_DELETE_RECHECK;
extern unsigned long CREATURE_DELETE_RECHECK;

extern const int FACTION_PLAYERFRIENDLY;
extern const int FACTION_PLAYERHOSTILE;

typedef std::vector<SceneryEffect>      SceneryEffectList;
typedef std::pair<int, SceneryEffectList> SceneryEffectPair;
typedef std::map<int, SceneryEffectList> SceneryEffectMap;

struct MapDefInfo
{
	string Name;
	string Primary;
	string Type;
	string parentMapImage;
	string image;
	float numPagesAcross;
	float numPagesDown;
	int u0;
	int v0;
	int u1;
	int v1;
	short priority;
	float scale_width;
	float scale_height;
};

class MapDefContainer
{
public:
	MapDefContainer();
	~MapDefContainer();

	vector<MapDefInfo> mMapList;
	int CreateMap(void);
	int SearchMap(const char *primary, int xpos, int ypos);
	int GetIndexByName(const char *name, const char *type);
	int LoadFile(const char *fileName);
	void FreeList(void);
};

// Locations are subdivisions of a primary region.  Specific locations on the overworld
// can be mapped to an arbitrary location, which allows for irregularly shaped
// territories to be more accurately defined.  This system uses a range of smaller
// bounding boxes, currently rows of tiles, for each definition.  Each of these
// locations is filled with coordinate positions to search within, and a name of the
// target map to use.
class MapLocationDef
{
public:
	MapLocationDef();
	~MapLocationDef();

	//This information is acquired during the load process and must be resolved later.
	string MapName;         //Name of the target map to use.
	short method;           //Method to expand the locations based on the tiles given
	short tilex;            //Starting X tile
	short tiley;            //Starting Y tile
	short spanx;            //Span of tiles along the X axis
	short spany;            //Span of tiles along the Y axis
	
	//This information is finalized after the load process, or is otherwise used
	//during run time lookups for location information.
	short priority;         //Priority for overlapping bounding boxes
	int resolvedIndex;      //If the target location has been resolved, this will be >= 0 as an index into the MapDef list.
	int x1;
	int y1;
	int x2;
	int y2;

	void clear(void);
};

//This class handles a set of location definitions.  All definitions within this set
//will correspond to a specific zone definition.
class MapLocationSet
{
public:
	MapLocationSet();
	~MapLocationSet();

	vector<MapLocationDef> mLocationList;
	int mZone;

	bool HasLocation(int xcoord, int zcoord);
};

//This is the base container class of all location definitions.  It can add new
//definitions, automatically sorting out which zone they belong to.  It is also
//responsible for searching the existing set for a particular zone, then
//searching the definitions within that zone for a more specific location area.
class MapLocationHandler
{
public:
	MapLocationHandler();
	~MapLocationHandler();

	vector<MapLocationSet> mLocationSet;
	int AddLocation(int zone, MapLocationDef &data);
	int ZoneExist(int zone);
	int SearchLocation(int zone, int x, int z);
	const char* GetInternalMapName(int zone, int x, int z);
	int ResolveItems(void);
	int LoadFile(const char *filename);
};

struct ScaleConfig
{
	int mPLayerLevel;
	bool mIsScaled;
	ScaleConfig()
	{
		mPLayerLevel = 0;
		mIsScaled = false;
	}
	bool IsScaled(void)
	{
		return mIsScaled;
	}
};

//Holds all the data required to log a player into an instance.
struct PlayerInstancePlacementData
{
	CreatureInstance *in_cInst;   //Pointer to the default player data.  This will be copied into the newly generated creature instance.
	SimulatorThread *in_simPtr;   //Pointer to the calling simulator.  Required to properly initialize the character instance so it has player data to point back to.
	int in_instanceID;            //Priority: highest.  Optional.  If this field is set, retrieve this exact instance if it exists.
	int in_partyID;               //Priority: high.  Recommended.  The party ID that the player belongs to, or zero if no party.
	int in_creatureDefID;         //Priority: normal.  Required.  If no party ID is provided, the instance will be created with this owner.
	int in_zoneID;                //Required.  The zone to create or retrieve (if the exact instance is not matched).
	int in_serverSessionID;       //If the player is in an instance when the server shuts down, this session ID must be used to resolve whether an instance has expired, since it's possible for instance IDs and other data to be the same in different sessions.
	
	int in_playerLevel;           //The player's level. Used for scale mode if no party ID is present.
	const InstanceScaleProfile *in_scaleProfile; //The scale profiler to use.

	CreatureInstance *out_cInst;  //On success, this will point to the CreatureInstance object of the registered player's creature instance.
	ZoneDefInfo *out_zoneDef;     //On success, this is the pointer to zone data that this instance uses.
	int out_instanceID;           //The instance ID that the player has been placed into.

	PlayerInstancePlacementData();
	void Clear();
	void SetInstanceScaler(const std::string &name);

private:
	int mPartyLeaderDefID;

	const InstanceScaleProfile* GetPartyLeaderInstanceScaler(void);

};

class ActiveInstance
{
public:
	ActiveInstance();
	~ActiveInstance();

	int mZone;                //The zone that this instance is for
	ZoneDefInfo *mZoneDefPtr; //Pointer to the ZoneDef this instance was derived from.
	int mInstanceID;          //The Instance ID, unique to all other instances.
	int mPlayers;             //Number of players registered into this instance.
	int mOwnerCreatureDefID;  //This is the DefID of the player who created this instance (actived for Instance-type zones)
	int mOwnerPartyID;        //If nonzero, this instance (if a dungeon) will be assigned ownership to a party ID to assist in lookups when placing players into dungeons.
	std::string mOwnerName;   //The owner's display_name.
	unsigned long mLastHealthUpdate;  //Time of the last health increment.  TODO: OBSOLETE
	unsigned long mNextCreatureBroadcast;
	unsigned long mLastMovementUpdate;
	unsigned long mNextTargetUpdate;
	unsigned long mNextUpdateTime;   //Next general update time
	unsigned long mNextCreatureLocalScan;
	unsigned long mNextCreatureDeleteScan;

	unsigned long mExpireTime;       //The scheduled expire time for this instance

	static const int DEFAULT_CREATURE_ID = 3000000;
	int GetNewActorID(void);


	int mNextEffectTag;

	SceneryEffectMap mSceneryEffects;

	typedef std::map<int, CreatureInstance> CREATURE_MAP;
	typedef std::pair<int, CreatureInstance> CREATURE_PAIR;
	typedef std::map<int, CreatureInstance>::iterator CREATURE_IT;

	list<CreatureInstance> PlayerList;
	vector<CreatureInstance*>PlayerListPtr;

	CREATURE_MAP NPCList;
	vector<CreatureInstance*>NPCListPtr;

	list<CreatureInstance> SidekickList;
	vector<CreatureInstance*>SidekickListPtr;

	vector<SimulatorThread*> RegSim;
	list<QuestScript::QuestScriptPlayer> questScriptList;

	HateProfileContainer hateProfiles;
	WorldLootContainer lootsys;
	TradeManager tradesys;
	SpawnManager spawnsys;
	InstanceScript::InstanceScriptDef scriptDef;
	InstanceScript::InstanceScriptPlayer scriptPlayer;
	InstanceScript::InstanceNutDef nutScriptDef;
	InstanceScript::InstanceNutPlayer nutScriptPlayer;
	//std::list<InstanceScript::ScriptPlayer> mConcurrentInstanceScripts;   DISABLED, NOT FINISHED

	EssenceShopContainer essenceShopList;
	EssenceShopContainer itemShopList;

	ScaleConfig scaleConfig;
	const InstanceScaleProfile *scaleProfile;
	const DropRateProfile *dropRateProfile;
	float mDropRateBonusMultiplier;   //Progressive drop rate multiplier, increased slightly per mob kill.  Only applies to dungeons.
	int mKillCount;                   //Total kill count of the dungeon.  No purpose than some generic tracking.
	
	ArenaRuleset arenaRuleset;

	UniqueSpawnManager uniqueSpawnManager;

	CreatureInstance * GetPlayerByID(int id);  //Searches PlayerList for a character ID, return pointer
	CreatureInstance * GetPlayerByCDefID(int CDefID);  //All player characters have unique creatureDefs.
	CreatureInstance * GetPlayerByName(const char *name);
	CreatureInstance * GetPlayerByID(int id, list<CreatureInstance>::iterator &resIterator);  //Optional version that returns a valid iterator to the found object, in case it needs to be deleted 
	int DeletePlayerByID(int id);               //Delete PlayerList element with character ID

	void Clear();
	void InitializeData(void);
	int SimExist(SimulatorThread *simPtr);

	CreatureInstance * LoadPlayer(CreatureInstance *source, SimulatorThread *simCall);
	int UnloadPlayer(SimulatorThread *callSim);
	int RemovePlayerByID(int creatureID);

	int ProcessMessage(MessageComponent *msg);
	void BroadcastMessage(const char *message);
	int LSendToAllSimulator(const char *buffer, int length, int ignoreIndex);
	void LSendToLocalSimulator(const char *buffer, int length, int x, int z, int ignoreIndex = -1);
	int LSendToOneSimulator(const char *buffer, int length, int simIndex);
	void LSendToOneSimulator(const char *buffer, int length, SimulatorThread *simPtr);

	void SendActors(void);  //Updates all NPC objects and sends updates to each active simulator
	void UpdateCreatureLocalStatus(void);  //Scans NPCs to determine which ones should stay active for detailed processing

	int AddSceneryEffect(char *outbuf, SceneryEffect *effect);
	int DetachSceneryEffect(char *outBuf, int sceneryId, int effectType, int tag);
	SceneryEffectList * GetSceneryEffectList(int PropID);
	SceneryEffect * RemoveSceneryEffect(int PropID, int tag);

	static int GetBoxRange(CreatureInstance *obj1, CreatureInstance *obj2);
	static int GetPlaneRange(CreatureInstance *obj1, CreatureInstance *obj2, int threshold);
	static int GetPointRange(CreatureInstance *obj1, float x, float y, float z, int threshold);
	static int GetPointRangeXZ(CreatureInstance *obj1, float x, float z, int threshold);
	static int GetActualRange(CreatureInstance *obj1, CreatureInstance *obj2, int threshold);
	static int ActivateAbility(CreatureInstance *cInst, short ability, int ActionType, ActiveAbilityInfo *abInfo);
	CreatureInstance * GetInstanceByCID(int CID);
	CreatureInstance * GetNPCInstanceByCID(int CID);
	void GetNPCInstancesByCDefID(int CDefID, vector<int> cids);
	CreatureInstance * GetNPCInstanceByCDefID(int CDefID);
	void ResolveCreatureDef(int CreatureInstanceID, int *responsePtr);

	void RebuildPlayerList(void);
	void RebuildNPCList(void);

	CreatureInstance* SpawnCreate(CreatureInstance * sourceActor, int CDefID);
	CreatureInstance* SpawnGeneric(int CDefID, int x, int y, int z, int facing, int flags);
	void SpawnAtProp(int CDefID, int PropID, int duration, int elevationOffset);
	void CreatureDelete(int CreatureID);

	void EraseAllCreatureReference(CreatureInstance *object);
	int EraseIndividualReference(CreatureInstance *object);

	int RemoveNPCInstance(int CreatureID);
	void UnHate(int CreatureDefID);

	void AdjustPlayerCount(int offset);
	bool QualifyDelete(void);

	CreatureInstance * GetClosestEnemy(CreatureInstance *orig);
	CreatureInstance * GetClosestAggroTarget(CreatureInstance *sourceActor);
	int PartyAll(CreatureInstance* host, char *outBuf);

	//Sidekick addition/removal
	void RebuildSidekickList(void);
	CreatureInstance* InstantiateSidekick(CreatureInstance *host, SidekickObject &skobj, int count);
	int CreateSidekick(CreatureInstance* host, SidekickObject &skobj);
	int SidekickRemoveOne(CreatureInstance* host, vector<SidekickObject> *sidekickList);
	int SidekickRemoveAll(CreatureInstance* host, vector<SidekickObject> *sidekickList);
	int SidekickRegister(CreatureInstance* host, vector<SidekickObject> *sidekickList);
	int SidekickUnregister(CreatureInstance* host);
	int SidekickLow(CreatureInstance* host, int percent);
	int SidekickParty(CreatureInstance* host, char *outBuf);
	void UpdateSidekickTargets(CreatureInstance *officer);

	void SidekickAttack(CreatureInstance* host);
	void SidekickCall(CreatureInstance* host);
	void SidekickWarp(CreatureInstance *host);
	void SidekickScatter(CreatureInstance* host);

	void RemoveDeadCreatures(void);
	CreatureInstance * inspectCreature(int CreatureID);
	void RunScripts(void);
	//void AttachConcurrentInstanceScript(const char *label);   DISABLED, NOT FINISHED
	void SendLoyaltyAggro(CreatureInstance *instigator, CreatureInstance *target, int loyaltyRadius);
	void SendLoyaltyLinks(CreatureInstance *instigator, CreatureInstance *target, SceneryObject *spawnPoint);
	QuestScript::QuestScriptPlayer* GetSimulatorQuestScript(SimulatorThread *simulatorPtr);
	void RunProcessingCycle(void);
	void UpdateEnvironmentCycle(const char *timeOfDay);
	bool KillScript();
	bool RunScript();
	void ScriptCallKill(int CreatureDefID, int CreatureID);
	void ScriptCallUse(int CreatureDefID);
	void ScriptCallUseHalt(int CreatureDefID);
	void ScriptCallUseFinish(int CreatureDefID);
	void ScriptCall(const char *name);
	void FetchNearbyCreatures(SimulatorThread *simPtr, CreatureInstance *player);
	void RunObjectInteraction(SimulatorThread *simPtr, int CDef);
	void ApplyCreatureScale(CreatureInstance *target);
	const DropRateProfile* GetDropRateProfile(void);

	int CountAlive(int creatureDefID);
	void LoadStaticObjects(const char *filename);

	CreatureInstance* GetMatchingSidekick(CreatureInstance *host, int searchID);
	void SetAllPlayerPVPStatus(int x, int z, int range, bool state);
	void NotifyKill(int mobRarity);

	void Script_ScanNPCCID(InstanceLocation *location, std::vector<int>& destResult);
};

class ActiveInstanceManager
{
public:
	ActiveInstanceManager();
	~ActiveInstanceManager();

	std::list<ActiveInstance> instList;
	std::vector<ActiveInstance*> instListPtr;
	unsigned long nextInstanceCheck;

	static const int BASE_INSTANCE_ID = 1000;
	static const int MAX_INSTANCE_ID = 32767;
	int NextInstanceID;
	
	void RebuildAccessList(void);
	ActiveInstance * CreateInstance(int zoneID, PlayerInstancePlacementData &pd);
	ActiveInstance * GetPtrByZoneOwner(int zoneID, int ownerDefID);
	ActiveInstance * GetPtrByZoneInstanceID(int zoneID, int instanceID);
	ActiveInstance * GetPtrByZonePartyID(int zoneID, int partyID);
	ActiveInstance * GetPtrByZoneID(int zoneID);
	int FlushSimulator(int SimulatorID);
	int GetNewInstanceID(void);
	void InitializeData(void);

	ActiveInstance* ResolveExistingInstance(PlayerInstancePlacementData& pd, ZoneDefInfo* zoneDef);
	int AddSimulator_Ex(PlayerInstancePlacementData &pd);

	CreatureInstance* GetPlayerCreatureByName(const char *name);
	CreatureInstance* GetPlayerCreatureByDefID(int CreatureDefID);

	void CheckActiveInstances(void);
	void DebugFlushInactiveInstances(void);
	void Clear(void);
};

extern MapDefContainer MapDef;
extern MapLocationHandler MapLocation;
extern ActiveInstanceManager g_ActiveInstanceManager;

#endif  //INSTANCE_H
