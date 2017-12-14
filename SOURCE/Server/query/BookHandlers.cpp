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
#include "../Crafting.h"

using namespace std;

//
// BookListHandler
//

int BookListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Books

	InventoryManager inv = creatureInstance->charPtr->inventory;
	std::set<int> booksFound;
	for (size_t a = 0; a < inv.containerList[INV_CONTAINER].size(); a++) {
		int slot = inv.containerList[INV_CONTAINER][a].GetSlot();
		int ID = inv.containerList[INV_CONTAINER][a].IID;
		ItemDef *itemDef =
				inv.containerList[INV_CONTAINER][a].ResolveSafeItemPtr();
		if (itemDef != NULL && itemDef->mType == ItemType::SPECIAL) {
			int book = itemDef->GetDynamicMax(ItemIntegerType::BOOK);
			if (book > -1
					&& booksFound.find(itemDef->mIvMax1) == booksFound.end()) {
				/* An item with ItemIntegerType::BOOK but NO ItemIntegerType::BOOK_PAGE is a complete book, so just add it */
				booksFound.insert(book);
				BookDefinition def = g_BookManager.GetBookDefinition(
						itemDef->mIvMax1);
				if (def.bookID > 0) {
					wpos += PutByte(&sim->SendBuf[wpos], 3);
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d", def.bookID);
					wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
					wpos += PutStringUTF(&sim->SendBuf[wpos], def.title.c_str());
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d",
							def.pages.size());
					wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
				} else {
					g_Logs.server->warn(
							"Item %v refers to an invalid book, %v", ID,
							itemDef->mIvMax1);
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

int BookGetHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query");

	BookDefinition def = g_BookManager.GetBookDefinition(query->GetInteger(0));
	if (def.bookID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid book");

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 0);  //Have pages

	InventoryManager inv = creatureInstance->charPtr->inventory;
	int pagesFound = 0;
	std::set<int> pagesFoundSet;
	for (size_t a = 0; a < inv.containerList[INV_CONTAINER].size(); a++) {
		int slot = inv.containerList[INV_CONTAINER][a].GetSlot();
		int ID = inv.containerList[INV_CONTAINER][a].IID;
		ItemDef *itemDef =
				inv.containerList[INV_CONTAINER][a].ResolveSafeItemPtr();
		if (itemDef != NULL && itemDef->mType == ItemType::SPECIAL) {
			int pageNo = itemDef->GetDynamicMax(ItemIntegerType::BOOK_PAGE);
			if (itemDef->GetDynamicMax(ItemIntegerType::BOOK_PAGE) == -1
					&& itemDef->GetDynamicMax(ItemIntegerType::BOOK)
							== def.bookID) {
				/* An item with ItemIntegerType::BOOK but NO ItemIntegerType::BOOK_PAGE is a complete book, so return all the pages */
				for (int i = 0; i < def.pages.size(); i++) {
					pagesFoundSet.insert(itemDef->mIvMax2);
					wpos += PutByte(&sim->SendBuf[wpos], 2);
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d", i);
					wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
					wpos += PutStringUTF(&sim->SendBuf[wpos],
							def.pages[i].c_str());
					pagesFound++;
				}
			} else if (itemDef->GetDynamicMax(ItemIntegerType::BOOK)
					== def.bookID && pageNo > 0
					&& pagesFoundSet.find(pageNo) == pagesFoundSet.end()) {
				/* An item with ItemIntegerType::BOOK and a ItemIntegerType::BOOK_PAGE is book page, so return just the pages */
				pagesFoundSet.insert(itemDef->mIvMax2);
				wpos += PutByte(&sim->SendBuf[wpos], 2);
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d",
						pageNo - 1);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
				wpos += PutStringUTF(&sim->SendBuf[wpos],
						def.pages[pageNo - 1].c_str());
				pagesFound++;
			}
		}
	}
	PutShort(&sim->SendBuf[7], pagesFound);
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
// BookItemHandler
//

int BookItemHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query");

	ItemDef *itemDef = g_ItemManager.GetSafePointerByID(
			atoi(query->GetString(0)));
	if (itemDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid item");

	int bookItemId = 0;
	int bookPages = 0;

	if (itemDef->mType == ItemType::SPECIAL) {
		int book = itemDef->GetDynamicMax(ItemIntegerType::BOOK);
		if (book > -1) {
			ItemManager::ITEM_CONT::iterator it;
			for (it = g_ItemManager.ItemList.begin();
					it != g_ItemManager.ItemList.end(); ++it) {
				if (it->second.GetDynamicMax(ItemIntegerType::BOOK) == book) {
					if (it->second.GetDynamicMax(ItemIntegerType::BOOK_PAGE)
							== -1) {
						bookItemId = it->second.mID;
					} else {
						bookPages++;
					}
				}
			}
		}
	}
	if (bookPages > 0) {

		/* Now we know the resultant item, look for a crafting definition that produces this item.
		 * This gives us the list of items required. We scan that list looking for items that
		 * are BOOK_MATERIAL types and return those as the list of material items
		 */
		const CraftRecipe *recipe = g_CraftManager.GetFirstRecipeForResult(
				bookItemId);
		if (recipe == NULL) {
			g_Logs.server->warn(
					"Request for recipe for book item that doesn't exist. Book item ID is %v",
					bookItemId);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Recipe for book missing");
		}

		std::vector<int> results;
		std::vector<int> requiredMaterials;
		recipe->GetRequiredItems(results);
		for (std::vector<int>::iterator it = results.begin();
				it != results.end(); ++it) {
			ItemDef *materialItem = g_ItemManager.GetSafePointerByID(*it);
			if (materialItem != NULL
					&& materialItem->GetDynamicMax(
							ItemIntegerType::BOOK_MATERIAL) != -1) {
				requiredMaterials.push_back(*it);
				requiredMaterials.push_back(recipe->GetRequiredItemCount(*it));
			}

		}

		int WritePos = 0;
		WritePos += PutByte(&sim->SendBuf[WritePos], 1); //_handleQueryResultMsg
		WritePos += PutShort(&sim->SendBuf[WritePos], 0);      //Message size
		WritePos += PutInteger(&sim->SendBuf[WritePos], query->ID); //Query response index
		WritePos += PutShort(&sim->SendBuf[WritePos], 1);      //Array count
		WritePos += PutByte(&sim->SendBuf[WritePos],
				2 + requiredMaterials.size()); //String count
		sprintf(sim->Aux2, "%d", bookItemId);
		WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux2);  //ID
		sprintf(sim->Aux2, "%d", bookPages);
		WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux2);  //ID
		for (std::vector<int>::iterator it = requiredMaterials.begin();
				it != requiredMaterials.end(); ++it) {
			sprintf(sim->Aux2, "%d", *it);
			WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux2);  //ID
		}
		PutShort(&sim->SendBuf[1], WritePos - 3);             //Set message size
		return WritePos;
	}

	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Not a book item");
}
