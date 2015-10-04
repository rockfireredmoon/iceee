#include "ChatChannel.h"
#include "string.h"

ChatChannelManager g_ChatChannelManager;



PrivateChannel :: PrivateChannel()
{
	mID = 0;
}


// Return true if the simulator was found and removed.
bool PrivateChannel :: RemoveUser(int SimulatorID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mSimulatorID == SimulatorID)
		{
			mMemberList.erase(mMemberList.begin() + i);
			return true;
		}
	}
	return false;
}

bool PrivateChannel :: HasSimulator(int SimulatorID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
		if(mMemberList[i].mSimulatorID == SimulatorID)
			return true;
	return false;
}

bool PrivateChannel :: MatchAuthentication(const char* password)
{
	if(password == NULL)
		return false;
	if(mPassword.compare(password) == 0)
		return true;
	
	return false;
}

void PrivateChannel :: AddSimulator(int SimulatorID, const char* username)
{
	ChannelMember newMember;
	newMember.mSimulatorID = SimulatorID;
	newMember.mUsername = username;
	mMemberList.push_back(newMember);
}

void PrivateChannel :: EnumMembers(std::vector<std::string> &output)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		std::string result = "> ";  //Just a short header to make the results look a bit prettier.
		result.append(mMemberList[i].mUsername);

		output.push_back(result);
	}
}

ChatChannelManager :: ChatChannelManager()
{
	mNextChannelID = CHANNEL_BEGIN_ID;
}

ChatChannelManager :: ~ChatChannelManager()
{
}

int ChatChannelManager :: GetNewChannelID(void)
{
	return mNextChannelID++;
}

int ChatChannelManager :: JoinChannel(int SimulatorID, const char* username, const char* channel, const char* password, std::string& returnData)
{
	if(username == NULL)
		return RESULT_UNHANDLED;
	if(channel == NULL)
		return RESULT_BAD_CHANNEL;
	if(strlen(channel) > MAX_CHANNEL_NAME_SIZE)
		return RESULT_CHANNEL_NAME_SIZE;

	PrivateChannel *exist = GetChannelWithSimulatorID(SimulatorID);
	if(exist != NULL)
	{
		returnData = exist->mName;
		return RESULT_ALREADY_IN_CHANNEL;
	}
	else
	{
		exist = GetExistingChannel(channel);
		if(exist != NULL)
		{
			if(exist->MatchAuthentication(password) == true)
			{
				exist->AddSimulator(SimulatorID, username);
				return RESULT_JOIN_SUCCESS;
			}
			else
				return RESULT_PASSWORD_REQUIRED;
		}
		else
		{
			exist = CreateChannel(channel, password);
			if(exist != NULL)
			{
				exist->AddSimulator(SimulatorID, username);
				return RESULT_CHANNEL_CREATED;
			}
			else
				return RESULT_CHANNEL_FAILED;
		}
	}
	return RESULT_UNHANDLED;
}

//If channel is NULL, leave all channels this player is a member of, otherwise
//remove from the specific channel name.
int ChatChannelManager :: LeaveChannel(int SimulatorID, const char *channel)
{
	int leaveCount = 0;
	CHANNEL_CONT::iterator it;
	it = mChannelList.begin();
	while(it != mChannelList.end())
	{
		if(channel != NULL)
		{
			if(it->second.mName.compare(channel) != 0)
			{
				//We're looking for a specific channel name but this isn't it, go to next iteration.
				++it;
				continue;   
			}
		}

		if(it->second.RemoveUser(SimulatorID) == true)
		{
			leaveCount++;
		}
		if(it->second.mMemberList.size() == 0)
			mChannelList.erase(it++);
		else
			++it;
	}
	if(leaveCount > 0)
		return RESULT_LEAVE_SUCCESS;

	return RESULT_NOT_IN_CHANNEL;
}

PrivateChannel* ChatChannelManager :: GetChannelWithSimulatorID(int SimulatorID)
{
	CHANNEL_CONT::iterator it;
	for(it = mChannelList.begin(); it != mChannelList.end(); ++it)
		if(it->second.HasSimulator(SimulatorID) == true)
			return &it->second;
	return NULL;
}

PrivateChannel* ChatChannelManager :: GetExistingChannel(const char* channel)
{
	CHANNEL_CONT::iterator it;
	for(it = mChannelList.begin(); it != mChannelList.end(); ++it)
		if(it->second.mName.compare(channel) == 0)
			return &it->second;
	return NULL;
}

PrivateChannel* ChatChannelManager :: CreateChannel(const char* channel, const char* password)
{
	int ID = GetNewChannelID();

	PrivateChannel newChannel;
	newChannel.mID = ID;
	newChannel.mName = channel;
	if(password != NULL)
		newChannel.mPassword = password;

	CHANNEL_CONT::iterator it;
	it = mChannelList.insert(mChannelList.end(), CHANNEL_PAIR(ID, newChannel));
	if(it == mChannelList.end())
		return NULL;

	return &it->second;
}

PrivateChannel* ChatChannelManager :: GetChannelForMessage(int SimulatorID, const char* channel)
{
	//Requested by the simulator to process a chat command.  Checks to see that the given
	//channel exists, and that the player is registered into the channel.
	CHANNEL_CONT::iterator it;
	for(it = mChannelList.begin(); it != mChannelList.end(); ++it)
		if(it->second.mName.compare(channel) == 0)
			if(it->second.HasSimulator(SimulatorID) == true)
				return &it->second;
	return NULL;
}

const char *ChatChannelManager :: GetErrorMessage(int returnCode)
{
	switch(returnCode)
	{
	case RESULT_JOIN_SUCCESS:       return "Successfully joined channel.";
	case RESULT_ALREADY_IN_CHANNEL: return "Already in channel.";
	case RESULT_PASSWORD_REQUIRED:  return "Password is required to join that channel.";
	case RESULT_CHANNEL_CREATED:    return "The channel was created.";
	case RESULT_CHANNEL_FAILED:     return "Could not create channel.";
	case RESULT_LEAVE_SUCCESS:      return "You have left the channel.";
	case RESULT_NOT_IN_CHANNEL:     return "You are not in a channel.";
	case RESULT_UNHANDLED:          return "Unhandled error.";
	}
	return "Unknown error.";
}


// This will generate a list of players that occupy any channels this user might be a member of.
// As of creating this function, the system only allows users to be in one channel at a time.
// However, this function searches all channels for results.
// channelName is optional, and if supplied it will filter by a specific channel.
// Note that if we were to look up channel by name, we'd still have to verify whether they were
// a member.
void ChatChannelManager :: EnumPlayersInMemberChannels(int SimulatorID, const char *channelName, std::vector<std::string> &output)
{
	CHANNEL_CONT::iterator it;
	for(it = mChannelList.begin(); it != mChannelList.end(); ++it)
	{
		if(it->second.HasSimulator(SimulatorID) == false)
			continue;
		
		if(channelName != NULL)
		{
			if(it->second.mName.compare(channelName) != 0)
				continue;
		}

		if(output.size() > 0)
			output.push_back("----------");
		
		std::string chName = "Channel [";
		chName.append(it->second.mName);
		chName.append("]");

		if(it->second.mPassword.size() != 0)
			chName.append(" (private)");

		output.push_back(chName);
		it->second.EnumMembers(output);
	}
}
