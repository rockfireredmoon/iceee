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

#include "GMHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Components.h"
#include "../Simulator.h"
#include "../Ability2.h"
#include "../util/Log.h"

//
//AddFundsHandler
//

int AddFundsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	std::string err;

	if (query->args[0].compare("CREDITS") == 0
			|| query->args[0].compare("COPPER") == 0) {
		int amount = atoi(query->args[1].c_str());
		g_CharacterManager.GetThread("SimulatorThread::AddFunds");
		CreatureInstance *creature =
				query->args[3].compare(
						creatureInstance->charPtr->cdef.css.display_name) == 0 ?
						creatureInstance :
						creatureInstance->actInst->GetPlayerByName(
								query->args[3].c_str());
		CharacterStatSet *css;
		CharacterData *cd;
		if (creature == NULL) {
			cd = g_CharacterManager.GetCharacterByName(query->args[3].c_str());
			if (cd != NULL)
				css = &(cd->cdef.css);
		} else {
			cd = creature->charPtr;
			css = &creature->css;
		}

		g_AccountManager.cs.Enter("AddFundsHandler::handleQuery");
		AccountData *ad = g_AccountManager.FetchIndividualAccount(
				cd->AccountID);

		if (cd == NULL) {
			err = "Cannot find character.";
		} else if (ad == NULL) {
			err = "Cannot find account.";
		} else {
			if (css != NULL) {
				if (query->args[0].compare("COPPER") == 0) {
					css->copper += amount;
					if (css->copper < 0)
						css->copper = 0;
					if (creatureInstance != NULL)
						creatureInstance->SendStatUpdate(STAT::COPPER);
					cd->pendingChanges++;
				} else {
					if (g_Config.AccountCredits)
						css->credits = ad->Credits;
					css->credits += amount;
					if (css->credits < 0)
						css->credits = 0;
					if (g_Config.AccountCredits) {
						ad->Credits = css->credits;
						ad->PendingMinorUpdates++;
					} else
						cd->pendingChanges++;
					if (creatureInstance != NULL)
						creatureInstance->SendStatUpdate(STAT::CREDITS);
				}

				g_Logs.event->info("[SAGE] %v gave %v %v copper because '%v'",
						pld->charPtr->cdef.css.display_name, css->display_name,
						amount, query->args[2].c_str());
			} else {
				err = "Cannot find player.";
			}
		}
		g_AccountManager.cs.Leave();
		g_CharacterManager.ReleaseThread();
	}

	if (err.size() == 0)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	else {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"Failed to add funds. %s", err.c_str());
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux2);
	}
}

//
//PVPZoneModeHandler
//

int PVPZoneModeHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	bool ok = sim->CheckPermissionSimple(Perm_Account, Permission_Sage);
	if (!ok) {
		if (pld->zoneDef->mGrove == true
				&& pld->zoneDef->mAccountID != pld->accPtr->ID)
			ok = true;
	}
	if (!ok)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	ActiveInstance *inst = creatureInstance->actInst;
	if (inst != NULL && query->argCount > 0) {
		if (inst->mZoneDefPtr->mArena == true)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Cannot change area PVP mode.");
		int mode = query->GetInteger(0);
		sim->SetZoneMode(mode);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	} else {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Zone mode for %d is %s",
				inst->mZoneDefPtr->mID,
				PVP::GameMode::GetNameByID(inst->mZoneDefPtr->mMode));
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}
}

//
//GMSpawnHandler
//

int GMSpawnHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	bool ok = sim->CheckPermissionSimple(Perm_Account, Permission_Sage);
	if (!ok) {
		if (pld->zoneDef->mGrove == true
				&& pld->zoneDef->mAccountID != pld->accPtr->ID)
			ok = true;
	}
	if (!ok)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (query->args.size() < 6)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	int creatureID = query->GetInteger(0);
	int qty = query->GetInteger(1);
	int spawnType = query->GetInteger(2);
	std::string data = query->GetString(3);
	int flags = query->GetInteger(4);
	int attackType = query->GetInteger(5);
	int abilityID = query->GetInteger(6);

	CreatureInstance *c = creatureInstance;
	if (c->CurrentTarget.targ != NULL) {
		c = c->CurrentTarget.targ;
	}

	CreatureInstance *s = NULL;

	for (int i = 0; i < qty; i++) {
		if (spawnType == 0) {
			// Random position around the players positions
			int radius = atoi(data.c_str());
			s = creatureInstance->actInst->SpawnGeneric(creatureID,
					c->CurrentX + randmodrng(1, radius) - (radius / 2),
					c->CurrentY,
					c->CurrentZ + randmodrng(1, radius) - (radius / 2),
					c->Rotation, flags);

		} else {
			// Random position around the players positions
			int propID = atoi(data.c_str());
			s = creatureInstance->actInst->SpawnAtProp(creatureID, propID,
					99999, flags);
		}

		if (s == NULL)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Failed to spawn.");

		int r;
		CreatureInstance *targ;
		std::vector<CreatureInstance*> l;

		switch (attackType) {
		case 0:
			s->SelectTarget(c);
			r = s->CallAbilityEvent(abilityID, EventType::onRequest);
			if (r != 0)
				g_Logs.simulator->error(
						"Failed to use ability %v on spawn master creature %v",
						abilityID, creatureID);
			break;
		case 1:
			l = creatureInstance->actInst->PlayerListPtr;
			if (l.size() > 0) {
				targ = l[randmodrng(0, l.size())];
				s->SelectTarget(targ);
				r = s->CallAbilityEvent(abilityID, EventType::onRequest);
				if (r != 0)
					g_Logs.simulator->error(
							"Failed to use ability %v for spawn master creature %v",
							abilityID, creatureID);
			}
			break;
		case 2:
			l = creatureInstance->actInst->NPCListPtr;
			if (l.size() > 0) {
				targ = l[randmodrng(0, l.size())];
				s->SelectTarget(targ);
				r = s->CallAbilityEvent(abilityID, EventType::onRequest);
				if (r != 0)
					g_Logs.simulator->error(
							"Failed to use ability %v for spawn master creature %v",
							abilityID, creatureID);
			}
			break;
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//StatsEffectSetHandler
//

int StatusEffectSetHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount < 3)
		return 0;

	if (!sim->CheckPermissionSimple(0, Permission_Sage)
			&& !sim->CheckPermissionSimple(0, Permission_Admin)) {
		sim->SendInfoMessage("Permission denied.", INFOMSG_ERROR);
	} else {
		int CDefID = atoi(query->args[0].c_str());
		int statusEffect = atoi(query->args[1].c_str());
		int state = atoi(query->args[2].c_str());

//		::_Connection.sendQuery("statuseffect.set", this, [
//						targetId,
//						this.StatusEffects.GM_SILENCED,
//						1,
//						time,
//						this.mGMSilenceReasonPopup.getText()
//					]);

		CreatureInstance *creature = creatureInstance->actInst->GetPlayerByID(
				CDefID);
		if (creature == NULL)
			sim->SendInfoMessage("No target!", INFOMSG_ERROR);
		else {
			if (state == 1) {
				int time =
						query->argCount > 2 ? atoi(query->args[3].c_str()) : 1;
				creature->_AddStatusList(statusEffect, time * 1000 * 60);
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
						"Status effect %d turned on for %s for %d minutes",
						statusEffect, creature->css.display_name, time);
				sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
				if (statusEffect == StatusEffects::GM_SILENCED) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
							"You have been silenced by %s for %d minutes because '%s'",
							pld->charPtr->cdef.css.display_name, time,
							query->args[4].c_str());
					creature->simulatorPtr->SendInfoMessage(sim->Aux2,
							INFOMSG_INFO);
					g_Logs.event->info(
							"[SAGE] %v silenced %v for %v minutes because",
							pld->charPtr->cdef.css.display_name,
							creature->charPtr->cdef.css.display_name, time,
							query->args[4].c_str());
				} else if (statusEffect == StatusEffects::GM_FROZEN) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
							"You have been frozen by %s for %d minutes.",
							pld->charPtr->cdef.css.display_name, time);
					creature->simulatorPtr->SendInfoMessage(sim->Aux2,
							INFOMSG_INFO);
					g_Logs.event->info("[SAGE] %v froze %v for %v minutes",
							pld->charPtr->cdef.css.display_name,
							creature->charPtr->cdef.css.display_name, time);
				} else {
					g_Logs.event->info(
							"[SAGE] %v removed status effect %v from %v",
							pld->charPtr->cdef.css.display_name, statusEffect,
							creature->charPtr->cdef.css.display_name);
				}
			} else {
				creature->_RemoveStatusList(statusEffect);
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
						"Status effect %d turned off for %s", statusEffect,
						creature->css.display_name);
				sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
				if (statusEffect == StatusEffects::GM_SILENCED) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
							"You have been unsilenced by %s",
							pld->charPtr->cdef.css.display_name);
					creature->simulatorPtr->SendInfoMessage(sim->Aux2,
							INFOMSG_INFO);
					g_Logs.event->info("[SAGE] %v unsilenced %v",
							pld->charPtr->cdef.css.display_name,
							creature->charPtr->cdef.css.display_name);
				} else if (statusEffect == StatusEffects::GM_FROZEN) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
							"You have been unfrozen by %s",
							pld->charPtr->cdef.css.display_name);
					creature->simulatorPtr->SendInfoMessage(sim->Aux2,
							INFOMSG_INFO);
					g_Logs.event->info("[SAGE] %v unfroze %v",
							pld->charPtr->cdef.css.display_name,
							creature->charPtr->cdef.css.display_name);
				} else {
					g_Logs.event->info(
							"[SAGE] %v set status effect %v for %v minutes on %v",
							pld->charPtr->cdef.css.display_name, statusEffect,
							time, creature->charPtr->cdef.css.display_name);
				}
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//UpdateContentHandler
//

int UpdateContentHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/* Called by the client ability import.  Placeholder function. */
	if (pld->accPtr->HasPermission(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}


//
//SummonHandler
//

int SummonHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if(!sim->CheckPermissionSimple(Perm_Account, Permission_Sage) && !sim->CheckPermissionSimple(Perm_Account, Permission_Admin))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Permission denied.");

	std::string op = query->GetString(0);
	std::string data = query->GetString(1);

	if(op.compare("player") == 0) {

		SimulatorThread *sim = GetSimulatorByCharacterName(data.c_str());
		if(sim == NULL) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Unknown player.");
		}
		if(creatureInstance != sim->creatureInst) {
			sim->pld.SetPortalRequestDest(creatureInstance->css.display_name, 1);
			sim->AttemptSend(sim->Aux1, PrepExt_CreatureEventPortalRequest(sim->Aux1, sim->creatureInst->CreatureID, creatureInstance->css.display_name, creatureInstance->css.display_name));
		}
	}
	else if(op.compare("zone") == 0) {
		int zId;
		if(data.length() == 0) {
			zId = creatureInstance->actInst->mZone;
		}
		else {
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByPartialWarpName(data.c_str());
			if(zd == NULL) {
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Unknown zone.");
			}
			zId = zd->mID;
		}

		SIMULATOR_IT it;
		for(it = Simulator.begin(); it != Simulator.end(); ++it)
		{
			if(it->isConnected == false)
				continue;
			if(it->LoadStage != SimulatorThread::LOADSTAGE_GAMEPLAY)
				continue;

			if(creatureInstance != it->creatureInst && zId == it->creatureInst->actInst->mZone) {
				it->pld.SetPortalRequestDest(creatureInstance->css.display_name, 1);
				it->AttemptSend(sim->Aux1, PrepExt_CreatureEventPortalRequest(sim->Aux1, it->creatureInst->CreatureID, creatureInstance->css.display_name, creatureInstance->css.display_name));
			}
		}
	}
	else if(op.compare("world") == 0) {
		SIMULATOR_IT it;
		for(it = Simulator.begin(); it != Simulator.end(); ++it)
		{
			if(it->isConnected == false)
				continue;

			if(it->LoadStage != SimulatorThread::LOADSTAGE_GAMEPLAY)
				continue;

			if(creatureInstance != it->creatureInst) {
				it->pld.SetPortalRequestDest(creatureInstance->css.display_name, 1);
				it->AttemptSend(sim->Aux1, PrepExt_CreatureEventPortalRequest(sim->Aux1, it->creatureInst->CreatureID, creatureInstance->css.display_name, creatureInstance->css.display_name));
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
