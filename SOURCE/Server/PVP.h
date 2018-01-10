#pragma once
#ifndef PVP_H
#define PVP_H

#include <vector>
#include <map>
#include <string>
#include "PartyManager.h"

namespace PVP {

namespace PVPGameType {
	enum {
		NONE = 0,
		MILLEAGE = 1,
		MASSACRE = 2,
		TEAMSLAYER = 3,
		CTF = 4,
		DEATHMATCH = 5
	};
};

struct PVPUpdateFlag {
	enum Flags {
		PVP_STATE_UPDATE = 1,
		PVP_TEAM_UPDATED = 2,
		PVP_STAT_UPDATED = 4,
		PVP_TIME_UPDATED = 8,
		PVP_FLAG_EVENT = 16
	};
};

namespace GameMode
{
	enum Enum
	{
		PVE = 0,
		PVP = 1,
		PVP_ONLY = 2,
		PVE_ONLY = 3,
		SPECIAL_EVENT = 4
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const std::string &eventName);
}

struct PVPGameState {
	enum State {
		WAITING_TO_START = 0,
		WAITING_TO_CONTINUE = 1,
		PLAYING = 2,
		POST_GAME_LOBBY = 3
	};
};

namespace PVPTeams {
	enum Team {
		NONE = 0, RED = 1, BLUE = 2, YELLOW = 3, GREEN = 4
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const std::string &eventName);
};

//
//struct FlagType
//{
//	enum
//	{
//		RED = 0,
//		BLUE = 1
//	};
//};
//
//struct FlagEvents
//{
//	enum
//	{
//		TAKEN = 0,
//		DROPPED = 1,
//		CAPTURED = 2,
//		RETURNED = 3
//	};
//};
//
//struct ClassBit
//{
//	enum
//	{
//		Knight = 1,
//		Rogue  = 2,
//		Mage   = 4,
//		Druid  = 8
//	};
//};
//
//struct PVPRule
//{
//	static const int MOD_TYPE_EXPLICIT = 0;       //The stat will be set to match this value.  Burns off all bonuses supplied by armor.
//	static const int MOD_TYPE_ADD      = 1;       //Add a flat amount.
//	static const int MOD_TYPE_MULTIPLY = 2;       //Multiply by a percentage using the current total.
//	static const int MOD_TYPE_MULTIPLYBASE = 3;   //Multiply by a percentage using the base unadjusted value.
//
//	short mClassFlags;   //See: ClassBit in Ability.h
//	std::string mName;   //Name of a character, if applying a direct handicap.
//	short mMinLevel;     //Minimum level to apply.  Zero for no minimum.
//	short mMaxLevel;     //Maximum level to apply.  Zero for no maximum.
//	short mStatID;       //Stat ID to adjust.
//	float mStatMod;      //Amount to convert.
//	short mStatModType;  //See MOD_TYPE_* above.
//};
//
//bool RulePass(const PVPRule& rule, CreatureInstance* player)
//{
//	//if(rule.mName.size() != 0)
//	//	return player->css.MatchName(rule.mName);
//
//	switch(player->css.profession)
//	{
//	case 1:
//		if(!(rule.mClassFlags & ClassBit::Knight))
//			return false;
//		break;
//	case 2:
//		if(!(rule.mClassFlags & ClassBit::Rogue))
//			return false;
//		break;
//	case 3:
//		if(!(rule.mClassFlags & ClassBit::Mage))
//			return false;
//		break;
//	case 4:
//		if(!(rule.mClassFlags & ClassBit::Druid))
//			return false;
//		break;
//	}
//	if(rule.mMinLevel != 0 && player->css.level < rule.mMinLevel)
//		return false;
//	if(rule.mMaxLevel != 0 && player->css.level > rule.mMaxLevel)
//		return false;
//
//	return true;
//}

class PVPGame {
public:
	int mId;
	int mGameType;
	PVPGameState::State mGameState;
	PVPGame(int id);
	~PVPGame();
	bool HasTeam(int teamID);
	std::map<int, ActiveParty*> mTeams;
	ActiveParty * GetTeamForPlayer(int CreatureID);
};

class PVPManager {
public:
	PVPManager();
	~PVPManager();
	int mNextPVPGameId;
	std::map<int, PVPGame*> mGames;
	PVPGame * NewGame();
	PVPGame * GetGame(int gameId);
	PVPGame * GetGameForTeam(int teamID);
	void ReleaseGame(int id);
};
}

int PrepExt_PVPTimeUpdate(char *buffer, PVP::PVPGame *game, long timeRemaining);
int PrepExt_PVPStatUpdate(char *buffer, PVP::PVPGame *game, PartyMember *teamMember);
int PrepExt_PVPTeamRemove(char *buffer, PVP::PVPGame *game, int playerID);
int PrepExt_PVPTeamAdd(char *buffer, PVP::PVPGame *game, const char * playerName, int playerID, int team);
int PrepExt_PVPStateUpdate(char *buffer, PVP::PVPGame *game);

extern PVP::PVPManager g_PVPManager;

#endif //#define PVP_H
