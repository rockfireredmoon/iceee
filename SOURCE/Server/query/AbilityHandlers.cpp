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

#include "AbilityHandlers.h"
#include "../Ability2.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//AbilityBuyHandler
//

int AbilityBuyHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: ab.buy
	 Attempt to purchase an ability.
	 Args : [0] = Ability ID to purchase.
	 */

	if (query->argCount < 1 == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Query error.");

	int abilityID = query->GetInteger(0);
	if (abilityID >= Ability2::AbilityManager2::NON_PURCHASE_ID_THRESHOLD)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot purchase that ability.");

	if (pld->charPtr->abilityList.GetAbilityIndex(abilityID) >= 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You already have that ability.");

	const Ability2::AbilityEntry2 *abData = g_AbilityManager.GetAbilityPtrByID(
			abilityID);
	if (abData == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Ability does not exist.");

	int pointCost = abData->GetPurchaseCost();
	if (creatureInstance->css.current_ability_points < pointCost)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Not enough ability points.");

	if (creatureInstance->css.level < abData->GetPurchaseLevel())
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your level is not high enough.");

	if (abData->IsPurchaseableBy(creatureInstance->css.profession) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your class cannot use that ability.");

	std::vector<short> preReq;
	abData->GetPurchasePrereqList(preReq);
	for (size_t i = 0; i < preReq.size(); i++) {
		if (preReq[i] > 0)
			if (pld->charPtr->abilityList.GetAbilityIndex(preReq[i]) == -1)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"You are missing prerequisite abilities.");
	}

	pld->charPtr->abilityList.AddAbility(abilityID);
	creatureInstance->css.current_ability_points -= pointCost;

	if (abData->IsPassive() == true) {
		creatureInstance->CallAbilityEvent(abilityID, EventType::onRequest);
		creatureInstance->CallAbilityEvent(abilityID, EventType::onActivate);

		//creatureInst->actInst->ActivateAbility(creatureInst, abilityID, Action::onRequest, 0);
		//creatureInst->actInst->ActivateAbility(creatureInst, abilityID, Action::onActivate, 0);
	}

	sim->AddMessage((long) creatureInstance, 0, BCM_UpdateCreatureInstance);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, query->GetString(0));
}

//
//AbilityOwnageListHandler
//

int AbilityOwnageListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: ab.ownage.list
	 Retrieves the list of abilities which the player is able to use.
	 Args : [none]
	 */

	int WritePos = 0;
	WritePos += PutByte(&sim->SendBuf[WritePos], 1);   //_handleQueryResultMsg
	WritePos += PutShort(&sim->SendBuf[WritePos], 0);          //Message size
	WritePos += PutInteger(&sim->SendBuf[WritePos], query->ID);

	int size = pld->charPtr->abilityList.AbilityList.size();
	WritePos += PutShort(&sim->SendBuf[WritePos], size);      //Row count

	int a;
	for (a = 0; a < size; a++) {
		WritePos += PutByte(&sim->SendBuf[WritePos], 1);
		sprintf(sim->Aux3, "%d", pld->charPtr->abilityList.AbilityList[a]);
		WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux3);
	}

	PutShort(&sim->SendBuf[1], WritePos - 3);             //Set message size
	return WritePos;
}

//
//AbilityRespecHandler
//

int AbilityRespecHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: ab.respec
	 Remove all abilities from the purchase list.
	 Args : [none]
	 */

	creatureInstance->Respec();
	pld->charPtr->AbilityRespec(creatureInstance);
	sim->ActivatePassiveAbilities();
	sim->UpdateEqAppearance();

	sim->AddMessage((long) creatureInstance, 0, BCM_UpdateCreatureInstance);
	sim->AddMessage((long) &pld->charPtr->cdef, 0, BCM_CreatureDefRequest);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "true");
}

//
//AbilityRespecPriceHandler
//

int AbilityRespecPriceHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: ab.respec.price
	 Only sent in 0.8.9.  Seems to be a different way of fetching the
	 respec cost.
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "0");
}

//
//AbilityRemainingCooldownsHandler
//

int AbilityRemainingCooldownsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: ab.remainingcooldowns
		 Args : none
		 */
		//Each row sent back to the client must have:
		// [0] = category name, [1] = remaining time(ms), [2] = time elapsed(ms)
		MULTISTRING response;
		STRINGLIST row;
		for (size_t i = 0; i < creatureInstance->cooldownManager.cooldownList.size();
				i++) {
			ActiveCooldown *cd = &creatureInstance->cooldownManager.cooldownList[i];
			long remain = cd->castEndTimeMS - g_ServerTime;
			if (remain > 0) {
				const char *cooldownName =
						g_AbilityManager.ResolveCooldownCategoryName(cd->category);
				if (cooldownName == NULL)
					continue;
				row.push_back(cooldownName);  // [0] = category name
				sprintf(sim->Aux1, "%d", cd->GetRemainTimeMS());
				row.push_back(sim->Aux1);
				sprintf(sim->Aux1, "%d", cd->GetElapsedTimeMS());
				row.push_back(sim->Aux1);
				response.push_back(row);
				row.clear();
			}
		}
		return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, response);
}


//BuffRemoveHandler
//

int BuffRemoveHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: buff.remove
	 Sent when a player attempts to cancel a buff on their character.
	 Args : [0] = Ability ID
	 */

	if (query->argCount > 0) {
		int abilityID = query->GetInteger(0);
		creatureInstance->RemoveBuffsFromAbility(abilityID, true);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
