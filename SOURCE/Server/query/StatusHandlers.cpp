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

#include "StatusHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Combat.h"
#include "../util/Log.h"

//
//MoreStatsHandler
//

int MoreStatsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	MULTISTRING response;
	StatInfo::GeneratePrettyStatTable(response, &creatureInstance->css);
	STRINGLIST row;
	row.push_back("Spirit Resist Percentage");
	sprintf(sim->Aux3, "%d%%",
			Combat::GetSpiritResistReduction(100, creatureInstance->css.spirit));
	row.push_back(sim->Aux3);
	response.push_back(row);

	row.clear();
	row.push_back("Psyche Resist Percentage");
	sprintf(sim->Aux3, "%d%%",
			Combat::GetPsycheResistReduction(100, creatureInstance->css.psyche));
	row.push_back(sim->Aux3);
	response.push_back(row);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, response);
}
