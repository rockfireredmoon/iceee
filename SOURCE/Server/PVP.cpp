#include "PVP.h"

PVPManager g_PVPManager;

PVPManager::PVPManager() {
	mNextPVPGameId = 0;
}

PVPManager::~PVPManager() {
}

PVPGame * PVPManager::NewGame() {
	PVPGame * game = new PVPGame(++mNextPVPGameId);
	mGames[game->mId] = game;
	return game;
}

void PVPManager::ReleaseGame(int id) {
	std::map<int, PVPGame*>::iterator it = mGames.find(id);
	if(it != mGames.end()) {
		PVPGame * game = it->second;
		delete game;
		mGames.erase(it);
	}
}

//
// Game
//

PVPGame::PVPGame(int id) {
	mId = id;
}

PVPGame::~PVPGame() {}


