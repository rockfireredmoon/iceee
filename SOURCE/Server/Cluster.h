#ifndef CLUSTER_H
#define CLUSTER_H

#include <string>
#include <map>
#include <vector>
#include <cpp_redis/cpp_redis>
#include "CommonTypes.h"
#include "Character.h"
#include "Chat.h"
#include "Entities.h"
#include <functional>

typedef std::function<void(const std::string &v)> ScanCallback;

typedef std::pair<std::string, std::string> KEY_VAL_PAIR;
typedef std::vector<std::pair<std::string, std::string>> KEY_VAL_LIST;

struct ClusterEntityStringValue {
	std::string section;
	std::string key;
	std::string value;
};

struct ClusterEntityRemoveValue {
	std::string section;
	std::string key;
};

class ClusterLogger : public cpp_redis::logger_iface {

public:
  ClusterLogger();
  ~ClusterLogger(void) = default;
  ClusterLogger(const ClusterLogger&) = default;
  ClusterLogger& operator=(const ClusterLogger&) = default;

public:
  void debug(const std::string& msg, const std::string& file, std::size_t line);
  void info(const std::string& msg, const std::string& file, std::size_t line);
  void warn(const std::string& msg, const std::string& file, std::size_t line);
  void error(const std::string& msg, const std::string& file, std::size_t line);

};

class ClusterEntityReader : public AbstractEntityReader {
public:
	ClusterEntityReader(cpp_redis::client *client);
	virtual ~ClusterEntityReader();

	virtual std::string Value(const std::string &key, std::string defaultValue = "");
	virtual std::vector<std::string> ListValue(const std::string & key, const std::string &separator = "");
	virtual std::vector<std::string> Sections();
	virtual std::vector<std::string> Keys();
	virtual bool Start();
	virtual bool End();
	virtual bool Abort();
	virtual bool Exists();

private:
	bool mTestedExistence;
	bool mExists;
	cpp_redis::client *mClient;
	std::map<std::string, KEY_VAL_LIST> mValues;
	bool mGotSections;
	std::vector<std::string> mSections;

	std::string CreateSectionKey();
	void CheckLoaded();
};

class ClusterEntityWriter: public AbstractEntityWriter {
public:
	ClusterEntityWriter(cpp_redis::client *client, bool sync = true);
	virtual ~ClusterEntityWriter();

	virtual bool Value(const std::string &key, const std::string &value);
	virtual bool ListValue(const std::string &key, std::vector<std::string> &value);

	virtual bool Start();
	virtual bool End();
	virtual bool Abort();

private:
	bool mSync;
	cpp_redis::client *mClient;
	std::vector<ClusterEntityStringValue> mStringValues;
	std::vector<ClusterEntityRemoveValue> mRemoveValues;
};

class Shard {
public:
	std::string mName;
	std::string mFullName;
	std::string mPassword;
	int mPlayers;
	unsigned long mLastSeen;
	int mSimulatorPort;
	unsigned long mStartTime;
	int mPing;
	std::string mSimulatorAddress;

	Shard();

	unsigned long GetLocalTime();
	unsigned long GetServerTime();
	void SetTimes(unsigned long localTime, unsigned long serverTimer);
	bool IsMaster();
private:
	unsigned long mServerTime;
	unsigned long mLocalTime;
	unsigned long mTimeSet;
};

class PendingShardPlayer {

public:
	std::string mToken;
	std::string mShardName;
	int mID;
	unsigned long mReceived;
};

class ShardPlayer {
public:
	int mID;
	int mZoneID;
	unsigned long mSimID;
	CharacterData * mCharacterData;
	std::string mShard;
	bool IsLocal();
	bool IsRemote();
};

class ClusterManager {
public:
	ClusterManager();
	bool mNoEvents;
	std::string mShardName;
	std::string mFullName;
	std::map<int, ShardPlayer> mActivePlayers;
	int Init(const std::string &configPath);
	void Ready();
	void Shutdown(bool wait = false);
	bool IsMaster();
	void RunProcessingCycle();
	STRINGLIST GetAvailableShardNames();
	int CountAccountSessions(int accountID, bool includeLocal = true, bool includeRemote = true);
	Shard GetActiveShard(const std::string &shardName);
	std::string GetClusterEnvironment();
	void JoinedShard(unsigned long simID, int zoneID, CharacterData *cdata);
	void LeftShard(int CDefID);
	void Login(int accountID);
	void Logout(int accountID);
	void Auction();
	void Chat(ChatMessage &message);
	void Thunder(int zoneId, const std::string &mapType);
	void Auction(int auctionItemId, const std::string &sellerName);
	void AuctionItemRemoved(int auctionItemId, int auctioneerCDefID);
	void AuctionItemUpdated(int auctionItemId);
	void PropUpdated(int propId);
	void Weather(int zoneId, const std::string &mapType, const std::string &type, int weight);
	PendingShardPlayer FindToken(const std::string &token);
	ShardPlayer GetActivePlayer(int CDefId);
	std::string SimTransfer(int CDefID, const std::string &shardName, int simID);
	void ConfirmTransfer(int cdefId, const string &shardName, const string &token, int simID);
	bool IsPlayerOnOtherShard(const std::string &characterName);
	std::string GetMaster();
	std::recursive_mutex mMutex;
	bool WriteEntity(AbstractEntity *entity, bool sync = true);
	bool ReadEntity(AbstractEntity *entity);
	bool RemoveEntity(AbstractEntity *entity);
	bool HasKey(const std::string &key);
	bool RemoveKey(const std::string &key, bool sync = true);
	std::string GetKey(const std::string &key);
	std::string GetKey(const std::string &key, const std::string &defaultValue);
	std::vector<std::string> GetList(const std::string &key);
	bool ListAdd(const std::string &key, const std::string &value, bool sync = true);
	bool ListSet(const std::string &key, const std::vector<std::string> &value, bool sync = true);
	bool ListRemove(const std::string &key, const std::string &value, bool sync = true);
	bool SetKey(const std::string &key, const std::string &defaultValue, bool sync = true);
	int Scan(const ScanCallback& task, const std::string &pattern, size_t max = 0);
	int64_t NextValue(const std::string &key, int incr = 1);
private:
	std::vector<PendingShardPlayer> mPending;
	std::string mMasterShard;
	bool mClusterable;
	bool mMaster;
	unsigned long mPingSentTime;
	unsigned long mNextPing;
	void TransferFromOtherShard(int cdefId, const std::string &shardName, std::string token, int simID);
	void ConfirmTransferToOtherShard(int cdefId, const std::string &shardName, std::string token, int simID);
	void FindMasterShard();
	void SendConfiguration();
	void NewShard(const std::string &shardName);
	void ShardRemoved(const std::string &shardName);
	void LeftOtherShard(const std::string &shardName, int cdefID);
	void JoinedOtherShard(const std::string &shardName, int cdefID, int zoneID, unsigned long simID);
	void ShardPing(const std::string &shardName, unsigned long localTime);
	void ShardPong(const std::string &shardName);
	void ServerConfigurationReceived(const std::string &shardName, const std::string &simulatorAddress, int simulatorPort, const std::string &fullName, int mPlayers, unsigned long startTime, unsigned long utcTime, unsigned long localTime);
	void OtherShardChat(const std::string &from, const std::string &to, const std::string &channel, bool tell, int senderClanID, const std::string &msg);
	void OtherShardWeather(int zoneId, const std::string &mapType, const std::string &type, int weight);
	void OtherShardThunder(int zoneId, const std::string &mapType);
	void OtherShardLogin(const std::string &shardName, int accountID);
	void OtherShardLogout(const std::string &shardName, int accountID);
	void OtherShardAuction(int auctionItemId, const string &sellerName);
	void OtherShardAuctionItemRemoved(int auctionItemId, int auctioneerCDefID);
	void OtherShardAuctionItemUpdated(int auctionItemId);
	void OtherShardPropUpdated(int propId);
	cpp_redis::subscriber mSub;
	cpp_redis::client mClient;
	std::map<std::string, Shard> mActiveShards;
};

extern ClusterManager g_ClusterManager;

#endif //#ifndef CLUSTER_H
