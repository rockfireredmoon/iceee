#include "FriendStatus.h"
#include "FileReader.h"

#include "Util.h"
#include "DirectoryAccess.h"
#include "util/Log.h"

FriendListManager g_FriendListManager;

SocialWindowEntry :: SocialWindowEntry(void)
{
	Clear();
}

void SocialWindowEntry :: Clear(void)
{
	creatureDefID = 0;
	name.clear();
	level = 0;
	profession = 0;
	online = false;
	status.clear();
	shard.clear();
}

void SocialWindowEntry :: CopyFrom(SocialWindowEntry& source)
{
	creatureDefID = source.creatureDefID;
	name = source.name;
	level = source.level;
	profession = source.profession;
	online = source.online;
	status = source.status;
	shard = source.shard;
}

void FriendListManager :: LoadAllData(void)
{
	char filename[256];

	Platform::GenerateFilePath(filename, "Dynamic", "FriendList.txt");
	Platform::FixPaths(filename);
	socialDataFile = filename;

	Platform::GenerateFilePath(filename, "Dynamic", "FriendNetwork.txt");
	Platform::FixPaths(filename);
	networkDataFile = filename;

	LoadSocialData();
	LoadNetworkData();

	g_Logs.data->info("Loaded %v Friend Social Entries", socialData.size());
	g_Logs.data->info("Loaded %v Friend Network Entries", networkData.size());
}

void FriendListManager :: SaveAllData(void)
{
	SaveSocialData();
	SaveNetworkData();
}

void FriendListManager :: LoadSocialData(void)
{
	if(socialDataFile.size() == 0)
	{
		g_Logs.data->error("Social Cache filename not set.");
		return;
	}
	FileReader lfr;
	if(lfr.OpenText(socialDataFile.c_str()) != Err_OK)
	{
		g_Logs.data->error("Error opening Social Cache file for reading: %v", socialDataFile.c_str());
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	SocialWindowEntry newItem;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil(",|", '|');
		if(r >= 6)
		{
			newItem.creatureDefID = lfr.BlockToIntC(0);
			newItem.name = lfr.BlockToStringC(1, 0);
			newItem.level = lfr.BlockToIntC(2);
			newItem.profession = lfr.BlockToIntC(3);
			newItem.online = lfr.BlockToBoolC(4);
			newItem.shard = lfr.BlockToStringC(5, 0);

			//HACK: since the friend list is only loaded when the server is launched,
			//it's safe to assume that everyone is offline.  This should auto fix any
			//players that are stuck offline.  If I had been thinking when I designed
			//this, the online flag wouldn't be saved at all.
			//TODO: remove onine status from save files
			newItem.online = false;

			//Get the status last in case it contains any unusual characters.
			if(r >= 7)
				newItem.status = lfr.BlockToStringC(6, 0);
			else
				newItem.status.clear();
			UpdateSocialEntry(newItem);
		}
	}
	lfr.CloseCurrent();
}

void FriendListManager :: SaveSocialData(void)
{
	if(socialDataFile.size() == 0)
	{
		g_Logs.data->error("Social Cache filename not set.");
		return;
	}
	FILE *output = fopen(socialDataFile.c_str(), "wb");
	if(output == NULL)
	{
		g_Logs.data->error("Error opening Social Cache file for writing: %v", socialDataFile.c_str());
		return;
	}
	SOCIAL_MAP::iterator it;
	for(it = socialData.begin(); it != socialData.end(); ++it)
	{
		fprintf(output, "%d,%s,%d,%d,%d,%s|%s\r\n",
			it->second.creatureDefID,
			it->second.name.c_str(),
			it->second.level,
			it->second.profession,
			it->second.online,
			it->second.shard.c_str(),
			it->second.status.c_str() );
	}
	fclose(output);
}

void FriendListManager :: UpdateSocialEntry(SocialWindowEntry &data)
{
	SOCIAL_MAP::iterator it;
	it = socialData.find(data.creatureDefID);
	if(it == socialData.end())
		socialData.insert(socialData.begin(), SOCIAL_PAIR(data.creatureDefID, data));
	else
		it->second.CopyFrom(data);
}

void FriendListManager :: EnumerateFriends(SEARCH_INPUT& inPlayers, SEARCH_OUTPUT& outResults)
{
	SocialWindowEntry data;
	SOCIAL_MAP::iterator it;
	for(size_t i = 0; i < inPlayers.size(); i++)
	{
		it = socialData.find(inPlayers[i].first);
		if(it != socialData.end())
			data.CopyFrom(it->second);
		else
		{
			data.creatureDefID = inPlayers[i].first;
			data.name = inPlayers[i].second;
			data.level = 0;
			data.profession = 0;
			data.online = false;
			data.status.clear();
			data.shard = "Unknown";
		}
		outResults.push_back(data);
	}
}

void FriendListManager :: UpdateNetworkEntry(int CreatureDefID, std::vector<int>& FriendDefIDs)
{
	networkData[CreatureDefID].assign(FriendDefIDs.begin(), FriendDefIDs.end());
}

bool FriendListManager :: IsMutualFriendship(int selfDefID, int otherDefID)
{
	if(HasPlayerInNetwork(selfDefID, otherDefID) == false)
		return false;

	//Search the reverse.  If we get to this point and it returns true, the friendship is mutual.
	return HasPlayerInNetwork(otherDefID, selfDefID);
}

bool FriendListManager :: HasPlayerInNetwork(int firstDefID, int otherDefID)
{
	NETWORK_MAP::iterator it;
	it = networkData.find(firstDefID);
	if(it == networkData.end())
		return false;
	for(size_t i = 0; i < it->second.size(); i++)
	{
		if(it->second[i] == otherDefID)
			return true;
	}
	return false;
}


void FriendListManager :: LoadNetworkData(void)
{
	if(networkDataFile.size() == 0)
	{
		g_Logs.data->error("Network Cache filename not set.");
		return;
	}
	FileReader lfr;
	if(lfr.OpenText(networkDataFile.c_str()) != Err_OK)
	{
		g_Logs.data->error("Error opening Network Cache file for reading: %v", networkDataFile.c_str());
		return;
	}

	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r >= 2)
		{
			int sourceDefID = lfr.BlockToIntC(0);
			std::vector<int> &ref = networkData[sourceDefID];
			for(int i = 1; i < r; i++)
				ref.push_back(lfr.BlockToIntC(i));
		}
	}
	lfr.CloseCurrent();
}

void FriendListManager :: SaveNetworkData(void)
{
	if(networkDataFile.size() == 0)
	{
		g_Logs.data->error("Network Cache filename not set.");
		return;
	}
	FILE *output = fopen(networkDataFile.c_str(), "wb");
	if(output == NULL)
	{
		g_Logs.data->error("Error opening Network Cache file for writing: %v", networkDataFile.c_str());
		return;
	}

	//The Util write function will take care of splitting up the entry over multiple lines.
	char buffer[32];
	NETWORK_MAP::iterator it;
	for(it = networkData.begin(); it != networkData.end(); ++it)
	{
		sprintf(buffer, "%d", it->first);
		Util::WriteIntegerList(output, buffer, it->second);
	}
	fclose(output);
}
