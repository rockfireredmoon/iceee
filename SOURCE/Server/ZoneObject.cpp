#include "ZoneObject.h"
#include "FileReader.h"
#include "StringList.h"
#include <string.h>
#include <math.h>
#include "util/Log.h"

ZoneMarkerDataManager g_ZoneMarkerDataManager;

ZoneMarkerData :: ZoneMarkerData()
{
	zoneID = 0;
}

ZoneMarkerData :: ~ZoneMarkerData()
{
	sanctuary.clear();
}

void ZoneMarkerData :: Clear(void)
{
	zoneID = 0;
	sanctuary.clear();
}

WorldCoord* ZoneMarkerData :: GetSanctuaryInRange(int x, int z, int range)
{
	for(size_t i = 0; i < sanctuary.size(); i++)
	{
		if(abs(sanctuary[i].x - x) > range)
			continue;
		if(abs(sanctuary[i].z - z) > range)
			continue;
		return &sanctuary[i];
	}
	return NULL;
}

WorldCoord* ZoneMarkerData :: GetNearestSanctuaryInZone(int x, int z)
{
	long closest = -1;
	WorldCoord *found = NULL;
	for(size_t i = 0; i < sanctuary.size(); i++)
	{
		//Some of the distances may be quite large, so scale their integer size down.
		int xlen = static_cast<int>(abs(sanctuary[i].x - x) / 100.0F);
		int zlen = static_cast<int>(abs(sanctuary[i].z - z) / 100.0F);
		long dist = static_cast<long>(sqrt((double)(xlen * xlen) + (zlen * zlen)));
		if(closest == -1 || dist < closest)
		{
			closest = dist;
			found = &sanctuary[i];
		}
	}
	return found;
}

WorldCoord* ZoneMarkerData :: GetNearestRegionSanctuaryInZone(const char *regionName, int x, int z)
{
	//Important NULL check, otherwise could crash on find().
	if(regionName == NULL)
		return GetNearestSanctuaryInZone(x, z);

	long closest = -1;
	WorldCoord *found = NULL;
	for(size_t i = 0; i < sanctuary.size(); i++)
	{
		if(sanctuary[i].serverMapName.find(regionName) == std::string::npos)
			continue;
	
		//Some of the distances may be quite large, so scale their integer size down to prevent numerical overflows.
		int xlen = static_cast<int>(abs(sanctuary[i].x - x) / 100.0F);
		int zlen = static_cast<int>(abs(sanctuary[i].z - z) / 100.0F);
		long dist = static_cast<long>(sqrt((double)(xlen * xlen) + (zlen * zlen)));
		if(closest == -1 || dist < closest)
		{
			closest = dist;
			found = &sanctuary[i];
		}
	}

	//Fall back to full zone search if no sanctuary was found in the region.
	if(found == NULL)
		found = GetNearestSanctuaryInZone(x, z);
	return found;
}

ZoneMarkerDataManager :: ZoneMarkerDataManager()
{
}

ZoneMarkerDataManager :: ~ZoneMarkerDataManager()
{
	zoneList.clear();
}

void ZoneMarkerDataManager :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.data->error("Could not open file [%v]", filename);
		return;
	}

	ZoneMarkerData entryData;
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(entryData.zoneID != 0)
					AddZoneMarkers(entryData);
				entryData.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "ZoneID") == 0)
				entryData.zoneID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Sanctuary") == 0)
			{
				WorldCoord newEntry;
				newEntry.x = lfr.BlockToFloatC(1);
				newEntry.y = lfr.BlockToFloatC(2);
				newEntry.z = lfr.BlockToFloatC(3);
				if(r > 4)
					newEntry.descName = lfr.BlockToStringC(4, 0);
				if(r > 5)
					newEntry.serverMapName = lfr.BlockToStringC(5, 0);
				//entryData.sanctuary.push_back(WorldCoord((float)lfr.BlockToDblC(1), (float)lfr.BlockToDblC(2), (float)lfr.BlockToDblC(3)));
				entryData.sanctuary.push_back(newEntry);
			}
		}
	}
	if(entryData.zoneID != 0)
		AddZoneMarkers(entryData);
	lfr.CloseCurrent();
}

void ZoneMarkerDataManager :: AddZoneMarkers(ZoneMarkerData &details)
{
	zoneList.push_back(details);
}

ZoneMarkerData* ZoneMarkerDataManager :: GetPtrByZoneID(int zoneID)
{
	for(size_t i = 0; i < zoneList.size(); i++)
		if(zoneList[i].zoneID == zoneID)
			return &zoneList[i];
	return NULL;
}

WorldCoord* ZoneMarkerDataManager :: GetSanctuaryInRange(int zoneID, int x, int z, int range)
{
	ZoneMarkerData* zone = GetPtrByZoneID(zoneID);
	if(zone == NULL)
		return NULL;

	return zone->GetSanctuaryInRange(x, z, range);
}

WorldCoord* ZoneMarkerDataManager :: GetNearestSanctuaryInZone(int zoneID, int x, int z)
{
	ZoneMarkerData* zone = GetPtrByZoneID(zoneID);
	if(zone == NULL)
		return NULL;

	return zone->GetNearestSanctuaryInZone(x, z);
}

//This function attempts to find the closest sanctuary within a particular defined region, only
//scanning outside the region if none are found.  This is to prevent players from suddenly finding
//themselves in a different region, possibly requiring a long walk around region boundaries (mountains)
WorldCoord* ZoneMarkerDataManager :: GetNearestRegionSanctuaryInZone(const char *regionName, int zoneID, int x, int z)
{
	ZoneMarkerData* zone = GetPtrByZoneID(zoneID);
	if(zone == NULL)
		return NULL;

	return zone->GetNearestRegionSanctuaryInZone(regionName, x, z);
}

