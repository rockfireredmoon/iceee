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

#include "SharedMessage.h"
#include "../auth/Auth.h"
#include "../Item.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//AcknowledgeHeartbeatMessage
//

int AcknowledgeHeartbeatMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	//Sent by modded client only.
	//We don't need to do anything here since any response to recv will reset the response.
	//LogMessageL(MSG_SHOW, "Received response.");
	return 0;
}

//
//InspectItemDefMessage
//

int InspectItemDefMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int itemID = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	//LogMessageL(MSG_SHOW, "inspectItemDef requested for %d", itemID);
	ItemDef *item = g_ItemManager.GetSafePointerByID(itemID);
	return PrepExt_ItemDef(sim->SendBuf, item, sim->ProtocolState);
}
