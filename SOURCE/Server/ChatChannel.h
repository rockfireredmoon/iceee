#include <string>
#include <vector>
#include <map>

struct ChannelMember
{
	int mSimulatorID;
	std::string mUsername;
};

struct PrivateChannel
{
	std::string mName;
	std::string mPassword;
	std::vector<ChannelMember> mMemberList;
	int mID;
	PrivateChannel();
	bool RemoveUser(int SimulatorID);
	bool HasSimulator(int SimulatorID);
	bool MatchAuthentication(const char* password);
	void AddSimulator(int SimulatorID, const char* username);
	void EnumMembers(std::vector<std::string> &output);
};

class ChatChannelManager
{
public:
	ChatChannelManager();
	~ChatChannelManager();

	typedef std::map<int, PrivateChannel> CHANNEL_CONT;
	typedef std::pair<int, PrivateChannel> CHANNEL_PAIR;
	static const int CHANNEL_BEGIN_ID = 1;   //Zero ID means not in a channel.
	static const int MAX_CHANNEL_NAME_SIZE = 10;

	CHANNEL_CONT mChannelList;
	int mNextChannelID;

	enum ReturnCode
	{
		RESULT_JOIN_SUCCESS       = 0,
		RESULT_ALREADY_IN_CHANNEL = 1,
		RESULT_PASSWORD_REQUIRED  = 2,
		RESULT_CHANNEL_CREATED    = 3,
		RESULT_CHANNEL_FAILED     = 4,
		RESULT_LEAVE_SUCCESS      = 5,
		RESULT_NOT_IN_CHANNEL     = 6,
		RESULT_BAD_CHANNEL        = 7,
		RESULT_CHANNEL_NAME_SIZE  = 8,
		RESULT_UNHANDLED          = 9
	};

	int GetNewChannelID(void);
	int JoinChannel(int SimulatorID, const char* username, const char* channel, const char* password, std::string& returnData);
	int LeaveChannel(int SimulatorID, const char *channel);
	PrivateChannel* GetChannelWithSimulatorID(int SimulatorID);
	PrivateChannel* GetExistingChannel(const char* channel);
	PrivateChannel* CreateChannel(const char* channel, const char* password);
	PrivateChannel* GetChannelForMessage(int SimulatorID, const char* channel);
	const char *GetErrorMessage(int returnCode);
	void EnumPlayersInMemberChannels(int SimulatorID, const char *channelName, std::vector<std::string> &output);
};

extern ChatChannelManager g_ChatChannelManager;