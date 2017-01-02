#include "Components.h"
#include "Debug.h"

#define VERIFYCINST(x)  EMPTY_OPERATION
#define SEND_DEBUG_MSG_ALL(msg,sim)  EMPTY_OPERATION
#define SEND_DEBUG_MSG_ONE(msg,sim)  EMPTY_OPERATION

int g_NextActorID = 3000000;

//#define SEND_DEBUG_MSG_ALL(msg,sim)  size = PrepExt_SendInfoMessage(GSendBuf, msg, INFOMSG_INFO); LSendToAllSimulator(GSendBuf, size, sim);
//#define SEND_DEBUG_MSG_ONE(msg,sim)  size = PrepExt_SendInfoMessage(GSendBuf, msg, INFOMSG_INFO); LSendToOneSimulator(GSendBuf, size, sim);

extern const int SANE_DISTANCE = 10000;        //All typical distance checks (target acquisition, movement checks, etc) have no purpose including objects beyond this box range.  Using this as a maximum threshold can avoid overflows.
extern const int DISTANCE_FAILED = 999999;     //If box distance checks are not within a heuristic range, abort with this range

extern const int FACTION_PLAYERFRIENDLY = 0;
extern const int FACTION_PLAYERHOSTILE = 1;
extern const int PARTY_SHARE_DISTANCE = 1920;

const bool DEFAULT_PVPSTATE = true;

int InstanceUpdateDelay = 100;  //Milliseconds to delay between general instance updates.  10 frames per second should be fine, although AI scripts might run a bit slow.
unsigned long INSTANCE_DELETE_MAXTIME = 900000; //Maximum time a non-persistent instance is allowed to stay open before it is deleted.
unsigned long INSTANCE_DELETE_RECHECK = 60000;  //Recheck delay between scanning for empty non-persistent instances

unsigned long CREATURE_DELETE_RECHECK = 60000; //Delay between rescanning for dead creatures and deleting them from the instance.

#include <math.h>
#include <algorithm>

#include "Instance.h"

#include "FileReader.h"
#include "StringList.h"

//All needed for local message processing
#include "Globals.h"
#include "Config.h" //Needed for MOTD
#include "Character.h"
#include "ActiveCharacter.h"
#include "Simulator.h"
#include "Util.h"
#include "Creature.h"
#include "Ability2.h"
#include "AIScript.h"  //For script resolution
#include "AIScript2.h"
#include "DebugTracer.h"
#include "DirectoryAccess.h"
#include "ZoneDef.h"
#include "CreatureSpawner2.h"
#include "Interact.h"
#include "Chat.h"
#include "PartyManager.h"
#include "InstanceScale.h"
#include "ScriptCore.h"

extern char GSendBuf[32767];
extern char GAuxBuf[1024];

//ZoneDefContainer ZoneDef;
MapDefContainer MapDef;
MapLocationHandler MapLocation;
ActiveInstanceManager g_ActiveInstanceManager;

WorldMarker::WorldMarker() {
	Clear();
}

void WorldMarker::Clear() {
	Name[0] = 0;
	Comment[0] = 0;
	X = 0;
	Y = 0;
	Z = 0;
}


WorldMarkerContainer::WorldMarkerContainer()
{
	Clear();
}

WorldMarkerContainer::~WorldMarkerContainer()
{
	WorldMarkerList.clear();
}

void WorldMarkerContainer::Clear()
{
	WorldMarkerList.clear();
}

void WorldMarkerContainer::Save()
{
	g_Log.AddMessageFormat("Saving world markers to %s.", mFilename.c_str());
	FILE *output = fopen(mFilename.c_str(), "wb");
	if(output == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] Saving world markers could not open: %s", mFilename.c_str());
		return;
	}

	vector<WorldMarker>::iterator it;
	for (it = WorldMarkerList.begin(); it != WorldMarkerList.end(); ++it) {
		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "Name=%s\r\n", it->Name);
		string r = it->Comment;
		Util::ReplaceAll(r, "\r\n", "\\r\\n");
		Util::ReplaceAll(r, "\n", "\\n");
		fprintf(output, "Comment=%s\r\n", r.c_str());
		fprintf(output, "Position=%1.1f,%1.1f,%1.1f\r\n", it->X, it->Y, it->Z);
		fprintf(output, "\r\n");
	}

	fflush(output);
	fclose(output);
}

void WorldMarkerContainer::Reload()
{
	Clear();
	FileReader lfr;
	if(lfr.OpenText(mFilename.c_str()) != Err_OK)
	{
		g_Log.AddMessageFormat("[NOTICE] WorldMarker file [%s] not found.", mFilename.c_str());
		return;
	}

	WorldMarker newItem;

	lfr.CommentStyle = Comment_Semi;
	int r;
	while(lfr.FileOpen() == true)
	{
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if(r > 0)
		{
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.Name[0] != 0)
				{
					WorldMarkerList.push_back(newItem);
					newItem.Clear();
				}
			}
			else if(strcmp(lfr.SecBuffer, "NAME") == 0)
			{
				Util::SafeCopy(newItem.Name, lfr.BlockToStringC(1, 0), sizeof(newItem.Name));
			}
			else if(strcmp(lfr.SecBuffer, "POSITION") == 0)
			{
				STRINGLIST locData;
				string str = lfr.BlockToStringC(1, 0);
				Util::Split(str.c_str(), ",", locData);
				if(locData.size() < 3)
				{
					g_Log.AddMessageFormat("[WARNING] WorldMarker:%s has incomplete Position string (%s)", newItem.Name, str.c_str());
				}
				else
				{
					newItem.X = static_cast<float>(strtof(locData[0].c_str(), NULL));
					newItem.Y = static_cast<float>(strtof(locData[1].c_str(), NULL));
					newItem.Z = static_cast<float>(strtof(locData[2].c_str(), NULL));
				}
			}
			else if(strcmp(lfr.SecBuffer, "COMMENT") == 0)
			{
				string r = lfr.BlockToStringC(1, 0);
				Util::ReplaceAll(r, "\\r\\n", "\r\n");
				Util::ReplaceAll(r, "\\n", "\n");
				Util::SafeCopy(newItem.Comment, r.c_str(), sizeof(newItem.Comment));
			}
		}
	}
	lfr.CloseCurrent();
	if(newItem.Name[0] != 0)
		WorldMarkerList.push_back(newItem);
}


void WorldMarkerContainer::LoadFromFile(const char *filename)
{
	mFilename = filename;
	Reload();
}

MapDefContainer :: MapDefContainer()
{

}

MapDefContainer :: ~MapDefContainer()
{
	FreeList();
}

void MapDefContainer :: FreeList(void)
{
	mMapList.clear();
}

int MapDefContainer :: CreateMap(void)
{
	//Creates a new map definition, initializing numerical elements to zero, and
	//returning in an index to the array position of the new last element in the
	//array.

	MapDefInfo newMap;
	newMap.numPagesAcross = 0.0F;
	newMap.numPagesDown = 0.0F;
	newMap.u0 = 0;
	newMap.v0 = 0;
	newMap.u1 = 0;
	newMap.v1 = 0;
	newMap.priority = 0;
	newMap.scale_width = 0.0F;
	newMap.scale_height = 0.0F;
	mMapList.push_back(newMap);
	return mMapList.size() - 1;
}

int MapDefContainer :: SearchMap(const char *primary, int xpos, int ypos)
{
	//Search through the map definitions for the map name best matching the given
	//location.  <primary> refers to the main world to search for, such as
	//"Maps-Anglorum" or "Maps-Europe".
	//All maps with a type of "Region" are compared with the given coordinates.
	//If two maps overlap coordinates, the one with the lower priority is selected.
	//Returns an index into the array of the matching item, or returns -1 if none found.

	int fIndex = -1;
	int fPriority = 255;

	for(size_t a = 0; a < mMapList.size(); a++)
	{
		if(xpos >= mMapList[a].u0 && xpos <= mMapList[a].u1)
			if(ypos >= mMapList[a].v0 && ypos <= mMapList[a].v1)
				if(strcmp(mMapList[a].Primary.c_str(), primary) == 0)
					if(strcmp(mMapList[a].Type.c_str(), "Region") == 0)
						if(fIndex == -1 || mMapList[a].priority < fPriority)
						{
							fIndex = a;
							fPriority = mMapList[a].priority;
						}
	}

	if(fIndex == -1)
	{
		//No local maps found, search for a world map this time.
		for(size_t a = 0; a < mMapList.size(); a++)
		{
			if(xpos >= mMapList[a].u0 && xpos <= mMapList[a].u1)
				if(ypos >= mMapList[a].v0 && ypos <= mMapList[a].v1)
					if(strcmp(mMapList[a].Primary.c_str(), primary) == 0)
						if(strcmp(mMapList[a].Type.c_str(), "World") == 0)
							if(fIndex == -1 || mMapList[a].priority < fPriority)
							{
								fIndex = a;
								fPriority = mMapList[a].priority;
							}
		}
	}
	return fIndex;
}

int MapDefContainer :: GetIndexByName(const char *name, const char *type)
{
	//Attempt to find the index of a map by searching for its name and type
	for(size_t a = 0; a < mMapList.size(); a++)
		if(strcmp(mMapList[a].Name.c_str(), name) == 0)
			if(strcmp(mMapList[a].Type.c_str(), type) == 0)
				return a;

	return -1;
}

int MapDefContainer :: LoadFile(const char *fileName)
{
	FileReader lfr;
	if(lfr.OpenText(fileName) != Err_OK)
	{
		g_Log.AddMessageFormat("Error: Could not open file %s", fileName);
		return -1;
	}
	lfr.CommentStyle = Comment_Semi;

	int opIndex = -1;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			lfr.SingleBreak("=");
			lfr.BlockToStringC(0, Case_Upper);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				opIndex = CreateMap();
				if(opIndex == -1)
				{
					g_Log.AddMessageFormat("Error: Could not create a new MapDef entry.");
					lfr.CloseCurrent();
					return -1;
				}
			}
			else
			{
				if(opIndex >= 0)
				{
					if(strcmp(lfr.SecBuffer, "NAME") == 0)
						mMapList[opIndex].Name = lfr.BlockToStringC(1, 0);
					else if(strcmp(lfr.SecBuffer, "PRIMARY") == 0)
						mMapList[opIndex].Primary = lfr.BlockToStringC(1, 0);
					else if(strcmp(lfr.SecBuffer, "TYPE") == 0)
						mMapList[opIndex].Type = lfr.BlockToStringC(1, 0);
					else if(strcmp(lfr.SecBuffer, "IMAGE") == 0)
						mMapList[opIndex].image = lfr.BlockToStringC(1, 0);
					else if(strcmp(lfr.SecBuffer, "PARENTMAPIMAGE") == 0)
						mMapList[opIndex].parentMapImage = lfr.BlockToStringC(1, 0);
					else if(strcmp(lfr.SecBuffer, "NUMPAGESACROSS") == 0)
						mMapList[opIndex].numPagesAcross = (float)lfr.BlockToDblC(1);
					else if(strcmp(lfr.SecBuffer, "NUMPAGESDOWN") == 0)
						mMapList[opIndex].numPagesDown = (float)lfr.BlockToDblC(1);
					else if(strcmp(lfr.SecBuffer, "U0") == 0)
						mMapList[opIndex].u0 = lfr.BlockToIntC(1);
					else if(strcmp(lfr.SecBuffer, "V0") == 0)
						mMapList[opIndex].v0 = lfr.BlockToIntC(1);
					else if(strcmp(lfr.SecBuffer, "U1") == 0)
						mMapList[opIndex].u1 = lfr.BlockToIntC(1);
					else if(strcmp(lfr.SecBuffer, "V1") == 0)
						mMapList[opIndex].v1 = lfr.BlockToIntC(1);
					else if(strcmp(lfr.SecBuffer, "PRIORITY") == 0)
						mMapList[opIndex].priority = lfr.BlockToIntC(1);
					else if(strcmp(lfr.SecBuffer, "SCALE_WIDTH") == 0)
						mMapList[opIndex].scale_width = (float)lfr.BlockToDblC(1);
					else if(strcmp(lfr.SecBuffer, "SCALE_HEIGHT") == 0)
						mMapList[opIndex].scale_height = (float)lfr.BlockToDblC(1);
					else
						g_Log.AddMessageFormat("Unknown identifier [%s] while reading from file %s.", lfr.SecBuffer, fileName);
				}
				else
				{
					lfr.CloseCurrent();
					g_Log.AddMessageFormat("Error reading %s, data must begin with an [ENTRY] block.", fileName);
					return -1;
				}
			}
		}
	}
	lfr.CloseCurrent();
	return 0;
}

MapLocationDef :: MapLocationDef()
{
	clear();
}

MapLocationDef :: ~MapLocationDef()
{
	MapName.clear();
}

void MapLocationDef :: clear(void)
{
	MapName.clear();
	method = 0;
	tilex = 0;
	tiley = 0;
	spanx = 0;
	spany = 0;
	priority = 1;
	resolvedIndex = -1;
	x1 = 0;
	y1 = 0;
	x2 = 0;
	y2 = 0;
}


MapLocationSet :: MapLocationSet()
{
	mZone = 0;
}

MapLocationSet :: ~MapLocationSet()
{
	mLocationList.clear();
}

MapLocationHandler :: MapLocationHandler()
{
}

MapLocationHandler :: ~MapLocationHandler()
{
	mLocationSet.clear();
}

int MapLocationHandler :: ZoneExist(int zone)
{
	for(size_t i = 0; i < mLocationSet.size(); i++)
		if(mLocationSet[i].mZone == zone)
			return i;

	return -1;
}

int MapLocationHandler :: AddLocation(int zone, MapLocationDef &data)
{
	//Add a location to the list.  First attempt to find the zone.  If the zone
	//does not exist, create a new entry for it.
	//Add the definition to either the new zone or existing zone's list.
	int found = ZoneExist(zone);
	if(found == -1)
	{
		MapLocationSet newset;
		newset.mZone = zone;
		mLocationSet.push_back(newset);
		found = mLocationSet.size() - 1;
		if(found >= 0)
			mLocationSet[found].mLocationList.push_back(data);
	}
	else
	{
		mLocationSet[found].mLocationList.push_back(data);
	}
	return found;
}

int MapLocationHandler :: SearchLocation(int zone, int x, int z)
{
	int found = ZoneExist(zone);
	if(found >= 0)
	{
		size_t a;
		for(a = 0; a < mLocationSet[found].mLocationList.size(); a++)
			if(x >= mLocationSet[found].mLocationList[a].x1 && x <= mLocationSet[found].mLocationList[a].x2)
				if(z >= mLocationSet[found].mLocationList[a].y1 && z <= mLocationSet[found].mLocationList[a].y2)
					return mLocationSet[found].mLocationList[a].resolvedIndex;
	}
	return -1;
}

const char* MapLocationHandler :: GetInternalMapName(int zone, int x, int z)
{
	int found = ZoneExist(zone);
	if(found >= 0)
	{
		size_t i;
		for(i = 0; i < mLocationSet[found].mLocationList.size(); i++)
			if(x >= mLocationSet[found].mLocationList[i].x1 && x <= mLocationSet[found].mLocationList[i].x2)
				if(z >= mLocationSet[found].mLocationList[i].y1 && z <= mLocationSet[found].mLocationList[i].y2)
					return mLocationSet[found].mLocationList[i].MapName.c_str();
	}
	return NULL;
}


int MapLocationHandler :: ResolveItems(void)
{
	size_t a, b;
	for(a = 0; a < mLocationSet.size(); a++)
	{
		for(b = 0; b < mLocationSet[a].mLocationList.size(); b++)
		{
			MapLocationDef *mld = &mLocationSet[a].mLocationList[b];
			mld->x1 = mld->tilex * 1920;
			mld->x2 = mld->x1 + (mld->spanx * 1920);

			mld->y1 = mld->tiley * 1920;
			mld->y2 = mld->y1 + (mld->spany * 1920);

			mld->resolvedIndex = MapDef.GetIndexByName(mld->MapName.c_str(), "Region");
			if(mld->resolvedIndex == -1)
			{
				g_Log.AddMessageFormat("Warning: could not resolve location name [%s]", mld->MapName.c_str());

				//Just set it to a null index so it won't crash if accessed at a
				//later time.
				mld->resolvedIndex = 0;
			}
		}
	}
	return 0;
}

int MapLocationHandler :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Error: could not open file [%s]", filename);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;

	MapLocationDef newitem;
	long curZone = 0;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			lfr.MultiBreak("=,");
			lfr.BlockToStringC(0, Case_Upper);
			if(strcmp(lfr.SecBuffer, "ZONEID") == 0)
			{
				curZone = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "HT") == 0)
			{
				newitem.clear();
				newitem.MapName = lfr.BlockToStringC(1, 0);
				newitem.tilex = lfr.BlockToIntC(2);
				newitem.tiley = lfr.BlockToIntC(3);
				newitem.spanx = lfr.BlockToIntC(4);
				newitem.spany = 1;
				AddLocation(curZone, newitem);
			}
			else if(strcmp(lfr.SecBuffer, "VT") == 0)
			{
				newitem.clear();
				newitem.MapName = lfr.BlockToStringC(1, 0);
				newitem.tilex = lfr.BlockToIntC(2);
				newitem.tiley = lfr.BlockToIntC(3);
				newitem.spanx = 1;
				newitem.spany = lfr.BlockToIntC(4);
				AddLocation(curZone, newitem);
			}
			else if(strcmp(lfr.SecBuffer, "RT") == 0)
			{
				newitem.clear();
				newitem.MapName = lfr.BlockToStringC(1, 0);
				newitem.tilex = lfr.BlockToIntC(2);
				newitem.tiley = lfr.BlockToIntC(3);
				newitem.spanx = lfr.BlockToIntC(4);
				newitem.spany = lfr.BlockToIntC(5);
				AddLocation(curZone, newitem);
			}
			else
			{
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s]", lfr.SecBuffer, filename);
			}
		}
	}
	lfr.CloseCurrent();

	ResolveItems();
	return 0;
}

ActiveInstance :: ActiveInstance()
{
	nutScriptPlayer = NULL;
	scriptPlayer = NULL;
	pvpGame = NULL;

	Clear();
}

ActiveInstance :: ~ActiveInstance()
{
	Clear();
}

void ActiveInstance :: Clear(void)
{
	mZone = 0;
	mMode = PVP::GameMode::PVE_ONLY;
	mZoneDefPtr = NULL;
	mInstanceID = 0;
	mPlayers = 0;
	mOwnerCreatureDefID = 0;
	mOwnerPartyID = 0;
	mLastHealthUpdate = 0;
	mNextCreatureBroadcast = 0;
	mLastMovementUpdate = 0;
	mNextTargetUpdate = 0;
	mNextUpdateTime = 0;
	mNextCreatureLocalScan = 0;
	mNextCreatureDeleteScan = 0;
	mExpireTime = 0;

	mSceneryEffects.clear();
	PlayerList.clear();
	PlayerListPtr.clear();
	mNextEffectTag = 0;
	uniqueSpawnManager.Clear();
	worldMarkers.Clear();

	StopPVP();

	size_t a;

#ifdef CREATUREQAV
	for(a = 0; a < NPCListPtr.size(); a++)
	{
		NPCListPtr[a]->UnloadResources();
		if(pendingOperations.Debug_HasCreature(NPCListPtr[a]) == true)
		{
			g_Log.AddMessageFormat("[CRITICAL] Creature not removed from access list: %s (%p)", NPCListPtr[a]->css.display_name, NPCListPtr[a]);
		}
		EraseAllCreatureReference(NPCListPtr[a]);
	}
#else
	ActiveInstance::CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
		it->second.UnloadResources();
#endif

	for(a = 0; a < SidekickListPtr.size(); a++)
	{
		SidekickListPtr[a]->UnloadResources();
		if(pendingOperations.Debug_HasCreature(SidekickListPtr[a]) == true)
			g_Log.AddMessageFormat("[CRITICAL] Creature not removed from access list: %s (%s)", SidekickListPtr[a]->css.display_name, SidekickListPtr[a]);
		EraseAllCreatureReference(SidekickListPtr[a]);
	}

	NPCList.clear();
	NPCListPtr.clear();

	SidekickList.clear();
	SidekickListPtr.clear();

	RegSim.clear();
	//questScriptList.clear();
	hateProfiles.profileList.clear();
	lootsys.Clear();
	tradesys.Clear();
	spawnsys.Clear();

	scaleProfile = NULL;
	dropRateProfile = NULL;
	mDropRateBonusMultiplier = 1.0F;
	mKillCount = 0;

	essenceShopList.Clear();
	itemShopList.Clear();

	ClearScriptObjects();
}

bool ActiveInstance :: StopPVP()
{
	if(pvpGame != NULL) {
		g_Log.AddMessageFormat("Stopping PVP game %d in %d", mZone, pvpGame->mId);
		g_PVPManager.ReleaseGame(pvpGame->mId);
		pvpGame = NULL;
		return true;
	}
	return false;
}

PVP::PVPGame * ActiveInstance :: StartPVP(int type)
{
	if(pvpGame != NULL) {
		g_Log.AddMessageFormat("Already in PVP game %d for %d", pvpGame->mId, mZone);
		return NULL;
	}
	pvpGame = g_PVPManager.NewGame();
	pvpGame->mGameType = type;
	g_Log.AddMessageFormat("New PVP game %d for %d", pvpGame->mId, mZone);

	char buf[64];
	int wpos = PrepExt_PVPStateUpdate(buf, pvpGame);
	LSendToAllSimulator(buf, wpos, -1);
	return pvpGame;
}

int ActiveInstance :: GetNewActorID(void)
{
	//New actors must be uniquely global ID.  Otherwise if a player swaps instances,
	//it may be assigned an ID that matches an ID of a creature from the old dungeon.
	//This will screw up the client and the player's avatar will assume the
	//former creature.

	return g_NextActorID++;
}

int ActiveInstance :: SimExist(SimulatorThread *simPtr)
{
	for(size_t i = 0; i < RegSim.size(); i++)
		if(RegSim[i] == simPtr)
			return i;
	return -1;
}

CreatureInstance * ActiveInstance :: GetPlayerByID(int id)
{
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
		if(it->CreatureID == id)
			return &*it;
	return NULL;
}

CreatureInstance * ActiveInstance :: GetPlayerByCDefID(int CDefID)
{
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
		if(it->CreatureDefID == CDefID)
			return &*it;
	return NULL;
}

CreatureInstance * ActiveInstance :: GetPlayerByName(const char *name)
{
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
		if(strcmp(it->css.display_name, name) == 0)
			return &*it;
	return NULL;
}

CreatureInstance * ActiveInstance :: GetPlayerByID(int id, list<CreatureInstance>::iterator &resIterator)
{
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
	{
		if(it->CreatureID == id)
		{
			resIterator = it;
			return &*it;
		}
	}
	return NULL;
}

int ActiveInstance :: DeletePlayerByID(int id)
{
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
	{
		if(it->CreatureID == id)
		{
			PlayerList.erase(it);
			return 1;
		}
	}
	return 0;
}

CreatureInstance * ActiveInstance :: LoadPlayer(CreatureInstance *source, SimulatorThread *simCall)
{
	//Load a character into the Active Player list.
	//Return an index into the quick access array.
	if(SimExist(simCall) >= 0)
	{
		g_Log.AddMessageFormat("LoadPlayer() Sim:%d already exists, quitting", simCall->InternalIndex);
		return NULL;
	}
	RegSim.push_back(simCall);

	CreatureInstance newItem;

	//Copy over instance information.  This will include buffs.
	newItem.CopyFrom(source);

	//Always set these parameters in case the source doesn't have them already.
	newItem.actInst = this;
	g_Log.AddMessageFormat("LoadPlayer() object created in %p", this);
	newItem.Faction = FACTION_PLAYERFRIENDLY;

	//CreatureID and CreatureDefID are the same for players
	newItem.CreatureDefID = source->CreatureDefID;
	//newItem.CreatureID = GetNewActorID();
	newItem.CreatureID = 2000000 + simCall->InternalIndex;

	//newItem.SimulatorIndex = source->SimulatorIndex;
	newItem.simulatorPtr = source->simulatorPtr;
	newItem.charPtr = source->charPtr;
	newItem.css.CopyFrom(&source->css);
	newItem.CurrentX = source->CurrentX;
	newItem.CurrentY = source->CurrentY;
	newItem.CurrentZ = source->CurrentZ;
	newItem.Heading = source->Heading;
	newItem.Rotation = source->Rotation;
	newItem.serverFlags = ServerFlags::IsPlayer;

	PlayerList.push_back(newItem);
	if(newItem.css.health == 0)
		PlayerList.back()._AddStatusList(StatusEffects::DEAD, -1);

	//SimList.push_back(SimIndex);

	//Now we can active the new Squirrel quest scripts in this
	newItem.charPtr->questJournal.activeQuests.StartScript(&PlayerList.back());

	RebuildPlayerList();


	//Notify the spawn management system to activate these tiles.
	int tx = source->CurrentX / SpawnTile::SPAWN_TILE_SIZE;
	int tz = source->CurrentZ / SpawnTile::SPAWN_TILE_SIZE;
	spawnsys.GenerateTile(tx, tz);
	//activityManager.UpdatePlayer(source->CreatureID, 0, 0);  //Pass dummy coords so the real update can take place.
	//activityManager.UpdatePlayer(source->CreatureID, tx, tz);

	return &PlayerList.back();
}

int ActiveInstance :: UnregisterPlayer(SimulatorThread *callSim)
{
	if(nutScriptPlayer != NULL && callSim->creatureInst != NULL) {
		std::vector<ScriptCore::ScriptParam> p;
		p.push_back(callSim->creatureInst->CreatureID);
		// Don't queue this, it's like the script will want to clean up before actual removal
		nutScriptPlayer->RunFunction("on_unregister", p, true);
		if(questNutScriptList.empty() == false)
		{
			for(uint i = 0 ; i < questNutScriptList.size(); i++) {
				QuestScript::QuestNutPlayer * p = questNutScriptList[i];
				p->RunFunction("on_unregister", std::vector<ScriptCore::ScriptParam>(), true);
			}
		}
	}
	return 0;
}

int ActiveInstance :: UnloadPlayer(SimulatorThread *callSim)
{
	int r = SimExist(callSim);
	if(r == -1)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] UnloadPlayer Sim:%d not found.", callSim->InternalIndex);
		return -1;
	}

	if(nutScriptPlayer != NULL && callSim->creatureInst != NULL) {
		std::vector<ScriptCore::ScriptParam> p;
		p.push_back(callSim->creatureInst->CreatureID);
		// Don't queue this, it's like the script will want to clean up before actual removal
		nutScriptPlayer->RunFunction("on_unload", p, true);
	}

	RegSim.erase(RegSim.begin() + r);

	int creatureID = callSim->creatureInst->CreatureID;

	//Notify the spawn management system to delete this player.
	//activityManager.RemovePlayer(creatureID);

	CreatureInstance * cInst = GetPlayerByID(creatureID);
	if(cInst == NULL)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] UnloadPlayer creature %d not found", creatureID);
		return -1;
	}

	RemovePlayerByID(creatureID);
	AdjustPlayerCount(-1);


	if(nutScriptPlayer != NULL && callSim->creatureInst != NULL) {
		std::vector<ScriptCore::ScriptParam> p;
		p.push_back(callSim->creatureInst->CreatureID);
		// Don't queue this, it's like the script will want to clean up before actual removal
		nutScriptPlayer->RunFunction("on_unloaded", p, true);
	}
	
	return 1;
}

int ActiveInstance :: RemovePlayerByID(int creatureID)
{
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
	{
		if(it->CreatureID == creatureID)
		{
			if(nutScriptPlayer != NULL) {
				std::vector<ScriptCore::ScriptParam> p;
				p.push_back(creatureID);
				// Don't queue this, it's like the script will want to clean up before actual removal
				nutScriptPlayer->RunFunction("on_remove", p, true);
			}

			int size = PrepExt_RemoveCreature(GSendBuf, it->CreatureID);
			LSendToLocalSimulator(GSendBuf, size, it->CurrentX, it->CurrentZ);
			EraseAllCreatureReference(&*it);
			g_Log.AddMessageFormat("PLAYER REMOVED (%s) at (%p)", it->css.display_name, &*it);
			PlayerList.erase(it);
			RebuildPlayerList();

			if(nutScriptPlayer != NULL) {
				std::vector<ScriptCore::ScriptParam> p;
				p.push_back(creatureID);
				// Don't queue this, it's like the script will want to clean up before actual removal
				nutScriptPlayer->RunFunction("on_removed", p, true);
			}

			return 1;
		}
	}
	return 0;
}

/*
int ActiveInstance :: ElementExist(int value, vector<int> &list)
{
	//Looks to see if the value exists in the given list
	size_t a;
	for(a = 0; a < list.size(); a++)
		if(list[a] == value)
			return a;
	return -1;
}

int ActiveInstance :: AddList(int value, vector<int> &list)
{
	int r = ElementExist(value, list);
	if(r == -1)
	{
		list.push_back(value);
		return 1;
	}
	return 0;
}

int ActiveInstance :: RemList(int value, vector<int> &list)
{
	int r = ElementExist(value, list);
	if(r >= 0)
	{
		list.erase(list.begin() + r);
		return 1;
	}

	return 0;
}
*/

int ActiveInstance :: ProcessMessage(MessageComponent *msg)
{
	BEGINTRY
	{

	int size;
	size = 0;
	int r;  //Generic result for some function calls
	switch(msg->message)
	{
	case BCM_CreatureDefRequest:
		SEND_DEBUG_MSG_ONE("BEGIN BCM_CreatureDefRequest", msg->simCall);
		size = PrepExt_CreatureDef(GSendBuf, (CreatureDefinition*)msg->param1);
		LSendToOneSimulator(GSendBuf, size, msg->SimulatorID);
		SEND_DEBUG_MSG_ONE("END BCM_CreatureDefRequest", msg->simCall);
		break;

	case BCM_UpdateCreatureDef:
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdateCreatureDef", -1);
		size = PrepExt_CreatureDef(GSendBuf, (CreatureDefinition*)msg->param1);
		//LSendToAllSimulator(GSendBuf, size, -1);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z);
		SEND_DEBUG_MSG_ALL("END BCM_UpdateCreatureDef", -1);
		break;
	case BCM_UpdateCreatureInstance:
		VERIFYCINST((CreatureInstance*)msg->param1);
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdateCreatureInstance", -1);
		size = PrepExt_CreatureInstance(GSendBuf, (CreatureInstance*) msg->param1);
		//LSendToAllSimulator(GSendBuf, size, -1);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z);
		SEND_DEBUG_MSG_ALL("END BCM_UpdateCreatureInstance", -1);
		break;
	case BCM_UpdateVelocity:
		VERIFYCINST((CreatureInstance*)msg->param1);
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdateVelocity", msg->SimulatorID);
		size = PrepExt_UpdateVelocity(GSendBuf, (CreatureInstance*) msg->param1);
		//LSendToAllSimulator(GSendBuf, size, msg->SimulatorID);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z, msg->SimulatorID);
		SEND_DEBUG_MSG_ALL("END BCM_UpdateVelocity", msg->SimulatorID);
		break;
	case BCM_UpdatePosition:
		VERIFYCINST((CreatureInstance*)msg->param1);
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdatePosition", -1);
		size = PrepExt_CreaturePos(GSendBuf, (CreatureInstance*)msg->param1);
		size += PrepExt_GeneralMoveUpdate(&GSendBuf[size], (CreatureInstance*) msg->param1);
		//LSendToAllSimulator(GSendBuf, size, -1);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z);
		SEND_DEBUG_MSG_ALL("END BCM_UpdatePosition", -1);
		break;
	case BCM_UpdateAppearance:
		VERIFYCINST((CreatureInstance*)msg->param1);
		size = PrepExt_UpdateAppearance(GSendBuf, (CreatureInstance*)msg->param1);
		//LSendToAllSimulator(GSendBuf, size, -1);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z);
		break;
	case BCM_UpdatePosInc:
		VERIFYCINST((CreatureInstance*)msg->param1);
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdatePosInc", -1);
		//size = PrepExt_UpdatePosInc(GSendBuf, (CreatureInstance*) msg->param1);
		// TODO: Change the name of this message if it works.
		size = PrepExt_GeneralMoveUpdate(GSendBuf, (CreatureInstance*) msg->param1);
		//LSendToAllSimulator(GSendBuf, size, -1);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z);
		SEND_DEBUG_MSG_ALL("END BCM_UpdatePosInc", -1);
		break;
	case BCM_UpdateElevation:
		VERIFYCINST((CreatureInstance*)msg->param1);
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdateElevation", msg->SimulatorID);
		size = PrepExt_UpdateElevation(GSendBuf, (CreatureInstance*) msg->param1);
		//LSendToAllSimulator(GSendBuf, size, msg->SimulatorID);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z, msg->SimulatorID);
		SEND_DEBUG_MSG_ALL("END BCM_UpdateElevation", msg->SimulatorID);
		break;
	case BCM_SendHealth:
		size = PrepExt_SendHealth(GSendBuf, msg->param1, (short)msg->param2);
		//LSendToAllSimulator(GSendBuf, size, -1);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z);
		break;
	case BCM_RequestTarget:
		((CreatureInstance*)msg->param1)->RequestTarget(msg->param2);
		break;
	case BCM_AbilityRequest:
		VERIFYCINST((CreatureInstance*)msg->param1);
		//ActivateAbility(msg->param1, (short)msg->param2, &SimChar[msg->simCall].csd);
		r = ActivateAbility((CreatureInstance*)msg->param1, (short)msg->param2, EventType::onRequest, 0);
		if(r > 0 && msg->SimulatorID >= 0)
		{
			SimulatorThread *simPtr = GetSimulatorByID(msg->SimulatorID);
			if(simPtr != NULL)
				simPtr->SendAbilityErrorMessage(r & 0xFF);
		}
		break;
	case BCM_AbilityActivate2:
		r = ActivateAbility((CreatureInstance*)msg->param1, (short)msg->param2, EventType::onActivate, 0);
		if(r == 0)
			((CreatureInstance*)(msg->param1))->ab[0].Clear("BCM_AbilityActivate2");

		break;
	case BCM_ActorJump:
		size = PrepExt_ActorJump(GSendBuf, msg->param1);
		//LSendToAllSimulator(GSendBuf, size, msg->SimulatorID);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z, msg->SimulatorID);
		break;
	case BCM_RemoveCreature:
		size = PrepExt_RemoveCreature(GSendBuf, msg->param1);
		//LSendToAllSimulator(GSendBuf, size, msg->SimulatorID);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z, msg->SimulatorID);
		break;
	case BCM_UpdateFullPosition:
		VERIFYCINST((CreatureInstance*)msg->param1);
		SEND_DEBUG_MSG_ALL("BEGIN BCM_UpdateFullPosition", msg->SimulatorID);
		size = PrepExt_UpdateFullPosition(GSendBuf, (CreatureInstance*) msg->param1);
		//LSendToAllSimulator(GSendBuf, size, msg->SimulatorID);
		LSendToLocalSimulator(GSendBuf, size, msg->x, msg->z, msg->SimulatorID);
		SEND_DEBUG_MSG_ALL("END BCM_UpdateFullPosition", msg->SimulatorID);
		break;
	case BCM_Notice_MOTD:
		if(g_MOTD_Message.size() == 0)
			break;
		size = PrepExt_GenericChatMessage(GSendBuf, 0, g_MOTD_Name.c_str(), g_MOTD_Channel.c_str(), g_MOTD_Message.c_str());
		LSendToOneSimulator(GSendBuf, size, msg->SimulatorID);
		break;
	case BCM_SpawnCreateCreature:
		SpawnCreate((CreatureInstance*)msg->param1, msg->param2);
		break;
	case BCM_CreatureDelete:
		CreatureDelete(msg->param1);
		break;
		/*
	case BCM_SidekickAdd:
		CreateSidekick((CreatureInstance*)msg->param1, msg->param2);
		break;
	case BCM_SidekickRemoveOne:
		SidekickRemoveOne((CreatureInstance*)msg->param1);
		break;
	case BCM_SidekickRemoveAll:
		SidekickRemoveAll((CreatureInstance*)msg->param1);
		break;
		*/
	case BCM_SidekickAttack:
		SidekickAttack((CreatureInstance*)msg->param1);
		break;
	case BCM_SidekickCall:
		SidekickCall((CreatureInstance*)msg->param1);
		break;
	case BCM_SidekickScatter:
		SidekickScatter((CreatureInstance*)msg->param1);
		break;
	case BCM_SidekickWarp:
		SidekickWarp((CreatureInstance*)msg->param1);
		break;
	case BCM_PlayerLogIn:
		sprintf(GAuxBuf, "%s has logged in.", ((CharacterData*)msg->param1)->cdef.css.display_name);
		LogChatMessage(GAuxBuf);
		size = PrepExt_SendInfoMessage(GSendBuf, GAuxBuf, INFOMSG_INFO);
		SendToAllSimulator(GSendBuf, size, msg->SimulatorID);
		break;
	case BCM_PlayerLogOut:
		sprintf(GAuxBuf, "%s has disconnected.", ((CharacterData*)msg->param1)->cdef.css.display_name);
		LogChatMessage(GAuxBuf);
		size = PrepExt_SendInfoMessage(GSendBuf, GAuxBuf, INFOMSG_INFO);
		SendToAllSimulator(GSendBuf, size, msg->param2);
		break;
	case BCM_PlayerFriendLogState:
		size = PrepExt_FriendsLogStatus(GSendBuf, ((SimulatorThread*)msg->param1)->pld.charPtr, msg->param2);
		SendToFriendSimulator(GSendBuf, size, ((SimulatorThread*)msg->param1)->pld.charPtr->cdef.CreatureDefID);
		break;
	case BCM_RunObjectInteraction:
		RunObjectInteraction((SimulatorThread*)msg->param1, msg->param2);
		break;
	case BCM_RunTranslocate:
		((SimulatorThread*)(msg->param1))->RunTranslocate();
		break;
	case BCM_RunPortalRequest:
		((SimulatorThread*)(msg->param1))->RunPortalRequest();
		break;
	case BCM_Disconnect:
		{
			SimulatorThread *sim = g_SimulatorManager.GetPtrByID(msg->param1);
			if(sim != NULL)
				sim->ProcessDisconnect();
		}
		break;
	}

	} //End try
	BEGINCATCH
	{
		char buffer[512];
		int wpos = 0;
		wpos += sprintf(&buffer[wpos], "Exception in ActiveInstance::ProcessMessage()\r\n");
		wpos += sprintf(&buffer[wpos], "  Instance ID: %d\r\n", mInstanceID);
		wpos += sprintf(&buffer[wpos], "Message Properties:\r\n");
		wpos += sprintf(&buffer[wpos], "  ID: %d\r\n", msg->message);
		wpos += sprintf(&buffer[wpos], "  Inst: %p\r\n", msg->actInst);
		wpos += sprintf(&buffer[wpos], "  Param1: %ld\r\n", msg->param1);
		wpos += sprintf(&buffer[wpos], "  Param2: %ld\r\n", msg->param2);
		wpos += sprintf(&buffer[wpos], "  SimCall: %d\r\n", msg->SimulatorID);
		POPUP_MESSAGE(buffer, "Critical Error");
	}

	return 0;
}

void ActiveInstance :: BroadcastMessage(const char *message)
{
	for(size_t i = 0; i < RegSim.size(); i++)
		if((RegSim[i]->CheckStateGameplayProtocol() == true))
			RegSim[i]->BroadcastMessage(message);
}

int ActiveInstance :: LSendToAllSimulator(const char *buffer, int length, int ignoreIndex)
{
	int success = 0;
	for(size_t i = 0; i < RegSim.size(); i++)
	{
		if((RegSim[i]->CheckStateGameplayProtocol() == true) && (RegSim[i]->InternalIndex != ignoreIndex))
		{
			int res = RegSim[i]->AttemptSend(buffer, length);
			if(res >= 0)
				success++;
		}
	}
	return success;
}

void ActiveInstance :: LSendToLocalSimulator(const char *buffer, int length, int x, int z, int ignoreIndex)
{
	//LSendToAllSimulator(buffer, length, -1);
	for(size_t i = 0; i < RegSim.size(); i++)
	{
		if(RegSim[i]->InternalID == ignoreIndex)
			continue;

		if(RegSim[i]->CheckStateGameplayProtocol() == false)
			continue;

		if(abs(RegSim[i]->creatureInst->CurrentX - x) > g_CreatureListenRange)
			continue;
		if(abs(RegSim[i]->creatureInst->CurrentZ - z) > g_CreatureListenRange)
			continue;

		RegSim[i]->AttemptSend(buffer, length);
	}
}

int ActiveInstance :: LSendToOneSimulator(const char *buffer, int length, int simIndex)
{
	SimulatorThread *simPtr = GetSimulatorByID(simIndex);
	if(simPtr == NULL)
		return 0;

	int success = 0;
	if(simPtr->CheckStateGameplayProtocol() == true)
	{
		int res = simPtr->AttemptSend(buffer, length);
		if(res >= 0)
			success++;
	}
	return success;
}

void ActiveInstance :: LSendToOneSimulator(const char *buffer, int length, SimulatorThread *simPtr)
{
	if(simPtr == NULL)
		return;
	simPtr->AttemptSend(buffer, length);
}

void ActiveInstance :: SendActors(void)
{
	int size = 0;
	if(g_ServerTime >= mNextCreatureBroadcast)
	{
#ifdef DEBUG_TIME
		Debug::TimeTrack("SendActors", 25);
#endif

		//g_Log.AddMessageFormat("Update");
		mNextCreatureBroadcast = g_ServerTime + g_RebroadcastDelay;
		size_t a, b;
		// Players are handled in the main loop
		for(b = 0; b < RegSim.size(); b++)
		{
			size = 0;
			//Build messages for creatures.  Assume that moving creatures
			//are already sending updates regularly.

#ifdef CREATUREQAV
			for(a = 0; a < NPCListPtr.size(); a++)
			{
				if(abs(NPCListPtr[a]->CurrentX - RegSim[b]->creatureInst->CurrentX) > g_CreatureListenRange)
					continue;
				if(abs(NPCListPtr[a]->CurrentZ - RegSim[b]->creatureInst->CurrentZ) > g_CreatureListenRange)
					continue;

				size += PrepExt_GeneralMoveUpdate(&GSendBuf[size], NPCListPtr[a]);
				if(size > Global::MAX_SEND_CHUNK_SIZE)
				{
					RegSim[b]->AttemptSend(GSendBuf, size);
					size = 0;
				}
			}
#else
			CREATURE_IT it;
			for(it = NPCList.begin(); it != NPCList.end(); ++it)
			{
				if(abs(it->second.CurrentX - RegSim[b]->creatureInst->CurrentX) > g_CreatureListenRange)
					continue;
				if(abs(it->second.CurrentZ - RegSim[b]->creatureInst->CurrentZ) > g_CreatureListenRange)
					continue;

				size += PrepExt_GeneralMoveUpdate(&GSendBuf[size], &it->second);
				if(size > Global::MAX_SEND_CHUNK_SIZE)
				{
					RegSim[b]->AttemptSend(GSendBuf, size, 4096);
					size = 0;
				}
			}
#endif

			for(a = 0; a < SidekickListPtr.size(); a++)
			{
				if(abs(SidekickListPtr[a]->CurrentX - RegSim[b]->creatureInst->CurrentX) > g_CreatureListenRange)
					continue;
				if(abs(SidekickListPtr[a]->CurrentZ - RegSim[b]->creatureInst->CurrentZ) > g_CreatureListenRange)
					continue;

				size += PrepExt_GeneralMoveUpdate(&GSendBuf[size], SidekickListPtr[a]);
				if(size > Global::MAX_SEND_CHUNK_SIZE)
				{
					RegSim[b]->AttemptSend(GSendBuf, size);
					size = 0;
				}
			}
			if(size > 0)
				RegSim[b]->AttemptSend(GSendBuf, size);
		}
	}
}

void ActiveInstance :: UpdateCreatureLocalStatus(void)
{
	//Scans all NPCs to determine if any are in local range of players.
	if(g_ServerTime >= mNextCreatureLocalScan)
	{
		mNextCreatureLocalScan = g_ServerTime + g_LocalActivityScanDelay;
#ifdef CREATUREQAV
		vector<int> localActive;
		localActive.assign(NPCList.size(), 0);
#endif
		for(size_t p = 0; p < RegSim.size(); p++)
		{
#ifdef CREATUREQAV
			for(size_t c = 0; c < NPCListPtr.size(); c++)
			{
				if(abs(NPCListPtr[c]->CurrentX - RegSim[p]->creatureInst->CurrentX) > g_LocalActivityRange)
					continue;
				if(abs(NPCListPtr[c]->CurrentZ - RegSim[p]->creatureInst->CurrentZ) > g_LocalActivityRange)
					continue;

				localActive[c]++;
			}
#else
			int c = 0;
			CREATURE_IT it;
			for(it = NPCList.begin(); it != NPCList.end(); ++it)
			{
				if(abs(it->second.CurrentX - RegSim[p]->creatureInst->CurrentX) > g_LocalActivityRange)
				{
					it->second.SetServerFlag(ServerFlags::LocalActive, false);
					continue;
				}
				if(abs(it->second.CurrentZ - RegSim[p]->creatureInst->CurrentZ) > g_LocalActivityRange)
				{
					it->second.SetServerFlag(ServerFlags::LocalActive, false);
					continue;
				}
				it->second.SetServerFlag(ServerFlags::LocalActive, true);
			}
#endif
		}


#ifdef CREATUREQAV
		for(size_t i = 0; i < localActive.size(); i++)
		{
			NPCListPtr[i]->SetServerFlag(ServerFlags::LocalActive, (localActive[i] > 0));
		}
#endif
	}
}

void ActiveInstance :: RemoveDeadCreatures(void)
{
	if(g_ServerTime < mNextCreatureDeleteScan)
		return;

	mNextCreatureDeleteScan = g_ServerTime + CREATURE_DELETE_RECHECK;

	std::vector<int> deadList;
#ifdef CREATUREQAV
	for(size_t i = 0; i < NPCListPtr.size(); i++)
	{
		if(NPCListPtr[i]->serverFlags & ServerFlags::TriggerDelete)
		{
			if(g_ServerTime >= NPCListPtr[i]->deathTime)
				deadList.push_back(NPCListPtr[i]->CreatureID);
		}
		else if(NPCListPtr[i]->HasStatus(StatusEffects::DEAD))
		{
			unsigned long compareTime = SpawnManager::GARBAGE_DELAY_DEAD;
			if(NPCListPtr[i]->activeLootID != 0)
				compareTime = SpawnManager::GARBAGE_DELAY_LOOT;

			if(g_ServerTime >= NPCListPtr[i]->deathTime + compareTime)
			{
				if(NPCListPtr[i]->spawnTile != NULL)
					NPCListPtr[i]->spawnTile->RemoveAttachedCreature(NPCListPtr[i]->CreatureID);
				deadList.push_back(NPCListPtr[i]->CreatureID);
			}
		}
	}
#else
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
	{
		if(it->second.serverFlags & ServerFlags::TriggerDelete)
		{
			if(g_ServerTime >= it->second.deathTime)
				deadList.push_back(it->second.CreatureID);
		}
		else if(it->second.HasStatus(StatusEffects::DEAD))
		{
			unsigned long compareTime = SpawnManager::GARBAGE_DELAY_DEAD;
			if(it->second.activeLootID != 0)
				compareTime = SpawnManager::GARBAGE_DELAY_LOOT;

			if(g_ServerTime >= it->second.deathTime + compareTime)
			{
				if(it->second.spawnTile != NULL)
					it->second.spawnTile->RemoveAssocID(it->second.CreatureID);
				deadList.push_back(it->second.CreatureID);
			}
		}
	}
#endif
	if(deadList.size() > 0)
	{
		for(size_t i = 0; i < deadList.size(); i++)
		{
			//g_Log.AddMessageFormat("Removing: %d", deadList[i]);
			RemoveNPCInstance(deadList[i]);
		}
		//RebuildNPCList();
	}

	/*
	int fIndex = 0;
	size_t a;
	while(fIndex >= 0)
	{
		fIndex = -1;
		for(a = 0; a < NPCListPtr.size(); a++)
		{
			if(NPCListPtr[a]->HasStatus(StatusEffects::DEAD))
			{
				unsigned long compareTime = SpawnManager::GARBAGE_DELAY_DEAD;
				if(NPCListPtr[a]->activeLootID != 0)
					compareTime = SpawnManager::GARBAGE_DELAY_LOOT;

				if(g_ServerTime >= NPCListPtr[a]->deathTime + compareTime)
				{
					fIndex = a;
					if(NPCListPtr[a]->spawnTile != NULL)
						NPCListPtr[a]->spawnTile->RemoveAssocID(NPCListPtr[a]->CreatureID);
					RemoveNPCInstance(NPCListPtr[a]->CreatureID, true);
				}
			}
		}
	}
	*/
}

CreatureInstance * ActiveInstance :: GetClosestEnemy(CreatureInstance *orig)
{
	//Sidekick targetting.
	const static int AGGRO_RANGE = 150;
	size_t a;
	int cdist = AGGRO_RANGE;
	int dist;
	CreatureInstance *targ = NULL;
	/*
	for(a = 0; a < (int)PlayerListPtr.size(); a++)
	{
		if(orig->AnchorObject != PlayerListPtr[a])
		{
			if(orig->_ValidTargetFlag(PlayerListPtr[a], TargetStatus::Enemy_Alive) == true)
			{
				dist = GetPlaneRange(PlayerListPtr[a], orig);
				if(dist < cdist)
				{
					cdist = dist;
					targ = PlayerListPtr[a];
				}
			}
		}
	}*/
#ifdef CREATUREQAV
	for(a = 0; a < NPCListPtr.size(); a++)
	{
		if(orig->_ValidTargetFlag(NPCListPtr[a], TargetStatus::Enemy_Alive) == true)
		{
			dist = GetPlaneRange(NPCListPtr[a], orig, cdist);
			if(dist < cdist)
			{
				cdist = dist;
				targ = NPCListPtr[a];
			}
		}
	}
#else
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
	{
		if(orig->_ValidTargetFlag(&it->second, TargetStatus::Enemy_Alive) == true)
		{
			dist = GetPlaneRange(&it->second, orig, cdist);
			if(dist < cdist)
			{
				cdist = dist;
				targ = &it->second;
			}
		}
	}
#endif
	for(a = 0; a < SidekickListPtr.size(); a++)
	{
		if(orig->_ValidTargetFlag(SidekickListPtr[a], TargetStatus::Enemy_Alive) == true)
		{
			dist = GetPlaneRange(SidekickListPtr[a], orig, cdist);
			if(dist < cdist)
			{
				cdist = dist;
				targ = SidekickListPtr[a];
			}
		}
	}
	return targ;
}

CreatureInstance * ActiveInstance :: GetClosestAggroTarget(CreatureInstance *sourceActor)
{
	//For creatures when selecting a target to attack.
	//INVINCIBLE status is used for player godmode, so use that as the ultimate target check.
	//Friendly NPCs will normally spawn with both UNATTACKABLE and INVINCIBLE.
	CreatureInstance *targ = NULL;
	targ = NULL;
	int cdist = AGGRO_MAXIMUM_RANGE;
	for(size_t i = 0; i < PlayerListPtr.size(); i++)
	{
		if(PlayerListPtr[i]->serverFlags & ServerFlags::Noncombatant)
			continue;
		if(PlayerListPtr[i]->css.health == 0)
			continue;
		if(PlayerListPtr[i]->HasStatus(StatusEffects::INVINCIBLE))
			continue;
		if(PlayerListPtr[i]->Faction == sourceActor->Faction)
			continue;
		if(abs(PlayerListPtr[i]->CurrentY - sourceActor->CurrentY) >= AGGRO_ELEV_RANGE)
			continue;

		int aggroRange = sourceActor->GetAggroRange(PlayerListPtr[i]);
		int dist = GetPlaneRange(sourceActor, PlayerListPtr[i], cdist);
		if(dist <= aggroRange && dist < cdist)
		{
			targ = PlayerListPtr[i];
			cdist = dist;
		}
	}
#ifdef CREATUREQAV
	for(size_t i = 0; i < NPCListPtr.size(); i++)
	{
		if(NPCListPtr[i] == sourceActor)
			continue;
		if(NPCListPtr[i]->css.health == 0)
			continue;
		if(NPCListPtr[i]->HasStatus(StatusEffects::INVINCIBLE))
			continue;
		if(NPCListPtr[i]->Faction == sourceActor->Faction)
			continue;
		if(abs(NPCListPtr[i]->CurrentY - sourceActor->CurrentY) >= AGGRO_ELEV_RANGE)
			continue;
		int dist = GetPlaneRange(sourceActor, NPCListPtr[i], cdist);
		if(dist < cdist)
		{
			targ = NPCListPtr[i];
			cdist = dist;
		}
	}
#else
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
	{
		if(&it->second == sourceActor)
			continue;
		if(it->second.css.health == 0)
			continue;
		if(it->second.HasStatus(StatusEffects::INVINCIBLE))
			continue;
		if(it->second.Faction == sourceActor->Faction)
			continue;
		if(abs(it->second.CurrentY - sourceActor->CurrentY) >= AGGRO_ELEV_RANGE)
			continue;
		int dist = GetPlaneRange(sourceActor, &it->second, cdist);
		if(dist < cdist)
		{
			targ = &it->second;
			cdist = dist;
		}
	}
#endif
	return targ;
}


int ActiveInstance :: GetBoxRange(CreatureInstance *obj1, CreatureInstance *obj2)
{
	//Returns the box distance between two Simulator players
	//This is the offset between them, based on the X or Z axis.
	//Useful for quick comparisons or when distance needs to be roughly estimated,
	//for example, creatures coming into extended visual range.
	int xlen = abs(obj1->CurrentX - obj2->CurrentX);
	int zlen = abs(obj1->CurrentZ - obj2->CurrentZ);
	if(xlen < zlen)
		return zlen;
	else
		return xlen;
}

int ActiveInstance :: GetPlaneRange(CreatureInstance *obj1, CreatureInstance *obj2, int threshold)
{
	//Returns the proper distance between two creatures, comparing only their
	//X and Z positions.
	int xlen = abs(obj1->CurrentX - obj2->CurrentX);
	if(xlen > threshold)
		return DISTANCE_FAILED;

	int zlen = abs(obj1->CurrentZ - obj2->CurrentZ);
	if(zlen > threshold)
		return DISTANCE_FAILED;

	double dist = sqrt((double)((xlen * xlen) + (zlen * zlen)));
	return (int)dist;
}

int ActiveInstance :: GetActualRange(CreatureInstance *obj1, CreatureInstance *obj2, int threshold)
{
	//Returns the proper distance between two creture players, comparing
	//all axis.
	int xlen = abs(obj1->CurrentX - obj2->CurrentX);
	if(xlen > threshold)
		return DISTANCE_FAILED;

	int zlen = abs(obj1->CurrentZ - obj2->CurrentZ);
	if(zlen > threshold)
		return DISTANCE_FAILED;

	int ylen = abs(obj1->CurrentY - obj2->CurrentY);
	if(ylen > threshold)
		return DISTANCE_FAILED;

	//Get X and Z first
	double dist = sqrt((double)((xlen * xlen) + (zlen * zlen)));
	int tdist = (int)dist;
	//Get Y with XZ (from previous)
	dist = sqrt((double)((ylen * ylen) + (tdist * tdist)));
	return (int)dist;
}

int ActiveInstance :: GetPointRange(CreatureInstance *obj1, float x, float y, float z, int threshold)
{
	//Returns distance between a creature instance and an arbitrary point.
	int xlen = abs(obj1->CurrentX - (int)x);
	if(xlen > threshold)
		return DISTANCE_FAILED;

	int zlen = obj1->CurrentZ - (int)z;
	if(zlen > threshold)
		return DISTANCE_FAILED;

	int ylen = abs(obj1->CurrentY - (int)y);
	if(ylen > threshold)
		return DISTANCE_FAILED;

	//Get X and Z first
	double dist = sqrt((double)((xlen * xlen) + (zlen * zlen)));
	int tdist = (int)dist;
	//Get Y with XZ (from previous)
	dist = sqrt((double)((ylen * ylen) + (tdist * tdist)));
	return (int)dist;
}

int ActiveInstance :: GetPointRangeXZ(CreatureInstance *obj1, float x, float z, int threshold)
{
	//Returns distance between a creature instance and an arbitrary point.
	int xlen = abs(obj1->CurrentX - (int)x);
	if(xlen > threshold)
		return DISTANCE_FAILED;

	int zlen = abs(obj1->CurrentZ - (int)z);
	if(zlen > threshold)
		return DISTANCE_FAILED;

	double dist = sqrt((double)((xlen * xlen) + (zlen * zlen)));
	return (int)dist;
}

CreatureInstance * ActiveInstance :: GetInstanceByCID(int CID)
{
	if(PlayerListPtr.size() != PlayerList.size())
	{
		g_Log.AddMessageFormatW(MSG_CRIT, "GetInstanceByCID PlayerList Mismatch %d:%d", PlayerListPtr.size() != PlayerList.size());
		return NULL;
	}

	size_t a;
	for(a = 0; a < PlayerListPtr.size(); a++)
	{
		if(PlayerListPtr[a]->CreatureID == CID)
		{
			//g_Log.AddMessageFormatW(MSG_DIAGV, "Found: %d [CID:%d]", a, CID);
			return PlayerListPtr[a];
		}
	}
#ifndef CREATUREMAP
	for(a = 0; a < NPCListPtr.size(); a++)
	{
		if(NPCListPtr[a]->CreatureID == CID)
		{
			//g_Log.AddMessageFormatW(MSG_DIAGV, "Found: %d [CID:%d]", a, CID);
			return NPCListPtr[a];
		}
	}
#else
	CREATURE_IT it = NPCList.find(CID);
	if(it != NPCList.end())
		return &it->second;
#endif

	if(SidekickListPtr.size() != SidekickList.size())
	{
		g_Log.AddMessageFormatW(MSG_CRIT, "GetInstanceByCID PlayerList Mismatch %d:%d", SidekickListPtr.size() != SidekickList.size());
		return NULL;
	}
	for(a = 0; a < SidekickListPtr.size(); a++)
	{
		if(SidekickListPtr[a]->CreatureID == CID)
		{
			//g_Log.AddMessageFormatW(MSG_DIAGV, "Found: %d [CID:%d]", a, CID);
			return SidekickListPtr[a];
		}
	}

	//g_Log.AddMessageFormatW(MSG_DIAGV, "CID not found: %d", CID);
	return NULL;
}

CreatureInstance * ActiveInstance :: GetNPCInstanceByCID(int CID)
{
#ifndef CREATUREMAP
	size_t a;
	for(a = 0; a < NPCListPtr.size(); a++)
		if(NPCListPtr[a]->CreatureID == CID)
			return NPCListPtr[a];

	return NULL;
#else
	CREATURE_IT it = NPCList.find(CID);
	if(it != NPCList.end())
		return &it->second;

	return NULL;
#endif
}

// The instance script needs a way to look up creatures by ID for special boss purposes.
CreatureInstance * ActiveInstance :: GetNPCInstanceByCDefID(int CDefID)
{
	for(size_t i = 0; i < NPCListPtr.size(); i++)
	{
		if(NPCListPtr[i]->CreatureDefID == CDefID)
			return NPCListPtr[i];
	}
	return NULL;
}

void ActiveInstance :: GetNPCInstancesByCDefID(int CDefID, vector<int> *cids)
{
	for(size_t i = 0; i < NPCListPtr.size(); i++)
		if(NPCListPtr[i]->CreatureDefID == CDefID)
			cids->push_back(NPCListPtr[i]->CreatureID);
}

void ActiveInstance :: ResolveCreatureDef(int CreatureInstanceID, int *responsePtr)
{
#ifndef CREATUREMAP
	size_t a;
	for(a = 0; a < NPCListPtr.size(); a++)
	{
		if(NPCListPtr[a]->CreatureID == CreatureInstanceID)
		{
			*responsePtr = NPCListPtr[a]->CreatureDefID;
			return;
		}
	}
	*responsePtr = -1;
#else
	CREATURE_IT it = NPCList.find(CreatureInstanceID);
	if(it != NPCList.end())
	{
		*responsePtr = it->second.CreatureDefID;
		return;
	}
	*responsePtr = -1;
#endif
}

/*
int ActiveInstance :: LoadNPC(void)
{
	//TODO: This function is obsolete.

	char filename[64];
	sprintf(filename, "Instance\\%d.txt", mInstanceExtID);
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open instance file [%s]", filename);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;

	CreatureInstance newItem;
	newItem.actInst = this;
	newItem.CreatureID = -1;
	newItem.Faction = FACTION_PLAYERHOSTILE;
	sprintf(newItem.CurrentZone, "[%d-%d-0]", mInstanceExtID, mZone);

	int r;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.CreatureID != -1)
				{
					newItem.Instantiate();
					NPCList.push_back(newItem);
					newItem.CreatureID = -1;
					newItem.aiScript = NULL;
				}
			}
			else if(strcmp(lfr.SecBuffer, "CID") == 0)
			{
				newItem.CreatureDefID = lfr.BlockToIntC(1);
				int cdef = CreatureDef.GetIndex(newItem.CreatureDefID);
				if(cdef >= 0)
					memcpy(&newItem.css, &CreatureDef.NPC[cdef].css, sizeof(CharacterStatSet));
				else
					g_Log.AddMessageFormat("[WARNING] Could not resolve CreatureDef [%d] in file [%s]", cdef, filename);
			}
			else if(strcmp(lfr.SecBuffer, "IID") == 0)
			{
				newItem.CreatureID = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "Pos") == 0)
			{
				newItem.CurrentX = lfr.BlockToIntC(1);
				newItem.CurrentY = lfr.BlockToIntC(2);
				newItem.CurrentZ = lfr.BlockToIntC(3);
			}
			else
			{
				g_Log.AddMessageFormat("[WARNING] Unhandled identifier [%s] in file [%s]", lfr.SecBuffer, filename);
			}
		}
	}
	if(newItem.CreatureID != -1)
	{
		newItem.Instantiate();
		NPCList.push_back(newItem);
		newItem.CreatureID = -1;
	}
	lfr.CloseCurrent();
	RebuildNPCList();
	g_Log.AddMessageFormat("Instance data loaded [%s]", filename);
	return 0;
}
*/

void ActiveInstance :: RebuildPlayerList(void)
{
	//Should only be called by the thread running actions for this instance.
	//Need to rebuild the array of quick-access pointers.


	PlayerListPtr.clear();
	list<CreatureInstance>::iterator it;
	for(it = PlayerList.begin(); it != PlayerList.end(); ++it)
		PlayerListPtr.push_back(&*it);
}

void ActiveInstance :: RebuildNPCList(void)
{
	//Should only be called by the thread running actions for this instance.
	//Need to rebuild the array of quick-access pointers.
#ifdef CREATUREQAV
	NPCListPtr.clear();
	//list<CreatureInstance>::iterator it;
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
		NPCListPtr.push_back(&it->second);
		//NPCListPtr.push_back(&*it);
#endif
}

CreatureInstance* ActiveInstance :: SpawnCreate(CreatureInstance * sourceActor, int CDefID)
{
	// Resulting code for query: spawn.create
	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if(cdef == NULL)
		return NULL;
	/*
	int cdindex = CreatureDef.GetIndex(CDefID);
	if(cdindex == -1)
		return;
	*/

	CreatureInstance newItem;
	newItem.CreatureDefID = CDefID;
	newItem.CreatureID = GetNewActorID();
	newItem.actInst = this;
	newItem.css.CopyFrom(&cdef->css);
	newItem.CurrentX = sourceActor->CurrentX;
	newItem.CurrentY = sourceActor->CurrentY;
	newItem.CurrentZ = sourceActor->CurrentZ;
	newItem.Faction = FACTION_PLAYERHOSTILE;
	newItem.BuildZoneString(mInstanceID, mZone, 0);
	newItem.css.health = newItem.GetMaxHealth(false);
	newItem.css.aggro_players = 1;
	newItem.tetherNodeX = newItem.CurrentX;
	newItem.tetherNodeZ = newItem.CurrentZ;

	newItem.SetServerFlag(ServerFlags::IsNPC, true);
	newItem.SetServerFlag(ServerFlags::LocalActive, true);

	CreatureInstance *retPtr = NULL;
#ifndef CREATUREMAP
	NPCList.push_back(newItem);
	NPCList.back().Instantiate();
	retPtr = &NPCList.back();
#else
	CREATURE_IT it = NPCList.insert(NPCList.end(), CREATURE_PAIR(newItem.CreatureID, newItem));
	it->second.Instantiate();
	retPtr = &it->second;
#endif
	RebuildNPCList();
	return retPtr;
}

CreatureInstance* ActiveInstance :: SpawnGeneric(int CDefID, int x, int y, int z, int facing, int SpawnFlags)
{
	// Generic spawn, do not assign any internal links to spawn tiles or such.
	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if(cdef == NULL)
		return NULL;

	CreatureInstance newItem;
	newItem.CreatureDefID = CDefID;
	newItem.CreatureID = GetNewActorID();
	newItem.actInst = this;
	newItem.css.CopyFrom(&cdef->css);
	newItem.CurrentX = x;
	newItem.CurrentY = y;
	newItem.CurrentZ = z;
	newItem.Heading = facing;
	newItem.Rotation = facing;
	newItem.Faction = FACTION_PLAYERHOSTILE;
	newItem.BuildZoneString(mInstanceID, mZone, 0);
	newItem.css.health = newItem.GetMaxHealth(false);
	newItem.tetherNodeX = newItem.CurrentX;
	newItem.tetherNodeZ = newItem.CurrentZ;

	newItem.SetServerFlag(ServerFlags::IsNPC, true);
	newItem.SetServerFlag(ServerFlags::LocalActive, true);
	newItem.SetServerFlag(ServerFlags::NeutralInactive, true);
	newItem.SetServerFlag(ServerFlags::Stationary, true);
	newItem.css.aggro_players = 0;

	CreatureInstance *retPtr = NULL;
	CREATURE_IT it = NPCList.insert(NPCList.end(), CREATURE_PAIR(newItem.CreatureID, newItem));
	if(it != NPCList.end())
	{
		it->second.Instantiate();
		retPtr = &it->second;
	}
	if(retPtr != NULL)
	{
		for(size_t i = 0; i < cdef->DefaultEffects.size(); i++)
			retPtr->_AddStatusList(cdef->DefaultEffects[i], -1);

		int aggro = 0;
		if((SpawnFlags & SpawnPackageDef::FLAG_USABLE) || (cdef->DefHints & CDEF_HINT_USABLE ) || (cdef->DefHints & CDEF_HINT_USABLE_SPARKLY ))
		{
			retPtr->_AddStatusList(StatusEffects::IS_USABLE, -1);
			aggro = 0;
		}
		if(SpawnFlags & SpawnPackageDef::FLAG_FRIENDLY)
		{
			retPtr->Faction = FACTION_PLAYERFRIENDLY;
			retPtr->_AddStatusList(StatusEffects::INVINCIBLE, -1);
			retPtr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
			aggro = 0;
		}
		if(SpawnFlags & SpawnPackageDef::FLAG_FRIENDLY_ATTACK)
		{
			retPtr->Faction = FACTION_PLAYERFRIENDLY;
			retPtr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
			aggro = 0;
		}
		if(SpawnFlags & SpawnPackageDef::FLAG_NEUTRAL)
		{
			retPtr->SetServerFlag(ServerFlags::NeutralInactive, true);
			aggro = 0;
		}
		if(SpawnFlags & SpawnPackageDef::FLAG_ENEMY)
		{
			retPtr->Faction = FACTION_PLAYERHOSTILE;
			aggro = 1;
		}

		//if(cdef->css.eq_appearance[0] != 0)
		//{
			short visw = 0;
			if(SpawnFlags & SpawnPackageDef::FLAG_VISWEAPON_MELEE)
				visw = 1;
			if(SpawnFlags & SpawnPackageDef::FLAG_VISWEAPON_RANGED)
				visw = 2;
			retPtr->css.vis_weapon = visw;
		//}

		if(SpawnFlags & SpawnPackageDef::FLAG_HIDEMAP)
			retPtr->css.hide_minimap = 1;

		if(aggro == 1 && (cdef->css.ai_package[0] == 0 && retPtr->css.ai_package[0] == 0))
			aggro = 0;

		retPtr->css.aggro_players = aggro;
	}

	RebuildNPCList();
	spawnsys.genericSpawns.push_back(retPtr->CreatureID);

	return retPtr;
}

CreatureInstance* ActiveInstance :: SpawnAtProp(int CDefID, int PropID, int duration, int elevationOffset)
{
	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if(cdef == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] SpawnAtProp() creature def not found: %d", CDefID);
		return NULL;
	}

	/*
	int cdindex = CreatureDef.GetIndex(CDefID);
	if(cdindex == -1)
	{
		g_Log.AddMessageFormat("[ERROR] SpawnAtProp() creature def not found: %d", CDefID);
		return;
	}
	*/

	int x = 0, y = 0, z = 0;
	g_SceneryManager.GetThread("ActiveInstance::SpawnAtProp");
	//SceneryObject *so = g_SceneryManager.GetPropPtr(mZone, PropID, NULL);
	SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(mZone, PropID, NULL);
	if(so != NULL)
	{
		x = (int)so->LocationX;
		y = (int)so->LocationY;
		z = (int)so->LocationZ;
	}
	g_SceneryManager.ReleaseThread();
	if(so == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] SpawnAtProp() prop not found: %d", PropID);
		return NULL;
	}

	CreatureInstance newItem;
	newItem.CreatureDefID = CDefID;
	newItem.CreatureID = GetNewActorID();
	newItem.actInst = this;
	newItem.css.CopyFrom(&cdef->css);
	newItem.CurrentX = x;
	newItem.CurrentY = y + elevationOffset;
	newItem.CurrentZ = z;
	newItem.BuildZoneString(mInstanceID, mZone, 0);
	newItem.css.health = newItem.GetMaxHealth(false);
	newItem.css.aggro_players = 1;
	newItem.SetServerFlag(ServerFlags::IsNPC, true);
	newItem.SetServerFlag(ServerFlags::LocalActive, true);
	newItem.tetherNodeX = newItem.CurrentX;
	newItem.tetherNodeZ = newItem.CurrentZ;
#ifndef CREATUREMAP
	NPCList.push_back(newItem); 
	CreatureInstance *add = &NPCList.back();
#else
	CREATURE_IT it = NPCList.insert(NPCList.end(), CREATURE_PAIR(newItem.CreatureID, newItem));
	CreatureInstance *add = &it->second;
#endif
	add->Instantiate();
	if(duration > 0)
	{
		add->_AddStatusList(StatusEffects::DEAD, -1);
		add->deathTime = g_ServerTime + duration; 
		add->SetServerFlag(ServerFlags::TriggerDelete, true);
	}

	RebuildNPCList();

	int size = PrepExt_GeneralMoveUpdate(GSendBuf, add);
	LSendToLocalSimulator(GSendBuf, size, x, z);
	return add;
}


void ActiveInstance :: EraseAllCreatureReference(CreatureInstance *object)
{


	object->UnloadResources();
	pendingOperations.UpdateList_Remove(object);
	pendingOperations.DeathList_Remove(object);
	EraseIndividualReference(object);
	if(pendingOperations.Debug_HasCreature(object))
	{
		pendingOperations.UpdateList_Remove(object);
		pendingOperations.DeathList_Remove(object);
	};
}

void ActiveInstance :: RunDeath(CreatureInstance *object)
{
	if(nutScriptPlayer != NULL) {
		std::vector<CreatureInstance*>::iterator it = std::find(PlayerListPtr.begin(), PlayerListPtr.end(), object);
		std::vector<ScriptCore::ScriptParam> p;
		p.push_back(object->CreatureID);
		if(it != PlayerListPtr.end()) {
			nutScriptPlayer->JumpToLabel("on_player_death", p);
			QuestScript::QuestNutPlayer *nut = GetSimulatorQuestNutScript(object->simulatorPtr);
			if(nut != NULL) {
				nut->JumpToLabel("on_player_death", p);
			}
		}
		else {
			nutScriptPlayer->JumpToLabel("on_death", p);
			QuestScript::QuestNutPlayer *nut = GetSimulatorQuestNutScript(object->simulatorPtr);
			if(nut != NULL) {
				nut->JumpToLabel("on_death", p);
			}
		}

	}
}

int ActiveInstance :: EraseIndividualReference(CreatureInstance *object)
{
	//Search for a creature instance and remove all possible references to it
	if(PlayerList.size() != PlayerListPtr.size())
		g_Log.AddMessageFormat("[CRITICAL] PlayerList mismatch: %d, %d", PlayerList.size(), PlayerListPtr.size());
	if(NPCList.size() != NPCListPtr.size())
		g_Log.AddMessageFormat("[CRITICAL] NPCList mismatch: %d, %d", NPCList.size(), NPCListPtr.size());
	if(SidekickList.size() != SidekickListPtr.size())
		g_Log.AddMessageFormat("[CRITICAL] SidekickList mismatch: %d, %d", SidekickList.size(), SidekickListPtr.size());

	int rcount = 0;
	size_t a;
	for(a = 0; a < PlayerListPtr.size(); a++)
		rcount += PlayerListPtr[a]->RemoveCreatureReference(object);
#ifdef CREATUREQAV
	for(a = 0; a < NPCListPtr.size(); a++)
		rcount += NPCListPtr[a]->RemoveCreatureReference(object);
#else
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
		it->second.RemoveCreatureReference(ref);
#endif
	for(a = 0; a < SidekickListPtr.size(); a++)
		rcount += SidekickListPtr[a]->RemoveCreatureReference(object);

	return rcount;
}

int ActiveInstance :: RemoveNPCInstance(int CreatureID)
{
#ifndef CREATUREMAP
	list<CreatureInstance>::iterator it;
#else
	CREATURE_IT it;
#endif
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
	{
#ifndef CREATUREMAP
		CreatureInstance *ptr = &*it;
#else
		CreatureInstance *ptr = &it->second;
#endif
		if(ptr->CreatureID == CreatureID)
		{
			int size = PrepExt_RemoveCreature(GSendBuf, CreatureID);
			LSendToLocalSimulator(GSendBuf, size, ptr->CurrentX, ptr->CurrentZ, -1);
			EraseAllCreatureReference(ptr);
			NPCList.erase(it);
			RebuildNPCList();
			return 1;
		}
	}
	//g_Log.AddMessageFormatW(MSG_ERROR, "Failed to delete creature: %d", CreatureID);
	return 0;
}

void ActiveInstance :: CreatureDelete(int CreatureID)
{
	// Resulting code for query: creature.delete
	RemoveNPCInstance(CreatureID);
	int size = PrepExt_RemoveCreature(GSendBuf, CreatureID);
	LSendToAllSimulator(GSendBuf, size, -1);
}

void ActiveInstance :: UnHate(int CreatureDefID)
{
	for(size_t i = 0; i < PlayerListPtr.size(); i++)
		if(PlayerListPtr[i]->CurrentTarget.hasTargetCDef(CreatureDefID) && PlayerListPtr[i]->HasStatus(StatusEffects::PVPABLE))
			PlayerListPtr[i]->SelectTarget(NULL);

	for(size_t i = 0; i < NPCListPtr.size(); i++)
	{
		if(NPCListPtr[i]->CurrentTarget.hasTargetCDef(CreatureDefID))
		{
			NPCListPtr[i]->SelectTarget(NULL);
			NPCListPtr[i]->SetAutoAttack(NULL, 0);
			NPCListPtr[i]->SetServerFlag(ServerFlags::HateInfoChanged, true);
		}
	}

	for(size_t i = 0; i < SidekickListPtr.size(); i++)
		if(SidekickListPtr[i]->CurrentTarget.hasTargetCDef(CreatureDefID))
		{
			SidekickListPtr[i]->SelectTarget(NULL);
			SidekickListPtr[i]->SetAutoAttack(NULL, 0);
		}

	hateProfiles.UnHate(CreatureDefID);
}

ActiveInstanceManager :: ActiveInstanceManager()
{
	Clear();
}

ActiveInstanceManager :: ~ActiveInstanceManager()
{
	Clear();
}

void ActiveInstanceManager :: Clear(void)
{
	NextInstanceID = BASE_INSTANCE_ID;
	nextInstanceCheck = 0;

	for(size_t i = 0; i < instListPtr.size(); i++)
		instListPtr[i]->Clear();

	instList.clear();
	instListPtr.clear();
}

ActiveInstance * ActiveInstanceManager :: GetPtrByZoneOwner(int zoneID, int ownerDefID)
{
	list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		if((it->mZone == zoneID) && (it->mOwnerCreatureDefID == ownerDefID))
			return &*it;
	}
	return NULL;
}

ActiveInstance * ActiveInstanceManager :: GetPtrByZoneInstanceID(int zoneID, int instanceID)
{
	//Searches the list of instances to find an active instance.
	list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		if((zoneID >= 0) && (it->mZone != zoneID))  //Negative zone IDs allow searching for a specific ID instead.
			continue;

		if(it->mInstanceID == instanceID || instanceID == 0)
			return &*it;
	}
	return NULL;
}

ActiveInstance * ActiveInstanceManager :: GetPtrByZonePartyID(int zoneID, int partyID)
{
	//Searches the list of instances to a zone that is registered to a particular party.
	list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		if((it->mZone == zoneID) && (it->mOwnerPartyID == partyID))
			return &*it;
	}
	return NULL;
}

ActiveInstance * ActiveInstanceManager :: GetPtrByZoneID(int zoneID)
{
	//Searches the list of instances to find an active instance matching a zone.
	list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		if(it->mZone == zoneID)
			return &*it;
	}
	return NULL;
}

void ActiveInstanceManager :: RebuildAccessList(void)
{
	list<ActiveInstance>::iterator it;
	instListPtr.clear();

	for(it = instList.begin(); it != instList.end(); ++it)
		instListPtr.push_back(&*it);
}

int ActiveInstanceManager :: GetNewInstanceID(void)
{
	//Search for an unused instance ID.
	bool found = 1;
	while(found == 1)
	{
		if(NextInstanceID++ >= MAX_INSTANCE_ID)
			NextInstanceID = BASE_INSTANCE_ID;
		found = 0;
		for(size_t i = 0; i < instListPtr.size(); i++)
		{
			if(instListPtr[i]->mInstanceID == NextInstanceID)
			{
				found = 1;
				break;
			}
		}
	}
	return NextInstanceID;
}

ActiveInstance * ActiveInstanceManager :: CreateInstance(int zoneID, PlayerInstancePlacementData &pd)
{
	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(zoneID);
	if(zoneDef == NULL)
		return NULL;

	ActiveInstance newItem;
	newItem.mInstanceID = GetNewInstanceID();
	newItem.mMode = zoneDef->mMode;
	newItem.mZone = zoneDef->mID;
	newItem.mZoneDefPtr = zoneDef;
	newItem.dropRateProfile = &zoneDef->GetDropRateProfile();
	newItem.mLastHealthUpdate = g_ServerTime;
	
	if(pd.in_scaleProfile != NULL && zoneDef->IsMobScalable() == true)
	{
		int playerLevel = pd.in_playerLevel;
		if(pd.in_partyID != 0)
		{
			ActiveParty *party = g_PartyManager.GetPartyByID(pd.in_partyID);
			if(party != NULL)
			{
				int partyLevel = party->GetMaxPlayerLevel();
				if(partyLevel > 0)
					playerLevel = partyLevel;
			}
		}
		newItem.scaleConfig.mPLayerLevel = playerLevel;
		newItem.scaleConfig.mIsScaled = true;
		newItem.scaleProfile = pd.in_scaleProfile;
		if(newItem.scaleProfile->mDropRateProfileName.size() > 0)
			newItem.dropRateProfile = &g_DropRateProfileManager.GetProfileByName(newItem.scaleProfile->mDropRateProfileName);
	}
	
	cs.Enter("ActiveInstanceManager::CreateInstance");
	instList.push_back(newItem);
	RebuildAccessList();
	cs.Leave();

	ActiveInstance *retPtr = &instList.back();
	retPtr->InitializeData();
	return retPtr;
}

void ActiveInstance :: AdjustPlayerCount(int offset)
{
	mPlayers += offset;
	if(mPlayers <= 0)
		mExpireTime = g_ServerTime + INSTANCE_DELETE_MAXTIME;
}

//This function should be called after the basic instance parameters are set up.
//This will load additional zone resources and configuration objects that are not
//directly tied to the ZoneDef.
void ActiveInstance :: InitializeData(void)
{
	//Important, need to set base pointer for the spawning system trackbacks.
	spawnsys.SetInstancePointer(this);

	//Load the new script system
	std::string path = InstanceScript::InstanceNutDef::GetInstanceScriptPath(mZoneDefPtr->mID, false, mZoneDefPtr->mGrove);
	if(Util::HasEnding(path, ".nut")) {
		nutScriptDef.Initialize(path.c_str());
		nutScriptPlayer = new InstanceScript::InstanceNutPlayer();
		nutScriptPlayer->SetInstancePointer(this);
		std::string errors;
		nutScriptPlayer->Initialize(&nutScriptDef, errors);
		if(errors.length() > 0)
			g_Log.AddMessageFormat("Failed to compile. %s", errors.c_str());
	}
	else if(Util::HasEnding(path, ".txt")) {
		scriptDef.CompileFromSource(path.c_str());
		scriptPlayer = new InstanceScript::InstanceScriptPlayer();
		scriptPlayer->Initialize(&scriptDef);
		scriptPlayer->SetInstancePointer(this);
		scriptPlayer->JumpToLabel("init");
	}
	else {
		g_Log.AddMessageFormat("No Squirrel script for instance %d", mZone);
	}


	/*  log debug disassembly
	FILE *output = fopen("instance_disassembly.txt", "wb");
	scriptPlayer.def->OutputDisassemblyToFile(output);
	fclose(output);
	*/

	//Load essence shop data.
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "%s\\%d\\EssenceShop.txt", mZoneDefPtr->mGrove ? "Grove" : "Instance", mZone);
	Platform::FixPaths(buffer);
	essenceShopList.LoadFromFile(buffer);
	if(essenceShopList.EssenceShopList.size() > 0)
		g_Log.AddMessageFormatW(MSG_DIAG, "Loaded %d essence shops.", essenceShopList.EssenceShopList.size());

	Util::SafeFormat(buffer, sizeof(buffer), "%s\\%d\\Shop.txt", mZoneDefPtr->mGrove ? "Grove" : "Instance", mZone);
	Platform::FixPaths(buffer);
	itemShopList.LoadFromFile(buffer);
	if(itemShopList.EssenceShopList.size() > 0)
		g_Log.AddMessageFormatW(MSG_DIAG, "Loaded %d item shops.", itemShopList.EssenceShopList.size());


	Util::SafeFormat(buffer, sizeof(buffer), "%s\\%d\\Static.txt", mZoneDefPtr->mGrove ? "Grove" : "Instance", mZone);
	Platform::FixPaths(buffer);
	LoadStaticObjects(buffer);

	//World markers (for devs)
	Util::SafeFormat(buffer, sizeof(buffer), "%s\\%d\\WorldMarkers.txt", mZoneDefPtr->mGrove ? "Grove" : "Instance", mZone);
	Platform::FixPaths(buffer);
	worldMarkers.LoadFromFile(buffer);
	if(worldMarkers.WorldMarkerList.size() > 0)
		g_Log.AddMessageFormatW(MSG_DIAG, "Loaded %d world markers.", worldMarkers.WorldMarkerList.size());

	//retPtr->SetScaleConfig(scaleConfigSetting, 47);
	arenaRuleset.mPVPStatus = mZoneDefPtr->mMode;
	if(mZoneDefPtr->mArena == true)
	{
		arenaRuleset.DebugInit();
	}
	if(mZoneDefPtr->mMode != PVP::GameMode::PVE_ONLY) {
		arenaRuleset.mEnabled = true;
	}
}



// Check if this instance meets the conditions for being unloaded from memory.
bool ActiveInstance :: QualifyDelete(void)
{
	if(mPlayers > 0)
		return false;
	if(mZoneDefPtr->mPersist == true)
		return false;
	if(g_ServerTime < mExpireTime)
		return false;

	return true;
}

CreatureInstance* ActiveInstance :: InstantiateSidekick(CreatureInstance *host, SidekickObject &skobj, int count)
{
	//Generates a new creature instance of a sidekick into this zone.
	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(skobj.CDefID);
	if(cdef == NULL)
		return NULL;

	CreatureInstance newItem;

	newItem.actInst = this;
	newItem.Faction = host->Faction;
	newItem.BuildZoneString(mInstanceID, mZone, 0);

	newItem.CreatureDefID = cdef->CreatureDefID;
	newItem.CreatureID = GetNewActorID();

	newItem.css.CopyFrom(&cdef->css);
	
	/*
	int resistBonus = (((50 - newItem.css.level) + host->css.level) / 2) * 10;
	newItem.css.dr_mod_melee = resistBonus;
	newItem.css.dr_mod_fire = resistBonus;
	newItem.css.dr_mod_frost = resistBonus;
	newItem.css.dr_mod_mystic = resistBonus;
	newItem.css.dr_mod_death = resistBonus;
	*/

	newItem.CurrentX = host->CurrentX;
	newItem.CurrentY = host->CurrentY;
	newItem.CurrentZ = host->CurrentZ;
	newItem.AnchorObject = host;

	newItem.movementTime = 0;
	newItem.Rotation = 255;
	newItem.Heading = 255;
	newItem.css.aggro_players = 0;
	newItem.SetServerFlag(ServerFlags::IsSidekick, true);

	if(skobj.summonType == SidekickObject::ABILITY)
		newItem.CAF_RunSidekickStatFilter(skobj.summonParam);
	else if(skobj.summonType == SidekickObject::QUEST)
		newItem.CAF_RunSidekickStatFilter(skobj.summonParam);
	else if(skobj.summonType == SidekickObject::PET)
		newItem.SetServerFlag(ServerFlags::Noncombatant, true);
	
	skobj.CID = newItem.CreatureID;

	SidekickList.push_back(newItem);
	SidekickList.back().Instantiate();
	SidekickList.back()._AddStatusList(StatusEffects::INVINCIBLE, -1);
	SidekickList.back()._AddStatusList(StatusEffects::UNATTACKABLE, -1);

	return &SidekickList.back();
}

int ActiveInstance :: CreateSidekick(CreatureInstance* host, SidekickObject &skobj)
{
	if(skobj.CDefID <= 0)
		return -1;

	CreatureInstance *cInst = InstantiateSidekick(host, skobj, 0);

	if(cInst == NULL)
		return -1;

	RebuildSidekickList();
	return cInst->CreatureID;
}

int ActiveInstance :: SidekickRemove(CreatureInstance* host, vector<SidekickObject> *sidekickList, int CID)
{
	int size = 0;
	list<CreatureInstance>::iterator it;
	for(it = SidekickList.begin(); it != SidekickList.end(); ++it)
	{
		if(it->CreatureID == CID)
		{
			for(size_t i = 0; i < sidekickList->size(); i++)
			{
				if(sidekickList->at(i).CID == it->CreatureID)
				{
					sidekickList->erase(sidekickList->begin() + i);
					break;
				}
			}
			size = PrepExt_RemoveCreature(GAuxBuf, CID);
			LSendToLocalSimulator(GAuxBuf, size, host->CurrentX, host->CurrentZ);
			EraseAllCreatureReference(&*it);
			SidekickList.erase(it);
			RebuildSidekickList();
			return 1;
		}
	}
	return -1;
}

int ActiveInstance :: SidekickRemoveOne(CreatureInstance* host, vector<SidekickObject> *sidekickList)
{
	//Removes a single sidekick instance from the host.  The host must currently have
	//the sidekick selected.
	//Use the GAuxBuf since RemoveAllCreatureReference might use GSendBuf
	//to cancel pending abilities.
	int size = 0;
	list<CreatureInstance>::iterator it;
	for(it = SidekickList.begin(); it != SidekickList.end(); ++it)
	{
		if(it->AnchorObject == host)
			if(&*it == host->CurrentTarget.targ)
			{
				for(size_t i = 0; i < sidekickList->size(); i++)
				{
					if(sidekickList->at(i).CDefID == it->CreatureDefID)
					{
						sidekickList->erase(sidekickList->begin() + i);
						break;
					}
				}

				size = PrepExt_RemoveCreature(GAuxBuf, it->CreatureID);
				LSendToLocalSimulator(GAuxBuf, size, host->CurrentX, host->CurrentZ);

				EraseAllCreatureReference(&*it);

				SidekickList.erase(it);
				RebuildSidekickList();
				return 1;
			}
	}
	return -1;
}

int ActiveInstance :: SidekickRemoveAll(CreatureInstance* host, vector<SidekickObject> *sidekickList)
{
	//Removes all sidekicks from a host.
	//Use the GAuxBuf since RemoveAllCreatureReference might use GSendBuf
	//to cancel pending abilities.
	size_t size = 0;
	bool bActive = false;
	list<CreatureInstance>::iterator it;
	int debug_iter = 0;
	do
	{
		bActive = false;
		for(it = SidekickList.begin(); it != SidekickList.end(); ++it)
		{
			if(it->AnchorObject == host)
			{
				size += PrepExt_RemoveCreature(&GAuxBuf[size], it->CreatureID);
				if(size > sizeof(GAuxBuf) - 100)
				{
					LSendToLocalSimulator(GAuxBuf, size, host->CurrentX, host->CurrentZ);
					size = 0;
				}

				EraseAllCreatureReference(&*it);

				SidekickList.erase(it);
				RebuildSidekickList();
				bActive = true;
				debug_iter++;
				break;
			}
		}
	} while(bActive == true);

	if(size > 0)
		LSendToLocalSimulator(GAuxBuf, size, host->CurrentX, host->CurrentZ);

	sidekickList->clear();

	g_Log.AddMessageFormatW(MSG_DIAG, "Debug: iterations: %d", debug_iter);
	return 1;
}

int ActiveInstance :: SidekickRegister(CreatureInstance* host, vector<SidekickObject> *sidekickList)
{
	//Registers a list of sidekicks into the active instance.  Should be called
	//whenever a player is placed into an instance.
	int count = 0;
	for(size_t i = 0; i < sidekickList->size(); i++)
	{
		if(InstantiateSidekick(host, sidekickList->at(i), count) != NULL)
		{
			g_Log.AddMessageFormatW(MSG_DIAGV, "Registering sidekick: %d", sidekickList->at(i).CDefID);
			count++;
		}
	}

	g_Log.AddMessageFormatW(MSG_DIAG, "Registered %d sidekicks", count);

	if(count > 0)
	{
		RebuildSidekickList();
		/*
		for(a = 0; a < (int)SidekickListPtr.size(); a++)
		{
			if(SidekickListPtr[a]->AnchorObject == host)
			{
				int size = PrepExt_AddParty(GSendBuf, SidekickListPtr[a]);
				LSendToAllSimulator(GSendBuf, size, -1);
			}
		}*/
		return 1;
	}

	return -1;
}

int ActiveInstance :: SidekickUnregister(CreatureInstance* host)
{
	//Unregisters a list of sidekicks from the active instance.  Should be called
	//whenever a player is removed from an instance.
	uint size = 0;
	bool bActive = false;
	list<CreatureInstance>::iterator it;
	do
	{
		bActive = false;
		for(it = SidekickList.begin(); it != SidekickList.end(); ++it)
		{
			if(it->AnchorObject == host)
			{
				size += PrepExt_RemoveCreature(&GAuxBuf[size], it->CreatureID);
				if(size > sizeof(GAuxBuf) - 100)
				{
					LSendToLocalSimulator(GAuxBuf, size, host->CurrentX, host->CurrentZ);
					size = 0;
				}

				EraseAllCreatureReference(&*it);

				SidekickList.erase(it);
				RebuildSidekickList();
				bActive = true;
				break;
			}
		}
	} while(bActive == true);

	if(size > 0)
		LSendToLocalSimulator(GAuxBuf, size, host->CurrentX, host->CurrentZ);


	//sidekickList->clear();
	return 1;
}

void ActiveInstance :: SidekickAttack(CreatureInstance* host)
{
	//All sidekicks registered with the given host will assume its current target.
	size_t a;
	for(a = 0; a < SidekickListPtr.size(); a++)
		if(SidekickListPtr[a]->AnchorObject == host)
			if(!(SidekickListPtr[a]->serverFlags & ServerFlags::Noncombatant))
				SidekickListPtr[a]->SelectTarget(host->CurrentTarget.targ);
}


void ActiveInstance :: SidekickCall(CreatureInstance* host)
{
	static int MED_SCATTER_RANGE = 20;
	size_t a;
	for(a = 0; a < SidekickListPtr.size(); a++)
		if(SidekickListPtr[a]->AnchorObject == host)
		{
			SidekickListPtr[a]->SelectTarget(NULL);
			SidekickListPtr[a]->movementTime = g_ServerTime;
			SidekickListPtr[a]->SetServerFlag(ServerFlags::CalledBack, true);
			//Replace the current destination.
			SidekickListPtr[a]->CurrentTarget.DesLocX = host->CurrentX + randint(-MED_SCATTER_RANGE, MED_SCATTER_RANGE);
			SidekickListPtr[a]->CurrentTarget.DesLocZ = host->CurrentZ + randint(-MED_SCATTER_RANGE, MED_SCATTER_RANGE);
		}
}

void ActiveInstance :: SidekickWarp(CreatureInstance *host)
{
	static int SCATTER_RANGE = 50;
	size_t size = 0;
	size_t a;
	for(a = 0; a < SidekickListPtr.size(); a++)
		if(SidekickListPtr[a]->AnchorObject == host)
		{
			SidekickListPtr[a]->movementTime = g_ServerTime;
			SidekickListPtr[a]->CurrentX = host->CurrentX;
			SidekickListPtr[a]->CurrentY = host->CurrentY;
			SidekickListPtr[a]->CurrentZ = host->CurrentZ;
			if(SidekickListPtr[a]->NotStatus(StatusEffects::DEAD) == true)
			{
				SidekickListPtr[a]->CurrentX += randint(-SCATTER_RANGE, SCATTER_RANGE);
				SidekickListPtr[a]->CurrentZ += randint(-SCATTER_RANGE, SCATTER_RANGE);
			}
			//SidekickListPtr[a]->CurrentTarget.targ = NULL;
			SidekickListPtr[a]->SelectTarget(NULL);
			SidekickListPtr[a]->CurrentTarget.DesLocX = 0;
			SidekickListPtr[a]->CurrentTarget.DesLocZ = 0;
			SidekickListPtr[a]->Speed = 0;
			size += PrepExt_GeneralMoveUpdate(&GSendBuf[size], SidekickListPtr[a]);
			if(size > sizeof(GSendBuf) - 1000)
			{
				LSendToLocalSimulator(GSendBuf, size, host->CurrentX, host->CurrentZ);
				size = 0;
			}
		}

	if(size > 0)
		LSendToLocalSimulator(GSendBuf, size, host->CurrentX, host->CurrentZ);
}

void ActiveInstance :: SidekickScatter(CreatureInstance* host)
{
	static int SCATTER_RANGE = 50;
	int x = 0;
	int z = 0;
	size_t a;
	for(a = 0; a < SidekickListPtr.size(); a++)
	{
		if(SidekickListPtr[a]->AnchorObject == host)
		{
			if(SidekickListPtr[a]->CurrentTarget.targ != NULL)
			{
				x = SidekickListPtr[a]->CurrentTarget.targ->CurrentX;
				z = SidekickListPtr[a]->CurrentTarget.targ->CurrentZ;
			}
			else
			{
				x = SidekickListPtr[a]->AnchorObject->CurrentX;
				z = SidekickListPtr[a]->AnchorObject->CurrentZ;
			}
			SidekickListPtr[a]->CurrentTarget.DesLocX = x + randint(-SCATTER_RANGE, SCATTER_RANGE);
			SidekickListPtr[a]->CurrentTarget.DesLocZ = z + randint(-SCATTER_RANGE, SCATTER_RANGE);
			SidekickListPtr[a]->movementTime = g_ServerTime;
		}
	}
}

int ActiveInstance :: SidekickLow(CreatureInstance* host, int percent)
{
	int lowest = -1;
	int lowestpc = percent;
	size_t a;
	for(a = 0; a < SidekickListPtr.size(); a++)
	{
		if(SidekickListPtr[a]->AnchorObject == host)
		{
			if(SidekickListPtr[a]->NotStatus(StatusEffects::DEAD) == true)
			{
				int maxhealth = SidekickListPtr[a]->GetMaxHealth(true);
				int hpercent = (int)(((float)SidekickListPtr[a]->css.health / (float)maxhealth) * 100);
				if(hpercent < lowestpc)
				{
					lowestpc = hpercent;
					lowest = a;
				}
			}
		}
	}
	if(lowest >= 0)
	{
		//host->CurrentTarget.targ = SidekickListPtr[lowest];
		host->SelectTarget(SidekickListPtr[lowest]);
		return 1;
	}
	return -1;
}

int ActiveInstance :: SidekickParty(CreatureInstance* host, char *outBuf)
{
	int wpos = 0;
	wpos += PutByte(&outBuf[wpos], 6);  //_handlePartyUpdateMsg
	wpos += PutShort(&outBuf[wpos], 0);

	wpos += PutByte(&outBuf[wpos], 5);  //Joined Party

	wpos += PutByte(&outBuf[wpos], 0);  //Party Size

	wpos += PutInteger(&outBuf[wpos], host->CreatureID); //leader
	wpos += PutInteger(&outBuf[wpos], 1); //member id

	int size = 0;
	int a;
	for(a = 0; a < (int)SidekickListPtr.size(); a++)
	{
		if(SidekickListPtr[a]->AnchorObject == host)
		{
			/*
			int len = strlen(SidekickListPtr[a]->css.display_name);
			if(len > sizeof(Name) - 4)
				len = sizeof(Name) - 4;
			strncpy(Name, SidekickListPtr[a]->css.display_name, len);
			sprintf(&Name[len], "%d", size + 1);
			*/

			wpos += PutInteger(&outBuf[wpos], SidekickListPtr[a]->CreatureID); //id
			wpos += PutStringUTF(&outBuf[wpos], SidekickListPtr[a]->css.display_name);
			size++;
		}
	}

	PutShort(&outBuf[1], wpos - 3);       //Set message size
	PutByte(&outBuf[4], size);
	return wpos;
}

int ActiveInstance :: DetachSceneryEffect(char *outBuf, int sceneryId, int effectType, int tag)
{
	int wpos = 0;
	wpos += PutByte(&outBuf[wpos], 98);       //_handleInfoMsg
	wpos += PutShort(&outBuf[wpos], 0);      //Placeholder for size

	wpos += PutByte(&outBuf[wpos], 2);
	wpos += PutInteger(&outBuf[wpos], sceneryId);
	wpos += PutInteger(&outBuf[wpos], effectType);
	wpos += PutInteger(&outBuf[wpos], tag);
	PutShort(&outBuf[1], wpos - 3);
	return wpos;
}

int ActiveInstance :: Shake(char *outbuf, float amount, float time, float range) {
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 97);       //_handleShake
	wpos += PutShort(&outbuf[wpos], 0);      //Placeholder for size
	wpos += PutFloat(&outbuf[wpos], amount);
	wpos += PutFloat(&outbuf[wpos], time);
	wpos += PutFloat(&outbuf[wpos], range);
	PutShort(&outbuf[1], wpos - 3);
	return wpos;
}

int ActiveInstance :: AddSceneryEffect(char *outbuf, SceneryEffect *effect)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 98);       //_handleSceneryEffectMsg
	wpos += PutShort(&outbuf[wpos], 0);      //Placeholder for size

	wpos += PutByte(&outbuf[wpos], 1); //Event
	wpos += PutInteger(&outbuf[wpos], effect->propID);
	wpos += PutInteger(&outbuf[wpos], effect->type);
	wpos += PutInteger(&outbuf[wpos], effect->tag);
	wpos += PutStringUTF(&outbuf[wpos], effect->effect);
	wpos += PutFloat(&outbuf[wpos], effect->offsetX);
	wpos += PutFloat(&outbuf[wpos], effect->offsetY);
	wpos += PutFloat(&outbuf[wpos], effect->offsetZ);
	wpos += PutFloat(&outbuf[wpos], effect ->scale);
	PutShort(&outbuf[1], wpos - 3);

	return wpos;
}

SceneryEffect* ActiveInstance :: RemoveSceneryEffect(int PropID, int tag)
{
	SceneryEffectMap::iterator it = mSceneryEffects.find(PropID);
	if(it != mSceneryEffects.end()) {
		for(std::vector<SceneryEffect>::size_type i = 0; i != it->second.size(); i++)
		{
			SceneryEffect *sceneryEffect = &it->second[i];
			if(sceneryEffect->tag == tag)
			{
				it->second.erase(it->second.begin() + i);
				if(it->second.size() == 0)
					mSceneryEffects.erase(i);
				return sceneryEffect;
			}
		}
	}
	return NULL;
}

SceneryEffectList* ActiveInstance :: GetSceneryEffectList(int PropID)
{
	SceneryEffectMap::iterator it = mSceneryEffects.find(PropID);
	if(it == mSceneryEffects.end()) {
		return &mSceneryEffects.insert(SceneryEffectMap::value_type(PropID, SceneryEffectList())).first->second;
	}
	return &it->second;
}

int ActiveInstance :: PartyAll(CreatureInstance* host, char *outBuf)
{
	int wpos = 0;
	wpos += PutByte(&outBuf[wpos], 6);  //_handlePartyUpdateMsg
	wpos += PutShort(&outBuf[wpos], 0);

	wpos += PutByte(&outBuf[wpos], 5);  //Joined Party

	wpos += PutByte(&outBuf[wpos], 0);  //Party Size

	wpos += PutInteger(&outBuf[wpos], host->CreatureID); //leader
	wpos += PutInteger(&outBuf[wpos], 1); //member id

	int size = 0;
	int a;
	for(a = 0; a < (int)PlayerListPtr.size(); a++)
	{
		wpos += PutInteger(&outBuf[wpos], PlayerListPtr[a]->CreatureID); //id
		wpos += PutStringUTF(&outBuf[wpos], PlayerListPtr[a]->css.display_name);
		size++;
	}
	PutShort(&outBuf[1], wpos - 3);       //Set message size
	PutByte(&outBuf[4], size);
	return wpos;
}

void ActiveInstance :: UpdateSidekickTargets(CreatureInstance *officer)
{
	//Updates all Sidekicks under the Officer with the Officer's target.
	if(officer->CurrentTarget.targ == NULL)
		return;
	if(officer->_ValidTargetFlag(officer->CurrentTarget.targ, TargetStatus::Enemy_Alive) == false)
		return;

	size_t a;
	for(a = 0; a < SidekickListPtr.size(); a++)
	{
		if(SidekickListPtr[a]->AnchorObject == officer)
		{
			if(!(SidekickListPtr[a]->serverFlags & ServerFlags::Noncombatant))
				SidekickListPtr[a]->SelectTarget(officer->CurrentTarget.targ);
		}
	}
}

void ActiveInstance :: RebuildSidekickList(void)
{
	//Should only be called by the thread running actions for this instance.
	//Need to rebuild the array of quick-access pointers.
	SidekickListPtr.clear();
	list<CreatureInstance>::iterator it;
	for(it = SidekickList.begin(); it != SidekickList.end(); ++it)
		SidekickListPtr.push_back(&*it);
}

CreatureInstance * ActiveInstance :: inspectCreature(int CreatureID)
{
	TIMETRACK("ActiveInstance::inspectCreature");
	TIMETRACKP(CreatureID, NPCList.size());

	size_t a;
	for(a = 0; a < PlayerListPtr.size(); a++)
		if(PlayerListPtr[a]->CreatureID == CreatureID)
			return PlayerListPtr[a];

#ifndef CREATUREMAP
	for(a = 0; a < NPCListPtr.size(); a++)
		if(NPCListPtr[a]->CreatureID == CreatureID)
			return NPCListPtr[a];
#else
	CREATURE_IT it;
	it = NPCList.find(CreatureID);
	if(it != NPCList.end())
		return &it->second;
#endif

	for(a = 0; a < SidekickListPtr.size(); a++)
		if(SidekickListPtr[a]->CreatureID == CreatureID)
			return SidekickListPtr[a];

	g_Log.AddMessageFormat("[WARNING] inspectCreature failed for ID [%d]", CreatureID);

	TIMETRACKF(1);
	return NULL;
}

void ActiveInstance :: RunScripts(void)
{
	if(questScriptList.empty() == false)
	{
		std::list<QuestScript::QuestScriptPlayer>::iterator it;
		it = questScriptList.begin();
		while(it != questScriptList.end())
		{
			if(it->mExecuting == true)
			{
				it->RunSingleInstruction();
				++it;
			}
			else
				questScriptList.erase(it++);
		}
	}

	if(questNutScriptList.empty() == false)
	{
		/* A for loop is used here as the script list may change while the queue is
		 * being execute (e.g. a halt instruction, or new quest opened).
		 */
		for(uint i = 0 ; i < questNutScriptList.size(); i++) {
			QuestScript::QuestNutPlayer * p = questNutScriptList[i];
			if(p->mActive)
				p->ExecQueue();
			else
				g_QuestNutManager.RemoveActiveScript(p);
		}
	}


	/* DISABLED, NOT FINISHED
	if(mConcurrentInstanceScripts.empty() == false)
	{
		std::list<InstanceScript::ScriptPlayer>::iterator it;
		it = mConcurrentInstanceScripts.begin();
		while(it != mConcurrentInstanceScripts.end())
		{
			if(it->active == true)
			{
				it->RunInstruction();
				++it;
			}
			else
				mConcurrentInstanceScripts.erase(it++);
		}
	}
	*/
}

/* DISABLED, NOT FINISHED
void ActiveInstance :: AttachConcurrentInstanceScript(const char *label)
{
	InstanceScript::ScriptPlayer newScript;
	mConcurrentInstanceScripts.push_back(newScript);
	
	std::list<InstanceScript::ScriptPlayer>::iterator it;
	it = mConcurrentInstanceScripts.back();
	if(it->active == false)
	{
		if(label == NULL)
			label = "init";

		it->Initialize(&scriptDef);
		it->SetInstancePointer(this);
		it->JumpToLabel(label);
	}
}
*/


void ActiveInstance :: SendLoyaltyAggro(CreatureInstance *instigator, CreatureInstance *target, int loyaltyRadius)
{
	if(instigator->HasStatus(StatusEffects::DEAD))
		return;
	if(instigator->HasStatus(StatusEffects::UNATTACKABLE))
		return;
	if(instigator->HasStatus(StatusEffects::INVINCIBLE))
		return;

	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
	{
		CreatureInstance *source = &it->second;
		if(source->css.health <= 0)
			continue;
		if(source->CurrentTarget.targ != NULL)
			continue;
		if(source->CreatureID == target->CreatureID)
			return;
		if(source->serverFlags & ServerFlags::CalledBack)
			continue;
		if(source->serverFlags & ServerFlags::LeashRecall)
			continue;
		if(source->Faction != target->Faction)
			continue;
		if(abs(target->CurrentX - source->CurrentX) > loyaltyRadius)
			continue;
		if(abs(target->CurrentZ - source->CurrentZ) > loyaltyRadius)
			continue;
		if(abs(target->CurrentY - source->CurrentY) > loyaltyRadius)
			continue;

		source->SelectTarget(instigator);
		source->SetServerFlag(ServerFlags::LocalActive, true);
		g_Log.AddMessageFormat("Loyalty aggro: %s (%d range)", source->css.display_name, loyaltyRadius);
	}
	//g_Log.AddMessageFormat("Loyalty aggro: %d (%d range)", count, loyaltyRadius);
}

void ActiveInstance :: SendLoyaltyLinks(CreatureInstance *instigator, CreatureInstance *target, SceneryObject *spawnPoint)
{
	if(spawnPoint == NULL)
		return;

	std::vector<int> openList;  //Prop IDs that need to be checked.
	std::vector<int> propLinks; //The links returned by a prop.

	//Add the first object to the list so we can begin search.  The prop will be searched
	//for any props it is linked to.  Those linked props will be added to the open list (if
	//they are not already in the open list).  Repeat the search+add phase until every object
	//in the open list has been processed.
	openList.push_back(spawnPoint->ID);
	size_t pos = 0;
	while(pos < openList.size())
	{
		SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(mZone, openList[pos], NULL);
		if(so != NULL)
		{
			so->EnumLinks(SceneryObject::LINK_TYPE_LOYALTY, propLinks);
			for(size_t propi = 0; propi < propLinks.size(); propi++)
			{
				bool found = false;
				for(size_t openi = 0; openi < openList.size(); openi++)
				{
					if(propLinks[propi] == openList[openi])
					{
						found = true;
						break;
					}
				}
				if(found == false)
					openList.push_back(propLinks[propi]);
			}
		}
		pos++;
	}

	//If no other props were added to the open list, there are no loyalty links.
	if(openList.size() <= 1)
		return;

	//Now resolve all props in the open list and make sure they're SpawnPoint props.
	std::vector<int> creatureIDs;
	for(size_t i = 0; i < openList.size(); i++)
	{
		SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(mZone, openList[i], NULL);
		if(so == NULL)
			continue;
		if(so->IsSpawnPoint() == false)
			continue;
		spawnsys.EnumAttachedCreatures(so->ID, (int)so->LocationX, (int)so->LocationZ, creatureIDs);
	}
	
	//Now look up all creatures in the list and pull them if necessary.
	for(size_t i = 0; i < creatureIDs.size(); i++)
	{
		CreatureInstance *source = GetNPCInstanceByCID(creatureIDs[i]);
		if(source == NULL)
			continue;
		if(source->css.health <= 0)
			continue;
		if(source->CurrentTarget.targ != NULL)
			continue;
		if(source->CreatureID == target->CreatureID)
			return;
		if(source->serverFlags & ServerFlags::CalledBack)
			continue;
		if(source->serverFlags & ServerFlags::LeashRecall)
			continue;
		if(source->Faction != target->Faction)
			continue;

		source->SelectTarget(instigator);
		source->SetServerFlag(ServerFlags::LocalActive, true);
	}
}

QuestScript::QuestNutPlayer* ActiveInstance :: GetSimulatorQuestNutScript(SimulatorThread *simulatorPtr)
{
	std::vector<QuestScript::QuestNutPlayer*>::iterator it;
	for(it = questNutScriptList.begin(); it != questNutScriptList.end(); ++it) {
		QuestScript::QuestNutPlayer* q = *it;
		if(q->source != NULL && simulatorPtr == q->source->simulatorPtr)
			return q;
	}
	return NULL;
}

QuestScript::QuestScriptPlayer* ActiveInstance :: GetSimulatorQuestScript(SimulatorThread *simulatorPtr)
{
	std::list<QuestScript::QuestScriptPlayer>::iterator it;
	for(it = questScriptList.begin(); it != questScriptList.end(); ++it)
		if(it->simCall == simulatorPtr)
			return &*it;

	return NULL;
}

void ActiveInstance :: RunProcessingCycle(void)
{
	if(mPlayers == 0)
		return;
	if(g_ServerTime < mNextUpdateTime)
		return;

	mNextUpdateTime = g_ServerTime + InstanceUpdateDelay;

#ifdef DEBUG_TIME
	Debug::TimeTrack("RunProcessingCycle", 100);
#endif

	//scriptPlayer.RunUntilWait();
	if(scriptPlayer != NULL)
		scriptPlayer->RunAtSpeed(25);
	else if(nutScriptPlayer != NULL)
		nutScriptPlayer->Tick();
	SendActors();
	UpdateCreatureLocalStatus();

	for(size_t i = 0; i < PlayerListPtr.size(); i++)
		PlayerListPtr[i]->RunProcessingCycle();

#ifdef CREATUREQAV
	for(size_t i = 0; i < NPCListPtr.size(); i++)
		NPCListPtr[i]->RunProcessingCycle();
#else
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
		it->second.RunProcessingCycle();
#endif

	for(size_t i = 0; i < SidekickListPtr.size(); i++)
		SidekickListPtr[i]->RunProcessingCycle();

	if(g_ServerTime > mNextTargetUpdate)
		mNextTargetUpdate = g_ServerTime + 1000;

	spawnsys.RunProcessing(false);
	RunScripts();
	RemoveDeadCreatures();
}

void ActiveInstance :: SendPlaySound(const char *assetPackage, const char *soundFile)
{
	for(size_t i = 0; i < RegSim.size(); i++)
		if((RegSim[i]->CheckStateGameplayProtocol() == true))
			RegSim[i]->SendPlaySound(assetPackage, soundFile);
}

void ActiveInstance :: UpdateEnvironmentCycle(const char *timeOfDay)
{
	if(timeOfDay == NULL)
		return;
	if(PlayerListPtr.size() == 0)
		return;
	if(mZoneDefPtr->mEnvironmentCycle == false)
		return;
	for(size_t i = 0; i < PlayerListPtr.size(); i++)
		PlayerListPtr[i]->simulatorPtr->SendTimeOfDay(timeOfDay);
}

//Calls a script with a particular kill event.
void ActiveInstance :: ScriptCallKill(int CreatureDefID, int CreatureID)
{
	if(nutScriptPlayer != NULL && nutScriptPlayer->mActive)
	{
		std::vector<ScriptCore::ScriptParam> parms;
		parms.push_back(ScriptCore::ScriptParam(CreatureDefID));
		parms.push_back(ScriptCore::ScriptParam(CreatureID));
		nutScriptPlayer->JumpToLabel("on_kill", parms);

		char buffer[64];
		Util::SafeFormat(buffer, sizeof(buffer), "on_kill_%d", CreatureDefID);
		ScriptCall(buffer);
	}
	else if(scriptPlayer && scriptPlayer->mActive)
	{
		//Checks for a scripted event for a kill of this creature type.

		char buffer[64];

		Util::SafeFormat(buffer, sizeof(buffer), "onKill_%d", CreatureDefID);

		//Need to run the script now, otherwise a simultaneous action may interrupt
		//the script, which could prevent certain instructions from firing or cause
		//problem with multi-instruction tasks like IF comparison.
		ScriptCall(buffer);
	}
}

void ActiveInstance :: ScriptCallPackageKill(const char *name)
{
	if(nutScriptPlayer != NULL && nutScriptPlayer->mActive)
	{
		std::vector<ScriptCore::ScriptParam> parms;
		parms.push_back(ScriptCore::ScriptParam(std::string(name)));
		nutScriptPlayer->JumpToLabel("on_package_kill", parms);

		char buffer[64];
		Util::SafeFormat(buffer, sizeof(buffer), "on_package_kill_%s", name);
		nutScriptPlayer->JumpToLabel(buffer);
	}
	else if(scriptPlayer != NULL && scriptPlayer->mActive)
		ScriptCall(name);
}

bool ActiveInstance :: ScriptCallUse(int sourceCreatureID, int usedCreatureID, int usedCreatureDefID)
{
	char buffer[64];
	if(nutScriptPlayer != NULL) {
		std::vector<ScriptCore::ScriptParam> p;
		Util::SafeFormat(buffer, sizeof(buffer), "on_use_%d", usedCreatureDefID);
		bool ok1 = nutScriptPlayer->RunFunctionWithBoolReturn(buffer, p, true);
		p.push_back(ScriptCore::ScriptParam(sourceCreatureID));
		p.push_back(ScriptCore::ScriptParam(usedCreatureID));
		p.push_back(ScriptCore::ScriptParam(usedCreatureDefID));
		bool ok2 = nutScriptPlayer->RunFunctionWithBoolReturn("on_use", p, true);
		return ok1 | ok2;
	}
	else {
		Util::SafeFormat(buffer, sizeof(buffer), "onUse_%d", usedCreatureDefID);
		return ScriptCall(buffer);
	}
}

void ActiveInstance :: ScriptCallUseHalt(int sourceCreatureID, int usedCreatureDefID)
{
	char buffer[64];
	if(nutScriptPlayer != NULL) {
		std::vector<ScriptCore::ScriptParam> p;
		Util::SafeFormat(buffer, sizeof(buffer), "on_use_halt_%d", usedCreatureDefID);
		nutScriptPlayer->JumpToLabel(buffer, p);
		p.push_back(ScriptCore::ScriptParam(sourceCreatureID));
		p.push_back(ScriptCore::ScriptParam(usedCreatureDefID));
		nutScriptPlayer->JumpToLabel("on_use_halt", p);
	}
	else {
		Util::SafeFormat(buffer, sizeof(buffer), "onUseHalt_%d", usedCreatureDefID);
		ScriptCall(buffer);
	}
}

void ActiveInstance :: ScriptCallUseFinish(int sourceCreatureID, int usedCreatureDefID)
{
	char buffer[64];
	if(nutScriptPlayer != NULL) {
		std::vector<ScriptCore::ScriptParam> p;
		Util::SafeFormat(buffer, sizeof(buffer), "on_use_finish_%d", usedCreatureDefID);
		nutScriptPlayer->JumpToLabel(buffer, p);
		p.push_back(ScriptCore::ScriptParam(sourceCreatureID));
		p.push_back(ScriptCore::ScriptParam(usedCreatureDefID));
		nutScriptPlayer->JumpToLabel("on_use_finish", p);

	}
	else {
		Util::SafeFormat(buffer, sizeof(buffer), "onUseFinish_%d", usedCreatureDefID);
		ScriptCall(buffer);
	}
}

//Calls a script jump label.  Can be used for any generic purpose.
bool ActiveInstance :: ScriptCall(const char *name)
{
	bool called = false;
	if(nutScriptPlayer != NULL) {

		if(nutScriptPlayer->JumpToLabel(name) == true) {
			g_Log.AddMessageFormat("Squirrel Script call %s in %d", name, mZone);
			//scriptPlayer->RunUntilWait();
			called = true;
		}
		else {
			g_Log.AddMessageFormat("Squirrel Refused to jump %s", name);
		}
	}
	if(scriptPlayer != NULL) {
		if(scriptPlayer->JumpToLabel(name) == true) {
			g_Log.AddMessageFormat("TSL Script call %s in %d", name, mZone);
			scriptPlayer->RunUntilWait();
			called = true;
		}
		else {
			g_Log.AddMessageFormat("TSL Refused to jump %s", name);
		}
	}
	if(!called) {
		g_Log.AddMessageFormat("Nothing handled script %s", name);
	}
	return called;
}

bool ActiveInstance :: RunScript(std::string &errors)
{
	if((scriptPlayer != NULL && scriptPlayer->mActive) || (nutScriptPlayer != NULL && nutScriptPlayer->mActive)) {
		g_Log.AddMessageFormat("Request to run script for %d when it is already running", mZone);
		return false;
	}

	if(scriptPlayer != NULL)
		delete scriptPlayer;
	if(nutScriptPlayer != NULL)
		delete nutScriptPlayer;

	//Load the new script system
	std::string path = InstanceScript::InstanceNutDef::GetInstanceScriptPath(mZoneDefPtr->mID, false, mZoneDefPtr->mGrove);
	if(Util::HasEnding(path, ".nut")) {
		g_Log.AddMessageFormat("Running Squirrel script %s", path.c_str());
		nutScriptDef.Initialize(path.c_str());
		nutScriptPlayer = new InstanceScript::InstanceNutPlayer();
		nutScriptPlayer->SetInstancePointer(this);
		nutScriptPlayer->Initialize(&nutScriptDef, errors);
	}
	else if(Util::HasEnding(path, ".txt")) {
		g_Log.AddMessageFormat("Running TSL script %s", path.c_str());
		scriptDef.CompileFromSource(path.c_str());
		scriptPlayer = new InstanceScript::InstanceScriptPlayer();
		scriptPlayer->Initialize(&scriptDef);
		scriptPlayer->SetInstancePointer(this);
		scriptPlayer->JumpToLabel("init");
	}
	else {
		g_Log.AddMessageFormat("No script for instance %d", mZone);
		return false;
	}
	return true;
}

void ActiveInstance :: ClearScriptObjects() {
	if(scriptPlayer != NULL) {
		delete scriptPlayer;
		scriptPlayer = NULL;
	}
	if(nutScriptPlayer != NULL) {
		delete nutScriptPlayer;
		nutScriptPlayer = NULL;
	}
}

bool ActiveInstance :: KillScript()
{
	bool ok = false;
	if(scriptPlayer != NULL && scriptPlayer->mActive) {
		g_Log.AddMessageFormat("Killing script for %d", mZone);
		scriptPlayer->EndExecution();
		ok = true;
	}
	else if(nutScriptPlayer != NULL) {
		if(nutScriptPlayer->mActive) {
			g_Log.AddMessageFormat("Killing squirrel script for %d", mZone);
			nutScriptPlayer->HaltExecution();
			ok = true;
		}
	}
	ClearScriptObjects();
	if(!ok) {
		g_Log.AddMessageFormat("Request to kill inactive squirrel script %d", mZone);
	}
	return ok;
}

void ActiveInstance :: FetchNearbyCreatures(SimulatorThread *simPtr, CreatureInstance *player)
{
	int wpos = 0;
#ifdef CREATUREQAV
	for(size_t i = 0; i < NPCListPtr.size(); i++)
	{
		int xlen = abs(player->CurrentX - NPCListPtr[i]->CurrentX);
		int zlen = abs(player->CurrentZ - NPCListPtr[i]->CurrentZ);
		if((xlen < g_CreatureListenRange) && (zlen < g_CreatureListenRange))
		{
			wpos += PrepExt_GeneralMoveUpdate(&simPtr->SendBuf[wpos], NPCListPtr[i]);
			if(wpos >= Global::MAX_SEND_CHUNK_SIZE)
			{
				simPtr->AttemptSend(simPtr->SendBuf, wpos);
				wpos = 0;
			}
		}
	}
#else
	CREATURE_IT it;
	for(it = NPCList.begin(); it != NPCList.end(); ++it)
	{
		int xlen = abs(player->CurrentX - it->second.CurrentX);
		int zlen = abs(player->CurrentZ - it->second.CurrentZ);
		if((xlen < g_CreatureListenRange) && (zlen < g_CreatureListenRange))
		{
			wpos += PrepExt_GeneralMoveUpdate(&simPtr->SendBuf[wpos], &it->second);
			if(wpos >= Global::MAX_SEND_CHUNK_SIZE)
			{
				simPtr->AttemptSend(simPtr->SendBuf, wpos, 0);
				wpos = 0;
			}
		}
	}
#endif
	if(wpos > 0)
		simPtr->AttemptSend(simPtr->SendBuf, wpos);
}

void ActiveInstance :: RunObjectInteraction(SimulatorThread *simPtr, int CDef)
{
	InteractObject *intObj = g_InteractObjectContainer.GetObjectByID(CDef, simPtr->pld.CurrentZoneID);
	if(intObj != NULL)
	{
		if(intObj->opType == InteractObject::TYPE_WARP)
		{
			simPtr->MainCallSetZone(intObj->WarpID, 0, false);
			simPtr->SetPosition(intObj->WarpX, intObj->WarpY, intObj->WarpZ, 1);
		}
		else if(intObj->opType == InteractObject::TYPE_LOCATIONRETURN)
		{
			int x = simPtr->pld.charPtr->groveReturnPoint[0];
			int y = simPtr->pld.charPtr->groveReturnPoint[1];
			int z = simPtr->pld.charPtr->groveReturnPoint[2];
			int zone = simPtr->pld.charPtr->groveReturnPoint[3];
			simPtr->MainCallSetZone(zone, 0, false);
			simPtr->SetPosition(x, y, z, 1);
		}
		else if(intObj->opType == InteractObject::TYPE_SCRIPT)
		{
			if(nutScriptPlayer != NULL) {
				std::vector<ScriptCore::ScriptParam> p;
				p.push_back(ScriptCore::ScriptParam(simPtr->creatureInst->CreatureID));
				p.push_back(ScriptCore::ScriptParam(CDef));
				nutScriptPlayer->JumpToLabel(intObj->scriptFunction, p);
			}
		}

		ScriptCallUseFinish(simPtr->creatureInst->CreatureID,  CDef);
	}
}

void ActiveInstance :: ApplyCreatureScale(CreatureInstance *target)
{
	if(scaleConfig.mPLayerLevel == 0)
		return;
	if(scaleProfile == NULL)
		return;
	if(target == NULL)
		return;

	target->PerformLevelScale(scaleProfile, scaleConfig.mPLayerLevel);
}

const DropRateProfile* ActiveInstance :: GetDropRateProfile(void)
{
	if(dropRateProfile == NULL && mZoneDefPtr != NULL)
		dropRateProfile = &mZoneDefPtr->GetDropRateProfile();

	return dropRateProfile;
}

//This is a potential command call from an instance script.
int ActiveInstance :: CountAlive(int creatureDefID)
{
	int count = 0;
	for(size_t i = 0; i < NPCListPtr.size(); i++)
		if(NPCListPtr[i]->CreatureDefID == creatureDefID)
			if(NPCListPtr[i]->HasStatus(StatusEffects::DEAD) == false)
				count++;
	return count;
}

void ActiveInstance :: LoadStaticObjects(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
		return;
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			int CDefID = lfr.BlockToIntC(0);
			int x = lfr.BlockToIntC(1);
			int y = lfr.BlockToIntC(2);
			int z = lfr.BlockToIntC(3);
			int facing = lfr.BlockToIntC(4);
			int flags = lfr.BlockLen[5] > 0 ? lfr.BlockToIntC(5) : 0;
			SpawnGeneric(CDefID, x, y, z, facing, flags);
		}
	}
	lfr.CloseCurrent();
}

CreatureInstance* ActiveInstance :: GetMatchingSidekick(CreatureInstance *host, int searchID)
{
	for(size_t i = 0; i < SidekickListPtr.size(); i++)
		if(SidekickListPtr[i]->AnchorObject == host)
			if(searchID == 0 || searchID == SidekickListPtr[i]->CreatureID)
				return SidekickListPtr[i];
	return NULL;
}

void ActiveInstance :: SetAllPlayerPVPStatus(int x, int z, int range, bool state)
{
	for(size_t i = 0; i < PlayerListPtr.size(); i++)
	{
		if(range == -1 || ActiveInstance::GetPointRangeXZ(PlayerListPtr[i], (float)x, (float)z, range) < DISTANCE_FAILED) //Half radius of minimap.
		{
			if(state == true)
				PlayerListPtr[i]->_AddStatusList(StatusEffects::PVPABLE, -1);
			else
				PlayerListPtr[i]->_RemoveStatusList(StatusEffects::PVPABLE);
		}
	}
}

void ActiveInstance :: NotifyKill(int mobRarity)
{
	mKillCount++;   //Just for fun.

	// Only increase drop rate for dungeons!  Overworld would be bad...
	if(mZoneDefPtr->IsDungeon() == false)
		return;

	//Make absolutely sure that our rarity index cannot exceed the data array size (0 to count-1)
	static const int MAX_COUNT = COUNT_ARRAY_ELEMENTS(g_Config.ProgressiveDropRateBonusMult);
	mobRarity = Util::ClipInt(mobRarity, 0, MAX_COUNT - 1);
	mDropRateBonusMultiplier += g_Config.ProgressiveDropRateBonusMult[mobRarity];
	mDropRateBonusMultiplier = Util::ClipFloat(mDropRateBonusMultiplier, 1.0F, g_Config.ProgressiveDropRateBonusMultMax);

	/*
	switch(mobRarity)
	{
	case CreatureRarityType::NORMAL: mDropRateBonusMultiplier += 0.0025F; break;
	case CreatureRarityType::HEROIC: mDropRateBonusMultiplier += 0.0050F; break;
	case CreatureRarityType::EPIC:   mDropRateBonusMultiplier += 0.0100F; break;
	case CreatureRarityType::LEGEND: mDropRateBonusMultiplier += 0.0200F; break;
	}
	g_Log.AddMessageFormat("DropMult: %g", mDropRateBonusMultiplier);
	*/
}

int ActiveInstance :: ActivateAbility(CreatureInstance *cInst, short ability, int ActionType, ActiveAbilityInfo *abInfo)
{
	Debug::ActivateAbility_cInst = cInst;
	Debug::ActivateAbility_ability = ability;
	Debug::ActivateAbility_ActionType = ActionType;
	Debug::ActivateAbility_abTargetCount = cInst->ab[0].TargetCount;
	memcpy(Debug::ActivateAbility_abTargetList, cInst->ab[0].TargetList, sizeof(cInst->ab[0].TargetList));

	if(ActionType == EventType::onRequest)
	{
		if(cInst->HasStatus(StatusEffects::STUN) == true)
			return Ability2::ABILITY_STUN;
		if(cInst->HasStatus(StatusEffects::DAZE) == true)
			return Ability2::ABILITY_DAZE;
	}

	int res = g_AbilityManager.ActivateAbility(cInst, ability, ActionType, abInfo);
	g_AbilityManager.RunImplicitActions();
	return res;
}

ActiveInstance* ActiveInstanceManager :: ResolveExistingInstance(PlayerInstancePlacementData& pd, ZoneDefInfo* zoneDef)
{
	if(zoneDef == NULL)
		return NULL;

	//Check if a direct warp to an instance.
	ActiveInstance *ptr = NULL;
	if(pd.in_instanceID != 0)
	{
		ptr = GetPtrByZoneInstanceID(pd.in_zoneID, pd.in_instanceID);
		if(ptr != NULL)
		{
			g_Log.AddMessageFormat("Found instance by instance ID: %d", pd.in_instanceID);
			return ptr;
		}
	}

	//If an instanced zone, check by owner.  If not, check by zone ID.
	if(zoneDef->mInstance == true)
	{
		//If in a party, use the party leader as the owner instead of the given player.
		if(pd.in_partyID != 0)
		{
			//First lookup with the direct party ID.
			ptr = GetPtrByZonePartyID(pd.in_zoneID, pd.in_partyID);
			if(ptr != NULL)
			{
				g_Log.AddMessageFormat("Found instance by party ID: %d", pd.in_partyID);
				return ptr;
			}

			//If that failed, transfer "ownership" lookups and assignments to the PARTY LEADER
			//instead of the particular individual that is looking to join this instance.
			ActiveParty* party = g_PartyManager.GetPartyByID(pd.in_partyID);
			if(party != NULL)
				pd.in_creatureDefID = party->mLeaderDefID;
		}
		ptr = GetPtrByZoneOwner(pd.in_zoneID, pd.in_creatureDefID);
		if(ptr != NULL)
		{
			g_Log.AddMessageFormat("Found instance by owner: %d", pd.in_creatureDefID);
			return ptr;
		}
	}
	else
	{
		ptr = GetPtrByZoneID(pd.in_zoneID);
		if(ptr != NULL)
		{
			g_Log.AddMessageFormat("Found instance by zone: %d", pd.in_zoneID);
			return ptr;
		}
	}
	return NULL;
}

int ActiveInstanceManager :: AddSimulator_Ex(PlayerInstancePlacementData &pd)
{
	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(pd.in_zoneID);
	if(zoneDef == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] AddSimulator() ZoneDef does not exist for ID: %d", pd.in_zoneID);
		return -1;
	}

	bool created = false;
	ActiveInstance *ptr = ResolveExistingInstance(pd, zoneDef);
	if(ptr == NULL)
	{
		g_Log.AddMessageFormat("Creating new instance for zone: %d", pd.in_zoneID);
		ptr = CreateInstance(pd.in_zoneID, pd);
		created = true;
	}
	if(ptr == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] AddSimulator() Unable to create instance for zone: %d", pd.in_zoneID);
		return -1;
	}

	/*
	if(pd.in_instanceID != 0 && pd.in_partyID == 0)
		ptr = GetPtrByZoneInstanceID(pd.in_zoneID, pd.in_instanceID);

	if(ptr == NULL)
	{
		if(zoneDef->mInstance == true)
		{
			ActiveParty* party = g_PartyManager.GetPartyByID(pd.in_partyID);
			if(party != NULL)
				CDefID = party->mLeaderDefID;
			ptr = GetPtrByZoneOwner(pd.in_zoneID, CDefID);
		}
		else
			ptr = GetPtrByZoneID(pd.in_zoneID);
	}

	if(ptr == NULL)
		ptr = CreateInstance(pd.in_zoneID, 0);
	*/

	//If a private instance, make sure ownership status is properly assigned or transferred.
	if(created == true && zoneDef->mInstance == true)
	{
		ptr->mOwnerCreatureDefID = pd.in_creatureDefID;
		ptr->mOwnerPartyID = pd.in_partyID;
		ptr->mOwnerName = pd.in_cInst->css.display_name;
	}

	CreatureInstance *cptr = ptr->LoadPlayer(pd.in_cInst, pd.in_simPtr);
	if(cptr == NULL)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] AddSimulator() Failed to add Sim:%d to zone %d", pd.in_simPtr->InternalIndex, pd.in_zoneID);
		return -1;
	}

	cptr->BuildZoneString(ptr->mInstanceID, ptr->mZone, 0);
	cptr->OnInstanceEnter(ptr->arenaRuleset);
	g_Log.AddMessageFormatW(MSG_DIAG, "Added Sim:%d to instance %d", pd.in_simPtr->InternalIndex, ptr->mInstanceID);
	ptr->AdjustPlayerCount(1);
	
	pd.out_cInst = cptr;
	pd.out_zoneDef = zoneDef;
	pd.out_instanceID = ptr->mInstanceID;
	return 1;
}


int ActiveInstanceManager :: FlushSimulator(int SimulatorID)
{
	Debug::LastFlushSimulatorID = SimulatorID;
	//Remove any pending messages left by this calling simulator.
	//Any remaining messages which use creature pointers are now unsafe.
	//int a;
	int debug_deleted = 0;
	debug_deleted = bcm.RemoveSimulator(SimulatorID);
	if(debug_deleted > 0)
		g_Log.AddMessageFormatW(MSG_DIAG, "Flushed %d remaining messages", debug_deleted);

	return 1;
}

CreatureInstance* ActiveInstanceManager :: GetPlayerCreatureByName(const char *name)
{
	CreatureInstance *search = NULL;
	std::list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		search = it->GetPlayerByName(name);
		if(search != NULL)
			break;
	}
	return search;
}

CreatureInstance* ActiveInstanceManager :: GetPlayerCreatureByDefID(int CreatureDefID)
{
	CreatureInstance *search = NULL;
	std::list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		search = it->GetPlayerByCDefID(CreatureDefID);
		if(search != NULL)
			break;
	}
	return search;
}


void ActiveInstanceManager :: CheckActiveInstances(void)
{
	//Check if any instances have zero players.  If they're not persistent,
	//they need to be deleted after a certain amount of time.
	if(g_ServerTime < nextInstanceCheck)
		return;

	nextInstanceCheck = g_ServerTime + INSTANCE_DELETE_RECHECK;

	if(instList.size() == 0)
		return;

	std::list<ActiveInstance>::iterator it;
	int delCount = 0;
	it = instList.begin();
	cs.Enter("ActiveInstanceManager :: CheckActiveInstances");
	while(it != instList.end())
	{
		if(it->QualifyDelete() == true)
		{
			g_Log.AddMessageFormatW(MSG_SHOW, "Removing inactive instance [%s : %d]", it->mZoneDefPtr->mName.c_str(), it->mZone);
			delCount++;
			it->Clear();
			instList.erase(it++);
		}
		else
		{
			++it;
		}
	}

	if(delCount > 0)
		RebuildAccessList();
	cs.Leave();
}

//All instances with zero players will be triggered for immediate deletion on the next cycle.
void ActiveInstanceManager :: DebugFlushInactiveInstances(void)
{
	std::list<ActiveInstance>::iterator it;
	for(it = instList.begin(); it != instList.end(); ++it)
	{
		if(it->mPlayers == 0)
			it->mExpireTime = g_ServerTime;
	}
	nextInstanceCheck = g_ServerTime;
}


PlayerInstancePlacementData :: PlayerInstancePlacementData()
{
	Clear();
}

void PlayerInstancePlacementData :: Clear(void)
{
	memset(this, 0, sizeof(PlayerInstancePlacementData));
}

void PlayerInstancePlacementData :: SetInstanceScaler(const std::string &name)
{
	const InstanceScaleProfile *profile = GetPartyLeaderInstanceScaler();
	if(profile == NULL)
	{
		if(in_partyID == 0 || (in_partyID != 0 && mPartyLeaderDefID == this->in_creatureDefID))
		{
			profile = g_InstanceScaleManager.GetProfile(name);
			//g_Log.AddMessageFormat("Setting default: Party:%d, profile:%s", in_partyID, name.c_str());
		}
	}
	/*
	if(profile != NULL)
		g_Log.AddMessageFormat("Setting [%s]", profile->mDifficultyName.c_str());
	*/

	in_scaleProfile = profile;
}

const InstanceScaleProfile* PlayerInstancePlacementData :: GetPartyLeaderInstanceScaler(void)
{
	mPartyLeaderDefID = 0;
	//g_Log.AddMessageFormat("Party: %d", in_partyID);

	if(in_partyID == 0)
		return NULL;

	ActiveParty *party = g_PartyManager.GetPartyByID(in_partyID);
	if(party == NULL)
		return NULL;

	PartyMember *member = party->GetMemberByDefID(party->mLeaderDefID);
	if(member == NULL)
		return NULL;

	CreatureInstance *creature = member->mCreaturePtr;
	if(creature == NULL)
		return NULL;

	if(!(creature->serverFlags & ServerFlags::IsPlayer))
		return NULL;

	CharacterData *charDat = creature->charPtr;
	if(charDat == NULL)
		return NULL;

	//g_Log.AddMessageFormat("Returning party leader %s [%d] scaler %s", creature->css.display_name, party->mLeaderDefID, charDat->InstanceScaler.c_str());
	mPartyLeaderDefID = party->mLeaderDefID;
	return g_InstanceScaleManager.GetProfile(charDat->InstanceScaler);
}
