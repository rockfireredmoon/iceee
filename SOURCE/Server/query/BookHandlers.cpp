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

#include "BookHandlers.h"
#include "../Books.h"
#include "../util/Log.h"

using namespace std;

//
// BookListHandler
//

int BookListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Books

	g_Logs.server->debug("Retrieving books for: %v", creatureInstance->CreatureDefID);

	InventoryManager inv = creatureInstance->charPtr->inventory;
	std::set<int> booksFound;
	for(size_t a = 0; a < inv.containerList[INV_CONTAINER].size(); a++) {
		int slot = inv.containerList[INV_CONTAINER][a].GetSlot();
		int ID = inv.containerList[INV_CONTAINER][a].IID;
		ItemDef *itemDef = inv.containerList[INV_CONTAINER][a].ResolveSafeItemPtr();
		if(itemDef != NULL) {
			if(itemDef->mIvType1 == ItemIntegerType::BOOK_PAGE && booksFound.find(itemDef->mIvMax1) == booksFound.end()) {
				BookDefinition def = g_BookManager.GetBookDefinition(itemDef->mIvMax1);
				if(def.bookID > 0) {
					booksFound.insert(itemDef->mIvMax1);
					wpos += PutByte(&sim->SendBuf[wpos], 3);
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d", def.bookID);
					wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
					wpos += PutStringUTF(&sim->SendBuf[wpos], def.title.c_str());
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d", def.pages.size());
					wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
					g_Logs.server->debug("   ID: %v Title: %v Pages: %v", def.bookID, def.title, def.pages.size());
				}
				else {
					g_Logs.server->warn("Item %v refers to an invalid book, %v", ID, itemDef->mIvMax1);
				}
			}
		}
	}
	PutShort(&sim->SendBuf[7], booksFound.size());
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
// BookGetHandler
//

int BookGetHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query");

	BookDefinition def = g_BookManager.GetBookDefinition(query->GetInteger(0));
	if(def.bookID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid book");


	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 0);  //Have pages

	InventoryManager inv = creatureInstance->charPtr->inventory;
	int pagesFound = 0;
	g_Logs.server->debug("Retrieving book: %v (%v) with %v pages", def.bookID, def.title, def.pages.size());
	std::set<int> pagesFoundSet;
	for(size_t a = 0; a < inv.containerList[INV_CONTAINER].size(); a++) {
		int slot = inv.containerList[INV_CONTAINER][a].GetSlot();
		int ID = inv.containerList[INV_CONTAINER][a].IID;
		ItemDef *itemDef = inv.containerList[INV_CONTAINER][a].ResolveSafeItemPtr();
		if(itemDef != NULL) {
			if(itemDef->mIvType1 == ItemIntegerType::BOOK_PAGE && itemDef->mIvMax1 == def.bookID && pagesFoundSet.find(itemDef->mIvMax2) == pagesFoundSet.end()) {
				pagesFoundSet.insert(itemDef->mIvMax2);
				wpos += PutByte(&sim->SendBuf[wpos], 2);
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d", itemDef->mIvMax2 - 1);
				g_Logs.server->debug("    Page: %v = %v", itemDef->mIvMax2, def.pages[itemDef->mIvMax2 - 1]);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
				wpos += PutStringUTF(&sim->SendBuf[wpos], def.pages[itemDef->mIvMax2 - 1].c_str());
				pagesFound++;
			}
		}
	}
	PutShort(&sim->SendBuf[7], pagesFound);
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}
