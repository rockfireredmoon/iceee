#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "SocketClass3.h"
#include "Packet.h"
#include "Components.h"

#include "ActiveCharacter.h"
#include "QuestScript.h"
#include "Report.h"
#include "Guilds.h"
#include "DropTable.h"
#include "PartyManager.h"

class AccountData;
struct PlayerInstancePlacementData;

class ThreadRequest
{
public:
	ThreadRequest();
	~ThreadRequest();

	static const int DEFAULT_WAIT_TIME = 30000;
	enum StatusNames
	{
		STATUS_NONE       = 0,  //No status.
		STATUS_WAITMAIN   = 1,  //Request sent, waiting for main thread to respond.
		STATUS_WAITWORK   = 2,  //Main thread has acknowledged and is now waiting for the worker thread to complete its task.
		STATUS_COMPLETE   = 3   //Worker thread has finished.
	};
	volatile int status;
	bool WaitForStatus(int statusID, int checkInterval, int maxError);
};

//A query that is sent to the simulator from the client.
//Composed of a query ID, a string identifier for the query, and a
//list of strings, one for each query argument.
struct SimulatorQuery
{
	int ID;
	std::string name;
	std::vector<std::string> args;
	uint argCount;

	SimulatorQuery();
	~SimulatorQuery();
	void Clear(void);

	bool ValidArgIndex(uint argIndex);
	const char* GetString(uint argIndex);
	int GetInteger(uint argIndex);
	float GetFloat(uint argIndex);
	bool GetBool(uint argIndex);
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
	static const int OTHERBUSYSKILL = -28;  //Other player is busy casting an ability or using a quest object.
};

class SimulatorThread
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
	int InternalIndex;     //Internal index of this simulator array index.
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
	int ReadPos;    //Current read position within RecBuf(), when extracting data from packets.
	int WritePos;   //Current write position within SendBuf(), when writing packets.
	bool PendingSend; //If set to true, the recv() handler will send any data marked for sending
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
	void RunMainLoop(void);
	void LogMessageL(unsigned short messageType, const char *format, ...);
	int InitThread(int instanceindex, int globalThreadID);
	void Disconnect(const char *debugCaller);
	void AddPendingDisconnect(void);
	void ProcessDisconnect(void);
	void ForceErrorMessage(const char *message, int msgtype);
	int AttemptSend(const char *buffer, uint buflen);
	void OnConnect(void);

	bool CheckStateGameplayProtocol(void) const;
	bool CheckStateGameplayActive(void) const;

	void HandleReceivedMessage2(void);
	void HandleGameMsg(int msgType);
	void HandleLobbyMsg(int msgType);
	void handle_lobby_authenticate(void);
	void handle_lobby_selectPersona(void);
	void handle_lobby_query(void);
	void handle_swimStateChange(void);
	void handle_inspectItemDef(void);
	void handle_updateVelocity(void);
	void handle_communicate(void);
	void handle_disconnect(void);

	void handle_debugServerPing(void);

	void handle_acknowledgeHeartbeat(void);
	void ReadQueryFromMessage(void);

	void handle_game_query(void);
	bool HandleQuery(int &PendingData);
	bool HandleCommand(int &PendingData);

	void ClearAuxBuffers(void);
	void BroadcastMessage(const char *message);
	void SendInfoMessage(const char *message, char eventID);
	void SendPlaySound(const char *assetPackage, const char *soundFile);
	void LoadAccountCharacters(AccountData *accPtr);
	void JoinGuild(GuildDefinition *gDef, int startValour);

	void ClearLoot(ActiveParty *party, ActiveLootContainer *loot);
	void ResetLoot(ActiveParty *party, ActiveLootContainer *loot, int itemId);
	void UndoLoot(ActiveParty *party, ActiveLootContainer *loot, int itemId, int creatureId);

	int OfferLoot(int mode, ActiveLootContainer *loot, ActiveParty *party, CreatureInstance *receivingCreature, int ItemID, bool needOrGreed, int CID, int conIndex);

	void CheckIfLootReadyToDistribute(ActiveLootContainer *loot, LootTag *lootTag);
	PartyMember * RollForPartyLoot(ActiveParty *party, std::set<int> creatureIds, const char *rollType, int itemId);

	bool ActivateActionAbilities(InventorySlot *slot);

	int handle_query_account_info(void);
	int handle_query_statuseffect_set(void);
	void handle_query_persona_list(void);
	int handle_query_persona_create(void);
	int handle_query_persona_delete(void);
	void handle_query_pref_getA(void);
	void handle_query_pref_get(void);
	void handle_query_pref_setA(void);
	void handle_query_pref_set(void);
	void handle_query_client_loading(void);
	int handle_query_admin_check(void);
	int handle_query_persona_gm(void);
	int handle_query_item_contents(void);
	int handle_query_scenery_list(void);
	void handle_query_account_tracking(void);
	void handle_query_account_fulfill(void);
	int handle_query_ab_remainingcooldowns(void);
	int handle_query_map_marker(void);
	int handle_query_item_move(void);
	int handle_query_item_delete(void);
	void handle_query_item_split(void);
	void handle_query_loot_list(void);
	void handle_query_loot_item(void);
	void handle_query_loot_need_greed_pass(void);
	int protected_helper_query_loot_need_greed_pass(void);
	void handle_query_loot_exit(void);
	void handle_query_creature_isusable(void);
	void handle_query_scenery_edit(void);
	void handle_query_scenery_delete(void);
	int handle_query_friends_list(void);
	void handle_query_friends_add(void);
	int handle_query_friends_remove(void);
	void handle_query_friends_status(void);
	void handle_query_friends_getstatus(void);
	int handle_query_creature_def_edit(void);
	int handle_query_item_def_use(void);
	int handle_query_item_use(void);
	void handle_query_ab_ownage_list(void);
	int handle_query_ab_buy(void);
	void handle_query_ab_respec(void);
	int handle_query_buff_remove(void);
	int handle_query_util_ping(void);
	void handle_query_util_pingsim(void);
	void handle_query_util_pingrouter(void);
	void handle_query_essenceShop_contents(void);
	void handle_query_shop_contents(void);
	int handle_query_trade_shop(void);
	int handle_query_trade_essence(void);
	void helper_shop_contents_self(void);
	int helper_trade_shop_sell(const char *itemHex);
	int helper_trade_shop_buyback(unsigned int CCSID);
	int handle_query_henge_setDest(void);
	int handle_query_portal_acceptRequest(void);
	int handle_query_creature_use(void);
	int handle_query_visWeapon(void);
	int handle_query_quest_indicator(void);
	int handle_query_quest_getquestoffer(void);
	int handle_query_quest_genericdata(void);
	int handle_query_quest_join(void);
	int handle_query_quest_hack(void);
	int handle_query_quest_list(void);
	int handle_query_quest_data(void);
	int handle_query_quest_getcompletequest(void);
	int handle_query_quest_complete(void);
	int handle_query_quest_leave(void);
	int handle_query_trade_start(void);
	int protected_helper_query_trade_start(void);
	int handle_query_trade_items(void);
	int protected_helper_query_trade_items(void);
	int handle_query_trade_cancel(void);
	int protected_helper_query_trade_cancel(void);
	int	handle_query_trade_offer(void);
	int protected_helper_query_trade_offer(void);
	int handle_query_trade_accept(void);
	int protected_helper_query_trade_accept(void);
	int handle_query_trade_currency(void);
	int protected_helper_query_trade_currency(void);
	int handle_query_craft_create(void);
	int protected_helper_query_craft_create(void);
	int handle_query_item_morph(void);
	int protected_helper_query_item_morph(bool command);
	int protected_helper_checkdistance(int creatureID);
	int handle_query_shard_list(void);
	int handle_query_sidekick_notifyexp(void);
	int handle_query_ab_respec_price(void);
	int handle_query_util_version(void);
	int handle_query_persona_resCost(void);
	int handle_query_petition_send(void);
	int handle_query_petition_doaction(void);
	int handle_query_itemdef_contents(void);
	int handle_query_itemdef_delete(void);
	int handle_query_item_create(void);
	int handle_query_item_market_list(void);
	int handle_query_item_market_edit(void);
	int handle_query_item_market_buy(void);
	int handle_query_item_market_reload(void);
	int handle_query_item_market_purchase_name(void);
	int handle_query_bug_report(void);
	int handle_query_util_addfunds();
	int handle_query_user_auth_reset();
	int handle_query_validate_name();
	int handle_query_petition_list(void);
	int handle_query_marker_list(void);
	int handle_query_marker_edit(void);
	int handle_query_marker_del(void);
	int handle_query_gm_spawn(void);
	int handle_query_guild_info(void);
	int handle_query_guild_leave(void);
	int handle_query_script_op(int op);
	int handle_query_clan_info(void);
	int handle_query_clan_list(void);
	int handle_query_spawn_list(void);
	int handle_query_spawn_create(void);
	int handle_query_creature_delete(void);
	int handle_query_build_template_list(void);
	int handle_query_spawn_property(void);
	int handle_query_spawn_emitters(void);
	int handle_query_scenery_link_add(void);
	int handle_query_scenery_link_del(void);
	int handle_query_unstick(void);
	int handle_query_ps_join(void);
	int handle_query_ps_leave(void);
	int handle_query_party(void);
	int handle_query_quest_share(void);
	int handle_query_vault_size(void);
	int handle_query_vault_send(void);
	int handle_query_vault_expand(void);
	int handle_query_shard_set(void);

	int handle_query_mod_setgrovestart(void);
	int handle_query_mod_setenvironment(void);
	int handle_query_mod_grove_togglecycle(void);
	int handle_query_mod_setats(void);
	int handle_query_mod_getURL(void);
	//int handle_query_mod_igforum_createcategory(void);
	int handle_query_mod_igforum_getcategory(void);
	int handle_query_mod_igforum_opencategory(void);
	int handle_query_mod_igforum_openthread(void);
	int handle_query_mod_igforum_sendpost(void);
	int handle_query_mod_igforum_deletepost(void);
	//int handle_query_mod_igforum_deletethread(void);
	int handle_query_mod_igforum_setlockstatus(void);
	int handle_query_mod_igforum_setstickystatus(void);
	int handle_query_mod_igforum_deleteobject(void);
	int handle_query_mod_igforum_runaction(void);
	int handle_query_mod_igforum_editobject(void);
	int handle_query_mod_igforum_move(void);
	int handle_query_mod_itempreview(void);
	int handle_query_mod_restoreappearance(void);
	int handle_query_mod_pet_list(void);
	int handle_query_mod_pet_purchase(void);
	int handle_query_mod_pet_preview(void);
	int handle_query_mod_ping_statistics(void);
	int handle_query_mod_emote(void);
	int handle_query_mod_emotecontrol(void);
	int handle_query_mod_getpet(void);
	int handle_query_mod_craft(void);
	int handle_query_mod_getdungeonprofiles(void);
	int handle_query_mod_morestats(void);
	int handle_query_updateContent(void);
	int handle_query_instance(void);
	int handle_query_go(void);
	int handle_query_script_exec(void);
	int handle_query_script_time(void);
	int handle_query_script_gc(void);
	int handle_query_zone_mode(void);
	int handle_query_mode(void);
	int handle_query_team(void);

	int protected_CheckDistanceBetweenCreatures(CreatureInstance *sourceCreatureInst, int creatureID);
	int protected_CheckDistanceBetweenCreaturesFor(CreatureInstance *sourceCreatureInst, int creatureID, int range);
	int protected_CheckDistance(int creatureID);
	int protected_helper_query_item_move(int origContainer, int origSlot, int destContainer, int destSlot, AccountData *destAccount);
	int protected_helper_query_loot_item(void);
	bool HasPropEditPermission(SceneryObject *prop, float x = 0.0F, float z = 0.0F);
	int protected_helper_query_scenery_edit(void);
	int protected_helper_query_scenery_delete(void);
	const char * GetErrorString(int error);
	bool HasQueryArgs(uint minCount);
	int ResolveEmoteTarget(int target);
	void JoinPrivateChannel(const char *channelname, const char *password);

	void RespondPrefGet(PreferenceContainer *prefSet);
	int UseItem(unsigned int CCSID);
	void DecrementStack(InventorySlot *slot);
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
	std::string chatHeader;
	std::string chatFooter;

	ThreadRequest threadsys;

	bool WaitUntilNonzero(int *data);
	int ResolveCreatureDef(int CreatureID);
	CreatureInstance* ResolveCreatureInstance(int CreatureID, int searchHint);

	//Actual functions called by the Main Call system.
	bool MainCallSetZone(int newZoneID, int newInstanceID, bool setDefaultLocation);
	bool ProtectedSetZone(int newZoneID, int newInstanceID);

	//Helper functions may not be thread safe and are only meant to be called
	//from within the Main Call system.
	void MainCallHelperInstanceUnregister(void);
	void MainCallHelperInstanceRegister(int ZoneID, int InstanceID);

	void FillPlayerInstancePlacementData(PlayerInstancePlacementData &pd, int ZoneID, int InstanceID);

	void LoadCharacterSession(void);
	void ChangeProtocol(int newProto);
	bool ValidPointers(void);
	void SendSetAvatar(int CreatureID);
	void SendZoneInfo(void);
	const char * GetTimeOfDay(void);
	void SendTimeOfDay(const char *envType);
	void CheckMapUpdate(bool force);
	void UpdateSocialEntry(bool newOnlineStatus, bool onlyUpdateFriendList);
	void BroadcastGuildChange(int guildDefID);
	void BroadcastShardChanged(void);
	void SendSetMap(void);
	void SetPosition(int xpos, int ypos, int zpos, int update);
	void UpdateEqAppearance(void);
	void ActivateSavedAbilities(void);
	void ActivatePassiveAbilities(void);
	void CheckSpawnTileUpdate(bool force);

	void handle_inspectCreatureDef(void);
	void handle_inspectCreature(void);
	void handle_selectTarget(void);
	void handle_abilityActivate(void);
	void AddMessage(long param1, long param2, int message);
	bool CheckPermissionSimple(int permissionSet, unsigned int permissionFlag);
	void WarpToZone(ZoneDefInfo *zoneDef, int xOverride, int yOverride, int zOverride);
	void SendAbilityErrorMessage(int abilityErrorCode);

	//Custom commands, accessed by chat /commands.
	int handle_command_pose(void);
	int handle_command_pose2(void);
	int handle_command_esay(void);
	int handle_command_warp(void);
	int handle_command_warpi(void);
	int handle_command_warpt(void);
	int handle_command_warpp(void);
	int handle_command_warpg(void);
	int handle_command_warpextoff(void);
	int handle_command_warpext(void);
	int handle_command_health(void);
	int handle_command_speed(void);
	int handle_command_fa(void);
	int handle_command_skadd(void);
	void handle_command_skremove(void);
	void handle_command_skremoveall(void);
	void handle_command_skattack(void);
	void handle_command_skcall(void);
	void handle_command_skwarp(void);
	void handle_command_skscatter(void);
	void handle_command_sklow(void);
	void handle_command_skparty(void);
	int handle_command_partylowest(void);
	int handle_command_who(void);
	int handle_command_gmwho(void);
	int handle_command_chwho(void);
	int handle_command_give(void);
	int handle_command_giveid(void);
	int handle_command_giveall(void);
	int handle_command_giveapp(void);
	int handle_command_deleteall(void);
	void handle_command_deleteabove(void);
	int handle_command_grove(void);
	int handle_command_pvp(void);
	int handle_command_complete(void);
	int handle_command_refashion(void);
	int handle_command_backup(void);
	int handle_command_restore(void);
	int handle_command_god(void);
	int handle_command_setstat(void);
	int handle_command_adjustexp(void);
	int handle_command_scale(void);
	int protected_helper_command_scale(void);
	int handle_command_partyall(void);
	int handle_command_partyquit(void);
	int handle_command_ccc(void);
	int handle_command_ban(void);
	int handle_command_unban(void);
	int handle_command_setbuildpermission(void);
	int handle_command_setpermission(void);
	int handle_command_setpermissionc(void);
	int handle_command_setbehavior(void);
	int handle_command_deriveset(void);
	int handle_command_igstatus(void);
	int handle_command_partyzap(void);
	int handle_command_zonename(void);
	int handle_command_forumlock(void);
	int handle_command_dtrig(void);
	int handle_command_sdiag(void);
	int handle_command_set_earsize(void);
	int handle_command_set_tailsize(void);
	int handle_command_sping(void);
	int handle_command_info(void);
	int handle_command_grovesetting(void);
	int handle_command_grovepermission(void);
	int handle_command_dngscale(void);
	int handle_command_pathlinks(void);
	int handle_command_partyinvite(void);
	int handle_command_roll(void);
	int handle_command_daily(void);

	bool QuestResetObjectives(int QuestID, int objective);
	bool QuestJoin(int QuestID);
	bool QuestClear(int QuestID);

	int AddSidekick(int CDefID);
	void AddPet(int CDefID);
	void DoWarp(int zoneID, int instanceID, int xpos, int ypos, int zpos);
	int CheckValidHengeDestination(const char *destName, int creatureID);
	//void CheckQuestItems(void); DISABLED, NEVER FINISHED

	void RunTranslocate(void);
	void RunPortalRequest(void);
	void CreatureUseHenge(int creatureID, int creatureDefID);
	void ShardSet(const char *shardName, const char *charName);

	const char * GetGenericErrorString(int errCode);
	int CheckValidWarpZone(int ZoneID);
	bool IsGMInvisible(void);

	bool TargetRarityAboveNormal(void);
	//void VerifySendBufSize(int length);
	void VerifyGenericBuffer(const char *buffer, uint buflen);
	void LogPingStatistics(bool server, bool client);

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
	Platform_CriticalSection cs;
};

class SimulatorManager
{
public:
	SimulatorManager();
	~SimulatorManager();
	void Init(void);
	void Free(void);

	//std::vector<SimulatorThread*> regList;
	std::vector<ThreadRequest*> regList;

	std::list<SimulatorThread> simList;  //List of Simulator objects.

	int nextSimulatorID;
	SimulatorThread * CreateSimulator(void);

	volatile int pendingActions;
	long baseByteSent;  //Total cumulative bytes sent from all unloaded Simulators.
	long baseByteRec;   //Total cumulative bytes received from all unloaded Simulators.

	bool debug_acquired;  //For assisting in traces, determines whether the main thread has been acquired and is running processing on this object

	void RegisterAction(ThreadRequest *reqData);
	void UnregisterAction(ThreadRequest *reqData);
	void RunPendingActions(void);

	SimulatorThread* GetPtrByID(int simID);
	
	void AddPendingDisconnect(SimulatorThread *callObject);
	void AddPendingPacketData(SimulatorThread *callObject);
	void BroadcastMessage(const char *message);

	//CreatureInstance* GetPlayerByID(int CreatureID);

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

	std::vector<SimulatorThread*> pendingDisconnects;
	std::vector<SimulatorThread*> pendingPacketData;  //Holds a list of simulators that require processing of their packets.

	void ProcessPendingDisconnects(void);
	void ProcessPendingPacketData(void);
	void ProcessPendingActions(void);
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
