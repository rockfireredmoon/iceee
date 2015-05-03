#ifndef ZONEDEF_H
#define ZONEDEF_H

#include <string>
#include <list>

#include "Components.h"
#include "Audit.h"
#include "Account.h"  //for ChangeData
#include "CommonTypes.h"
#include "Report.h"
#include "Guilds.h"

class DropRateProfile;

class ZoneEditPermission
{
public:
	int mEditType;  //Corresponds to EditType enum below.
	int mID;
	std::string mCharacterName;
	int mX1;
	int mY1;
	int mX2;
	int mY2;
	ZoneEditPermission();
	void Clear(void);
	void SaveToStream(FILE *output);
	void LoadFromConfig(const char *configStr);
	bool IsValid(void);
	bool HasEditPermission(int accountID, int characterDefID, const char *characterName, float x, float z, int pageSize);

	enum EditType
	{
		PERMISSION_TYPE_NONE = 0,
		PERMISSION_TYPE_CDEF,
		PERMISSION_TYPE_ACCOUNT,
		PERMISSION_TYPE_PLAYER
	};

private:
	int GetTypeIDByName(const std::string &name);
	const char *GetTypeNameByID(int ID);
};


struct EnvironmentTileKey
{
	int x;
	int y;
	EnvironmentTileKey() { x = 0; y = 0; }
	EnvironmentTileKey(int setX, int setY) { x = setX; y = setY; }
	bool Compare(const EnvironmentTileKey& other) const
	{
		return ((x == other.x) && (y == other.y));
	}
	bool operator <(const EnvironmentTileKey& other) const
	{
		if(x < other.x)
			return true;
		else if(x == other.x)
			return (y < other.y);
		return false;
	}
};

class ZoneDefInfo
{
public:
	// Values used in the old zone system.  These parameters were derived
	// from the Sparkplay audit logs.
	int mID;                       //Internal ID.  Must be unique to all other ZoneDefs.
	int mAccountID;                //Matches the grove to an account ID
	std::string mDesc;
	std::string mName;             //Client: Name of the Zone [Minimap: Name (Shard)]
	std::string mTerrainConfig;    //Client: the terrain configuration file.
	std::string mEnvironmentType;  //Client: Environment (sky effect)
	std::string mMapName;          //Client: The image to use as the background map.
	std::string mRegions;          //Client: Unknown, the color map for region boundaries?

	// These values were taken from the instance.
	std::string mShardName;        //Prefix of the shard name.
	std::string mGroveName;        //For groves, the special grove name that is provided when creating an account, used to help trace a grove back to its owner.
	std::string mWarpName;         //Internal name used for on-demand warping.
	int DefX;                 //Default X coordinate when entering the region.
	int DefY;                 //Default Y coordinate when entering the region.
	int DefZ;                 //Default Z coordinate when entering the region.
	int mPageSize;            //Page size for scenery.
	int mReturnZone;          //If an instance no longer exists when a character logs in, place the character into this zone instead.
	bool mPersist;           //If true, an instantiated zone remains resident in memory.
	bool mInstance;          //If true, this zone is an instance, and spawns will be limited.
	bool mGrove;             //If true, this is a user grove and should be saved to the custom groves file.
	bool mArena;             //If true, this zone is an arena and has custom rulesets attached to it.
	bool mGuildHall;		 //If true, this zone is a guild hall and has custom rulesets attached to it
	bool mEnvironmentCycle;  //If true, environment time of day cycling is enabled.
	bool mAudit;             //If true, scenery edits are forced to be audited (by default, groves are not but normal gameplay zones are).
	int mMaxAggroRange;      //This is a forced limit to the range that mobs may aggro.  Intended for places like the Rotted Maze
	int mMaxLeashRange;      //Maximum leash range.  Even if the spawn point is given a higher leash range, it will never be higher than this value, if set.

	int mPlayerFilterType;    //If nonzero, filter players according to type.
	std::vector<int> mPlayerFilterID;  //Creature Def IDs of the players to filter.

	std::string mDropRateProfile;
	std::map<EnvironmentTileKey, string> mTileEnvironment; // Use a specific environment for certain tiles (the key is a string "<x>,<y>")
	
	int PendingChanges;  //Used internally to track whether this zone needs to be saved back to file.

	static const int DEFAULT_PAGESIZE = 1920;
	static const int DEFAULT_MAXAGGRORANGE = 1920;  //This value doesn't matter if it's greater than the range of the spawn system's maximum aggro range.  Only if it's lower.
	static const int DEFAULT_MAXLEASHRANGE = 500;
	static const int DEFAULT_MAXLEASHRANGEINSTANCE = 375;  //Automatically limit leash range for instances.

	static const int FILTER_PLAYER_NONE      = 0;   //Allow all players.
	static const int FILTER_PLAYER_WHITELIST = 1;   //Allow the players on the filter list, and only them.
	static const int FILTER_PLAYER_BLACKLIST = 2;   //Block the players on the filter list.

	std::vector<ZoneEditPermission> mEditPermissions;

	unsigned long mNextAuditSave;
	ZoneAudit mZoneAudit;

	ZoneDefInfo();
	~ZoneDefInfo();
	void Clear(void);
	void CopyFrom(const ZoneDefInfo& other);
	void SetDefaults(void);
	bool IsPlayerGrove(void);
	bool IsGuildHall(void);
	bool IsPVPArena(void);
	bool IsFreeTravel(void);
	bool IsDungeon(void);
	bool IsMobScalable(void);
	bool HasDeathPenalty(void);
	bool CanPlayerWarp(int CreatureDefID, int AccountID);
	void SaveToStream(FILE *output);
	void AddPlayerFilterID(int CreatureDefID, bool loadStage = false);
	void RemovePlayerFilter(int CreatureDefID);
	void ClearPlayerFilter(void);
	bool HasPlayerFilterID(int CreatureDefID);
	bool HasEditPermission(int accountID, int characterDefID, const char *characterName, float x, float z);
	void UpdateGrovePermission(STRINGLIST &params);

	const DropRateProfile& GetDropRateProfile(void);

	void ChangeDefaultLocation(int newX, int newY, int newZ);
	void ChangeShardName(const char *newName);
	void ChangeName(const char *newName);
	void ChangeEnvironment(const char *newEnvironment);
	void ChangeEnvironmentUsage(void);
	bool QualifyDelete(void);
	std::string * GetTileEnvironment(int x, int y);

	bool AllowSceneryAudits(void);
	void AuditScenery(const char *username, int zone, const SceneryObject *sceneryObject, int opType);
	void AutosaveAudits(bool force);

private:
	void CreateDefaultGrovePermission(void);
};

struct ZoneIndexEntry
{
	int mID;
	int mAccountID;
	std::string mWarpName;
	std::string mGroveName;
	ZoneIndexEntry();
	void CopyFrom(const ZoneIndexEntry& other);
};

class ZoneDefManager
{
public:
	ZoneDefManager();
	~ZoneDefManager();
	void Free(void);

	//New groves will expand the Zone Definitions, so use a list instead of vector.
	std::map<int, ZoneDefInfo> mZoneList;
	typedef std::map<int, ZoneDefInfo>::iterator ZONEDEF_ITERATOR;

	std::map<int, ZoneIndexEntry> mZoneIndex;     //Maps ZoneID to an Index;
	typedef std::map<int, ZoneIndexEntry>::iterator ZONEINDEX_ITERATOR;

	int NextZoneID;  //Should only be modified by the class, but may be explicitly retrieved and set through session saving/loading

	void LoadData(void);

	ZoneDefInfo* GetPointerByID(int ID);
	ZoneDefInfo* GetPointerByPartialWarpName(const char *name);
	ZoneDefInfo* GetPointerByExactWarpName(const char *name);
	ZoneDefInfo* GetPointerByGroveName(const char *name);

	static const int GROVE_ZONE_ID_INCREMENT = 8;
	static const int GROVE_ZONE_ID_DEFAULT = 5000;

	int CreateGrove(int accountID, const char *grovename);
	int CheckAutoSave(bool force);

	//int EnumerateGroves(std::string &groveName, std::vector<ZoneDefInfo*> &groveList);
	int EnumerateGroves(int searchAccountID, int creatureDefId, std::vector<std::string>& groveList);
	int EnumerateArenas(std::vector<std::string>& arenaList);
	void UpdateGroveAccountID(const char *groveName, int newAccountID);
	void UpdateZoneIndex(int zoneID, int accountID, const char *warpName, const char *groveName, bool allowCreate);

	void NotifyConfigurationChange(void);
	bool ZoneUnloadReady(void);
	void UnloadInactiveZones(std::vector<int>& activeZones);
	
	void GenerateReportActive(ReportBuffer &report);

	//Special case destinations for warping (such as selecting from the grove list)
	static const int HENGE_ID_CUSTOMWARP = 1;
private:
	static const int ZONE_AUTOSAVE_DELAY = 120000;
	static const int ZONE_UNLOAD_DELAY = 120000;
	static const int DEFAULT_GROVE_PAGE_SIZE = 1920;

	PlatformTime::TIME_VALUE mNextAutosaveTime;
	PlatformTime::TIME_VALUE mNextZoneUnload;
	ChangeData ZoneListChanges;
	ChangeData ZoneIndexChanges;
	Platform_CriticalSection cs;
	std::string mDataFileIndex;

	int GetNewZoneID(void);
	int LoadFile(const char *fileName);
	bool SaveIndex(void);
	void LoadIndex(void);
	void InitDefaultZoneIndex(void);

	void InsertZone(const ZoneDefInfo& newZone, bool createIndex);
	ZoneDefInfo * ResolveZoneDef(int ID);
	ZoneDefInfo * LoadZoneDef(int ID);
	void GetZoneFileName(int ID, std::string &outStr);
};

struct MapBarrierPoint
{
	int x1;
	int x2;
	int z1;
	int z2;
	MapBarrierPoint();
	void Clear(void);
	bool isValid(void);
};

class ZoneBarrierManager
{
public:
	typedef std::map<int, std::vector<MapBarrierPoint> > BARRIER_CONT;
	BARRIER_CONT zoneList;

	void AddEntry(int zoneID, MapBarrierPoint &data);
	bool CheckCollision(int zoneID, int &x, int &z);
	void LoadFromFile(const char *filename);
	int GetLoadedCount(void);
};

class EnvironmentCycleManager
{
public:
	static const int DEFAULT_CYCLE_TIME = 3600000;  //1 hour
	static const int MILLISECOND_PER_SECOND = 1000;  //60000;

	std::string mConfig;
	unsigned long mNextUpdateTime;
	std::vector<std::string> mCycleStrings;
	std::vector<int> mCycleTimes;
	size_t mCurrentCycleIndex;

	EnvironmentCycleManager();
	~EnvironmentCycleManager();
	void ApplyConfig(const char *str);
	bool HasCycleUpdated(void);
	const char *GetCurrentTimeOfDay(void);
	void EndCurrentCycle(void);
};

class GroveTemplate
{
public:
	std::string mShortName;    //Used for internal lookups by name.
	std::string mFileName;     //Not actually required.
	std::string mTerrainCfg;   //The ZoneDefInfo::mTerrainConfig property.
	std::string mEnvType;      //The ZoneDefInfo::mEnvironmentType property.
	std::string mMapName;      //The ZoneDefInfo::mMapName property.
	std::string mRegionsPng;   //The ZoneDefInfo::mRegions property.
	int mTileX1;               //Scenery tile coordinate (distinct from terrain tiles, scenery tiles are 1920 units wide).
	int mTileY1;
	int mTileX2;
	int mTileY2;
	int mDefX;                 //Default character placement coordinates when warping to this zone.
	int mDefY;
	int mDefZ;

	GroveTemplate();
	void Clear();
};

class GroveTemplateManager
{
public:
	GroveTemplateManager();
	const GroveTemplate* GetTemplateByShortName(const char *name);
	const GroveTemplate* GetTemplateByTerrainCfg(const char *cfg);
	void LoadData(void);

private:
	std::map<std::string, GroveTemplate> mTemplateEntries;    //This holds the actual template entries.
	std::map<std::string, const GroveTemplate*> mTerrainMap;  //This maps GroveTemplate::mTerrainCfg values to their template for faster internal lookups.
	void ResolveTerrainMap(void);
	void LoadFile(const char *filename);
};

extern ZoneDefManager g_ZoneDefManager;
extern ZoneBarrierManager g_ZoneBarrierManager;
extern EnvironmentCycleManager g_EnvironmentCycleManager;
extern GroveTemplateManager g_GroveTemplateManager;

#endif  //ZONEDEF_H
