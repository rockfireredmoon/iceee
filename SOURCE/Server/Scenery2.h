#ifndef SCENERY2_H
#define SCENERY2_H

#include <vector>
#include <map>
#include <list>
#include "Packet.h"
#include "Components.h"
#include "CreatureSpawner2.h"
#include "Entities.h"
#include "json/json.h"

static std::string KEYPREFIX_GROVE = "Grove";
static std::string KEYPREFIX_SCENERY = "Scenery";
static std::string ID_NEXT_SCENERY = "NextSceneryID";
static std::string KEYPREFIX_SCENERY_OBJECT = "SceneryObject";


/*  
      [SceneryManager]     <<< scenery.list <<<    [SimulatorThread]
	        |                                                   ^
       (Has Zone?)                                              ^
     no/       \yes                    /no <Load!>        [Generate Packets]
  <Load!> >>>  [Zone]  >>>  (Has Page?)         |               ^
                                       \yes [Page]  >>> [Enum Props]


*/


enum SceneryEffectType {
	PARTICLE_EFFECT = 1,
	ASSET_UPDATE = 2,
	TRANSFORMATION = 3
};

/* Effects can be attached to scenery items by scripts. This
 * happens at the Instance level, each piece of scenery having
 * a list of scenery effects attached to them
 */
class SceneryEffect
{
public:
	int tag;
	SceneryEffectType type;
	int propID;
	float scale;
	float offsetX;
	float offsetY;
	float offsetZ;
	const char *effect;

	SceneryEffect() { type = PARTICLE_EFFECT, tag = 0; propID = 0; scale = 1.0; offsetX = 0; offsetY = 0 ; offsetZ = 0 ; effect = ""; }
};

struct GlobalSceneryVars
{
	unsigned long LastSave;
	int PendingItems;

	GlobalSceneryVars()
	{
		LastSave = 0;
		PendingItems = 0;
	}
};

namespace ActiveLocation
{
	typedef std::vector<int> CONTAINER;
	bool IsLocationInUse(const CONTAINER& source);
}



//Used as a key for storing and retrieving pages from a zone.
struct SceneryPageKey
{
	int x;
	int y;
	SceneryPageKey() { x = 0; y = 0; }
	SceneryPageKey(int setX, int setY) { x = setX; y = setY; }
	bool Compare(const SceneryPageKey& other) const
	{
		return ((x == other.x) && (y == other.y));
	}
	bool operator <(const SceneryPageKey& other) const
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
};

//Not the same as the SceneObject class in the client, this is just going to be specific to props.
class SceneryObject: public AbstractEntity {
public:
	SceneryObject();
	SceneryObject(const SceneryObject &ob);
	~SceneryObject();
	void Clear(void);

	//This data is used internally by the server program
	//int PlacedBy;          //ID of the creature/player who placed this object.  If an object is being edited, this is a simulator index.

	//The following data is used in the client
	int ID;

	SceneryPageKey Key;
	int Zone;
	std::string Asset;   //Asset file
	std::string Name;    //Arbitrary object name

	float LocationX;
	float LocationY;
	float LocationZ;

	float QuatX;
	float QuatY;
	float QuatZ;
	float QuatW;

	float ScaleX;
	float ScaleY;
	float ScaleZ;

	int Flags;

	int Layer;
	int patrolSpeed;
	std::string patrolEvent;

	bool hasExtraData;
	CreatureSpawnDef extraData;

	static const int LINK_TYPE_LOYALTY = 0;   //Purple link type.  Not sure what the original purpose is.  Used in this server for linked mobs.
	static const int LINK_TYPE_PATH = 1;      //Blue link type.  Most often used to link path nodes.

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	int SetPosition(const std::string &buffer);
	int SetQ(const std::string &buffer);
	int SetS(const std::string &buffer);
	void copyFrom(const SceneryObject *source);
	bool CreateExtraData(void);
	bool IsExtendedProperty(const std::string &propertyName);
	bool SetExtendedProperty(const std::string &propertyName, const std::string &propertyValue);
	void WriteToStream(FILE *file) const;
	void AddLink(int PropID, int type);
	void RemoveLink(int PropID);
	bool HasLinks(int linkType);
	void EnumLinks(int linkType, std::vector<int> &output);
	bool IsSpawnPoint(void);
	std::string GetClusterKey();

	const char *GetSpawnPackageName(void);

	std::string GetAssetName();
	bool ExtractATS(std::string& outputStr) const;

	void WriteToJSON(Json::Value &value);
	void ReadFromJSON(Json::Value &value);
};

//Contains all scenery in a page, subdivided into props.
class SceneryPage: public AbstractEntity {
	friend class SimulatorThread; //For debugging purposes!
public:
	typedef std::map<int, SceneryObject> SCENERY_MAP;  //Map PropID to its SceneryObject data.
	typedef SCENERY_MAP::iterator SCENERY_IT;

	SceneryPage();
	~SceneryPage();
	void Destroy(void);

	SCENERY_MAP mSceneryList;
	
	int mTileX;       //X coordinate for this tile
	int mTileY;       //Y coordinate for this tile
	int mZone;        //Zone ID that this tile belongs to.
	int mPendingChanges;  //Number of pending changes within this tile, used for autosave checks.
	bool mHasSourceFile;  //If true, this page was loaded from an existing file, or was successfully saved to a file.

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	SceneryObject* AddProp(const SceneryObject& prop, bool notifyPendingChange);
	bool DeleteProp(int propID);
	void LoadScenery(void);
	void CheckAutosave(int& debugPagesSaved, int& debugPropsSaved);
	std::string GetFileName();
	std::string GetFolderName();
	SceneryObject *GetPropPtr(int propID);
	void NotifyAccess(bool notifyPendingChange);
	bool IsTileExpired(void);

	bool IsGroveZone();
	bool IsClusteredZone();
	void RemoveFromCluster();
	void LoadSceneryFromCluster();
	void LoadSceneryFromFile(std::string fileName);

private:
	unsigned long mLastAccessTime;
	void RemoveFile(std::string fileName);
	bool SaveFile(std::string fileName);
	bool SaveToCluster();
};

//Contains all scenery in a zone, subdivided into pages.
class SceneryZone
{
public:
	typedef std::map<SceneryPageKey, SceneryPage> PAGEMAP;
	PAGEMAP mPages;      //Pages hold the prop objects themselves.

	static const int DEFAULT_PAGE_SIZE = 1920;
	int mZone;
	int mPageSize;

	SceneryZone();
	~SceneryZone();
	void Destroy(void);

	SceneryObject* AddProp(const SceneryObject& prop, bool notifyPendingChange);  //Add a prop, automatically determining the page from the prop's coordinates.  Create a new page for it, if the page doesn't not exist.
	SceneryObject* ReplaceProp(const SceneryObject& prop);
	void DeleteProp(int propID);
	bool UpdateLink(int propID1, int propID2, int type);
	SceneryPage* GetOrCreatePage(const SceneryPageKey& key);
	SceneryObject* GetPropPtr(int propID, SceneryPage** foundPage);

	void CheckAutosave(int& debugPagesSaved, int& debugPropsSaved);

	void RemoveInactiveTiles(const ActiveLocation::CONTAINER& activeList);
	size_t GetTileCount(void);

private:
	void SetPageKeyFromProp(SceneryPageKey& pageKey, const SceneryObject& prop);
	SceneryPage* LoadPage(const SceneryPageKey& key);
};

//Page requests are stored in a list so they can be somewhat asynchronously distributed to
//the client socket.
struct SceneryPageRequest
{
	int socket;    //Socket to send the scenery.list response query to.
	int queryID;   //Query ID to use when building the response.
	int zone;      //Zone to fetch scenery from.
	int x;         //Tile coordinate to fetch scenery from.
	int y;         //Tile coordinate to fetch scenery from.
	bool skipQuery;  //Don't compile a query response.
	std::list<int> excludedProps; //A list of prop IDs that should be excluded
};

//The root container for all scenery objects, subdivided into zones.
class SceneryManager
{
public:
	typedef std::map<int, SceneryZone> CONTAINER;
	typedef std::map<int, SceneryZone>::iterator ITERATOR;
	CONTAINER mZones;

	SceneryZone* FindZone(int zoneID);
	SceneryZone* GetZone(int zoneID);
	SceneryZone* GetOrCreateZone(int zoneID);
	SceneryPage* GetOrCreatePage(int zoneID, int sceneryPageX, int sceneryPageY);

	SceneryManager();
	~SceneryManager();
	void Destroy(void);

	void CheckAutosave(bool force);

	void GetThread(const char *request);
	void ReleaseThread(void);

	bool ValidATSEntry(const std::string& atsName);
	bool VerifyATS(SceneryObject& prop);
	SceneryObject* GlobalGetPropPtr(int zoneID, int propID, SceneryPage** foundPage);
	SceneryObject* AddProp(int zoneID, const SceneryObject& prop);
	SceneryObject* ReplaceProp(int zoneID, const SceneryObject& prop);
	bool DeleteZone(int id);
	void DeleteProp(int zoneID, int propID);
	bool UpdateLink(int zoneID, int propID1, int propID2, int type);
	void NotifyChangedProp(int zoneID, int propID);
	
	void AddPageRequest(int socket, int queryID, int zone, int x, int y, bool skipQuery, std::list<int> excludedProps);

	bool IsGarbageCheckReady(void);
	void TransferActiveLocations(const ActiveLocation::CONTAINER& source);

	void EnumPropsInRange(int zoneID, int posX, int posZ, int radius, std::vector<SceneryObject*>& searchResults);
	
	// Utilities
	static int WriteAttachParticles(char *outbuf, const char *itemDefName, char roll, const char *bidder);
	static int WriteDetachParticles(char *outbuf, const char *lootTag, const char *originalTag, const char *winner, int creatureId, int slotIndex);

	//Thread delegation for loading scenery.
	static void ThreadProc(SceneryManager *object);
	void ThreadMain(void);
	void LaunchThread(void);
	void ShutdownThread(void);
	bool bThreadActive;

	static const unsigned int GARBAGE_CHECK_EXPIRE_TIME = 3600000;
	static const unsigned int GARBAGE_CHECK_SCAN_DELAY = 300000;
	static const unsigned int GARBAGE_CHECK_TILE_RANGE = 10;

private:
	Platform_CriticalSection cs;
	unsigned long mNextAutosaveTime;
	std::vector<SceneryPageRequest> mPendingPageRequest;
	std::vector<SceneryPageRequest> mImmediatePageRequest;

	unsigned long mNextGarbageCheckTime;
	ActiveLocation::CONTAINER mActiveLocations;

	char prepBuf[4096];

	void TransferPageRequests(void);
	void ProcessPageRequests(void);
	void SendPageRequest(const SceneryPageRequest& request, std::list<PacketManager::PACKET_PAIR>& outgoingPackets);
	void RunGarbageCheck(void);
};

extern SceneryManager g_SceneryManager;
extern GlobalSceneryVars g_SceneryVars;

int PrepExt_UpdateScenery(char *buffer, SceneryObject *so);

#endif //#ifndef SCENERY2_H
