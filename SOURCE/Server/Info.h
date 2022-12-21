#pragma once
#ifndef INFO_H
#define INFO_H

#include "Entities.h"
#include <vector>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

static string KEYPREFIX_TIP= "Tip";
static string KEYPREFIX_MOTD= "MOTD";
static string KEYPREFIX_IN_GAME_NEWS= "InGameNews";
static string LISTPREFIX_LOADING_ANNOUNCMENTS = "LoadingAnnouncements";

class Tip: public AbstractEntity {
public:
	string mText;
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
	string GetMOTD();
	string GetInGameNews();
	vector<string> GetLoadingAnnouncments();
	vector<Tip> GetTips();

	string mGameName;
	string mEdition;
	int GetStartZone();
	int GetStartX();
	int GetStartY();
	int GetStartZ();
	int GetStartRotation();

private:
	vector<Tip> mTips;
	int mStartZone;
	int mStartX;
	int mStartY;
	int mStartZ;
	int mStartRotation;
};

extern InfoManager g_InfoManager;


#endif /* INFO_H */
