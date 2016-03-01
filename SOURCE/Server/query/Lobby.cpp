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

#include "Lobby.h"
#include "../Util.h"
#include "ClanHandlers.h"
#include "../Account.h"
#include "../Config.h"
#include "../URL.h"
#include "../util/Log.h"

//
//PersonaListHandler
//

int PersonaListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: persona.list
	 Args : [none]
	 Notes: First query sent to the server.
	 Response: Send back the list of characters available on this account.
	 */

	//Seems to be a rare condition when the account can indeed be NULL at this point.  Possibly
	//disconnecting after the query is sent, but before it's processed?
	if (pld->accPtr == NULL) {
		g_Logs.simulator->error("[%v] persona.list null account", sim->InternalID);
		return 0;
	}

	g_Log.AddMessageFormat("Retrieving persona list for account:%d",
			pld->accPtr->ID);
	//TODO: Fix a potential buffer overflow.

	int WritePos = 0;
	WritePos += PutByte(&sim->SendBuf[WritePos], 1);     //_handleQueryResultMsg
	WritePos += PutShort(&sim->SendBuf[WritePos], 0);           //Message size
	WritePos += PutInteger(&sim->SendBuf[WritePos], query->ID);  //Query ID

	//Character row count
	int charCount = pld->accPtr->GetCharacterCount();
	if (g_ProtocolVersion >= 38) {
		//Version 0.8.9 has a an extra row in the beginning
		//with one string, the number of characters following.
		WritePos += PutShort(&sim->SendBuf[WritePos], charCount + 1);

		WritePos += PutByte(&sim->SendBuf[WritePos], 1);
		sprintf(sim->Aux3, "%d", charCount + 1);
		WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux3);
	} else {
		WritePos += PutShort(&sim->SendBuf[WritePos], charCount);
	}

	g_CharacterManager.GetThread("SimulatorThread::handle_query_persona_list");

	int b;
	for (b = 0; b < AccountData::MAX_CHARACTER_SLOTS; b++) {
		if (pld->accPtr->CharacterSet[b] != 0) {
			int cdefid = pld->accPtr->CharacterSet[b];
			CharacterCacheEntry *cce =
					pld->accPtr->characterCache.ForceGetCharacter(cdefid);
			if (cce == NULL) {
				g_Logs.simulator->error("[%v] Could not request character: %v", sim->InternalID, cdefid);
				sim->ForceErrorMessage("Critical: could not load a character.",
						INFOMSG_ERROR);
				sim->Disconnect("SimulatorThread::handle_query_persona_list");
				return 0;
			}

			if (g_Config.AprilFools != 0) {
				WritePos += PutByte(&sim->SendBuf[WritePos], 6); //6 character data strings
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->display_name.c_str()); //Seems to be sent twice in 0.8.9.  Unknown purpose.
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->display_name.c_str());
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->appearance.c_str());

				const char *eqApp = cce->eq_appearance.c_str();
				switch (cce->profession) {
				case 1:
					eqApp =
							"{[1]=3163,[0]=141760,[6]=3019,[10]=3008,[11]=2831}";
					break;
				case 2:
					eqApp =
							"{[0]=141763,[1]=141764,[6]=2107,[10]=2442,[11]=2898}";
					break;
				case 3:
					eqApp = "{[6]=2810,[10]=1980,[11]=2108,[2]=141765}";
					break;
				case 4:
					eqApp = "{[2]=143609,[6]=3160,[10]=3161,[11]=3162}";
					break;
				}
				WritePos += PutStringUTF(&sim->SendBuf[WritePos], eqApp);

				//sprintf(Aux3, "%d", cce->level);
				WritePos += PutStringUTF(&sim->SendBuf[WritePos], "1");

				sprintf(sim->Aux3, "%d", cce->profession);
				WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux3);
				if (WritePos >= (int) sizeof(sim->SendBuf))
					g_Logs.server->error("Buffer overflow in persona.list");
			} else {
				//Normal stuff.
				WritePos += PutByte(&sim->SendBuf[WritePos], 6); //6 character data strings
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->display_name.c_str()); //Seems to be sent twice in 0.8.9.  Unknown purpose.
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->display_name.c_str());
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->appearance.c_str());
				WritePos += PutStringUTF(&sim->SendBuf[WritePos],
						cce->eq_appearance.c_str());

				sprintf(sim->Aux3, "%d", cce->level);
				WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux3);

				sprintf(sim->Aux3, "%d", cce->profession);
				WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux3);
				if (WritePos >= (int) sizeof(sim->SendBuf))
					g_Logs.server->error("Buffer overflow in persona.list");
			}
		}
	}
	PutShort(&sim->SendBuf[1], WritePos - 3);       //Set message size
	g_CharacterManager.ReleaseThread();

	return WritePos;
}

//
//PersonaCreateHandler
//

int PersonaCreateHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: persona.create
	 Args : [variable] list of attributes
	 Notes: Attributes are customized by the player in the Character Creation
	 steps.
	 Return: Unknown.  Possibly an index for use in the client?
	 */
	g_AccountManager.cs.Enter("SimulatorThread::handle_query_persona_create");
	CharacterData newChar;
	int r = g_AccountManager.CreateCharacter(query->args, pld->accPtr, newChar);
	g_AccountManager.cs.Leave();
	if (r < AccountManager::CHARACTER_SUCCESS)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_AccountManager.GetCharacterErrorMessage(r));
	g_CharacterManager.SaveCharacter(newChar.cdef.CreatureDefID);

	sprintf(sim->Aux1, "%d", r + 1);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, sim->Aux1);
}

//
//PersonaDeleteHandler
//

int PersonaDeleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: persona.delete
	 Args : [0] index to remove
	 */
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query");

	int index = query->GetInteger(0);
	if (index >= 0 && index <= pld->accPtr->MAX_CHARACTER_SLOTS) {
		int CDefID = pld->accPtr->CharacterSet[index];
		CharacterData *cd = g_CharacterManager.RequestCharacter(CDefID, true);
		if (cd->clan > 0) {
			Clans::Clan c = g_ClanManager.mClans[cd->clan];
			if (c.mId > 0) {
				Clans::ClanMember me = c.GetMember(CDefID);
				c.RemoveMember(me);
				if (c.mMembers.size() < 1) {
					g_Log.AddMessageFormat(
							"Disbanding clan %s because the last member left.",
							c.mName.c_str());
					g_ClanManager.RemoveClan(c);
				} else {
					if (me.mRank == Clans::Rank::LEADER) {
						Clans::ClanMember nextLeader = c.GetFirstMemberOfRank(
								Clans::Rank::OFFICER);
						if (nextLeader.mID == 0) {
							nextLeader = c.GetFirstMemberOfRank(
									Clans::Rank::MEMBER);
							if (nextLeader.mID == 0) {
								nextLeader = c.GetFirstMemberOfRank(
										Clans::Rank::INITIATE);
								if (nextLeader.mID == 0) {
									g_Log.AddMessageFormat(
											"[WARNING] There is nobody to pass leadership of clan of %s to! Removing the clan",
											c.mName.c_str());
									g_ClanManager.RemoveClan(c);
									BroadcastClanDisbandment(c);
								}

							}
						}
						if (nextLeader.mID != 0) {
							nextLeader.mRank = Clans::Rank::LEADER;
							c.UpdateMember(nextLeader);
							BroadcastClanRankChange(cd->cdef.css.display_name,
									c, nextLeader);
							g_ClanManager.SaveClan(c);
						}
					} else {
						g_ClanManager.SaveClan(c);
					}
				}
			}
		}
	}

	g_AccountManager.DeleteCharacter(index, pld->accPtr);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//LobbyPingHandler
//

int LobbyPingHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//AccountTrackingHandler
//

int AccountTrackingHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: account.tracking
	 Args : 0 = Seems to be a bit flag corresponding to the following
	 screens:
	 1 = Character Selection
	 2 = Character Creation Page 1 (Gender/Race/Body/Face)
	 4 = Character Creation Page 2 (Detail Colors/Size)
	 8 = Character Creation Page 3 (Class)
	 16 = Character Creation Page 4 (Name, Clothing)
	 Notes: Doesn't seem to be sent in 0.6.0 or 0.8.9, but is sent
	 in 0.8.6.  The proper response for this query is unknown.
	 The query is sent in any forward page changes, but only sends for
	 back pages from 4 to 16.
	 */
	int value = 0;
	if (sim->query.argCount >= 1)
		value = atoi(query->args[0].c_str());
	if (value >= 2)
		sim->characterCreation = true;
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}


//
// ModGetURLHandler
//

int ModGetURLHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, g_URLManager.GetURLs());
}
