#pragma once
#ifndef INFO_H
#define INFO_H

#include <vector>
#include <string>
#include "Entities.h"

static std::string KEYPREFIX_TIP= "Tip";
static std::string KEYPREFIX_MOTD= "MOTD";
static std::string KEYPREFIX_IN_GAME_NEWS= "InGameNews";
static std::string LISTPREFIX_LOADING_ANNOUNCMENTS = "LoadingAnnouncements";

class Tip: public AbstractEntity {
public:
	std::string mText;
	int mID;

	Tip();
	~Tip();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
	void Clear(void);
};

class InfoManager
{
public:
	InfoManager();
	~InfoManager();

	bool Init();
	std::string GetMOTD();
	std::string GetInGameNews();
	std::vector<std::string> GetLoadingAnnouncments();
	std::vector<Tip> GetTips();

	std::string mGameName;
	std::string mEdition;
	int GetStartZone();
	int GetStartX();
	int GetStartY();
	int GetStartZ();
	int GetStartRotation();

private:
	std::vector<Tip> mTips;
	int mStartZone;
	int mStartX;
	int mStartY;
	int mStartZ;
	int mStartRotation;
};

extern InfoManager g_InfoManager;


#endif /* INFO_H */
