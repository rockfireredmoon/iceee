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

#include "Query.h"
#include "TradeHandlers.h"
#include "../IGForum.h"
#include "../util/Log.h"
#include "../Instance.h"
#include "../VirtualItem.h"

//
// TradeShopHandler
//

int TradeShopHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: trade.shop
	 Sent when an item is purchased from a shop.
	 Args : [0] = Creature ID of the NPC

	 If buying from shop:
	 [1] = Item proto of selection (ex: item143548:0:0:0);

	 If buying from buyback:
	 [1] = Hex string of item container/slot ID

	 If selling
	 [1] = String constant: "sell"
	 [2] = Hex string of item container/slot ID
	 */

	if (query->argCount < 2)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

	if (pld->zoneDef->mGrove == true)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You cannot use shops in a grove.");

	//if(HasQueryArgs(2) == false)
	//	return 0;

	int CID = atoi(query->args[0].c_str());
	int CDef = sim->ResolveCreatureDef(CID);

	if (query->args[1].compare("sell") == 0) {
		//if(HasQueryArgs(3) == false)
		//	return 0;
		if (query->argCount < 3)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

		//[2] = Hex String, pass it to the helper function
		return helper_trade_shop_sell(sim, pld, query, creatureInstance, query->args[2].c_str());
	}

	//Since the Essence Shop scanning functions modify the string while searching
	//for tokens, copy it to a buffer here.
	Util::SafeCopy(sim->Aux1, query->args[1].c_str(), sizeof(sim->Aux1));

	EssenceShopItem *iptr = NULL;
	EssenceShop *esptr = creatureInstance->actInst->itemShopList.GetEssenceShopPtr(
			CDef, sim->Aux1, &iptr);
	if (esptr == NULL || iptr == NULL) {
		//This might be a buyback item.
		unsigned int CCSID = strtol(sim->Aux1, NULL, 16);
		int r = helper_trade_shop_buyback(sim, pld, query, creatureInstance,CCSID);
		if (r > 0)
			return r;

		g_Logs.simulator->error(
				"[%d] Failed to process Shop item [%s] for CreatureDef [%d]",
				sim->InternalID, sim->Aux1, CDef);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to determine item selection.");
	}

	ItemDef *itemPtr = g_ItemManager.GetPointerByID(iptr->ItemID);
	if (itemPtr == NULL)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
				"Server error: item does not exist.");

	int cost = (int) (itemPtr->mValue * g_VendorMarkup);
	if (creatureInstance->css.copper < cost)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
				"Not enough coin.");

	//Check to see if this was a gambled item, and modify its ID accordingly.
	int newItemID = 0;
	newItemID = g_ItemManager.RunPurchaseModifier(itemPtr->mID);

	//If the normal gamble options failed, it will return the same ID.  Check for new randomized gamble options instead.
	if (newItemID == itemPtr->mID) {
		VirtualItemSpawnParams viParam;
		if (g_ItemManager.CheckVirtualItemGamble(itemPtr,
				creatureInstance->css.level, &viParam, cost) == true) {
			//Coin amount may be modified.
			if (creatureInstance->css.copper < cost)
				return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
						"Not enough coin.");

			newItemID = g_ItemManager.RollVirtualItem(viParam);
		}
	}
	if (newItemID == 0)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
				"A purchase modifier has failed.");

	InventorySlot *newItem = pld->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
			newItemID, 1);
	if (newItem == NULL) {
		pld->charPtr->pendingChanges++;
		int err = pld->charPtr->inventory.LastError;
		if (err == InventoryManager::ERROR_ITEM)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"Server error: item does not exist.");
		else if (err == InventoryManager::ERROR_SPACE)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"You do not have any free inventory space.");
		else if (err == InventoryManager::ERROR_LIMIT)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"You already the maximum amount of these items.");
		else
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"Server error: undefined error.");
	} else {
		sim->ActivateActionAbilities(newItem);
	}

	int wpos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, newItem);
	creatureInstance->AdjustCopper(-cost);
	return wpos;
}

int TradeShopHandler::helper_trade_shop_sell(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance,
		const char *itemHex) {
	//Return the query response when selling stuff to an NPC shop.
	unsigned int CCSID = strtol(itemHex, NULL, 16);
	InventorySlot *item = pld->charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if (item == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item not found in inventory.");

	ItemDef *itemPtr = item->ResolveItemPtr();
	if (itemPtr == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item does not exist in server database.");

	int wpos = 0;
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");

	int cost = itemPtr->mValue;
	cost *= item->GetStackCount();
	wpos += pld->charPtr->inventory.AddBuyBack(item, &sim->SendBuf[wpos]);
	wpos += RemoveItemUpdate(&sim->SendBuf[wpos], sim->Aux3, item);
	pld->charPtr->inventory.RemItem(CCSID);
	pld->charPtr->pendingChanges++;
	creatureInstance->AdjustCopper(cost);

	return wpos;
}


int TradeShopHandler::helper_trade_shop_buyback(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance,unsigned int CCSID) {
	InventorySlot *buybackItem = pld->charPtr->inventory.GetItemPtrByCCSID(
			CCSID);
	if (buybackItem == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item not found in buyback list.");

	int freeslot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if (freeslot == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No free inventory space.");

	ItemDef *itemPtr = buybackItem->ResolveItemPtr();
	if (itemPtr == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item does not exist in database.");

	int cost = itemPtr->mValue * buybackItem->GetStackCount();
	if (creatureInstance->css.copper < cost)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Not enough coin.");

	InventorySlot newItem;
	newItem.CopyFrom(*buybackItem, false);
	newItem.CCSID = pld->charPtr->inventory.GetCCSID(INV_CONTAINER, freeslot);

	int r = pld->charPtr->inventory.AddItem(INV_CONTAINER, newItem);
	if (r == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to create item.");
	pld->charPtr->pendingChanges++;

	int wpos = 0;
	wpos = RemoveItemUpdate(sim->SendBuf, sim->Aux3, buybackItem);
	wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, &newItem);
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	pld->charPtr->inventory.RemItem(buybackItem->CCSID);
	pld->charPtr->pendingChanges++;
	creatureInstance->AdjustCopper(-cost);
	return wpos;
}
//
// TradeItemsHandler
//

int TradeItemsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: trade.items
	 A player has offered an item to trade.
	 Args : [variable] = List of items offered, Hex Inventory Item ID format.
	 */
	int rval = protected_helper_query_trade_items(sim, pld, query, creatureInstance);
	if (rval <= 0) {
		sim->SendInfoMessage(sim->GetErrorString(rval), INFOMSG_INFO);
		rval = PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	}
	return rval;
}

int TradeItemsHandler::protected_helper_query_trade_items(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int selfID = creatureInstance->CreatureID;
	int tradeID = creatureInstance->activeLootID;

	ActiveInstance *actInst = creatureInstance->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(
			tradeID);
	if (tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if (pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, sim->SendBuf);

	if (pData->otherPlayerData->tradeWindowOpen == false)
		return QueryErrorMsg::TRADENOTOPENED;

	InventorySlot item;
	pData->itemList.clear();
	for (unsigned int a = 0; a < query->argCount; a++) {
		unsigned long CCSID = strtol(query->args[a].c_str(), NULL, 16);
		InventorySlot *itemPtr = pld->charPtr->inventory.GetItemPtrByCCSID(
				CCSID);
		if (itemPtr == NULL)
			return QueryErrorMsg::INVALIDITEM;

		item.CopyFrom(*itemPtr, false);
		item.CCSID = CCSID;
		pData->itemList.push_back(item);
	}

	CreatureInstance *cInst = pData->otherPlayerData->cInst;
	int wpos = PrepExt_TradeItemOffer(sim->SendBuf, sim->Aux3, selfID, pData->itemList);

	SendToOneSimulator(sim->SendBuf, wpos, cInst->simulatorPtr);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// TradeEssenceHandler
//

int TradeEssenceHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: trade.essence
	 Sent when an item is purchased from a chest using tokens or
	 essences, instead of gold.
	 Args : [0] = Creature Instance ID
	 [1] = Item proto of the player's selection.
	 */

	// [0] = Creature Instance ID
	// [1] = Item Proto that was selected ex: "item143548:0:0:0"
	if (query->argCount < 2)
		return 0;

	int CID = atoi(query->args[0].c_str());
	int CDef = sim->ResolveCreatureDef(CID);

	//Since the Essence Shop scanning functions modify the string while searching
	//for tokens, copy it to a buffer here.
	Util::SafeCopy(sim->Aux1, query->args[1].c_str(), sizeof(sim->Aux1));

	EssenceShopItem *iptr = NULL;
	EssenceShop *esptr =
			creatureInstance->actInst->essenceShopList.GetEssenceShopPtr(CDef, sim->Aux1,
					&iptr);
	if (esptr == NULL || iptr == NULL) {
		g_Logs.simulator->error(
				"[%d] Failed to process EssenceShop item [%s] for CreatureDef [%d]",
				sim->InternalID, sim->Aux1, CDef);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to determine item selection.");
	}

	InventoryManager &inv = pld->charPtr->inventory;
	int currentItemCount = inv.GetItemCount(INV_CONTAINER, esptr->EssenceID);
	if (currentItemCount < iptr->EssenceCost) {
		g_Logs.simulator->warn("[%v] Essence requirement for item %v: %v / %v",
				sim->InternalID, esptr->EssenceID, currentItemCount,
				iptr->EssenceCost);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
				"You do not have enough essences.");
	}

	InventorySlot *newItem = pld->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
			iptr->ItemID, 1);
	if (newItem == NULL) {
		pld->charPtr->pendingChanges++;
		int err = pld->charPtr->inventory.LastError;
		if (err == InventoryManager::ERROR_ITEM)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"Server error: item does not exist.");
		else if (err == InventoryManager::ERROR_SPACE)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"You do not have any free inventory space.");
		else if (err == InventoryManager::ERROR_LIMIT)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"You already the maximum amount of these items.");
		else
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"Server error: undefined error.");
	}

	sim->ActivateActionAbilities(newItem);

	STRINGLIST result;
	result.push_back("OK");
	sprintf(sim->Aux3, "%d", iptr->EssenceCost);
	result.push_back(sim->Aux3);
	int wpos = PrepExt_QueryResponseStringList(sim->SendBuf, query->ID, result);
	wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, newItem);

	wpos += inv.RemoveItemsAndUpdate(INV_CONTAINER, esptr->EssenceID,
			iptr->EssenceCost, &sim->SendBuf[wpos]);
	pld->charPtr->pendingChanges++;
	return wpos;
}

//
// TradeStartHandler
//

int TradeStartHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: trade.start
	 Attempt to open a trade with another player.
	 Args : [0] = Creature ID of the player to trade with.
	 */
	int rval = protected_helper_query_trade_start(sim, pld, query,
			creatureInstance);
	if (rval <= 0) {
		rval = PrepExt_SendInfoMessage(sim->SendBuf,
				sim->GetErrorString(rval), INFOMSG_ERROR);
		rval += PrepExt_QueryResponseError(&sim->SendBuf[rval], query->ID,
				"");
	}

	return rval;
}

int TradeStartHandler::protected_helper_query_trade_start(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	// Note: query errors don't seem to do anything in the client.

	// Note: The trade starter has an argument for the target player.
	//       When the target player accepts, there is no argument to this
	//       query.

	int selfPlayerID = creatureInstance->CreatureID;
	int otherPlayerID = 0;
	if (query->argCount > 0)
		otherPlayerID = query->GetInteger(0);

	ActiveInstance *actInst = creatureInstance->actInst;
	if (actInst == NULL) {
		g_Logs.simulator->error(
				"[%v] trade.start active instance is NULL", sim->InternalID);
		return QueryErrorMsg::INVALIDOBJ;
	}

	CreatureInstance *target = NULL;
	if (otherPlayerID != 0) {
		//Make sure we're not busy.
		if (sim->CanMoveItems() == false)
			return QueryErrorMsg::SELFBUSYSKILL;

		//Make sure the other player exists
		target = actInst->GetPlayerByID(otherPlayerID);
		if (target == NULL)
			return QueryErrorMsg::INVALIDOBJ;

		//Make sure the other player isn't busy
		if (actInst->tradesys.GetExistingTradeForPlayer(otherPlayerID) != NULL)
			return QueryErrorMsg::TRADEBUSY;

		if (target->simulatorPtr->CanMoveItems() == false)
			return QueryErrorMsg::OTHERBUSYSKILL;
	}

	//It only takes one player to start the trade for both, so initialize a
	//transaction for both players.
	int CheckID = creatureInstance->activeLootID;
	if (CheckID == 0)
		CheckID = selfPlayerID;
	TradeTransaction *tradeData = actInst->tradesys.GetNewTransaction(CheckID);
	//tradeData->Clear();
	bool initiator = false;
	if (otherPlayerID != 0) {
		initiator = true;
		tradeData->SetPlayers(creatureInstance, target);
		creatureInstance->activeLootID = CheckID;
		target->activeLootID = CheckID;
		tradeData->init = true;
	} else {
		//This is the accepting player, so the target must be the origin.
		otherPlayerID = tradeData->player[0].selfPlayerID;
		target = tradeData->player[0].cInst;
	}

	if (target == NULL || otherPlayerID == 0)
		return QueryErrorMsg::INVALIDOBJ;

	tradeData->GetPlayerData(selfPlayerID)->tradeWindowOpen = true;

	if (initiator == true) {
		// Send trade request message to the other player.
		int wpos = 0;
		wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);
		wpos += PutInteger(&sim->SendBuf[wpos], selfPlayerID);   //traderID
		wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST); //eventType
		PutShort(&sim->SendBuf[1], wpos - 3);
		SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
	} else {
		int wpos = 0;
		wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);
		wpos += PutInteger(&sim->SendBuf[wpos], selfPlayerID);   //traderID
		wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST_ACCEPTED); //eventType
		PutShort(&sim->SendBuf[1], wpos - 3);
		SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// TradeCancelHandler
//

int TradeCancelHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: trade.cancel
	 A player has cancelled the trade.
	 Args : [none]
	 */
	int rval = protected_helper_query_trade_cancel(sim, pld, query, creatureInstance);
	if (rval <= 0)
		rval = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

int TradeCancelHandler::protected_helper_query_trade_cancel(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int selfID = creatureInstance->CreatureID;
	int tradeID = creatureInstance->activeLootID;
	ActiveInstance *actInst = creatureInstance->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(
			tradeID);
	if (tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;
	actInst->tradesys.CancelTransaction(selfID, tradeID, sim->SendBuf);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// TradeOfferHandler
//

int TradeOfferHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: trade.offer
	 A player has offered a coin amount.
	 Args : [none]
	 */
	int rval = protected_helper_query_trade_offer(sim, pld, query, creatureInstance);
	if (rval <= 0) {
		sim->SendInfoMessage(sim->GetErrorString(rval), INFOMSG_INFO);
		rval = PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	}

	return rval;
}

int TradeOfferHandler::protected_helper_query_trade_offer(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int selfID = creatureInstance->CreatureID;
	int tradeID = creatureInstance->activeLootID;
	ActiveInstance *actInst = creatureInstance->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(
			tradeID);
	if (tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if (pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, sim->SendBuf);

	CreatureInstance *cInst = pData->otherPlayerData->cInst;

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&sim->SendBuf[wpos], creatureInstance->CreatureID);   //traderID
	wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::OFFER_MADE);    //eventType
	PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size

	SendToOneSimulator(sim->SendBuf, wpos, cInst->simulatorPtr);
	//actInst->LSendToOneSimulator(SendBuf, wpos, cInst->SimulatorIndex);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// TradeAcceptHandler
//

int TradeAcceptHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: trade.accept
	 A player has accepted the trade offer.
	 Args : [none]
	 */
	int rval = protected_helper_query_trade_accept(sim, pld, query, creatureInstance);
	if (rval <= 0)
		rval = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

int TradeAcceptHandler::protected_helper_query_trade_accept(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int selfID = creatureInstance->CreatureID;
	int tradeID = creatureInstance->activeLootID;
	ActiveInstance *actInst = creatureInstance->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(
			tradeID);
	if (tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if (pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, sim->SendBuf);

	CreatureInstance *cInst = pData->otherPlayerData->cInst;

	pData->SetAccepted(true);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&sim->SendBuf[wpos], creatureInstance->CreatureID);   //traderID
	wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::OFFER_ACCEPTED); //eventType
	PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size
	SendToOneSimulator(sim->SendBuf, wpos, cInst->simulatorPtr);
	//actInst->LSendToOneSimulator(SendBuf, wpos, cInst->SimulatorIndex);

	CreatureInstance *origin = tradeData->player[0].cInst;
	CreatureInstance *target = tradeData->player[1].cInst;
	if (origin == NULL || target == NULL)
		return QueryErrorMsg::INVALIDOBJ;

	if (tradeData->MutualAccept() == true) {
		//Process the trade.
		int wpos = 0;

		//When counting slots, get the currently free slots.
		//Then add the number of items that would be traded (given away).
		//This allows a currently full inventory to potentially receive items
		//after the transaction is processed.

		int oslots = origin->charPtr->inventory.CountFreeSlots(INV_CONTAINER);
		oslots += tradeData->player[0].itemList.size();

		int tslots = target->charPtr->inventory.CountFreeSlots(INV_CONTAINER);
		tslots += tradeData->player[1].itemList.size();

		if (oslots < (int) tradeData->player[1].itemList.size()) {
			//Origin player does not have enough space to receive items.
			wpos = 0;
			wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&sim->SendBuf[wpos], origin->CreatureID);   //traderID
			wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED); //eventType
			wpos += PutByte(&sim->SendBuf[wpos], CloseReasons::INSUFFICIENT_SPACE); //eventType
			PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Logs.simulator->debug("[%v] Origin lacks space", sim->InternalID);
			goto exit;
		}
		if (tslots < (int) tradeData->player[0].itemList.size()) {
			//Target player does not have enough space to receive items.
			wpos = 0;
			wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&sim->SendBuf[wpos], target->CreatureID);   //traderID
			wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED); //eventType
			wpos += PutByte(&sim->SendBuf[wpos], CloseReasons::INSUFFICIENT_SPACE); //eventType
			PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Logs.simulator->debug("[%v] Target lacks space", sim->InternalID);
			goto exit;
		}

		//Check that each player has the required currencies.
		if (tradeData->player[0].coin > origin->css.copper) {
			wpos = 0;
			wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&sim->SendBuf[wpos], origin->CreatureID);   //traderID
			wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED); //eventType
			wpos += PutByte(&sim->SendBuf[wpos], CloseReasons::INSUFFICIENT_FUNDS); //eventType
			PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Logs.simulator->debug("[%v] Origin lacks copper", sim->InternalID);
			goto exit;
		}
		if (tradeData->player[1].coin > target->css.copper) {
			wpos = 0;
			wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&sim->SendBuf[wpos], target->CreatureID);   //traderID
			wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED); //eventType
			wpos += PutByte(&sim->SendBuf[wpos], CloseReasons::INSUFFICIENT_FUNDS); //eventType
			PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Logs.simulator->debug("[%v] Target lacks copper", sim->InternalID);
			goto exit;
		}

		//Ready to trade.
		g_Logs.simulator->debug("[%v] Trade requirements passed", sim->InternalID);

		//Adjust and send coin transfer to both players.
		origin->css.copper -= tradeData->player[0].coin;
		target->css.copper -= tradeData->player[1].coin;

		origin->css.copper += tradeData->player[1].coin;
		target->css.copper += tradeData->player[0].coin;

		static const short statSend = STAT::COPPER;
		wpos = PrepExt_SendSpecificStats(sim->SendBuf, origin, &statSend, 1);
		SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);

		wpos = PrepExt_SendSpecificStats(sim->SendBuf, target, &statSend, 1);
		SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);

		//Adjust and send items for first player.

		//Remove items from first player.
		wpos = 0;
		CharacterData *p1 = origin->charPtr;
		CharacterData *p2 = target->charPtr;
		g_Logs.simulator->debug("[%v] Trade betweeen [%v] and [%v]",
				sim->InternalID, p1->cdef.css.display_name, p2->cdef.css.display_name);
		for (size_t a = 0; a < tradeData->player[0].itemList.size(); a++) {
			unsigned long CCSID = tradeData->player[0].itemList[a].CCSID;
			InventorySlot *item = p1->inventory.GetItemPtrByCCSID(CCSID);
			if (item == NULL) {
				g_Logs.simulator->error(
						"[%v] Failed to remove item from first player.", sim->InternalID);
			} else {
				wpos += p1->inventory.RemoveItemUpdate(&sim->SendBuf[wpos], sim->Aux3,
						item);
				p1->inventory.RemItem(CCSID);
				p1->pendingChanges++;
			}
		}
		SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
		g_Logs.simulator->debug("[%v] Removed %v items from first player.", sim->InternalID,
				tradeData->player[0].itemList.size());

		//Remove items from second player.
		wpos = 0;
		for (size_t a = 0; a < tradeData->player[1].itemList.size(); a++) {
			unsigned long CCSID = tradeData->player[1].itemList[a].CCSID;
			InventorySlot *item = p2->inventory.GetItemPtrByCCSID(CCSID);
			if (item == NULL) {
				g_Logs.simulator->error(
						"[%v] Failed to remove item from first player.", sim->InternalID);
			} else {
				wpos += p2->inventory.RemoveItemUpdate(&sim->SendBuf[wpos], sim->Aux3,
						item);
				p2->inventory.RemItem(CCSID);
				p2->pendingChanges++;
			}
		}
		SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
		//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
		g_Logs.simulator->debug("[%v] Removed %v items from second player.", sim->InternalID,
				tradeData->player[1].itemList.size());

		//Give items to first player
		wpos = 0;
		for (size_t a = 0; a < tradeData->player[1].itemList.size(); a++) {
			int itemID = tradeData->player[1].itemList[a].IID;
			int count = tradeData->player[1].itemList[a].count + 1;
			InventorySlot *item = p1->inventory.AddItem_Ex(INV_CONTAINER,
					itemID, count);
			if (item == NULL)
				g_Logs.simulator->error("[%v] Failed to add item to first player.", sim->InternalID);
			else {
				p1->pendingChanges++;
				g_Logs.event->info("[TRADE] From %v to %v (%v)",
						tradeData->player[1].cInst->css.display_name,
						tradeData->player[0].cInst->css.display_name,
						item->IID);
				item->CopyWithoutCount(tradeData->player[1].itemList[a], false);
				sim->ActivateActionAbilities(item);
				wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, item);
			}
		}
		SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
		g_Logs.simulator->debug("[%v] Gave %v items to first player.", sim->InternalID,
				tradeData->player[1].itemList.size());

		//Give items to second player
		wpos = 0;
		for (size_t a = 0; a < tradeData->player[0].itemList.size(); a++) {
			int itemID = tradeData->player[0].itemList[a].IID;
			int count = tradeData->player[0].itemList[a].count + 1;
			InventorySlot *item = p2->inventory.AddItem_Ex(INV_CONTAINER,
					itemID, count);
			if (item == NULL)
				g_Logs.simulator->error("[%v] Failed to add item to second player.", sim->InternalID);
			else {
				p2->pendingChanges++;
				g_Logs.event->info("[TRADE] From %v to %v (%v)",
						tradeData->player[0].cInst->css.display_name,
						tradeData->player[1].cInst->css.display_name,
						item->IID);
				item->CopyWithoutCount(tradeData->player[0].itemList[a], false);
				sim->ActivateActionAbilities(item);
				wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, item);
			}
		}
		SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
		//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
		g_Logs.simulator->debug("[%v] Gave %v items to second player.", sim->InternalID,
				tradeData->player[0].itemList.size());

		//Send trade completion message.
		wpos = 0;
		wpos += PutByte(&sim->SendBuf[wpos], 51);    //_handleTradeMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);    //Placeholder for size
		wpos += PutInteger(&sim->SendBuf[wpos], origin->CreatureID);     //traderID
		wpos += PutByte(&sim->SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED); //eventType
		wpos += PutByte(&sim->SendBuf[wpos], CloseReasons::COMPLETE);     //eventType
		PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size

		SendToOneSimulator(sim->SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);

		PutInteger(&sim->SendBuf[3], target->CreatureID);     //traderID
		SendToOneSimulator(sim->SendBuf, wpos, target->simulatorPtr);
		//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);

		//Clear trade IDs.
		origin->activeLootID = 0;
		target->activeLootID = 0;

		g_Logs.simulator->debug("[%v] Trade complete", sim->InternalID);
		actInst->tradesys.RemoveTransaction(tradeID);
	}

	//Yes, I'm using goto.
	//Yes, I know this whole thing is badly programmed.
	//Deal with it.

	exit: return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// TradeCurrencyHandler
//

int TradeCurrencyHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: trade.currency
	 Sent when the player offers a coin amount.
	 Args : [0] currency name, ex: "COPPER"
	 [1] amount offered
	 */
	int rval = protected_helper_query_trade_currency(sim, pld, query, creatureInstance);
	if (rval <= 0)
		rval = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

int TradeCurrencyHandler::protected_helper_query_trade_currency(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 2)
		return QueryErrorMsg::GENERIC;

	//Ignore currency type for now, assume always copper.  Just get the amount.
	// Note: I found no evidence that the credit currency could actually be used
	// in trades.
	//args[0] is usually COPPER
	int amount = strtol(query->args[1].c_str(), NULL, 10);

	int selfID = creatureInstance->CreatureID;
	int tradeID = creatureInstance->activeLootID;
	ActiveInstance *actInst = creatureInstance->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(
			tradeID);
	if (tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if (pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, sim->SendBuf);

	CreatureInstance *cInst = pData->otherPlayerData->cInst;

	pData->SetCoin(amount);

	if (tradeData->player->otherPlayerData->tradeWindowOpen == true) {
		int wpos = PrepExt_TradeCurrencyOffer(sim->SendBuf, selfID, amount);
		SendToOneSimulator(sim->SendBuf, wpos, cInst->simulatorPtr);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
