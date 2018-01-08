#include "Info.h"
#include "Config.h"
#include "StringUtil.h"
#include "Cluster.h"
#include "util/Log.h"

InfoManager g_InfoManager;

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
}

InfoManager::~InfoManager() {
}

std::string InfoManager::GetMOTD() {
	return g_ClusterManager.GetKey(KEYPREFIX_MOTD, StringUtil::Format("Welcome to Earth Eternal. You can set your own #3Message Of The Day# by setting the key #3'%s'# in the Redis database.", KEYPREFIX_MOTD.c_str()));
}

std::string InfoManager::GetInGameNews() {
	return g_ClusterManager.GetKey(KEYPREFIX_IN_GAME_NEWS, StringUtil::Format("Welcome to Earth Eternal. You can set your own <b>In Game News</b> by setting the key <b>'%s'</b> in the Redis database. This supports a small subset of HTML, as used elsewhere in the game.", KEYPREFIX_IN_GAME_NEWS.c_str()));
}

std::vector<std::string> InfoManager::GetLoadingAnnouncments() {
	STRINGLIST l = g_ClusterManager.GetList(LISTPREFIX_LOADING_ANNOUNCMENTS);
	if(l.size() == 0) {
		l.push_back(StringUtil::Format("Welcome to Earth Eternal. You can set your own <b>Loading Announcements</b> by creating and adding multiple elements to the list <b>'%s'</b> in the Redis database.", LISTPREFIX_LOADING_ANNOUNCMENTS.c_str()));
	}
	return l;
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

	return true;

}

std::vector<Tip> InfoManager::GetTips() {
	return mTips;
}

