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

#include "PreferenceHandlers.h"
#include "../Preferences.h"
#include "../Account.h"
#include "../util/Log.h"

int RespondPrefGet(PreferenceContainer *prefSet, char *SendBuf,
		SimulatorQuery *query) {
	//Helper function for the "pref.getA" and "pref.get" queries.
	//Since the both accounts and characters use the same class to store
	//preferences, the query handlers will call this function with the
	//appropriate pointer.

	int WritePos = 0;
	WritePos += PutByte(&SendBuf[WritePos], 1);          //_handleQueryResultMsg
	WritePos += PutShort(&SendBuf[WritePos], 0);           //Message size
	WritePos += PutInteger(&SendBuf[WritePos], query->ID); //Query response index

	//Each preference request will have a matching response field.
	WritePos += PutShort(&SendBuf[WritePos], query->argCount);

	for (unsigned int i = 0; i < query->argCount; i++) {
		const char * pref = prefSet->GetPrefValue(query->args[i].c_str());

		//One string for each preference result.
		WritePos += PutByte(&SendBuf[WritePos], 1);
		if (pref != NULL) {
			WritePos += PutStringUTF(&SendBuf[WritePos], pref);
		} else {
			WritePos += PutStringUTF(&SendBuf[WritePos], "");
		}
	}

	PutShort(&SendBuf[1], WritePos - 3);

	return WritePos;
}

//
//PrefGetAHandler
//

int PrefGetAHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: pref.getA
	 Args : [variable] list of preferences (by named string) to retrieve.
	 Notes: Retrieves the account preferences.  Sent after the client login is
	 successful.
	 Response: Search for each preference in the account data and send back the
	 string data.
	 */
	return RespondPrefGet(&pld->accPtr->preferenceList, sim->SendBuf, query);
}

int PrefGetHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: pref.get
	 Args : [variable] list of preferences (by named string) to retrieve.
	 Notes: Retrieves the character preferences.
	 Response: Search for each preference in the character data and send back
	 the string data.
	 */
	return RespondPrefGet(&pld->charPtr->preferenceList, sim->SendBuf, query);
}

int PrefSetHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: pref.set
	 Args : [variable, multiple of two] list of string pairs of the name and
	 value to set (in the character permission set).
	 Response: Return standard "OK".
	 */

	//The character system needs extra debug steps.
	for (unsigned int i = 0; i < query->argCount; i += 2) {
		const char *name = query->args[i].c_str();
		const char *value = query->args[i + 1].c_str();
		//if(LoadStage < LOADSTAGE_LOADED)
		//	LogMessageL(MSG_WARN, "[WARNING] Pref set while loading [%s]=[%s"], name, value);

		bool allow = true;
		if (strstr(name, "quickbar") != NULL) {
			if (sim->LoadStage < 2) {
				allow = false;
				g_Logs.simulator->warn("[%v] Tried to set quickbar preference before gameplay [%v]=[%v]", sim->InternalID,
						name, value);
			} else if (strlen(value) < 3) {
				allow = false;
				g_Logs.simulator->warn("[%v] Tried to set quickbar preference to NULL [%v]=[%v]",
						sim->InternalID,
						name, value);
			}
		}

		if (allow == true)
			pld->charPtr->preferenceList.SetPref(name, value);
	}

	int WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	WritePos += RespondPrefGet(&pld->charPtr->preferenceList, &sim->SendBuf[WritePos], query);
	return WritePos;
}

int PrefSetAHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: pref.setA
	 Args : [variable, multiple of two] list of string pairs of the name and
	 value to set (in the account permission set).
	 Response: Return standard "OK".
	 */

	for (unsigned int i = 0; i < query->argCount; i += 2)
		pld->accPtr->preferenceList.SetPref(query->args[i].c_str(),
				query->args[i + 1].c_str());


	//Account data was changed, flag the change so the system can autosave the
	//file when needed.
	pld->accPtr->PendingMinorUpdates++;

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

