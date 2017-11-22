#include <vector>
#include <string>

const int SANCTUARY_ELEVATION_ADDITIVE = 7;  //Players stand higher than the sanctuary prop by this much. Used when warping to sanctuary points.
const int SANCTUARY_PROXIMITY_USE = 150;     //Maximum map units away from a sanctuary that it may be used for zone warps.

struct WorldCoord
{
	float x;
	float y;
	float z;
	std::string descName;         //Descriptive name
	std::string serverMapName;    //Internal map name used by the server.  Corresponds to "MapLocations.txt" location names.
	WorldCoord() { Clear(); };
	WorldCoord(float nx, float ny, float nz, int zn) { x = nx; y = ny; z = nz;  }
	void Clear(void) { x = 0.0F; y = 0.0F; z = 0.0f; descName.clear(); serverMapName.clear(); };
};

class ZoneMarkerData
{
public:
	ZoneMarkerData();
	~ZoneMarkerData();

	int zoneID;
	std::vector<WorldCoord> sanctuary;
	void Clear(void);

	WorldCoord* GetSanctuaryInRange(int x, int z, int range);
	WorldCoord* GetNearestSanctuaryInZone(int x, int z);
	WorldCoord* GetNearestRegionSanctuaryInZone(const char *regionName, int x, int z);
};

class ZoneMarkerDataManager
{
public:
	ZoneMarkerDataManager();
	~ZoneMarkerDataManager();

	std::vector<ZoneMarkerData> zoneList;
	void LoadFile(const char *filename);
	void AddZoneMarkers(ZoneMarkerData &details);
	ZoneMarkerData* GetPtrByZoneID(int zoneID);
	WorldCoord* GetSanctuaryInRange(int zoneID, int x, int z, int range);
	WorldCoord* GetNearestSanctuaryInZone(int zoneID, int x, int z);
	WorldCoord* GetNearestRegionSanctuaryInZone(const char *regionName, int zoneID, int x, int z);
};

extern ZoneMarkerDataManager g_ZoneMarkerDataManager;
