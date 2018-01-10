#ifndef CREATURESPAWNER2_H
#define CREATURESPAWNER2_H

#include <map>
#include <vector>
#include <list>
#include "CommonTypes.h"

class SceneryObject;
class ActiveInstance;
class CreatureInstance;
extern unsigned long g_ServerTime;
class SpawnPackageDef;
class SpawnManager;

class Timer
{
public:
	Timer();
	~Timer();
	unsigned long NextFire;
	bool Ready();
	void Update(unsigned long newTimeOffset);
	bool ReadyWithUpdate(unsigned long newTimeOffset);
	static unsigned long GetTime(void)
	{
		return g_ServerTime;
	}
};

struct ExtraDataLink
{
	int propID;
	char type;
};

class CreatureSpawnDef
{
public:
	CreatureSpawnDef();
	~CreatureSpawnDef();

	static const int DEFAULT_MAXACTIVE = 1;
	static const int DEFAULT_DESPAWNTIME = 150;
	static const int DEFAULT_MAXLEASH = 500;
	
	std::string spawnName;      //Internal name for this spawner entity
	int leaseTime;           //Original purpose unknown... spawn delay?
	std::string spawnPackage;   //Package to base spawn types on
	std::string dialog;   		 //Name of NPC Dialog file
	int mobTotal;            //Total mobs (including dead?) that can be anchored to this point?
	int maxActive;           //Total active that can be anchored to this point?
	std::string aiModule;       //Unused?
	int maxLeash;            //Maximum attack distance until a mob is forced to return
	int loyaltyRadius;       //Distance to aggro nearby idle mobs
	int wanderRadius;        //Distance for random stray mobs
	int despawnTime;        //Unknown purpose.  Used for spawn times.
	int sequential;          //Unknown
	std::string spawnLayer;     //Unknown
	float xpos;              //Spawn position
	float ypos;              //Spawn position
	float zpos;              //Spawn position
	unsigned char facing;    //Directional facing on spawn (0-255)

	std::string sceneryName;
	int innerRadius;
	int outerRadius;
	std::vector<ExtraDataLink> link;
	void Clear();
	void copyFrom(CreatureSpawnDef *source);
	int GetLeashLength(void);
};

struct ActiveSpawner
{
	SceneryObject *spawnPoint;   //Pointer to the scenery object that holds the prop and extra spawn information.
	int spawnCount;              //Number of active spawns that have been generated.
	unsigned long nextSpawn;     //Time that the next spawn will trigger.
	SpawnPackageDef *spawnPackage;  //Pointer to the spawn package definition, for selecting a new spawn.
	short refCount;              //Number of creatures currently spawned by this point.
	std::vector<int> attachedCreatureID;  //A list of creature IDs attached to this spawn point.

	ActiveSpawner();
	bool HasCreature(int creatureID);
	void Clear(void);
	bool UpdateSourcePackage(void);
	void Invalidate(void);
	void UpdateSpawnDelay(int specificDelaySeconds);
	void OnSpawnSuccess(void);
	int GetRespawnDelaySeconds(void);
	int GetLoyaltyRadius(void);
	bool HasLoyaltyLinks(void);
	void AddAttachedCreature(int creatureID);
	void RemoveAttachedCreature(int creatureID);
};

class SpawnTile
{
public:
	typedef std::map<int, ActiveSpawner> SPAWN_MAP;
	typedef std::pair<int, ActiveSpawner> SPAWN_PAIR;   //Key is Prop ID

	/*	Notes on the following two static constants:
	SIZE is the width of a tile.
	RANGE is the number of additional tiles to load around a player.  The player
	is always standing in a 1x1 tile.  Therefore the visible grid will always have
	a width and height of ((RANGE * 2) + 1).
	So if RANGE=1, the player is located in the center of a 3x3 grid.

	Because players can be standing near the edges of a tile, the player may be able
	to see creatures between these two distances:
	Mininum: (SIZE * RANGE);
	Maximum: (SIZE * (RANGE + 1))
	So if SIZE is 500 and RANGE is 1, the effective visible range becomes 500 to 1000.

	10 units is equivalent to one "meter" in game.
	The longest skill range is 50 meters (500 units).

	The minimum visible range should always exceed 500 units.
	*/

	static const int SPAWN_TILE_SIZE = 560;
	static const int SPAWN_TILE_RANGE = 2;
	int TileX;
	int TileY;
	bool sceneryLoaded;
	unsigned long releaseTime;  //If the page is not accessed again before this time (access time + offset), flag it as garbage and delete it.
	SpawnManager *manager;
	SPAWN_MAP activeSpawn;
	std::vector<int> attachedCreatureID;  //Contains all Creature IDs attached to this tile.

	SpawnTile();
	~SpawnTile();
	void LoadPropSpawnPoint(int zone, int sceneryPageX, int sceneryPageY, int PropID);
	void LoadTileSpawnPoints(int zone, int sceneryPageX, int sceneryPageY, int x1, int y1, int x2, int y2);
	bool AddSpawnerFromProp(SPAWN_MAP::iterator &position, SceneryObject *prop);
	void RunProcessing(ActiveInstance *inst);
	bool Debug_VerifyProp(int zone, int propID, SceneryObject *ptr);
	CreatureInstance * SpawnCreature(ActiveInstance *inst, ActiveSpawner *spawner, int forceCreatureDef, int forceFlags);
	void RemoveAllAttachedCreature(ActiveInstance *inst);
	void RemoveAttachedCreature(int CreatureID);
	void UpdateSpawnPoint(SceneryObject *prop);
	void Despawn(int CreatureID);
	bool RemoveSpawnPoint(ActiveInstance *inst, int PropID);
	void RemoveSpawnPointCreatures(ActiveSpawner *spawner);
	void RemoveSpawnPointCreature(ActiveSpawner *spawner, int creatureID);
	ActiveSpawner * GetActiveSpawner(int PropID);
	bool QualifyDelete(void);
	void GetAttachedCreatures(int sceneryID, std::vector<int> &results);
};

//This class holds the data of a spawn package defition.  It allows
//multiple creature types to be defined as potential spawnables.
class SpawnPackageDef
{
public:
	SpawnPackageDef();
	~SpawnPackageDef();

	static const int FLAG_FRIENDLY = 1;
	static const int FLAG_HIDEMAP = 2;
	static const int FLAG_NEUTRAL = 4;
	//static const int FLAG_INVINCIBLE = 8;
	static const int FLAG_FRIENDLY_ATTACK = 16;
	static const int FLAG_ENEMY = 32;
	static const int FLAG_VISWEAPON_MELEE = 64;
	static const int FLAG_VISWEAPON_RANGED = 128;
	static const int FLAG_ALLBITS = 0xFFFF;
	static const int FLAG_USABLE = 512;
	static const int FLAG_USABLE_BY_COMBATANT = 1024;
	static const int FLAG_HIDE_NAMEBOARD = 2048;
	static const int FLAG_STATIONARY = 4096;
	static const int FLAG_KILLABLE = 8192;

	static const int MAX_SPAWNCOUNT = 12;
	static const int DEFAULT_MAXSHARES = 100;
	static const int MAX_ID = 0xFFFF;  //Should match the total integer size of a spawnID[] entry

	char packageName[56];  //Name to identify this package.

	int spawnCount;
	int maxShares;
	unsigned short SpawnFlags;
	unsigned short spawnID[MAX_SPAWNCOUNT];
	unsigned short spawnShare[MAX_SPAWNCOUNT];
	bool isScriptCall;    //If true, attempt to jump to a label in the instance script with the package name
	bool isSequential;    //If true, call an onKill label within the instance script.
	short loyaltyRadius;  //Loyalty radius override of the prop.
	short spawnDelay;     //Spawn delay override of the prop.
	short wanderRadius;   //Wander radius override of the prop.

	static const int MAX_POINT_OVERRIDE = 2;
	unsigned char numPointOverride;
	int PointOverridePropID[MAX_POINT_OVERRIDE];
	int PointOverrideCDef[MAX_POINT_OVERRIDE];
	
	//In rare cases, different locations may share the same spawn points, but have
	//different spawns based on vague east/west or north/south geographic lines.
	//These controls allow the spawn control to determine which "side" a group of
	//creatures may spawn on.
	static const char DIVIDER_VERTICAL = 1;
	static const char DIVIDER_HORIZONTAL = 2;
	char divideOrient;
	int divideLocation;
	short divideShareThreshold;
	short UniquePoints;      //If nonzero, indicates that a spawn should be unique, selected to appear randomly from a particular point.  This number should match the total number of possible spawnpoints of this package name.
	
	void Clear(void);
	int GetRandomSpawn(SceneryObject *spawnPoint);
	void AddPointOverride(int propID, int creatureDefID);
	bool HasCreatureDef(int CreatureDefID);
};

//A list of spawn package definitions, as loaded from a file.
//Each file is loaded into a single group.
class SpawnPackageList
{
public:
	SpawnPackageList();
	~SpawnPackageList();
	void Free(void);

	int ZoneID;
	std::vector<SpawnPackageDef> defList;
	int LoadFromFile(std::string filename);
	void AddIfValid(SpawnPackageDef &newItem);
	SpawnPackageDef * GetPointerByName(const char *name);
	SpawnPackageDef * HasCreatureDef(int CreatureDefID);
};

//The core container of all loaded spawn package groups.
class SpawnPackageManager
{
public:
	SpawnPackageManager();
	~SpawnPackageManager();

	//typedef std::map<int, SpawnPackageList> SPAWNPACKAGECONT;  //A dedicated list for each zoneID
	//SPAWNPACKAGECONT packageList;
	std::vector<SpawnPackageList> packageList;
	SpawnPackageDef nullSpawnPackage;
	int fakeCreatureId;

	void LoadFromFile(std::string subfolder, std::string filename);
	SpawnPackageDef * GetPointerByName(const char *name);
	//SpawnPackageList* GetZone(int zoneID);
	void EnumPackagesForCreature(int CreatureDefID, STRINGLIST &output);
};

class SpawnManager
{
	typedef std::pair<int, int> TILE_COORD;
	typedef std::vector<TILE_COORD> TILELIST_CONT;
public:
	std::list<SpawnTile> spawnTiles;
	std::list<int> genericSpawns;
	SpawnTile *GetTile(int tilePageX, int tilePageY);
	ActiveInstance *actInst;
	SpawnManager();
	~SpawnManager();
	void Clear(void);
	void SetInstancePointer(ActiveInstance *ptr);
	SpawnTile * GenerateTile(int tilePageX, int tilePageY);
	void GenerateAreaTile(int tilePageX, int tilePageY);
	void RunProcessing(bool force);
	bool NotifyKill(ActiveSpawner *sourceSpawner, int creatureID);
	void ScanActivePlayerTiles(TILELIST_CONT &outputList);
	void EnumAttachedCreatures(int sceneryID, int sceneryX, int sceneryZ, std::vector<int> &results);

	static const long PROCESSING_INTERVAL = 1000;     //How frequently to iterate over tiles and process spawns.
	static const long GARBAGE_CHECK_DELAY = 60000;    //Scan for garbage tiles at this interval.
	static const long GARBAGE_TILE_DELAY = 30000;     //Delete spawn tiles if their last access time is older than this.
	static const long GARBAGE_DELAY_DEAD = 45000;     //Delay time before dead mobs are removed from the server.
	static const long GARBAGE_DELAY_LOOT = 300000;    //Delay time before dead mobs (with loot) are removed from the server.

	void RunGarbageCheck(TILELIST_CONT &activeTileList);
	void UpdateSpawnPoint(SceneryObject *prop);
	void RemoveSpawnPoint(int PropID);
	int GetCIDForProp(int PropID);
	int TriggerSpawn(int PropID, int forceCreatureDef, int forceFlags);
	void Despawn(int CreatureID);

private:
	bool HasTileCoord(int x, int y, TILELIST_CONT &searchList);
	unsigned long nextGarbageCheck;
	unsigned long nextProcessTime;
};


struct UniqueSpawnEntry
{
	int mMaxSpawnCount;    //Maximum number spawn points to roll from.
	size_t mRandomIndex;      //The chosen random index of a prop in the list to spawn from.
	int mSpawnTime;        //Delay between restarting the cycle when a reroll is requested.
	unsigned long mRestartTime;  //Time required to allow a new spawn cycle.
	std::vector<int> mPropID;  //IDs of the SpawnPoints that called a request.
	UniqueSpawnEntry();
	size_t GetPropIndex(int PropID);
	void ReRoll(int durationSeconds);
};

//Handles spawns that should only be in
class UniqueSpawnManager
{
public:
	UniqueSpawnManager();
	~UniqueSpawnManager();
	void Clear(void);

	std::map<std::string, UniqueSpawnEntry> mEntryList;   //Maps spawn package names to their entries.
	typedef std::map<std::string, UniqueSpawnEntry>::iterator ITERATOR;

	bool TestSpawn(const char *spawnPackageName, int maxSpawnPoints, int callPropID);
	void ReRoll(const char *spawnPackageName, int durationSeconds);
};

extern SpawnPackageManager g_SpawnPackageManager;

#endif //#ifndef CREATURESPAWNER2_H
