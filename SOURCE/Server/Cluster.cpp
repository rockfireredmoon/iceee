#include "Cluster.h"
#include "Account.h"
#include "Config.h"
#include "Components.h"
#include "Scheduler.h"
#include "Util.h"
#include "StringUtil.h"
#include "Simulator.h"
#include "util/Log.h"
#include "FileReader.h"
#include "Character.h"
#include "AuctionHouse.h"
#include "Chat.h"
#include "FriendStatus.h"
#include "Instance.h"
#include <cpp_redis/cpp_redis>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <stdlib.h>     /* atoi */

#define CLUSTER_PING_INTERVAL 30000
#define CLUSTER_SHARD_TIMEOUT 60000
#define DEFAULT_SHARD_NAME "Default shard name for ICEEE Earth Eternal server"

ClusterManager g_ClusterManager;

static string SERVER_STARTED = "server-started";
static string SERVER_STOPPED = "server-stopped";
static string SERVER_PING = "server-ping";
static string SERVER_PONG = "server-pong";
static string SERVER_CONFIGURATION = "server-configuration";
static string SIM_TRANSFER = "sim-transfer";
static string SERVER_RECONFIGURE = "server-reconfigure";
static string PLAYER_JOINED_SHARD = "player-joined-shard";
static string PLAYER_LEFT_SHARD = "player-left-shard";
static string CHAT = "chat";
static string WEATHER = "weather";
static string THUNDER = "thunder";
static string LOGIN = "login";
static string LOGOUT = "logout";
static string AUCTION_ITEM = "auction-item";
static string AUCTION_ITEM_REMOVED = "auction-item-removed";
static string AUCTION_ITEM_UPDATED = "auction-item-updated";
static string CONFIRM_TRANSFER = "confirm-transfer";
static string PROP_UPDATED = "prop-updated";

using namespace std;

string RedisConnectStatus(cpp_redis::connect_state status) {
	switch (status) {
	case cpp_redis::connect_state::dropped:
		return "Dropped";
	case cpp_redis::connect_state::failed:
		return "Failed";
	case cpp_redis::connect_state::lookup_failed:
		return "Lookup Failed";
	case cpp_redis::connect_state::ok:
		return "OK";
	case cpp_redis::connect_state::sleeping:
		return "Sleeping";
	case cpp_redis::connect_state::start:
		return "Start";
	case cpp_redis::connect_state::stopped:
		return "Stopped";
	default:
		return "Unknown";
	}
}

//
// ClusterLogger
//

ClusterLogger::ClusterLogger() {
}

void ClusterLogger::debug(const string &msg, const string &file, size_t line) {
	g_Logs.cluster->debug("[%v:%v] %v", file, line, msg);
}

void ClusterLogger::info(const string &msg, const string &file, size_t line) {
	g_Logs.cluster->info("[%v:%v] %v", file, line, msg);
}

void ClusterLogger::warn(const string &msg, const string &file, size_t line) {
	g_Logs.cluster->warn("[%v:%v] %v", file, line, msg);

}

void ClusterLogger::error(const string &msg, const string &file, size_t line) {
	g_Logs.cluster->error("[%v:%v] %v", file, line, msg);

}

//
// ClusterEntityReader
//
ClusterEntityReader::ClusterEntityReader(cpp_redis::client *client) {
	mClient = client;
	mGotSections = false;
	mExists = false;
	mTestedExistence = false;
}

ClusterEntityReader::~ClusterEntityReader() {
}

bool ClusterEntityReader::Start() {
	return true;
}

bool ClusterEntityReader::Abort() {
	return true;
}

bool ClusterEntityReader::End() {
	return true;
}

bool ClusterEntityReader::Exists() {
	if (!mClient->is_connected())
		return false;
	if (!mTestedExistence) {
		mTestedExistence = true;
		auto get = mClient->exists( { CreateSectionKey() });
		mClient->sync_commit();
		auto reply = get.get();
		if (reply.is_integer()) {
			mExists = reply.as_integer() == 1;
		}
	}
	return mExists;
}

vector<string> ClusterEntityReader::Sections() {
	if (!mGotSections) {
		mGotSections = true;
		/* Read the list of all the subkeys created for this entity.  */
		string listKey = StringUtil::Format("%s:%s^%s", mCatalog.c_str(),
				mID.c_str(), "_SUBKEYS_");
		auto getListKey = mClient->lrange(listKey, 0, -1);
		mClient->sync_commit();
		auto reply = getListKey.get();
		set<string> currentKeys;
		vector<cpp_redis::reply> arr = reply.as_array();
		for (auto a = arr.begin(); a != arr.end(); ++a) {
			if ((*a).is_string()) {
				mSections.push_back(
						(*a).as_string().substr(
								mCatalog.length() + mID.length() + 2));
			}
		}

		/* We also now know existence */
		if (!mTestedExistence) {
			mTestedExistence = true;
			mExists = mSections.size() > 0;
		}
	}

	/* Now find those that are in the current section path (not recursively either) */
	STRINGLIST l;
	for (auto a = mSections.begin(); a != mSections.end(); ++a) {
		if (Util::HasBeginning(*a, mSection)) {
			string s = (*a).substr(
					mSection.length() == 0
							|| (*a).length() == mSection.length() ?
							0 : mSection.length() + 1);
			if (s.find_first_of("/") == string::npos) {
				l.push_back(*a);
			}
		}
	}

	return l;
}

string ClusterEntityReader::CreateSectionKey() {
	if (mSection.length() == 0)
		return StringUtil::Format("%s:%s:DEFAULT", mCatalog.c_str(),
				mID.c_str());
	else
		return StringUtil::Format("%s:%s:%s", mCatalog.c_str(), mID.c_str(),
				mSection.c_str());
}

void ClusterEntityReader::CheckLoaded() {
	if (mValues.find(mSection) == mValues.end()) {
		string secKey = CreateSectionKey();
		auto secGet = mClient->hgetall(secKey);
		mClient->sync_commit();
		cpp_redis::reply rep = secGet.get();
		auto arr = rep.as_array();
		for (auto a = arr.begin(); a != arr.end(); ++a) {
			string n = (*a).as_string();
			string v = (*++a).as_string();
			KEY_VAL_PAIR p(n, v);
			mValues[mSection].push_back(p);
		}
	}
}

vector<string> ClusterEntityReader::Keys() {
	CheckLoaded();
	KEY_VAL_LIST l = mValues[mSection];
	STRINGLIST k;
	for (auto a = l.begin(); a != l.end(); ++a) {
		k.push_back(a->first);
	}
	return k;
}

string ClusterEntityReader::Value(const string &key, string defaultValue) {
	CheckLoaded();
	KEY_VAL_LIST l = mValues[mSection];

	// TODO make a map?
	for (auto a = l.begin(); a != l.end(); ++a) {
		if ((*a).first.compare(key) == 0) {
			return (*a).second;
		}
	}

	return defaultValue;
}

vector<string> ClusterEntityReader::ListValue(const string &key,
		const string &separator) {
	string v = Value(key);
	if (v.length() == 0)
		return {};
	STRINGLIST l;
	STRINGLIST o;
	string sep = separator;
	if (sep.length() == 0)
		sep = ",";
	Util::SplitEscaped(v, ",", l);
	for (auto a = l.begin(); a != l.end(); ++a) {
		o.push_back(
				StringUtil::ReplaceAll(
						StringUtil::ReplaceAll((*a),
								StringUtil::Format("\\%s", sep.c_str()), sep),
						"\\\\", "\\"));
	}
	return o;
}

//
// ClusterEntityWriter
//
ClusterEntityWriter::ClusterEntityWriter(cpp_redis::client *client, bool sync) {
	mClient = client;
	mSync = sync;
}

ClusterEntityWriter::~ClusterEntityWriter() {
}

bool ClusterEntityWriter::Start() {
	return true;
}

bool ClusterEntityWriter::Abort() {
	mStringValues.clear();
	/// TODO does client need to rollback?
	return true;
}

bool ClusterEntityWriter::End() {
	string sec;
	string secKey = StringUtil::Format("%s:%s", mCatalog.c_str(), mID.c_str());

	g_Logs.cluster->debug("Writing entity %v", secKey);

	vector<pair<string, string>> a;
	string lastSec;

	/* Write a key that contains a list of all the subkeys created for this entity. First
	 * get the key. If it exists, remove all the existing keys first. Then we trim
	 * the list, and then add the new ones on the fly */
	string listKey = StringUtil::Format("%s^%s", secKey.c_str(), "_SUBKEYS_");
	auto getListKey = mClient->lrange(listKey, 0, -1);
	mClient->sync_commit();
	auto reply = getListKey.get();
	set<string> currentKeys;
	vector<cpp_redis::reply> arr = reply.as_array();
	for (vector<cpp_redis::reply>::iterator it = arr.begin(); it != arr.end();
			++it) {
		if ((*it).is_string()) {
			g_Logs.cluster->debug("Existing entity key %v", (*it).as_string());
			currentKeys.insert((*it).as_string());
		}
	}

	for (auto it = mStringValues.begin(); it != mStringValues.end(); ++it) {
		ClusterEntityStringValue val = *it;
		if (val.section.compare(sec) != 0) {
			if (a.size() > 0) {
				if (sec.length() > 0)
					secKey = StringUtil::Format("%s:%s:%s", mCatalog.c_str(),
							mID.c_str(), sec.c_str());
				else
					secKey = StringUtil::Format("%s:%s:DEFAULT",
							mCatalog.c_str(), mID.c_str());
				auto ck = find(currentKeys.begin(), currentKeys.end(), secKey);
				g_Logs.cluster->debug("Section key %v exists", secKey);
				if (ck != currentKeys.end())
					currentKeys.erase(ck);
				mClient->lrem(listKey, 0, secKey);
				mClient->rpush(listKey, { secKey });
				mClient->hmset(secKey, a);
			}
			sec = val.section;
			a.clear();
		}
		a.push_back(pair<string, string>(val.key, val.value));
	}
	if (a.size() > 0) {
		if (sec.length() > 0)
			secKey = StringUtil::Format("%s:%s:%s", mCatalog.c_str(),
					mID.c_str(), sec.c_str());
		else
			secKey = StringUtil::Format("%s:%s:DEFAULT", mCatalog.c_str(),
					mID.c_str());
		auto ck = find(currentKeys.begin(), currentKeys.end(), secKey);
		g_Logs.cluster->debug("Section key %v exists", secKey);
		if (ck != currentKeys.end())
			currentKeys.erase(ck);
		mClient->lrem(listKey, 0, secKey);
		mClient->rpush(listKey, { secKey });
		mClient->hmset(secKey, a);
	}
	if (currentKeys.size() > 0) {
		for (auto it = currentKeys.begin(); it != currentKeys.end(); ++it) {
			g_Logs.cluster->debug("Clearing %v", secKey);
			mClient->del( { *it });
		}
	}
	if (mSync)
		mClient->sync_commit();
	else
		mClient->commit();
	return true;
}

bool ClusterEntityWriter::Value(const string &key, const string &value) {
	mStringValues.push_back( { mSection, key, value });
	return true;
}

bool ClusterEntityWriter::ListValue(const string &key, vector<string> &value) {
	string v;
	for (auto it = value.begin(); it != value.end(); ++it) {
		if (v.size() > 0)
			v += ",";
		v += StringUtil::ReplaceAll(StringUtil::ReplaceAll((*it), "\\", "\\\\"),
				",", "\\,");
	}
	mStringValues.push_back( { mSection, key, v });
	return true;
}

//
// ShardPlayer
//
bool ShardPlayer::IsLocal() {
	return mShard.compare(g_ClusterManager.mShardName) == 0;
}

bool ShardPlayer::IsRemote() {
	return mShard.compare(g_ClusterManager.mShardName) != 0;
}

//
// Shard
//

Shard::Shard() {
	mName = "";
	mLastSeen = 0;
	mSimulatorPort = 0;
	mSimulatorAddress = "";
	mHTTPAddress = "";
	mPlayers = 0;
	mFullName = DEFAULT_SHARD_NAME;
	mServerTime = 0;
	mStartTime = 0;
	mPing = 0;
	mTimeSet = 0;
	mLocalTime = 0;
}

void Shard::WriteToJSON(Json::Value &value) {
	value["name"] = mName;
	value["lastSeen"] = Json::UInt64(mLastSeen);
	value["simulatorPort"] = mSimulatorPort;
	value["simulatorAddress"] = mSimulatorAddress;
	value["httpAddress"] = mHTTPAddress;
	value["players"] = mPlayers;
	value["fullName"] = mFullName;
	value["serverTime"] = Json::UInt64(mServerTime);
	value["startTime"] = Json::UInt64(mStartTime);
	value["ping"] = mPing;
	value["timeSet"] = Json::UInt64(mTimeSet);
	value["localTime"] = Json::UInt64(mLocalTime);
}

bool Shard::IsMaster() {
	return g_ClusterManager.GetMaster().compare(mName) == 0;
}

unsigned long Shard::GetLocalTime() {
	return mLocalTime
			+ (mTimeSet == 0 ? 0 : g_PlatformTime.getMilliseconds() - mTimeSet);
}
unsigned long Shard::GetServerTime() {
	return mServerTime
			+ (mTimeSet == 0 ? 0 : g_PlatformTime.getMilliseconds() - mTimeSet);
}

void Shard::SetTimes(unsigned long localTime, unsigned long serverTime) {
	mTimeSet = g_PlatformTime.getMilliseconds();
	mLocalTime = localTime;
	mServerTime = serverTime;
}

//
// ClusterManager
//

ClusterManager::ClusterManager() {
	mNextPing = 0;
	mShardName = "ICEEE1";
	mFullName = DEFAULT_SHARD_NAME;
	mPingSentTime = 0;
	mMasterShard = "";
	mMaster = false;
	mClusterable = false;
	mNoEvents = false;
	mHost = "127.0.0.1";
	mPort = 6379;
	mPassword = "";
}

string ClusterManager::GetMaster() {
	return mMasterShard;
}

STRINGLIST ClusterManager::GetAvailableShardNames() {
	STRINGLIST l;
	SYNCHRONIZED(mMutex)
	{
		for (map<string, Shard>::iterator it = mActiveShards.begin();
				it != mActiveShards.end(); ++it) {
			l.push_back(it->first);
		}
	}
	return l;
}

bool ClusterManager::ListAdd(const std::string &key, const std::string &value,
		bool sync) {
	mClient.rpush(key, { value });
	if (sync)
		mClient.sync_commit();
	else
		mClient.commit();
	return true;
}

bool ClusterManager::ListSet(const std::string &key,
		const std::vector<std::string> &value, bool sync) {
	mClient.del( { key });
	mClient.rpush(key, value);
	if (sync)
		mClient.sync_commit();
	else
		mClient.commit();
	return true;
}

bool ClusterManager::ListRemove(const std::string &key,
		const std::string &value, bool sync) {
	if (sync) {
		bool ok;
		mClient.lrem(key, 1, value, [this, &ok](const cpp_redis::reply &reply) {
			ok = reply.as_integer() > 0;
		});
		mClient.sync_commit();
		return ok;
	} else {
		mClient.lrem(key, 1, value);
		mClient.commit();
	}
	return true;
}

vector<string> ClusterManager::GetList(const std::string &key) {
	STRINGLIST l;
	mClient.lrange(key, 0, -1, [this, &l](const cpp_redis::reply &reply) {
		if (reply.ok()) {
			auto arr = reply.as_array();
			for (auto a = arr.begin(); a != arr.end(); ++a) {
				l.push_back((*a).as_string());
			}
		}
	});
	mClient.sync_commit();
	return l;
}

bool ClusterManager::HasKey(const string &key) {
	g_Logs.cluster->debug("Testing for key %v", key);
	auto get = mClient.exists( { key });
	mClient.sync_commit();
	auto reply = get.get();
	return reply.is_integer() && reply.as_integer();
}

string ClusterManager::GetKey(const string &key) {
	return GetKey(key, "");
}

string ClusterManager::GetKey(const string &key, const string &defaultValue) {
	g_Logs.cluster->debug("Getting key %v", key);
	auto getGet = mClient.get(key);
	mClient.sync_commit();
	cpp_redis::reply rep = getGet.get();
	if (rep.is_null())
		return defaultValue;
	else
		return rep.as_string();
}

int ClusterManager::GetIntKey(const string &key) {
	return GetIntKey(key, 0);
}

int ClusterManager::GetIntKey(const string &key, const int &defaultValue) {
	g_Logs.cluster->debug("Getting key %v", key);
	auto getGet = mClient.get(key);
	mClient.sync_commit();
	cpp_redis::reply rep = getGet.get();
	if (rep.is_null())
		return defaultValue;
	else
		return StringUtil::SafeParseInt(rep.as_string());
}

bool ClusterManager::RemoveKey(const string &key, bool sync) {
	g_Logs.cluster->debug("Removing key %v", key);
	auto getGet = mClient.del( { key });
	if (sync) {
		mClient.sync_commit();
		cpp_redis::reply rep = getGet.get();
		return rep.ok();
	} else {
		mClient.commit();
		return true;
	}
}

bool ClusterManager::SetKey(const string &key, const string &value, bool sync) {
	g_Logs.cluster->debug("Setting key %v to %v", key, value);
	auto setGet = mClient.set(key, value);
	if (sync) {
		mClient.sync_commit();
		cpp_redis::reply rep = setGet.get();
		return rep.is_integer() && rep.as_integer() == 1;
	} else
		mClient.commit();
	return false;
}

int64_t ClusterManager::NextValue(const string &key, int incr) {
	g_Logs.cluster->debug("Adjusting key %v by %v", key, incr);
	if (incr > 0) {
		auto getGet = mClient.incrby(key, incr);
		mClient.sync_commit();
		cpp_redis::reply rep = getGet.get();
		if (rep.ok())
			return rep.as_integer();
		else {
			g_Logs.cluster->error(
					"Failed to increase key %v by %v. Assuming 0.", key, incr);
			return 0;
		}
	} else if (incr < 0) {
		auto getGet = mClient.decrby(key, incr * -1);
		mClient.sync_commit();
		cpp_redis::reply rep = getGet.get();
		if (rep.ok())
			return rep.as_integer();
		else {
			g_Logs.cluster->error(
					"Failed to decrease key %v by %v. Assuming 0.", key, incr);
			return 0;
		}
	} else {
		g_Logs.cluster->warn("Request to adjust key %v by %v. Assuming 0.", key,
				incr);
		return 0;
	}
}

int ClusterManager::Scan(const ScanCallback &task, const string &pattern,
		size_t max) {
	int64_t cursor = 0;
	size_t count = 0;
	size_t scanned = 0;
	size_t pages = 0;
	if (max == 0)
		max = 10000;
	if (g_Logs.cluster->enabled(el::Level::Debug))
		g_Logs.cluster->debug("Scanning for keys matching %v", pattern);
	do {
		mClient.scan(cursor, pattern, max,
				[this, task, pattern, &cursor, &count, max, &scanned, &pages](
						const cpp_redis::reply &reply) {
					auto repl = reply.as_array();
					cursor = std::stoi(repl[0].as_string());
					pages++;
					auto data = repl[1].as_array();
					scanned += data.size();
					for (auto el = data.begin();
							(max == 0 || count < max) && el != data.end();
							++el) {
						task((*el).as_string());
						count++;
					}
				});
		mClient.sync_commit();
	} while (cursor > 0 && (max == 0 || count < max));
	if (g_Logs.cluster->enabled(el::Level::Debug))
		g_Logs.cluster->debug("Scanning for keys matching %v, matched %v", pattern, count);
	return 0;
}

void ClusterManager::JoinedShard(unsigned long simID, int zoneID,
		CharacterData *cdata) {
	SYNCHRONIZED(mMutex)
	{
		ShardPlayer sp = mActivePlayers[cdata->cdef.CreatureDefID];
		if (sp.mID == 0) {
			mActiveShards[mShardName].mPlayers++;
			g_Logs.cluster->info(
					"Notify cluster of new player (%v@%v) on Sim %v",
					cdata->cdef.css.display_name, zoneID, simID);
		} else
			g_Logs.cluster->info(
					"Notify cluster of player (%v@%v) update on Sim %v",
					cdata->cdef.css.display_name, zoneID, simID);

		sp.mID = cdata->cdef.CreatureDefID;
		sp.mShard = mShardName;
		sp.mCharacterData = cdata;
		sp.mZoneID = zoneID;
		sp.mSimID = simID;
		mActivePlayers[sp.mID] = sp;

		if (mClusterable) {
			Json::Value cfg;
			cfg["shardName"] = mShardName;
			cfg["creatureDefId"] = cdata->cdef.CreatureDefID;
			cfg["zoneId"] = zoneID;
			cfg["simId"] = Json::LargestUInt(simID);
			Send(PLAYER_JOINED_SHARD, cfg);
		}
	}
}

void ClusterManager::ShardPing(const string &shardName,
		unsigned long localTime) {
	SYNCHRONIZED(mMutex)
	{
		if (mActiveShards[shardName].mName.compare("") == 0) {
			g_Logs.cluster->warn(
					"Ping from shard %v that we did not know about. This may be because this shard was recently restarted. Requesting it's configuration",
					shardName);
			mClient.publish(SERVER_RECONFIGURE, shardName);
			mClient.commit();
		} else {
			long ms = g_PlatformTime.getMilliseconds();
			Shard s = mActiveShards[shardName];
			s.mLastSeen = ms;
			s.SetTimes(g_PlatformTime.getLocalMilliSeconds(), ms);
			mActiveShards[shardName] = s;
			mClient.publish(SERVER_PONG, mShardName);
			mClient.commit();
			g_Logs.cluster->debug("Ping from shard %v at %v", mShardName, ms);
		}
	}
}

void ClusterManager::ShardPong(const string &shardName) {
	SYNCHRONIZED(mMutex)
	{
		if (mActiveShards[shardName].mName.compare("") == 0) {
			g_Logs.cluster->warn(
					"Pong from shard %v that we did not know about.",
					shardName);
		} else {
			int tm =
					mPingSentTime == 0 ?
							0 :
							g_PlatformTime.getMilliseconds() - mPingSentTime;
			mActiveShards[shardName].mPing = tm;
			g_Logs.cluster->debug("Pong from shard %v took %v ms", shardName,
					tm);
		}
	}
}

void ClusterManager::LeftOtherShard(const string &shardName, int cdefid) {
	SYNCHRONIZED(mMutex)
	{
		for (auto it = mPending.begin(); it != mPending.end(); ++it) {
			if ((*it).mID == cdefid) {
				g_Logs.cluster->info(
						"Ignoring that %v left shard %v, as we will be their new shard.",
						cdefid, shardName);
				return;
			}
		}

		map<int, ShardPlayer>::iterator it = mActivePlayers.find(cdefid);
		if (it == mActivePlayers.end()) {
			g_Logs.cluster->info(
					"Player %v left shard %v, but this shard wasn't aware of this character!",
					cdefid, shardName);
		} else {
			mActiveShards[shardName].mPlayers--;
			ShardPlayer sp = mActivePlayers[cdefid];
			g_Logs.cluster->info(
					"Player %v (%v) left shard %v, now have %v left.", cdefid,
					sp.mCharacterData->cdef.css.display_name, shardName,
					mActiveShards[shardName].mPlayers);
			mActivePlayers.erase(it);

			/* Send status */
			CharacterData *cd = sp.mCharacterData;

			g_Scheduler.Submit([this, cd]() {
				char buf[128];
				/* TODO - Can we send to friend simulators only like it does on source server? */
				g_SimulatorManager.SendToAllSimulators(buf, PrepExt_FriendsLogStatus(buf, cd, 0), NULL);
				g_CharacterManager.UnloadCharacter(cd->cdef.CreatureDefID);
			});

		}
	}
}

void ClusterManager::FindMasterShard() {
	SYNCHRONIZED(mMutex)
	{
		Shard m;
		unsigned long earliest = g_PlatformTime.getMilliseconds() + 10000;
		for (auto it = mActiveShards.begin(); it != mActiveShards.end(); ++it) {
			if (it->second.mStartTime < earliest) {
				m = it->second;
				earliest = m.mStartTime;
			}
		}
		if (mMasterShard.compare(m.mName) != 0) {

			mMasterShard = m.mName;
			g_Logs.cluster->info("The master shard has now changed to %v",
					mMasterShard);
			mMaster = mMasterShard.compare(mShardName) == 0;

			if (mMaster) {
				/* If we are now the master, we have to take over the Auction House times
				 * and weather control
				 */
				g_Scheduler.Submit([]() {
					g_AuctionHouseManager.LoadItems();
				});
			} else {
				g_AuctionHouseManager.CancelAllTimers();
			}
		}
	}
}

void ClusterManager::JoinedOtherShard(const string &shardName, int cdefid,
		int zoneID, unsigned long simID) {
	g_CharacterManager.GetThread("SimulatorThread::SetPersona");
	CharacterData *cd = g_CharacterManager.RequestCharacter(cdefid, false);
	g_CharacterManager.ReleaseThread();
	if (cd == NULL)
		g_Logs.cluster->info(
				"Player %v joined shard %v, but this shard cannot find his character!",
				cdefid, shardName);
	else {

		/* Shard Player Entry */
		ShardPlayer sp;
		sp.mShard = shardName;
		sp.mZoneID = zoneID;
		sp.mSimID = simID;
		sp.mID = cdefid;
		sp.mCharacterData = cd;
		SYNCHRONIZED(mMutex)
		{

			if (mActivePlayers.find(cdefid) == mActivePlayers.end()) {

				mActiveShards[shardName].mPlayers++;
				g_Logs.cluster->info("Player %v (%v) joined shard %v", cdefid,
						cd->cdef.css.display_name, shardName);
			} else {
				if (mActivePlayers[cdefid].mShard.compare(shardName) == 0)
					g_Logs.cluster->debug("Player %v (%v) updated for shard %v",
							cdefid, cd->cdef.css.display_name, shardName);
				else {
					g_Logs.cluster->debug(
							"Player %v (%v) changed shards from %v to %v ",
							cdefid, cd->cdef.css.display_name,
							mActivePlayers[cdefid].mShard, shardName);

					mActiveShards[mActivePlayers[cdefid].mShard].mPlayers--;
				}
			}

			mActivePlayers[cdefid] = sp;
		}

		/* Social Stuff */
		SocialWindowEntry data;
		data.creatureDefID = cdefid;
		data.name = cd->cdef.css.display_name;
		data.level = cd->cdef.css.level;
		data.profession = static_cast<char>(cd->cdef.css.profession);
		data.online = true;
		data.status = cd->StatusText;
		data.shard = shardName;

		g_FriendListManager.UpdateSocialEntry(data);

		/* Send status */
		g_Scheduler.Submit(
				[this, sp]() {
					char buf[128];
					/* TODO - Can we send to friend simulators only like it does on source server? */
					g_SimulatorManager.SendToAllSimulators(buf,
							PrepExt_FriendsLogStatus(buf, sp.mCharacterData, 1),
							NULL);
				});
	}
}

void ClusterManager::ServerConfigurationReceived(const string &shardName,
		const string &simulatorAddress, int simulatorPort,
		const string &fullName, int players, unsigned long startTime,
		unsigned long utcTime, unsigned long localTime,
		const std::string &httpAddress) {
	SYNCHRONIZED(mMutex)
	{
		Shard s = mActiveShards[shardName];
		if (s.mName.compare("") == 0) {
			NewShard(shardName);
			s = mActiveShards[shardName];
		}
		long ms = g_PlatformTime.getMilliseconds();
		s.mLastSeen = ms;
		s.mFullName = fullName;
		s.mPlayers = players;
		s.mSimulatorAddress = simulatorAddress;
		s.mHTTPAddress = httpAddress;
		s.mSimulatorPort = simulatorPort;
		s.mStartTime = startTime;
		s.SetTimes(localTime, utcTime);
		int tm = mPingSentTime == 0 ? 0 : ms - mPingSentTime;
		s.mPing = tm;
		g_Logs.cluster->info(
				"Simulator for shard %v is %v:%v (S2S ping %v). HTTP server for this shard is %v",
				shardName, s.mSimulatorAddress, s.mSimulatorPort, tm,
				s.mHTTPAddress);
		mActiveShards[s.mName] = s;
		FindMasterShard();
	}
}

string ClusterManager::SimTransfer(int cdefId, const string &shardName,
		int simID) {

	/* Called from the sim that the player is currently on. We expect a CONFIRM_TRANSFER
	 * message to come back when the receiving sim is ready.
	 */
	string token = Util::RandomStr(32, false);
	g_Logs.cluster->info(
			"Notifying shard %v that %v (sim %v) is about to connect to them using token %v.",
			shardName, cdefId, simID, token);

	Json::Value cfg;
	cfg["shardName"] = mShardName;
	cfg["target"] = shardName;
	cfg["creatureDefId"] = cdefId;
	cfg["token"] = token;
	cfg["simId"] = Json::LargestUInt(simID);
	Send(SIM_TRANSFER, cfg);
	return token;
}

void ClusterManager::ConfirmTransfer(int cdefId, const string &shardName,
		const string &token, int simID) {
	g_Logs.cluster->info(
			"Notifying shard %v that %v will be accepted for transfer to this shard using token %v [%v]",
			shardName, cdefId, token, simID);
	Json::Value cfg;
	cfg["shardName"] = mShardName;
	cfg["target"] = shardName;
	cfg["creatureDefId"] = cdefId;
	cfg["token"] = token;
	cfg["simId"] = Json::LargestUInt(simID);
	Send(CONFIRM_TRANSFER, cfg);
}

void ClusterManager::Send(const std::string &msg, const Json::Value &val) {
	Json::StyledWriter writer;
	std::string str = writer.write(val);
	if (g_Logs.cluster->enabled(el::Level::Debug))
		g_Logs.cluster->debug("Send cluster message: %v [%v]", msg, str);
	mClient.publish(msg, str);
	mClient.commit();
}

void ClusterManager::SendConfiguration() {
	g_Logs.cluster->info(
			"Received a request to send our shard configuration (Simulator %v:%v, HTTP %v).",
			g_SimulatorAddress, g_SimulatorPort, g_Config.ResolveHTTPAddress());

	// TODO make all other messages JSON too

	Json::Value cfg;
	cfg["shardName"] = mShardName;
	cfg["simulatorAddress"] = g_SimulatorAddress;
	cfg["simulatorPort"] = g_SimulatorPort;
	cfg["fullName"] = mFullName;
	cfg["players"] = GetActiveShard(mShardName).mPlayers;
	cfg["launchTime"] = Json::LargestUInt(g_ServerLaunchTime);
	cfg["utc"] = Json::LargestUInt(g_PlatformTime.getUTCMilliSeconds());
	cfg["time"] = Json::LargestUInt(g_PlatformTime.getLocalMilliSeconds());
	cfg["http"] = g_Config.ResolveHTTPAddress();
	Send(SERVER_CONFIGURATION, cfg);
	SYNCHRONIZED(mMutex)
	{
		/* Send all of the players we know about as well */
		for (map<int, ShardPlayer>::iterator it = mActivePlayers.begin();
				it != mActivePlayers.end(); ++it) {
			ShardPlayer p = it->second;
			if (p.IsLocal()) {
				/* Only send players that belong to our cluster */
				Json::Value cfg;
				cfg["shardName"] = mShardName;
				cfg["creatureDefId"] = p.mID;
				cfg["zoneId"] = p.mZoneID;
				cfg["simId"] = Json::LargestUInt(p.mSimID);
				Send(PLAYER_JOINED_SHARD, cfg);
			}
		}

		/* And all of the local account logins (possibly multiple times for each account) */
		// TODO this wont work. we might end up sending to shards that already are aware of account
//		for(map<int, int>::iterator it = mActiveLocalAccounts.begin(); it != mActiveLocalAccounts.end(); ++it) {
//			int count = it->second;
//			for(int j = 0 ; j < count ; j++) {
//				/* Only send players that belong to our cluster */
//				mClient.publish(LOGIN,
//						StringUtil::Format("%s:%d", mShardName.c_str(),
//								it->first));
//				mClient.commit();
//			}
//		}
	}
}

void ClusterManager::LeftShard(int CDefID) {
	SYNCHRONIZED(mMutex)
	{
		map<int, ShardPlayer>::iterator it = mActivePlayers.find(CDefID);
		if (it != mActivePlayers.end()) {
			mActiveShards[mShardName].mPlayers--;
			g_Logs.cluster->info(
					"Local player %v left shard, now have %v players left.",
					CDefID, mActiveShards[mShardName].mPlayers);
			mActivePlayers.erase(it);
			if (mClusterable && mClient.is_connected()) {
				Json::Value cfg;
				cfg["shardName"] = mShardName;
				cfg["creatureDefId"] = CDefID;
				Send(PLAYER_LEFT_SHARD, cfg);
			}
		} else
			/* Happens on sim switch */
			g_Logs.cluster->debug(
					"Local player %v left shard, but we didn't know about them!",
					CDefID);
	}
}

Shard ClusterManager::GetActiveShard(const string &shardName) {
	return mActiveShards[shardName];
}

ShardPlayer ClusterManager::GetActivePlayer(int CDefId) {
	return mActivePlayers[CDefId];
}

void ClusterManager::Login(int accountID) {
	if (mClient.is_connected()) {
		mClient.hset(
				StringUtil::Format("%s:%d", KEYPREFIX_ACCOUNT_SESSIONS.c_str(),
						accountID), StringUtil::Format("%lu", g_ServerTime),
				mShardName);
		if (mClusterable) {
			Json::Value cfg;
			cfg["shardName"] = mShardName;
			cfg["accountID"] = accountID;
			Send(LOGIN, cfg);
		}
	}
}

void ClusterManager::Logout(int accountID) {
	if (mClient.is_connected()) {
		string key = StringUtil::Format("%s:%d",
				KEYPREFIX_ACCOUNT_SESSIONS.c_str(), accountID);
		auto asget = mClient.hgetall(key);
		mClient.sync_commit();
		cpp_redis::reply rep = asget.get();
		auto arr = rep.as_array();
		for (auto a = arr.begin(); a != arr.end(); ++a) {
			string n = (*a).as_string();
			string v = (*++a).as_string();
			if (v.compare(mShardName) == 0) {
				mClient.hdel(key, { n });
				/* NOTE: Currently, it MIGHT not be the exact session, but this doesnt matter too much
				 * as long as only one is removed
				 */
				mClient.commit();
				break;
			}
		}
		if (mClusterable) {
			Json::Value cfg;
			cfg["shardName"] = mShardName;
			cfg["accountID"] = accountID;
			Send(LOGOUT, cfg);
		}
	}
}

int ClusterManager::CountAccountSessions(int accountID, bool includeLocal,
		bool includeRemote) {
	int count = 0;
	SYNCHRONIZED(mMutex)
	{
		auto asget = mClient.hgetall(
				StringUtil::Format("%s:%d", KEYPREFIX_ACCOUNT_SESSIONS.c_str(),
						accountID));
		mClient.sync_commit();
		cpp_redis::reply rep = asget.get();
		if (rep.is_array()) {
			auto arr = rep.as_array();
			for (auto a = arr.begin(); a != arr.end(); ++a) {
				string n = (*a).as_string();
				string v = (*++a).as_string();
				if ((v.compare(mShardName) == 0 && includeLocal)
						|| (v.compare(mShardName) != 0 && includeRemote))
					count++;
			}
		}
	}
	return count;
}

bool ClusterManager::IsPlayerOnOtherShard(const string &characterName) {
	SYNCHRONIZED(mMutex)
	{
		for (auto it2 = mActivePlayers.begin(); it2 != mActivePlayers.end();
				++it2) {
			if (it2->second.mCharacterData != NULL
					&& it2->second.mShard.compare(mShardName) != 0
					&& strcmp(it2->second.mCharacterData->cdef.css.display_name,
							characterName.c_str()) == 0) {
				return true;
			}
		}
	}
	return false;
}

void ClusterManager::ShardRemoved(const string &shardName) {
	SYNCHRONIZED(mMutex)
	{
		map<string, Shard>::iterator it = mActiveShards.find(shardName);
		if (it != mActiveShards.end()) {
			vector<int> l;

			/* Remove all players on this shard */
			for (auto it2 = mActivePlayers.begin(); it2 != mActivePlayers.end();
					) {
				if (it2->second.mShard.compare(shardName) == 0) {
					l.push_back((it2++)->second.mID);
				} else
					++it2;
			}

			for (auto it2 = l.begin(); it2 != l.end(); ++it2)
				LeftOtherShard(shardName, *it2);

			g_Scheduler.Submit(
					[this, shardName]() {
						char buf[128];
						g_SimulatorManager.SendToAllSimulators(buf,
								PrepExt_SendInfoMessage(buf,
										StringUtil::Format(
												"Shard %s is now offline.",
												shardName.c_str()).c_str(),
										INFOMSG_INFO), NULL);
					});

			mActiveShards.erase(it);

			g_Logs.cluster->info(
					"A shard was shutdown [%v]. %v players were on this shard at the time. There are now %v in the cluster.",
					shardName, l.size(), mActiveShards.size());

			FindMasterShard();
		} else
			g_Logs.cluster->info(
					"A shard we didn't know about has shutdown [%v]",
					shardName);
	}
}

void ClusterManager::NewShard(const string &shardName) {
	SYNCHRONIZED(mMutex)
	{
		if (mActiveShards.find(shardName) != mActiveShards.end()) {
			g_Logs.cluster->warn(
					"Got a new shard notification [%v] for a shard we already knew about. This suggests it crashed uncleanly, but came back up before the ping interval expired.",
					shardName);
			ShardRemoved(shardName);
		}
		mActiveShards[shardName] = Shard();
		mActiveShards[shardName].mName = shardName;
		mActiveShards[shardName].mLastSeen = g_PlatformTime.getMilliseconds();
		g_Logs.cluster->info(
				"New shard in cluster [%v]. There are now %v in the cluster.",
				shardName, mActiveShards.size());

		g_Scheduler.Submit(
				[this, shardName]() {
					char buf[128];
					g_SimulatorManager.SendToAllSimulators(buf,
							PrepExt_SendInfoMessage(buf,
									StringUtil::Format(
											"Shard %s is now online.",
											shardName.c_str()).c_str(),
									INFOMSG_INFO), NULL);
				});
	}
}

PendingShardPlayer ClusterManager::FindToken(const string &token) {
	SYNCHRONIZED(mMutex)
	{
		for (auto it = mPending.begin(); it != mPending.end(); ++it) {
			if ((*it).mToken.compare(token) == 0) {
				PendingShardPlayer p = *it;

				map<int, ShardPlayer>::iterator ait = mActivePlayers.find(
						p.mID);
				if (ait != mActivePlayers.end()) {

					mActiveShards[p.mShardName].mPlayers--;
					ShardPlayer sp = mActivePlayers[p.mID];
					g_Logs.cluster->info(
							"Player %v (%v) left shard %v for us, removing their current shard information.",
							p.mID, sp.mCharacterData->cdef.css.display_name,
							p.mShardName);
					mActivePlayers.erase(ait);
				}

				mPending.erase(it);

				return p;
			}
		}
	}
	return PendingShardPlayer();
}

void ClusterManager::ConfirmTransferToOtherShard(int cdefId,
		const std::string &shardName, string token, int simID) {
	SimulatorThread *sim = g_SimulatorManager.GetPtrByID(simID);
	if (sim == NULL)
		g_Logs.cluster->error(
				"Got sim transfer confirmation for simulator we know nothing about, ID: %v (for CDefID %v and token %v).",
				simID, cdefId, token);
	else {
		Shard s = GetActiveShard(shardName);
		sim->FinaliseTransfer(s, token);
	}
}

void ClusterManager::TransferFromOtherShard(int cdefId,
		const std::string &shardName, string token, int simID) {
	g_Logs.cluster->info("Expecting sim transfer for %v (using %v).", cdefId,
			token);

	/* Reload any data for this character now */
	g_CharacterManager.ReloadCharacter(cdefId, false);
	g_AccountManager.ReloadAccountID(
			g_CharacterManager.GetPointerByID(cdefId)->AccountID);

	PendingShardPlayer p;
	p.mID = cdefId;
	p.mToken = token;
	p.mShardName = shardName;
	p.mReceived = g_PlatformTime.getMilliseconds();
	SYNCHRONIZED(mMutex)
	{
		mPending.push_back(p);
	}
	ConfirmTransfer(cdefId, shardName, token, simID);
}

bool ClusterManager::IsMaster() {
	return mMaster;
}

void ClusterManager::Weather(int zoneId, const string &mapName,
		const string &weatherType, int weight) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on weather %v (%v).", weatherType,
				weight);
		Json::Value w;
		w["shardName"] = mShardName;
		w["zoneId"] = zoneId;
		w["mapName"] = mapName;
		w["type"] = weatherType;
		w["weight"] = weight;
		Send(WEATHER, w);
	}
}

void ClusterManager::Auction(int auctionItemId, const string &sellerName) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on new auction item %v", auctionItemId);
		Json::Value w;
		w["shardName"] = mShardName;
		w["auctionItemId"] = auctionItemId;
		w["sellerName"] = sellerName;
		Send(AUCTION_ITEM, w);
	}
}

void ClusterManager::AuctionItemUpdated(int auctionItemId) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on updated auction item %v",
				auctionItemId);
		Json::Value w;
		w["shardName"] = mShardName;
		w["auctionItemId"] = auctionItemId;
		Send(AUCTION_ITEM_UPDATED, w);
	}
}

void ClusterManager::PropUpdated(int propId) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on updated prop %v", propId);
		Json::Value w;
		w["shardName"] = mShardName;
		w["propId"] = propId;
		Send(PROP_UPDATED, w);
	}
}

void ClusterManager::AuctionItemRemoved(int auctionItemId,
		int auctioneerCDefID) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on removed auction item %v",
				auctionItemId);
		Json::Value w;
		w["shardName"] = mShardName;
		w["auctionItemId"] = auctionItemId;
		w["auctioneerCDefID"] = auctioneerCDefID;
		Send(AUCTION_ITEM_REMOVED, w);
	}
}

void ClusterManager::Thunder(int zoneId, const string &mapName) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on thunder");
		Json::Value w;
		w["shardName"] = mShardName;
		w["zoneId"] = zoneId;
		w["mapName"] = mapName;
		Send(THUNDER, w);
	}
}

void ClusterManager::Chat(ChatMessage &message) {
	if (mClusterable) {
		g_Logs.cluster->info("Sending on chat message from %v to %v.",
				message.mSender, message.mChannel);
		Json::Value val;
		message.WriteToJSON(val);
		Send(CHAT, val);
	}
}

bool ClusterManager::WriteEntity(AbstractEntity *entity, bool sync) {
	if (!mClient.is_connected())
		return false;
	ClusterEntityWriter cew(&mClient, sync);
	if (cew.Start()) {
		if (entity->WriteEntity(&cew)) {
			return cew.End();
		}
	}
	return false;
}

bool ClusterManager::RemoveEntity(AbstractEntity *entity) {
	ClusterEntityReader cer(&mClient);
	if (cer.Start()) {
		if (entity->EntityKeys(&cer)) {
			g_Logs.data->info("Removing entity %v from %v", cer.mID,
					cer.mCatalog);
			STRINGLIST sections = cer.Sections();
			for (auto a = sections.begin(); a != sections.end(); ++a) {
				string k = StringUtil::Format("%s:%s:%s", cer.mCatalog.c_str(),
						cer.mID.c_str(), (*a).c_str());
				RemoveKey(k);
			}
			RemoveKey(
					StringUtil::Format("%s:%s^_SUBKEYS_", cer.mCatalog.c_str(),
							cer.mID.c_str()));
			return cer.End();
		} else
			cer.Abort();
	}
	return false;
}

bool ClusterManager::ReadEntity(AbstractEntity *entity) {
	ClusterEntityReader cer(&mClient);
	if (cer.Start()) {
		if (entity->EntityKeys(&cer) && entity->ReadEntity(&cer)) {
			return cer.End();
		} else
			cer.Abort();
	}
	return false;
}

int ClusterManager::LoadConfiguration(const string &filename) {
	FileReader lfr;
	if (lfr.OpenText(filename.c_str()) != Err_OK) {
		g_Logs.cluster->error(
				"Could not open cluster config file [%v] for reading.",
				filename);
		return -1;
	}
	lfr.CommentStyle = Comment_Semi;

	while (lfr.FileOpen() == true) {
		int r = lfr.ReadLine();
		if (r > 0) {
			lfr.SingleBreak("=");
			lfr.BlockToString(0);
			char *NameBlock = lfr.BlockToString(0);
			if (strcmp(NameBlock, "ShardName") == 0) {
				mShardName = lfr.BlockToString(1);
			} else if (strcmp(NameBlock, "FullName") == 0) {
				mFullName = lfr.BlockToString(1);
			} else if (strcmp(NameBlock, "Host") == 0) {
				mHost = lfr.BlockToString(1);
			} else if (strcmp(NameBlock, "Port") == 0) {
				mPort = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "Password") == 0) {
				mPassword = lfr.BlockToString(1);
			} else
				g_Logs.cluster->error("Unknown identifier [%v] in file [%v]",
						NameBlock, filename);
		}
	}
	lfr.CloseCurrent();
	g_Logs.cluster->info("Loaded cluster configuration file %v.", filename);
	return 0;
}

bool ClusterManager::Init() {
	if (mHost.length() == 0) {
		g_Logs.cluster->error(
				"No Redis host specified in any cluster configuration file. Check all Cluster.txt configuration files.");
		return false;
	}

	//! High availablity requires at least 2 io service workers
	cpp_redis::network::set_default_nb_workers(10);

	long ms = g_PlatformTime.getMilliseconds();
	mNextPing = ms + CLUSTER_PING_INTERVAL;
	cpp_redis::active_logger = unique_ptr<ClusterLogger>(new ClusterLogger());

	/* We can only support sharding if simulator addresses are actually specified */

	mClusterable = strlen(g_SimulatorAddress) != 0;
	if (!mClusterable) {
		g_Logs.cluster->warn(
				"Shards not supported, as SimulatorAddress is not specified in ServerConfig.txt");
	}

	/* Our own shard is always active */
	Shard shard;
	shard.mName = mShardName;
	shard.mSimulatorAddress = g_SimulatorAddress;
	shard.mSimulatorPort = g_SimulatorPort;
	shard.mHTTPAddress = g_Config.ResolveHTTPAddress();
	shard.mFullName = mFullName;
	shard.mStartTime = ms;
	shard.SetTimes(g_PlatformTime.getLocalMilliSeconds(), ms);
	mActiveShards[mShardName] = shard;
	FindMasterShard();

	bool ok = false;
	mClient.connect(mHost, mPort,
			[this, &ok](const string &host, size_t port,
					cpp_redis::connect_state status) {
				if (status == cpp_redis::connect_state::dropped) {
					g_Logs.cluster->error(
							"Redis client disconnected from %v:%v", host, port);
				} else {
					g_Logs.cluster->info("Redis state change %v:%v (%v)", host,
							port, RedisConnectStatus(status));
					ok = true;
				}
			}, 30000, 99999, 30000);

	if (!ok)
		return false;

	if (mPassword.length() > 0) {
		mClient.auth(mPassword,
				[this, &ok](const cpp_redis::reply &reply) {
					if (reply.is_error()) {
						g_Logs.cluster->error(
								"Redis client failed to authenticate from %v:%v",
								mHost, mPort);
					} else {
						g_Logs.cluster->info("Redis authenticated %v:%v", mHost,
								mPort);
					}
				});
		mClient.sync_commit();
		if (ok)
			return PostInit();
		return true;
	} else
		return PostInit();

	return false;
}

bool ClusterManager::PostInit() {

	/*
	 * Subscribe to channel to listen for other shards coming online
	 */
	if (mClusterable && !mNoEvents) {
		bool ok = false;
		mSub.connect(mHost, mPort,
				[&ok](const string &host, size_t port,
						cpp_redis::connect_state status) {
					if (status == cpp_redis::connect_state::dropped)
						g_Logs.cluster->error(
								"Redis event channel disconnected from %v:%v",
								host, port);
					else {
						g_Logs.cluster->info(
								"Redis event channel state change %v:%v (%v)",
								host, port, RedisConnectStatus(status));
						ok = true;
					}
				}, 30000, 99999, 30000);
		if (!ok)
			return false;
		if (mPassword.length() > 0) {
			mSub.auth(mPassword,
					[this, &ok](const cpp_redis::reply &reply) {
						if (reply.is_error()) {
							g_Logs.cluster->error(
									"Redis event channel failed to authenticate from %v:%v",
									mHost, mPort);
						} else {
							g_Logs.cluster->info(
									"Redis event channel authenticated %v:%v",
									mHost, mPort);
							ok = true;
						}
					});
			mSub.commit();
		}
	}

	/*
	 * We are just starting up, there shouldn't be any AccountSessions in the database
	 * for this shard
	 */
	STRINGLIST keys;
	Scan([this, &keys](const string &key) {
		keys.push_back(key);
	}, StringUtil::Format("%s:*", KEYPREFIX_ACCOUNT_SESSIONS.c_str()));
	if (keys.size() > 0) {
		g_Logs.cluster->info("Found %v sessions", keys.size());
		int removed = 0;
		for (auto it = keys.begin(); it != keys.end(); ++it) {
			string key = *it;
			auto asget = mClient.hgetall(key);
			mClient.sync_commit();
			cpp_redis::reply rep = asget.get();
			if (rep.is_array()) {
				auto arr = rep.as_array();
				for (auto a = arr.begin(); a != arr.end(); ++a) {
					string n = (*a).as_string();
					string v = (*++a).as_string();
					if (v.compare(mShardName) == 0) {
						g_Logs.cluster->info(
								"Removing stale account session for %v (%v)", n,
								v);
						mClient.hdel(key, { n });
						mClient.commit();
						removed++;
					}
				}
			}
		}
		g_Logs.cluster->info("Removed %v stale sessions", removed);
	} else {
		g_Logs.cluster->info("No stale sessions found");
	}

	return true;
}

void ClusterManager::RunProcessingCycle() {
	/* Because this runs in the server processing thread we use g_ServerTime instead of g_PlatformTime.getMilliseconds() */
	if (g_ServerTime > mNextPing) {

		g_Logs.cluster->debug("Sending shard ping for %v at %v", mShardName,
				g_ServerTime);

		Json::Value ping;
		ping["shardName"] = mShardName;
		ping["time"] = Json::LargestUInt(g_ServerTime);
		Send(SERVER_PING, ping);

		mPingSentTime = g_ServerTime;
		mNextPing = g_ServerTime + CLUSTER_PING_INTERVAL;
		unsigned long expire = g_ServerTime - CLUSTER_SHARD_TIMEOUT;
		vector<string> r;

		SYNCHRONIZED(mMutex)
		{
			for (map<string, Shard>::iterator it = mActiveShards.begin();
					it != mActiveShards.end(); ++it) {
				if (it->first.compare(mShardName) != 0) {
					g_Logs.cluster->debug(
							"Expiry check. Comparing %v against %v for %v",
							it->second.mLastSeen, expire, it->first);
					if (it->second.mLastSeen != 0
							&& it->second.mLastSeen < expire) {
						g_Logs.cluster->warn(
								"Haven't had ping from shard %v for a while (last seen %v, expire %v). Will remove.",
								it->first,
								StringUtil::FormatTimeHHMM(
										it->second.mLastSeen),
								StringUtil::FormatTimeHHMM(expire));
						r.push_back(it->first);
					}
				} else {
					it->second.SetTimes(g_PlatformTime.getLocalMilliSeconds(),
							g_ServerTime);
				}
			}
		}

		for (vector<string>::iterator it = r.begin(); it != r.end(); ++it) {
			ShardRemoved(*it);
		}
	}
}

void ClusterManager::Ready() {
	if (mClusterable) {

		g_Logs.cluster->info("Start listening for cluster events");

		mSub.subscribe(SERVER_STARTED,
				[this](const string &chan, const string &msg) {
					if (msg.compare(mShardName) != 0) {
						g_Scheduler.Submit([this, msg]() {
							NewShard(msg);
							mClient.publish(SERVER_RECONFIGURE, msg);
							mClient.commit();
						});
					}
				});
		mSub.subscribe(SERVER_RECONFIGURE,
				[this](const string &chan, const string &msg) {
					if (msg.compare(mShardName) == 0) {
						g_Scheduler.Submit([this]() {
							SendConfiguration();
						});
					}
				});
		mSub.subscribe(SERVER_CONFIGURATION,
				[this](const string &chan, const string &msg) {

					if (g_Logs.cluster->enabled(el::Level::Debug))
						g_Logs.cluster->debug("Received configuration %v : %v",
								chan, msg);

					Json::Value cfg;
					Json::Reader reader;
					if (reader.parse(msg, cfg)) {
						if (cfg["shardName"].asString() != mShardName) {
							g_Scheduler.Submit(
									[this, cfg]() {
										ServerConfigurationReceived(
												cfg["shardName"].asString(),
												cfg["simulatorAddress"].asString(),
												cfg["simulatorPort"].asInt(),
												cfg["fullName"].asString(),
												cfg["players"].asInt(),
												cfg["launchTime"].asLargestUInt(),
												cfg["utc"].asLargestUInt(),
												cfg["time"].asLargestUInt(),
												cfg["http"].asString());
									});
						}
					} else {
						g_Logs.cluster->error(
								"Malformed server configuration event.");
					}

				});
		mSub.subscribe(SERVER_STOPPED,
				[this](const string &chan, const string &msg) {
					if (msg.compare(mShardName) != 0) {
						g_Scheduler.Submit([this, msg]() {
							ShardRemoved(msg);
						});
					}
				});
		mSub.subscribe(SERVER_PING,
				[this](const string &chan, const string &msg) {
					Json::Value cfg;
					Json::Reader reader;
					if (reader.parse(msg, cfg)) {
						if (cfg["shardName"].asString() != mShardName) {
							g_Scheduler.Submit(
									[this, cfg]() {
										ShardPing(cfg["shardName"].asString(),
												cfg["time"].asLargestUInt());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed server ping.");
					}
				});
		mSub.subscribe(CHAT, [this](const string &chan, const string &msg) {
			Json::Value cfg;
			Json::Reader reader;
			if (reader.parse(msg, cfg)) {
				if (cfg["shardName"].asString() != mShardName) {
					ChatMessage msg;
					msg.ReadFromJSON(cfg);
					g_Scheduler.Submit([this, &msg]() {
						g_ChatManager.DeliverChatMessage(msg, NULL);
					});
				}
			} else {
				g_Logs.cluster->error("Malformed server configuration event.");
			}

		});
		mSub.subscribe(WEATHER,
				[this](const string &chan, const string &msg) {
					Json::Value weather;
					Json::Reader reader;
					if (reader.parse(msg, weather)) {
						if (weather["shardName"].asString() != mShardName) {
							g_Scheduler.Submit(
									[this, weather]() {
										g_WeatherManager.ZoneWeather(
												weather["zoneId"].asInt(),
												weather["mapName"].asString(),
												weather["type"].asString(),
												weather["weight"].asInt());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed weather event.");
					}
				});
		mSub.subscribe(THUNDER,
				[this](const string &chan, const string &msg) {
					Json::Value thunder;
					Json::Reader reader;
					if (reader.parse(msg, thunder)) {
						if (thunder["shardName"].asString() != mShardName) {
							g_Scheduler.Submit(
									[this, thunder]() {
										g_WeatherManager.ZoneThunder(
												thunder["zoneId"].asInt(),
												thunder["mapName"].asString());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed thunder event.");
					}

				});
		mSub.subscribe(AUCTION_ITEM,
				[this](const string &chan, const string &msg) {

					Json::Value auctionItem;
					Json::Reader reader;
					if (reader.parse(msg, auctionItem)) {
						if (auctionItem["shardName"].asString() != mShardName) {
							g_Scheduler.Pool(
									[this, auctionItem]() {
										AuctionHouseItem item =
												g_AuctionHouseManager.LoadItem(
														auctionItem["auctionItemId"].asInt());
										g_Scheduler.Submit(
												[this, auctionItem, &item]() {
													g_AuctionHouseManager.BroadcastAndSetupTimer(
															&item,
															auctionItem["sellerName"].asString());
												});
									});
						}
					} else {
						g_Logs.cluster->error("Malformed auction item event.");
					}

				});
		mSub.subscribe(AUCTION_ITEM_REMOVED,
				[this](const string &chan, const string &msg) {
					Json::Value auctionItem;
					Json::Reader reader;
					if (reader.parse(msg, auctionItem)) {
						if (auctionItem["shardName"].asString() != mShardName) {
							g_Scheduler.Submit(
									[this, auctionItem]() {
										CreatureInstance *instance =
												g_ActiveInstanceManager.GetNPCCreatureByDefID(
														auctionItem["auctioneerCDefID"].asInt());
										if (instance != NULL) {
											g_AuctionHouseManager.BroadcastRemovedItem(
													instance->CreatureID,
													auctionItem["auctionItemId"].asInt());
										} else {
											g_Logs.server->debug(
													"No auctioneer instance %v to broadcast remove of item %v to, ignoring.",
													auctionItem["auctioneerCDefID"].asInt(),
													auctionItem["auctionItemId"].asInt());
										}
									});
						}
					} else {
						g_Logs.cluster->error(
								"Malformed auction item remote event.");
					}

				});
		mSub.subscribe(AUCTION_ITEM_UPDATED,
				[this](const string &chan, const string &msg) {
					Json::Value auctionItem;
					Json::Reader reader;
					if (reader.parse(msg, auctionItem)) {
						if (auctionItem["shardName"].asString() != mShardName) {
							g_Scheduler.Pool(
									[this, auctionItem]() {
										AuctionHouseItem item =
												g_AuctionHouseManager.LoadItem(
														auctionItem["auctionItemId"].asInt());
										if (item.mId == 0) {
											g_Logs.server->debug(
													"Update for auction item %v failed, the item doesn't exist.",
													auctionItem["auctionItemId"].asInt());
										} else {
											g_Scheduler.Submit(
													[this, auctionItem, item]() {
														CreatureInstance *instance =
																g_ActiveInstanceManager.GetNPCCreatureByDefID(
																		item.mAuctioneer);
														if (instance != NULL) {
															g_AuctionHouseManager.BroadcastRemovedItem(
																	instance->CreatureID,
																	auctionItem["auctionItemId"].asInt());
														} else {
															g_Logs.server->debug(
																	"No auctioneer instance of %v to broadcast update of item %v to, ignoring.",
																	item.mAuctioneer,
																	auctionItem["auctionItemId"].asInt());
														}
													});
										}
									});
						}
					} else {
						g_Logs.cluster->error(
								"Malformed auction item update event.");
					}
				});

		mSub.subscribe(PROP_UPDATED,
				[this](const string &chan, const string &msg) {
					Json::Value auctionItem;
					Json::Reader reader;
					if (reader.parse(msg, auctionItem)) {
						if (auctionItem["shardName"].asString() != mShardName) {
							// TODO send prop updates
						}
					} else {
						g_Logs.cluster->error(
								"Malformed auction prop update event.");
					}
				});

		mSub.subscribe(SERVER_PONG,
				[this](const string &chan, const string &msg) {
					if (msg.compare(mShardName) != 0) {
						g_Scheduler.Pool([this, msg]() {
							ShardPong(msg);
						});
					}
				});
		mSub.subscribe(PLAYER_JOINED_SHARD,
				[this](const string &chan, const string &msg) {
					Json::Value player;
					Json::Reader reader;
					if (reader.parse(msg, player)) {
						if (player["shardName"].asString() != mShardName) {
							g_Scheduler.Pool(
									[this, player]() {
										JoinedOtherShard(
												player["shardName"].asString(),
												player["creatureDefId"].asInt(),
												player["zoneId"].asInt(),
												player["simId"].asLargestUInt());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed player join event.");
					}
				});
		mSub.subscribe(PLAYER_LEFT_SHARD,
				[this](const string &chan, const string &msg) {
					Json::Value player;
					Json::Reader reader;
					if (reader.parse(msg, player)) {
						if (player["shardName"].asString() != mShardName) {
							g_Scheduler.Pool(
									[this, player]() {
										LeftOtherShard(
												player["shardName"].asString(),
												player["creatureDefId"].asInt());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed player left event.");
					}
				});

		mSub.subscribe(LOGIN,
				[this](const string &chan, const string &msg) {
					Json::Value player;
					Json::Reader reader;
					if (reader.parse(msg, player)) {
						if (player["shardName"].asString() != mShardName) {
							g_Scheduler.Pool(
									[this, player]() {
										g_Logs.cluster->info(
												"Account %v logged in to shard %v",
												player["accountID"].asInt(),
												player["shardName"].asString());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed player left event.");
					}
				});
		mSub.subscribe(LOGOUT,
				[this](const string &chan, const string &msg) {
					Json::Value player;
					Json::Reader reader;
					if (reader.parse(msg, player)) {
						if (player["shardName"].asString() != mShardName) {
							g_Scheduler.Pool(
									[this, player]() {
										g_Logs.cluster->info(
												"Account %v logged out of shard %v",
												player["accountID"].asInt(),
												player["shardName"].asString());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed player left event.");
					}
				});

		mSub.subscribe(SIM_TRANSFER,
				[this](const string &chan, const string &msg) {

					Json::Value sim;
					Json::Reader reader;
					if (reader.parse(msg, sim)) {
						if (sim["shardName"].asString() != mShardName
								&& sim["target"].asString() == mShardName) {
							g_Scheduler.Submit(
									[this, sim]() {
										TransferFromOtherShard(
												sim["creatureDefId"].asInt(),
												sim["shardName"].asString(),
												sim["token"].asString(),
												sim["simId"].asLargestUInt());
									});
						}
					} else {
						g_Logs.cluster->error("Malformed sim transfer event.");
					}
				});

		mSub.subscribe(CONFIRM_TRANSFER,
				[this](const string &chan, const string &msg) {

					Json::Value sim;
					Json::Reader reader;
					if (reader.parse(msg, sim)) {
						if (sim["shardName"].asString() != mShardName
								&& sim["target"].asString() == mShardName) {
							g_Scheduler.Submit(
									[this, sim]() {
										ConfirmTransferToOtherShard(
												sim["creatureDefId"].asInt(),
												sim["shardName"].asString(),
												sim["token"].asString(),
												sim["simId"].asLargestUInt());
									});
						}
					} else {
						g_Logs.cluster->error(
								"Malformed confirm sim transfer event.");
					}
				});

		mSub.commit();

		g_Logs.cluster->info("Informing cluster of readyness");
		mClient.publish(SERVER_STARTED, mShardName);
		mClient.sync_commit();
		g_Logs.cluster->info("Informed cluster of readyness");
	}
}

void ClusterManager::Shutdown(bool wait) {
	if (mClusterable) {
		g_Logs.cluster->info("Informing cluster of shutdown");
		mClient.publish(SERVER_STOPPED, mShardName);
		try {
			mClient.sync_commit(chrono::milliseconds(2000));
			g_Logs.cluster->info("Informed cluster of shutdown");
		} catch (cpp_redis::redis_error &e) {
			g_Logs.cluster->info("Failed to inform client of shutdown. %v",
					e.what());
		}
		g_Logs.cluster->info("Disconnecting from Database");
		mClient.disconnect(wait);
		mSub.disconnect(wait);
	} else {
		g_Logs.cluster->info("Disconnecting from Database");
		mClient.disconnect(wait);
	}
}
