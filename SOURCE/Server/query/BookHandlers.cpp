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

	QueryResponse resp(query->ID);
	InventoryManager inv = creatureInstance->charPtr->inventory;
	std::set<int> booksFound;
	PopulateBookList(resp, inv, INV_CONTAINER, booksFound);
	PopulateBookList(resp, inv, BOOKSHELF_CONTAINER, booksFound);
	return resp.Write(sim->SendBuf);
}

void BookListHandler::PopulateBookList(QueryResponse &response, InventoryManager &inv, int container, std::set<int> &booksFound) {

	for (size_t a = 0; a < inv.containerList[INV_CONTAINER].size(); a++) {
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
					auto row = response.Row();
					row->push_back(to_string(def.bookID));
					row->push_back(def.title);
					row->push_back(to_string(def.pages.size()));
				} else {
					g_Logs.server->warn(
							"Item %v refers to an invalid book, %v", ID,
							itemDef->mIvMax1);
				}
			}
		}
	}
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

	QueryResponse resp(query->ID);

	InventoryManager inv = creatureInstance->charPtr->inventory;
	std::set<int> pagesFoundSet;
	PopulateBookDetails(resp, inv, INV_CONTAINER, pagesFoundSet, def);
	PopulateBookDetails(resp, inv, BOOKSHELF_CONTAINER, pagesFoundSet, def);
	return resp.Write(sim->SendBuf);
}

void BookGetHandler :: PopulateBookDetails(QueryResponse &response, InventoryManager &inv, int container, std::set<int> &pagesFoundSet, BookDefinition &def) {
	for (size_t a = 0; a < inv.containerList[INV_CONTAINER].size(); a++) {
		ItemDef *itemDef =
				inv.containerList[INV_CONTAINER][a].ResolveSafeItemPtr();
		if (itemDef != NULL && itemDef->mType == ItemType::SPECIAL) {
			int pageNo = itemDef->GetDynamicMax(ItemIntegerType::BOOK_PAGE);
			if(pageNo == -1 && itemDef->GetDynamicMax(ItemIntegerType::BOOK) == def.bookID) {
				/* An item with ItemIntegerType::BOOK but NO ItemIntegerType::BOOK_PAGE is a complete book, so return all the pages */
				for (size_t i = 0; i < def.pages.size(); i++) {
					pagesFoundSet.insert(itemDef->mIvMax2);
					auto row = response.Row();
					row->push_back(to_string(i));
					row->push_back(def.pages[i]);
				}
			} else if (itemDef->GetDynamicMax(ItemIntegerType::BOOK)
					== def.bookID && pageNo > 0
					&& pagesFoundSet.find(pageNo) == pagesFoundSet.end()) {
				/* An item with ItemIntegerType::BOOK and a ItemIntegerType::BOOK_PAGE is book page, so return just the pages */
				pagesFoundSet.insert(itemDef->mIvMax2);

				auto row = response.Row();
				row->push_back(to_string(pageNo - 1));
				row->push_back(def.pages[pageNo - 1]);
			}
		}
	}
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
					"This page cannot be bound.");
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

		QueryResponse resp(query->ID);
		auto row = resp.Row();
		row->push_back(to_string(bookItemId));
		row->push_back(to_string(bookPages));
		for(auto mat : requiredMaterials) {
			row->push_back(to_string(mat));
		}
		return resp.Write(sim->SendBuf);
	}

	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Not a book item");
}
