/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#include "AssetCatalogueHandlers.h"
#include "../ByteBuffer.h"
#include "../Config.h"
#include "../Chat.h"
#include "../Instance.h"
#include "../Cluster.h"
#include <algorithm>

using namespace std;

bool itemSort(AssetCatalogueItem *l1, AssetCatalogueItem *l2) {
	if(l1->mOrder == l2->mOrder) {
		return l1->GetDisplayName() > l2->GetDisplayName();
	}
	return l1->mOrder > l2->mOrder;
}

//
// GetPropCategoriesHandler
//

int GetPropCategoriesHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	std::string parent = query->GetString(0);
	if(!g_AssetCatalogueManager.Contains(parent)) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such prop catalogue item.");
	}


	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);      //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);   //Query response index

	AssetCatalogueItem *item = g_AssetCatalogueManager.GetItem(parent);
	std::vector<AssetCatalogueItem*> items =  g_AssetCatalogueManager.GetChildren(parent, pld);

	int rows = items.size();
	if (rows > 99) {
		rows = 99;
	}

	wpos += PutShort(&sim->SendBuf[wpos], rows + 1);

	// First row contains category data
	wpos += PutByte(&sim->SendBuf[wpos], 6);
	wpos += WriteAssetCatalogueItem(&sim->SendBuf[wpos], item);

	// Subsequent rows contain each child name, asset and type
	sort(items.begin(), items.end(), itemSort);
	for (auto it = items.begin(); it != items.end(); ++it) {
		wpos += PutByte(&sim->SendBuf[wpos], 4);
		wpos += PutStringUTF(&sim->SendBuf[wpos], (*it)->mName);
		wpos += PutStringUTF(&sim->SendBuf[wpos], (*it)->GetAsset());
		wpos += PutStringUTF(&sim->SendBuf[wpos], std::to_string((*it)->mType));
		wpos += PutStringUTF(&sim->SendBuf[wpos], (*it)->GetDisplayName());
		rows--;
		if(rows < 1)
			break;
	}

	PutShort(&sim->SendBuf[1], wpos - 3);             //Set message size
	return wpos;
}

//
// SearchPropsHandler
//

int SearchPropsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	std::string search = query->GetString(0);
	std::vector<AssetCatalogueItem*> items;
	AssetCatalogueSearch ps;
	ps.mSearch = search;
	g_AssetCatalogueManager.Search(ps, &items);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);      //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);   //Query response index

	int rows = items.size();
	if (rows > 99) {
		rows = 99;
	}

	wpos += PutShort(&sim->SendBuf[wpos], rows + 1);

	// First row contains root data
	wpos += PutByte(&sim->SendBuf[wpos], 6);
	AssetCatalogueItem searchResults;
	searchResults.mDescription = "Search Results";
	searchResults.mName = "Search Results";
	wpos += WriteAssetCatalogueItem(&sim->SendBuf[wpos], &searchResults);

	// Subsequent rows contain each search match
	sort(items.begin(), items.end(), itemSort);
	for (auto it = items.begin(); it != items.end(); ++it) {
		wpos += PutByte(&sim->SendBuf[wpos], 4);
		wpos += PutStringUTF(&sim->SendBuf[wpos], (*it)->mName.c_str());
		wpos += PutStringUTF(&sim->SendBuf[wpos], (*it)->GetAsset().c_str());
		wpos += PutStringUTF(&sim->SendBuf[wpos], std::to_string((*it)->mType));
		wpos += PutStringUTF(&sim->SendBuf[wpos], (*it)->GetDisplayName());
		rows--;
		if(rows < 1)
			break;
	}

	PutShort(&sim->SendBuf[1], wpos - 3);             //Set message size
	return wpos;
}
