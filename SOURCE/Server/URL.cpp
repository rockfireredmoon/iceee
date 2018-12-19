#include "URL.h"

#include "FileReader3.h"
#include "DirectoryAccess.h"
#include "Config.h"
#include "util/Log.h"

URLManager g_URLManager;

URLManager::URLManager() {
	mLoaded = false;
}

void URLManager::LoadFile(void) {

	std::vector<std::string> paths = g_Config.ResolveLocalConfigurationPath();
	for (std::vector<std::string>::iterator it = paths.begin();
			it != paths.end(); ++it) {
		std::string dir = *it;
		std::string filename = Platform::JoinPath(
				dir, "URL.txt");

		FileReader3 fr;
		if (fr.OpenFile(filename.c_str()) == FileReader3::SUCCESS) {
			fr.SetCommentChar(';');
			while (fr.Readable() == true) {
				fr.ReadLine();
				int r = fr.SingleBreak("=");
				if (r >= 2) {
					STRINGLIST row;
					row.push_back(fr.BlockToStringC(0));
					row.push_back(fr.BlockToStringC(1));
					mLoadedURLs.push_back(row);
				}
			}
			fr.CloseFile();
			mLoaded = true;
		}
	}
	if(!mLoaded) {
		g_Logs.server->error("Could not locate any URL files in any configuration directory");
		return;
	}
}

std::string URLManager::GetURL(std::string name) {
	if (mLoaded == false)
		LoadFile();
	for (std::vector<STRINGLIST>::iterator it = mLoadedURLs.begin();
			it != mLoadedURLs.end(); ++it) {
		std::string n = (*it)[0];
		if (n.compare(name) == 0) {
			return (*it)[1];
		}
	}
	return "http://unknown";
}

const MULTISTRING& URLManager::GetURLs(void) {
	if (mLoaded == false)
		LoadFile();

	return mLoadedURLs;
}
