#include "Instance.h"
#include "StringList.h"
#include "CreatureSpawner2.h"
#include "Scenery2.h"
#include "FileReader.h"
#include "Util.h"
#include <math.h>
#include <algorithm>
#include "Globals.h"
#include "EliteMob.h"

#include "Config.h"  //For debug behavior state

SpawnPackageManager g_SpawnPackageManager;

Timer :: Timer()
{
	Update(0);
}

Timer :: ~Timer()
{
}

bool Timer :: Ready(void)
{
	//return GetTickCount() >= NextFire;
	return GetTime() >= NextFire;
}

void Timer :: Update(unsigned long newTimeOffset)
{
	//NextFire = GetTickCount() + newTimeOffset;
	NextFire = GetTime() + newTimeOffset;
}

bool Timer :: ReadyWithUpdate(unsigned long newTimeOffset)
{
	//unsigned long curTime = GetTickCount();
	unsigned long curTime = GetTime();
	if(curTime < NextFire)
		return false;

	NextFire = curTime + newTimeOffset;
	return true;
}

ActiveSpawner :: ActiveSpawner()
{
	Clear();
}

void ActiveSpawner :: Clear(void)
{
	spawnPoint = NULL;
	spawnCount = 0;
	nextSpawn = 0;
	spawnPackage = NULL;
	refCount = 0;
	attachedCreatureID.clear();
}

bool ActiveSpawner :: UpdateSourcePackage(void)
{
	if(spawnPoint == NULL)
	{
		g_Log.AddMessageFormat("[WARNING] UpdateSourcePackage() cannot derive from null spawnPoint"); 
		return false;
	}

	const char *pkgName = spawnPoint->GetSpawnPackageName();
	if(pkgName == NULL)
		return false;

	if(pkgName[0] == '#')
		spawnPackage = &g_SpawnPackageManager.nullSpawnPackage;
	else
		spawnPackage = g_SpawnPackageManager.GetPointerByName(pkgName);

	if((spawnPackage == NULL) && (pkgName[0] != 0))
	{
		g_Log.AddMessageFormat("[WARNING] Spawn point ID [%d, loc: %g %g] undefined package [%s]", spawnPoint->ID, spawnPoint->LocationX, spawnPoint->LocationZ, pkgName); 
		return false;
	}
	return true;
}

void ActiveSpawner :: Invalidate(void)
{
	nextSpawn = PlatformTime::MAX_TIME;
}

void ActiveSpawner :: UpdateSpawnDelay(int specificDelaySeconds)
{
	//If no delay was specified, use the default spawn time of this SpawnPoint.
	if(specificDelaySeconds == 0)
		specificDelaySeconds = GetRespawnDelaySeconds();
	nextSpawn = g_ServerTime + (GetRespawnDelaySeconds() * 1000);
}

void ActiveSpawner :: OnSpawnSuccess(void)
{
	spawnCount++;
	refCount++;
	UpdateSpawnDelay(0);
}

int ActiveSpawner :: GetRespawnDelaySeconds(void)
{
	int delaySeconds = CreatureSpawnDef::DEFAULT_DESPAWNTIME;
	if(spawnPoint->extraData != NULL)
	{
		int delay = spawnPoint->extraData->despawnTime;
		if(delay != 0 && delay != CreatureSpawnDef::DEFAULT_DESPAWNTIME)
			delaySeconds = delay;
	}
	if(delaySeconds == CreatureSpawnDef::DEFAULT_DESPAWNTIME)
		if(spawnPackage != NULL && spawnPackage->spawnDelay != 0)
			delaySeconds = spawnPackage->spawnDelay;

	return delaySeconds;
}

int ActiveSpawner :: GetLoyaltyRadius(void)
{
	if(spawnPoint == NULL)
		return 0;
	if(spawnPoint->extraData == NULL)
		return 0;
	if(spawnPackage == NULL)
		return 0;

	int loyaltyRadius = spawnPoint->extraData->loyaltyRadius;
	if(spawnPackage->loyaltyRadius > 0)
		loyaltyRadius = spawnPackage->loyaltyRadius;
	
	return loyaltyRadius;
}

bool ActiveSpawner :: HasLoyaltyLinks(void)
{
	if(spawnPoint == NULL)
		return false;
	return spawnPoint->HasLinks(SceneryObject::LINK_TYPE_LOYALTY);
}

void ActiveSpawner :: AddAttachedCreature(int creatureID)
{
	attachedCreatureID.push_back(creatureID);
}

void ActiveSpawner :: RemoveAttachedCreature(int creatureID)
{
	for(size_t i = 0; i < attachedCreatureID.size(); i++)
	{
		if(attachedCreatureID[i] == creatureID)
		{
			attachedCreatureID.erase(attachedCreatureID.begin() + i);
			return;
		}
	}
}

SpawnTile :: SpawnTile()
{
	TileX = 0;
	TileY = 0;
	sceneryLoaded = false;
	releaseTime = 0;
	manager = NULL;
}

SpawnTile :: ~SpawnTile()
{
	activeSpawn.clear();
}

void SpawnTile :: LoadPropSpawnPoint(int zone, int sceneryPageX, int sceneryPageY, int PropID)
{
	g_SceneryManager.GetThread("SpawnTile::LoadPropSpawnPoint");
	SceneryPage *page = g_SceneryManager.GetOrCreatePage(zone, sceneryPageX, sceneryPageY);
	g_SceneryManager.ReleaseThread();
	if(page == NULL)
		return;

	SPAWN_MAP::iterator lastInsert = activeSpawn.begin();
	SceneryPage::SCENERY_IT it;
	for(it = page->mSceneryList.begin(); it != page->mSceneryList.end(); ++it)
	{
		SceneryObject *so = &it->second;
		if(so->ID != PropID)
			continue;
		if(AddSpawnerFromProp(lastInsert, &it->second) == true)
			return;
	}
}

void SpawnTile :: LoadTileSpawnPoints(int zone, int sceneryPageX, int sceneryPageY, int x1, int y1, int x2, int y2)
{
	//Loads the spawn points from a scenery page with the given page coordinates.
	//Spawn points are only added if their positions fall within the absolute
	//coordinates of the spawn tile.
	g_SceneryManager.GetThread("SpawnTile::LoadTileSpawnPoints");
	//SceneryPage *page = g_SceneryManager.LoadPage(zone, sceneryPageX, sceneryPageY);
	SceneryPage *page = g_SceneryManager.GetOrCreatePage(zone, sceneryPageX, sceneryPageY);
	g_SceneryManager.ReleaseThread();
	if(page == NULL)
		return;

	//g_Log.AddMessageFormat("LoadTileSpawnPoints() %d, %d", sceneryPageX, sceneryPageY);

	SPAWN_MAP::iterator lastInsert = activeSpawn.begin();
	//SceneryPage::SCENERY_CONT::iterator it;
	SceneryPage::SCENERY_IT it;
	int count = 0;
	for(it = page->mSceneryList.begin(); it != page->mSceneryList.end(); ++it)
	{
		SceneryObject *so = &it->second;
		if((int)so->LocationX < x1 || (int)so->LocationX > x2)
			continue;
		if((int)so->LocationZ < y1 || (int)so->LocationZ > y2)
			continue;
		if(AddSpawnerFromProp(lastInsert, &it->second) == true)
			count++;
	}
	//g_Log.AddMessageFormat("[DEBUG] LoadTileSpawnPoints() added: %d", count);
}

bool SpawnTile :: AddSpawnerFromProp(SPAWN_MAP::iterator &position, SceneryObject *prop)
{
	//The position iterator allows easier batch processing if necessary.

	ActiveSpawner newSpawner;
	if(prop->IsSpawnPoint() == true)
	{
		newSpawner.spawnPoint = prop;
		if(newSpawner.UpdateSourcePackage() == true)
		{
			position = activeSpawn.insert(position, SPAWN_PAIR(prop->ID, newSpawner));
			return true;
		}
	}
	return false;
}


bool SpawnTile :: Debug_VerifyProp(int zone, int propID, SceneryObject *ptr)
{
	// DEBUG VERIFICATION OF PROP
	if(ptr == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] SpawnTile::Debug_VerifyProp existing pointer is null: %d", propID);
		return false;
	}
	/*
	SceneryObject *so = g_SceneryManager.GetPropPtr(zone, propID, NULL);
	if(so == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] SpawnTile::Debug_VerifyProp could not find prop: %d", propID);
		return false;
	}
	if(so != ptr)
	{
		g_Log.AddMessageFormat("[CRITICAL] SpawnTile::RunProcessing found prop does not match location: %d (looking for %p, found %p)", propID, ptr, so);
		return false;
	}
	*/
	return true;
}

void SpawnTile :: RunProcessing(ActiveInstance *inst)
{
	if(inst == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] SpawnTile::RunProcessing inst is NULL");
		return;
	}
	Debug::LastTileZone = inst->mZone;
	Debug::LastTileX = TileX;
	Debug::LastTileY = TileY;

	//Since processing is being called, the tile must still be active.
	releaseTime = g_ServerTime + SpawnManager::PROCESSING_INTERVAL;
	SPAWN_MAP::iterator it;
	for(it = activeSpawn.begin(); it != activeSpawn.end(); ++it)
	{
		Debug::LastTilePropID = it->first;
		Debug::LastTilePtr = (void*)it->second.spawnPoint;
		Debug::LastTilePackage = (void*)it->second.spawnPackage;

		if(Debug_VerifyProp(inst->mZone, it->first, it->second.spawnPoint) == false)
			continue;

		if(g_ServerTime < it->second.nextSpawn)
			continue;

		if(it->second.spawnPoint == NULL)
		{
			g_Log.AddMessageFormat("[CRITICAL] SpawnTile::RunProcessing no spawn point");
			it->second.Invalidate();
			continue;
		}

		//User-placed spawn points may not yet have extra data attached.
		if(it->second.spawnPoint->extraData == NULL)
		{
			it->second.Invalidate();
			continue;
		}

		if(it->second.refCount >= it->second.spawnPoint->extraData->maxActive)
			continue;

		if(it->second.spawnPackage == NULL)
		{
			it->second.Invalidate();
			continue;
		}
		if(it->second.spawnPackage->UniquePoints != 0)
		{
			bool test = inst->uniqueSpawnManager.TestSpawn(it->second.spawnPackage->packageName, it->second.spawnPackage->UniquePoints, it->second.spawnPoint->ID);
			if(test == false)
			{
				//If it's an instance, don't retry if it failed.  Otherwise it might be an
				//overworld spawn, and we'll want to check again later.
				if(inst->mZoneDefPtr->mInstance == true)
					it->second.Invalidate();
				else
					it->second.UpdateSpawnDelay(60);

				continue;
			}
		}

		if(inst->mZoneDefPtr == NULL)
		{
			g_Log.AddMessageFormat("[CRITICAL] SpawnTile::RunProcessing inst->mZoneDefPtr is NULL");
			continue;
		}

		if(inst->mZoneDefPtr->mInstance == true)
			if(it->second.spawnCount >= 1)
				continue;

		/*
		if(it->second.spawnPoint->extraData->sequential != 0)
			continue;
		*/

		CreatureInstance *cInst = SpawnCreature(inst, &it->second, 0, 0);
		if(cInst != NULL)
		{
			it->second.OnSpawnSuccess();
			it->second.AddAttachedCreature(cInst->CreatureID);
		}
	}
}

CreatureInstance * SpawnTile :: SpawnCreature(ActiveInstance *inst, ActiveSpawner *spawner, int forceCreatureDef, int forceFlags)
{
	if(forceCreatureDef == 0 && spawner->spawnPackage == NULL)
	{
		g_Log.AddMessageFormat("[WARNING] SpawnCreature() spawnPackage is NULL (prop ID: %d)", spawner->spawnPoint->ID);
		spawner->Invalidate();
		return NULL;
	}

	bool propSpawn = false;
	int CDefID = -1;
	int SpawnFlags = 0;
	if(forceCreatureDef == 0 && spawner->spawnPackage != NULL && spawner->spawnPoint->extraData->spawnPackage[0] == '#')
	{
		size_t len = strlen(spawner->spawnPoint->extraData->spawnPackage);
		int cOffset = 1;
		for(size_t i = 1; i < len; i++)
		{
			char p = spawner->spawnPoint->extraData->spawnPackage[i];
			if(p == 'f' || p == 'F')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_FRIENDLY;
				cOffset++;
			}
			else if(p == 'h' || p == 'H')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_HIDEMAP;
				cOffset++;
			}
			else if(p == 'n' || p == 'N')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_NEUTRAL;
				cOffset++;
			}
			else if(p == 'a' || p == 'A')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_FRIENDLY_ATTACK;
				cOffset++;
			}
			else if(p == 'e' || p == 'E')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_ENEMY;
				cOffset++;
			}
			else if(p == 'm' || p == 'M')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_VISWEAPON_MELEE;
				cOffset++;
			}
			else if(p == 'r' || p == 'R')
			{
				SpawnFlags |= SpawnPackageDef::FLAG_VISWEAPON_RANGED;
				cOffset++;
			}
			else if(p == 'p' || p == 'P')
			{
				propSpawn = true;
				cOffset++;
				break;
			}
			else
				break;
		}

		if(propSpawn)
		{
			char buffer[256];
			char* prop = spawner->spawnPoint->extraData->spawnPackage + 2;
			Util::SafeFormat(buffer, sizeof(buffer), "p1:{[\"a\"]=\"%s\"}", prop);

			/* Make a fake creature consisting of a prop. This means instance scripts (and so groves) could spawn props by
			 * by using a spawn point with maxActive of 0, then use the script to spawn them.
			 */
			CreatureDefinition newItem;
			CDefID = g_SpawnPackageManager.fakeCreatureId++;
			newItem.css.level = 1;
			newItem.css.profession = 1;
			memcpy(newItem.css.creature_category, "Inanimate\0", 9);
			newItem.css.SetAppearance(buffer);
			newItem.CreatureDefID = CDefID;
			newItem.css.SetDisplayName(spawner->spawnPoint->extraData->spawnName);

			CreatureDef.AddNPCDef(newItem);

			g_Log.AddMessageFormat("[INFO] Creating ad-hoc prop creature: %d ()", newItem.CreatureDefID);

		}
		else
		{
			CDefID = atoi(&spawner->spawnPoint->extraData->spawnPackage[cOffset]);
		}
		//g_Log.AddMessageFormat("[WARNING] SpawnCreature() resolved direct spawn: %d", CDefID);
	}
	else
	{
		if(forceCreatureDef == 0)
		{
			CDefID = spawner->spawnPackage->GetRandomSpawn(spawner->spawnPoint);
			SpawnFlags = spawner->spawnPackage->SpawnFlags;
		}
		else
		{
			CDefID = forceCreatureDef;
			SpawnFlags = forceFlags;
		}
	}

	if(CDefID == -1)
	{
		g_Log.AddMessageFormat("[WARNING] SpawnCreature() could not determine spawn for package [%s]", spawner->spawnPackage->packageName);

		//Bump it to never.  Prevents logging spam from repeated attempts.
		spawner->Invalidate();
		return NULL;
	}

	//Make sure the creature ID actually exists in the database

	/*
	int cdef = CreatureDef.GetIndex(CDefID);
	if(cdef == -1)
	{
		g_Log.AddMessageFormat("[WARNING] SpawnCreature() package [%s] returned unknown creature ID [%d]", spawner->spawnPackage->packageName, CDefID);

		//Bump it to never.  Prevents logging spam from repeated attempts.
		spawner->Invalidate();
		return NULL;
	}*/

	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if(cdef == NULL)
	{
		g_Log.AddMessageFormat("[WARNING] SpawnCreature() package [%s] returned unknown creature ID [%d]", spawner->spawnPackage->packageName, CDefID);

		//Bump it to never.  Prevents logging spam from repeated attempts.
		spawner->Invalidate();
		return NULL;
	}

	//g_Log.AddMessageFormat("[DEBUG] Spawning from package [%s, %s] at (%g, %g, %g)", SpawnDefs.defList[spawnDef].spawnPackage, spawnPackage->packageName, SpawnDefs.defList[spawnDef].xpos, SpawnDefs.defList[spawnDef].ypos, SpawnDefs.defList[spawnDef].zpos);

	CreatureInstance newItem;

	newItem.actInst = inst;

	newItem.CreatureDefID = CDefID;
	newItem.CreatureID = inst->GetNewActorID();

	newItem.Faction = FACTION_PLAYERHOSTILE;
	newItem.BuildZoneString(inst->mInstanceID, inst->mZone, 0);
	newItem.css.CopyFrom(&cdef->css);

	newItem.CurrentX = (int)spawner->spawnPoint->LocationX;
	newItem.CurrentY = (int)spawner->spawnPoint->LocationY;
	newItem.CurrentZ = (int)spawner->spawnPoint->LocationZ;
	newItem.lastIdleX = newItem.CurrentX;
	newItem.lastIdleZ = newItem.CurrentZ;
	newItem.tetherNodeX = newItem.CurrentX;
	newItem.tetherNodeZ = newItem.CurrentZ;

	int facing = Util::QuaternionToByteFacing(spawner->spawnPoint->QuatX, spawner->spawnPoint->QuatY, spawner->spawnPoint->QuatZ, spawner->spawnPoint->QuatW);

	int leaseTime = 0;
	if(spawner->spawnPoint->extraData != NULL)
	{
		if(spawner->spawnPoint->extraData->facing != 0)
			facing = spawner->spawnPoint->extraData->facing;

		if(spawner->spawnPoint->extraData->linkCount > 0)
		{
			newItem.previousPathNode = spawner->spawnPoint->ID;
			newItem.nextPathNode = spawner->spawnPoint->ID;  //Zero so it can immediate begin a search for pathnodes.
		}

		int inner = spawner->spawnPoint->extraData->innerRadius;
		int outer = spawner->spawnPoint->extraData->outerRadius;
		if(inner != 0 || outer != 0)
		{
			double angle = randdbl(0, DOUBLE_PI);
			int span = outer - inner;
			if(span < 0)
				span = 0;
			int dist = inner + randint(0, span);
			newItem.CurrentX += (int)((double)dist * cos(angle));
			newItem.CurrentZ += (int)((double)dist * sin(angle));
		}
		leaseTime = spawner->spawnPoint->extraData->leaseTime;

		if(strlen(spawner->spawnPoint->extraData->aiModule) > 0)
			newItem.css.SetAIPackage(spawner->spawnPoint->extraData->aiModule);
	}
	
	newItem.Speed = 0;
	newItem.Heading = facing;
	newItem.Rotation = facing;

	//newItem.SpawnerDef = spawnDef;
	//newItem.SpawnerTile = QPage;
	newItem.SetServerFlag(ServerFlags::IsNPC, true);
	newItem.SetServerFlag(ServerFlags::LocalActive, true);

#ifndef CREATUREMAP
	inst->NPCList.push_back(newItem);
	CreatureInstance *ptr = &inst->NPCList.back();
#else
	ActiveInstance::CREATURE_IT it;
	it = inst->NPCList.insert(inst->NPCList.end(), ActiveInstance::CREATURE_PAIR(newItem.CreatureID, newItem));
	CreatureInstance *ptr = &it->second;
#endif

	//1 is persona, all other DefHints flags are for friendly NPCs.
	int aggro = 1;
	bool setFlags = false;
	if(cdef->DefHints > 1)
	{
		setFlags = true;
		if(inst->mZone < ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
			ptr->SetServerFlag(ServerFlags::Stationary, true);
	}

/*	if(strstr(cdef->css.appearance, "p1:") != NULL)
		setFlags = true;
*/
	if(cdef->css.IsPropAppearance() == true)
	{
		ptr->SetServerFlag(ServerFlags::Stationary, true);
		setFlags = true;
	}

	if(setFlags == true)
	{
		ptr->_AddStatusList(StatusEffects::INVINCIBLE, -1);
		ptr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
		ptr->_AddStatusList(StatusEffects::IS_USABLE, -1);
		ptr->Faction = FACTION_PLAYERFRIENDLY;
		ptr->SetServerFlag(ServerFlags::Noncombatant, true);
		ptr->SetServerFlag(ServerFlags::NeutralInactive, true);
		aggro = 0;
	}

	for(size_t i = 0; i < cdef->DefaultEffects.size(); i++)
		ptr->_AddStatusList(cdef->DefaultEffects[i], -1);

	if(SpawnFlags & SpawnPackageDef::FLAG_FRIENDLY)
	{
		ptr->Faction = FACTION_PLAYERFRIENDLY;
		ptr->_AddStatusList(StatusEffects::INVINCIBLE, -1);
		ptr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
		aggro = 0;
	}
	if(SpawnFlags & SpawnPackageDef::FLAG_FRIENDLY_ATTACK)
	{
		ptr->Faction = FACTION_PLAYERFRIENDLY;
		ptr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
		aggro = 0;
	}
	if(SpawnFlags & SpawnPackageDef::FLAG_NEUTRAL)
	{
		ptr->SetServerFlag(ServerFlags::NeutralInactive, true);
		aggro = 0;
	}
	if(SpawnFlags & SpawnPackageDef::FLAG_ENEMY)
	{
		ptr->Faction = FACTION_PLAYERHOSTILE;
		aggro = 1;
	}

	//if(cdef->css.eq_appearance[0] != 0)
	//{
		short visw = 0;
		if(SpawnFlags & SpawnPackageDef::FLAG_VISWEAPON_MELEE)
			visw = 1;
		if(SpawnFlags & SpawnPackageDef::FLAG_VISWEAPON_RANGED)
			visw = 2;
		ptr->css.vis_weapon = visw;
	//}

	if(SpawnFlags & SpawnPackageDef::FLAG_HIDEMAP)
		ptr->css.hide_minimap = 1;

	if(aggro == 1 && (cdef->css.ai_package[0] == 0 && ptr->css.ai_package[0] == 0))
		aggro = 0;

	ptr->css.aggro_players = aggro;

	bool tryElite = g_Config.AllowEliteMob;
	if(tryElite == true)
	{
		if(ptr->Faction == FACTION_PLAYERFRIENDLY || aggro == 0)
			tryElite = false;
		if(cdef->IsNamedMob() == true)
			tryElite = false;
		if(inst->mZoneDefPtr->IsDungeon())
			tryElite = false;
		if(inst->mZoneDefPtr->IsPlayerGrove())
			tryElite = false;

		//If still applicable, call transformation before initialization since it may adjust aggro and
		//health amounts.
		if(tryElite == true)
			g_EliteManager.ApplyTransformation(ptr); 
	}
	
	ptr->Instantiate();

	if(leaseTime > 0)
	{
		ptr->_AddStatusList(StatusEffects::DEAD, -1);
		ptr->SetServerFlag(ServerFlags::TriggerDelete, true);
		ptr->deathTime = g_ServerTime + (leaseTime * 1000);
	}

	ptr->ApplyGlobalInstanceBuffs();

	ptr->spawnGen = spawner;
	ptr->spawnTile = this;

	inst->RebuildNPCList();
	inst->ApplyCreatureScale(ptr);

	attachedCreatureID.push_back(ptr->CreatureID);

	//g_Log.AddMessageFormat("[DEBUG] Pushed %d onto %d,%d", ptr->CreatureID, TileX, TileY);

	return ptr;
}

void SpawnTile :: RemoveAllAttachedCreature(ActiveInstance *inst)
{
	//Removes all active creatures from an instance.  Should be called before the
	//tile is destroyed.
	for(size_t i = 0; i < attachedCreatureID.size(); i++)
		inst->RemoveNPCInstance(attachedCreatureID[i]);
	attachedCreatureID.clear();
}

void SpawnTile :: RemoveAttachedCreature(int CreatureID)
{
	//Called when an instance has deleted this creature.
	for(size_t i = 0; i < attachedCreatureID.size(); i++)
	{
		if(attachedCreatureID[i] == CreatureID)
		{
			attachedCreatureID.erase(attachedCreatureID.begin() + i);
			return;
		}
	}
}

void SpawnTile :: UpdateSpawnPoint(SceneryObject *prop)
{
	SPAWN_MAP::iterator it;
	it = activeSpawn.find(prop->ID);
	if(it == activeSpawn.end())
	{
		it = activeSpawn.begin(); //Code::Blocks won't allow the argument on the next line
		AddSpawnerFromProp(it, prop);
		g_Log.AddMessageFormat("[DEBUG] UpdateSpawnPoint() added");
	}
	else
	{
		RemoveSpawnPointCreatures(&it->second);
		g_Log.AddMessageFormat("[DEBUG] UpdateSpawnPoint() update");
		it->second.spawnPoint = prop;
		if(it->second.UpdateSourcePackage() == false)
			it->second.Invalidate();
		else
			it->second.nextSpawn = g_ServerTime;

		//Reset the spawn data in case an error reset the spawnsystem.
		it->second.spawnCount = 0;
		it->second.refCount = 0;
	}
}

bool SpawnTile :: RemoveSpawnPoint(ActiveInstance *inst, int PropID)
{
	SPAWN_MAP::iterator it;
	it = activeSpawn.find(PropID);
	if(it == activeSpawn.end())
		return false;

	RemoveSpawnPointCreatures(&it->second);
	activeSpawn.erase(it);
	return true;
}

void SpawnTile :: RemoveSpawnPointCreatures(ActiveSpawner *spawner)
{
	RemoveSpawnPointCreature(spawner, 0);
}

void SpawnTile :: RemoveSpawnPointCreature(ActiveSpawner *spawner, int creatureID)
{
	//Removes all creatures spawned by this point. 
#ifndef CREATUREMAP
	std::list<CreatureInstance>::iterator cit;
	cit = manager->actInst->NPCList.begin();
	int count = 0;
	while(cit != manager->actInst->NPCList.end())
	{
		if((creatureID == 0 || creatureID == cit->creatureID) && (cit->serverFlags & ServerFlags::IsNPC) && cit->spawnGen == spawner)
		{
			MessageComponent msg;
			msg.SimulatorID = -1;
			msg.message = BCM_RemoveCreature;
			msg.param1 = cit->CreatureID;
			msg.param2 = 0;
			msg.x = cit->CurrentX;
			msg.z = cit->CurrentZ;
			msg.actInst = manager->actInst;
			bcm.AddEventCopy(msg);

			cit->Destroy();
			pendingOperations.UpdateList_Remove(&*cit);
			pendingOperations.DeathList_Remove(&*cit);
			manager->actInst->RemoveAllCreatureReference(&*cit);
			manager->actInst->NPCList.erase(cit++);
			count++;
		}
		else
			++cit;
	}
#else
	ActiveInstance::CREATURE_IT cit;
	cit = manager->actInst->NPCList.begin();
	int count = 0;
	while(cit != manager->actInst->NPCList.end())
	{
		CreatureInstance *ptr = &cit->second;
		if((creatureID == 0 || creatureID == ptr->CreatureID) && (ptr->serverFlags & ServerFlags::IsNPC) && ptr->spawnGen == spawner)
		{
			MessageComponent msg;
			msg.SimulatorID = -1;
			msg.message = BCM_RemoveCreature;
			msg.param1 = ptr->CreatureID;
			msg.param2 = 0;
			msg.x = ptr->CurrentX;
			msg.z = ptr->CurrentZ;
			msg.actInst = manager->actInst;
			bcm.AddEventCopy(msg);

			manager->actInst->EraseAllCreatureReference(ptr);
			manager->actInst->NPCList.erase(cit++);
			manager->actInst->RebuildNPCList();
			count++;
		}
		else
			++cit;
	}
#endif
	manager->actInst->RebuildNPCList();

//	g_Log.AddMessageFormat("[DEBUG] RemoveSpawnPoint() removed %d spawned creatures", count);

	/*
	for(cit = inst->NPCList.begin(); cit != inst->NPCList.end(); ++cit)
	{
		if(cit->serverFlags & ServerFlags::IsNPC)
		{
			if(cit->spawnGen == spawner)
			{
				cit->spawnGen = NULL;
				cit->spawnTile = NULL;
				debugRemoved++;
			}
		}
	}
	g_Log.AddMessageFormat("[DEBUG] RemoveSpawnPoint() unattached %d creatures", debugRemoved);
	*/
}

void SpawnTile :: Despawn(int CreatureID)
{
	SPAWN_MAP::iterator it;
	for(it = activeSpawn.begin(); it != activeSpawn.end(); ++it)
	{
		RemoveSpawnPointCreature(&it->second, CreatureID);
	}
}

ActiveSpawner * SpawnTile :: GetActiveSpawner(int PropID)
{
	//Search for a prop and return a pointer to its spawner, if it exists.
	SPAWN_MAP::iterator it;
	it = activeSpawn.find(PropID);
	if(it == activeSpawn.end())
		return NULL;

	return &it->second;
}

//Check whether this tile meets the time criteria necessary to allow unloading from memory.
//Check whether any spawns are pending, such as bosses.  When running garbage checks, we
//don't want to delete a tile with a pending spawn because leaving and returning to an area
//could trigger a boss faster than its spawn timer is set to allow.
bool SpawnTile :: QualifyDelete(void)
{
	if(g_ServerTime < releaseTime)
		return false;

	SPAWN_MAP::iterator it;
	for(it = activeSpawn.begin(); it != activeSpawn.end(); it++)
	{
		if((it->second.nextSpawn != PlatformTime::MAX_TIME) && (g_ServerTime < it->second.nextSpawn))
		{
			//g_Log.AddMessageFormat("Failed timer check.");
			return false;
		}
	}
	return true;
}

void SpawnTile :: GetAttachedCreatures(int sceneryID, std::vector<int> &results)
{
	SPAWN_MAP::iterator it;
	it = activeSpawn.find(sceneryID);
	if(it == activeSpawn.end())
		return;

	results.insert(results.end(), it->second.attachedCreatureID.begin(), it->second.attachedCreatureID.end());
}


SpawnManager :: SpawnManager()
{
	actInst = NULL;
	nextGarbageCheck = 0;
	nextProcessTime = 0;
}

SpawnManager :: ~SpawnManager()
{
	Clear();
}

void SpawnManager :: Clear(void)
{
	spawnTiles.clear();
	genericSpawns.clear();
}

void SpawnManager :: SetInstancePointer(ActiveInstance *ptr)
{
	actInst = ptr;
}

// Retrieve a pointer to spawn tile that has the given coordinates.
// Search for an existing tile, or return a new one.
SpawnTile* SpawnManager :: GetTile(int tilePageX, int tilePageY)
{
	std::list<SpawnTile>::iterator it;
	for(it = spawnTiles.begin(); it != spawnTiles.end(); ++it)
		if(it->TileX == tilePageX && it->TileY == tilePageY)
			return &*it;

	SpawnTile newTile;
	newTile.TileX = tilePageX;
	newTile.TileY = tilePageY;
	newTile.sceneryLoaded = false;
	newTile.manager = this;
	spawnTiles.push_back(newTile);
	return &spawnTiles.back();
}

// Makes sure a tile is loaded.
SpawnTile * SpawnManager :: GenerateTile(int tilePageX, int tilePageY)
{
	SpawnTile *tile = GetTile(tilePageX, tilePageY);

	if(tile == NULL)
		return NULL;

	tile->releaseTime = g_ServerTime + GARBAGE_CHECK_DELAY;

	//static const int ZONE_TILE_SIZE = 1920;

	if(tile->sceneryLoaded == false)
	{
		//g_Log.AddMessageFormat("GenerateTile() %d, %d", tilePageX, tilePageY);
		//Convert the spawn tile page coordinates to zone actor coordinates,
		//then to zone tile coordinates.
		int x1 = tilePageX * SpawnTile::SPAWN_TILE_SIZE;
		int y1 = tilePageY * SpawnTile::SPAWN_TILE_SIZE;
		int x2 = x1 + SpawnTile::SPAWN_TILE_SIZE - 1; 
		int y2 = y1 + SpawnTile::SPAWN_TILE_SIZE - 1;

		//Changed from ZONE_TILE_SIZE since some zones use custom scenery page sizes.
		int stx1 = x1 / actInst->mZoneDefPtr->mPageSize; // ZONE_TILE_SIZE;
		int sty1 = y1 / actInst->mZoneDefPtr->mPageSize; // ZONE_TILE_SIZE;
		int stx2 = x2 / actInst->mZoneDefPtr->mPageSize; // ZONE_TILE_SIZE;
		int sty2 = y2 / actInst->mZoneDefPtr->mPageSize; // ZONE_TILE_SIZE;

		//Load up the spawn points from the scenery list.
		g_SceneryManager.GetThread("SpawnManager::GenerateTile");
		for(int px = stx1; px <= stx2; px++)
			for(int py = sty1; py <= sty2; py++)
				tile->LoadTileSpawnPoints(actInst->mZone, px, py, x1, y1, x2, y2);
		g_SceneryManager.ReleaseThread();

		tile->sceneryLoaded = true;
	}

	return tile;
}

void SpawnManager :: GenerateAreaTile(int tilePageX, int tilePageY)
{
	//Called to load the spawn tiles around a player.
	for(int x = tilePageX - SpawnTile::SPAWN_TILE_RANGE; x <= tilePageX + SpawnTile::SPAWN_TILE_RANGE; x++)
		for(int y = tilePageY - SpawnTile::SPAWN_TILE_RANGE; y <= tilePageY + SpawnTile::SPAWN_TILE_RANGE; y++)
			GenerateTile(x, y);
}

void SpawnManager :: RunProcessing(bool force)
{
	if((g_ServerTime < nextProcessTime) || (force == true))
		return;
	nextProcessTime = g_ServerTime + PROCESSING_INTERVAL;

	TILELIST_CONT searchList;

	ScanActivePlayerTiles(searchList);

	for(size_t i = 0; i < searchList.size(); i++)
	{
		SpawnTile *tile = GetTile(searchList[i].first, searchList[i].second);
		if(tile != NULL)
			tile->RunProcessing(actInst);
	}

	if(actInst->mZoneDefPtr->mInstance == false)
		RunGarbageCheck(searchList);
}

bool SpawnManager :: HasTileCoord(int x, int y, TILELIST_CONT &searchList)
{
	for(size_t i = 0; i < searchList.size(); i++)
		if(x == searchList[i].first && y == searchList[i].second)
			return true;
	return false;
}

bool SpawnManager :: NotifyKill(ActiveSpawner *sourceSpawner, int creatureID)
{
	//Updates the active spawner to reflect a kill.
	//Additionally, check whether the spawnpoint was marked for trigger behavior.
	//If true, the calling function should update the instance script system
	//that a particular creature has been killed.

	if(sourceSpawner != NULL)
	{
		sourceSpawner->UpdateSpawnDelay(0);
		sourceSpawner->refCount--;
		sourceSpawner->RemoveAttachedCreature(creatureID);

		if(sourceSpawner->spawnPackage != NULL)
		{
			if(sourceSpawner->spawnPackage->UniquePoints != 0)
				actInst->uniqueSpawnManager.ReRoll(sourceSpawner->spawnPackage->packageName, sourceSpawner->GetRespawnDelaySeconds());
		}
		
		if(sourceSpawner->spawnPoint->extraData == NULL)
			return false;
		if(sourceSpawner->spawnPoint->extraData->sequential != 0)
			return true;
	}

	return false;
}

void SpawnManager :: ScanActivePlayerTiles(TILELIST_CONT &outputList)
{
	size_t i;
	for(i = 0; i < actInst->PlayerListPtr.size(); i++)
	{
		int tx = actInst->PlayerListPtr[i]->CurrentX / SpawnTile::SPAWN_TILE_SIZE;
		int tz = actInst->PlayerListPtr[i]->CurrentZ / SpawnTile::SPAWN_TILE_SIZE;
		for(int z = tz - SpawnTile::SPAWN_TILE_RANGE; z <= tz + SpawnTile::SPAWN_TILE_RANGE; z++)
			for(int x = tx - SpawnTile::SPAWN_TILE_RANGE; x <= tx + SpawnTile::SPAWN_TILE_RANGE; x++)
				if(HasTileCoord(x, z, outputList) == false)
					outputList.push_back(TILE_COORD(x, z));
	}
}

void SpawnManager :: EnumAttachedCreatures(int sceneryID, int sceneryX, int sceneryZ, std::vector<int> &results)
{
	int tx = sceneryX / SpawnTile::SPAWN_TILE_SIZE;
	int tz = sceneryZ / SpawnTile::SPAWN_TILE_SIZE;
	SpawnTile *tile = GetTile(tx, tz);
	if(tile != NULL)
	{
		tile->GetAttachedCreatures(sceneryID, results);
	}
}

void SpawnManager :: RunGarbageCheck(TILELIST_CONT &activeTileList)
{
	if(nextGarbageCheck > g_ServerTime)
		return;

	std::list<SpawnTile>::iterator it;
	it = spawnTiles.begin();
	bool bFound = false;
	while(it != spawnTiles.end())
	{
		//Check whether the countdown timers are all finished. 
		if(it->QualifyDelete() == false)
		{
			++it;
			continue;
		}

		//Check whether this tile is within range of an active player.
		bFound = false;
		for(size_t i = 0; i < activeTileList.size(); i++)
		{
			if(activeTileList[i].first == it->TileX && activeTileList[i].second == it->TileY)
			{
				bFound = true;
				break;
			}
		}
		if(bFound == false)
		{
			//g_Log.AddMessageFormat("[DEBUG] Removing inactive spawn tile: %d, %d", it->TileX, it->TileY);
			it->RemoveAllAttachedCreature(actInst);
			spawnTiles.erase(it++);
		}
		else
		{
			++it;
		}
	}
	nextGarbageCheck = g_ServerTime + GARBAGE_CHECK_DELAY;
}

void SpawnManager :: UpdateSpawnPoint(SceneryObject *prop)
{
	int tx = (int)(prop->LocationX / SpawnTile::SPAWN_TILE_SIZE);
	int tz = (int)(prop->LocationZ / SpawnTile::SPAWN_TILE_SIZE);
	SpawnTile *tile = GetTile(tx, tz);
	if(tile != NULL)
		tile->UpdateSpawnPoint(prop);
}

void SpawnManager :: RemoveSpawnPoint(int PropID)
{
	// We don't have prop data to find a tile, so brute force
	// search and remove.
	std::list<SpawnTile>::iterator it;
	for(it = spawnTiles.begin(); it != spawnTiles.end(); ++it)
	{
		if(it->RemoveSpawnPoint(actInst, PropID) == true)
			g_Log.AddMessageFormat("Removing %d from %d, %d", PropID, it->TileX, it->TileY);
		//	return;
	}
}

void SpawnManager :: Despawn(int CreatureID)
{
	std::list<SpawnTile>::iterator it;
	for(it = spawnTiles.begin(); it != spawnTiles.end(); ++it)
		it->Despawn(CreatureID);
	std::list<int>::iterator cit = std::find(genericSpawns.begin(), genericSpawns.end(), CreatureID);
	if(cit != genericSpawns.end()) {
		actInst->RemoveNPCInstance(CreatureID);
		genericSpawns.erase(cit);
	}
}

int SpawnManager :: TriggerSpawn(int PropID, int forceCreatureDef, int forceFlags)
{
	//Used by the quest script system, forces a prop to spawn.
	ActiveSpawner *obj = NULL;
	std::list<SpawnTile>::iterator it;
	for(it = spawnTiles.begin(); it != spawnTiles.end(); ++it)
	{
		obj = it->GetActiveSpawner(PropID);
		if(obj != NULL)
		{
			CreatureInstance *inst = it->SpawnCreature(actInst, obj, forceCreatureDef, forceFlags);
			if(inst != NULL)
			{
				obj->OnSpawnSuccess();

				char buffer[100];
				int wpos = PrepExt_GeneralMoveUpdate(buffer, inst);
				actInst->LSendToLocalSimulator(buffer, wpos, inst->CurrentX, inst->CurrentZ);

				return inst->CreatureID;
			}
			g_Log.AddMessageFormat("Found spawner, but no creature for spawn of %d (%d, %d)", PropID, forceCreatureDef, forceFlags);
			return -1;
		}
	}

	g_Log.AddMessageFormat("No spawn of %d (%d, %d) because no active spawners found (maybe the prop is not in a tile that is yet loaded)", PropID, forceCreatureDef, forceFlags);
	return -1;
}

SpawnPackageDef :: SpawnPackageDef()
{
	Clear();
}

SpawnPackageDef :: ~SpawnPackageDef()
{
}

void SpawnPackageDef :: Clear(void)
{
	memset(packageName, 0, sizeof(packageName));
	spawnCount = 0;
	maxShares = DEFAULT_MAXSHARES;
	SpawnFlags = 0;
	memset(spawnID, 0, sizeof(spawnID));
	memset(spawnShare, 0, sizeof(spawnShare));
	divideOrient = 0;
	divideLocation = 0;
	divideShareThreshold = 0;
	isScriptCall = false;
	isSequential = false;
	loyaltyRadius = 0;
	spawnDelay = 0;
	wanderRadius = 0;

	UniquePoints = 0;
	numPointOverride = 0;
	memset(PointOverridePropID, 0, sizeof(PointOverridePropID));
	memset(PointOverrideCDef, 0, sizeof(PointOverrideCDef));
}

int SpawnPackageDef :: GetRandomSpawn(SceneryObject *spawnPoint)
{
	//Run a spawn check.
	
	//First check the overrides, otherwise proceed to normal rolls.
	for(int i = 0; i < numPointOverride; i++)
		if(PointOverridePropID[i] == spawnPoint->ID)
			return PointOverrideCDef[i];

	/* Sample spawn table, if 100 max shares.  For efficiency they should
	*  be sorted to have the most likely spawns first.
	*   CID   Share  ShareCheck
	*    100   25     1 to 25    (1+0 to 25)
	*    101   40     26 to 65   (1+25 to 25+40)
	*    102   35     66 to 100  (1+25+40 to 25+40+35
	*  - So if rolled a 70, return Creature ID 102.
	*/
	int minRoll = 1;
	int maxRoll = maxShares;
	if(divideLocation != 0)
	{
		if(spawnPoint != NULL)
		{
			//Horizontal cut divides north/south haves, vertical divides east/west 
			int compareLoc = 0;
			if(divideOrient == DIVIDER_HORIZONTAL)
				compareLoc = (int)spawnPoint->LocationZ;
			else if(divideOrient == DIVIDER_VERTICAL)
				compareLoc = (int)spawnPoint->LocationX;

			if(compareLoc < divideLocation)
				maxRoll = divideShareThreshold;
			else
				minRoll = 1 + divideShareThreshold;
		}
	}

	int rcheck = randint(minRoll, maxRoll);
	int base = 1;
	int a;
	for(a = 0; a < spawnCount; a++)
	{
		//Use '<' since '<=' was triggering the wrong spawn sometimes.
		//A spawn package with 20 shares, location divide of 10/10, was triggering the first half
		//even though was supposed to spawn the second half.
		//Since rcheck rolled 11, base+SpawnShare[0] = 1+10 = 11,
		//which triggered against a 11 to 20 min/max.
//			11 to 20, rcheck:11, base:1, spawnShare:10, a:0, spawncount:2
		if(rcheck >= base && rcheck < base + spawnShare[a])
		{
			return spawnID[a];
		}

		base += spawnShare[a];
	}

	return -1;
}

void SpawnPackageDef :: AddPointOverride(int propID, int creatureDefID)
{
	if(numPointOverride >= MAX_POINT_OVERRIDE)
	{
		g_Log.AddMessageFormat("Cannot add point more point overrides for [%s]", packageName);
		return;
	}

	PointOverridePropID[numPointOverride] = propID;
	PointOverrideCDef[numPointOverride] = creatureDefID;
	numPointOverride++;
}

//Debug function to test whether the given ID appears in the spawn list
bool SpawnPackageDef :: HasCreatureDef(int CreatureDefID)
{
	for(int s = 0; s < MAX_SPAWNCOUNT; s++)
		if(spawnID[s] == CreatureDefID)
			return true;
	return false;
}


SpawnPackageList :: SpawnPackageList()
{
	ZoneID = 0;
}

SpawnPackageList :: ~SpawnPackageList()
{
	Free();
}

void SpawnPackageList :: Free(void)
{
	defList.clear();
}

int SpawnPackageList :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Cannot open spawn package definition file: %s", filename);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;

	SpawnPackageDef newItem;
	int r;
	unsigned long DefaultFlags = 0;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				AddIfValid(newItem);
				newItem.SpawnFlags = static_cast<unsigned char>(DefaultFlags);
			}
			else if(strcmp(lfr.SecBuffer, "Name") == 0)
			{
				Util::SafeCopy(newItem.packageName, lfr.BlockToStringC(1, 0), sizeof(newItem.packageName));
			}
			else if(strcmp(lfr.SecBuffer, "Spawn") == 0)
			{
				if(newItem.spawnCount < newItem.MAX_SPAWNCOUNT)
				{
					int ID = lfr.BlockToIntC(1);
					if(ID > newItem.MAX_ID)
					{
						g_Log.AddMessageFormat("[WARNING] Spawn target definition cannot exceed %d (file: %s, line: %d)", newItem.MAX_ID, filename, lfr.LineNumber);
						ID = 0;
					}
					newItem.spawnID[newItem.spawnCount] = ID;
					newItem.spawnShare[newItem.spawnCount] = lfr.BlockToIntC(2);
					newItem.spawnCount++;
				}
				else
				{
					g_Log.AddMessageFormat("[WARNING] Cannot exceed %d spawning options (file: %s, line: %d)", newItem.MAX_SPAWNCOUNT, filename, lfr.LineNumber);
				}
			}
			else if(strcmp(lfr.SecBuffer, "Shares") == 0)
			{
				newItem.maxShares = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "Flags") == 0)
			{
				newItem.SpawnFlags = lfr.BlockToIntC(1) & newItem.FLAG_ALLBITS;
			}
			else if(strcmp(lfr.SecBuffer, "Divide") == 0)
			{
				newItem.divideLocation = lfr.BlockToIntC(1);
				newItem.divideOrient = lfr.BlockToIntC(2);
				newItem.divideShareThreshold = lfr.BlockToIntC(3);
			}
			else if(strcmp(lfr.SecBuffer, "loyaltyRadius") == 0)
			{
				newItem.loyaltyRadius = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "spawnDelay") == 0)
			{
				newItem.spawnDelay = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "wanderRadius") == 0)
			{
				newItem.wanderRadius = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "ScriptCall") == 0)
			{
				newItem.isScriptCall = lfr.BlockToBool(1);
			}
			else if(strcmp(lfr.SecBuffer, "Sequential") == 0)
			{
				newItem.isSequential = true;
			}
			else if(strcmp(lfr.SecBuffer, "PointOverride") == 0)
			{
				int propID = lfr.BlockToIntC(1);
				int creatureDef = lfr.BlockToIntC(2);
				newItem.AddPointOverride(propID, creatureDef);
			}
			else if(strcmp(lfr.SecBuffer, "UniquePoints") == 0)
			{
				newItem.UniquePoints = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "DefaultFlags") == 0)
			{
				DefaultFlags = lfr.BlockToIntC(1) & newItem.FLAG_ALLBITS;
			}
			else if(strcmp(lfr.SecBuffer, "Zone") == 0)
			{
				ZoneID = lfr.BlockToIntC(1);
			}
			else
				g_Log.AddMessageFormat("Unknown Spawn Package property [%s] in file [%s] on line [%d]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	AddIfValid(newItem);
	lfr.CloseCurrent();
	return defList.size();
}
void SpawnPackageList :: AddIfValid(SpawnPackageDef &newItem)
{
	if(newItem.packageName[0] == 0)
		return;

	defList.push_back(newItem);
	newItem.Clear();
}

SpawnPackageDef * SpawnPackageList :: GetPointerByName(const char *name)
{
	for(size_t i = 0; i < defList.size(); i++)
	{
		if(strcmp(defList[i].packageName, name) == 0)
			return &defList[i];
	}
	return NULL;
}

// This is a debug function to assist in determining whether an arbitrary creature is used by
// a spawn point.
SpawnPackageDef * SpawnPackageList :: HasCreatureDef(int CreatureDefID)
{
	for(size_t i = 0; i < defList.size(); i++)
		if(defList[i].HasCreatureDef(CreatureDefID) == true)
			return &defList[i];
	return NULL;
}

SpawnPackageManager :: SpawnPackageManager()
{
	Util::SafeCopy(nullSpawnPackage.packageName, "nullSpawnPackage", sizeof(nullSpawnPackage.packageName));
	fakeCreatureId = 60000;
}

SpawnPackageManager :: ~SpawnPackageManager()
{
	packageList.clear();
}

void SpawnPackageManager :: LoadFromFile(const char *subfolder, const char *filename)
{
	FileReader lfr;
	char FileName[256];
	Platform::GenerateFilePath(FileName, subfolder, filename);
	if(lfr.OpenText(FileName) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Could not open master spawn package list [%s].", FileName);
		return;
	}

	SpawnPackageList newItem;

	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		r = lfr.RemoveBeginningWhitespace();
		if(r > 0)
		{
			Platform::GenerateFilePath(FileName, subfolder, lfr.DataBuffer);
			if(newItem.LoadFromFile(FileName) >= 0)
			{
				packageList.push_back(newItem);
				newItem.defList.clear();
				newItem.ZoneID = 0;
			}
		}
	}
	lfr.CloseCurrent();
}

SpawnPackageDef * SpawnPackageManager :: GetPointerByName(const char *name)
{
	/*
	SPAWNPACKAGECONT::iterator it = packageList.find(zoneID);
	if(it == packageList.end())
		it = packageList[0];
	for(size_t i = 0; i < it->second.size(); i++)
	{
		retptr = it->second[i].GetPointerByName(name);
		if(retptr != NULL)
			return retptr;
	}
	return NULL;
	*/

	SpawnPackageDef *retptr = NULL;
	for(size_t i = 0; i < packageList.size(); i++)
	{
		retptr = packageList[i].GetPointerByName(name);
		if(retptr != NULL)
			return retptr;
	}

	return NULL;
}

void SpawnPackageManager :: EnumPackagesForCreature(int CreatureDefID, STRINGLIST &output)
{
	for(size_t i = 0; i < packageList.size(); i++)
	{
		SpawnPackageDef *pkg = packageList[i].HasCreatureDef(CreatureDefID);
		if(pkg != NULL)
		{
			bool bFound = false;
			for(size_t s = 0; s < output.size(); s++)
				if(output[s].compare(pkg->packageName) == 0)
				{
					bFound = true;
					break;
				}

			if(bFound == false)
				output.push_back(pkg->packageName);
		}
	}
}


/*
SpawnPackageList* SpawnPackageDef :: GetZone(int zoneID)
{
	return &packageList[zoneID].second;
}*/


CreatureSpawnDef :: CreatureSpawnDef()
{
	Clear();
}

CreatureSpawnDef :: ~CreatureSpawnDef()
{
}

void CreatureSpawnDef :: Clear()
{
	memset(this, 0, sizeof(CreatureSpawnDef));
	maxActive = DEFAULT_MAXACTIVE;
	despawnTime = DEFAULT_DESPAWNTIME;
	maxLeash = DEFAULT_MAXLEASH;
}

void CreatureSpawnDef :: copyFrom(CreatureSpawnDef *source)
{
	if(this == source)
		return;
	memcpy(spawnName, source->spawnName, sizeof(spawnName));
	leaseTime = source->leaseTime;
	memcpy(spawnPackage, source->spawnPackage, sizeof(spawnPackage));
	mobTotal = source->mobTotal;
	maxActive = source->maxActive;
	memcpy(aiModule, source->aiModule, sizeof(aiModule));
	maxLeash = source->maxLeash;
	loyaltyRadius = source->loyaltyRadius;
	wanderRadius = source->wanderRadius;
	despawnTime = source->despawnTime;
	sequential = source->sequential;
	memcpy(spawnLayer, source->spawnLayer, sizeof(spawnLayer));
	xpos = source->xpos;
	ypos = source->ypos;
	zpos = source->zpos;
	facing = source->facing;

	propCount = source->propCount;
	linkCount = source->linkCount;
	memcpy(sceneryName, source->sceneryName, sizeof(sceneryName));
	memcpy(prop, source->prop, sizeof(prop));
	memcpy(link, source->link, sizeof(link));
	innerRadius = source->innerRadius;
	outerRadius = source->outerRadius;
}

int CreatureSpawnDef :: GetLeashLength(void)
{
	//Sometimes this property isn't set correctly.
	//If zero, always return the default.
	//Update: turns out the Clear() function wasn't setting the default, I had changed
	//the unused file and not the actual working file.  Still leaving these modifications.
	if(maxLeash <= 0)
		return DEFAULT_MAXLEASH;

	return maxLeash;
}



UniqueSpawnEntry :: UniqueSpawnEntry()
{
	mSpawnTime = 0;
	mMaxSpawnCount = 0;
	mRandomIndex = 0;
	mRestartTime = 0;
}

size_t UniqueSpawnEntry :: GetPropIndex(int PropID)
{
	for(size_t i = 0; i < mPropID.size(); i++)
		if(mPropID[i] == PropID)
			return i;

	mPropID.push_back(PropID);
	return mPropID.size() - 1;
}

void UniqueSpawnEntry :: ReRoll(int durationSeconds)
{
	mRandomIndex = randint(0, mMaxSpawnCount - 1);
	mRestartTime = g_ServerTime + (durationSeconds * 1000);
}

UniqueSpawnManager :: UniqueSpawnManager()
{
}

UniqueSpawnManager :: ~UniqueSpawnManager()
{
	Clear();
}

void UniqueSpawnManager :: Clear(void)
{
	mEntryList.clear();
}

bool UniqueSpawnManager :: TestSpawn(const char *spawnPackageName, int maxSpawnPoints, int callPropID)
{
	if(maxSpawnPoints <= 1)
		return true;

	UniqueSpawnEntry &data = mEntryList[spawnPackageName];
	
	//Not ready to spawn again yet.
	if(data.mRestartTime > g_ServerTime)
		return false;

	//If a new entry, initialize it.
	if(data.mMaxSpawnCount == 0)
	{
		data.mMaxSpawnCount = maxSpawnPoints;
		data.ReRoll(0);
	}

	//Make sure the prop is in the list.
	size_t index = data.GetPropIndex(callPropID);
	if(index == data.mRandomIndex)
		return true;

	return false;
}

void UniqueSpawnManager :: ReRoll(const char *spawnPackageName, int durationSeconds)
{
	g_Log.AddMessageFormat("Rerolling: %s", spawnPackageName);
	ITERATOR it = mEntryList.find(spawnPackageName);
	if(it != mEntryList.end())
		it->second.ReRoll(durationSeconds);
}
