#ifndef UTIL_H
#define UTIL_H

#include "Scenery2.h"
#include "ActiveCharacter.h"
#include "CommonTypes.h"

const int MODMESSAGE_EVENT_SUPERCRIT = 1;
const int MODMESSAGE_EVENT_EMOTE = 2;
const int MODMESSAGE_EVENT_EMOTE_CONTROL = 3;
const int MODMESSAGE_EVENT_PING_START = 20;
const int MODMESSAGE_EVENT_PING_STOP = 21;
const int MODMESSAGE_EVENT_PING_QUERY = 22;
const int MODMESSAGE_EVENT_GENERIC_REQUEST = 23;
const int MODMESSAGE_EVENT_STOP_SWIM = 40;
const int MODMESSAGE_EVENT_POPUP_MSG = 30;

extern char MessageBuf[128];

//Typically used with the SendInfoMessage() functions.
enum InfoMsgEnum
{
	INFOMSG_INFO          =  0,     //Yellow information text (IGIS.info())
	INFOMSG_ERROR         =  1,     //Red information text (IGIS.error())
	INFOMSG_FAIL          =  2,     //Same as INFOMSG_ERROR
	INFOMSG_ANNOUNCE      =  3,     //Same as INFOMSG_INFO
	INFOMSG_DEBUG         =  4,     //Same as INFOMSG_INFO
	INFOMSG_SYSERROR      =  5,     //  (likely client implementation error)
	INFOMSG_SYSNOTIFY     =  6,     //  (notification to client / warning(s))
	INFOMSG_LOCATION      =  7,     //Update minimap location text
	INFOMSG_SHARD         =  8,     //Update minimap shard text
	INFOMSG_MAPNAME       =  9,     //Update worldmap name
	INFOMSG_NONLOGERROR   = 10,     //Untested behavior, possibly different from INFOMSG_INFO and INFOMSG_ERROR
	INFOMSG_FALLINGDAMAGE = 11      //Falling damage
};


int WriteCharacterStats(int clIndex, char *buffer, int &wpos, int flagMask);
int PrepExt_CreatureDef(char *buffer, CreatureDefinition *cdef);
int PrepExt_UpdateCreatureDef(char *buffer, int CDefID, int defHints, std::vector<short>& statID, CharacterStatSet *css);
int PrepExt_UpdateAppearance(char *buffer, CreatureInstance *cInst);
int PrepExt_CreatureInstance(char *buffer, CreatureInstance *cInst);
int PrepExt_CreatureFullInstance(char *buffer, CreatureInstance *cInst);
int PrepExt_CreaturePos(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateVelocity(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdatePosInc(char *buffer, CreatureInstance *cInst);
int PrepExt_GeneralMoveUpdate(char *buffer, CreatureInstance *cInst);  //General server movement update (combines 3 flags of data)
int PrepExt_UpdateElevation(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateFullPosition(char *buffer, CreatureInstance *cInst);
int PrepExt_SetAvatar(char *buffer, int creatureID);
int PrepExt_SetMap(char *buffer, CharacterServerData *pldata, int x, int z);
int PrepExt_SetTimeOfDay(char *buffer, char *envType);
int PrepExt_UpdateScenery(char *buffer, SceneryObject *so);
int PrepExt_AbilityActivate(char *buffer, CreatureInstance *cInst, ActiveAbilityInfo *ability, int aevent, bool ground = false);
int PrepExt_AbilityActivateEmpty(char *buffer, CreatureInstance *cInst, ActiveAbilityInfo *ability, int aevent);
int PrepExt_AbilityEvent(char *buffer, int creatureID, int abilityID, int abilityEvent);
int PrepExt_SendAbilityOwn(char *buffer, int CID, int abilityID, int eventID);
int PrepExt_CancelUseEvent(char *buffer, int CreatureID);
int PrepExt_ActorJump(char *buffer, int actor);
int PrepExt_RemoveCreature(char *buffer, int actorID);
int PrepExt_SendInfoMessage(char *buffer, const char *message, unsigned char eventID);
int PrepExt_SendFallDamage(char *buffer, int damage);
int PrepExt_GenericChatMessage(char *buffer, int creatureID, const char *name, const char *channel, const char *message);
int PrepExt_FriendsAdd(char *buffer, CharacterData *charData);
int PrepExt_FriendsLogStatus(char *buffer, CharacterData *charData, int logStatus);

int AddItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
int RemoveItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);

//Specific stat updates
int PrepExt_SendHealth(char *buffer, long CreatureID, int healthAmount);
int PrepExt_SendSpecificStats(char *buffer, CreatureInstance *cInst, vector<short> &statList);
int PrepExt_SendSpecificStats(char *buffer, CreatureInstance *cInst, const short *statList, int statCount);
int PrepExt_UpdateMods(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateOrbs(char *buffer, CreatureInstance *cInst);
int PrepExt_SendEqAppearance(char *buffer, int creatureDefID, const char *eqAppearance);
int PrepExt_UpdateEquipStats(char *buffer, CreatureInstance *cInst);
int PrepExt_SendVisWeapon(char *buffer, int CreatureID, short visWeapon);
int PrepExt_CooldownExpired(char *buffer, long actor, const char *cooldownCategory);
int PrepExt_ChangeTarget(char *buffer, int sourceID, int targetID);
int PrepExt_ExperienceGain(char *buffer, int CreatureID, int ExpAmount);
int PrepExt_SendValour(char *buffer, int CreatureID, int ValourAmount, int GuildDefID, int rank, int title);
int PrepExt_SendExperience(char *buffer, int CreatureID, int ExpAmount);
int PrepExt_QueryResponseNull(char *buffer, int queryIndex);
int PrepExt_QueryResponseString(char *buffer, int queryIndex, const char *strData);
int PrepExt_QueryResponseString2(char *buffer, int queryIndex, const char *strData1, const char *strData2);
int PrepExt_QueryResponseStringList(char *buffer, int queryIndex, const STRINGLIST &strData);
int PrepExt_QueryResponseMultiString(char *buffer, int queryIndex, const MULTISTRING &strData);
int PrepExt_QueryResponseError(char *buffer, int queryIndex, const char *message);
int PrepExt_SendEffect(char *buffer, int sourceID, const char *effectName, int targetID);
int PrepExt_SendHeartbeatMessage(char *buffer, unsigned int elapsedMilliseconds);
int PrepExt_SendAdvancedEmote(char *buffer, int creatureID, const char *emoteName, float emoteSpeed, int loop);
int PrepExt_SendEmoteControl(char *buffer, int creatureID, int emoteEvent);
int PrepExt_ModStopSwimFlag(char *buffer);
int PrepExt_TradeCurrencyOffer(char *buffer, int offeringPlayerID, int tradeAmount);
int PrepExt_TradeItemOffer(char *buffer, char *convBuf, int offeringPlayerID, std::vector<InventorySlot>& itemList);
int PrepExt_QuestCompleteMessage(char *buffer, int questID, int objectiveIndex);
int PrepExt_CreatureEventPortalRequest(char *buffer, int actorID, const char *casterName, const char *locationName);
int PrepExt_CreatureEventVaultSize(char *buffer, int actorID, int vaultSize);
int PrepExt_SendEnvironmentUpdateMsg(char *buffer, const char *zoneIDString, ZoneDefInfo *zoneDef, int x, int z);
int PrepExt_SendTimeOfDayMsg(char *buffer, const char *envType);
int PrepExt_Broadcast(char *buffer, const char *message);

int SendToAllSimulator(char *buffer, int length, int ignoreIndex);
int SendToOneSimulator(char *buffer, int length, int simIndex);
int SendToOneSimulator(char *buffer, int length, SimulatorThread *simPtr);
int SendToFriendSimulator(char *buffer, int length, int CDefID);
int randint(int min, int max);
int randmod(int max);
int randi(int max);
int randmodrng(int min, int max);
double randdbl(double min, double max);

char *StringFromInt(char *buffer, int value);
char *StringFromFloat(char *buffer, double value);
char *StringFromBool(char *buffer, bool value);
char *StringFromBool(char *buffer, int value);

namespace Util
{
	void WriteString(FILE *output, const char *label, std::string &str);
	void WriteString(FILE *output, const char *label, const char *str);
	void WriteInteger(FILE *output, const char *label, int value);
	void WriteIntegerIfNot(FILE *output, const char *label, int value, int ignoreVal);
	void WriteAutoSaveHeader(FILE *output);
	FILE * OpenSaveFile(const char *filename);
	int Split(const std::string &source, const char *delim, std::vector<std::string> &dest);
	void Join(std::vector<std::string> &source, const char *delim, std::string &dest);
	void Replace(std::string &source, char find, char replace);
	void SafeCopy(char *dest, const char *source, int destSize);
	void SafeCopyN(char *dest, const char *source, int destSize, int copySize);
	int IsStringTerminated(const char *buffer, int bufferSize);
	int SafeFormat(char *destBuf, size_t maxCount, const char *format, ...);
	int SafeFormatArg(char *destBuf, size_t maxCount, const char *format, va_list argList);
	void StringAppendInt(std::string &dest, int value);
	int ClipInt(int value, int min, int max);
	int ClipIntMin(int value, int min);
	float ClipFloat(float value, float min, float max);
	float Round(float value);
	int QuaternionToByteFacing(double X, double Y, double Z, double W);
	void ClearString(char *buffer, int size);
	char* FormatTime(char *outBuf, int bufSize, int seconds);
	void WriteIntegerList(FILE *output, const char* label, std::vector<int>& dataList);
	bool DoubleEquivalent(double left, double right);
	bool FloatEquivalent(float left, float right);
	int GetAdditiveFromIntegralPercent100(int value, int multiplier);
	int GetAdditiveFromIntegralPercent1000(int value, int multiplier);
	int GetAdditiveFromIntegralPercent10000(int value, int multiplier);
	bool IntToBool(int value);
	void SanitizeClientString(char *string);
	void RemoveStringsFrom(const char *search, std::string& operativeString);
	void ToLowerCase(std::string &input);
	bool HasBeginning(std::string const &fullString, std::string const &ending);
	bool HasEnding (std::string const &fullString, std::string const &ending);
	void TrimWhitespace(std::string &modify);
	float StringToFloat(const std::string &str);
	int ParseDate(const std::string &str, time_t &time);
	std::string FormatDate(time_t *time);
	void ReplaceAll(std::string& str, const std::string& from, const std::string& to);

	int GetInteger(const STRINGLIST &strList, size_t index);
	int GetInteger(const std::string &str);
	float GetFloat(const char *value);
	float GetFloat(const STRINGLIST &strList, size_t index);
	float GetFloat(const std::string &str);
	const char *GetString(const STRINGLIST &strList, size_t index);
	const char *GetSafeString(const STRINGLIST &strList, size_t index);

	void AssignFloatArrayFromStringSplit(float *arrayDest, size_t arraySize, const std::string &strData);
	void TokenizeByWhitespace(const std::string &input, STRINGLIST &output);
}

/*
class StringWrite
{
public:
	StringWrite(char *buffer, int size);
	~StringWrite();

	int PutChar(char value);
	int Format(const char *format, ...);
	int FormatPos(const char *format, ...);
	bool Overflow(void);

	unsigned int writePos;
private:
	static const int OVERFLOW_WARN = 1;
	bool overflow;

	char *writeBuf;
	unsigned int bufSize;
	unsigned int maxSafeLen;
};
*/

struct ChangeData
{
	ChangeData();
	int PendingChanges;
	unsigned long LastChange;    //The time of the most recent change.
	unsigned long FirstChange;   //The time of the first pending change. 
	void AddChange(void);
	bool IsLastChangeSince(unsigned long milliseconds);
	bool CheckUpdateAndClear(unsigned long milliseconds);
	void ClearPending(void);
};

extern ChangeData SessionVarsChangeData;

#endif //UTIL_H
