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

	string parent = query->GetString(0);
	if(!g_AssetCatalogueManager.Contains(parent)) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such prop catalogue item.");
	}

	QueryResponse resp(query->ID);

	auto item = g_AssetCatalogueManager.GetItem(parent);
	vector<AssetCatalogueItem*> items =  g_AssetCatalogueManager.GetChildren(parent, pld);

	int rows = items.size();
	if (rows > 99) {
		rows = 99;
	}

	// First row contains category data
	WriteAssetCatalogueItem(resp, *item);

	// Subsequent rows contain each child name, asset and type
	sort(items.begin(), items.end(), itemSort);
	for (auto it :items) {
		auto row = resp.Row();
		row->push_back(it->mName);
		row->push_back(it->GetAsset());
		row->push_back(to_string(it->mType));
		row->push_back(it->GetDisplayName());
		rows--;
		if(rows < 1)
			break;
	}
	return resp.Write(sim->SendBuf);
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

	string search = query->GetString(0);
	vector<AssetCatalogueItem*> items;
	AssetCatalogueSearch ps;
	ps.mSearch = search;
	g_AssetCatalogueManager.Search(ps, &items);

	QueryResponse resp(query->ID);

	int rows = items.size();
	if (rows > 99) {
		rows = 99;
	}

	// First row contains root data
	AssetCatalogueItem searchResults;
	searchResults.mDescription = "Search Results";
	searchResults.mName = "Search Results";
	WriteAssetCatalogueItem(resp, searchResults);

	// Subsequent rows contain each search match
	sort(items.begin(), items.end(), itemSort);
	for (auto it : items) {
		auto row = resp.Row();
		row->push_back(it->mName);
		row->push_back(it->GetAsset());
		row->push_back(to_string(it->mType));
		row->push_back(it->GetDisplayName());
		rows--;
		if(rows < 1)
			break;
	}
	return resp.Write(sim->SendBuf);
}

void WriteAssetCatalogueItem(QueryResponse &resp, AssetCatalogueItem &item) {
	auto row = resp.Row();
	// 1
	row->push_back(item.mName);
	// 2
	row->push_back(item.GetAsset());
	// 3
	row->push_back(item.mDescription);
	// 4
	STRINGLIST l;
	for(auto it = item.mParents.begin(); it != item.mParents.end(); ++it) {
		l.push_back((*it)->mName);
	}
	std::string p;
	Util::Join(l, ",", p);
	row->push_back(p);
	//5
	row->push_back(Util::Format("%d", item.mType));
	//6
	row->push_back(item.GetDisplayName());
}
