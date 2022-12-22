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

#include "LootHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Random.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//LootListHandler
//

int LootListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: loot.list
	 Retrieves a list of loot for a particular dead creature.
	 Args : [0] = Creature Instance ID.
	 */

	if (query->argCount < 1)
		return 0;

	int CreatureID = query->GetInteger(0);
	int r = creatureInstance->actInst->lootsys.GetCreature(CreatureID);
	if (r == -1)
		return PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
				"Creature does not have any loot.");
	else
		return creatureInstance->actInst->lootsys.WriteLootQueryToBuffer(r,
				sim->SendBuf, sim->Aux2, query->ID);

}

//
//LootItemHandler
//

int LootItemHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: loot.item
	 Loots a particular item off a creature.
	 Args : [0] = Creature Instance ID
	 [1] = Item Definition ID
	 */
	if (query->argCount >= 2) {

		int CID = atoi(query->args[0].c_str());
		int ItemID = atoi(query->args[1].c_str());
		int conIndex = 0;
		int result = protected_helper_query_loot_item(sim, CID, ItemID, creatureInstance, pld, conIndex);
		if (result < 0)
			return PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
					sim->GetErrorString(result));

//		creatureInstance->actInst->LSendToOneSimulator(
//				sim->SendBuf, result, sim);

		// Always response to the looter
		STRINGLIST qresponse;
		qresponse.push_back("OK");
		sprintf(sim->Aux3, "%d", conIndex);
		qresponse.push_back(sim->Aux3);
		return PrepExt_QueryResponseStringList(&sim->SendBuf[result], query->ID,
				qresponse);
	}
	return 0;
}

//
//LootExitHandler
//

int LootExitHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: loot.exit
	 Signals the server that the loot window has been closed.
	 The server currently does not process this information.
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//LootNeedGreedPassHandler
//

int LootNeedGreedPassHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: loot.need
	 Signals players interested in being needy.
	 Args : [0] = Loot tag
	 */

	if (query->argCount < 2)
		return 0;

	int WritePos = 0;
	WritePos = protected_helper_query_loot_need_greed_pass(sim, pld, query, creatureInstance);
	if (WritePos <= 0)
		WritePos = PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
				sim->GetErrorString(WritePos));
	return WritePos;
}


void CheckIfLootReadyToDistribute(SimulatorThread *sim, CreatureInstance *creatureInstance, ActiveLootContainer *loot,
		LootTag lootTag) {

	/* NOTE: We expect this method to do it's own sending of the buffer (SendBuf) to whoever. The return
		 *       value is for errors only.
		 */

	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInstance->PartyID);
	bool needOrGreed = (party->mLootFlags & NEED_B4_GREED) > 0;

	// How many decisions do we expect to process the roll. This will depend
	// on if this is the secondary roll when round robin or loot master is in use
	// has been processed or not
	unsigned int requiredDecisions = party->GetOnlineMemberCount();
	if ((party->mLootMode == ROUND_ROBIN || party->mLootMode == LOOT_MASTER)) {
		if (loot->IsStage2(lootTag.mItemId)) {
			requiredDecisions = party->GetOnlineMemberCount() - 1;
		} else {
			requiredDecisions = 1;
		}
	}
	unsigned int decisions = (unsigned int) loot->CountDecisions(
			lootTag.mItemId);
	g_Logs.simulator->info("[%v] Loot requires %v decisions, we have %v",
			sim->InternalID, requiredDecisions, decisions);

	if (decisions >= requiredDecisions) {
		g_Logs.simulator->info("[%v] Loot %v is ready to distribute",
				sim->InternalID, lootTag.mItemId);

		CreatureInstance *receivingCreature = NULL;

		/*
		 * If the loot mode is loot master, and this roll was from the leader, then
		 * either give them the item, or offer again to the rest of the party depending
		 * on whether they needed or not
		 */
		if (!loot->IsStage2(lootTag.mItemId) && party->mLootMode == LOOT_MASTER
				&& party->mLeaderID == creatureInstance->CreatureID) {
			g_Logs.simulator->info(
					"[%v] Got loot roll from party leader %v for %v",
					sim->InternalID, party->mLeaderID, lootTag.mItemId);
			if (loot->IsNeeded(lootTag.mItemId, creatureInstance->CreatureID)
					|| loot->IsGreeded(lootTag.mItemId,
							creatureInstance->CreatureID)) {
				g_Logs.simulator->info("[%v] Leader %v needed for %v",
						sim->InternalID, party->mLeaderID, lootTag.mItemId);
				receivingCreature = creatureInstance;
			} else {
				// Offer again to the rest of the party
				int iid = lootTag.mItemId;
				int cid = lootTag.mCreatureId;
				int lcid = lootTag.mLootCreatureId;
				g_Logs.simulator->info("[%v] Offering %v to rest of party",
						sim->InternalID, iid);
				loot->SetStage2(iid, true);
				loot->RemoveCreatureRolls(iid, cid);
				party->RemoveCreatureTags(iid, cid);


//				int OfferLoot(int mode, ActiveLootContainer *loot,
//						ActiveParty *party, CreatureInstance *receivingCreature, int ItemID,
//						bool needOrGreed, int CID, int conIndex) {

				if (OfferLoot(sim, -1, loot, party, creatureInstance, iid, needOrGreed,
						lcid, 0) == QueryErrorMsg::GENERIC) {
					// Nobody to offer to, clean up as if the item had never been looted
					g_Logs.simulator->info(
							"[%v] Nobody to offer loot in %v to, cleaning up as if not yet looted.",
							sim->InternalID, lcid);
					ResetLoot(party, loot, iid);
				}
				return;
			}
		}

		/*
		 * If the loot mode is round robin, and this roll was from them, then
		 * either give them the item, or offer again to the rest of the party depending
		 * on whether they needed or not
		 */
		if (!loot->IsStage2(lootTag.mItemId) && party->mLootMode == ROUND_ROBIN
				&& loot->robinID == creatureInstance->CreatureID) {
			g_Logs.simulator->info("[%v] Got loot roll from robin %v for %v",
					sim->InternalID, loot->robinID, lootTag.mItemId);
			if (loot->IsNeeded(lootTag.mItemId, creatureInstance->CreatureID)
					|| loot->IsGreeded(lootTag.mItemId,
							creatureInstance->CreatureID)) {
				g_Logs.simulator->info("[%v] Robin %v needed or greeded for %v",
						sim->InternalID, loot->robinID, lootTag.mItemId);
				receivingCreature = creatureInstance;
			} else {
				// Offer again to the rest of the party
				int iid = lootTag.mItemId;
				int cid = lootTag.mCreatureId;
				int lcid = lootTag.mLootCreatureId;
				g_Logs.simulator->info(
						"[%v] Robin passed, offering %d to rest of party",
						sim->InternalID, iid);
				loot->SetStage2(iid, true);
				loot->RemoveCreatureRolls(iid, cid);
				party->RemoveCreatureTags(iid, cid);
				if (OfferLoot(sim, -1, loot, party, creatureInstance, iid, needOrGreed,
						lcid, 0) == QueryErrorMsg::GENERIC) {
					// Nobody to offer to, clean up as if the item had never been looted
					g_Logs.simulator->info(
							"[%v] Nobody to offer loot in %d to, cleaning up as if not yet looted.",
							sim->InternalID, lcid);
					ResetLoot(party, loot, iid);
				}
				return;
			}
		}

		if (receivingCreature == NULL) {
			// No specific creature, first pick one of the needers if any
			set<int> needers = loot->needed[lootTag.mItemId];
			if (needers.size() > 0) {
				g_Logs.simulator->info("[%v] Rolling for %v needers",
						sim->InternalID, needers.size());
				receivingCreature = RollForPartyLoot(sim, party, needers, "Need",
						lootTag.mItemId)->mCreaturePtr;
			} else {
				set<int> greeders = loot->greeded[lootTag.mItemId];
				if (greeders.size() > 0) {
					g_Logs.simulator->info("[%v] Rolling for %v greeders",
							sim->InternalID, greeders.size());
					receivingCreature = RollForPartyLoot(sim, party, greeders,
							"Greed", lootTag.mItemId)->mCreaturePtr;
				}
			}
		}

		if (receivingCreature == NULL) {
			g_Logs.simulator->info("[%v] Everybody passed on loot %v",
					sim->InternalID, lootTag.mItemId);
			// Send a winner with a tag of '0'. This will close the window
			for (unsigned int i = 0; i < party->mMemberList.size(); i++) {
				// Skip the loot master or robin

				LootTag tag =
						party->mMemberList[i].IsOnlineAndValid() ?
								party->GetTag(lootTag.mItemId,
										party->mMemberList[i].mCreaturePtr->CreatureID) :
								NULL;
				if (tag.Valid()) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d:%d",
							tag.mCreatureId, tag.mSlotIndex);
					Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", 0);
					int WritePos = PartyManager::WriteLootWin(sim->SendBuf, sim->Aux2, "0",
							"Nobody", lootTag.mCreatureId, 999);
					party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(
							sim->SendBuf, WritePos,
							party->mMemberList[i].mCreaturePtr->simulatorPtr);
				}
			}
			ResetLoot(party, loot, lootTag.mItemId);
			return;
		}

		InventorySlot *newItem = NULL;

		// Send the actual winner to all of the party that have a tag
		LootTag winnerTag = party->GetTag(lootTag.mItemId,
				receivingCreature->CreatureID);
		for (unsigned int i = 0; i < party->mMemberList.size(); i++) {
			if (!party->mMemberList[i].IsOnlineAndValid()) {
				g_Logs.simulator->info(
						"[%v] Skipping informing %v of the winner (%v) as they have no simulator",
						sim->InternalID, party->mMemberList[i].mCreatureID,
						lootTag.mCreatureId);
				continue;
			}

			g_Logs.simulator->info("[%v] Informing %v of the winner (%v)",
					sim->InternalID, party->mMemberList[i].mCreaturePtr->CreatureID,
					lootTag.mCreatureId);
			LootTag tag = party->GetTag(lootTag.mItemId,
					party->mMemberList[i].mCreaturePtr->CreatureID);
			if (tag.Valid()) {
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d:%d", tag.mCreatureId,
						tag.mSlotIndex);
				Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", tag.lootTag);
				int WritePos = PartyManager::WriteLootWin(sim->SendBuf, sim->Aux2, sim->Aux3,
						receivingCreature->css.display_name,
						lootTag.mCreatureId, 999);
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(
						sim->SendBuf, WritePos,
						party->mMemberList[i].mCreaturePtr->simulatorPtr);
			} else {
				g_Logs.simulator->warn(
						"[%v] No tag for item %v for a player %d to be informed",
						sim->InternalID, lootTag.mItemId,
						party->mMemberList[i].mCreaturePtr->CreatureID);
			}
		}

		// Update the winners inventory
		CharacterData *charData = receivingCreature->charPtr;
		int slot = charData->inventory.GetFreeSlot(INV_CONTAINER);
		if (slot == -1) {
			Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
					"%s doesn't have enough space. Starting bidding again",
					receivingCreature->css.display_name);
			party->BroadcastInfoMessageToAllMembers(sim->Aux3);
			g_Logs.simulator->warn("[%v] Receive (%v) has no slots.",
					sim->InternalID, receivingCreature->CreatureID);
			ResetLoot(party, loot, lootTag.mItemId);
			return;
		} else {
			newItem = charData->inventory.AddItem_Ex(INV_CONTAINER,
					winnerTag.mItemId, 1);
			if (newItem == NULL) {
				g_Logs.simulator->warn(
						"[%v] Item to loot (%v) has disappeared.", sim->InternalID,
						winnerTag.mItemId);
				ResetLoot(party, loot, lootTag.mItemId);
				return;
			}
			charData->pendingChanges++;
			sim->ActivateActionAbilities(newItem);
		}

		int conIndex = loot->HasItem(lootTag.mItemId);
		if (conIndex == -1) {
			g_Logs.simulator->warn("[%v] Item to loot (%v) missing.",
					sim->InternalID, lootTag.mItemId);
		} else {
			// Remove the loot from the container
			loot->RemoveItem(conIndex);

			g_Logs.simulator->warn(
					"[%v] There is now %v items in loot container %v.",
					sim->InternalID, loot->itemList.size(), loot->CreatureID);

			/* Reset the loot tags etc, we don't need them anymore.
			 * NOTE: Be careful not to use the lootTag object after this point as it may have been
			 * deleted.
			 */
			int lootCreatureID = lootTag.mLootCreatureId;
			ResetLoot(party, loot, lootTag.mItemId);

			if (loot->itemList.size() == 0) {
				ClearLoot(party, loot);

				// Loot container now empty, remove it
				CreatureInstance *lootCreature =
						creatureInstance->actInst->GetNPCInstanceByCID(
								lootCreatureID);
				if (lootCreature != NULL) {
					lootCreature->activeLootID = 0;
					lootCreature->css.ClearLootSeeablePlayerIDs();
					lootCreature->css.ClearLootablePlayerIDs();
					lootCreature->_RemoveStatusList(StatusEffects::IS_USABLE);
					lootCreature->css.appearance_override =
							LootSystem::DefaultTombstoneAppearanceOverride;
					static const short statList[3] = {
							STAT::APPEARANCE_OVERRIDE,
							STAT::LOOTABLE_PLAYER_IDS,
							STAT::LOOT_SEEABLE_PLAYER_IDS };
					int WritePos = PrepExt_SendSpecificStats(sim->SendBuf, lootCreature,
							&statList[0], 3);
					creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf,
							WritePos, creatureInstance->CurrentX,
							creatureInstance->CurrentZ);
				}
				creatureInstance->actInst->lootsys.RemoveCreature(lootCreatureID);

				g_Logs.simulator->warn(
						"[%v] Loot %v is now empty (%v tags now in the party).",
						sim->InternalID, loot->CreatureID, party->lootTags.size());
			}

			if (newItem != NULL && receivingCreature != NULL)
				// Send an update to the actual of the item
				receivingCreature->actInst->LSendToOneSimulator(sim->SendBuf,
						AddItemUpdate(sim->SendBuf, sim->Aux3, newItem),
						receivingCreature->simulatorPtr);
		}
	} else {
		g_Logs.simulator->info("[%v] Loot %v not ready yet to distribute",
				sim->InternalID, lootTag.mItemId);
	}
}

int protected_helper_query_loot_item(SimulatorThread *sim, int CID, int ItemID, CreatureInstance *creatureInstance, CharacterServerData *pld, int &conIndex) {
	// 1. Free for all + Need or greed OFF. Loot will go to the looter
	// 2. Free for all + Need or greed ON. Loot will be offered to all, player is picked accordig to greed roll
	// 3. Round Robin + Need or greed OFF. Loot will be offered to robin, then party to greed or pass.
	// 4. Round Robin + Need or greed ON. Loot will be offered to robin, then party to need, greed or pass.
	// 5. Loot Master + Need or greed OFF. Loot will be offered to leader, then party to greed or pass.
	// 6. Loot Master + Need or greed ON. Loot will be offered to leader, then party to need, greed or pass.

	//LogMessageL(MSG_SHOW, "loot.item: %d, %d", CID, ItemID);

	CreatureInstance *receivingCreature = creatureInstance;
	CharacterData *charData = pld->charPtr;
	ItemDef *itemDef = g_ItemManager.GetPointerByID(ItemID);

	/* If the player is in a party, check the party loot rules.
	 */
	ActiveParty *party = g_PartyManager.GetPartyByID(
			receivingCreature->PartyID);
	bool partyLootable =
			party != NULL
					&& (itemDef->mQualityLevel >= 2
							|| (party->mLootFlags & MUNDANE) > 0);
	bool needOrGreed = partyLootable && (party->mLootFlags & NEED_B4_GREED) > 0;

	ActiveInstance *aInst = receivingCreature->actInst;

	//The client uses creature definitions as lootable IDs.
	int PlayerCDefID = receivingCreature->CreatureDefID;

	// Make sure this object isn't too far away.
	int distCheck = sim->protected_CheckDistance(CID);
	if (distCheck != 0)
		return distCheck;

	int r = aInst->lootsys.GetCreature(CID);
	if (r == -1)
		return QueryErrorMsg::LOOT;

	ActiveLootContainer *loot = &aInst->lootsys.creatureList[r];

	int canLoot = loot->HasLootableID(PlayerCDefID);
	if (canLoot == -1)
		return QueryErrorMsg::LOOTDENIED;

	conIndex = loot->HasItem(ItemID);
	if (conIndex == -1)
		return QueryErrorMsg::LOOTMISSING;

	// Check if already waiting on rolls
	if (partyLootable && party->HasTags(PlayerCDefID, ItemID)) {
		g_Logs.simulator->warn("[%v] Denying loot. Already looting %v",
				sim->InternalID, ItemID);
		return QueryErrorMsg::LOOTDENIED;
	}

	int WritePos = 0;

	// Offer the loot instead if appropriate
	if (partyLootable && !(party->mLootMode == FREE_FOR_ALL && !needOrGreed)) {
		g_Logs.simulator->info("[%v] Trying party loot for %v", sim->InternalID,
				ItemID);
		return OfferLoot(sim, party->mLootMode, loot, party, receivingCreature,
				ItemID, needOrGreed, CID, conIndex);
	} else {
		g_Logs.simulator->info("[%v] An ordinary single player loot %v",
				sim->InternalID, ItemID);

		// Either there is no party, or the loot rules decided that the looter just gets the item

		int slot = charData->inventory.GetFreeSlot(INV_CONTAINER);
		if (slot == -1)
			return QueryErrorMsg::INVSPACE;

		InventorySlot *newItem = charData->inventory.AddItem_Ex(INV_CONTAINER,
				ItemID, 1);
		if (newItem == NULL)
			return QueryErrorMsg::INVCREATE;

		charData->pendingChanges++;
		sim->ActivateActionAbilities(newItem);

		loot->RemoveItem(conIndex);
		if (loot->itemList.size() == 0) {
			CreatureInstance *lootCreature = aInst->GetNPCInstanceByCID(CID);
			if (lootCreature != NULL) {
				lootCreature->activeLootID = 0;
				lootCreature->css.ClearLootSeeablePlayerIDs();
				lootCreature->css.ClearLootablePlayerIDs();
				lootCreature->_RemoveStatusList(StatusEffects::IS_USABLE);
				lootCreature->css.appearance_override =
						LootSystem::DefaultTombstoneAppearanceOverride;
				static const short statList[3] =
						{ STAT::APPEARANCE_OVERRIDE, STAT::LOOTABLE_PLAYER_IDS,
								STAT::LOOT_SEEABLE_PLAYER_IDS };
				aInst->LSendToLocalSimulator(sim->SendBuf, PrepExt_SendSpecificStats(sim->SendBuf, lootCreature,
						&statList[0], 3), creatureInstance->CurrentX, creatureInstance->CurrentZ);
			}
			aInst->lootsys.RemoveCreature(CID);
		}

		WritePos = AddItemUpdate(sim->SendBuf, sim->Aux3, newItem);

	}
	return WritePos;
}

int LootNeedGreedPassHandler::protected_helper_query_loot_need_greed_pass(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInstance->PartyID);
	if (party == NULL)
		return QueryErrorMsg::GENERIC;

	if (query->argCount < 2)
		return QueryErrorMsg::GENERIC;

	ActiveInstance *aInst = creatureInstance->actInst;
	int lootTag = atoi(query->args[1].c_str());
	LootTag tag = party->lootTags[lootTag];
	if (!tag.Valid()) {
		g_Logs.simulator->warn("[%v] Loot tag missing %v", sim->InternalID, lootTag);
		return QueryErrorMsg::INVALIDITEM;
	}
	ActiveLootContainer *loot =
			&aInst->lootsys.creatureList[tag.mLootCreatureId];
	if (loot == NULL) {
		g_Logs.simulator->warn("[%v] Loot container missing %v", sim->InternalID,
				tag.mLootCreatureId);
		return QueryErrorMsg::INVALIDITEM;
	}
	g_Logs.simulator->info("[%v] %v is choosing on %v (%v / %v)", sim->InternalID,
			creatureInstance->CreatureID, lootTag, tag.mItemId, tag.mCreatureId,
			tag.mLootCreatureId);
	if (loot->HasAnyDecided(tag.mItemId, creatureInstance->CreatureID)) {
		g_Logs.simulator->warn("[%v] %v has already made loot decision on %v",
				sim->InternalID, creatureInstance->CreatureID, tag.mItemId);
		return QueryErrorMsg::LOOTDENIED;
	}
	if (tag.mCreatureId != creatureInstance->CreatureID) {
		g_Logs.simulator->warn(
				"[%d] Loot tag %d is for a different creature. The tag said %d, but this player is %d.",
				sim->InternalID, lootTag, tag.mCreatureId,
				creatureInstance->CreatureID);
		return QueryErrorMsg::LOOTDENIED;
	}

	const char *command = query->args[0].c_str();
	if (strcmp(command, "loot.need") == 0) {
		loot->Need(tag.mItemId, tag.mCreatureId);
	} else if (strcmp(command, "loot.greed") == 0) {
		loot->Greed(tag.mItemId, tag.mCreatureId);
	} else if (strcmp(command, "loot.pass") == 0) {
		loot->Pass(tag.mItemId, tag.mCreatureId);
	}
	CheckIfLootReadyToDistribute(sim, creatureInstance, loot, tag);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int OfferLoot(SimulatorThread *sim, int mode, ActiveLootContainer *loot,
		ActiveParty *party, CreatureInstance *receivingCreature, int ItemID,
		bool needOrGreed, int CID, int conIndex) {
	/* NOTE: We expect this method to do it's own sending of the buffer (SendBuf) to whoever. The return
	 *       value is for errors only.
	 */
	if (mode == ROUND_ROBIN) {
		// Offer to the robin first
		g_Logs.simulator->info("[%v] Offer Loot Round Robin", sim->InternalID);
		PartyMember *robin = party->GetNextLooter();

		loot->robinID = robin->mCreatureID;

		// Offer to the robin first
		LootTag tag = party->TagItem(ItemID, robin->mCreaturePtr->CreatureID,
				CID, 0);
//		int slot = robin->mCreaturePtr->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
//		tag->mSlotIndex = slot;
		Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", tag.lootTag);
		int WritePos = PartyManager::OfferLoot(sim->SendBuf, ItemID, sim->Aux3, false);
		if (receivingCreature->CreatureID == robin->mCreaturePtr->CreatureID) {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Round Robin to looter, so returning with this response",
					sim->InternalID);
		} else {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Round Robin to someone other than looter, so returning on its simulator",
					sim->InternalID);

//			ItemDef *item = g_ItemManager.GetSafePointerByID(tag->mItemId);
//			WritePos = PrepExt_ItemDef(SendBuf, item, ProtocolState);

		}

		robin->mCreaturePtr->actInst->LSendToOneSimulator(
				sim->SendBuf, WritePos,
				robin->mCreaturePtr->simulatorPtr);

		return QueryErrorMsg::NONE;
	}

	if (mode == LOOT_MASTER) {
		g_Logs.simulator->info("[%v] Offer Loot Master", sim->InternalID);
		// Offer to the leader first
		PartyMember *member = party->GetMemberByID(party->mLeaderID);
		unsigned int idx = 0;
		while ((member == NULL || !member->IsOnlineAndValid())
				&& idx < party->mMemberList.size()) {
			member = &party->mMemberList[idx++];
		}
		if (member == NULL || !member->IsOnlineAndValid()) {
			/* Nobody left! */
			return QueryErrorMsg::GENERIC;
		}
		int slot = member->mCreaturePtr->charPtr->inventory.GetFreeSlot(
				INV_CONTAINER);
		LootTag tag = party->TagItem(ItemID, member->mCreaturePtr->CreatureID,
				CID, slot);
		Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", tag.lootTag);
		int WriteIdx = PartyManager::OfferLoot(sim->SendBuf, ItemID, sim->Aux3, false);

		if (receivingCreature->CreatureID == member->mCreaturePtr->CreatureID) {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Master to looter, so returning with this response",
					sim->InternalID);
		} else {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Master to someone other than looter, so returning on its simulator",
					sim->InternalID);
		}
		member->mCreaturePtr->actInst->LSendToOneSimulator(sim->SendBuf,
				WriteIdx, member->mCreaturePtr->simulatorPtr);
		return QueryErrorMsg::NONE;
	}

	// First tags for other party members
	g_Logs.simulator->info("[%v] Offering to %v party member", sim->InternalID,
			party->mMemberList.size());
	int offers = 0;
	for (unsigned int i = 0; i < party->mMemberList.size(); i++) {
		if (!party->mMemberList[i].IsOnlineAndValid()) {
			g_Logs.simulator->info(
					"[%v] Skipping party member %v because they have no simulator",
					sim->InternalID, party->mMemberList[i].mCreatureID);
			continue;
		}

		if (receivingCreature == NULL
				|| party->mMemberList[i].mCreatureID
						!= receivingCreature->CreatureID) {
			// Only send offer to players in range
			int distCheck = sim->protected_CheckDistanceBetweenCreaturesFor(
					party->mMemberList[i].mCreaturePtr, CID, SimulatorThread::PARTY_LOOT_RANGE);
			if (distCheck == 0) {
				LootTag tag = party->TagItem(ItemID,
						party->mMemberList[i].mCreaturePtr->CreatureID, CID, 0);
				Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", tag.lootTag);
				g_Logs.simulator->info("Sending offer of %v to %v using tag %v",
						ItemID, party->mMemberList[i].mCreatureID, sim->Aux3);
				int WriteIdx = PartyManager::OfferLoot(sim->SendBuf, ItemID, sim->Aux3,
						needOrGreed);
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(
						sim->SendBuf, WriteIdx,
						party->mMemberList[i].mCreaturePtr->simulatorPtr);
				offers++;
			} else {
				g_Logs.event->info(
						"%v is too far away from %d to receive loot (%v)",
						party->mMemberList[i].mCreaturePtr->CreatureID, CID,
						distCheck);
			}
		}
	}

	// Now the tag for the looting creature. We send the slot with this one
	if (mode > -1 && receivingCreature != NULL) {
		g_Logs.simulator->info("[%v] Offering loot to looter", sim->InternalID);
		int slot = receivingCreature->charPtr->inventory.GetFreeSlot(
				INV_CONTAINER);
		LootTag tag = party->TagItem(ItemID, receivingCreature->CreatureID, CID,
				slot);
		offers++;

//		STRINGLIST qresponse;
//		qresponse.push_back("OK");
//		sprintf(Aux3, "%d", conIndex);
//		qresponse.push_back(Aux3);
//		WriteIdx= PrepExt_QueryResponseStringList(&SendBuf[WriteIdx], query.ID, qresponse);
		Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", tag.lootTag);
		g_Logs.simulator->info(
				"Sending offer of %v to original looter (%v) using tag %v",
				ItemID, receivingCreature->CreatureID, sim->Aux3);
		return PartyManager::OfferLoot(sim->SendBuf, ItemID, sim->Aux3, needOrGreed);
	}

	if (offers == 0) {
		return QueryErrorMsg::GENERIC;
	} else {
		return QueryErrorMsg::NONE;
	}
}



void ClearLoot(ActiveParty *party, ActiveLootContainer *loot) {
	party->RemoveTagsForLootCreatureId(loot->CreatureID, 0, 0);
	loot->RemoveAllRolls();
}

void ResetLoot(ActiveParty *party, ActiveLootContainer *loot,
		int itemId) {
	party->RemoveTagsForLootCreatureId(loot->CreatureID, itemId, 0);
	loot->RemoveCreatureRolls(itemId, 0);
}

void UndoLoot(ActiveParty *party, ActiveLootContainer *loot,
		int itemId, int creatureId) {
	party->RemoveTagsForLootCreatureId(loot->CreatureID, itemId, creatureId);
	loot->RemoveCreatureRolls(itemId, creatureId);

}

PartyMember * RollForPartyLoot(SimulatorThread *sim,  ActiveParty *party,
		std::set<int> creatureIds, const char *rollType, int itemId) {
	g_Logs.simulator->info("[%v] Rolling for %v players", sim->InternalID,
			creatureIds.size());
	int maxRoll = 0;
	ItemDef *cdef = g_ItemManager.GetPointerByID(itemId);
	PartyMember *maxRoller = NULL;
	if (creatureIds.size() == 1) {
		return party->GetMemberByID(*creatureIds.begin());
	}
	for (std::set<int>::iterator it = creatureIds.begin();
			it != creatureIds.end(); ++it) {
		int rolled = g_RandomManager.RandModRng(1, 100);
		PartyMember *m = party->GetMemberByID(*it);
		if (rolled > maxRoll) {
			maxRoller = m;
			maxRoll = rolled;
		}
		int wpos = PartyManager::WriteLootRoll(sim->SendBuf,
				cdef->mDisplayName.c_str(), (char) rolled,
				m->mCreaturePtr->css.display_name);
		party->BroadCast(sim->SendBuf, wpos);

	}
	return maxRoller;
}
