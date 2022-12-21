#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "SocketClass3.h"
#include "Packet.h"
#include "Clan.h"
#include "Components.h"

#include "ActiveCharacter.h"
#include "QuestScript.h"
#include "Report.h"
#include "Guilds.h"
#include "Scheduler.h"
#include "DropTable.h"
#include "PartyManager.h"
#include "Cluster.h"

#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread.hpp>

extern char GAuxBuf[1024];    //Note, if this size is modified, change all "extern" references
extern char GSendBuf[32767];  //Note, if this size is modified, change all "extern" references

class AccountData;
struct PlayerInstancePlacementData;

//A query that is sent to the simulator from the client.
//Composed of a query ID, a string identifier for the query, and a
//list of strings, one for each query argument.
struct SimulatorQuery
{
	int ID;
	std::string name;
	std::vector<std::string> args;
	unsigned int argCount;

	SimulatorQuery();
	~SimulatorQuery();
	void Clear(void);

	bool ValidArgIndex(unsigned int argIndex);
	const char* GetString(unsigned int argIndex);
	std::string GetStringObject(unsigned int argIndex);
	int GetInteger(unsigned int argIndex);
	long GetLong(unsigned int argIndex);
	float GetFloat(unsigned int argIndex);
	bool GetBool(unsigned int argIndex);
};

struct QueryErrorMsg
{
	static const int NONE          = 0;    //No error.
	static const int GENERIC       = -1;   //Generic error message.
	static const int INVSPACE      = -2;   //Not enough free inventory space.
	static const int LOOT          = -3;   //The referenced creature does not have a loot package.
	static const int LOOTDENIED    = -4;   //Player cannot loot that target.
	static const int LOOTMISSING   = -5;   //The requested item no longer exists in the loot container.
	static const int INVCREATE     = -6;   //Could not create an item in a player's inventory.
	static const int INVALIDITEM   = -7;   //Item does not exist in the database.
	static const int INVMISSING    = -8;   //The player does not have a required item in their inventory.
	static const int INVALIDOBJ    = -9;   //The Creature Instance ID could not be found.
	static const int OUTOFRANGE    = -10;  //Object is too far away to interact with. 
	static const int COIN          = -11;  //Player doesn't have enough copper.
	static const int TRADENOTFOUND = -12;  //A trade object was not found.
	static const int TRADENOTOPENED = -13; //The other player has not accepted the trade request.
	static const int PROPNOEXIST   = -20;  //The prop could not be found.
	static const int PROPLOCATION  = -21;  //The prop location was off limits (no permission to build there).
	static const int PROPATS       = -22;  //The prop asset references an invalid ATS.
	static const int PROPOWNER     = -23;  //Cannot edit props that do not belong to its owner.
	static const int PROPASSETNULL = -24;  //The prop cannot have a null asset name.
	static const int TRADEBUSY     = -25;  //The character is busy with another trade.
	static const int GENERALBUSY   = -26;  //The character is busy or cannot move items at this time.
	static const int SELFBUSYSKILL = -27;  //Player is busy casting an ability or using a quest object.
	static const int OTHERBUSYSKILL = -28; //Other player is busy casting an ability or using a quest object.
	static const int NOREFASHION = -29;    //Cannot refashion item because ability to do so is missing.
	static const int NOCRAFT= -30;    		//Cannot craft item because ability to do so is missing.
};

class SimulatorThread: public Schedulable
{
public:
	SimulatorThread();
	~SimulatorThread();
	void ResetValues(bool hardReset);  //Reset all member variables to a zero, newly initialized state.

	int Debug_TestCondition;  //General variable for helping to test certain debug conditions.

	//Thread management
	Platform_CriticalSection sim_cs;
	unsigned long ThreadID;  //ID of the created thread
	bool isThreadExist;      //If true, the thread is considered to exist.
	bool isThreadActive;     //If true, the thread main loop is active and will continue to loop.
	int Status;              //Status of the thread loop
	bool isConnected;        //Set to true when the client socket has connected.  It doesn't fully correlate to socket connectivity, but is used as a general flag to determine if the client should still be connected, barring any unusual errors.
	bool firstConnect;       //If true, the first incoming packet must contain expected data.

	//Logging, debugging, or auxillary info
	char LogBuffer[4096];  //Holds a constructed log message generated by this thread
	int InternalID;
	int GlobalThreadID;   //A unique application-defined thread ID for debugging purposes.

	//Packet and data buffers
	SocketClass sc;
	char RecBuf[8192];       //Holds the data that was received from a connection
	const char *readPtr;
	Packet recvData;          //Buffered data that is immediately stored immediately from the recv() results.
	Packet procData;         //Data that is grabbed out of recData when processed by the main thread. 
	char SendBuf[24576];     //Holds data that is being prepared for sending
	int SocketStatus;        //If nonzero, a potential error was flagged.  Interestingly enough, having this helped detect a buffer overflow, since any nonzero value would disconnect the client on the next loop.
	long TotalMessageSent;
	long TotalMessageReceived;
	long TotalSendBytes;
	long TotalRecBytes;
	int MessageEnd;
	size_t ReadPos;    //Current read position within RecBuf(), when extracting data from packets.
	char Aux1[4096];          //Generic large buffer.
	char Aux2[128];           //Generic small buffer.
	char Aux3[128];           //Generic small buffer.
	char clientPeerName[64];  //Holds IP address information.
	unsigned long LastChatTime;  //Holds the time of the most recent chat.
	short chatMessageFrequency;  //Holds a counter of the chat messages.  When the counter is full, a certain amount of time must pass before more messages are accepted.

	//Game state
	short ProtocolState;  // 0 = lobby, 1 = gameplay
	bool ClientLoading;
	short LoadStage;
	static const int PROTOCOLSTATE_LOBBY = 0;
	static const int PROTOCOLSTATE_GAME  = 1;
	unsigned long LastUpdate;     //The time that a player hard position update was last sent.
	unsigned long LastRecv;       //The time that the simulator last received a message
	unsigned long LastHeartbeatSend; //The time that the simulator last sent a heartbeat
	bool characterCreation;

	int PendingHeartbeatResponse;             //Should equalize to zero if communications are operating normally.
	AppearanceModifier *creatureTweakModifier;

	//FUNCTIONS
	void InitThread(int globalThreadID);
	void Disconnect(const char *debugCaller);
	void ProcessDisconnect(void);

	void ForceErrorMessage(const char *message, int msgtype);
	int AttemptSend(const char *buffer, unsigned int buflen);
	void OnConnect(void);

	bool CheckStateGameplayProtocol(void) const;
	bool CheckStateGameplayActive(void) const;

	void HandleReceivedMessage2(void);
	void HandleGameMsg(int msgType);
	void HandleLobbyMsg(int msgType);

	void ReadQueryFromMessage(void);

	void ClearAuxBuffers(void);
	void BroadcastMessage(const char *message);
	void SendInfoMessage(const char *message, char eventID);
	void SendPlaySound(const char *assetPackage, const char *soundFile);
	void LoadAccountCharacters(AccountData *accPtr);
	void JoinGuild(GuildDefinition *gDef, int startValour);

	bool ActivateActionAbilities(InventorySlot *slot);
	int CheckDistance(int creatureID);
	int ItemMorph(bool command);
	void SetZoneMode(int mode);

	int protected_CheckDistanceBetweenCreatures(CreatureInstance *sourceCreatureInst, int creatureID);
	int protected_CheckDistanceBetweenCreaturesFor(CreatureInstance *sourceCreatureInst, int creatureID, int range);
	int protected_CheckDistance(int creatureID);
	int protected_helper_query_item_move(int origContainer, int origSlot, int destContainer, int destSlot, AccountData *destAccount);

	int protected_helper_tweak_self(int CDefID, int defhints, int argOffset);

	bool HasPropEditPermission(SceneryObject *prop, float x = 0.0F, float z = 0.0F);
	const char * GetErrorString(int error);
	bool HasQueryArgs(unsigned int minCount);
	int ResolveEmoteTarget(int target);
	void JoinPrivateChannel(const char *channelname, const char *password);

	void RespondPrefGet(PreferenceContainer *prefSet);
	int UseItem(unsigned int CCSID);
	void RunFinishedCast(bool success);
	bool CanMoveItems(void);

	void SetLoadingStatus(bool status, bool shutdown);
	void ProcessDailyRewards(unsigned int days, unsigned int level);
	void SetPersona(int personaIndex);
	void SaveCharacterStats(void);
	bool CheckWriteFlush(int &curPos);

	void SetAccountCharacterCache(void);

	int SendInventoryData(std::vector<InventorySlot> &cont);

	CharacterServerData pld;
	SimulatorQuery query;
	CreatureInstance *creatureInst;
	CreatureInstance defcInst;
	unsigned long TimeOnline;
	unsigned long TimeLastAutoSave;

	bool WaitUntilNonzero(int *data);
	int ResolveCreatureDef(int CreatureID);
	CreatureInstance* ResolveCreatureInstance(int CreatureID, int searchHint);

	//Actual functions called by the Main Call system.
	bool MainCallSetZone(int newZoneID, int newInstanceID, bool setDefaultLocation);
	bool ProtectedSetZone(int newZoneID, int newInstanceID);

	void FillPlayerInstancePlacementData(PlayerInstancePlacementData &pd, int ZoneID, int InstanceID);

	void LoadCharacterSession(void);
	void ChangeProtocol(int newProto);
	bool ValidPointers(void);
	void SendSetAvatar(int CreatureID);
	std::string GetTimeOfDay(void);
	void CheckMapUpdate(bool force);
	void UpdateSocialEntry(bool newOnlineStatus, bool onlyUpdateFriendList);
	void BroadcastGuildChange(int guildDefID);
	void BroadcastShardChanged(void);
	void SendSetMap(void);
	void SetRotation(int rot, int update);
	void SetPosition(int xpos, int ypos, int zpos, int update);
	void UpdateEqAppearance(void);
	void ActivateSavedAbilities(void);
	void ActivatePassiveAbilities(void);
	void CheckSpawnTileUpdate(bool force);

	void handle_abilityActivate(void);
	bool CheckPermissionSimple(int permissionSet, unsigned int permissionFlag);
	void WarpToZone(ZoneDefInfo *zoneDef, int xOverride, int yOverride, int zOverride);
	void SendAbilityErrorMessage(int abilityErrorCode);

	bool QuestResetObjectives(int QuestID, int objective);
	bool QuestJoin(int QuestID);
	bool QuestInvite(int QuestID);
	bool QuestClear(int QuestID);

	void DecrementStack(InventorySlot *slot);
	void DoWarp(int zoneID, int instanceID, int xpos, int ypos, int zpos);
	int CheckValidHengeDestination(const char *destName, int creatureID);
	//void CheckQuestItems(void); DISABLED, NEVER FINISHED

	void RunTranslocate(void);
	void RunPortalRequest(void);
	void CreatureUseHenge(int creatureID, int creatureDefID);

	void FinaliseTransfer(Shard &shard, const std::string &token);
	std::string ShardSet(std::string shardName, std::string charName);

	const char * GetGenericErrorString(int errCode);
	int CheckValidWarpZone(int ZoneID);
	bool IsGMInvisible(void);

	bool TargetRarityAboveNormal(void);
	//void VerifySendBufSize(int length);
	void VerifyGenericBuffer(const char *buffer, unsigned int buflen);
	const char * GetScriptUsable(CreatureInstance *target);

	void Debug_GenerateReport(ReportBuffer *report);
	void Debug_GenerateCreatureReport(ReportBuffer &report);
	void Debug_GenerateItemReport(ReportBuffer &report, bool simple);

	int ErrorMessageAndQueryOK(char *buffer, const char *errorMessage);

	enum LoadStages
	{
		LOADSTAGE_UNLOADED = 0,
		LOADSTAGE_LOADING,
		LOADSTAGE_LOADED,
		LOADSTAGE_GAMEPLAY
	};
	static const int OverflowThreshold = 0x8000;
	static const int OverflowAdditive = 0x10000;
	static const int DefaultWarpDistance = 200;

	static const int INTERACT_RANGE = 50;
	static const int PARTY_LOOT_RANGE =1920;

	QuestScript::QuestScriptPlayer questScript;

	static const int ERROR_NONE = 0;
	static const int ERROR_INVALIDZONE = -1;
	static const int ERROR_WARPGROVEONLY = -2;
	static const int ERROR_USERBLOCK = -3;

private:
	void ProcessDetach(void);
	//Helper functions may not be thread safe and are only meant to be called
	//from within the Main Call system.
	void MainCallHelperInstanceUnregister(void);
	void MainCallHelperInstanceRegister(int ZoneID, int InstanceID);
	void RunMain(void);
	void RunMainLoop(void);

	Platform_CriticalSection cs;
};

class SimulatorManager
{
public:
	SimulatorManager();
	~SimulatorManager();
	void Init(void);
	void Free(void);

	std::list<SimulatorThread> simList;  //List of Simulator objects.

	int nextSimulatorID;
	SimulatorThread * CreateSimulator(void);

	long baseByteSent;  //Total cumulative bytes sent from all unloaded Simulators.
	long baseByteRec;   //Total cumulative bytes received from all unloaded Simulators.

	bool debug_acquired;  //For assisting in traces, determines whether the main thread has been acquired and is running processing on this object

	void RunPendingActions(void);
	void SendToAllSimulators(const char *buffer, unsigned int buflen, SimulatorThread *except);

	SimulatorThread* GetPtrByID(int simID);
	
	void AddPendingPacketData(SimulatorThread *callObject);
	void BroadcastMessage(const char *message);
	void BroadcastChat(int characterID, const char *display_name, const char *channel, const char *message);

	void FlushHangingSimulators(void);
	void CheckIdleSimulatorBoot(AccountData *account);
	static const long SIMULATOR_FLUSH_INTERVAL = 5000;
	static const long SIMULATOR_FLUSH_INACTIVE = 120000;
	static const long SIMULATOR_FLUSH_CHARCREATE = 600000;
	static const long SIMULATOR_BOOT_INACTIVE = 5000;  //If there is an explicit call to boot an inactive account, this idle duration must have passed.

private:

	Platform_CriticalSection cs;
	volatile int ActiveCount;
	volatile bool RequestSimulator;
	unsigned long nextFlushTime;

	std::vector<SimulatorThread*> pendingPacketData;  //Holds a list of simulators that require processing of their packets.

	void ProcessPendingPacketData(void);
};

//This object calls an automatic query response when the object destructor is called.
class DefaultQueryResponse
{
public:
	DefaultQueryResponse(const SimulatorThread& callSim, const char *outMsg);
	~DefaultQueryResponse();
private:
	const SimulatorThread& mSimulatorObject;
	const char *mResponseStr;
};


//This class provides a special wrapper to clear the contents of the simulator's auxiliary processing
//buffers when the instantiated object goes out of scope.
class OnExitFunctionClearBuffers
{
public:
	OnExitFunctionClearBuffers(SimulatorThread *caller);
	~OnExitFunctionClearBuffers();
private:
	SimulatorThread *mCaller;
};

void Debug_GenerateSimulatorReports(ReportBuffer *report);

extern std::list<SimulatorThread> Simulator;
typedef std::list<SimulatorThread>::iterator SIMULATOR_IT;

extern SimulatorManager g_SimulatorManager;
SimulatorThread * GetSimulatorByID(int ID);
SimulatorThread * GetSimulatorByCharacterName(const char *name);

int SendToOneSimulator(char *buffer, int length, SimulatorThread *simPtr);
int SendToAllSimulator(char *buffer, int length, int ignoreIndex);
int SendToOneSimulator(char *buffer, int length, int simIndex);
int SendToFriendSimulator(char *buffer, int length, int CDefID);

#endif //SIMULATOR_H
