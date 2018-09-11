#include <time.h>
#include "Chat.h"
#include "ChatChannel.h"
#include "RotatingList.h"
#include "Simulator.h"
#include "ByteBuffer.h"
#include "FriendStatus.h"
#include "Instance.h"
#include "Clan.h"
#include "Config.h"
#include "Cluster.h"
#include "http/SiteClient.h"
#include "util/Log.h"

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
	{CHAT_SCOPE_CLAN,  "clan", "[Clan]", "/clan", true },
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
// ChatMessage
//
ChatMessage::ChatMessage() {
	mMessage = "";
	mChannel = &ValidChatChannel[0];
	mSender = "";
	mSenderCreatureDefID = -1;
	mSenderCreatureID = -1;
	mTell = false;
	mRecipient = "";
	mSendingInstance = -1;
	mTime = time(NULL);
	mChannelName = "";
	mSimulatorID = -1;
	mSenderClanID = 0;
}


ChatMessage::ChatMessage(std::string msg)
{
	mMessage = msg;
	mChannel = &ValidChatChannel[0];
	mSender = "";
	mSenderCreatureDefID = -1;
	mSenderCreatureID = -1;
	mTell = false;
	mRecipient = "";
	mSendingInstance = -1;
	mTime = time(NULL);
	mChannelName = "";
	mSimulatorID = -1;
	mSenderClanID = 0;
}

ChatMessage::ChatMessage(const ChatMessage &msg)
{

	mMessage = msg.mMessage;
	mChannel = msg.mChannel;
	mSender = msg.mSender;
	mSenderCreatureDefID = msg.mSenderCreatureDefID;
	mSenderCreatureID = msg.mSenderCreatureID;
	mTell = msg.mTell;
	mRecipient = msg.mRecipient;
	mSendingInstance = msg.mSendingInstance;
	mTime = msg.mTime;
	mChannelName = msg.mChannelName;
	mSimulatorID = msg.mSimulatorID;
	mSenderClanID = 0;
}
ChatMessage::ChatMessage(CharacterServerData *pld)
{
	mMessage = "";
	mChannel = &ValidChatChannel[0];
	mSender = pld->charPtr->cdef.css.display_name;
	mSenderCreatureDefID = pld->CreatureDefID;
	mSenderCreatureID = pld->CreatureID;
	mTell = false;
	mRecipient = "";
	mSendingInstance = pld->CurrentInstanceID;
	mTime = time(NULL);
	mChannelName = "";
	mSimulatorID = -1;
	mSenderClanID = 0;
}

void ChatMessage::WriteToJSON(Json::Value &value) {

	struct tm * timeinfo;
	time_t tt = mTime;
	timeinfo = localtime(&tt);
	char tbuf[64];
	strftime(tbuf, sizeof(tbuf), "%m/%d %H:%M", timeinfo);

	value["message"] = mMessage;
	value["channelName"] = mChannelName;
	value["senderCreatureDefID"] = mSenderCreatureDefID;
	value["senderCreatureID"] = mSenderCreatureID;
	value["recipient"] = mRecipient;
	value["sendingInstance"] = mSendingInstance;
	value["simulatorID"] = mSimulatorID;
	value["tell"] = mTell;
	value["time"] = Json::UInt64(mTime);
	value["timeReadable"] = tbuf;
	value["sender"] = mSender;
	if(mChannel != NULL) {
		Json::Value ch;

		ch["scope"] = mChannel->chatScope;
		ch["channel"] = mChannel->channel;
		if(mChannel->prefix != NULL)
			ch["prefix"] = mChannel->prefix;
		if(mChannel->friendly != NULL)
			ch["friendly"] = mChannel->friendly;
		ch["name"] = mChannel->name;

		value["channel"] = ch;
	}
}

//
// ChatManager - Handles external logging of chat and a circular chat buffer for use by external services
//

ChatManager :: ChatManager()
{
	cs.Init();
	cs.SetDebugName("CS_ZONEDEFMGR");
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
	ChatMessage cm(message);
	LogChatMessage(cm);

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

void ChatManager :: LogMessage(std::string message)
{
	g_Logs.chat->info(message);
}

bool ChatManager ::SendChatMessageAsOffline(ChatMessage &message, HTTPD::SiteSession *session) {
	/* Look up the account name for the character */
	int cdefID = g_UsedNameDatabase.GetIDByName(message.mRecipient);
	if(cdefID == -1) {
		g_Logs.server->error("No such character \"%v\" .", message.mRecipient.c_str());
	}
	else {
		CharacterData *cd = g_CharacterManager.RequestCharacter(cdefID, true);
		if(cd == NULL || cd->AccountID < 1) {
			g_Logs.server->error("Could not find creature definition for \"%v\" (%v).", message.mRecipient.c_str(), cdefID);
		}
		else {
			AccountData *data = g_AccountManager.FetchIndividualAccount(cd->AccountID);
			if(data == NULL) {
				g_Logs.server->error("Could not find account for \"%v\" (%v, %v).", message.mRecipient.c_str(), cdefID, cd->AccountID);
			}
			else {
				char subject[256];
				Util::SafeFormat(subject, sizeof(subject), "In-game private message for %s from %s", message.mRecipient.c_str(), message.mSender.c_str());

				SiteClient siteClient(g_Config.ServiceAuthURL);
				if(siteClient.sendPrivateMessage(session, data->Name, subject, message.mMessage)) {
					return true;
				}
				else {
					g_Logs.server->warn("Player \"%v\" is not logged in and no private message sent.", message.mRecipient.c_str());
				}
			}
		}
	}
	return false;
}

bool ChatManager::SendChatMessage(ChatMessage &message, CreatureInstance *sendingCreatureInstance) {
	bool ok = DeliverChatMessage(message, sendingCreatureInstance);
	if(ok) {
		g_ClusterManager.Chat(message);
	}
	else {
		/* If the message was a /tell for a player on another shard, pass it on */
		if(message.mTell && g_ClusterManager.IsPlayerOnOtherShard(message.mRecipient)) {
			g_ClusterManager.Chat(message);
			ok = true;
		}
	}
	return ok;
}

bool ChatManager::DeliverChatMessage(ChatMessage &message, CreatureInstance *sendingCreatureInstance) {

	message.mTime =  time(NULL);

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 50);       //_handleCommunicationMsg
	wpos += PutShort(&SendBuf[wpos], 0);         //Placeholder for size
	wpos += PutInteger(&SendBuf[wpos], message.mSenderCreatureID);    //Character ID who's sending the message
	wpos += PutStringUTF(&SendBuf[wpos], message.mSender.c_str()); //pld.charPtr->cdef.css.display_name);  //Character name
	wpos += PutStringUTF(&SendBuf[wpos], message.mTell ? "t/" : message.mChannel->channel);
	wpos += PutStringUTF(&SendBuf[wpos], message.mMessage.c_str());
	PutShort(&SendBuf[1], wpos - 3);     //Set size

	bool found = false;
	bool log = !message.mTell && ( message.mChannel->chatScope == CHAT_SCOPE_REGION || message.mChannel->chatScope == CHAT_SCOPE_SERVER );

	bool breakLoop = false;
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(breakLoop == true)
			break;

		if(it->ProtocolState == 0)
		{
			//LogMessageL(MSG_ERROR, "[WARNING] Cannot not send chat to lobby protocol simulator");
			continue;
		}

		if(it->isConnected == false)
			continue;

		if(it->pld.charPtr == NULL)
			continue;

		bool send = false;
		if(message.mTell == true)
		{
			if(strcmp(it->pld.charPtr->cdef.css.display_name, message.mRecipient.c_str()) == 0)
			{
				send = true;
				found = true;
			}
		}
		else
		{
			switch(message.mChannel->chatScope)
			{
			case CHAT_SCOPE_LOCAL:
				if(sendingCreatureInstance == NULL || it->pld.CurrentInstanceID != message.mSendingInstance)
					break;
				if(ActiveInstance::GetPlaneRange(it->creatureInst, sendingCreatureInstance, LOCAL_CHAT_RANGE) > LOCAL_CHAT_RANGE)
					break;

				send = true;
				break;

			case CHAT_SCOPE_REGION:
				if(it->pld.CurrentInstanceID != message.mSendingInstance)
					break;
				send = true;
				break;
			case CHAT_SCOPE_SERVER:
				send = true;

				if(message.mChannelName.compare("gm/earthsages") == 0 && !it->pld.accPtr->HasPermission(Perm_Account, Permission_GMChat) && !it->pld.accPtr->HasPermission(Perm_Account, Permission_Admin)) {
					send = false;
				}

				break;
			case CHAT_SCOPE_FRIEND:
				if(it->pld.CreatureDefID == message.mSenderCreatureDefID)  //Send to self
					send = true;
				else if(g_FriendListManager.IsMutualFriendship(it->pld.CreatureDefID, message.mSenderCreatureDefID) ==true)
					send = true;
				break;
			case CHAT_SCOPE_CHANNEL:
			{
				PrivateChannel *privateChannelData = g_ChatChannelManager.GetChannelForMessage(message.mSimulatorID, message.mChannelName.c_str() + 3);
				if(privateChannelData != NULL) {
					for(size_t i = 0; i < privateChannelData->mMemberList.size(); i++)
						if(it->InternalID == privateChannelData->mMemberList[i].mSimulatorID)
							send = true;
				}
				break;
			}
			case CHAT_SCOPE_PARTY:
				if(sendingCreatureInstance == NULL)
					break;
				g_PartyManager.BroadCastPacket(sendingCreatureInstance->PartyID, sendingCreatureInstance->CreatureDefID, SendBuf, wpos);
				breakLoop = true;
				break;
			case CHAT_SCOPE_CLAN:
			{

				Clans::Clan c = g_ClanManager.GetClan(message.mSenderClanID);
				if(it->pld.CreatureDefID == message.mSenderCreatureDefID)  //Send to self
					send = true;
				else {
					if(c.mId > 0 && it->pld.charPtr->clan == c.mId)
						send = true;
					else if(g_GuildManager.IsMutualGuild(it->pld.CreatureDefID, message.mSenderCreatureDefID) ==true)
						send = true;
				}
				break;
			}
			default:
				break;
			}
		}

		if(send == true) {
			found = true;
			it->AttemptSend(SendBuf, wpos);
		}
	}

	if(log == true)
		LogChatMessage(message);

	return found;
}

void ChatManager :: LogChatMessage(ChatMessage &message)
{
	cs.Enter("ChatManager::LogChatMessage");
	CircularChatBuffer.push_back(message);
	while(CircularChatBuffer.size() >= MAX_CHAT_BUFFER_SIZE - 1)
		CircularChatBuffer.pop_front();
	cs.Leave();
	// TODO if(message.mChannel->chatScope == ChatBroadcastRange::CHAT_SCOPE_NONE)
	if(message.mChannel->chatScope == 0)
		g_Logs.chat->info("%v: %v", message.mSender.c_str(), message.mMessage.c_str());
	else
		g_Logs.chat->info("%v %v: %v", message.mChannel->prefix, message.mSender.c_str(), message.mMessage.c_str());
}
