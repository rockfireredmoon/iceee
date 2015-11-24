// Handles chat messages for logging or redirection

#pragma once

#ifndef CHAT_H
#define CHAT_H

#include <vector>
#include <string>
#include <deque>
#include "RotatingList.h"
#include "StringList.h"
#include "Creature.h"
#include "ActiveCharacter.h"
#include "json/json.h"
#include "http/HTTP.h"

using namespace std;

extern char ChatPrefix_SysChat[];
extern char ChatPrefix_GM[];
extern char ChatPrefix_Region[];
extern char ChatPrefix_Say[];

extern RotatingList<int> ChatLogIndex;
extern vector<string> ChatLogString;

enum ChatBroadcastRange
{
	CHAT_SCOPE_NONE    = 0,  //No broadcast.
	CHAT_SCOPE_LOCAL      ,  //Broadcast only to local players.
	CHAT_SCOPE_REGION     ,  //Broadcast to all players in a region.
	CHAT_SCOPE_SERVER     ,  //Broadcast to all players on the server.
	CHAT_SCOPE_CLAN       ,  //Broadcast only to clan member.
	CHAT_SCOPE_FRIEND     ,  //Broadcast only to friends.
	CHAT_SCOPE_PARTY      ,  //Broadcast only to party members.
	CHAT_SCOPE_CHANNEL       //Broadcast only to a private channel (special restrictions apply)
};

struct ChannelCompare
{
	int chatScope;          //An enum type (ChatBroadcastRange)
	const char *channel;    //internal name of the channel
	const char *prefix;     //prefix string to show in logging ([Region], [Clan], etc)
	const char *friendly;   //User-friendly name to show in window controls (/say, /region, etc)
	bool name;        //Whether or not the name is included in the message string
};

extern const int LOCAL_CHAT_RANGE;

extern const int NumValidChatChannel;
extern ChannelCompare ValidChatChannel[];

const ChannelCompare * GetChatInfoByChannel(const char *channel);

extern bool NewChat;


class ChatMessage
{
public:
	ChatMessage();
	ChatMessage(std::string message);
	ChatMessage(CharacterServerData *pld);
	ChatMessage(const ChatMessage &msg);

	unsigned long mTime;
	std::string mMessage;
	int mSenderCreatureID;
	int mSenderCreatureDefID;
	int mSenderClanID;
	int mSendingInstance;
	std::string mSender;
	std::string mChannelName;
	int mSimulatorID;
	bool mTell;
	std::string mRecipient;
	const ChannelCompare *mChannel;

	void WriteToJSON(Json::Value &value);
};

class ChatManager
{
public:
	ChatManager();
	~ChatManager();

	deque<ChatMessage> CircularChatBuffer;

	void CloseChatLogFile(void);
	void FlushChatLogFile(void);
	void OpenChatLogFile(const char *filename);
	bool SendChatMessageAsOffline(ChatMessage &message, HTTPD::SiteSession *session);
	bool SendChatMessage(ChatMessage &message, CreatureInstance *sendingCreatureInstance);
	void LogChatMessage(ChatMessage &message);
	void LogMessage(std::string message);
	int handleCommunicationMsg(char *channel, char *message, char *name);
	Platform_CriticalSection cs;  //Needed for circular chat buffer which may be inserted into by many threads

private:
	static const int MAX_CHAT_BUFFER_SIZE = 100;

	FILE* m_ChatLogFile;
	char SendBuf[24576];     //Holds data that is being prepared for sending
	char LogBuffer[1024];
};

extern ChatManager g_ChatManager;

#endif //CHAT_H
