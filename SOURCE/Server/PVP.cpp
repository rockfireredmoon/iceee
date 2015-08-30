#include "PVP.h"
#include "Util.h"

PVP::PVPManager g_PVPManager;

namespace PVP {
PVPManager::PVPManager() {
	mNextPVPGameId = 0;
}

PVPManager::~PVPManager() {
}

namespace PVPTeams {

const char *GetNameByID(int id) {
	switch (id) {
	case NONE:
		return "NONE";
	case RED:
		return "RED";
	case BLUE:
		return "BLUE";
	case YELLOW:
		return "YELLOW";
	case GREEN:
		return "GREEN";
	}
	return "<undefined>";
}

int GetIDByName(const std::string &name) {
	if (Util::CaseInsensitiveStringCompare("red", name))
		return RED;
	if (Util::CaseInsensitiveStringCompare("blue", name))
		return BLUE;
	if (Util::CaseInsensitiveStringCompare("yellow", name))
		return YELLOW;
	if (Util::CaseInsensitiveStringCompare("green", name))
		return GREEN;
	return NONE;
}

}

PVPGame * PVPManager::NewGame() {
	PVPGame * game = new PVPGame(++mNextPVPGameId);
	mGames[game->mId] = game;
	return game;
}

void PVPManager::ReleaseGame(int id) {
	std::map<int, PVPGame*>::iterator it = mGames.find(id);
	if (it != mGames.end()) {
		PVPGame * game = it->second;
		delete game;
		mGames.erase(it);
	}
}


PVPGame * PVPManager::GetGameForTeam(int teamID) {
	for(std::map<int, PVPGame*>::iterator it = mGames.begin(); it != mGames.end(); ++it) {
		PVPGame * game = it->second;
		if(game->HasTeam(teamID)) {
			return game;
		}
	}
	return NULL;
}

//
// Game
//

PVPGame::PVPGame(int id) {
	mId = id;
	mGameState = PVPGameState::WAITING_TO_START;
	mGameType = PVPGameType::DEATHMATCH;
}

PVPGame::~PVPGame() {
}

bool PVPGame::HasTeam(int teamID)
{
	return mTeams.find(teamID) != mTeams.end();
}

ActiveParty * PVPGame::GetTeamForPlayer(int CreatureID) {
	for(std::map<int, ActiveParty*>::iterator it = mTeams.begin(); it != mTeams.end(); ++it) {
		if(it->second->GetMemberByID(CreatureID) != NULL) {
			return it->second;
		}
	}
	return NULL;
}

}

//
// Utilities
//



//if (updateType & this.PVP_TIME_UPDATED)
//{
//	local timeLeft = data.getFloat();
//
//	if (pvpScreen)
//	{
//		pvpScreen.updateTime(timeLeft);
//	}
//}

int PrepExt_PVPTimeUpdate(char *buffer, PVP::PVPGame *game, long remainingMS)
{
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 80);     //_handlePVPStatUpdateMessage
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], PVP::PVPUpdateFlag::PVP_TIME_UPDATED);
	wpos += PutByte(&buffer[wpos], game->mGameType);

	wpos += PutFloat(&buffer[wpos], remainingMS);

	PutShort(&buffer[1], wpos - 3);

	return wpos;
}

int PrepExt_PVPStatUpdate(char *buffer, PVP::PVPGame *game, PartyMember *partyMember)
{
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 80);     //_handlePVPStatUpdateMessage
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], PVP::PVPUpdateFlag::PVP_STAT_UPDATED);
	wpos += PutByte(&buffer[wpos], game->mGameType);

	wpos += PutInteger(&buffer[wpos], partyMember->mCreatureID);

	switch(game->mGameType) {
	case PVP::PVPGameType::CTF:
		wpos += PutByte(&buffer[wpos], 3);
		wpos += PutInteger(&buffer[wpos], partyMember->mPVPKills);
		wpos += PutInteger(&buffer[wpos], partyMember->mPVPDeaths);
		wpos += PutInteger(&buffer[wpos], partyMember->mPVPGoals);
		break;
	case PVP::PVPGameType::TEAMSLAYER:
		wpos += PutByte(&buffer[wpos], 2);
		wpos += PutInteger(&buffer[wpos], partyMember->mPVPKills);
		wpos += PutInteger(&buffer[wpos], partyMember->mPVPDeaths);
		break;
	default:
		wpos += PutByte(&buffer[wpos], 2);
		break;
	}

	PutShort(&buffer[1], wpos - 3);

	return wpos;
}

int PrepExt_PVPStateUpdate(char *buffer, PVP::PVPGame *game) {
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 80);     //_handlePVPStatUpdateMessage
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&buffer[wpos], PVP::PVPUpdateFlag::PVP_STATE_UPDATE);
	wpos += PutByte(&buffer[wpos], game->mGameType);

	wpos += PutByte(&buffer[wpos], game->mGameState);

	PutShort(&buffer[1], wpos - 3);       //Set message size

	return wpos;
}


int PrepExt_PVPTeamRemove(char *buffer, PVP::PVPGame *game, int playerID) {
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 80);     //_handlePVPStatUpdateMessage
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&buffer[wpos], PVP::PVPUpdateFlag::PVP_TEAM_UPDATED);
	wpos += PutByte(&buffer[wpos], game->mGameType);

	wpos += PutInteger(&buffer[wpos], playerID);
	wpos += PutByte(&buffer[wpos], 0);

	PutShort(&buffer[1], wpos - 3);       //Set message size

	return wpos;
}


int PrepExt_PVPTeamAdd(char *buffer, PVP::PVPGame *game, const char * playerName, int playerID, int team) {
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 80);     //_handlePVPStatUpdateMessage
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], PVP::PVPUpdateFlag::PVP_TEAM_UPDATED);
	wpos += PutByte(&buffer[wpos], game->mGameType);

	wpos += PutInteger(&buffer[wpos], playerID);
	wpos += PutByte(&buffer[wpos], team);
	wpos += PutStringUTF(&buffer[wpos], playerName);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}
