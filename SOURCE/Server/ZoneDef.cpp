#include "ZoneDef.h"
#include "FileReader.h"
#include "FileReader3.h"
#include "Config.h"
#include "GameConfig.h"
#include "PVP.h"
#include "Random.h"
#include "Audit.h"  //For scenery audits.
#include "InstanceScale.h"
#include "Report.h"
#include "Character.h"
#include "Globals.h"
#include "Instance.h"
#include "Simulator.h"
#include "Cluster.h"
#include "Scheduler.h"
#include "util/Log.h"
#include "ByteBuffer.h"

ZoneDefManager g_ZoneDefManager;
ZoneBarrierManager g_ZoneBarrierManager;
EnvironmentCycleManager g_EnvironmentCycleManager;
GroveTemplateManager g_GroveTemplateManager;
WeatherManager g_WeatherManager;

ZoneEditPermission::ZoneEditPermission() {
	Clear();
}

void ZoneEditPermission::Clear(void) {
	mEditType = PERMISSION_TYPE_NONE;
	mID = 0;
	mX1 = 0;
	mY1 = 0;
	mX2 = 0;
	mY2 = 0;
}

void ZoneEditPermission::WriteEntity(AbstractEntityWriter *writer) {
	switch (mEditType) {
	case PERMISSION_TYPE_ACCOUNT:
		writer->Value("BuildPermission",
				Util::Format("account,%d,%d,%d,%d,%d", mID, mX1, mY1, mX2,
						mY2));
		break;
	case PERMISSION_TYPE_CDEF:
		writer->Value("BuildPermission",
				Util::Format("cdef,%d,%d,%d,%d,%d", mID, mX1, mY1, mX2,
						mY2));
		break;
	case PERMISSION_TYPE_PLAYER:
		writer->Value("BuildPermission",
				Util::Format("account,%d,%d,%d,%d,%d",
						mCharacterName.c_str(), mX1, mY1, mX2, mY2));
		break;
	}
}

int ZoneEditPermission::GetTypeIDByName(const string &name) {
	if (name.compare("account") == 0)
		return PERMISSION_TYPE_ACCOUNT;
	if (name.compare("cdef") == 0)
		return PERMISSION_TYPE_CDEF;
	if (name.compare("player") == 0)
		return PERMISSION_TYPE_PLAYER;

	return PERMISSION_TYPE_NONE;
}

const char* ZoneEditPermission::GetTypeNameByID(int ID) {
	switch (ID) {
	case PERMISSION_TYPE_ACCOUNT:
		return "account";
	case PERMISSION_TYPE_CDEF:
		return "cdef";
	case PERMISSION_TYPE_PLAYER:
		return "player";
	}
	return NULL;
}

void ZoneEditPermission::LoadFromConfig(const char *configStr) {
	mEditType = PERMISSION_TYPE_NONE;

	if (configStr == NULL)
		return;

	string args = configStr;
	STRINGLIST paramList;
	Util::Split(args, ",", paramList);

	if (paramList.size() == 0)
		return;

	int editType = GetTypeIDByName(paramList[0]);
	if (editType == PERMISSION_TYPE_NONE)
		return;

	size_t argBase = 2;
	switch (editType) {
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

bool ZoneEditPermission::IsValid(void) {
	switch (mEditType) {
	case PERMISSION_TYPE_ACCOUNT: //fall through to cdef since both just use mID
	case PERMISSION_TYPE_CDEF:
		if (mID != 0)
			return true;
		break;
	case PERMISSION_TYPE_PLAYER:
		if (mCharacterName.size() > 0)
			return true;
		break;
	case PERMISSION_TYPE_NONE:  //fall through
	default:
		return false;
	}
	return false;
}

bool ZoneEditPermission::HasEditPermission(int accountID, int characterDefID,
		const char *characterName, float x, float z, int pageSize) {
	switch (mEditType) {
	case PERMISSION_TYPE_CDEF:
		if (mID != characterDefID)
			return false;
		break;
	case PERMISSION_TYPE_ACCOUNT:
		if (mID != accountID)
			return false;
		break;
	case PERMISSION_TYPE_PLAYER:
		if (mCharacterName.compare(characterName) != 0)
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
	if (checkX < x1 || checkX > x2)
		return false;

	int y1 = mY1 * pageSize;
	int y2 = ((mY2 + 1) * pageSize) - 1;
	int checkY = static_cast<int>(z); //For world objects, Y axis is elevation so we convert from Z instead
	if (checkY < y1 || checkY > y2)
		return false;

	return true;
}

ZoneIndexEntry::ZoneIndexEntry() {
	mID = 0;
	mAccountID = 0;
}

void ZoneIndexEntry::CopyFrom(const ZoneIndexEntry &other) {
	mID = other.mID;
	mAccountID = other.mAccountID;
	mWarpName = other.mWarpName;
	mGroveName = other.mGroveName;
}

ZoneDefInfo::ZoneDefInfo() {
	Clear();
}

ZoneDefInfo::~ZoneDefInfo() {
}

void ZoneDefInfo::Clear(void) {
	mID = 0;
	mStatic = false;
	mAccountID = 0;
	mDesc.clear();
	mName.clear();
	mTerrainConfig.clear();
	mEnvironmentType.clear();
	mMapName.clear();
	mRegions.clear();

	mGroveName.clear();
	mWarpName.clear();
	DefX = 0;
	DefY = 0;
	DefZ = 0;
	mPageSize = DEFAULT_PAGESIZE;
	mMaxAggroRange = DEFAULT_MAXAGGRORANGE;
	mMaxLeashRange = DEFAULT_MAXLEASHRANGE;
	mMinLevel = 0;
	mMaxLevel = 9999;

	mReturnZone = 0;
	mPersist = false;
	mInstance = false;
	mGrove = false;
	mGuildHall = false;
	mArena = false;
	mAudit = false;
	mTimeOfDay = "";
	mEnvironmentCycle = false;
	mClan = 0;

	mPlayerFilterType = FILTER_PLAYER_NONE;
	mPlayerFilterID.clear();

	mMode = PVP::GameMode::PVE_ONLY;

	mDropRateProfile.clear();

	mNextAuditSave = 0;
	mZoneAudit.Clear();
	mTileEnvironment.clear();
}

void ZoneDefInfo::CopyFrom(const ZoneDefInfo &other) {
	mID = other.mID;
	PopulateFrom(other);
}

void ZoneDefInfo::PopulateFrom(const ZoneDefInfo &other) {
	mAccountID = other.mAccountID;
	mDesc = other.mDesc;
	mName = other.mName;
	mTerrainConfig = other.mTerrainConfig;
	mEnvironmentType = other.mEnvironmentType;
	mMapName = other.mMapName;
	mRegions = other.mRegions;
	mTimeOfDay = other.mTimeOfDay;

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
	mClan = other.mClan;
	mArena = other.mArena;
	mMode = other.mMode;
	mAudit = other.mAudit;
	mEnvironmentCycle = other.mEnvironmentCycle;
	mMaxAggroRange = other.mMaxAggroRange;
	mMaxLeashRange = other.mMaxLeashRange;
	mMinLevel = other.mMinLevel;
	mMaxLevel = other.mMaxLevel;

	mPlayerFilterType = other.mPlayerFilterType;
	mPlayerFilterID.assign(other.mPlayerFilterID.begin(),
			other.mPlayerFilterID.end());

	mDropRateProfile = other.mDropRateProfile;

	mEditPermissions.assign(other.mEditPermissions.begin(),
			other.mEditPermissions.end());

	mTileEnvironment.clear();
	mTileEnvironment.insert(other.mTileEnvironment.begin(),
			other.mTileEnvironment.end());
}

void ZoneDefInfo::SetDefaults(void) {
	// Should be called before a ZoneDef is added into the ZoneDefManager list.
	// Makes sure to fall back on certain defaults if not explicitly defined in
	// the data file.
	if (mPageSize == 0)
		mPageSize = DEFAULT_PAGESIZE;
	if (mMaxAggroRange == 0)
		mMaxAggroRange = DEFAULT_MAXAGGRORANGE;

	if (mReturnZone == 0)
		mReturnZone = mID;

	if (mDropRateProfile.size() == 0) {
		if (mInstance == true)
			mDropRateProfile = DropRateProfileManager::INSTANCE;
		else
			mDropRateProfile = DropRateProfileManager::STANDARD;
	}

	if (IsPlayerGrove() == true)
		CreateDefaultGrovePermission();
}

bool ZoneDefInfo::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_ZONE_DEF_INFO, Util::Format("%d", mID));
	return true;
}

bool ZoneDefInfo::ReadEntity(AbstractEntityReader *reader) {
	if (!reader->Exists())
		return false;

	mAccountID = atoi(reader->Value("AccountID", "0").c_str());
	mDesc = reader->Value("Desc");
	mName = reader->Value("Name");
	mTerrainConfig = reader->Value("TerrainConfig");
	mEnvironmentType = reader->Value("EnvironmentType");
	mTimeOfDay = reader->Value("TimeOfDay");
	mMapName = reader->Value("MapName");
	mRegions = reader->Value("Regions");
	mGroveName = reader->Value("GroveName");
	mWarpName = reader->Value("WarpName");
	mMode = reader->ValueInt("Mode", PVP::GameMode::PVE_ONLY);
	mPageSize = reader->ValueInt("PageSize", DEFAULT_PAGESIZE);
	mMaxAggroRange = reader->ValueInt("MaxAggroRange", DEFAULT_MAXAGGRORANGE);
	mMaxLeashRange = reader->ValueInt("MaxLeashRange", DEFAULT_MAXLEASHRANGE);
	mMinLevel = reader->ValueInt("MinLevel");
	mMaxLevel = reader->ValueInt("MaxLevel", 9999);
	STRINGLIST defloc = reader->ListValue("DefLoc", ",");
	if (defloc.size() > 0)
		DefX = atoi(defloc[0].c_str());
	if (defloc.size() > 1)
		DefY = atoi(defloc[1].c_str());
	if (defloc.size() > 2)
		DefZ = atoi(defloc[2].c_str());
	mPersist = reader->ValueBool("Persist");
	mInstance = reader->ValueBool("Instance");
	mGrove = reader->ValueBool("Grove");
	mGuildHall = reader->ValueBool("GuildHall");
	mClan= reader->ValueInt("Clan");
	mArena = reader->ValueBool("Arena");
	mAudit = reader->ValueBool("Audit");
	SetDropRateProfile(reader->Value("DropRateProfile"));
	mEnvironmentCycle = reader->ValueBool("EnvironmentCycle");
	mPlayerFilterType = reader->ValueInt("PlayerFilterType",
			FILTER_PLAYER_NONE);
	STRINGLIST filterIds = reader->ListValue("PlayerFilterID", ",");
	for (auto a = filterIds.begin(); a != filterIds.end(); ++a)
		AddPlayerFilterID(atoi((*a).c_str()), true);
	STRINGLIST buildPermissions = reader->ListValue("BuildPermission");
	for (auto a = buildPermissions.begin(); a != buildPermissions.end(); ++a) {
		ZoneEditPermission perm;
		perm.LoadFromConfig((*a).c_str());
		if (perm.IsValid() == true) {
			mEditPermissions.push_back(perm);
		}
	}
	STRINGLIST tileEnvs = reader->ListValue("TileEnvironment");
	for (auto a = tileEnvs.begin(); a != tileEnvs.end(); ++a) {
		STRINGLIST l;
		Util::Split(*a, ",", l);
		if (l.size() > 2) {
			EnvironmentTileKey etk;
			etk.x = atoi(l[0].c_str());
			etk.y = atoi(l[1].c_str());
			mTileEnvironment[etk] = atoi(l[2].c_str());
		} else {
			g_Logs.data->warn("Invalid TileEnvironment '%v' for %v", *a, mID);
		}
	}
	STRINGLIST areaEnvs = reader->ListValue("AreaEnvironment");
	for (auto a = areaEnvs.begin(); a != areaEnvs.end(); ++a) {
		STRINGLIST l;
		Util::Split(*a, ",", l);
		if (l.size() > 4) {
			int tx = atoi(l[0].c_str());
			int ty = atoi(l[1].c_str());
			int tw = atoi(l[2].c_str());
			int th = atoi(l[3].c_str());
			for (int iy = 0; iy < th; iy++) {
				for (int ix = 0; ix < tw; ix++) {
					EnvironmentTileKey etk;
					etk.x = tx + ix;
					etk.y = ty + iy;
					mTileEnvironment[etk] = l[4];
				}
			}
		} else {
			g_Logs.data->warn("Invalid TileEnvironment '%v' for %v", *a, mID);
		}
	}
	return true;
}

bool ZoneDefInfo::WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_ZONE_DEF_INFO, Util::Format("%d", mID));
	writer->Value("AccountID", mAccountID);
	writer->Value("Name", mName);
	writer->Value("Desc", mDesc);
	writer->Value("TerrainConfig", mTerrainConfig);
	writer->Value("EnvironmentType", mEnvironmentType);
	writer->Value("MapName", mMapName);
	writer->Value("Regions", mRegions);
	if (mMode != PVP::GameMode::PVE_ONLY)
		writer->Value("Mode", mMode);
	writer->Value("GroveName", mGroveName);
	writer->Value("TimeOfDay", mTimeOfDay);
	writer->Value("WarpName", mWarpName);
	writer->Value("DropRateProfile", mDropRateProfile);
	if (mPersist == true)
		writer->Value("Persist", mPersist);
	if (mAudit == true)
		writer->Value("Audit", mAudit);
	writer->Value("Grove", mGrove);
	writer->Value("Instance", mInstance);
	writer->Value("Persist", mPersist);
	writer->Value("GuildHall", mGuildHall);
	writer->Value("Clan", mClan);
	writer->Value("Arena", mArena);
	writer->Value("Audit", mAudit);
	writer->Value("MinLevel", mMinLevel);
	writer->Value("MaxLevel", mMaxLevel);
	writer->Value("EnvironmentCycle", mEnvironmentCycle);
	if (mPageSize != ZoneDefInfo::DEFAULT_PAGESIZE)
		writer->Value("PageSize", mPageSize);
	if (mMaxAggroRange != ZoneDefInfo::DEFAULT_MAXAGGRORANGE)
		writer->Value("MaxAggroRange", mMaxAggroRange);
	if (mMaxLeashRange != ZoneDefInfo::DEFAULT_MAXLEASHRANGE)
		writer->Value("MaxLeashRange", mMaxLeashRange);
	if (mPlayerFilterType != 0)
		writer->Value("PlayerFilterType", mPlayerFilterType);
	STRINGLIST l;
	for (auto a = mPlayerFilterID.begin(); a != mPlayerFilterID.end(); ++a) {
		l.push_back(Util::Format("%d", *a));
	}
	if (l.size() > 0)
		writer->ListValue("PlayerFilterID", l);

	writer->Value("DefLoc", Util::Format("%d,%d,%d", DefX, DefY, DefZ));
	STRINGLIST l2, l2b;
	for (auto a = mTileEnvironment.begin(); a != mTileEnvironment.end(); ++a) {
		STRINGLIST ll;
		Util::Split(a->second, ",", ll);
		if (ll.size() > 3) {
			l2b.push_back(
					Util::Format("%d,%d,%s", a->first.x, a->first.y,
							a->second.c_str()));
		} else {
			l2.push_back(
					Util::Format("%d,%d,%s", a->first.x, a->first.y,
							a->second.c_str()));
		}
	}
	if (l2.size() > 0)
		writer->ListValue("TileEnvironment", l2);
	if (l2b.size() > 0)
		writer->ListValue("AreaEnvironment", l2b);
	STRINGLIST l3;
	for (auto a = mEditPermissions.begin(); a != mEditPermissions.end(); ++a)
		(*a).WriteEntity(writer);
	if (l3.size() > 0)
		writer->ListValue("BuildPermission", l2);

	return true;
}

bool ZoneDefInfo::IsGuildHall(void) {
	return mGuildHall;
}

bool ZoneDefInfo::IsPlayerGrove(void) {
	if (mGrove == true && mID >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
		return true;
	return false;
}

bool ZoneDefInfo::IsPVPArena(void) {
	return mArena;
}

// Free travel means that players can are free to warp into a particular zone if external criteria are met
// like sanctuary proximity.
bool ZoneDefInfo::IsFreeTravel(void) {
	if (IsPlayerGrove() == true)
		return true;
	if (IsGuildHall() == true)
		return true;
	if (IsPVPArena() == true)
		return true;

	return false;
}

bool ZoneDefInfo::IsDungeon(void) {
	if (mGuildHall == false && mGrove == false && mArena == false
			&& mInstance == true)
		return true;

	return false;
}

bool ZoneDefInfo::IsOverworld(void) {
	if (mGuildHall == false && mGrove == false && mArena == false
			&& mInstance == false)
		return true;

	return false;
}

bool ZoneDefInfo::IsMobScalable(void) {
	if (mInstance == true)
		return true;
	return false;
}

bool ZoneDefInfo::HasDeathPenalty(void) {
	if (mGrove == true || mArena == true)
		return false;
	return true;
}

bool ZoneDefInfo::CanPlayerWarp(int CreatureDefID, int AccountID) {
	//Check if a player is allowed to warp.
	//Always allow the account owner.
	if (mAccountID == AccountID)
		return true;
	if (mPlayerFilterType == FILTER_PLAYER_NONE)
		return true;

	static const int FILTER_PLAYER_WHITELIST = 1; //Allow the players on the filter list, and only them.
	static const int FILTER_PLAYER_BLACKLIST = 2; //Block the players on the filter list.

	if (mPlayerFilterType == FILTER_PLAYER_WHITELIST) {
		if (HasPlayerFilterID(CreatureDefID) == true)
			return true;
		return false;
	}
	if (mPlayerFilterType == FILTER_PLAYER_BLACKLIST) {
		if (HasPlayerFilterID(CreatureDefID) == true)
			return false;
		return true;
	}
	return false;
}

void ZoneDefInfo::SetDropRateProfile(string profile) {
	mDropRateProfile = profile;
}

int ZoneDefInfo::GetNextPropID() {
	return g_ClusterManager.NextValue(
			Util::Format("%s:%d", ID_NEXT_SCENERY.c_str(), mID));
}

string ZoneDefInfo::GetTileEnvironment(int x, int y) {
	// Convert to scenery tile (use /info scenerytile to help with this)
	int px = x / mPageSize;
	int py = y / mPageSize;
	if (x >= 0 && y >= 0) {
		map<EnvironmentTileKey, string>::iterator it;
		EnvironmentTileKey etk;
		etk.x = px;
		etk.y = py;
		it = mTileEnvironment.find(etk);
		if (it != mTileEnvironment.end()) {
			return it->second;
		}
	}
	return mEnvironmentType;
}

void ZoneDefInfo::AddPlayerFilterID(int CreatureDefID, bool loadStage) {
	for (size_t i = 0; i < mPlayerFilterID.size(); i++)
		if (mPlayerFilterID[i] == CreatureDefID)
			return;
	mPlayerFilterID.push_back(CreatureDefID);

	// If we're not adding players from the ZoneDef load stage, this was probably added by a command, so
	// mark a pending change.
	if (loadStage == false)
		g_ClusterManager.WriteEntity(this, false);
}

void ZoneDefInfo::RemovePlayerFilter(int CreatureDefID) {
	for (size_t i = 0; i < mPlayerFilterID.size(); i++) {
		if (mPlayerFilterID[i] == CreatureDefID) {
			mPlayerFilterID.erase(mPlayerFilterID.begin() + i);
			g_ClusterManager.WriteEntity(this, false);
			return;
		}
	}
}

void ZoneDefInfo::ClearPlayerFilter(void) {
	if (mPlayerFilterID.size() > 0) {
		mPlayerFilterID.clear();
		g_ClusterManager.WriteEntity(this, false);
	}
}

bool ZoneDefInfo::HasPlayerFilterID(int CreatureDefID) {
	for (size_t i = 0; i < mPlayerFilterID.size(); i++)
		if (mPlayerFilterID[i] == CreatureDefID)
			return true;
	return false;
}

bool ZoneDefInfo::HasEditPermission(int accountID, int characterDefID,
		const char *characterName, float x, float z) {
	//Quick hack since negative coordinates seem to cause glitched scenery tiles in the client and
	//infinite load screens.  I'm not sure yet why this is, but we can probably assume this area is
	//off limits anyway.
	if (x < 0.0F || z < 0.0F)
		return false;

	//Play can always edit their own grove
	if ((mAccountID != 0) && (accountID == mAccountID))
		return true;

	if(mClan != 0) {
		/* If a clan, make sure the character is in this clan  and is of the right
		 * rank
		 */
		Clans::Clan clan = g_ClanManager.GetClan(mClan);
		if(clan.HasMember(characterDefID)) {
			Clans::ClanMember member = clan.GetMember(characterDefID);
			if(member.mRank != Clans::Rank::LEADER && member.mRank != Clans::Rank::OFFICER)
				return false;
		}
		else
			return false;

	}

	if(mEditPermissions.size() > 0) {
		for (size_t i = 0; i < mEditPermissions.size(); i++) {
			if (mEditPermissions[i].HasEditPermission(accountID, characterDefID,
					characterName, x, z, DEFAULT_PAGESIZE) == true)
				return true;
		}
		return false;
	}

	return true;
}

void ZoneDefInfo::UpdateGrovePermission(STRINGLIST &params) {
	const char *action = Util::GetSafeString(params, 0);
	if (strcmp(action, "add") == 0) {
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
		g_ClusterManager.WriteEntity(this, false);
	} else if (strcmp(action, "delete") == 0) {
		size_t index = static_cast<size_t>(Util::GetInteger(params, 1));
		if (index >= 0 && index < mEditPermissions.size()) {
			mEditPermissions.erase(mEditPermissions.begin() + index);
			g_ClusterManager.WriteEntity(this, false);
		}
	}
}

string ZoneDefInfo::GetDropRateProfile() {
	if (mDropRateProfile.length() > 0)
		return mDropRateProfile;
	else {
		if (mInstance) {
			return "instance";
		} else
			return "standard";
	}
}

void ZoneDefInfo::ChangeDefaultLocation(int newX, int newY, int newZ) {
	DefX = newX;
	DefY = newY;
	DefZ = newZ;
	g_ClusterManager.WriteEntity(this, false);
}

void ZoneDefInfo::ChangeName(const char *newName) {
	mName = newName;
	g_ClusterManager.WriteEntity(this, false);
}

void ZoneDefInfo::ChangeEnvironment(const char *newEnvironment) {
	mEnvironmentType = newEnvironment;
	g_ClusterManager.WriteEntity(this, false);
}

void ZoneDefInfo::ChangeEnvironmentUsage(void) {
	mEnvironmentCycle = !mEnvironmentCycle;
	g_ClusterManager.WriteEntity(this, false);
}

bool ZoneDefInfo::QualifyDelete(void) {
	if (mID >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
		return true;

	return false;
}

string ZoneDefInfo::GetTimeOfDay() {
	//If the environment time string is null, attempt to find the active time for the
	//current zone, if applicable.
	if (mEnvironmentCycle == true)
		return g_EnvironmentCycleManager.GetCurrentCycle().mName;
	else if (mTimeOfDay.length() > 0)
		return mTimeOfDay;
	return "Day";
}

bool ZoneDefInfo::AllowSceneryAudits(void) {
	if (g_Config.SceneryAuditAllow == false)
		return false;

	//Anything set to explicitly audit.
	if (mAudit == true)
		return true;

	//Don't audit groves by default.
	if (mGrove == true)
		return false;

	//If we get here we're probably an official zone, so audit it.
	return true;
}

void ZoneDefInfo::AuditScenery(const char *username, int zone,
		const SceneryObject *sceneryObject, int opType) {
	if (sceneryObject == NULL)
		return;
	if (AllowSceneryAudits() == false)
		return;

	mZoneAudit.PerformSceneryAudit(username, zone, sceneryObject, opType);
}

void ZoneDefInfo::AutosaveAudits(bool force) {
	if (g_ServerTime < mNextAuditSave && force == false)
		return;

	mZoneAudit.AutosaveAudits();
	mNextAuditSave = g_ServerTime + g_Config.SceneryAuditDelay;
}

void ZoneDefInfo::CreateDefaultGrovePermission(void) {
	//Can't set up a permission if it doesn't have an owner.
	if (mAccountID == 0)
		return;

	//Check to see if we already have a permission set up for the account.
	for (size_t i = 0; i < mEditPermissions.size(); i++) {
		if ((mEditPermissions[i].mEditType
				== ZoneEditPermission::PERMISSION_TYPE_ACCOUNT)
				&& (mEditPermissions[i].mID == mAccountID))
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
	const GroveTemplate *gt = g_GroveTemplateManager.GetTemplateByTerrainCfg(
			mTerrainConfig.c_str());
	if (gt != NULL) {
		entry.mX1 = gt->mTileX1;
		entry.mX2 = gt->mTileX2;
		entry.mY1 = gt->mTileY1;
		entry.mY2 = gt->mTileY2;
	}
	mEditPermissions.insert(mEditPermissions.begin(), entry);
	g_ClusterManager.WriteEntity(this, false);
}

void ZoneDefInfo::ReadFromJSON(Json::Value &value) {

	mID = value.get("id", 0).asInt();
	mAccountID = value.get("account", 0).asInt();
	mDesc = value.get("description", "").asString();
	mName = value.get("name", "").asString();
	mTerrainConfig = value.get("terrain", "").asString();
	mEnvironmentType = value.get("environment", "").asString();
	mMapName = value.get("map", "").asString();
	mRegions = value.get("regions", "").asString();
	mWarpName = value.get("warp", "").asString();

	Json::Value defLoc = value["def"];
	DefX = defLoc.get("x", 0).asInt();
	DefY = defLoc.get("y", 0).asInt();
	DefZ = defLoc.get("z", 0).asInt();

	mPageSize = value["page"].asInt();
	mMode = value["mode"].asInt();
	mReturnZone = value["return"].asInt();
	mPersist = value["persist"].asBool();
	mInstance = value["instance"].asBool();
	mGrove = value["grove"].asBool();
	mArena = value["arena"].asBool();
	mGuildHall = value["hall"].asBool();
	mClan = value["clan"].asInt();
	mEnvironmentCycle = value["cycle"].asBool();
	mAudit = value["audit"].asBool();

	mMaxAggroRange = value["maxAggro"].asInt();
	mMaxLeashRange = value["maxLeash"].asInt();

	mDropRateProfile = value.get("profile", "").asString();

	Json::Value filter = value["filter"];
	mPlayerFilterType = filter.get("type", 0).asInt();
	Json::Value filteredCreatures = filter["creatureDefs"];
	mPlayerFilterID.clear();
	for (Json::Value::iterator it = filteredCreatures.begin();
			it != filteredCreatures.end(); ++it) {
		mPlayerFilterID.push_back((*it).asInt());
	}

	mTileEnvironment.clear();
	Json::Value envTiles = value["environmentTiles"];
	for (Json::Value::iterator it = envTiles.begin(); it != envTiles.end();
			++it) {
		Json::Value item = *it;
		Json::Value key = item["key"];
		EnvironmentTileKey k;
		k.ReadFromJSON(key);
		mTileEnvironment[k] = item["environment"].asString();
	}
}

void ZoneDefInfo::WriteToJSON(Json::Value &value) {
	value["id"] = mID;
	value["account"] = mAccountID;
	value["description"] = mDesc;
	value["name"] = mName;
	value["terrain"] = mTerrainConfig;
	value["environment"] = mEnvironmentType;
	value["map"] = mMapName;
	value["regions"] = mRegions;
	value["warp"] = mWarpName;

	Json::Value defLoc;
	defLoc["x"] = DefX;
	defLoc["y"] = DefY;
	defLoc["z"] = DefZ;
	value["def"] = defLoc;

	value["page"] = mPageSize;
	value["mode"] = mMode;
	value["return"] = mReturnZone;
	value["persist"] = mPersist;
	value["instance"] = mInstance;
	value["grove"] = mGrove;
	value["arena"] = mArena;
	value["hall"] = mGuildHall;
	value["clan"] = mClan;
	value["cycle"] = mEnvironmentCycle;
	value["audit"] = mAudit;
	value["maxAggro"] = mMaxAggroRange;
	value["maxLeash"] = mMaxLeashRange;

	if (mPlayerFilterID.size() > 0) {
		Json::Value filter;
		filter["type"] = mPlayerFilterType;
		Json::Value creatureDefs;
		for (vector<int>::iterator it = mPlayerFilterID.begin();
				it != mPlayerFilterID.end(); ++it)
			creatureDefs.append(*it);
		filter["creatureDefs"] = creatureDefs;
		value["filter"] = filter;
	}

	value["profile"] = mDropRateProfile;

	if (mTileEnvironment.size() > 0) {
		Json::Value envtiles;
		for (map<EnvironmentTileKey, string>::iterator it =
				mTileEnvironment.begin(); it != mTileEnvironment.end(); ++it) {
			Json::Value item;
			Json::Value key;
			EnvironmentTileKey k = it->first;
			k.WriteToJSON(key);
			item["key"] = key;
			item["environment"] = it->second;
			value["tile"] = item;
		}
		value["environmentTiles"] = envtiles;
	}

}

ZoneDefManager::ZoneDefManager() {
	mNextAutosaveTime = 0;
	mNextZoneUnload = 0;
	cs.Init();
	cs.SetDebugName("CS_ZONEDEFMGR");
}

ZoneDefManager::~ZoneDefManager() {
	Free();
}

void ZoneDefManager::Free(void) {
	CheckAutoSave(true);
	mZoneList.clear();
}

int ZoneDefManager::LoadFile(string fileName) {
	//Note: the official grove file is loaded first, then the custom grove file.
	//This should point here. 
	FileReader lfr;
	if (lfr.OpenText(fileName) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", fileName);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;
	ZoneDefInfo newItem;
	newItem.mStatic = true;

	while (lfr.FileOpen() == true) {
		int r = lfr.ReadLine();
		if (r > 0) {
			r = lfr.BreakUntil("=", '='); //Don't use SingleBreak since we won't be able to re-split later.
			lfr.BlockToStringC(0, Case_Upper);
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (newItem.mID != 0) {
					newItem.SetDefaults();
					mZoneList[newItem.mID].CopyFrom(newItem);
					newItem.Clear();
					newItem.mStatic = true;
				}
			} else {
				if (strcmp(lfr.SecBuffer, "ID") == 0)
					newItem.mID = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "ACCOUNTID") == 0)
					newItem.mAccountID = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "DESC") == 0)
					newItem.mDesc = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "NAME") == 0)
					newItem.mName = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "TERRAINCONFIG") == 0)
					newItem.mTerrainConfig = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "ENVIRONMENTTYPE") == 0)
					newItem.mEnvironmentType = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "TIMEOFDAY") == 0)
					newItem.mTimeOfDay = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "MAPNAME") == 0)
					newItem.mMapName = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "REGIONS") == 0)
					newItem.mRegions = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "GROVENAME") == 0)
					newItem.mGroveName = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "WARPNAME") == 0)
					newItem.mWarpName = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "MODE") == 0)
					newItem.mMode = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "PAGESIZE") == 0)
					newItem.mPageSize = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "MAXAGGRORANGE") == 0)
					newItem.mMaxAggroRange = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "MAXLEASHRANGE") == 0)
					newItem.mMaxLeashRange = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "MINLEVEL") == 0)
					newItem.mMinLevel = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "MAXLEVEL") == 0)
					newItem.mMaxLevel = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "DEFLOC") == 0) {
					lfr.MultiBreak("=,"); //Re-split for this particular data.
					newItem.DefX = lfr.BlockToIntC(1);
					newItem.DefY = lfr.BlockToIntC(2);
					newItem.DefZ = lfr.BlockToIntC(3);
				} else if (strcmp(lfr.SecBuffer, "PERSIST") == 0)
					newItem.mPersist = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "INSTANCE") == 0)
					newItem.mInstance = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "GROVE") == 0)
					newItem.mGrove = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "GUILDHALL") == 0)
					newItem.mGuildHall = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "CLAN") == 0)
					newItem.mClan = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "ARENA") == 0)
					newItem.mArena = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "AUDIT") == 0)
					newItem.mAudit = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "DROPRATEPROFILE") == 0)
					newItem.SetDropRateProfile(lfr.BlockToStringC(1, 0));
				else if (strcmp(lfr.SecBuffer, "ENVIRONMENTCYCLE") == 0)
					newItem.mEnvironmentCycle = lfr.BlockToBoolC(1);
				else if (strcmp(lfr.SecBuffer, "PLAYERFILTERTYPE") == 0)
					newItem.mPlayerFilterType = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "PLAYERFILTERID") == 0) {
					r = lfr.MultiBreak("=,"); //Re-split for this particular data.
					for (int s = 1; s < r; s++)
						newItem.AddPlayerFilterID(lfr.BlockToIntC(s), true);
				} else if (strcmp(lfr.SecBuffer, "BUILDPERMISSION") == 0) {
					ZoneEditPermission perm;
					perm.LoadFromConfig(lfr.BlockToStringC(1, 0));
					if (perm.IsValid() == true)
						newItem.mEditPermissions.push_back(perm);
				} else if (strcmp(lfr.SecBuffer, "TILEENVIRONMENT") == 0) {
					lfr.MultiBreak("=,"); //Re-split for this particular data.
					EnvironmentTileKey etk;
					etk.x = lfr.BlockToIntC(1);
					etk.y = lfr.BlockToIntC(2);
					newItem.mTileEnvironment[etk] = lfr.BlockToStringC(3, 0);
				} else if (strcmp(lfr.SecBuffer, "AREAENVIRONMENT") == 0) {
					lfr.MultiBreak("=,"); //Re-split for this particular data.
					int tx = lfr.BlockToIntC(1);
					int ty = lfr.BlockToIntC(2);
					int tw = lfr.BlockToIntC(3);
					int th = lfr.BlockToIntC(4);
					for (int iy = 0; iy < th; iy++) {
						for (int ix = 0; ix < tw; ix++) {
							EnvironmentTileKey etk;
							etk.x = tx + ix;
							etk.y = ty + iy;

							newItem.mTileEnvironment[etk] = lfr.BlockToStringC(
									3, 0);
							newItem.mTileEnvironment[etk] = lfr.BlockToStringC(
									5, 0);
						}
					}
				} else
					g_Logs.data->info(
							"Unknown identifier [%v] while reading from file %v.",
							lfr.SecBuffer, fileName);
			}
		}
	}
	lfr.CloseCurrent();

	if (newItem.mID != 0) {
		newItem.SetDefaults();
		mZoneList[newItem.mID].CopyFrom(newItem);
	}
	g_Logs.data->info("Loaded zone file [%v]", fileName);
	return 0;
}

ZoneDefInfo* ZoneDefManager::ResolveZoneDef(int ID) {
	ZONEDEF_ITERATOR it = mZoneList.find(ID);
	if (it == mZoneList.end())
		return LoadZoneDef(ID);
	return &it->second;
}

ZoneDefInfo* ZoneDefManager::LoadZoneDef(int ID) {
	//Official zones are static and can't be loaded at runtime..
	if (ID < GROVE_ZONE_ID_DEFAULT) {
		g_Logs.server->error("Could not find ZoneDef: %v", ID);
		return NULL;
	}

	ZoneDefInfo newItem;
	newItem.mID = ID;
	if (g_ClusterManager.ReadEntity(&newItem)) {
		g_Logs.data->debug("Loaded user zone %v from cluster", ID);
		mZoneList[ID] = newItem;
	}

	//Search to verify that the entry exists.
	ZONEDEF_ITERATOR it;
	it = mZoneList.find(ID);
	if (it == mZoneList.end()) {
		g_Logs.server->error("LoadZoneDef failed to load zone.");
		return NULL;
	}
	return &it->second;
}

int ZoneDefManager::UpdateWorldZone(ZoneDefInfo &newZone) {
	mZoneList[newZone.mID].CopyFrom(newZone);
	auto instancesPath = g_Config.ResolveVariableDataPath() / "Instance";
	auto p = instancesPath / to_string(newZone.mID);
	if(!fs::exists(p)) {
		fs::create_directories(p);
	}
	p = p / "ZoneDef.txt";
	TextFileEntityWriter tew(p);
	tew.PushSection("ENTITY");
	if (tew.Start()) {
		if (newZone.WriteEntity(&tew)) {
			tew.End();
		}
	}
	return 0;
}

void ZoneDefManager::LoadData(void) {
	//Load static entries (usually official instances, never modified during run time)
//	LoadFile(
//			Platform::JoinPath(
//					Platform::JoinPath(g_Config.ResolveStaticDataPath(),
//							"Data"), "ZoneDef.txt"));

	auto instancesPath = g_Config.ResolveVariableDataPath() / "Instance";

	//
	// Temporary. Convert All static zones to instance zonedef.txt files
	//
//	map<int, ZoneDefInfo>::iterator it2;
//	for (it2 = mZoneList.begin(); it2 != mZoneList.end(); ++it2) {
//		ZoneDefInfo zd = it2->second;
//		UpdateWorldZone(zd);
//	}

	for(const fs::directory_entry& entry : fs::directory_iterator(instancesPath)) {
		auto dir  = entry.path();
		auto p = dir / "ZoneDef.txt";
		TextFileEntityReader ter(p, Case_None, Comment_Semi);
		ter.Start();
		if (!ter.Exists()) {
			g_Logs.server->warn("No ZoneDef.txt in instance directory [%v].",
					p);
			continue;
		}

		ter.Key("", "", true);
		ter.Index("ENTRY");
		STRINGLIST sections = ter.Sections();
		for (auto a = sections.begin(); a != sections.end(); ++a) {
			ter.PushSection(*a);
			ZoneDefInfo t;
			t.mID = Util::SafeParseInt(dir.stem().c_str());
			if (!t.EntityKeys(&ter) || !t.ReadEntity(&ter)) {
				continue;
			}
			t.SetDefaults();
			mZoneList[t.mID].CopyFrom(t);
			ter.PopSection();
		}
		ter.End();
	}

	g_Logs.server->info("Loaded %v ZoneDef.", mZoneList.size());

	if (!g_ClusterManager.HasKey(ID_NEXT_ZONE_ID)) {
		g_ClusterManager.SetKey(ID_NEXT_ZONE_ID,
				Util::Format("%d", GROVE_ZONE_ID_DEFAULT));
	}

	/* Check the highest world zone ID */
	map<int, ZoneDefInfo>::iterator it2;
	int highest = 0;
	for (it2 = mZoneList.begin(); it2 != mZoneList.end(); ++it2) {
		ZoneDefInfo zd = it2->second;
		if(!zd.mGrove && !zd.mArena) {
			highest = max(zd.mID, highest);
		}
	}
	g_ClusterManager.SetKey(ID_NEXT_WORLD_ZONE_ID,
			Util::Format("%d", highest));
}

ZoneDefInfo* ZoneDefManager::GetPointerByID(int ID) {
	ZoneDefInfo *retptr = NULL;
	cs.Enter("ZoneDefManager::GetPointerByID");
	retptr = ResolveZoneDef(ID);

	/* OBSOLETE
	 list<ZoneDefInfo>::iterator it;
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

ZoneDefInfo* ZoneDefManager::GetPointerByPartialWarpName(
		const string &name) {
	ZoneDefInfo *retptr = NULL;

	STRINGLIST l;
	g_ClusterManager.Scan([this, &l](const string &match) {
		l.push_back(match);
	}
			,
			Util::Format("%s:*%s*",
					KEYPREFIX_WARP_NAME_TO_ZONE_ID.c_str(), name.c_str()), 1);

	int zoneID = -1;
	if (l.size() > 0)
		zoneID = atoi(g_ClusterManager.GetKey(l[0]).c_str());

	/* Only lock the zone manager once we have everything from the cluster we need. This
	 * might get called from a thread other than the server thread and we want to lock for
	 * a short a time as possible */
	cs.Enter("ZoneDefManager::GetPointerByPartialWarpName");
	if (zoneID != -1) {
		retptr = ResolveZoneDef(zoneID);
	}
	if (retptr == NULL) {
		for (auto it = mZoneList.begin(); it != mZoneList.end(); ++it)
			if (it->second.mWarpName.find(name) != string::npos) {
				retptr = &it->second;
				break;
			}
	}
	cs.Leave();

	return retptr;
}

string ZoneDefManager::GetNextGroveName(string groveName) {
	/* Find the next available grove name. The accounts original grove name is
	 * used, and a number appended until a grove with that name is not found.
	 */
	int index = 1;
	char name[256];
	name[0] = 0;
	ZoneDefInfo *zoneDef = NULL;
	while (index < 999) {
		Util::SafeFormat(name, sizeof(name), "%s%d", groveName.c_str(), index);
		zoneDef = g_ZoneDefManager.GetPointerByExactWarpName(name);
		if (zoneDef == NULL) {
			break;
		}
		index++;
	}
	return name;
}

ZoneDefInfo* ZoneDefManager::GetPointerByExactWarpName(const char *name) {
	ZoneDefInfo *retptr = NULL;

	string zoneIdStr = g_ClusterManager.GetKey(
			Util::Format("%s:%s", KEYPREFIX_WARP_NAME_TO_ZONE_ID.c_str(),
					name));
	if (zoneIdStr.length() > 0) {
		retptr = ResolveZoneDef(atoi(zoneIdStr.c_str()));
	}

	cs.Enter("ZoneDefManager::GetPointerByExactWarpName");
	if (retptr == NULL) {
		for (auto it = mZoneList.begin(); it != mZoneList.end(); ++it)
			if (it->second.mWarpName.compare(name) == 0) {
				retptr = &it->second;
				break;
			}
	}
	cs.Leave();

	return retptr;
}

ZoneDefInfo* ZoneDefManager::GetPointerByGroveName(const char *name) {
	ZoneDefInfo *retptr = NULL;

	vector<string> zoneIdStrs = g_ClusterManager.GetList(
			Util::Format("%s:%s",
					LISTPREFIX_GROVE_NAME_TO_ZONE_ID.c_str(), name));
	if (zoneIdStrs.size() > 0) {
		retptr = ResolveZoneDef(atoi(zoneIdStrs[0].c_str()));
	}

	cs.Enter("ZoneDefManager::GetPointerByGroveName");
	for (auto it = mZoneList.begin(); it != mZoneList.end(); ++it)
		if (it->second.mGroveName.compare(name) == 0) {
			retptr = &it->second;
			break;
		}
	cs.Leave();

	return retptr;
}

int ZoneDefManager::GetNewGroveZoneID(void) {
	return g_ClusterManager.NextValue(ID_NEXT_ZONE_ID, GROVE_ZONE_ID_INCREMENT);
}

int ZoneDefManager::GetNewWorldZoneID(void) {
	return g_ClusterManager.NextValue(ID_NEXT_WORLD_ZONE_ID, WORLD_ZONE_ID_INCREMENT);
}

int ZoneDefManager::CreateWorldZone() {
	int curId = g_ClusterManager.GetIntKey(ID_NEXT_WORLD_ZONE_ID);
	/* Don't let this clash with grove zones. If we get to this point,
	 * there will be other things to worry about!
	 */
	if(curId < GROVE_ZONE_ID_DEFAULT - 100) {
		ZoneDefInfo zd;
		zd.mID = GetNewWorldZoneID();
		InsertZone(zd, false);
		return zd.mID;
	}
	else {
		g_Logs.server->error("Hit maximum world zone ID [%v]", GROVE_ZONE_ID_INCREMENT - 101);
		return 0;
	}
}

int ZoneDefManager::CreateGroveZone(ZoneDefInfo &newZone) {
	if(!newZone.mGrove && !newZone.mGuildHall) {
		return 0;
	}

	cs.Enter("ZoneDefManager::CreateGroveZone");
	int newZoneID = GetNewGroveZoneID();

	newZone.mID = newZoneID;
	//Flag for the next autosave.

	InsertZone(newZone, true);

	cs.Leave();
	g_ClusterManager.WriteEntity(&newZone, false);
	return newZoneID;
}

int ZoneDefManager::CreateGrove(int accountID, const char *grovename) {
	int newZoneID = GetNewGroveZoneID();
	ZoneDefInfo newZone;

	newZone.mAccountID = accountID;
	newZone.mID = newZoneID;
	newZone.mName = "Grove";
	newZone.mTerrainConfig = "Terrain-Blend#Terrain-Blend.cfg";
	newZone.mEnvironmentType = "CloudyDay";
	newZone.mTimeOfDay = "";

	newZone.mPageSize = DEFAULT_GROVE_PAGE_SIZE;

	newZone.mGroveName = grovename;
	newZone.mWarpName = grovename;
	newZone.mWarpName.append("1");
	newZone.DefX = 2958;
	newZone.DefY = 260;
	newZone.DefZ = 3821;
	newZone.mGrove = true;
	newZone.mGuildHall = false;

	cs.Enter("ZoneDefManager::CreateGrove");
	InsertZone(newZone, true);
	cs.Leave();
	g_ClusterManager.WriteEntity(&newZone, false);
	return newZoneID;
}

//Copy zone data into the active table.  If creating an entirely new grove, an index
//can be created for it.
void ZoneDefManager::InsertZone(const ZoneDefInfo &newZone, bool createIndex) {
	//Copy into the resident zone list.
	mZoneList[newZone.mID].CopyFrom(newZone);

	//Create the index entry.
	if (createIndex == true) {
		g_ClusterManager.SetKey(
				Util::Format("%s:%s",
						KEYPREFIX_WARP_NAME_TO_ZONE_ID.c_str(),
						newZone.mWarpName.c_str()),
				Util::Format("%d", newZone.mID), false);
		g_ClusterManager.ListAdd(
				Util::Format("%s:%s",
						LISTPREFIX_GROVE_NAME_TO_ZONE_ID.c_str(),
						newZone.mGroveName.c_str()),
				Util::Format("%d", newZone.mID), false);
		g_ClusterManager.ListAdd(
				Util::Format("%s:%d",
						LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(),
						newZone.mAccountID),
				Util::Format("%d", newZone.mID), false);
	}
}

bool ZoneDefManager::DeleteZone(int id) {
	if (id < GROVE_ZONE_ID_DEFAULT) {
		g_Logs.data->warn("Request to delete non-grove zone %v, ignoring", id);
		return false;
	}

	bool ok = false;
	ZoneDefInfo *def = GetPointerByID(id);
	if (def != NULL) {
		if (g_ClusterManager.RemoveEntity(def)) {
			g_Logs.data->info("Removed zone %v", id);
			g_ZoneDefManager.RemoveZoneFromIndexes(def);
			g_SceneryManager.DeleteZone(id);
			ok = true;
		}
		mZoneList.erase(mZoneList.find(id));
	}
	return ok;
}

void ZoneDefManager::RemoveZoneFromIndexes(ZoneDefInfo *def) {
	g_ClusterManager.RemoveKey(
			Util::Format("%s:%s", KEYPREFIX_WARP_NAME_TO_ZONE_ID.c_str(),
					def->mWarpName.c_str()), false);
	g_ClusterManager.ListRemove(
			Util::Format("%s:%s",
					LISTPREFIX_GROVE_NAME_TO_ZONE_ID.c_str(),
					def->mGroveName.c_str()),
			Util::Format("%d", def->mID), false);
	g_ClusterManager.ListRemove(
			Util::Format("%s:%d",
					LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(), def->mAccountID),
			Util::Format("%d", def->mID), false);

}

int ZoneDefManager::CheckAutoSave(bool force) {
	if (g_ServerTime < (unsigned long) mNextAutosaveTime && force == false)
		return 0;

	int saveOps = 0; //Number of save operations completed.  The session autosave will want to know if it needs to update in case of zone information.

	ZONEDEF_ITERATOR it;
	for (it = mZoneList.begin(); it != mZoneList.end(); ++it) {
		ZoneDefInfo *def = &it->second;

		def->AutosaveAudits(force); //Audits doesn't use the pending stuff, so we'll just toss it in here.

		saveOps++;
	}
	return saveOps;
}

int ZoneDefManager::EnumerateGroveIds(int searchAccountID, int characterDefId,
		vector<int> &groveIdList) {
	cs.Enter("ZoneDefManager::EnumerateGroveIds");
	ZONEINDEX_ITERATOR it;
	CharacterData *cdata =
			characterDefId == 0 ?
					NULL : g_CharacterManager.GetPointerByID(characterDefId);

	STRINGLIST accountGroves = g_ClusterManager.GetList(
			Util::Format("%s:%d",
					LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(), searchAccountID));
	for (auto a = accountGroves.begin(); a != accountGroves.end(); ++a) {
		groveIdList.push_back(atoi((*a).c_str()));
	}
	if (cdata != NULL) {
		// Check if the zone is a guild hall for a guild the character is in
		for (unsigned int i = 0; i < cdata->guildList.size(); i++) {
			int guildDefID = cdata->guildList[0].GuildDefID;
			GuildDefinition *gDef = g_GuildManager.GetGuildDefinition(
					guildDefID);
			if (gDef == NULL) {
				g_Logs.server->error("Guild definition %v does not exist",
						guildDefID);
			} else {
				groveIdList.push_back(gDef->guildHallZone);
			}
		}
	}
	cs.Leave();
	return static_cast<int>(groveIdList.size());
}


int ZoneDefManager::EnumerateGroves(int searchAccountID, int characterDefId,
		vector<string> &groveList) {
	cs.Enter("ZoneDefManager::EnumerateGroves");
	ZONEINDEX_ITERATOR it;
	CharacterData *cdata =
			characterDefId == 0 ?
					NULL : g_CharacterManager.GetPointerByID(characterDefId);

	STRINGLIST accountGroves = g_ClusterManager.GetList(
			Util::Format("%s:%d",
					LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(), searchAccountID));
	for (auto a = accountGroves.begin(); a != accountGroves.end(); ++a) {
		ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(atoi((*a).c_str()));
		if (zd != NULL)
			groveList.push_back(zd->mWarpName);
	}
	if (cdata != NULL) {
		// Check if the zone is a guild hall for a guild the character is in
		for (unsigned int i = 0; i < cdata->guildList.size(); i++) {
			int guildDefID = cdata->guildList[0].GuildDefID;
			GuildDefinition *gDef = g_GuildManager.GetGuildDefinition(
					guildDefID);
			if (gDef == NULL) {
				g_Logs.server->error("Guild definition %v does not exist",
						guildDefID);
			} else {
				ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(
						gDef->guildHallZone);
				if (zd != NULL)
					groveList.push_back(zd->mWarpName);
			}
		}
	}
	cs.Leave();
	return static_cast<int>(groveList.size());
}

int ZoneDefManager::EnumerateArenas(vector<string> &arenaList) {
	ZONEDEF_ITERATOR it;
	for (it = mZoneList.begin(); it != mZoneList.end(); ++it) {
		if (it->second.mArena == true)
			arenaList.push_back(it->second.mWarpName);
	}
	return static_cast<int>(arenaList.size());
}

void ZoneDefManager::UpdateGroveAccountID(const char *groveName,
		int newAccountID) {
	ZONEDEF_ITERATOR it;
	for (it = mZoneList.begin(); it != mZoneList.end(); ++it) {
		if (it->second.mGrove == false)
			continue;
		string name = it->second.mWarpName;
		size_t pos = name.find_first_of("0123456789");
		if (pos != string::npos)
			name.erase(pos);
		if (name.compare(groveName) == 0) {
			g_Logs.server->info(
					"Resetting grove [ID:%v, Name:%v] account ID from %v to %v",
					it->second.mID, it->second.mWarpName.c_str(),
					it->second.mAccountID, newAccountID);
			it->second.mAccountID = newAccountID;
			it->second.mGroveName = groveName;
			ZoneListChanges.AddChange();
		}
	}
}

void ZoneDefManager::NotifyConfigurationChange(void) {
	ZoneListChanges.AddChange();
}

bool ZoneDefManager::ZoneUnloadReady(void) {
	if (g_ServerTime >= (unsigned long) mNextZoneUnload) {
		mNextZoneUnload = g_ServerTime + ZONE_UNLOAD_DELAY;
		return true;
	}
	return false;
}

void ZoneDefManager::UnloadInactiveZones(vector<int> &activeZones) {
	ZONEDEF_ITERATOR it;
	it = mZoneList.begin();
	while (it != mZoneList.end()) {
		bool has = false;
		for (size_t i = 0; i < activeZones.size(); i++) {
			if (activeZones[i] == it->second.mID) {
				has = true;
				break;
			}
		}
		if ((has == false) && (it->second.QualifyDelete() == true)) {
			g_Logs.server->info("Unloading inactive zone def: %v (%v)",
					it->second.mID, it->second.mWarpName.c_str());
			mZoneList.erase(it++);
		} else
			++it;
	}
}

void ZoneDefManager::GenerateReportActive(ReportBuffer &report) {
	report.AddLine("ZONEDEFS (currently loaded groves)");
	ZONEDEF_ITERATOR it;
	for (it = mZoneList.begin(); it != mZoneList.end(); ++it) {
		ZoneDefInfo *zd = &it->second;
		if (zd->IsPlayerGrove() == false)
			continue;
		report.AddLine("Name:%s ID:%d", zd->mName.c_str(), zd->mID);
		report.AddLine(NULL);
	}
}

MapBarrierPoint::MapBarrierPoint() {
	Clear();
}

void MapBarrierPoint::Clear(void) {
	x1 = 0;
	x2 = 0;
	z1 = 0;
	z2 = 0;
}

bool MapBarrierPoint::isValid(void) {
	return ((x1 + x2 + z1 + z2) != 0);
}

void ZoneBarrierManager::AddEntry(int zoneID, MapBarrierPoint &data) {
	zoneList[zoneID].push_back(data);
}

bool ZoneBarrierManager::CheckCollision(int zoneID, int &x, int &z) {
	BARRIER_CONT::iterator it = zoneList.find(zoneID);
	if (it == zoneList.end())
		return false;

	for (size_t i = 0; i < it->second.size(); i++) {
		if (x < it->second[i].x1)
			continue;
		if (x > it->second[i].x2)
			continue;
		if (z < it->second[i].z1)
			continue;
		if (z > it->second[i].z2)
			continue;

		int test[4];
		test[0] = abs(x - it->second[i].x1);
		test[1] = abs(x - it->second[i].x2);
		test[2] = abs(z - it->second[i].z1);
		test[3] = abs(z - it->second[i].z2);

		int lowest = 0;
		int findex = -1;
		for (int index = 0; index < 4; index++) {
			if (test[index] < lowest || findex == -1) {
				findex = index;
				lowest = test[index];
			}
		}
		if (findex == 0)
			x = it->second[i].x1 - 1;
		else if (findex == 1)
			x = it->second[i].x2 + 1;
		else if (findex == 2)
			z = it->second[i].z1 - 1;
		else if (findex == 3)
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

void ZoneBarrierManager::LoadFromFile(const fs::path &filename) {
	FileReader lfr;
	if (lfr.OpenText(filename) != Err_OK) {
		g_Logs.data->error("Could not load Zone Barrier file: %v", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;

	int curZone = 0;
	MapBarrierPoint newItem;
	newItem.Clear();
	while (lfr.FileOpen() == true) {
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if (r > 0) {
			lfr.BlockToStringC(0, 0);
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (newItem.isValid()) {
					AddEntry(curZone, newItem);
					newItem.Clear();
				}
			} else if (strcmp(lfr.SecBuffer, "1") == 0) {
				newItem.x1 = lfr.BlockToIntC(1);
				newItem.z1 = lfr.BlockToIntC(2);
			} else if (strcmp(lfr.SecBuffer, "2") == 0) {
				newItem.x2 = lfr.BlockToIntC(1);
				newItem.z2 = lfr.BlockToIntC(2);
			} else if (strcmp(lfr.SecBuffer, "Zone") == 0)
				curZone = lfr.BlockToIntC(1);
			else
				g_Logs.data->info(
						"Unknown identifier [%v] in file [%v] on line %v",
						lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	lfr.CloseCurrent();
	if (newItem.isValid())
		AddEntry(curZone, newItem);
}

int ZoneBarrierManager::GetLoadedCount(void) {
	BARRIER_CONT::iterator it;
	size_t count = 0;
	for (it = zoneList.begin(); it != zoneList.end(); ++it)
		count += it->second.size();
	return static_cast<int>(count);
}

unsigned long EnvironmentCycle::GetDuration() {
	if (mStart > mEnd) {
		/* If start is greater than end, then this cycle cross midnight */
		return mEnd + ( DAY_MS - mStart);
	} else {
		return mEnd - mStart;
	}
}

unsigned long EnvironmentCycle::GetNext() {
	unsigned long now = g_PlatformTime.getPseudoTimeOfDayMilliseconds();
	if (mStart > mEnd) {
		/* If start is greater than end, then this cycle cross midnight */
		if (mEnd < now)
			return ( DAY_MS - now) + mEnd;
		else
			return mEnd - now;
	} else {
		if (mEnd > now)
			return mEnd - now;
		else {
			return ( DAY_MS - now) + mEnd;
		}
	}
}

bool EnvironmentCycle::InRange(unsigned long time) {
	if (mStart > mEnd) {
		/* If start is greater than end, then this cycle cross midnight */
		return (time >= mStart && time < DAY_MS) || time < mEnd;
	} else {
		return time >= mStart && time < mEnd;
	}
}

EnvironmentCycleManager::EnvironmentCycleManager() {
	mChangeTaskID = 0;
	//Environment default starts at day.
}

EnvironmentCycleManager::~EnvironmentCycleManager() {
}

void EnvironmentCycleManager::Init() {
	//ApplyConfig("Sunrise=05:30,Day=08:30,Sunset=18:00,Night=20:30");
	ApplyConfig(g_GameConfig.EnvironmentCycle);
	RescheduleUpdate();
}

void EnvironmentCycleManager::RescheduleUpdate() {
	if (mChangeTaskID != 0)
		g_Scheduler.Cancel(mChangeTaskID);

	EnvironmentCycle current = GetCurrentCycle();
	unsigned long next = current.GetNext();
	unsigned long wait = next / TIME_FACTOR;
	g_Logs.server->info(
			"Will switch environment from %v [Start: %v  End: %v   Next: %v (%v)] in %vms (%v)",
			current.mName, Util::FormatTimeHHMMSS(current.mStart),
			Util::FormatTimeHHMMSS(current.mEnd),
			Util::FormatTimeHHMMSS(next), next, wait,
			Util::FormatTimeHHMMSS(wait));
	mChangeTaskID = g_Scheduler.Schedule([this]() {
		string tod = GetCurrentCycle().mName;
		g_Logs.server->info("Switching environment to %v", tod);
		for (size_t i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++) {
	g_ActiveInstanceManager.instListPtr[i]->UpdateEnvironmentCycle(tod);
}
		RescheduleUpdate();
	}, wait + g_PlatformTime.getMilliseconds());

}

void EnvironmentCycleManager::ApplyConfig(const string str) {
	//Config expects a list of time ranges
	//Ex: Sunrise=05:30,Day=08:30,Sunset=18:00,Night=20:30

	mConfig = str;

	STRINGLIST pairs;
	STRINGLIST keyval;
	Util::Split(mConfig, ",", pairs);
	EnvironmentCycle prev;
	for (size_t i = 0; i < pairs.size(); i++) {
		Util::Split(pairs[i], "=", keyval);
		if (keyval.size() < 2)
			continue;
		EnvironmentCycle c;
		c.mName = keyval[0];

		unsigned long ms = Util::ParseTimeHHMM(keyval[1]);

		if (i > 0) {
			/* Fill in the previous cycles end time */
			prev.mEnd = ms;
			mCycles[i - 1] = prev;
			g_Logs.server->info("Environment cycle %v runs from %v to %v",
					prev.mName, Util::FormatTimeHHMM(prev.mStart),
					Util::FormatTimeHHMM(prev.mEnd));
		}

		c.mStart = ms;
		mCycles.push_back(c);
		prev = c;
	}
	if (mCycles.size() > 0) {
		mCycles[mCycles.size() - 1].mEnd = mCycles[0].mStart;
		g_Logs.server->info("Environment cycle %v runs from %v to %v",
				mCycles[mCycles.size() - 1].mName,
				Util::FormatTimeHHMM(mCycles[mCycles.size() - 1].mStart),
				Util::FormatTimeHHMM(mCycles[mCycles.size() - 1].mEnd));
	}

}

EnvironmentCycle EnvironmentCycleManager::GetCurrentCycle(void) {
	unsigned long pseudoTime = g_PlatformTime.getPseudoTimeOfDayMilliseconds();
	for (auto it = mCycles.begin(); it != mCycles.end(); ++it) {
		if ((*it).InRange(pseudoTime)) {
			return *it;
		}
	}
	return mCycles[0];

}

GroveTemplate::GroveTemplate() {
	Clear();
}

void GroveTemplate::Clear(void) {
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

bool GroveTemplate::HasProps() const {
	return fs::exists(g_Config.ResolveStaticDataPath() / "GroveTemplates" / mShortName);
}

void GroveTemplate::GetProps(vector<SceneryObject> &objects) const {
	auto fileName = g_Config.ResolveStaticDataPath() / "GroveTemplates" /  mShortName;

	for(const fs::directory_entry& entry : fs::directory_iterator(fileName)) {
		auto file = entry.path();
		if(file.stem() == ".txt") {
			SceneryPage page;
			page.LoadSceneryFromFile(file);
			for (auto mit = page.mSceneryList.begin(); mit != page.mSceneryList.end(); ++mit) {
				objects.push_back((*mit).second);
			}
		}
	}
}

GroveTemplateManager::GroveTemplateManager() {
}

void GroveTemplateManager::LoadData(void) {
	LoadFile(g_Config.ResolveStaticDataPath() / "Data" / "GroveTemplate.txt");

	// Need to set up the TerrainCfg lookup table.
	ResolveTerrainMap();
	g_Logs.data->info("Loaded %v Grove Templates", mTemplateEntries.size());
}

void GroveTemplateManager::LoadFile(const fs::path &filename) {
	FileReader3 fr;
	if (fr.OpenFile(filename) != fr.SUCCESS) {
		g_Logs.data->error("Unable to open grove template file [%v]", filename);
		return;
	}

	fr.SetCommentChar(';');
	fr.ReadLine(); //Assume first line is header.

	while (fr.Readable() == true) {
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if (r > 1) {
			string key = fr.BlockToStringC(0);
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

const GroveTemplate* GroveTemplateManager::GetTemplateByShortName(
		const char *name) {
	map<string, GroveTemplate>::iterator it;
	it = mTemplateEntries.find(name);
	if (it != mTemplateEntries.end())
		return &it->second;
	return NULL;
}

const GroveTemplate* GroveTemplateManager::GetTemplateByTerrainCfg(
		const char *cfg) {
	map<string, const GroveTemplate*>::iterator it;
	it = mTerrainMap.find(cfg);
	if (it != mTerrainMap.end())
		return it->second;
	return NULL;
}

void GroveTemplateManager::ResolveTerrainMap(void) {
	mTerrainMap.clear();
	map<string, GroveTemplate>::iterator it;
	for (it = mTemplateEntries.begin(); it != mTemplateEntries.end(); ++it) {
		mTerrainMap[it->second.mTerrainCfg] = &it->second;
	}
}

//
// Weather
//

unsigned long RandomFutureTime(int min, int max) {
	return g_ServerTime
			+ ((unsigned long) (g_RandomManager.RandModRng(min, max) * 1000l) / TIME_FACTOR);
}

WeatherDef::WeatherDef() {
	Clear();
}

WeatherDef::WeatherDef(const WeatherDef &other) {
	CopyFrom(other);
}

void WeatherDef::CopyFrom(const WeatherDef &other) {
	mMapName = other.mMapName;
	mUse = other.mUse;
	mTimeOfDay = other.mTimeOfDay;
	mFineMin = other.mFineMin;
	mFineMax = other.mFineMax;
	mLightChance = other.mLightChance;
	mMediumChance = other.mMediumChance;
	mHeavyChance = other.mHeavyChance;
	mWeatherMin = other.mWeatherMin;
	mWeatherMax = other.mWeatherMax;
	mWeatherTypes = other.mWeatherTypes;
	mThunderChance = other.mThunderChance;
	mThunderGapMin = other.mThunderGapMin;
	mThunderGapMax = other.mThunderGapMax;
	mEscalateChance = other.mEscalateChance;
}

void WeatherDef::Clear() {
	mMapName = "";
	mUse = "";
	mTimeOfDay = "";
	mFineMin = 0;
	mFineMax = 99999999;
	mLightChance = 0;
	mMediumChance = 0;
	mHeavyChance = 0;
	mWeatherMin = 0;
	mWeatherMax = 0;
	mWeatherTypes.clear();
	mThunderChance = 0;
	mThunderGapMin = 0;
	mThunderGapMax = 0;
	mEscalateChance = 0;

}

void WeatherDef::SetDefaults() {
	Clear();
}

WeatherState::WeatherState(int instanceId, WeatherDef &def) {
	mInstanceId = instanceId;
	mDefinition = def;
	mNextStateChange = RandomFutureTime(def.mFineMin, def.mFineMax);
	mWeatherType = "";
	mWeatherWeight = WeatherState::LIGHT;
	mEscalateState = WeatherState::ONE_OFF;
	mThunder = false;
	mNextThunder = 0;

	if (g_ServerTime >= mNextStateChange) {
		PickNewWeather();
	}
}

WeatherState::~WeatherState() {
}

void WeatherState::RunCycle(ActiveInstance *instance) {
	if (mThunder && g_ServerTime >= mNextThunder) {
		if (g_ClusterManager.IsMaster()
				|| !instance->mZoneDefPtr->IsOverworld()) {
			/* If on the master, or this is not the overworld, calculate next thunder and notify all players on this shard */
			g_Logs.server->debug("Thunder state change");
			mNextThunder = RandomFutureTime(mDefinition.mThunderGapMin,
					mDefinition.mThunderGapMax);
			SendThunder(instance, true);
		} else {
			/* Disable timed thunder, the cluster will trigger it */
			mNextThunder = 0;
			mThunder = false;
		}

	}

	if (mNextStateChange != 0 && g_ServerTime > mNextStateChange) {

		if (g_ClusterManager.IsMaster()
				|| !instance->mZoneDefPtr->IsOverworld()) {
			/* If on the master, or this is not the overworld, calculate next thunder and notify all players on this shard */
			g_Logs.server->debug("Weather state change");

			if (mEscalateState == WeatherState::ONE_OFF) {
				if (PickNewWeather()) {
					SendWeatherUpdate(instance, true);
				}
			} else if (mEscalateState == WeatherState::ESCALATING) {
				mWeatherWeight++;
				if (mWeatherWeight >= WeatherState::MAX_WEIGHT) {
					mEscalateState = WeatherState::DEESCALATING;
					mWeatherWeight = WeatherState::MAX_WEIGHT - 2;
				}
				mNextStateChange = RandomFutureTime(mDefinition.mWeatherMin,
						mDefinition.mWeatherMax);
				SendWeatherUpdate(instance, true);

				/* During escalation we also roll for thunder to start */
				if (!mThunder)
					RollThunder();
			} else if (mEscalateState == WeatherState::DEESCALATING) {
				mWeatherWeight--;
				if (mWeatherWeight < 0) {
					/* Completely de-escalated */
					if (PickNewWeather()) {
						SendWeatherUpdate(instance, true);
					}
				} else {
					mNextStateChange = RandomFutureTime(mDefinition.mWeatherMin,
							mDefinition.mWeatherMax);
					SendWeatherUpdate(instance, true);
				}

				/* During de-escalation we also roll for thunder to stop using the inverse chance that was used to start */
				if (mThunder) {
					int chance = g_RandomManager.RandModRng(0, 100);
					mThunder = chance < (100 - mDefinition.mThunderChance);
					if (!mThunder) {
						mNextThunder = 0;
					}
				}
			} else
				mNextStateChange = 0;
		} else {
			/* Disable timed weather, the cluster will take care of it */
			mNextStateChange = 0;
		}
	}
}

void WeatherState::SendWeatherUpdate(ActiveInstance *instance,
		bool sentToCluster) {
	for (vector<SimulatorThread*>::iterator it = instance->RegSim.begin();
			it != instance->RegSim.end(); ++it) {
		for (vector<string>::iterator it2 = mMapNames.begin();
				it2 != mMapNames.end(); it2++) {
			SimulatorThread *sim = *it;
			if (sim->pld.CurrentMapInt == -1
					|| MapDef.mMapList[sim->pld.CurrentMapInt].Name.compare(
							*it2) == 0) {
				sim->AttemptSend(sim->Aux1,
						PrepExt_SetWeather(sim->Aux1, mWeatherType,
								mWeatherWeight));
				if (sentToCluster && g_ClusterManager.IsMaster()
						&& instance->mZoneDefPtr->IsOverworld()) {
					/* If this is an overworld zone, notify all remote players of thunder */
					g_ClusterManager.Weather(instance->mZone,
							sim->pld.CurrentMapInt == -1 ? "" : MapDef.mMapList[sim->pld.CurrentMapInt].Name,
							mWeatherType, mWeatherWeight);
				}
				break;
			}
		}
	}
}
void WeatherState::SendThunder(ActiveInstance *instance, bool sendToCluster) {
	for (vector<SimulatorThread*>::iterator it = instance->RegSim.begin();
			it != instance->RegSim.end(); ++it) {
		SimulatorThread *sim = *it;
		for (vector<string>::iterator it2 = mMapNames.begin();
				it2 != mMapNames.end(); it2++) {
			if (sim->pld.CurrentMapInt == -1
					|| MapDef.mMapList[sim->pld.CurrentMapInt].Name.compare(
							*it2) == 0) {
				sim->AttemptSend(sim->Aux1,
						PrepExt_Thunder(sim->Aux1, mWeatherWeight));
				if (g_ClusterManager.IsMaster()
						&& instance->mZoneDefPtr->IsOverworld()) {
					/* If this is an overworld zone, notify all remote players of thunder */
					g_ClusterManager.Thunder(instance->mZone,
							sim->pld.CurrentMapInt == -1 ? "" : MapDef.mMapList[sim->pld.CurrentMapInt].Name);
				}
				break;
			}
		}
	}
}

void WeatherState::RollThunder() {
	int chance = g_RandomManager.RandModRng(0, 100);
	mThunder = chance < mDefinition.mThunderChance;
	mNextThunder = 0;
	if (mThunder) {
		mNextThunder = RandomFutureTime(mDefinition.mThunderGapMin,
				mDefinition.mThunderGapMax);
	}
}

bool WeatherState::PickNewWeather() {
	if (mWeatherType.length() == 0) {

		/* Weather is starting */
		int chance = g_RandomManager.RandModRng(0, 100);
		if (chance < mDefinition.mHeavyChance) {
			mWeatherWeight = WeatherState::HEAVY;
		} else if (chance < mDefinition.mMediumChance) {
			mWeatherWeight = WeatherState::MEDIUM;
		} else if (chance < mDefinition.mLightChance) {
			mWeatherWeight = WeatherState::LIGHT;
		} else {
			/* No weather to start, wait for next cycle */
			mNextStateChange = RandomFutureTime(mDefinition.mFineMin,
					mDefinition.mFineMax);
			return false;
		}

		mNextStateChange = RandomFutureTime(mDefinition.mWeatherMin,
				mDefinition.mWeatherMax);
		mWeatherType = mDefinition.mWeatherTypes[g_RandomManager.RandModRng(0,
				mDefinition.mWeatherTypes.size())];

		/* Roll for thunder */
		RollThunder();

		/* Roll for escalation */
		chance = g_RandomManager.RandModRng(0, 100);
		mEscalateState = WeatherState::ONE_OFF;
		if (chance < mDefinition.mEscalateChance) {
			mEscalateState = WeatherState::ESCALATING;
		}
	} else {
		StopWeather();
	}

	return true;
}

void WeatherState::StopWeather() {
	/* Weather is stopping */
	mThunder = false;
	mNextThunder = 0;
	mNextStateChange = RandomFutureTime(mDefinition.mFineMin,
			mDefinition.mFineMax);
	mWeatherType = "";
	mWeatherWeight = WeatherState::LIGHT;
	mEscalateState = WeatherState::ONE_OFF;
}

bool WeatherManager::MaybeAddWeatherDef(int instanceID,
		string actualMapName, vector<WeatherState*> &m) {
	if (mWeatherDefinitions.find(actualMapName) == mWeatherDefinitions.end())
		return false;

	WeatherDef wdef = mWeatherDefinitions[actualMapName];
	while (wdef.mUse.length() != 0)
		wdef = mWeatherDefinitions[wdef.mUse];

	/* When 'use' is used, both the parent map name and this map name are stored in the
	 * mWeather map, but both pointing to the same state instance
	 */
	WeatherKey k;
	k.instance = instanceID;
	k.mapName = actualMapName;

	WeatherState *state = NULL;

	if (mWeather.find(k) == mWeather.end()) {
		/* Not currently available under the actual map name, is it availabe under the parent? */
		if (wdef.mMapName.compare(actualMapName) != 0) {
			/* Using 'use' */
			k.mapName = wdef.mMapName;
			if (mWeather.find(k) == mWeather.end()) {
				/* Not available under parent, create it as parent */
				state = new WeatherState(instanceID, wdef);
				m.push_back(state);
				mWeather[k] = state;
				mWeather[k]->mMapNames.push_back(wdef.mMapName);
			}

			/* Available under the parent map name, add the actual name to the weather map */
			WeatherKey k2;
			k2.instance = instanceID;
			k2.mapName = actualMapName;
			mWeather[k2] = mWeather[k];
			mWeather[k]->mMapNames.push_back(actualMapName);
		} else {
			/* Not using 'use' */
			state = new WeatherState(instanceID, wdef);
			state->mMapNames.push_back(actualMapName);
			m.push_back(state);
			mWeather[k] = state;
		}
	}

	return true;
}

vector<WeatherState*> WeatherManager::RegisterInstance(
		ActiveInstance *instance) {
	vector<MapDefInfo> d;
	vector<WeatherState*> m;

	MapDef.GetZone(instance->mZoneDefPtr->mMapName.c_str(), d);

	if (d.size() == 0) {
		/* If no mapdefs for this zone, assume a single area of weather for the whole instance */
		MapDefInfo dd;
		dd.Name = instance->mZoneDefPtr->mName;
		d.push_back(dd);
	}

	if (!g_Config.UseWeather) {
		g_Logs.server->info(
				"Not adding weather system as weather is turned off globally.");
		return m;
	}

	/* Set up a weather state for all the MapDefInfo in this instance that have a weather def */
	for (vector<MapDefInfo>::iterator it = d.begin(); it != d.end();
			++it) {
		if (!MaybeAddWeatherDef(instance->mInstanceID, (*it).Name, m))
			continue;
	}

	return m;
}

void WeatherManager::ZoneThunder(int zoneId, string mapName) {
	/* This is only for overworld zones triggered over the cluster, so there will only be one active instance */
	ActiveInstance *inst = g_ActiveInstanceManager.GetPtrByZoneID(zoneId);
	if (inst == NULL)
		g_Logs.server->warn(
				"Request for thunder on zone %v but no active instance found.",
				zoneId);
	else {
		WeatherState *state = GetWeather(mapName, inst->mInstanceID);
		if (state == NULL) {
			g_Logs.server->warn("No weather state for %v:%v.", mapName,
					inst->mInstanceID);
		} else {
			state->SendThunder(inst, false);
		}
	}
}

void WeatherManager::ZoneWeather(int zoneId, string mapName,
		string weatherType, int weight) {
	/* This is only for overworld zones triggered over the cluster, so there will only be one active instance */
	ActiveInstance *inst = g_ActiveInstanceManager.GetPtrByZoneID(zoneId);
	if (inst == NULL)
		g_Logs.server->warn(
				"Request for weather on zone %v but no active instance found.",
				zoneId);
	else {
		WeatherState *state = GetWeather(mapName, inst->mInstanceID);
		if (state == NULL) {
			g_Logs.server->warn("No weather state for %v:%v.", mapName,
					inst->mInstanceID);
		} else {
			state->mWeatherType = weatherType;
			state->mWeatherWeight = weight;
			state->SendWeatherUpdate(inst, false);
		}
	}
}

WeatherState* WeatherManager::GetWeather(string mapName, int instanceId) {

	WeatherKey k;
	k.instance = instanceId;
	k.mapName = mapName;
	return mWeather[k];
}

void WeatherManager::Deregister(vector<WeatherState*> *states) {

	/* Set up a weather state for all the map locations in this instance that have a weather def */
	for (vector<WeatherState*>::iterator it = states->begin();
			it != states->end(); ++it) {
		WeatherState *ws = *it;

		if(g_Logs.simulator->enabled(el::Level::Trace)) {
			g_Logs.simulator->trace("Clearing up weather for %v (%v)",
					ws->mInstanceId, ws->mDefinition.mMapName.c_str());
		}

		for (vector<string>::iterator it2 = ws->mMapNames.begin();
				it2 != ws->mMapNames.end(); ++it2) {

			if(g_Logs.simulator->enabled(el::Level::Trace)) {
				g_Logs.simulator->trace("    Map (%v)", (*it2).c_str());
			}

			WeatherKey k;
			k.instance = ws->mInstanceId;
			k.mapName = *it2;
			mWeather.erase(k);
		}

		delete ws;
	}

	states->clear();
}

int WeatherManager::LoadFromFile(string fileName) {

	//Note: the official grove file is loaded first, then the custom grove file.
	//This should point here.
	FileReader lfr;
	if (lfr.OpenText(fileName) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", fileName);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;
	WeatherDef newItem;

	while (lfr.FileOpen() == true) {
		int r = lfr.ReadLine();
		if (r > 0) {
			r = lfr.BreakUntil("=", '='); //Don't use SingleBreak since we won't be able to re-split later.
			lfr.BlockToStringC(0, Case_Upper);
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (newItem.mMapName.length() != 0) {
					WeatherDef d(newItem);
					mWeatherDefinitions[newItem.mMapName] = d;
					newItem.Clear();
				}
			} else {
				if (strcmp(lfr.SecBuffer, "NAME") == 0)
					newItem.mMapName = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "USE") == 0)
					newItem.mUse = lfr.BlockToStringC(1, 0);
				else if (strcmp(lfr.SecBuffer, "FINEMIN") == 0)
					newItem.mFineMin = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "FINEMAX") == 0)
					newItem.mFineMax = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "LIGHTCHANCE") == 0)
					newItem.mLightChance = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "MEDIUMCHANCE") == 0)
					newItem.mMediumChance = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "HEAVYCHANCE") == 0)
					newItem.mHeavyChance = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "WEATHERMIN") == 0)
					newItem.mWeatherMin = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "WEATHERMAX") == 0)
					newItem.mWeatherMax = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "WEATHERTYPE") == 0) {
					r = lfr.MultiBreak("=,"); //Re-split for this particular data.
					for (int s = 1; s < r; s++) {
						string typeStr = lfr.BlockToStringC(1, 0);
						newItem.mWeatherTypes.push_back(typeStr);
					}
				} else if (strcmp(lfr.SecBuffer, "THUNDERCHANCE") == 0)
					newItem.mThunderChance = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "THUNDERGAPMIN") == 0)
					newItem.mThunderGapMin = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "THUNDERGAPMAX") == 0)
					newItem.mThunderGapMax = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "ESCALATE") == 0)
					newItem.mEscalateChance = lfr.BlockToIntC(1);
				else
					g_Logs.server->error(
							"Unknown identifier [%v] while reading from file %v.",
							lfr.SecBuffer, fileName);
			}
		}
	}
	lfr.CloseCurrent();

	if (newItem.mMapName.length() != 0) {
		WeatherDef d;
		d.SetDefaults();
		d.CopyFrom(newItem);
		mWeatherDefinitions[newItem.mMapName] = d;
	}
	return 0;
}

int PrepExt_SendEnvironmentUpdateMsg(char *buffer, ActiveInstance *instance,
		const char *zoneIDString, ZoneDefInfo *zoneDef, int x, int z,
		int mask) {
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 42);   //_handleEnvironmentUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutByte(&buffer[wpos], mask);   //Mask

	wpos += PutStringUTF(&buffer[wpos], zoneIDString);
	wpos += PutInteger(&buffer[wpos], zoneDef->mID);
	wpos += PutShort(&buffer[wpos], zoneDef->mPageSize);
	wpos += PutStringUTF(&buffer[wpos], zoneDef->mTerrainConfig.c_str());
	if (instance == NULL) {
		wpos += PutStringUTF(&buffer[wpos],
				zoneDef->GetTileEnvironment(x, z).c_str());
	} else {
		wpos += PutStringUTF(&buffer[wpos],
				instance->GetEnvironment(x, z).c_str());
	}
	wpos += PutStringUTF(&buffer[wpos], zoneDef->mMapName.c_str());

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}



