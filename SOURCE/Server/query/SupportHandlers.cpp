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

#include "SupportHandlers.h"
#include <curl/curl.h>
#include "../GM.h"
#include "../Account.h"
#include "../Config.h"

//
// BugReportHandler
//
int BugReportHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	std::string summary = query->GetString(0);
	std::string desc = query->GetString(2);
	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
			"{ \"title\": \"%s\", \"body\": \"Reported By: %s\\nCategory: %s\\n\\n%s\", \"labels\": [ \"bug\", \"in game\" ] }",
			Util::EncodeJSONString(summary).c_str(),
			pld->charPtr->cdef.css.display_name, query->GetString(1),
			Util::EncodeJSONString(desc).c_str());
	g_Log.AddMessageFormat(
			"Posting bug report with summary %s and category of %s",
			query->GetString(0), query->GetString(2));

	CURL *curl;
	curl = curl_easy_init();
	if (curl) {
		struct curl_slist *headers = NULL;
		std::string auth = g_Config.GitHubToken;

		curl_easy_setopt(curl, CURLOPT_URL,
				"https://api.github.com/repos/rockfireredmoon/iceee/issues");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "PlanetForever");

		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth.c_str());

		headers = curl_slist_append(headers, "Expect:");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sim->Aux1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);

#ifdef OUTPUT_TO_CONSOLE
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

		// TODO might need config item to disable SSL verification
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		CURLcode res;
		res = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		curl_easy_cleanup (curl);
		int wpos;
		if (res == CURLE_OK) {
			wpos = PrepExt_QueryResponseString(sim->Aux2, query->ID, "OK");
		} else {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Failed to send bug report to Github. Response code %d",
					res);
			wpos = PrepExt_QueryResponseError(sim->Aux2, query->ID, sim->Aux1);
		}
		sim->AttemptSend(sim->Aux2, wpos);
	}

	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Failed to configure HTTP connection.");
}

//
// PetitionListHandler
//
int PetitionListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index

	vector<Petition> pets = g_PetitionManager.GetPetitions(
			pld->charPtr->cdef.CreatureDefID);
	vector<Petition>::iterator it;
	wpos += PutShort(&sim->SendBuf[wpos], pets.size());
	struct tm * timeinfo;
	g_CharacterManager.GetThread("SimulatorThread::PetitionList");
	for (it = pets.begin(); it != pets.end(); ++it) {
		wpos += PutByte(&sim->SendBuf[wpos], 8);
		wpos += PutStringUTF(&sim->SendBuf[wpos],
				it->status == PENDING ? "pending" : "mine");
		CharacterData *petitioner = g_CharacterManager.GetPointerByID(
				it->petitionerCDefID);
		if (petitioner == NULL)
			wpos += PutStringUTF(&sim->SendBuf[wpos], "<Deleted>");
		else
			wpos += PutStringUTF(&sim->SendBuf[wpos],
					petitioner->cdef.css.display_name);
		sprintf(sim->Aux1, "%d", it->petitionId);
		wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
		if (petitioner == NULL)
			wpos += PutStringUTF(&sim->SendBuf[wpos], "");
		else {
			AccountData * accData = g_AccountManager.FetchIndividualAccount(
					petitioner->AccountID);
			if (accData == NULL)
				wpos += PutStringUTF(&sim->SendBuf[wpos], "<Missing account>");
			else {
				int c = 0;
				memset(&sim->Aux1, 0, sizeof(sim->Aux1));
				for (unsigned int i = 0; i < accData->MAX_CHARACTER_SLOTS;
						i++) {
					if (accData->CharacterSet[i] != 0
							&& accData->CharacterSet[i]
									!= petitioner->cdef.CreatureDefID) {
						CharacterCacheEntry *cce =
								accData->characterCache.ForceGetCharacter(
										accData->CharacterSet[i]);
						if (cce != NULL) {
							if (c > 0)
								strcat(sim->Aux1, ",");
							strcat(sim->Aux1, cce->display_name.c_str());
							c++;
						}
					}
				}
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
			}
		}
		sprintf(sim->Aux1, "%d", it->category);
		wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
		wpos += PutStringUTF(&sim->SendBuf[wpos], it->description);
		wpos += PutStringUTF(&sim->SendBuf[wpos], "0");  // TODO score
		time_t ts;
		timeinfo = localtime(&ts);
		sprintf(sim->Aux1, "%s", asctime(timeinfo));
		wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
	}
	g_CharacterManager.ReleaseThread();

	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
// PetitionSendHandler
//
int PetitionSendHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int pet = g_PetitionManager.NewPetition(pld->CreatureDefID,
			atoi(query->args[0].c_str()), query->args[1].c_str());
	if (pet > -1) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"**%s has submitted petition %d, could the next available Earthsage please take the petition**",
				pld->charPtr->cdef.css.display_name, pet);
		char buffer[4096];
		int wpos = PrepExt_GenericChatMessage(buffer, 0, "Petition Manager",
				"gm/earthsages", sim->Aux1);

		SIMULATOR_IT it;
		for (it = Simulator.begin(); it != Simulator.end(); ++it) {
			if (it->isConnected == true && it->ProtocolState == 1
					&& it->pld.accPtr->HasPermission(Perm_Account,
							Permission_Sage))
				it->AttemptSend(buffer, wpos);
		};
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	} else {
		g_Log.AddMessageFormat("Failed to create petition.");
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "Failed"); //??
	}
}

//
// PetitionDoActionHandler
//
int PetitionDoActionHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	int id = atoi(query->args[0].c_str());
	if (query->args[1].compare("take") == 0) {
		if (g_PetitionManager.Take(id, pld->CreatureDefID)) {
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		} else {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Failed to take petition.");
		}
	} else if (query->args[1].compare("untake") == 0) {
		if (g_PetitionManager.Untake(id, pld->CreatureDefID)) {
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		} else {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Failed to untake petition.");
		}
	} else if (query->args[1].compare("close") == 0) {
		if (g_PetitionManager.Close(id, pld->CreatureDefID)) {
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		} else {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Failed to close petition.");
		}
	} else {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown petition op.");
	}
}

