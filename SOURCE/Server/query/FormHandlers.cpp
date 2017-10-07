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

#include "FormHandlers.h"
#include "../Account.h"
#include "../Character.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Config.h"
#include <algorithm>

using namespace std;

//
// FormSubmitHandler
//

int FormSubmitHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 3)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query");

	if (creatureInstance->actInst->nutScriptPlayer == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No script running in this instance");

	Sqrat::Table t(creatureInstance->actInst->nutScriptPlayer->vm);
	STRINGLIST args;
	Util::Split(query->GetString(2), "&", args);
	for (size_t i = 0; i < args.size(); i++) {
		STRINGLIST parts;
		Util::Split(args[i].c_str(), "=", parts);
		t.SetValue(_SC(parts[0].c_str()), _SC(parts[1].c_str()));
	}

	std::vector<ScriptCore::ScriptParam> parms;
	parms.push_back(ScriptCore::ScriptParam(query->GetInteger(0)));
	parms.push_back(ScriptCore::ScriptParam(creatureInstance->CreatureID));
	parms.push_back(ScriptCore::ScriptParam(query->GetStringObject(1)));
	parms.push_back(ScriptCore::ScriptParam(t));
	creatureInstance->actInst->nutScriptPlayer->RunFunction("on_form_submit", parms,
			false);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
