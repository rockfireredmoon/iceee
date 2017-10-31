#include "ZoneDef.h"
#include "FileReader.h"
#include "FileReader3.h"
#include "StringList.h"
#include "Config.h"
#include "PVP.h"
#include "Util.h"
#include "Audit.h"  //For scenery audits.
#include "InstanceScale.h"
#include "Report.h"
#include "Character.h"
#include "Globals.h"
#include "Instance.h"

ZoneDefManager g_ZoneDefManager;
ZoneBarrierManager g_ZoneBarrierManager;
EnvironmentCycleManager g_EnvironmentCycleManager;
GroveTemplateManager g_GroveTemplateManager;

ZoneEditPermission::ZoneEditPermission()
{
	Clear();
}

void ZoneEditPermission::Clear(void)
{
	mEditType = PERMISSION_TYPE_NONE;
	mID = 0;
	mX1 = 0;
	mY1 = 0;
	mX2 = 0;
	mY2 = 0;
}

void ZoneEditPermission::SaveToStream(FILE *output)
{
	switch(mEditType)
	{
	case PERMISSION_TYPE_ACCOUNT:
		fprintf(output, "BuildPermission=account,%d,%d,%d,%d,%d\r\n", mID, mX1, mY1, mX2, mY2);
		break;
	case PERMISSION_TYPE_CDEF:
		fprintf(output, "BuildPermission=cdef,%d,%d,%d,%d,%d\r\n", mID, mX1, mY1, mX2, mY2);
		break;
	case PERMISSION_TYPE_PLAYER:
		fprintf(output, "BuildPermission=player,%s,%d,%d,%d,%d\r\n", mCharacterName.c_str(), mX1, mY1, mX2, mY2);
		break;
	}
}

int ZoneEditPermission :: GetTypeIDByName(const std::string &name)
{
	if(name.compare("account") == 0)
		return PERMISSION_TYPE_ACCOUNT;
	if(name.compare("cdef") == 0)
		return PERMISSION_TYPE_CDEF;
	if(name.compare("player") == 0)
		return PERMISSION_TYPE_PLAYER;

	return PERMISSION_TYPE_NONE;
}

const char* ZoneEditPermission :: GetTypeNameByID(int ID)
{
	switch(ID)
	{
	case PERMISSION_TYPE_ACCOUNT:  return "account";
	case PERMISSION_TYPE_CDEF:     return "cdef";
	case PERMISSION_TYPE_PLAYER:   return "player";
	}
	return NULL;
}

void ZoneEditPermission::LoadFromConfig(const char *configStr)
{
	mEditType = PERMISSION_TYPE_NONE;

	if(configStr == NULL)
		return;

	std::string args = configStr;
	STRINGLIST paramList;
	Util::Split(args, ",", paramList);

	if(paramList.size() == 0)
		return;

	int editType = GetTypeIDByName(paramList[0]);
	if(editType == PERMISSION_TYPE_NONE)
		return;

	size_t argBase = 2;
	switch(editType)
	{
	case PERMISSION_TYPE_ACCOUNT:    //account,id,x1,y1,x2,y2
		mID = Util::GetInteger(paramList, 1);
		break;
	case PERMISSION_TYPE_CDEF:       //cdef,id,x1,y1,x2,y2
		mID = Util::GetInteger(paramList, 1);
		break;
	case PERMISSION_TYPE_PLAYER:     //cdef,name,x1,y1,x2,y2
		mCharacterName = Util::GetSafeString(paramList, 1);
		break;
	}
	mEditType = editType;
	mX1 = Util::GetInteger(paramList, argBase + 0);
	mY1 = Util::GetInteger(paramList, argBase + 1);
	mX2 = Util::GetInteger(paramList, argBase + 2);
	mY2 = Util::GetInteger(paramList, argBase + 3);
}


bool ZoneEditPermission::IsValid(void)
{
	switch(mEditType)
	{
	case PERMISSION_TYPE_ACCOUNT:  //fall through to cdef since both just use mID
	case PERMISSION_TYPE_CDEF:
		if(mID != 0)
			return true;
		break;
	case PERMISSION_TYPE_PLAYER:
		if(mCharacterName.size() > 0)
			return true;
		break;
	case PERMISSION_TYPE_NONE:  //fall through
	default:
		return false;
	}
	return false;
}

bool ZoneEditPermission::HasEditPermission(int accountID, int characterDefID, const char *characterName, float x, float z, int pageSize)
{
	switch(mEditType)
	{
	case PERMISSION_TYPE_CDEF:
		if(mID != characterDefID)
			return false;
		break;
	case PERMISSION_TYPE_ACCOUNT:
		if(mID != accountID)
			return false;
		break;
	case PERMISSION_TYPE_PLAYER:
		if(mCharacterName.compare(characterName) != 0)
			return false;
		break;
	}
	//If we get to this point, verify the location.

	//Need to cover the whole tile so offset another page, then subtract a single unit.
	//If the build permission contains: 0,0 to 1,1  (a 2x2 tile area) then with pageSize 1920
	//it will cover the following world object coordinates: 0,0 to 3839,3839
	//This also allows a single tile (0,0 to 0,0) to cover the full area (0,0 to 1919,1919)
	int x1 = mX1 * pageSize;
	int x2 = ((mX2 + 1) * pageSize) - 1;
	int checkX = static_cast<int>(x);
	if(checkX < x1 || checkX > x2)
		return false;

	int y1 = mY1 * pageSize;
	int y2 = ((mY2 + 1) * pageSize) - 1;
	int checkY = static_cast<int>(z);  //For world objects, Y axis is elevation so we convert from Z instead
	if(checkY < y1 || checkY > y2)
		return false;
	
	return true;
}

ZoneIndexEntry :: ZoneIndexEntry()
{
	mID = 0;
	mAccountID = 0;
}

void ZoneIndexEntry :: CopyFrom(const ZoneIndexEntry& other)
{
	mID = other.mID;
	mAccountID = other.mAccountID;
	mWarpName = other.mWarpName;
	mGroveName = other.mGroveName;
}

ZoneDefInfo :: ZoneDefInfo()
{
	Clear();
}

ZoneDefInfo :: ~ZoneDefInfo()
{
}

void ZoneDefInfo :: Clear(void)
{
	PendingChanges = 0;
	mID = 0;
	mAccountID = 0;
	mDesc.clear();
	mName.clear();
	mTerrainConfig.clear();
	mEnvironmentType.clear();
	mMapName.clear();
	mRegions.clear();

	mShardName.clear();
	mGroveName.clear();
	mWarpName.clear();
	DefX = 0;
	DefY = 0;
	DefZ = 0;
	mPageSize = DEFAULT_PAGESIZE;
	mMaxAggroRange = DEFAULT_MAXAGGRORANGE;
	mMaxLeashRange = DEFAULT_MAXLEASHRANGE;
	
	mReturnZone = 0;
	mPersist = false;
	mInstance = false;
	mGrove = false;
	mGuildHall = false;
	mArena = false;
	mAudit = false;
	mTimeOfDay = "";
	mEnvironmentCycle = false;

	mPlayerFilterType = FILTER_PLAYER_NONE;
	mPlayerFilterID.clear();

	mMode = PVP::GameMode::PVE_ONLY;

	mDropRateProfile.clear();

	mNextAuditSave = 0;
	mZoneAudit.Clear();
	mTileEnvironment.clear();
}

void ZoneDefInfo :: CopyFrom(const ZoneDefInfo& other)
{
	mID = other.mID;
	mAccountID = other.mAccountID;
	mDesc = other.mDesc;
	mName = other.mName;
	mTerrainConfig = other.mTerrainConfig;
	mEnvironmentType = other.mEnvironmentType;
	mMapName = other.mMapName;
	mRegions = other.mRegions;
	mTimeOfDay = other.mTimeOfDay;

	mShardName = other.mShardName;
	mGroveName = other.mGroveName;
	mWarpName = other.mWarpName;
	DefX = other.DefX;
	DefY = other.DefY;
	DefZ = other.DefZ;
	mPageSize = other.mPageSize;
	mReturnZone = other.mReturnZone;
	mPersist = other.mPersist;
	mInstance = other.mInstance;
	mGrove = other.mGrove;
	mGuildHall = other.mGuildHall;
	mArena = other.mArena;
	mMode = other.mMode;
	mAudit = other.mAudit;
	mEnvironmentCycle = other.mEnvironmentCycle;
	mMaxAggroRange = other.mMaxAggroRange;
	mMaxLeashRange = other.mMaxLeashRange;

	mPlayerFilterType = other.mPlayerFilterType;
	mPlayerFilterID.assign(other.mPlayerFilterID.begin(), other.mPlayerFilterID.end());

	mDropRateProfile = other.mDropRateProfile;

	mEditPermissions.assign(other.mEditPermissions.begin(), other.mEditPermissions.end());

	mTileEnvironment.clear();
	mTileEnvironment.insert(other.mTileEnvironment.begin(), other.mTileEnvironment.end());

	PendingChanges = other.PendingChanges;
}
	
void ZoneDefInfo :: SetDefaults(void)
{
	// Should be called before a ZoneDef is added into the ZoneDefManager list.
	// Makes sure to fall back on certain defaults if not explicitly defined in
	// the data file.
	if(mPageSize == 0)
		mPageSize = DEFAULT_PAGESIZE;
	if(mMaxAggroRange == 0)
		mMaxAggroRange = DEFAULT_MAXAGGRORANGE;

	if(mReturnZone == 0)
		mReturnZone = mID;

	if(mDropRateProfile.size() == 0)
	{
		if(mInstance == true)
			mDropRateProfile = DropRateProfileManager::INSTANCE;
		else
			mDropRateProfile = DropRateProfileManager::STANDARD;
	}

	if(IsPlayerGrove() == true)
		CreateDefaultGrovePermission();
}

bool ZoneDefInfo :: IsGuildHall(void)
{
	return mGuildHall;
}

bool ZoneDefInfo :: IsPlayerGrove(void)
{
	if(mGrove == true && mID >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
		return true;
	return false;
}

bool ZoneDefInfo :: IsPVPArena(void)
{
	return mArena;
}

// Free travel means that players can are free to warp into a particular zone if external criteria are met
// like sanctuary proximity.
bool ZoneDefInfo :: IsFreeTravel(void)
{
	if(IsPlayerGrove() == true)
		return true;
	if(IsGuildHall() == true)
		return true;
	if(IsPVPArena() == true)
		return true;

	return false;
}

bool ZoneDefInfo :: IsDungeon(void)
{
	if(mGuildHall == false && mGrove == false && mArena == false && mInstance == true)
		return true;

	return false;
}

bool ZoneDefInfo :: IsMobScalable(void)
{
	if(mInstance == true)
		return true;
	return false;
}


bool ZoneDefInfo :: HasDeathPenalty(void)
{
	if(mGrove == true || mArena == true)
		return false;
	return true;
}

bool ZoneDefInfo :: CanPlayerWarp(int CreatureDefID, int AccountID)
{
	//Check if a player is allowed to warp.
	//Always allow the account owner.
	if(mAccountID == AccountID)
		return true;
	if(mPlayerFilterType == FILTER_PLAYER_NONE)
		return true;

	static const int FILTER_PLAYER_WHITELIST = 1;   //Allow the players on the filter list, and only them.
	static const int FILTER_PLAYER_BLACKLIST = 2;   //Block the players on the filter list.

	if(mPlayerFilterType == FILTER_PLAYER_WHITELIST)
	{
		if(HasPlayerFilterID(CreatureDefID) == true)
			return true;
		return false;
	}
	if(mPlayerFilterType == FILTER_PLAYER_BLACKLIST)
	{
		if(HasPlayerFilterID(CreatureDefID) == true)
			return false;
		return true;
	}
	return false;
}

void ZoneDefInfo :: SetDropRateProfile(std::string profile) {
	mDropRateProfile = profile;
}

void ZoneDefInfo :: SaveToStream(FILE *output)
{
	fprintf(output, "[ENTRY]\r\n");
	Util::WriteString(output, "DESC", mDesc);
	fprintf(output, "ID=%d\r\n", mID);
	fprintf(output, "ACCOUNTID=%d\r\n", mAccountID);
	Util::WriteString(output, "NAME", mName);
	Util::WriteString(output, "TERRAINCONFIG", mTerrainConfig);
	Util::WriteString(output, "ENVIRONMENTTYPE", mEnvironmentType);
	Util::WriteString(output, "MAPNAME", mMapName);
	Util::WriteString(output, "REGIONS", mRegions);
	Util::WriteIntegerIfNot(output, "Mode", mPageSize, PVP::GameMode::PVE_ONLY);
	Util::WriteString(output, "ShardName", mShardName);
	Util::WriteString(output, "GroveName", mGroveName);
	Util::WriteString(output, "TimeOfDay", mTimeOfDay);
	Util::WriteString(output, "WarpName", mWarpName);
	Util::WriteString(output, "DropRateProfile", mDropRateProfile);

	if(mPersist == true)
		fprintf(output, "Persist=%d\r\n", mPersist);
	if(mAudit == true)
		fprintf(output, "Audit=%d\r\n", mAudit);

	fprintf(output, "Grove=%d\r\n", mGrove);
	fprintf(output, "EnvironmentCycle=%d\r\n", mEnvironmentCycle);

	Util::WriteIntegerIfNot(output, "PageSize", mPageSize, ZoneDefInfo::DEFAULT_PAGESIZE);
	Util::WriteIntegerIfNot(output, "MaxAggroRange", mMaxAggroRange, ZoneDefInfo::DEFAULT_MAXAGGRORANGE);
	Util::WriteIntegerIfNot(output, "PlayerFilterType", mPlayerFilterType, 0);
	Util::WriteIntegerList(output, "PlayerFilterID", mPlayerFilterID);

	std::map<EnvironmentTileKey, string>::iterator it;
	for (it = mTileEnvironment.begin(); it != mTileEnvironment.end(); ++it)
		fprintf(output, "TileEnvironment=%d,%d,%s\r\n", it->first.x, it->first.y, it->second.c_str());

	for(size_t i = 0; i < mEditPermissions.size(); i++)
		mEditPermissions[i].SaveToStream(output);

	fprintf(output, "DefLoc=%d,%d,%d\r\n", DefX, DefY, DefZ);
	fprintf(output, "\r\n");
}

std::string ZoneDefInfo :: GetTileEnvironment(int x, int y)
{
	// Convert to scenery tile (use /info scenerytile to help with this)
	int px = x / mPageSize;
	int py = y / mPageSize;
	if(x >= 0 && y >= 0) {
		std::map<EnvironmentTileKey, string>::iterator it;
		EnvironmentTileKey etk;
		etk.x = px;
		etk.y = py;
		it = mTileEnvironment.find(etk);
		if(it != mTileEnvironment.end()) {
			return it->second;
		}
	}
	return mEnvironmentType;
}

void ZoneDefInfo :: AddPlayerFilterID(int CreatureDefID, bool loadStage)
{
	for(size_t i = 0; i < mPlayerFilterID.size(); i++)
		if(mPlayerFilterID[i] == CreatureDefID)
			return;
	mPlayerFilterID.push_back(CreatureDefID);
	
	// If we're not adding players from the ZoneDef load stage, this was probably added by a command, so
	// mark a pending change.
	if(loadStage == false)
		PendingChanges++;
}

void ZoneDefInfo :: RemovePlayerFilter(int CreatureDefID)
{
	for(size_t i = 0; i < mPlayerFilterID.size(); i++)
	{
		if(mPlayerFilterID[i] == CreatureDefID)
		{
			mPlayerFilterID.erase(mPlayerFilterID.begin() + i);
			PendingChanges++;
			return;
		}
	}
}

void ZoneDefInfo :: ClearPlayerFilter(void)
{
	if(mPlayerFilterID.size() > 0)
	{
		mPlayerFilterID.clear();
		PendingChanges++;
	}
}

bool ZoneDefInfo :: HasPlayerFilterID(int CreatureDefID)
{
	for(size_t i = 0; i < mPlayerFilterID.size(); i++)
		if(mPlayerFilterID[i] == CreatureDefID)
			return true;
	return false;
}

bool ZoneDefInfo :: HasEditPermission(int accountID, int characterDefID, const char *characterName, float x, float z)
{
	//Quick hack since negative coordinates seem to cause glitched scenery tiles in the client and
	//infinite load screens.  I'm not sure yet why this is, but we can probably assume this area is
	//off limits anyway.
	if(x < 0.0F || z < 0.0F)
		return false;

	for(size_t i = 0; i < mEditPermissions.size(); i++)
	{
		if(mEditPermissions[i].HasEditPermission(accountID, characterDefID, characterName, x, z, DEFAULT_PAGESIZE) == true)
			return true;
	}
	
	//Put the account check here in case a custom account permission was set up in the list.
	if((mAccountID != 0) && (accountID == mAccountID))
		return true;
	return false;
}

void ZoneDefInfo :: UpdateGrovePermission(STRINGLIST &params)
{
	const char *action = Util::GetSafeString(params, 0);
	if(strcmp(action, "add") == 0)
	{
		int characterID = Util::GetInteger(params, 1);
		int x1 = Util::GetInteger(params, 2);
		int y1 = Util::GetInteger(params, 3);
		int x2 = Util::GetInteger(params, 4);
		int y2 = Util::GetInteger(params, 5);
		ZoneEditPermission p;
		p.mID = characterID;
		p.mX1 = x1;
		p.mY1 = y1;
		p.mX2 = x2;
		p.mY2 = y2;
		mEditPermissions.push_back(p);
		PendingChanges++;
	}
	else if(strcmp(action, "delete") == 0)
	{
		size_t index = static_cast<size_t>(Util::GetInteger(params, 1));
		if(index >= 0 && index < mEditPermissions.size())
		{
			mEditPermissions.erase(mEditPermissions.begin() + index);
			PendingChanges++;
		}
	}
}

std::string ZoneDefInfo :: GetDropRateProfile()
{
	if(mDropRateProfile.length() > 0)
		return mDropRateProfile;
	else {
		if(mInstance) {
			return "instance";
		}
		else
			return "standard";
	}
}

void ZoneDefInfo :: ChangeDefaultLocation(int newX, int newY, int newZ)
{
	DefX = newX;
	DefY = newY;
	DefZ = newZ;
	PendingChanges++;
}

void ZoneDefInfo :: ChangeShardName(const char *newName)
{
	mShardName = newName;
	PendingChanges++;
}

void ZoneDefInfo :: ChangeName(const char *newName)
{
	mName = newName;
	PendingChanges++;
}

void ZoneDefInfo :: ChangeEnvironment(const char *newEnvironment)
{
	mEnvironmentType = newEnvironment;
	PendingChanges++;
}

void ZoneDefInfo :: ChangeEnvironmentUsage(void)
{
	mEnvironmentCycle = !mEnvironmentCycle;
	PendingChanges++;
}

bool ZoneDefInfo :: QualifyDelete(void)
{
	if(PendingChanges > 0)
		return false;

	if(mID >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
		return true;

	return false;
}

std::string ZoneDefInfo :: GetTimeOfDay() {
	//If the environment time string is null, attempt to find the active time for the
	//current zone, if applicable.
	if(mEnvironmentCycle == true)
		return g_EnvironmentCycleManager.GetCurrentTimeOfDay();
	else if(mTimeOfDay.length() > 0)
		return mTimeOfDay;
	return "Day";
}


bool ZoneDefInfo :: AllowSceneryAudits(void)
{
	if(g_Config.SceneryAuditAllow == false)
		return false;

	//Anything set to explicitly audit.
	if(mAudit == true)
		return true;

	//Don't audit groves by default.
	if(mGrove == true)
		return false;

	//If we get here we're probably an official zone, so audit it.
	return true;
}

void ZoneDefInfo :: AuditScenery(const char *username, int zone, const SceneryObject *sceneryObject, int opType)
{
	if(sceneryObject == NULL)
		return;
	if(AllowSceneryAudits() == false)
		return;

	mZoneAudit.PerformSceneryAudit(username, zone, sceneryObject, opType);
}

void ZoneDefInfo :: AutosaveAudits(bool force)
{
	if(g_ServerTime < mNextAuditSave && force == false)
		return;

	mZoneAudit.AutosaveAudits();
	mNextAuditSave = g_ServerTime + g_Config.SceneryAuditDelay;
}

void ZoneDefInfo :: CreateDefaultGrovePermission(void)
{
	//Can't set up a permission if it doesn't have an owner.
	if(mAccountID == 0)
		return;

	//Check to see if we already have a permission set up for the account.
	for(size_t i = 0; i < mEditPermissions.size(); i++)
	{
		if((mEditPermissions[i].mEditType == ZoneEditPermission::PERMISSION_TYPE_ACCOUNT) && (mEditPermissions[i].mID == mAccountID))
			return;
	}

	//If we get here, a default permission should be added.
	ZoneEditPermission entry;
	entry.mEditType = ZoneEditPermission::PERMISSION_TYPE_ACCOUNT;
	entry.mID = mAccountID;
	entry.mX1 = 0;
	entry.mY1 = 0;
	entry.mX2 = 2;  //Basic defaults for empty grove.
	entry.mY2 = 2;

	//Get build area defaults from the terrain template, if the template is known.
	const GroveTemplate *gt = g_GroveTemplateManager.GetTemplateByTerrainCfg(mTerrainConfig.c_str());
	if(gt != NULL)
	{
		entry.mX1 = gt->mTileX1;
		entry.mX2 = gt->mTileX2;
		entry.mY1 = gt->mTileY1;
		entry.mY2 = gt->mTileY2;
	}
	mEditPermissions.insert(mEditPermissions.begin(), entry);
	PendingChanges++;
}

ZoneDefManager :: ZoneDefManager()
{
	mNextAutosaveTime = 0;
	mNextZoneUnload = 0;
	NextZoneID = GROVE_ZONE_ID_DEFAULT;
	cs.Init();
	cs.SetDebugName("CS_ZONEDEFMGR");
}

ZoneDefManager :: ~ZoneDefManager()
{
	Free();
}

void ZoneDefManager :: Free(void)
{
	CheckAutoSave(true);
	mZoneList.clear();
}

int ZoneDefManager :: LoadFile(const char *fileName)
{
	//Note: the official grove file is loaded first, then the custom grove file.
	//This should point here. 
	FileReader lfr;
	if(lfr.OpenText(fileName) != Err_OK)
	{
		g_Log.AddMessageFormat("Error: Could not open file [%s]", fileName);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;
	ZoneDefInfo newItem;

	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			r = lfr.BreakUntil("=", '=');  //Don't use SingleBreak since we won't be able to re-split later.
			lfr.BlockToStringC(0, Case_Upper);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.mID != 0)
				{
					newItem.SetDefaults();
					mZoneList[newItem.mID].CopyFrom(newItem);
					newItem.Clear();
				}
			}
			else
			{
				if(strcmp(lfr.SecBuffer, "ID") == 0)
					newItem.mID = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "ACCOUNTID") == 0)
					newItem.mAccountID = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "DESC") == 0)
					newItem.mDesc = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "NAME") == 0)
					newItem.mName = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "TERRAINCONFIG") == 0)
					newItem.mTerrainConfig = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "ENVIRONMENTTYPE") == 0)
					newItem.mEnvironmentType = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "TIMEOFDAY") == 0)
					newItem.mTimeOfDay = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "MAPNAME") == 0)
					newItem.mMapName = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "REGIONS") == 0)
					newItem.mRegions = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "SHARDNAME") == 0)
					newItem.mShardName = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "GROVENAME") == 0)
					newItem.mGroveName = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "WARPNAME") == 0)
					newItem.mWarpName = lfr.BlockToStringC(1, 0);
				else if(strcmp(lfr.SecBuffer, "MODE") == 0)
					newItem.mMode = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "PAGESIZE") == 0)
					newItem.mPageSize = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "MAXAGGRORANGE") == 0)
					newItem.mMaxAggroRange = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "MAXLEASHRANGE") == 0)
					newItem.mMaxLeashRange = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "DEFLOC") == 0)
				{
					lfr.MultiBreak("=,"); //Re-split for this particular data.
					newItem.DefX = lfr.BlockToIntC(1);
					newItem.DefY = lfr.BlockToIntC(2);
					newItem.DefZ = lfr.BlockToIntC(3);
				}
				else if(strcmp(lfr.SecBuffer, "PERSIST") == 0)
					newItem.mPersist = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "INSTANCE") == 0)
					newItem.mInstance = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "GROVE") == 0)
					newItem.mGrove = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "GUILDHALL") == 0)
					newItem.mGuildHall = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "ARENA") == 0)
					newItem.mArena = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "AUDIT") == 0)
					newItem.mAudit = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "DROPRATEPROFILE") == 0)
					newItem.SetDropRateProfile(lfr.BlockToStringC(1, 0));
				else if(strcmp(lfr.SecBuffer, "ENVIRONMENTCYCLE") == 0)
					newItem.mEnvironmentCycle = lfr.BlockToBoolC(1);
				else if(strcmp(lfr.SecBuffer, "PLAYERFILTERTYPE") == 0)
					newItem.mPlayerFilterType = lfr.BlockToIntC(1);
				else if(strcmp(lfr.SecBuffer, "PLAYERFILTERID") == 0)
				{
					r = lfr.MultiBreak("=,"); //Re-split for this particular data.
					for(int s = 1; s < r; s++)
						newItem.AddPlayerFilterID(lfr.BlockToIntC(s), true);
				}
				else if(strcmp(lfr.SecBuffer, "BUILDPERMISSION") == 0)
				{
					ZoneEditPermission perm;
					perm.LoadFromConfig(lfr.BlockToStringC(1, 0));
					if(perm.IsValid() == true)
						newItem.mEditPermissions.push_back(perm);
				}
				else if(strcmp(lfr.SecBuffer, "TILEENVIRONMENT") == 0)
				{
					lfr.MultiBreak("=,"); //Re-split for this particular data.
					EnvironmentTileKey etk;
					etk.x = lfr.BlockToIntC(1);
					etk.y = lfr.BlockToIntC(2);
					newItem.mTileEnvironment[etk] = lfr.BlockToStringC(3, 0);
				}
				else if(strcmp(lfr.SecBuffer, "AREAENVIRONMENT") == 0)
				{
					lfr.MultiBreak("=,"); //Re-split for this particular data.
					int tx = lfr.BlockToIntC(1);
					int ty = lfr.BlockToIntC(2);
					int tw = lfr.BlockToIntC(3);
					int th = lfr.BlockToIntC(4);
					for(int iy = 0; iy < th ; iy++)
					{
						for(int ix = 0; ix < tw ; ix++)
						{
							EnvironmentTileKey etk;
							etk.x = tx + ix;
							etk.y = ty + iy;

							newItem.mTileEnvironment[etk] = lfr.BlockToStringC(3, 0);newItem.mTileEnvironment[etk] = lfr.BlockToStringC(5, 0);
						}
					}
				}
				else
					g_Log.AddMessageFormat("Unknown identifier [%s] while reading from file %s.", lfr.SecBuffer, fileName);
			}
		}
	}
	lfr.CloseCurrent();

	if(newItem.mID != 0)
	{
		newItem.SetDefaults();
		mZoneList[newItem.mID].CopyFrom(newItem);
	}
	g_Log.AddMessageFormat("Loaded zone file [%s]", fileName);
	return 0;
}

ZoneDefInfo * ZoneDefManager :: ResolveZoneDef(int ID)
{
	ZONEDEF_ITERATOR it = mZoneList.find(ID);
	if(it == mZoneList.end())
		return LoadZoneDef(ID);
	return &it->second;
}

ZoneDefInfo * ZoneDefManager :: LoadZoneDef(int ID)
{
	//Official zones are static and can't be loaded at runtime..
	if(ID < GROVE_ZONE_ID_DEFAULT)
	{
		g_Log.AddMessageFormat("[CRITICAL] Could not find ZoneDef: %d", ID);
		return NULL;
	}

	std::string fileName;
	GetZoneFileName(ID, fileName);
	LoadFile(fileName.c_str());

	//Search to verify that the entry exists.
	ZONEDEF_ITERATOR it;
	it = mZoneList.find(ID);
	if(it == mZoneList.end())
	{
		g_Log.AddMessageFormat("[ERROR] LoadZoneDef failed to load zone.");
		return NULL;
	}
	return &it->second;
}

void ZoneDefManager :: GetZoneFileName(int ID, std::string &outStr)
{
	outStr.clear();

	char fileName[256];
	Util::SafeFormat(fileName, sizeof(fileName), "ZoneDef\\%d.txt", ID);
	Platform::FixPaths(fileName);
	outStr = fileName;
}

void ZoneDefManager :: LoadData(void)
{
	char fileName[256];

	//Load static entries (usually offical instances, never modified during run time)
	LoadFile(Platform::GenerateFilePath(fileName, "Data", "ZoneDef.txt"));


	Platform::GenerateFilePath(mDataFileIndex, "Dynamic", "ZoneIndex.txt");
	LoadIndex();

	g_Log.AddMessageFormat("Loaded %d ZoneDef.", mZoneList.size());
	g_Log.AddMessageFormat("Loaded %d ZoneIndex.", mZoneIndex.size());

	InitDefaultZoneIndex();
}

ZoneDefInfo * ZoneDefManager :: GetPointerByID(int ID)
{
	ZoneDefInfo *retptr = NULL;
	cs.Enter("ZoneDefManager::GetPointerByID");
	retptr = ResolveZoneDef(ID);


	/* OBSOLETE
	std::list<ZoneDefInfo>::iterator it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
		if(it->mID == ID)
		{
			retptr = &*it;
			break;
		}
	*/
	
	cs.Leave();
	return retptr;
}

ZoneDefInfo * ZoneDefManager :: GetPointerByPartialWarpName(const char *name)
{
	ZoneDefInfo *retptr = NULL;
	cs.Enter("ZoneDefManager::GetPointerByPartialWarpName");

	ZONEINDEX_ITERATOR it;
	for(it = mZoneIndex.begin(); it != mZoneIndex.end(); ++it)
	{
		if(it->second.mWarpName.find(name) != string::npos)
		{
			retptr = ResolveZoneDef(it->first);
			break;
		}
	}
			
	/* OBSOLETE
	std::list<ZoneDefInfo>::iterator it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
		if(it->mWarpName.find(name) != string::npos)
		{
			retptr = &*it;
			break;
		}
		*/
	cs.Leave();

	return retptr;
}

ZoneDefInfo * ZoneDefManager :: GetPointerByExactWarpName(const char *name)
{
	ZoneDefInfo *retptr = NULL;
	cs.Enter("ZoneDefManager::GetPointerByExactWarpName");

	ZONEINDEX_ITERATOR it;
	for(it = mZoneIndex.begin(); it != mZoneIndex.end(); ++it)
	{
		if(it->second.mWarpName.compare(name) == 0)
		{
			retptr = ResolveZoneDef(it->first);
			break;
		}
	}
	/* OBSOLETE
	std::list<ZoneDefInfo>::iterator it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
		if(it->mWarpName.compare(name) == 0)
		{
			retptr = &*it;
			break;
		}
	*/

	cs.Leave();

	return retptr;
}

ZoneDefInfo * ZoneDefManager :: GetPointerByGroveName(const char *name)
{
	ZoneDefInfo *retptr = NULL;
	cs.Enter("ZoneDefManager::GetPointerByGroveName");


	ZONEINDEX_ITERATOR it;
	for(it = mZoneIndex.begin(); it != mZoneIndex.end(); ++it)
	{
		if(it->second.mGroveName.compare(name) == 0)
		{
			retptr = ResolveZoneDef(it->first);
			break;
		}
	}

	/* OBSOLETE
	std::list<ZoneDefInfo>::iterator it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
		if(it->mGroveName.compare(name) == 0)
		{
			retptr = &*it;
			break;
		}
		*/
	cs.Leave();

	return retptr;
}

int ZoneDefManager :: GetNewZoneID(void)
{
	int rval = NextZoneID;
	NextZoneID += GROVE_ZONE_ID_INCREMENT;
	return rval;
}

int ZoneDefManager :: CreateZone(ZoneDefInfo &newZone)
{
	cs.Enter("ZoneDefManager::CreateZone");
	int newZoneID = GetNewZoneID();

	newZone.mID = newZoneID;
	//Flag for the next autosave.
	newZone.PendingChanges = 1;

	InsertZone(newZone, true);

	cs.Leave();
	return newZoneID;
}

int ZoneDefManager :: CreateGrove(int accountID, const char *grovename)
{
	cs.Enter("ZoneDefManager::CreateGrove");
	int newZoneID = GetNewZoneID();
	ZoneDefInfo newZone;

	newZone.mAccountID = accountID;
	newZone.mID = newZoneID;
	newZone.mName = "Grove";
	newZone.mTerrainConfig = "Terrain-Blend#Terrain-Blend.cfg";
	newZone.mEnvironmentType = "CloudyDay";
	newZone.mTimeOfDay = "";

	newZone.mPageSize = DEFAULT_GROVE_PAGE_SIZE;

	newZone.mShardName = grovename;
	newZone.mGroveName = grovename;
	newZone.mWarpName = grovename;
	newZone.mWarpName.append("1");
	newZone.DefX = 2958;
	newZone.DefY = 260;
	newZone.DefZ = 3821;
	newZone.mGrove = true;
	newZone.mGuildHall = false;

	//Flag for the next autosave.
	newZone.PendingChanges = 1;

	InsertZone(newZone, true);

	cs.Leave();
	return newZoneID;
}

//Copy zone data into the active table.  If creating an entirely new grove, an index
//can be created for it.
void ZoneDefManager :: InsertZone(const ZoneDefInfo& newZone, bool createIndex)
{
	//Copy into the resident zone list.
	mZoneList[newZone.mID].CopyFrom(newZone);

	//Create the index entry.
	if(createIndex == true)
	{
		UpdateZoneIndex(newZone.mID, newZone.mAccountID, newZone.mWarpName.c_str(), newZone.mGroveName.c_str(), true);
		/*
		ZoneIndexEntry &entry = mZoneIndex[newZone.mID];
		entry.mID = newZone.mID;
		entry.mGroveName = newZone.mGroveName;
		entry.mWarpName = newZone.mWarpName;

		ZoneListChanges.AddChange();
		*/
	}
}

int ZoneDefManager :: CheckAutoSave(bool force)
{
	if(g_ServerTime < (unsigned long)mNextAutosaveTime && force == false)
		return 0;

	int saveOps = 0;  //Number of save operations completed.  The session autosave will want to know if it needs to update in case of zone information.

	if(ZoneIndexChanges.PendingChanges > 0)
	{
		//If the save failed, we'll probably want to retry later so keep the pending status.
		if(SaveIndex() == true)
		{
			ZoneIndexChanges.ClearPending();
			saveOps++;
		}
	}

	std::string fileName;
	ZONEDEF_ITERATOR it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
	{
		ZoneDefInfo *def = &it->second;

		def->AutosaveAudits(force);  //Audits doesn't use the pending stuff, so we'll just toss it in here.

		if(def->PendingChanges == 0)
			continue;

		GetZoneFileName(def->mID, fileName);
		FILE *output = fopen(fileName.c_str(), "wb");
		if(output == NULL)
		{
			g_Log.AddMessageFormat("[ERROR] Could not open file [%s] for saving", fileName.c_str());
			continue;
		}
		def->SaveToStream(output);
		def->PendingChanges = 0;
		fclose(output);
		saveOps++;
	}
	return saveOps;
}

int ZoneDefManager :: EnumerateGroves(int searchAccountID, int characterDefId, std::vector<std::string>& groveList)
{
	cs.Enter("ZoneDefManager::EnumerateGroves");
	ZONEINDEX_ITERATOR it;
	CharacterData  *cdata = g_CharacterManager.GetPointerByID(characterDefId);

	for(it = mZoneIndex.begin(); it != mZoneIndex.end(); ++it)
	{
		if(it->second.mAccountID == searchAccountID)
		{
			groveList.push_back(it->second.mWarpName);
		}
		else
		{
			// Check if the zone is a guild hall for a guild the character is in
			for(uint i = 0 ; i < cdata->guildList.size(); i++)
			{
				int guildDefID = cdata->guildList[0].GuildDefID;
				GuildDefinition *gDef = g_GuildManager.GetGuildDefinition(guildDefID);
				if(gDef == NULL) {
					g_Log.AddMessageFormat("Guild definition %d does not exist", guildDefID);
				}
				else {
					if(it->second.mID == gDef->guildHallZone && cdata->IsInGuildAndHasValour(guildDefID, 0))
					{
						groveList.push_back(it->second.mWarpName);
						break;
					}
				}
			}
		}
	}
	cs.Leave();
	return static_cast<int>(groveList.size());
}

int ZoneDefManager :: EnumerateArenas(std::vector<std::string>& arenaList)
{
	ZONEDEF_ITERATOR it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
	{
		if(it->second.mArena == true)
			arenaList.push_back(it->second.mWarpName);
	}
	return static_cast<int>(arenaList.size());
}

void ZoneDefManager :: UpdateGroveAccountID(const char *groveName, int newAccountID)
{
	ZONEDEF_ITERATOR it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
	{
		if(it->second.mGrove == false)
			continue;
		std::string name = it->second.mWarpName;
		size_t pos = name.find_first_of("0123456789");
		if(pos != string::npos)
			name.erase(pos);
		if(name.compare(groveName) == 0)
		{
			g_Log.AddMessageFormat("Resetting grove [ID:%d, Name:%s] account ID from %d to %d", it->second.mID, it->second.mWarpName.c_str(), it->second.mAccountID, newAccountID);
			it->second.mAccountID = newAccountID;
			it->second.mGroveName = groveName;
			ZoneListChanges.AddChange();
		}
	}
}

void ZoneDefManager :: UpdateZoneIndex(int zoneID, int accountID, const char *warpName, const char *groveName, bool allowCreate)
{
	ZoneIndexEntry *entry = NULL;
	if(allowCreate == true)
	{
		entry = &mZoneIndex[zoneID];
	}
	else
	{
		ZONEINDEX_ITERATOR it = mZoneIndex.find(zoneID);
		if(it != mZoneIndex.end())
			entry = &it->second;
	}
	if(entry != NULL)
	{
		g_Log.AddMessageFormat("UpdateZoneIndex() original: [%d] [%d] [%s] [%s]", entry->mID, entry->mAccountID, entry->mWarpName.c_str(), entry->mGroveName.c_str());
		entry->mID = zoneID;
		entry->mAccountID = accountID;
		entry->mWarpName = warpName;
		entry->mGroveName = groveName;
		ZoneIndexChanges.AddChange();
		g_Log.AddMessageFormat("UpdateZoneIndex() updated: [%d] [%d] [%s] [%s]", entry->mID, entry->mAccountID, entry->mWarpName.c_str(), entry->mGroveName.c_str());
	}
}


void ZoneDefManager :: NotifyConfigurationChange(void)
{
	ZoneListChanges.AddChange();
}

bool ZoneDefManager :: ZoneUnloadReady(void)
{
	if(g_ServerTime >= (unsigned long)mNextZoneUnload)
	{
		mNextZoneUnload = g_ServerTime + ZONE_UNLOAD_DELAY;
		return true;
	}
	return false;
}

void ZoneDefManager :: UnloadInactiveZones(std::vector<int>& activeZones)
{
	ZONEDEF_ITERATOR it;
	it = mZoneList.begin();
	while(it != mZoneList.end())
	{
		bool has = false;
		for(size_t i = 0; i < activeZones.size(); i++)
		{
			if(activeZones[i] == it->second.mID)
			{
				has = true;
				break;
			}
		}
		if((has == false) && (it->second.QualifyDelete() == true))
		{
			g_Log.AddMessageFormat("Unloading inactive zone def: %d (%s)", it->second.mID, it->second.mWarpName.c_str());
			mZoneList.erase(it++);
		}
		else
			++it;
	}
}

bool ZoneDefManager :: SaveIndex(void)
{
	FILE *output = fopen(mDataFileIndex.c_str(), "wb");
	if(output == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] SaveIndex() could not open file [%s]", mDataFileIndex.c_str());
		return false;
	}

	ZONEINDEX_ITERATOR it;
	for(it = mZoneIndex.begin(); it != mZoneIndex.end(); ++it)
		fprintf(output, "%d;%d;%s;%s\r\n", it->second.mID, it->second.mAccountID, it->second.mWarpName.c_str(), it->second.mGroveName.c_str());
	fclose(output);
	return true;
}

void ZoneDefManager :: LoadIndex(void)
{
	FileReader3 fr;
	if(fr.OpenFile(mDataFileIndex.c_str()) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] LoadIndex() could not open file [%s]", mDataFileIndex.c_str());
		return;
	}

	fr.SetCommentChar(0); //Clear the default semicolon delimiter
	
	mZoneIndex.clear();

	while(fr.Readable() == true)
	{
		fr.ReadLine();
		int r = fr.MultiBreak(";");
		if(r >= 1)
		{
			ZoneIndexEntry entry;
			entry.mID = fr.BlockToIntC(0);
			entry.mAccountID = fr.BlockToIntC(1);
			entry.mWarpName = fr.BlockToStringC(2);
			entry.mGroveName = fr.BlockToStringC(3);
			mZoneIndex[entry.mID].CopyFrom(entry);
		}
	}
	fr.CloseFile();
}

// Merge the default official ZoneDef entries (overworld/dungeons/etc) into the zone index.
// This function helps keep the base part of the zone index up to date without needing to
// manually adjust it except in rare cases.
void ZoneDefManager :: InitDefaultZoneIndex(void)
{
	ZONEDEF_ITERATOR it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
	{
		ZoneIndexEntry entry;
		entry.mID = it->second.mID;
		entry.mAccountID = it->second.mAccountID;
		entry.mWarpName = it->second.mWarpName;
		entry.mGroveName = it->second.mGroveName;
		mZoneIndex[entry.mID].CopyFrom(entry);
	}
}

void ZoneDefManager :: GenerateReportActive(ReportBuffer &report)
{
	report.AddLine("ZONEDEFS (currently loaded groves)");
	ZONEDEF_ITERATOR it;
	for(it = mZoneList.begin(); it != mZoneList.end(); ++it)
	{
		ZoneDefInfo *zd = &it->second;
		if(zd->IsPlayerGrove() == false)
			continue;
		report.AddLine("Name:%s (%s) ID:%d", zd->mName.c_str(), zd->mShardName.c_str(), zd->mID);
		if(zd->PendingChanges > 0)
			report.AddLine("Has pending changes.");
		report.AddLine(NULL);
	}
}

MapBarrierPoint :: MapBarrierPoint()
{
	Clear();
}

void MapBarrierPoint :: Clear(void)
{
	x1 = 0;
	x2 = 0;
	z1 = 0;
	z2 = 0;
}

bool MapBarrierPoint :: isValid(void)
{
	return ((x1 + x2 + z1 + z2) != 0);
}

void ZoneBarrierManager :: AddEntry(int zoneID, MapBarrierPoint &data)
{
	zoneList[zoneID].push_back(data);
}

bool ZoneBarrierManager :: CheckCollision(int zoneID, int &x, int &z)
{
	BARRIER_CONT::iterator it = zoneList.find(zoneID);
	if(it == zoneList.end())
		return false;

	for(size_t i = 0; i < it->second.size(); i++)
	{
		if(x < it->second[i].x1)
			continue;
		if(x > it->second[i].x2)
			continue;
		if(z < it->second[i].z1)
			continue;
		if(z > it->second[i].z2)
			continue;

		int test[4];
		test[0] = abs(x - it->second[i].x1);
		test[1] = abs(x - it->second[i].x2);
		test[2] = abs(z - it->second[i].z1);
		test[3] = abs(z - it->second[i].z2);

		int lowest = 0;
		int findex = -1;
		for(int index = 0; index < 4; index++)
		{
			if(test[index] < lowest || findex == -1)
			{
				findex = index;
				lowest = test[index];
			}
		}
		if(findex == 0)
			x = it->second[i].x1 - 1;
		else if(findex == 1)
			x = it->second[i].x2 + 1;
		else if(findex == 2)
			z = it->second[i].z1 - 1;
		else if(findex == 3)
			z = it->second[i].z2 + 1;

		/*
		int c1, c2;
		c1 = abs(x - it->second[i].x1);
		c2 = abs(x - it->second[i].x2);
		if(c1 <= c2)
			x = it->second[i].x1 - 1;
		else
			x = it->second[i].x2 + 1;

		c1 = abs(z - it->second[i].z1);
		c2 = abs(z - it->second[i].z2);
		if(c1 <= c2)
			z = it->second[i].z1 - 1;
		else
			z = it->second[i].z2 + 1;
		*/
		return true;
	}
	return false;
}

void ZoneBarrierManager :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not load Zone Barrier file: %s", filename); 
		return;
	}
	lfr.CommentStyle = Comment_Semi;

	int curZone = 0;
	MapBarrierPoint newItem;
	newItem.Clear();
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.isValid())
				{
					AddEntry(curZone, newItem);
					newItem.Clear();
				}
			}
			else if(strcmp(lfr.SecBuffer, "1") == 0)
			{
				newItem.x1 = lfr.BlockToIntC(1);
				newItem.z1 = lfr.BlockToIntC(2);
			}
			else if(strcmp(lfr.SecBuffer, "2") == 0)
			{
				newItem.x2 = lfr.BlockToIntC(1);
				newItem.z2 = lfr.BlockToIntC(2);
			}
			else if(strcmp(lfr.SecBuffer, "Zone") == 0)
				curZone = lfr.BlockToIntC(1);
			else
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s] on line %d", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	lfr.CloseCurrent();
	if(newItem.isValid())
		AddEntry(curZone, newItem);
}

int ZoneBarrierManager :: GetLoadedCount(void)
{
	BARRIER_CONT::iterator it;
	size_t count = 0;
	for(it = zoneList.begin(); it != zoneList.end(); ++it)
		count += it->second.size();
	return static_cast<int>(count);
}

EnvironmentCycleManager :: EnvironmentCycleManager()
{
	//Environment default starts at day.
	ApplyConfig("Day=7200&Sunset=3600&Night=7200&Sunrise=3600");
}

EnvironmentCycleManager :: ~EnvironmentCycleManager()
{
}

void EnvironmentCycleManager :: ApplyConfig(const char *str)
{
	//Config expects a list of keys and values of TimeString=Seconds
	//Ex: Sunrise=3600&Day=7200&Sunset=3600&Night=3600

	mConfig = str;

	mCycleStrings.clear();
	mCycleTimes.clear();
	mCurrentCycleIndex = 0;
	mNextUpdateTime = g_ServerTime;

	STRINGLIST pairs;
	STRINGLIST keyval;
	Util::Split(mConfig, "&", pairs);
	for(size_t i = 0; i < pairs.size(); i++)
	{
		Util::Split(pairs[i], "=", keyval);
		if(keyval.size() < 1)
			continue;

		int time = DEFAULT_CYCLE_TIME;
		if(keyval.size() >= 2)
			time = atoi(keyval[1].c_str()) * MILLISECOND_PER_SECOND;
		mCycleStrings.push_back(keyval[0]);
		mCycleTimes.push_back(time);
	}

	if(mCycleTimes.size() > 0)
		mNextUpdateTime += mCycleTimes[0];
}

bool EnvironmentCycleManager :: HasCycleUpdated(void)
{
	if(g_ServerTime < mNextUpdateTime)
		return false;
	if(mCycleStrings.size() == 0)
		return false;

	mCurrentCycleIndex++;
	if(mCurrentCycleIndex >= mCycleStrings.size())
		mCurrentCycleIndex = 0;

	mNextUpdateTime = g_ServerTime + mCycleTimes[mCurrentCycleIndex];
	return true;
}

std::string EnvironmentCycleManager :: GetCurrentTimeOfDay(void)
{
	if(mCurrentCycleIndex >= mCycleStrings.size())
		return "";
	return mCycleStrings[mCurrentCycleIndex];
}

void EnvironmentCycleManager :: EndCurrentCycle(void)
{
	mNextUpdateTime = g_ServerTime; 
}

GroveTemplate :: GroveTemplate()
{
	Clear();
}

void GroveTemplate :: Clear(void)
{
	mShortName.clear();
	mFileName.clear();
	mTerrainCfg.clear();
	mEnvType.clear();
	mMapName.clear();
	mRegionsPng.clear();
	mTileX1 = 0;
	mTileY1 = 0;
	mTileX2 = 0;
	mTileY2 = 0;
	mDefX = 0;
	mDefY = 0;
	mDefZ = 0;
}

GroveTemplateManager :: GroveTemplateManager()
{
}

void GroveTemplateManager :: LoadData(void)
{
	char fileName[256];
	Platform::GenerateFilePath(fileName, "Data", "GroveTemplate.txt");
	LoadFile(fileName);

	// Need to set up the TerrainCfg lookup table.
	ResolveTerrainMap();
	g_Log.AddMessageFormat("Loaded %d Grove Templates", mTemplateEntries.size());
}

void GroveTemplateManager :: LoadFile(const char *filename)
{
	FileReader3 fr;
	if(fr.OpenFile(filename) != fr.SUCCESS)
	{
		g_Log.AddMessageFormat("Unable to open grove template file [%s]", filename);
		return;
	}

	fr.SetCommentChar(';');
	fr.ReadLine(); //Assume first line is header.

	while(fr.Readable() == true)
	{
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if(r > 1)
		{
			std::string key = fr.BlockToStringC(0);
			GroveTemplate &data = mTemplateEntries[key];

			data.mShortName = key;
			data.mFileName = fr.BlockToStringC(1);
			data.mTerrainCfg = fr.BlockToStringC(2);
			data.mEnvType = fr.BlockToStringC(3);
			data.mMapName = fr.BlockToStringC(4);
			data.mRegionsPng = fr.BlockToStringC(5);
			data.mTileX1 = fr.BlockToIntC(6);
			data.mTileY1 = fr.BlockToIntC(7);
			data.mTileX2 = fr.BlockToIntC(8);
			data.mTileY2 = fr.BlockToIntC(9);
			data.mDefX = fr.BlockToIntC(10);
			data.mDefY = fr.BlockToIntC(11);
			data.mDefZ = fr.BlockToIntC(12);
		}
	}
	fr.CloseFile();
}

const GroveTemplate* GroveTemplateManager :: GetTemplateByShortName(const char *name)
{
	std::map<std::string, GroveTemplate>::iterator it;
	it = mTemplateEntries.find(name);
	if(it != mTemplateEntries.end())
		return &it->second;
	return NULL;
}

const GroveTemplate* GroveTemplateManager :: GetTemplateByTerrainCfg(const char *cfg)
{
	std::map<std::string, const GroveTemplate*>::iterator it;
	it = mTerrainMap.find(cfg);
	if(it != mTerrainMap.end())
		return it->second;
	return NULL;
}

void GroveTemplateManager :: ResolveTerrainMap(void)
{
	mTerrainMap.clear();
	std::map<std::string, GroveTemplate>::iterator it;
	for(it = mTemplateEntries.begin(); it != mTemplateEntries.end(); ++it)
	{
		mTerrainMap[it->second.mTerrainCfg] = &it->second;
	}
}


int PrepExt_SendEnvironmentUpdateMsg(char *buffer, ActiveInstance *instance, const char *zoneIDString, ZoneDefInfo *zoneDef, int x, int z, int mask)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 42);   //_handleEnvironmentUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutByte(&buffer[wpos], mask);   //Mask

	wpos += PutStringUTF(&buffer[wpos], zoneIDString);
	wpos += PutInteger(&buffer[wpos], zoneDef->mID);
	wpos += PutShort(&buffer[wpos], zoneDef->mPageSize);
	wpos += PutStringUTF(&buffer[wpos], zoneDef->mTerrainConfig.c_str());
	if(instance == NULL) {
		wpos += PutStringUTF(&buffer[wpos], zoneDef->GetTileEnvironment(x, z).c_str());
	}
	else {
		wpos += PutStringUTF(&buffer[wpos], instance->GetEnvironment(x, z).c_str());
	}
	wpos += PutStringUTF(&buffer[wpos], zoneDef->mMapName.c_str());

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}
