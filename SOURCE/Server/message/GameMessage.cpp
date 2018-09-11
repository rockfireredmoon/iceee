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

#include "GameMessage.h"
#include "../query/Query.h"
#include "../auth/Auth.h"
#include "../Ability2.h"
#include "../Creature.h"
#include "../Character.h"
#include "../Instance.h"
#include "../Chat.h"
#include "../ChatChannel.h"
#include "../Fun.h"
#include "../Debug.h"
#include "../DebugProfiler.h"
#include "../Config.h"
#include "../ScriptCore.h"
#include "../util/Log.h"
#include "../http/SiteClient.h"
#include <math.h>

//
//GameQueryMessage
//
int GameQueryMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

#ifdef DEBUG_TIME
	unsigned long startTime = g_PlatformTime.getMilliseconds();
#endif

	int PendingData = 0;
	query->Clear(); //Clear it here so that debug polls can fetch the most recent query.
	sim->ReadQueryFromMessage();

	//LogMessageL(MSG_SHOW, "[DEBUG] handle_game_query:%s (ID:%d)", query.name.c_str(), query.ID);

	// Debug
	/*
	 LogMessageL(MSG_SHOW, "Query: %d=%s", query.ID, query.name.c_str());
	 for(int i = 0; i < query.argCount; i++)
	 LogMessageL(MSG_SHOW, "  %d=%s", i, query.args[i].c_str());
	 */

	QueryHandler *qh = g_QueryManager.getQueryHandler(query->name);
	if (qh == NULL) {
		// See if the instance script will handle the command
		if (creatureInstance != NULL && creatureInstance->actInst != NULL
				&& creatureInstance->actInst->nutScriptPlayer != NULL) {
			std::vector<ScriptCore::ScriptParam> p;
			p.push_back(creatureInstance->CreatureID);
			p.insert(p.end(), query->args.begin(), query->args.end());
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "on_command_%s",
					query->name.c_str());
			if (creatureInstance->actInst->nutScriptPlayer->RunFunction(
					sim->Aux1, p, true)) {
				return PrepExt_QueryResponseString(sim->SendBuf,
						query->ID, "OK.");
			}
		}

		g_Logs.simulator->warn("[%v] Unhandled query in game: %v",
				sim->InternalID, query->name.c_str());
		for (unsigned int i = 0; i < query->argCount; i++)
			g_Logs.simulator->warn("[%v]   [%v]=[%v]", sim->InternalID, i,
					query->args[i].c_str());

		PendingData = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown query.");
	} else {
		PendingData = qh->handleQuery(sim, pld, query, creatureInstance);
	}


#ifdef DEBUG_TIME
	unsigned long passTime = g_PlatformTime.getMilliseconds() - startTime;
	if (passTime > 50) {
		g_Logs.simulator->debug(
				"[%v] TIME PASS handle_game_query() %v ms for query:%v (ID:%v)",
				sim->InternalID, passTime, query->name.c_str(), query->ID);
		for (unsigned int i = 0; i < query->argCount; i++)
			g_Logs.simulator->debug("[%v]   [%v]=%v", sim->InternalID, i,
					query->args[i].c_str());
	}

#ifdef DEBUG_PROFILER
	_DebugProfiler.AddQuery(query->name, passTime);
#endif

#endif
	return PendingData;
}

//
//InspectCreatureDefMessage
//
int InspectCreatureDefMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	int CDefID = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);

	g_CharacterManager.GetThread("SimulatorThread::handle_inspectCreatureDef");
	CharacterData *charData = g_CharacterManager.GetPointerByID(CDefID);
	g_CharacterManager.ReleaseThread();
	if (charData != NULL) {
		/* Is a player, is it for a player in the current active instance? If so, we need
		 * appearance modifiers so use the creature instance's calculated appearance.
		 *
		 * TODO Em - I don't really like this. It makes me wonder if modifiers should
		 * be on the creature def.
		 */
		CreatureInstance* cInst =
				creatureInstance == NULL || creatureInstance->actInst == NULL ?
						NULL :
						creatureInstance->actInst->GetPlayerByCDefID(
								charData->cdef.CreatureDefID);
		if (cInst != NULL) {
			CreatureDefinition cd(charData->cdef);
			cd.css.SetAppearance(cInst->PeekAppearance().c_str());
			cd.css.SetEqAppearance(cInst->PeekAppearanceEq().c_str());
			return PrepExt_CreatureDef(sim->SendBuf, &cd);

//			std::string currentAppearance = charData->cdef.css.appearance;
//			std::string currentEqAppearance = charData->cdef.css.eq_appearance;
//			charData->cdef.css.SetAppearance(cInst->PeekAppearance().c_str());
//			charData->cdef.css.SetEqAppearance(cInst->PeekAppearanceEq().c_str());
//
//			AttemptSend(SendBuf, PrepExt_CreatureDef(SendBuf, &charData->cdef));
//
//			charData->cdef.css.SetAppearance(currentAppearance.c_str());
//			charData->cdef.css.SetEqAppearance(currentEqAppearance.c_str());
		} else
			return PrepExt_CreatureDef(sim->SendBuf, &charData->cdef);
	} else {
		CreatureDefinition *target = CreatureDef.GetPointerByCDef(CDefID);
		if (target != NULL)
			return PrepExt_CreatureDef(sim->SendBuf, target);
		else
			g_Logs.simulator->warn(
					"[%d] inspectCreatureDef: could not find ID [%d]",
					sim->InternalID, CDefID);
	}
	return 0;
}

//
//UpdateVelocityMessage
//
int UpdateVelocityMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	if (g_ServerTime < pld->MovementBlockTime) {
		creatureInstance->Speed = 0;
		return 0;
	}
	//When entering a new zone from a portal, a velocity update always seems to be sent.
	//This is designed to block that message from interrupting the noncombat status.
	if (pld->IgnoreNextMovement == true)
		pld->IgnoreNextMovement = false;
	else
		creatureInstance->RemoveNoncombatantStatus("updateVelocity");

	int x = GetShort(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	int z = GetShort(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	int y = GetShort(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	creatureInstance->Heading = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	creatureInstance->Rotation = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	int speed = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);

//	LogMessageL(MSG_SHOW, "Heading:%d, Rot:%d, Spd:%d, X: %d, Y: %d, Z: %d", creatureInstance->Heading, creatureInstance->Rotation, speed, x, y, z);

	if (g_Config.FallDamage && !creatureInstance->actInst->mZoneDefPtr->mGrove) {
		int deltaY = creatureInstance->CurrentY - y;
		if (deltaY >= 30)
			pld->bFalling = true;
		if (pld->bFalling == true) {
			pld->DeltaY += deltaY;
			g_Logs.simulator->debug("[%v] Delta: %v, %v", sim->InternalID, deltaY,
					pld->DeltaY);
		}
		if (deltaY < 30) {
			if (pld->bFalling == true) {
				creatureInstance->CheckFallDamage(pld->DeltaY);
				g_Logs.simulator->debug("[%v] Damage: %v", sim->InternalID,
						pld->DeltaY);
				pld->bFalling = false;
			}
			pld->DeltaY = 0;
		}
	}

	if (g_Config.HasAdministrativeBehaviorFlag(ADMIN_BEHAVIOR_VERIFYSPEED)
			== true) {
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false) {
			int xlen = abs(x - (creatureInstance->CurrentX & 0xFFFF));
			int zlen = abs(z - (creatureInstance->CurrentZ & 0xFFFF));
			if (xlen < SimulatorThread::OverflowThreshold && zlen < SimulatorThread::OverflowThreshold) {
				int pspeed = 120 + creatureInstance->css.mod_movement;
				int distPerSecond = (int) (((float) pspeed / 100.0F) * 40.0F);
				//The number of units to move for this update.
				if (speed > pspeed || xlen > distPerSecond
						|| zlen > distPerSecond) {
					g_Logs.cheat->warn(
							"[SPEED] Spd: %d / %d, xlen: %d, zlen: %d (%d)",
							speed, pspeed, xlen, zlen, distPerSecond);
					return PrepExt_GeneralMoveUpdate(sim->SendBuf, creatureInstance);
				}
			}
		}
	}

	creatureInstance->Speed = speed;

	int oldX = creatureInstance->CurrentX;
	int oldZ = creatureInstance->CurrentZ;
	int newX = oldX;
	int newZ = oldZ;

	//Since these updates are unsigned short, the maximum value they can relay
	//is 65535.  Some maps are much larger (Europe), thus some kind of overflow
	//handling must be supplied to prevent characters from having their
	//position reach 0. Check for an overflow by comparing the new location
	//with the last location.  If the distance is too great, a rollover
	//must've happened (although coordinate warping might be a possibility
	//too?) If the resulting short value is small, it means it overflowed
	//into a higher value.  If not, it underflowed into a smaller value.
	if (abs(x - (oldX & 0xFFFF)) >= SimulatorThread::OverflowThreshold) {
		if (x < SimulatorThread::OverflowThreshold)
			newX += SimulatorThread::OverflowAdditive;
		else
			newX -= SimulatorThread::OverflowAdditive;
	}
	if (abs(z - (oldZ & 0xFFFF)) >= SimulatorThread::OverflowThreshold) {
		if (z < SimulatorThread::OverflowThreshold)
			newZ += SimulatorThread::OverflowAdditive;
		else
			newZ -= SimulatorThread::OverflowAdditive;
	}

	//Zero out the lower short, then OR it back with the short coords we received from the client.
	newX = (newX ^ (newX & 0xFFFF)) | x;
	newZ = (newZ ^ (newZ & 0xFFFF)) | z;
	creatureInstance->CurrentX = newX;
	creatureInstance->CurrentY = y;
	creatureInstance->CurrentZ = newZ;

	//LogMessageL(MSG_SHOW, "Loc: X:%d, Y:%d, Z:%d", x, y, z);
	//LogMessageL(MSG_SHOW, "Vel: Hd:%d, Rot:%d, Spd:%d", creatureInstance->Heading, creatureInstance->Rotation, creatureInstance->Speed);

	if (creatureInstance->Speed != 0) {
		//Check movement timers.
		int xlen = newX - oldX;
		int zlen = newZ - oldZ;
		float offset = sqrt((float) ((xlen * xlen) + (zlen * zlen)));
		creatureInstance->CheckMovement(offset);
		pld->TotalDistanceMoved += (int) offset;

		//Movement data verification against cheats.  Only run it for normal players.
		//If administrative /setstat command was used to increase speed above 255 (speed is 1 byte)
		//the expected velocity won't match.

		//The client uses a recurring event pulse to send the avatar's velocity updates to the server.
		//The pulse activates at 0.25 second intervals, regardless of whether movement occurred.
		//If the client's update flag is true (at least some movement happened), the update is sent
		//with the player's current position and rotation, then the update flag is cleared.  The
		//closed beta does not use any forced updates, although the client function allows that as
		//an optional parameter (forced updating would send the velocity update immediately,
		//independently from the update pulse, and would not trigger the update flag.

		//The pulse is not always accurate, and can vary with framerate.  Also, when first logging in
		//the pulse seems to be ignored, instead sending sporadic updates whenever required.  For example,
		//autorunning into a hill too steep to climb produces an indefinite flood of updates.  At some
		//currently unknown point, the pulse seems to take over and subsequent floods will stop.

		//There are tons of weird cases where verification fails on legit movement, so it may be necessary
		//to just disable it entirely.

		if ((g_Config.VerifyMovement == true)
				&& (sim->CheckPermissionSimple(Perm_Account, Permission_Admin)
						== false)) {
			int expectedSpeed = 100 + creatureInstance->css.base_movement
					+ creatureInstance->css.mod_movement;

			//The heading and rotation fields use only a single byte (0 to 255) to represent 360 degree
			//rotation.  So 90 degrees (360/4) -> (255/4) = 64.
			//Usually if we're moving backward in a single direction (pressing Backward only)
			//then the difference is almost always ~128.  However, if we're moving Left or Right while
			//moving backward, sometimes heading and rotation is the same so we can't judge by that.
			//Instead we have to look at the incoming speed.  Since moving backward is half speed,
			//division may be off slightly so we allow a small tolerance.
			int halfExpected = abs(speed - (expectedSpeed / 2));
			if ((speed != expectedSpeed) && (halfExpected > 3)) {
				g_Logs.cheat->info(
						"[SPEED] Unexpected speed by %s @ Zn:%d,X%d,Y%d, Spd:%d, Expected:%d",
						creatureInstance->css.display_name, pld->CurrentZoneID, newX,
						newZ, speed, expectedSpeed);
			} else {
				//Moving backwards, change to half speed for distance calculations.
				if ((speed != expectedSpeed) && (halfExpected <= 3))
					expectedSpeed /= 2;
			}
			//Movement updates are typically every 250 milliseconds, but even on localhost can vary a bit.
			//Using the client, update intervals of ~218ms were semi common.
			unsigned long timeSinceLastMovement = g_ServerTime
					- pld->MovementTime;
			/* Removed: natural latency triggers this so often that it spams too many logs messages.
			 if(timeSinceLastMovement < 200)
			 {
			 Debug::cheatLogger.Log("[SPEED] Rapid request by %s @ Zn:%d,X%d,Y%d, Spd:%d, Time:%lu", creatureInstance->css.display_name, pld->CurrentZoneID, newX, newZ, speed, timeSinceLastMovement);
			 LogMessageL(MSG_SHOW, "[SPEED] Rapid request by %s @ Zn:%d,X%d,Y%d, Spd:%d, Time:%lu", creatureInstance->css.display_name, pld->CurrentZoneID, newX, newZ, speed, timeSinceLastMovement);
			 }
			 */

			//We want to limit this to a quarter second so that we can calculate our expected
			//distance correctly, accepting lower update intervals but not large intervals which
			//would otherwise translate to potentially huge gaps with spoofed movement.
			//if(timeSinceLastMovement > 250)
			timeSinceLastMovement = 250;

			//Calculate how far the creature should have moved.  Default speed is 100, so
			//convert that into an expected distance while factoring the incoming time interval
			//and a 50% tolerance.
			float expectedOffset = ((expectedSpeed / 100.0F)
					* DEFAULT_CREATURE_SPEED)
					* (timeSinceLastMovement / 1000.0F) * 1.5F;

			// Disabled: was testing incoming movement for verification but it doesn't really help.
			/*
			 Sleep(1);
			 g_Log.AddMessageFormat("Offset:%g, Expected:%g", offset, expectedOffset);
			 */

			if (offset > expectedOffset) {
				g_Logs.cheat->info(
						"[SPEED] Moved too far by %s @ Zn:%d,X%d,Z%d, Spd:%d, Offset X:%d,Z:%d  Moved:%g/Expected:%g",
						creatureInstance->css.display_name, pld->CurrentZoneID, newX,
						newZ, speed, xlen, zlen, offset, expectedOffset);
			}
		}

		//Check our current position against any quest objective locations.
		//The movement counter will help us check every other step instead as a small optimization.
		if ((pld->PendingMovement & 1) && (pld->charPtr != NULL)) {
			int r = pld->charPtr->questJournal.CheckTravelLocations(
					creatureInstance->CreatureID, sim->Aux1, creatureInstance->CurrentX,
					creatureInstance->CurrentY, creatureInstance->CurrentZ,
					pld->CurrentZoneID);
			if (r > 0)
				sim->AttemptSend(sim->Aux1, r);
		}

		if (pld->PendingMovement >= 10) {
			pld->PendingMovement = 0;
		}

	}

	if (creatureInstance->actInst != NULL) {
		creatureInstance->actInst->PlayerMovement(creatureInstance);
	}

	//Check for zone boundaries, if a player is trying to go somewhere they should not be able to access.
	if (g_ZoneBarrierManager.CheckCollision(pld->CurrentZoneID,
			creatureInstance->CurrentX, creatureInstance->CurrentZ) == true)
		sim->AddMessage((long) creatureInstance, 0, BCM_UpdateFullPosition);

	int wpos = 0;

	//If GM invisible, send the update to ourself.  Otherwise broadcast it.
	if (sim->IsGMInvisible() == true) {
		wpos = PrepExt_GeneralMoveUpdate(sim->SendBuf, creatureInstance);
	} else {
		sim->AddMessage((long) creatureInstance, 0, BCM_UpdateVelocity);
		sim->AddMessage((long) creatureInstance, 0, BCM_UpdatePosInc);
	}

	pld->PendingMovement++;
	pld->MovementTime = g_ServerTime;

	sim->CheckSpawnTileUpdate(false);
	sim->CheckMapUpdate(false);

	return wpos;
}


//
//SelectTargetMessage
//
int SelectTargetMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int targetID = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	if (targetID != 0)
		creatureInstance->RemoveNoncombatantStatus("selectTarget");
	creatureInstance->RequestTarget(targetID);
	return 0;
}


//
//CommunicateMessage
//
int CommunicateMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	//For security and privacy reasons, we want to clear the buffers so that the buffer contents
	//are not visible in the debug report.
	OnExitFunctionClearBuffers fnCleanup(sim);

	//Since this function merges two older functions, create some aliases
	//to make things easier to keep track of.
	GetStringUTF(&sim->readPtr[sim->ReadPos], sim->Aux2, sizeof(sim->Aux2), sim->ReadPos); //Channel (small buffer)
	GetStringUTF(&sim->readPtr[sim->ReadPos], sim->Aux1, sizeof(sim->Aux1), sim->ReadPos); //Message (large ubffer)

	Util::SanitizeClientString(sim->Aux1);

	if (sim->CheckPermissionSimple(Perm_Account, Permission_TrollChat) == true) {
		if (strcmp(sim->Aux2, "rc/") == 0) {
			std::string messageCopy;
			if (Fun::oFunReplace.Replace(sim->Aux1, messageCopy) == true)
				Util::SafeCopy(sim->Aux1, messageCopy.c_str(), sizeof(sim->Aux1));
		}
	}

	//Aliases to make the stuff below easier.
	const char *channel = sim->Aux2;
	const char *message = sim->Aux1;

	if (sim->Aux1[0] == 0 || sim->Aux2[0] == 0) {
		g_Logs.simulator->warn("[%v] Invalid communication [%v][%v].",
				sim->InternalID, channel, message);
		return 0;
	}

	const ChannelCompare* channelInfo = GetChatInfoByChannel(sim->Aux2);

	//Tell is a special case.  It's not in the scope list because the target
	//character is embedded into the channel text.

	bool tell = false;
	bool perm = true;
//	bool privateChannel = false;
	PrivateChannel* privateChannelData = NULL;

	if (strncmp(channel, "t/", 2) == 0) {
		tell = true;
		//Extract the target character name from the channel string into a variable
		//that will be used to compare against the names of currently logged characters.
		//The string might contain quotations around a character name, so they
		//must not be copied.
		size_t len = strlen(channel);

		//If for some strange reason the incoming name is too long, clip it down to
		//prevent a possible buffer overflow.
		if (len > sizeof(sim->Aux3) - 1)
			len = sizeof(sim->Aux3) - 1;

		if (channel[2] == '"' && channel[len - 1] == '"') {
			if (len - 4 < 0) {
				g_Logs.simulator->error("[%v] Invalid offset (%v)", sim->InternalID,
						len);
				return 0;
			}

			//strncpy(sim->Aux3, &channel[3], len - 4);
			Util::SafeCopyN(sim->Aux3, &channel[3], sizeof(sim->Aux3), len - 4);
			sim->Aux3[len - 4] = 0;
		} else {
			if (len - 2 < 0) {
				g_Logs.simulator->error("[%v] Invalid offset (%v)", sim->InternalID,
						len);
				return 0;
			}

			//strncpy(sim->Aux3, &channel[2], len - 2);
			Util::SafeCopyN(sim->Aux3, &channel[2], sizeof(sim->Aux3), len - 2);
			sim->Aux3[len - 2] = 0;
		}
	} else if (strncmp(channel, "ch/", 3) == 0) {
		//Format: ch/<channel>
		privateChannelData = g_ChatChannelManager.GetChannelForMessage(
				sim->InternalID, channel + 3);
		if (privateChannelData == NULL) {
			sim->SendInfoMessage("You are not registered in that channel.",
					INFOMSG_ERROR);
			return 0;
		}
//		privateChannel = true;
		channelInfo = GetChatInfoByChannel("ch/");
	} else {
		if (channelInfo->chatScope == CHAT_SCOPE_NONE) {
			g_Logs.simulator->warn("[%v] Unknown chat channel: %v", sim->InternalID,
					channel);
			return 0;
		}
		if (strcmp(channel, "emote") == 0) {
			sim->AttemptSend(sim->SendBuf, pld->charPtr->questJournal.FilterEmote(
					creatureInstance->CreatureID, sim->SendBuf, message,
					creatureInstance->CurrentX, creatureInstance->CurrentZ,
					pld->CurrentZoneID));
		}
		if (strcmp(channel, "gm/earthsages") == 0)
			perm = sim->CheckPermissionSimple(Perm_Account, Permission_GMChat);
		else if (strcmp(channel, "*SysChat") == 0)
			perm = sim->CheckPermissionSimple(Perm_Account, Permission_SysChat);
		else if (strcmp(channel, "rc/") == 0)
			perm = sim->CheckPermissionSimple(Perm_Account, Permission_RegionChat);
		else if (strcmp(channel, "tc/") == 0)
			perm = sim->CheckPermissionSimple(Perm_Account, Permission_RegionChat);

		if (creatureInstance->HasStatus(StatusEffects::GM_SILENCED)) {
			sim->SendInfoMessage(
					"You are currently silenced by an Earthsage: you cannot use that channel.",
					INFOMSG_ERROR);
			return 0;
		}

		if (perm == false) {
			sim->SendInfoMessage("Permission denied: you cannot use that channel.",
					INFOMSG_ERROR);
			return 0;
		}
	}

	//Compose the packet for the outgoing chat data

	const char *charName = pld->charPtr->cdef.css.display_name;
	if (g_Config.AprilFoolsAccount == pld->accPtr->ID) {
		if (g_Config.AprilFoolsName.size() > 0)
			charName = g_Config.AprilFoolsName.c_str();
	}

	ChatMessage msg;
	if (pld->charPtr != NULL)
		msg.mSenderClanID = pld->charPtr->clan;
	msg.mChannelName = channel;
	msg.mMessage = message;
	msg.mSender = charName;
	msg.mSenderCreatureDefID = pld->CreatureDefID;
	msg.mSenderCreatureID = pld->CreatureID;
	msg.mTell = tell;
	msg.mRecipient = sim->Aux3;
	msg.mSendingInstance = pld->CurrentInstanceID;
	msg.mChannel = channelInfo;
	msg.mSimulatorID = sim->InternalID;

	bool found = g_ChatManager.SendChatMessage(msg, creatureInstance);

	if (tell == true && found == false) {

		/* Look up the account name for the character */
		int cdefID = g_UsedNameDatabase.GetIDByName(sim->Aux3);
		int msgCode = INFOMSG_ERROR;
		if (cdefID == -1) {
			sprintf(LogBuffer, "No such character \"%s\" .", sim->Aux3);
		} else {
			CharacterData *cd = g_CharacterManager.RequestCharacter(cdefID,
					true);
			if (cd == NULL || cd->AccountID < 1) {
				sprintf(LogBuffer,
						"Could not find creature definition for \"%s\" (%d), please report this error to an admin.",
						sim->Aux3, cdefID);
			} else {
				AccountData *data = g_AccountManager.FetchIndividualAccount(
						cd->AccountID);
				if (data == NULL) {
					sprintf(LogBuffer,
							"Could not find account for \"%s\" (%d, %d), please report this error to an admin.",
							sim->Aux3, cdefID, cd->AccountID);
				} else {
					char subject[256];
					Util::SafeFormat(subject, sizeof(subject),
							"In-game private message for %s from %s", sim->Aux3,
							creatureInstance->css.display_name);

					SiteClient siteClient(g_Config.ServiceAuthURL);
					if (siteClient.sendPrivateMessage(&pld->accPtr->SiteSession,
							data->Name, subject, message)) {
						sprintf(LogBuffer,
								"Sent offline private message to \"%s\".",
								sim->Aux3);
						msgCode = INFOMSG_INFO;
					} else {
						sprintf(LogBuffer, "Player \"%s\" is not logged in.",
								sim->Aux3);
					}
				}
			}
		}
		sim->SendInfoMessage(LogBuffer, msgCode);
	}

	return 0;
}

//
//InspectCreatureMessage
//
int InspectCreatureMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int CreatureID = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	CreatureInstance* cInst = creatureInstance->actInst->inspectCreature(
			CreatureID);
	if (cInst != NULL) {
		return PrepExt_CreatureFullInstance(sim->SendBuf, cInst);
	}
	return 0;
}

//
//AbilityActiveMessage
//
int AbilityActiveMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	creatureInstance->RemoveNoncombatantStatus("abilityActivate");

	// Flags appears to be intended for party casting, but isn't used outside the /do command.
	// We don't use it in the server, but we still need to read it from the message.
	short aID = GetShort(&sim->readPtr[sim->ReadPos], sim->ReadPos);            //Ability ID

	// Unused?
	GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	//unsigned char flags = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);   //flags

	unsigned char ground = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);  //ground
	float x, y, z;

	//LogMessageL(MSG_SHOW, "abilityActivate: %d, flags: %d", aID, flags);

	if (ground != 0) {
		x = GetFloat(&sim->RecBuf[sim->ReadPos], sim->ReadPos);
		y = GetFloat(&sim->RecBuf[sim->ReadPos], sim->ReadPos);
		z = GetFloat(&sim->RecBuf[sim->ReadPos], sim->ReadPos);
		creatureInstance->ab[0].SetPosition(x, y, z);
		//LogMessageL(MSG_DIAGV, "ground: %d, x: %g, y: %g, z: %g", ground, x, y, z);
	}

	if (creatureInstance->serverFlags & ServerFlags::IsTransformed) {
		const Ability2::AbilityEntry2 *abData =
				g_AbilityManager.GetAbilityPtrByID(aID);
		if (abData != NULL) {
			/* TODO: old junk, update or remove this
			 if(abData->isMagic == true)
			 {
			 sim->SendInfoMessage("You may not use elemental abilities while transformed.", INFOMSG_INFO);
			 return;
			 }
			 if(abData->isRanged == true)
			 {
			 sim->SendInfoMessage("You may not use ranged abilities while transformed.", INFOMSG_INFO);
			 return;
			 }
			 */
		}
	}

	if (aID == g_JumpConstant) {
		if (sim->IsGMInvisible() == false)
			sim->AddMessage(pld->CreatureID, 0, BCM_ActorJump);
	} else {
		bool allow = false;
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == true)
			allow = true;
		else if (pld->charPtr->abilityList.GetAbilityIndex(aID) >= 0)
			allow = true;
		else {
			if (g_AbilityManager.IsGlobalIntrinsicAbility(aID) == true)
				allow = true;
		}

		if (allow == true) {
			creatureInstance->RequestAbilityActivation(aID);
			if (sim->TargetRarityAboveNormal() == false) {
				if (pld->NotifyCast(creatureInstance->CurrentX,
						creatureInstance->CurrentZ, aID) == true) {
					CreatureInstance *target = creatureInstance->CurrentTarget.targ;
					const char *targetName = "no target";
					int targetID = 0;
					if (target != NULL) {
						targetID = target->CreatureID;
						targetName = target->css.display_name;
					}
					g_Logs.cheat->info(
							"[BOT] Attacks in area by %s @ %d:%d,%d(%d) (%d:%s) [%s:%d]",
							creatureInstance->css.display_name, pld->CurrentZoneID,
							creatureInstance->CurrentX, creatureInstance->CurrentZ,
							creatureInstance->Rotation, aID,
							g_AbilityManager.GetAbilityNameByID(aID),
							targetName, targetID);
				}
			}
		}
	}

	return 0;
}

//
//SwimStateChangeMessage
//
int SwimStateChangeMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	bool swim = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos) == 1;
	creatureInstance->swimming = swim;
	if (swim && creatureInstance->IsTransformed()) {
		creatureInstance->Untransform();
	}
	return 0;
}

//
//DisconnectMessage
//
int DisconnectMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	//This command is used by 0.8.9.
	g_Logs.simulator->info("[%v] The client sent a disconnect message.",
			sim->InternalID);
	sim->Disconnect("SimulatorThread: handle_disconnect");
	return 0;
}

//
//MouseClickMessage
//
int MouseClickMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int mouseX = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	int mouseY = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	int mouseZ = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);

	std::vector<ScriptCore::ScriptParam> p;
	p.push_back(ScriptCore::ScriptParam(mouseX));
	p.push_back(ScriptCore::ScriptParam(mouseY));
	p.push_back(ScriptCore::ScriptParam(mouseZ));

	if(creatureInstance != NULL && creatureInstance->actInst != NULL && creatureInstance->actInst->nutScriptPlayer != NULL && creatureInstance->actInst->nutScriptPlayer->mActive) {
		creatureInstance->actInst->nutScriptPlayer->mCaller = creatureInstance->CreatureID;
		creatureInstance->actInst->nutScriptPlayer->JumpToLabel("on_click", p);
		creatureInstance->actInst->nutScriptPlayer->mCaller = 0;
	}

	return 0;
}

