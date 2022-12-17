#include "Info.h"
#include "Config.h"
#include "GameConfig.h"
#include "StringUtil.h"
#include "Cluster.h"
#include "util/Log.h"

InfoManager g_InfoManager;

std::string ReplaceBrandingPatterns(std::string str) {
	Util::ReplaceAll(str, "${GameName}", g_InfoManager.mGameName);
	Util::ReplaceAll(str, "${Edition}", g_InfoManager.mEdition);
	return str;
}

Tip::Tip() {
	mText = "";
	mID = 0;
}

Tip::~Tip() {

}

bool Tip::WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_TIP, StringUtil::Format("%d", mID));
	writer->Value("Text", mText);
	return true;
}

bool Tip::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_TIP, StringUtil::Format("%d", mID), true);
	return true;
}

bool Tip::ReadEntity(AbstractEntityReader *reader) {
	mText = reader->Value("Text");
	return true;
}

void Tip::Clear(void) {
	mText = "";
	mID = 0;
}

InfoManager::InfoManager() {

	/* Fallback defaults */
	mGameName="Earth Eternal";
	mEdition="Valkal's Shadow";
	mStartZone=59;
	mStartX=5822;
	mStartY=684;
	mStartZ=5877;
	mStartRotation=81;
}

InfoManager::~InfoManager() {
}

std::string GetOverrideZone() {
	STRINGLIST output;
	Util::Split(g_GameConfig.OverrideStartLoc, ";", output);
	return output[0];
}

STRINGLIST GetOverridePos() {
	STRINGLIST output;
	Util::Split(g_GameConfig.OverrideStartLoc, ";", output);
	STRINGLIST output2;
	Util::Split(output[1], ",", output2);
	return output2;
}

int InfoManager::GetStartX() {
	auto pos = GetOverridePos();
	if(pos.size() > 2) {
		return std::stoi(pos[0]);
	}
	return mStartX;
}

int InfoManager::GetStartY() {
	auto pos = GetOverridePos();
	if(pos.size() > 2) {
		return std::stoi(pos[1]);
	}
	return mStartY;
}

int InfoManager::GetStartZ() {
	auto pos = GetOverridePos();
	if(pos.size() > 2) {
		return std::stoi(pos[2]);
	}
	return mStartZ;
}

int InfoManager::GetStartZone() {
	auto zone = GetOverrideZone();
	if(zone.size() > 0)
		return std::stoi(zone);
	else
		return mStartZone;
}

int InfoManager::GetStartRotation() {
	auto pos = GetOverridePos();
	if(pos.size() > 3) {
		return std::stoi(pos[3]);
	}
	return mStartRotation;
}

std::string InfoManager::GetMOTD() {
	return ReplaceBrandingPatterns(g_ClusterManager.GetKey(KEYPREFIX_MOTD, StringUtil::Format("Welcome to ${GameName} - ${Edition}. You can set your own #3Message Of The Day# by setting the key #3'%s'# in the Redis database.", KEYPREFIX_MOTD.c_str())));
}

std::string InfoManager::GetInGameNews() {
	return ReplaceBrandingPatterns(g_ClusterManager.GetKey(KEYPREFIX_IN_GAME_NEWS, StringUtil::Format("Welcome to ${GameName} - ${Edition}. You can set your own <b>In Game News</b> by setting the key <b>'%s'</b> in the Redis database. This supports a small subset of HTML, as used elsewhere in the game.", KEYPREFIX_IN_GAME_NEWS.c_str())));
}

std::vector<std::string> InfoManager::GetLoadingAnnouncments() {
	STRINGLIST l = g_ClusterManager.GetList(LISTPREFIX_LOADING_ANNOUNCMENTS);
	STRINGLIST s;
	if(l.size() == 0) {
		s.push_back(StringUtil::Format(ReplaceBrandingPatterns("Welcome to ${GameName} - ${Edition}. You can set your own <b>Loading Announcements</b> by creating and adding multiple elements to the list <b>'%s'</b> in the Redis database."), LISTPREFIX_LOADING_ANNOUNCMENTS.c_str()));
	}
	else {
		for(auto it = l.begin(); it != l.end(); ++it) {
			s.push_back(ReplaceBrandingPatterns(*it));
		}
	}
	return s;

}

bool InfoManager::Init() {

	TextFileEntityReader ter(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveStaticDataPath(), "Data"), "Tips.txt" ), Case_None, Comment_Semi);
	ter.Start();
	if (!ter.Exists())
		return false;

	ter.Key("", "", true);
	ter.Index("ENTRY");
	STRINGLIST sections = ter.Sections();
	int i = 0;
	for (auto a = sections.begin(); a != sections.end(); ++a) {
		ter.PushSection(*a);
		Tip t;
		t.mID = ++i;
		if (!t.EntityKeys(&ter) || !t.ReadEntity(&ter))
			return false;
		mTips.push_back(t);
		ter.PopSection();
	}
	ter.End();

	std::string filename = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveStaticDataPath(), "Data"), "Game.txt" );
	FileReader lfr;
	if (lfr.OpenText(filename.c_str()) != Err_OK) {
		g_Logs.data->error("Could not open configuration file: %v", filename);
		return false;
	}
	else {
		static char Delimiter[] = { '=', 13, 10 };
		lfr.Delimiter = Delimiter;
		lfr.CommentStyle = Comment_Semi;

		while (lfr.FileOpen() == true) {
			int r = lfr.ReadLine();
			if (r > 0) {
				lfr.SingleBreak("=");
				char *NameBlock = lfr.BlockToString(0);
				if (strcmp(NameBlock, "GameName") == 0) {
					mGameName = lfr.BlockToStringC(1, 0);
				} else if (strcmp(NameBlock, "Edition") == 0) {
					mEdition = lfr.BlockToStringC(1, 0);
				} else if (strcmp(NameBlock, "StartZone") == 0) {
					mStartZone = lfr.BlockToInt(1);
				} else if (strcmp(NameBlock, "StartX") == 0) {
					mStartX = lfr.BlockToInt(1);
				} else if (strcmp(NameBlock, "StartY") == 0) {
					mStartY = lfr.BlockToInt(1);
				} else if (strcmp(NameBlock, "StartZ") == 0) {
					mStartZ = lfr.BlockToInt(1);
				} else if (strcmp(NameBlock, "StartRotation") == 0) {
					mStartRotation = lfr.BlockToInt(1);
				}
				else {
					g_Logs.data->error("Unknown identifier [%v] in config file [%v]",
							lfr.BlockToString(0), filename);
				}
			}
		}
		lfr.CloseCurrent();
	}

	return true;

}

std::vector<Tip> InfoManager::GetTips() {
	return mTips;
}

