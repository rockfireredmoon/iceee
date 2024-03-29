#ifndef ZONEDEF_H
#define ZONEDEF_H


#include "Components.h"
#include "Audit.h"
#include "Account.h"  //for ChangeData
#include "CommonTypes.h"
#include "Report.h"
#include "Guilds.h"
#include "Entities.h"

#include "json/json.h"

#include <string>
#include <list>
#include <map>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

static string KEYPREFIX_ZONE_DEF_INFO = "ZoneDefInfo";
static string LISTPREFIX_GROVE_NAME_TO_ZONE_ID = "GroveNameToZoneID";
static string KEYPREFIX_WARP_NAME_TO_ZONE_ID = "WarpNameToZoneID";
static string LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID = "AccountIDToZoneID";
static string ID_NEXT_ZONE_ID = "NextZoneID";
static string ID_NEXT_WORLD_ZONE_ID = "NextWorldZoneID";

class DropRateProfile;

class ZoneEditPermission
{
public:
	int mEditType;  //Corresponds to EditType enum below.
	int mID;
	string mCharacterName;
	int mX1;
	int mY1;
	int mX2;
	int mY2;
	ZoneEditPermission();
	void Clear(void);
	void WriteEntity(AbstractEntityWriter *writer);
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
	int GetTypeIDByName(const string &name);
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

	void ReadFromJSON(Json::Value &value) {
		x = value["x"].asInt();
		y = value["y"].asInt();
	}

	void WriteToJSON(Json::Value &value) {
		value["x"] = x;
		value["y"] = y;
	}
};


struct WeatherKey
{
	int instance;
	string mapName;
	WeatherKey(const WeatherKey &other) { instance = other.instance; mapName = other.mapName; };
	WeatherKey() { instance = 0; mapName = ""; }
	WeatherKey(int pInstance, int pMapName) { instance = pInstance; mapName = pMapName; }
	bool Compare(const WeatherKey& other) const
	{
		return ((instance == other.instance) && (mapName.compare(other.mapName) == 0));
	}
	bool operator <(const WeatherKey& other) const
	{
		if(instance < other.instance)
			return true;
		else if(instance == other.instance)
			return mapName.compare(other.mapName) < 0;
		return false;
	}
	bool operator >(const WeatherKey& other) const
	{
		if(instance > other.instance)
			return true;
		else if(instance == other.instance)
			return mapName.compare(other.mapName) > 0;
		return false;
	}
};


class WeatherDef
{
public:
	string mMapName;
	string mTimeOfDay;
	string mUse; // an alternative definition to use for this region
	unsigned long mFineMin; // minimum number of seconds the weather is fine for (zero means never fine)
	unsigned long mFineMax; // maximum number of seconds the weather is fine for
	int mLightChance; //  chance (out of 100) that the new weather will be light
	int mMediumChance; //  chance (out of 100) that the new weather will be medium
	int mHeavyChance; //  chance (out of 100) that the new weather will be heavy
	unsigned long mWeatherMin; // minimum number of seconds the weather will last
	unsigned long mWeatherMax; // maximum number of seconds the weather will last
	vector<string> mWeatherTypes;
	int mThunderChance; // chance that there will be thunder with the new weather
	unsigned long mThunderGapMin; // minimum number of seconds between thunder
	unsigned long mThunderGapMax; // maximum number of seconds between thunder
	int mEscalateChance; // chance (out of 100) that the weather will escalate (and de-escalate)

	WeatherDef(const WeatherDef &other);
	WeatherDef();
	void CopyFrom(const WeatherDef& other);
	void SetDefaults(void);
	void Clear(void);
};

class WeatherState
{
public:

	enum WeatherWeight
	{
		LIGHT = 0,
		MEDIUM,
		HEAVY,
		MAX_WEIGHT
	};

	enum WeatherEscalate
	{
		ONE_OFF = 0,
		ESCALATING,
		DEESCALATING
	};

	WeatherState(int instanceId, WeatherDef &def);
	~WeatherState();
	vector<string> mMapNames; //all the map names this state is for
	WeatherDef mDefinition; // the weather definition this state was derived from
	int mInstanceId; // the instance the weather applies to
	unsigned long mNextStateChange; // server time when the next state change occurs
	string mWeatherType; // the type of weather chosen for this activation
	int mWeatherWeight; // whether currently light, medium or heavy
	int mEscalateState; // the current state of escalation, 0 - dont escalate, 1 - escalating, 2 - de-escalating
	bool mThunder; // whether or not thunder will occur
	unsigned long mNextThunder; // -1, thunder won't occur, otherwise server time when it next occurs
	void RunCycle(ActiveInstance *instance); // run the cycle for the active instance
	void SendWeatherUpdate(ActiveInstance *instance, bool sendToCluster); // send the weather update message to everyone in this instance/area
	void SendThunder(ActiveInstance *instance, bool sendToCluser); // send the thunder message to everyone in this instance/area
	bool PickNewWeather();
	void StopWeather();

private:
	void RollThunder(); // send the thunder message to everyone in this instance/area
};

class ZoneDefInfo: public AbstractEntity {
public:
	// Values used in the old zone system.  These parameters were derived
	// from the Sparkplay audit logs.
	int mID;                       //Internal ID.  Must be unique to all other ZoneDefs.
	int mAccountID;                //Matches the grove to an account ID
	string mDesc;
	string mName;             //Client: Name of the Zone [Minimap: Name (Shard)]
	string mTerrainConfig;    //Client: the terrain configuration file.
	string mEnvironmentType;  //Client: Environment (sky effect)
	string mMapName;          //Client: The image to use as the background map.
	string mRegions;          //Client: Unknown, the color map for region boundaries?

	// These values were taken from the instance.
	string mGroveName;        //For groves, the special grove name that is provided when creating an account, used to help trace a grove back to its owner.
	string mWarpName;         //Internal name used for on-demand warping.
	string mTimeOfDay;		   //Start time of day (for when using TOD, but not cycling)
	int DefX;                 //Default X coordinate when entering the region.
	int DefY;                 //Default Y coordinate when entering the region.
	int DefZ;                 //Default Z coordinate when entering the region.
	int mPageSize;            //Page size for scenery.
	int mMode;				  //The default mode for the zone. Current PVP or PVE.
	int mReturnZone;          //If an instance no longer exists when a character logs in, place the character into this zone instead.
	bool mPersist;           //If true, an instantiated zone remains resident in memory.
	bool mInstance;          //If true, this zone is an instance, and spawns will be limited.
	bool mGrove;             //If true, this is a user grove and should be saved to the custom groves file.
	bool mArena;             //If true, this zone is an arena and has custom rulesets attached to it.
	bool mGuildHall;		 //If true, this zone is a guild hall and has custom rulesets attached to it
	bool mEnvironmentCycle;  //If true, environment time of day cycling is enabled.
	bool mAudit;             //If true, scenery edits are forced to be audited (by default, groves are not but normal gameplay zones are).
	bool mStatic;			 //If true, this is a read-only zone defined in static data. This is deprecated, all zones should place ZoneDef.txt in each Instance directory
	int mMaxAggroRange;      //This is a forced limit to the range that mobs may aggro.  Intended for places like the Rotted Maze
	int mMaxLeashRange;      //Maximum leash range.  Even if the spawn point is given a higher leash range, it will never be higher than this value, if set.
	int mMinLevel;			 //Minimum level that characters must be to enter this zone
	int mMaxLevel;			 //Maximum level that characters must be to enter this zone
	int mClan;				 //If non-zero, this is a clan grove for the clan with that ID

	int mPlayerFilterType;    //If nonzero, filter players according to type.
	vector<int> mPlayerFilterID;  //Creature Def IDs of the players to filter.

	map<EnvironmentTileKey, string> mTileEnvironment; // Use a specific environment for certain tiles (the key is a string "<x>,<y>")

	static const int DEFAULT_PAGESIZE = 1920;
	static const int DEFAULT_MAXAGGRORANGE = 1920;  //This value doesn't matter if it's greater than the range of the spawn system's maximum aggro range.  Only if it's lower.
	static const int DEFAULT_MAXLEASHRANGE = 500;
	static const int DEFAULT_MAXLEASHRANGEINSTANCE = 375;  //Automatically limit leash range for instances.

	static const int FILTER_PLAYER_NONE      = 0;   //Allow all players.
	static const int FILTER_PLAYER_WHITELIST = 1;   //Allow the players on the filter list, and only them.
	static const int FILTER_PLAYER_BLACKLIST = 2;   //Block the players on the filter list.


	vector<ZoneEditPermission> mEditPermissions;

	unsigned long mNextAuditSave;
	ZoneAudit mZoneAudit;

	ZoneDefInfo();
	~ZoneDefInfo();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	int GetNextPropID();
	void Clear(void);
	void CopyFrom(const ZoneDefInfo& other);
	void PopulateFrom(const ZoneDefInfo& other);
	void SetDefaults(void);
	bool IsPlayerGrove(void);
	bool IsGuildHall(void);
	bool IsPVPArena(void);
	bool IsFreeTravel(void);
	bool IsDungeon(void);
	bool IsOverworld(void);
	bool IsMobScalable(void);
	bool HasDeathPenalty(void);
	bool CanPlayerWarp(int CreatureDefID, int AccountID);
	void AddPlayerFilterID(int CreatureDefID, bool loadStage = false);
	void RemovePlayerFilter(int CreatureDefID);
	void ClearPlayerFilter(void);
	bool HasPlayerFilterID(int CreatureDefID);
	bool HasEditPermission(int accountID, int characterDefID, const char *characterName, float x, float z);
	void UpdateGrovePermission(STRINGLIST &params);

	void ChangeDefaultLocation(int newX, int newY, int newZ);
	void ChangeName(const char *newName);
	void ChangeEnvironment(const char *newEnvironment);
	void ChangeEnvironmentUsage(void);
	bool QualifyDelete(void);
	string GetTileEnvironment(int x, int y);
	string GetDropRateProfile();
	string GetTimeOfDay();
	void SetDropRateProfile(string profile);

	bool AllowSceneryAudits(void);
	void AuditScenery(const char *username, int zone, const SceneryObject *sceneryObject, int opType);
	void AutosaveAudits(bool force);

	void WriteToJSON(Json::Value &value);
	void ReadFromJSON(Json::Value &value);

private:
	string mDropRateProfile;
	void CreateDefaultGrovePermission(void);
};

struct ZoneIndexEntry
{
	int mID;
	int mAccountID;
	string mWarpName;
	string mGroveName;
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
	map<int, ZoneDefInfo> mZoneList;
	typedef map<int, ZoneDefInfo>::iterator ZONEDEF_ITERATOR;

	typedef map<int, ZoneIndexEntry>::iterator ZONEINDEX_ITERATOR;

	void LoadData(void);

	ZoneDefInfo* GetPointerByID(int ID);
	ZoneDefInfo* GetPointerByPartialWarpName(const string &name);
	ZoneDefInfo* GetPointerByExactWarpName(const char *name);
	ZoneDefInfo* GetPointerByGroveName(const char *name);

	string GetNextGroveName(string groveName);

	static const int GROVE_ZONE_ID_INCREMENT = 8;
	static const int GROVE_ZONE_ID_DEFAULT = 5000;
	static const int WORLD_ZONE_ID_INCREMENT = 1;

	int CreateWorldZone();
	int CreateGroveZone(ZoneDefInfo &newZone);
	int UpdateWorldZone(ZoneDefInfo &newZone);
	int CreateGrove(int accountID, const char *grovename);
	int CheckAutoSave(bool force);

	//int EnumerateGroves(string &groveName, vector<ZoneDefInfo*> &groveList);
	int EnumerateGroves(int searchAccountID, int creatureDefId, vector<string>& groveList);
	int EnumerateGroveIds(int searchAccountID, int creatureDefId, vector<int>& groveList);
	int EnumerateArenas(vector<string>& arenaList);
	void UpdateGroveAccountID(const char *groveName, int newAccountID);
	void UpdateZoneIndex(int zoneID, int accountID, const char *warpName, const char *groveName, bool allowCreate);

	void NotifyConfigurationChange(void);
	bool ZoneUnloadReady(void);
	void UnloadInactiveZones(vector<int>& activeZones);
	void RemoveZoneFromIndexes(ZoneDefInfo *def);
	bool DeleteZone(int id);
	
	void GenerateReportActive(ReportBuffer &report);
	void InsertZone(const ZoneDefInfo& newZone, bool createIndex);

	//Special case destinations for warping (such as selecting from the grove list)
	static const int HENGE_ID_CUSTOMWARP = 1;
	static const int DEFAULT_GROVE_PAGE_SIZE = 1920;
private:
	static const int ZONE_AUTOSAVE_DELAY = 120000;
	static const int ZONE_UNLOAD_DELAY = 120000;

	PlatformTime::TIME_VALUE mNextAutosaveTime;
	PlatformTime::TIME_VALUE mNextZoneUnload;
	ChangeData ZoneListChanges;
	Platform_CriticalSection cs;

	int GetNewGroveZoneID(void);
	int GetNewWorldZoneID(void);
	int LoadFile(string fileName);

	ZoneDefInfo * ResolveZoneDef(int ID);
	ZoneDefInfo * LoadZoneDef(int ID);
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
	typedef map<int, vector<MapBarrierPoint> > BARRIER_CONT;
	BARRIER_CONT zoneList;

	void AddEntry(int zoneID, MapBarrierPoint &data);
	bool CheckCollision(int zoneID, int &x, int &z);
	void LoadFromFile(const fs::path &filename);
	int GetLoadedCount(void);
};

class WeatherManager
{
public:
	map<WeatherKey, WeatherState*> mWeather; // all currently maintained weather
	map<string, WeatherDef> mWeatherDefinitions; // all weather definitions
	vector<WeatherState*> RegisterInstance(ActiveInstance *instance); // when an instance loads, we find all of it's weather regions (i.e. map names) and start maintaining them if there is a weather definition
	void Deregister(vector<WeatherState*> *states); // when an instance dies, we stop maintaining its weather regions (i.e. map names)
	int LoadFromFile(string filename);
	WeatherState* GetWeather(string mapName, int instanceId);
	void ZoneThunder(int zoneId, string mapName);
	void ZoneWeather(int zoneId, string mapName, string weatherType, int weight);
private:
	bool MaybeAddWeatherDef(int instanceID, string actualMapName, vector<WeatherState*> &m);
};

class EnvironmentCycle
{
public:
	string mName;
	unsigned long mStart;
	unsigned long mEnd;
	unsigned long GetDuration();
	unsigned long GetNext();
	bool InRange(unsigned long time);
};

class EnvironmentCycleManager
{
public:
	static const int DEFAULT_CYCLE_TIME = 3600000;  //1 hour
	static const int MILLISECOND_PER_SECOND = 1000;  //60000;

	string mConfig;
	vector<EnvironmentCycle> mCycles;

	EnvironmentCycleManager();
	~EnvironmentCycleManager();
	void RescheduleUpdate();
	void Init();
	void ApplyConfig(const string str);
	EnvironmentCycle GetCurrentCycle(void);
private:
	int mChangeTaskID;
};

class GroveTemplate
{
public:
	string mShortName;    //Used for internal lookups by name.
	string mFileName;     //Not actually required.
	string mTerrainCfg;   //The ZoneDefInfo::mTerrainConfig property.
	string mEnvType;      //The ZoneDefInfo::mEnvironmentType property.
	string mMapName;      //The ZoneDefInfo::mMapName property.
	string mRegionsPng;   //The ZoneDefInfo::mRegions property.
	int mTileX1;               //Scenery tile coordinate (distinct from terrain tiles, scenery tiles are 1920 units wide).
	int mTileY1;
	int mTileX2;
	int mTileY2;
	int mDefX;                 //Default character placement coordinates when warping to this zone.
	int mDefY;
	int mDefZ;

	GroveTemplate();
	void Clear();
	bool HasProps() const;
	void GetProps(vector<SceneryObject> &objects) const;
};

class GroveTemplateManager
{
public:
	GroveTemplateManager();
	const GroveTemplate* GetTemplateByShortName(const char *name);
	const GroveTemplate* GetTemplateByTerrainCfg(const char *cfg);
	void LoadData(void);

private:
	map<string, GroveTemplate> mTemplateEntries;    //This holds the actual template entries.
	map<string, const GroveTemplate*> mTerrainMap;  //This maps GroveTemplate::mTerrainCfg values to their template for faster internal lookups.
	void ResolveTerrainMap(void);
	void LoadFile(const fs::path &filename);
};

extern ZoneDefManager g_ZoneDefManager;
extern ZoneBarrierManager g_ZoneBarrierManager;
extern EnvironmentCycleManager g_EnvironmentCycleManager;
extern GroveTemplateManager g_GroveTemplateManager;
extern WeatherManager g_WeatherManager;

int PrepExt_SendEnvironmentUpdateMsg(char *buffer, ActiveInstance *instance, const char *zoneIDString, ZoneDefInfo *zoneDef, int x, int z, int mask);

#endif  //ZONEDEF_H
