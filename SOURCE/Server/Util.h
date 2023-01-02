#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <chrono>

#include "CommonTypes.h"
#include "Books.h"
#include "Forms.h"
#include "Achievements.h"

#define DAY_MS 86400000
#define HOUR_MS 3600000
#define MINUTE_MS 60000
#define SECOND_MS 1000

using namespace std;

#ifdef _WIN32
void SetNativeThreadName(uint32_t dwThreadID, const char* threadName);
void SetNativeThreadName( const char* threadName);
void SetNativeThreadName( thread* thread, const char* threadName);
#elif defined(__linux__)
void SetNativeThreadName( const char* threadName);
#else
void SetNativeThreadName(thread* thread, const char* threadName);
#endif

const int MODMESSAGE_EVENT_SUPERCRIT = 1;
const int MODMESSAGE_EVENT_EMOTE = 2;
const int MODMESSAGE_EVENT_EMOTE_CONTROL = 3;
const int MODMESSAGE_EVENT_PING_START = 20;
const int MODMESSAGE_EVENT_PING_STOP = 21;
const int MODMESSAGE_EVENT_PING_QUERY = 22;
const int MODMESSAGE_EVENT_GENERIC_REQUEST = 23;
const int MODMESSAGE_EVENT_STOP_SWIM = 40;
const int MODMESSAGE_EVENT_START_SWIM = 41;
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


int PrepExt_SetAvatar(char *buffer, int creatureID);
int PrepExt_SetTimeOfDay(char *buffer, const char *timeOfDay);
int PrepExt_SetWeather(char *buffer, string type, int weight);
int PrepExt_Thunder(char *buffer, int weight);
int PrepExt_AbilityEvent(char *buffer, int creatureID, int abilityID, int abilityEvent);
int PrepExt_CancelUseEvent(char *buffer, int CreatureID);
int PrepExt_ActorJump(char *buffer, int actor);
int PrepExt_RemoveCreature(char *buffer, int actorID);
int PrepExt_Damage(char *buffer, int actorID, const char *damageString, const char *ability, int critical, int absorbed);
int PrepExt_SendInfoMessage(char *buffer, const char *message, unsigned char eventID);
int PrepExt_SendFallDamage(char *buffer, int damage);
int PrepExt_GenericChatMessage(char *buffer, int creatureID, const char *name, const char *channel, const char *message);

//Specific stat updates
int PrepExt_Achievement(char *buffer, int creatureID, const char *achievement, const char *scoreSpec);
int PrepExt_SendFormOpen(char *buffer, FormDefinition form);
int PrepExt_SendFormClose(char *buffer, int formId);
int PrepExt_SendBookOpen(char *buffer, int bookID, int page, int op);
int PrepExt_SendUICommand(char *buffer, const char *op, const char *window);
int PrepExt_Refashion(char *buffer);
int PrepExt_Craft(char *buffer);
int PrepExt_CooldownExpired(char *buffer, long actor, const char *cooldownCategory);
int PrepExt_ChangeTarget(char *buffer, int sourceID, int targetID);
int PrepExt_ExperienceGain(char *buffer, int CreatureID, int ExpAmount);
int PrepExt_SendValour(char *buffer, int CreatureID, int ValourAmount, int GuildDefID, int rank, int title);
int PrepExt_QueryResponseNull(char *buffer, int queryIndex);
int PrepExt_QueryResponseString(char *buffer, int queryIndex, const string &strData);
int PrepExt_QueryResponseString(char *buffer, int queryIndex, const char *strData);
int PrepExt_QueryResponseString2(char *buffer, int queryIndex, const char *strData1, const char *strData2);
int PrepExt_QueryResponseStringList(char *buffer, int queryIndex, const STRINGLIST &strData);
int PrepExt_QueryResponseStringRows(char *buffer, int queryIndex, const STRINGLIST &strData);
int PrepExt_QueryResponseMultiString(char *buffer, int queryIndex, const MULTISTRING &strData);
int PrepExt_QueryResponseError(char *buffer, int queryIndex, const char *message);
int PrepExt_SendEffect(char *buffer, int sourceID, const char *effectName, int targetID);
int PrepExt_SendHeartbeatMessage(char *buffer, unsigned long elapsedMilliseconds);
int PrepExt_SendAdvancedEmote(char *buffer, int creatureID, const char *emoteName, float emoteSpeed, int loop);
int PrepExt_SendEmoteControl(char *buffer, int creatureID, int emoteEvent);
int PrepExt_ModStopSwimFlag(char *buffer, bool swim);
int PrepExt_TradeCurrencyOffer(char *buffer, int offeringPlayerID, int tradeAmount);
int PrepExt_CreatureEventPortalRequest(char *buffer, int actorID, const char *casterName, const char *locationName);
int PrepExt_CreatureEventVaultSize(char *buffer, int actorID, int vaultSize, int deliverySlots);
int PrepExt_Broadcast(char *buffer, const char *message);
int PrepExt_Info(char *buffer, const char *message, char eventID);
int PrepExt_Chat(char *buffer, int characterID, const char *display_name, const char *channel, const char *message);

string StringFromBool(bool value);

namespace Util
{
	void WriteString(FILE *output, const char *label, string &str);
	void WriteString(FILE *output, const char *label, const char *str);
	void WriteInteger(FILE *output, const char *label, int value);
	void WriteIntegerIfNot(FILE *output, const char *label, int value, int ignoreVal);
	void WriteAutoSaveHeader(FILE *output);
	FILE * OpenSaveFile(const char *filename);

	string StripLeadingTrailing(const string &source, const char *delim);
	string Unescape(const string &source);
	int SplitEscaped(const string &source, const char *delim, vector<string> &dest);
	int Split(const string &source, const char *delim, vector<string> &dest);
	void Join(vector<string> &source, const char *delim, string &dest);
	void Replace(string &source, char find, char replace);
	void SafeCopy(char *dest, const char *source, int destSize);
	void SafeCopyN(char *dest, const char *source, int destSize, int copySize);
	int IsStringTerminated(const char *buffer, int bufferSize);
	int SafeFormat(char *destBuf, size_t maxCount, const char *format, ...);
	int SafeFormatArg(char *destBuf, size_t maxCount, const char *format, va_list argList);
	void StringAppendInt(string &dest, int value);
	int ClipInt(int value, int min, int max);
	int ClipIntMin(int value, int min);
	float ClipFloat(float value, float min, float max);
	float Round(float value);
	int QuaternionToByteFacing(double X, double Y, double Z, double W);
	void ClearString(char *buffer, int size);
	char* FormatTime(char *outBuf, int bufSize, int seconds);
	void WriteIntegerList(FILE *output, const char* label, vector<int>& dataList);
	bool DoubleEquivalent(double left, double right);
	bool FloatEquivalent(float left, float right);
	int GetAdditiveFromIntegralPercent100(int value, int multiplier);
	int GetAdditiveFromIntegralPercent1000(int value, int multiplier);
	int GetAdditiveFromIntegralPercent10000(int value, int multiplier);
	bool IntToBool(int value);
	void SanitizeClientString(char *string);
	void RemoveStringsFrom(const string &search, string& operativeString);
	bool CaseInsensitiveStringCompare(const string& str1, const string& str2);
	bool CaseInsensitiveStringFind(const string& str1, const string& str2);
	void ToLowerCase(string &input);
	string LowerCase(const string& in);
	string UpperCase(const string& in);
	string URLDecode(string const &src);
	bool HasBeginning(string const &fullString, string const &ending);
	bool HasEnding (string const &fullString, string const &ending);
	void TrimWhitespace(string &modify);
	float StringToFloat(const string &str);
	int ParseDate(const string &str, time_t &time);
	string ReplaceAllTo(const string str, const string& from,
			const string& to);
	void EncodeHTML(string& data);
	string EncodeJSONString(string &str);
	string FormatDate(time_t *time);
	string FormatDateTime(time_t *time);
	string FormatTimeOfDay(time_t *time);
	string FormatTimeOfDayMS(unsigned long);
	string CaptureCommand(string cmd);
	void ReplaceAll(string& str, const string& from, const string& to);
	void URLDecode(string &str);
	void URLEncode(string &str);
	string &LTrim(string &s);
	string &RTrim(string &s);
	string &Trim(string &s);

	int GetInteger(const STRINGLIST &strList, size_t index);
	int GetInteger(const string &str);
	float GetFloat(const char *value);
	float GetFloat(const STRINGLIST &strList, size_t index);
	float GetFloat(const string &str);
	const char *GetString(const STRINGLIST &strList, size_t index);
	const char *GetSafeString(const STRINGLIST &strList, size_t index);

	void AssignFloatArrayFromStringSplit(float *arrayDest, size_t arraySize, const string &strData);
	void TokenizeByWhitespace(const string &input, STRINGLIST &output);
	string RandomStr(unsigned int size, bool all);
	string RandomStrFrom(unsigned int size, string from);
	string RandomHexStr(unsigned int size);

	float RadianToRotation(float radians);
	float RotationToRadians(unsigned int rotation);
	unsigned char DistanceToRotationByte(int xlen, int zlen);
	unsigned char RadianToRotationByte(float radians);

	time_t ToTimeT(fs::file_time_type &tp);
    string RightOf(const string &path, const string &delim);

	int SafeParseInt(const string& str);
	int SafeParseInt(const string& str, int defaultValue);

	unsigned long ParseTimeHHMM(const string& timeString);
	unsigned long ParseTimeHHMMSS(const string& timeString);

	string FormatTimeHHMM(unsigned long ms);
	string FormatTimeHHMMSS(unsigned long ms);
	string FormatTimeHHMMSSmm(unsigned long ms);
	string Format(const string &fmt_str, ...);
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
