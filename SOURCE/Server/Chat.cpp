#include <time.h>
#include "Chat.h"
#include "RotatingList.h"
#include "Simulator.h"
#include "ByteBuffer.h"

ChatManager g_ChatManager;
bool NewChat = false;

ChannelCompare ValidChatChannel[] = {
	{CHAT_SCOPE_NONE,  "NULL", NULL, "NULL", true },
	{CHAT_SCOPE_LOCAL,  "s", "[Say]", "/say", true },
	{CHAT_SCOPE_LOCAL,  "emote", "[Emote]", "/emote", true },
	{CHAT_SCOPE_SERVER, "gm/earthsages", "[Earthsage]", "/gm", true },
	{CHAT_SCOPE_SERVER, "*SysChat", "[SysChat]", "/syschat", true },
	{CHAT_SCOPE_SERVER,  "rc/", "[Region]", "/region", true },
	{CHAT_SCOPE_SERVER,  "tc/", "[Trade]", "/trade", true },
	{CHAT_SCOPE_CHANNEL, "ch/", "[Channel]", "/ch", true },
	{CHAT_SCOPE_FRIEND,  "clan", "[Clan]", "/clan", true },
	{CHAT_SCOPE_PARTY,  "party", "[Party]", "/party", true },

	//These only show the message.  The name is used but there is no "says: " dividing them.
	{CHAT_SCOPE_LOCAL, "friends", "[Friends]", NULL, false },
	{CHAT_SCOPE_LOCAL, "mci", "[My incoming damage]", "Incoming Dmg", false },
	{CHAT_SCOPE_LOCAL, "mco", "[My outgoing damage]", "Outgoing Dmg", false },
	{CHAT_SCOPE_LOCAL, "oci", "[OtherPlayer incoming damage]", "Other Incoming", false },
	{CHAT_SCOPE_LOCAL, "oco", "[OtherPlayer outgoing damage]", "Other Outgoing", false },

	{CHAT_SCOPE_LOCAL, "conGrey", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conDarkGreen", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conLightGreen", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conBrightGreen", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conWhite", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conBlue", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conYellow", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conOrange", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conRed", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conPurple", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "conBrightPurple", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "sys/info", NULL, NULL, false },
	{CHAT_SCOPE_LOCAL, "err/error", NULL, NULL, false },
};
const int NumValidChatChannel = sizeof(ValidChatChannel) / sizeof(ChannelCompare);

const ChannelCompare * GetChatInfoByChannel(const char *channel)
{
	//First entry is null
	for(int a = 1; a < NumValidChatChannel; a++)
		if(strcmp(ValidChatChannel[a].channel, channel) == 0)
			return &ValidChatChannel[a];
	
	return &ValidChatChannel[0];
}

const int LOCAL_CHAT_RANGE = 750;

RotatingList<int> ChatLogIndex(100);
vector<string> ChatLogString(100);

//
// ChatManager - Handles external logging of chat and a circular chat buffer for use by external services
//

ChatManager :: ChatManager()
{
	cs.Init();
	cs.SetDebugName("CS_ZONEDEFMGR");
	m_ChatLogFile = NULL;
}

ChatManager :: ~ChatManager()
{
	CircularChatBuffer.clear();
}

int ChatManager :: handleCommunicationMsg(char *channel, char *message, char *name)
{
	//Returns the number of bytes composing the message data.

	//Mostly copied from the Simulator function with the same name, but without permissions
	//checks.  To be used internally by the server when it needs to send arbitrary messages
	//to clients.  Should not be used for /tell messages.

	//Handles a "communicate" (0x04) message containing a channel and string
	//Sends back a "_handleCommunicationMsg" (50 = 0x32) to the client
	//Ideally this function should broadcast the message to all active player threads
	//in the future so that all players can receive the communication message.
	
	char ComBuf[1024];
	char LogBuffer[1024];
	
	if(strlen(channel) == 0 || strlen(message) == 0)
		return 0;

	//LogMessageL("[%s]:[%s]", channel, message);

	/*
	bool log = false;
	char *prefix = NULL;
	if(strcmp(channel, "s") == 0)
	{
		log = true;
		prefix = ChatPrefix_Say;
	}
	else if(strcmp(channel, "gm/earthsages") == 0)
	{
		log = true;
		prefix = ChatPrefix_GM;
	}
	else if(strcmp(channel, "*SysChat") == 0)
	{
		log = true;
		prefix = ChatPrefix_SysChat;
	}
	else if(strcmp(channel, "rc/") == 0)
	{
		log = true;
		prefix = ChatPrefix_Region;
	}

	if(log == true)
	{
		if(prefix != NULL)
			sprintf(LogBuffer, "%s %s: %s", prefix, name, message);
		else
			sprintf(LogBuffer, "%s: %s", name, message);
		LogChatMessage(LogBuffer);
	}
	else
	{
		sprintf(LogBuffer, "%s: %s", name, message);
		LogChatMessage(LogBuffer);
	}*/

	int found = 0;
	int a;
	for(a = 1; a < NumValidChatChannel; a++)
	{
		if(strcmp(ValidChatChannel[a].channel, channel) == 0)
		{
			found = a;
			break;
		}
	}

	int wpos = 0;
	if(ValidChatChannel[found].prefix != NULL)
		wpos += sprintf(&LogBuffer[wpos], "%s ", ValidChatChannel[found].prefix);
	if(ValidChatChannel[found].name == true)
		wpos += sprintf(&LogBuffer[wpos], "%s: ", name);
	else
		name[0] = 0;   //Disable name from showing in the data buffer

	wpos += sprintf(&LogBuffer[wpos], "%s", message);
	LogChatMessage(LogBuffer);

	wpos = 0;
	wpos += PutByte(&ComBuf[wpos], 0x32);       //_handleCommunicationMsg
	wpos += PutShort(&ComBuf[wpos], 0);         //Placeholder for size

	wpos += PutInteger(&ComBuf[wpos], 0);    //Character ID who's sending the message
	wpos += PutStringUTF(&ComBuf[wpos], name);  //Character name
	wpos += PutStringUTF(&ComBuf[wpos], channel);  //return the channel type
	wpos += PutStringUTF(&ComBuf[wpos], message);  //return the message

	PutShort(&ComBuf[1], wpos - 3);     //Set size

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true)
			it->AttemptSend(ComBuf, wpos);
	}
	return wpos;
}

void ChatManager :: OpenChatLogFile(const char *filename)
{
	m_ChatLogFile = fopen(filename, "a");
}

void ChatManager :: CloseChatLogFile(void)
{
	if(m_ChatLogFile != NULL)
	{
		fclose(m_ChatLogFile);
		m_ChatLogFile = NULL;
	}
}

void ChatManager :: FlushChatLogFile(void)
{
	if(m_ChatLogFile != NULL)
		fflush(m_ChatLogFile);
}

void ChatManager :: LogChatMessage(const char *messageStr)
{
	cs.Enter("ChatManager::LogChatMessage");
	ChatMessage cm;
	cm.mTime = g_ServerTime;
	cm.mMessage = messageStr;
	CircularChatBuffer.push_back(cm);
	while(CircularChatBuffer.size() > MAX_CHAT_BUFFER_SIZE)
		CircularChatBuffer.pop_front();
	cs.Leave();

	static char timeBuf[256];
	if(m_ChatLogFile != NULL)
	{
		time_t curtime;
		time(&curtime);
		strftime(timeBuf, sizeof(timeBuf), "%x %X", localtime(&curtime));
		fprintf(m_ChatLogFile, "%s : %s\r\n", timeBuf, messageStr);
	}
}

