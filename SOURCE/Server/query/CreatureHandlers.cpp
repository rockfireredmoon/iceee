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

#include "CreatureHandlers.h"
#include "../Creature.h"
#include "../Interact.h"
#include "../Quest.h"
#include "../Instance.h"
#include "../InstanceScale.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Scheduler.h"
#include "../util/Log.h"

extern char GSendBuf[32767];

//
//CreatureIsUsableHandler
//

int CreatureIsUsableHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: creature.isusable
	 Determines whether a creature can be used by a specific player.
	 Commonly indicated in the client with a shimmer effect and/or paw icon
	 during mouse over.
	 Args : [0] = Creature Instance ID
	 */

	if (query->argCount < 1)
		return 0;

	int WritePos = 0;

	int CID = query->GetInteger(0);

	if (g_Logs.simulator->enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("Querying if creature %v is usable.", CID);
	}

	int CDef = -1;
	CreatureInstance *target = creatureInstance->actInst->GetNPCInstanceByCID(
			CID);
	bool failed = false;
	if (target != NULL) {
		CDef = target->CreatureDefID;
		if (target->serverFlags & ServerFlags::IsUnusable)
			failed = true;
		else if (creatureInstance->actInst->mZoneDefPtr->mGrove == true && !target->HasStatus(StatusEffects::MOUNTABLE))
				//Creatures cannot be used in groves, to prevent abuse of shops, except
				//for mounts
			failed = true;
	} else
		failed = true;



	if (failed == true) {
		if (g_Logs.simulator->enabled(el::Level::Trace)) {
			g_Logs.simulator->trace("Creature %v is not usable, either there is no such creature nearby, player is in grove, or server flags say is unusable.", CID);
		}
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "N");
	}

	//creatureInstance->actInst->ResolveCreatureDef(CID, &CDef);

	int lootable = creatureInstance->actInst->lootsys.GetCreature(CID);
	if (lootable > 0) {
		int self =
				creatureInstance->actInst->lootsys.creatureList[lootable].HasLootableID(
						creatureInstance->CreatureDefID);
		if (self >= 0) {
			if (g_Logs.simulator->enabled(el::Level::Trace)) {
				g_Logs.simulator->trace("Creature %v is loot.", CID);
			}
			WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID,
					"Q");
		}
	} else {
		auto status = pld->charPtr->questJournal.CreatureIsUsable(CDef);
		if (status == "N") {

			if (g_Logs.simulator->enabled(el::Level::Trace)) {
				g_Logs.simulator->trace("Creature %v is not quest usable, continue ...", CID);
			}

			if (creatureInstance->actInst->nutScriptPlayer != NULL) {
				status = sim->GetScriptUsable(target);
				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v script usable status is %v.", CID, status);
				}
			}
		}

		if (status == "N") {

			if (g_Logs.simulator->enabled(el::Level::Trace)) {
				g_Logs.simulator->trace("Creature %v is not script usable, continue ...", CID);
			}

			InteractObject *ptr = g_InteractObjectContainer.GetObjectByID(
					CDef, pld->CurrentZoneID);
			if (ptr != NULL) {

				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v has an interact def.", CID);
				}

				bool hasReq = false;
				if (ptr->questReq != 0) {
					if (pld->charPtr->questJournal.completedQuests.HasQuestID(
							ptr->questReq) >= 0)
						hasReq = true;
					else {
						if (ptr->questComp == false)
							if (pld->charPtr->questJournal.activeQuests.HasQuestID(
									ptr->questReq) >= 0)
								hasReq = true;
					}
				} else
					hasReq = true;

				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v has requirements = %v.", CID, hasReq);
				}

				status = (hasReq == true) ? "Y" : "N";
			}
		}

		if (status == "N") {

			if (g_Logs.simulator->enabled(el::Level::Trace)) {
				g_Logs.simulator->trace("Creature %v is not InteractDef usable, continue ...", CID);
			}

			CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(target->CreatureDefID);
			if(cdef != NULL && (cdef->DefHints & CDEF_HINT_ITEM_GIVER)) {
				for(vector<int>::iterator it = cdef->Items.begin(); it != cdef->Items.end(); ++it) {
					/* For now we only allow use if the player doesn't already have
					 * the item. There could be other uses for this though. I'll
					 * add logic as and when it's needed
					 */
					int id = (*it);
					if(creatureInstance->charPtr->inventory.GetItemPtrByID(id) == NULL) {
						if (g_Logs.simulator->enabled(el::Level::Trace)) {
							g_Logs.simulator->trace("Creature %v is item giver, continue ...", CID);
						}

						if(cdef->DefHints & CDEF_HINT_USABLE_SPARKLY)
							status = "Q";
						else
							status = "Y";
						break;
					}
				}
			}
			else if (cdef != NULL && (cdef->DefHints & CDEF_HINT_USABLE_SPARKLY)) {

				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v is CDEF_HINT_USABLE_SPARKLY.", CID);
				}

				status = "Q";
			}
			else if (cdef != NULL && (cdef->DefHints & CDEF_HINT_USABLE)) {

				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v is CDEF_HINT_USABLE.", CID);
				}

				status = "Y";
			}
			else if (target->HasStatus(StatusEffects::HENGE)) {

				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v is henge.", CID);
				}

				status = "Y";
			}
			else if (target->HasStatus(StatusEffects::MOUNTABLE) && !target->HasStatus(StatusEffects::MOUNTED)) {

				if (g_Logs.simulator->enabled(el::Level::Trace)) {
					g_Logs.simulator->trace("Creature %v is mount.", CID);
				}

				status = "Y";
			}
		}

		if (g_Logs.simulator->enabled(el::Level::Trace)) {
			g_Logs.simulator->trace("Creature %v final usable status is %v.", CID, status);
		}

		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, status.c_str());
		//LogMessageL(MSG_SHOW, "  creature.isusable: %d (%d) = %s", CID, CDef, status);
	}

	if (WritePos == 0) {
		if (g_Logs.simulator->enabled(el::Level::Trace)) {
			g_Logs.simulator->trace("Creature %v is not usable, nothing wanted to handle.", CID);
		}

		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "N");
	}
	return WritePos;
}
//
//CreatureDefEditHandler
//

int CreatureDefEditHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: creature.def.edit
	 Notifies the server of an updated creature definition, as modified by
	 CreatureTweak
	 Args : [0] = Creature Def ID
	 [n+0] = Property name to change.
	 [n+1] = New value for property.
	 [...]
	 */

	//::_Connection.sendQuery("creature.def.edit", this, [
	//				"REASON",
//					this.mRenameReasonPopup.getText(),
//					targetId,
//					"name",
//					name
//				]);
//				::_Connection.sendQuery("creature.def.edit", this, [
//					"REASON",
//					this.mRenameReasonPopup.getText(),
//					targetId,
//					"DISPLAY_NAME",
//					name
//				]);
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	// Em - If the first argument is 'REASON', then this is a character rename
	// from the GM screen. Weird they overloaded this
	int argOffset = 0;
	if (strcmp(query->GetString(0), "REASON") == 0) {
		argOffset = 2;
	}

	int CDefID = atoi(query->args[argOffset].c_str());

	CreatureInstance* cInst = NULL;
	CreatureDefinition* cDef = NULL;
	CharacterData* charData = NULL;
	cInst = creatureInstance->actInst->GetPlayerByCDefID(CDefID);
	if (cInst == NULL) {
		cDef = CreatureDef.GetPointerByCDef(CDefID);
		/*
		 int index = CreatureDef.GetIndex(CDefID);
		 if(index >= 0)
		 cDef = &CreatureDef.NPC[index];
		 */
	} else {
		charData = g_CharacterManager.GetPointerByID(CDefID);
		if (charData != NULL)
			cDef = &charData->cdef;
	}

	if (cDef == NULL) {
		sim->SendInfoMessage("Could not resolve creature template.",
				INFOMSG_ERROR);
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	}

	if (argOffset > 0) {
		// A character rename, so sage permissions needed
		if (sim->CheckPermissionSimple(0, Permission_Sage) == false) {
			sim->SendInfoMessage(
					"Permission denied: Only sages can rename characters.",
					INFOMSG_ERROR);
			return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
		}
	} else {
		if (pld->CreatureDefID == CDefID) {
			if (sim->CheckPermissionSimple(0, Permission_TweakSelf) == false) {
				int res = sim->protected_helper_tweak_self(CDefID, cDef->DefHints, argOffset);
				if(res == -1) {
					sim->SendInfoMessage("Permission denied: cannot edit self.", INFOMSG_ERROR);
					return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
				}
				else {
					return res;
				}
			}
		} else {
			if (charData != NULL) {
				if (sim->CheckPermissionSimple(0, Permission_TweakOther)
						== false) {
					int res = sim->protected_helper_tweak_self(CDefID, cDef->DefHints, argOffset);
					if(res == -1) {
						sim->SendInfoMessage("Permission denied: cannot edit other players.", INFOMSG_ERROR);
						return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
					}
					else
						return res;
				}
			} else {
				if (sim->CheckPermissionSimple(0, Permission_TweakNPC)
						== false) {
					int res = sim->protected_helper_tweak_self(CDefID, cDef->DefHints, argOffset);
					if(res == -1) {
						sim->SendInfoMessage("Permission denied: cannot edit creatures.", INFOMSG_ERROR);
						return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
					}
					else
						return res;
				}
			}
		}
	}

	CharacterStatSet *css = &cDef->css;

	//Now there should be an even number of data pairs of a key
	//and value.  The key specifies which item to change,
	//(appearance so far), and the value contains the information
	//to apply.
	for (unsigned int i = 1 + argOffset; i < query->argCount; i += 2) {
		string n = query->args[i];
		transform(n.begin(), n.end(), n.begin(), ::tolower);
		const char *name = n.c_str();
		const char *value = query->args[i + 1].c_str();

		if (g_Logs.simulator->enabled(el::Level::Trace)) {
			g_Logs.simulator->trace("[%v]   Name: %v", sim->InternalID, name);
			g_Logs.simulator->trace("[%v]   Value: %v", sim->InternalID, value);
		}

		if (strcmp(name, "name") == 0) {
			if (charData == NULL) {
				sim->SendInfoMessage("Error setting name, no character data.",
						INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
			}
			AccountData * acc = g_AccountManager.FetchIndividualAccount(
					charData->AccountID);
			if (acc == NULL) {
				sim->SendInfoMessage("Error setting name. Missing account",
						INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
			}
			CharacterCacheEntry *ce = acc->characterCache.GetCacheCharacter(
					cInst->CreatureDefID);
			if (ce == NULL) {
				sim->SendInfoMessage("Error setting name. Missing cache entry",
						INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
			}

			// Update used names
			g_UsedNameDatabase.Remove(ce->creatureDefID);
			g_UsedNameDatabase.Add(ce->creatureDefID, value);

			ce->display_name = value;
			acc->PendingMinorUpdates++;
		} else if (strcmp(name, "appearance") == 0
				|| strcmp(name, "display_name") == 0) {
			int wr = WriteStatToSetByName(name, value, css);
			if (wr == -1) {
				sim->SendInfoMessage("Error setting attribute.", INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
			}
		}
	}

	//Update the associated definition for a player.
	if (cInst != NULL)
		cInst->css.CopyFrom(css);

	creatureInstance->Submit(bind(&ActiveInstance::BroadcastUpdateCreatureDef, creatureInstance->actInst, cDef, creatureInstance->CurrentX, creatureInstance->CurrentZ));

	if (argOffset > 0 && cInst != NULL) {
		// If update from a sage, sent the updates to the target as well
		cInst->simulatorPtr->Submit(bind(&ActiveInstance::BroadcastUpdateCreatureDef, cInst->actInst, cDef, cInst->CurrentX, cInst->CurrentZ));
	}

	if (pld->CreatureDefID == CDefID) {
		// Tweak Self

		//Set this so the main thread won't try to auto update
		//the character data for this simulator.
		//The message broadcast will take care of it.
		sim->LastUpdate = g_ServerTime;
	} else {
		if (charData != NULL) {
			// Tweak other
		} else {
			// Tweak NPC
			CreatureDef.SaveCreatureTweak(cDef);
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//CreatureUseHandler
//

int CreatureUseHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: creature.use
	 Sent when a usable creature is clicked on (quest giver/ender, quest objective
	 gather/interact, etc)
	 Args : [0] = Creature Instance ID
	 */
	creatureInstance->RemoveNoncombatantStatus("creature_use");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	if (creatureInstance->HasStatus(StatusEffects::DEAD)) {
		sim->SendInfoMessage("You must be alive.", INFOMSG_ERROR);
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	}

	creatureInstance->CancelInvisibility();

	int CID = atoi(query->args[0].c_str());

	CreatureInstance *target = creatureInstance->actInst->GetNPCInstanceByCID(
			CID);

	if (target == NULL)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	if (creatureInstance->actInst->GetBoxRange(creatureInstance, target)
			> SimulatorThread::INTERACT_RANGE)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	//Creatures cannot be interacted with in groves unless it is a mount.
	if (creatureInstance->actInst->mZoneDefPtr->mGrove == true && !target->HasStatus(StatusEffects::MOUNTABLE))
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	else if(target->HasStatus(StatusEffects::MOUNTABLE)) {
		if(creatureInstance->Mount(target)) {
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		}
		else
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Failed to mount.");
	}

	//LogMessageL(MSG_DIAG, "  Request creature.use for %d", CID);
	//int CDef = ResolveCreatureDef(CID);
	int CDef = target->CreatureDefID;

	if (target->HasStatus(StatusEffects::HENGE)) {

		/* Ask the instance script if it's OK for this creature to Henge. If there is no script
		 * function, assume it's OK
		 */
		if(!creatureInstance->actInst->ScriptCallUse(creatureInstance->CreatureID, target->CreatureID, CDef, true))
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Cannot use object.");

		sim->CreatureUseHenge(CID, CDef);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}

	int QuestID = 0;
	int QuestAct = 0;
	QuestObjective *qo = pld->charPtr->questJournal.CreatureUse(CDef, QuestID,
			QuestAct);
	if (qo == NULL) {
		//Not a quest interact, check for interactibles.
		InteractObject *intObj = g_InteractObjectContainer.GetObjectByID(CDef,
				pld->CurrentZoneID);
		if (intObj != NULL) {
			if (intObj->opType == InteractObject::TYPE_WARP
					|| intObj->opType == InteractObject::TYPE_LOCATIONRETURN) {
				if (intObj->WarpID != pld->CurrentZoneID) {
					g_Logs.server->info("%v is a warp interact to %v", CDef,
							intObj->WarpID);
					ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
							intObj->WarpID);


					/* Ask the instance script if it's OK for this creature to warp. If there is no script
					 * function, assume it's OK
					 */
					if(!creatureInstance->actInst->ScriptCallUse(creatureInstance->CreatureID, target->CreatureID, CDef, true))
						return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Cannot use object.");

					if (zoneDef != NULL) {

						if(creatureInstance->css.level < zoneDef->mMinLevel) {
							Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "You must be of at least level %d to enter this zone.", zoneDef->mMinLevel);
							return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
						}
						else if(creatureInstance->css.level > zoneDef->mMaxLevel) {
							Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "You can be of no more than level %d to enter this zone.", zoneDef->mMaxLevel);
							return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
						}

						PlayerInstancePlacementData pd;
						sim->FillPlayerInstancePlacementData(pd, intObj->WarpID, 0);
						ActiveInstance *inst =
								g_ActiveInstanceManager.ResolveExistingInstance(
										pd, zoneDef);
						string outputMsg;
						if (inst != NULL) {
							g_Logs.server->info(
									"There is an active instance for zone %v for player %v",
									intObj->WarpID,
									creatureInstance->CreatureDefID);
							if (inst->scaleConfig.IsScaled() == true) {
								outputMsg =
										"You are entering a scaled instance. Difficulty: ";
								if (inst->scaleProfile != NULL)
									outputMsg.append(
											inst->scaleProfile->mDifficultyName);
							}
						} else {
							g_Logs.server->info(
									"There is no active instance for zone %v for player %v",
									intObj->WarpID,
									creatureInstance->CreatureDefID);

							if (zoneDef->IsMobScalable()
									== true&& pd.in_scaleProfile != NULL) {
								outputMsg =
										"You are about to create a new scaled instance. Difficulty: ";
								outputMsg.append(
										pd.in_scaleProfile->mDifficultyName);
							}
						}
						if (outputMsg.size() > 0) {
							sim->SendInfoMessage(outputMsg.c_str(),
									INFOMSG_INFO);
						}
					} else {
						g_Logs.server->error(
								"Zone %v does not exist. %v cannot be warped.",
								intObj->WarpID, CDef);
					}
				}
			}

			creatureInstance->LastUseDefID = CDef;

			/* Ask the instance script if it's OK for this creature to use this interact. If there is no script
			 * function, assume it's OK
			 */
			if(!creatureInstance->actInst->ScriptCallUse(creatureInstance->CreatureID, target->CreatureID, CDef, true))
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Cannot use object.");

			int size = creatureInstance->NormalInteractObject(sim->SendBuf,
					intObj);
			if (size > 0) {
				g_Logs.server->info(
						"Creature %v is a normal interact being used by %v",
						CDef, creatureInstance->CreatureDefID);
				sim->AttemptSend(sim->SendBuf, size);
			}

			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		}

		if (target->HasStatus(StatusEffects::USABLE_BY_SCRIPT)) {
			g_Logs.server->info("Creature %v is usable by scripts by %v", CDef,
					creatureInstance->CreatureDefID);
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		}

		bool ok = false;
		CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(target->CreatureDefID);
		std::string error = "Cannot use object.";

		/* Is the object an ITEM_GIVER? (Item ids given are in Extra Data) */
		if(cdef != NULL && (cdef->DefHints & CDEF_HINT_ITEM_GIVER) != 0) {

			for(std::vector<int>::iterator it = cdef->Items.begin() ; it != cdef->Items.end(); ++it) {
				/* For now we only allow use if the player doesn't already have
				 * the item. There could be other uses for this though. I'll
				 * add logic as and when it's needed
				 */
				int id = (*it);
				if(creatureInstance->charPtr->inventory.GetItemPtrByID(id) == NULL) {
					ItemDef *item = g_ItemManager.GetSafePointerByID(id);
					if(item->mID == 0) {
						Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Item with ID [%d] not found. Incorrect Creature Definition for an ITEM_GIVER.", id);
						error = sim->Aux1;
					}
					else {
						creatureInstance->SelectTarget(target);
						Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Taking %s", item->mDisplayName.c_str());
						sim->AttemptSend(GSendBuf, creatureInstance->QuestInteractObject(GSendBuf, sim->Aux1, 5000, true));
						ok = true;
					}

					break;
				}
			}
		}
		else
			if(cdef != NULL && ((cdef->DefHints & CDEF_HINT_USABLE) ||(cdef->DefHints & CDEF_HINT_USABLE_SPARKLY)))
				ok = true;

		/* Ask the instance script if it's OK for this creature to use this generic object. Only assume
		 * this is OK if the script function exists and returns true
		 */
		if(creatureInstance->actInst->ScriptCallUse(creatureInstance->CreatureID, target->CreatureID, CDef, false))
			ok = true;

		if(ok)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		else
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID, error.c_str());
	}

	int wpos = 0;
	if (qo->type == QuestObjective::OBJECTIVE_TYPE_ACTIVATE) {
		if (creatureInstance->CurrentTarget.targ != NULL) {
			creatureInstance->LastUseDefID = CDef;

			/* Ask the instance script if it's OK for this creature to use this quest related object. If
			 * the function doesn't exist, assume it is OK
			 */
			if(!creatureInstance->actInst->ScriptCallUse(creatureInstance->CreatureID, target->CreatureID, CDef, true))
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Cannot use object.");

			/* Instance Script */
			if(creatureInstance->actInst->nutScriptPlayer != NULL) {
				creatureInstance->actInst->QueueCallUsed(creatureInstance->CreatureID, target->CreatureID, CDef, qo->ActivateTime);
			}

			QuestScript::QuestNutPlayer *questNutScript =
					g_QuestNutManager.GetActiveScript(
							creatureInstance->CreatureID, QuestID);
			if (questNutScript == NULL) {
				// Old script system
				sim->questScript.def = &g_QuestScript;
				sim->questScript.simCall = sim;
				int wpos = creatureInstance->QuestInteractObject(GSendBuf,
						qo->ActivateText.c_str(), qo->ActivateTime,
						qo->gather);
				sprintf(sim->Aux1, "onActivate_%d_%d", QuestID, QuestAct);
				sprintf(sim->Aux1, "onActivate_%d_%d", QuestID, QuestAct);
				if (sim->questScript.JumpToLabel(sim->Aux1) == true) {
					sim->questScript.mActive = true;
					sim->questScript.nextFire = 0;
					sim->questScript.sourceID = creatureInstance->CreatureID;
					sim->questScript.targetID = CID;
					sim->questScript.targetCDef = CDef;
					sim->questScript.QuestID = QuestID;
					sim->questScript.QuestAct = QuestAct;
					sim->questScript.actInst = creatureInstance->actInst;
					sim->questScript.RunFlags = 0;
					sim->questScript.activateX = target->CurrentX;
					sim->questScript.activateY = target->CurrentY;
					sim->questScript.activateZ = target->CurrentZ;
					//questScript.RunScript();
					creatureInstance->actInst->questScriptList.push_back(
							sim->questScript);
				}
				//creatureInstance->actInst->ScriptCallUse(CDef);
				sim->AttemptSend(GSendBuf, wpos);
			} else {
				// New script system
				int wpos = creatureInstance->QuestInteractObject(GSendBuf,
						qo->ActivateText.c_str(), qo->ActivateTime,
						qo->gather);
				questNutScript->source = creatureInstance;
				//				questNutScript->target = target;
				//				questNutScript->CurrentQuestAct = QuestAct;
				questNutScript->activate.Set(target->CurrentX, target->CurrentY,
						target->CurrentZ);
				sprintf(sim->Aux1, "on_activate_%d", QuestAct);
				questNutScript->JumpToLabel(sim->Aux1);
				sim->AttemptSend(GSendBuf, wpos);

				// Queue up the on_activated as well, this MIGHT not be get called if the activation is interrupted
				sprintf(sim->Aux1, "on_activated_%d", QuestAct);
				questNutScript->activateEvent = new ScriptCore::NutScriptEvent(
						new ScriptCore::TimeCondition(qo->ActivateTime),
						new ScriptCore::RunFunctionCallback(questNutScript,
								string(sim->Aux1)));
				questNutScript->QueueAdd(questNutScript->activateEvent);

			}
		}
	} else {
		wpos += pld->charPtr->questJournal.CheckQuestTalk(&sim->SendBuf[wpos],
				CDef, CID, creatureInstance->CreatureID);
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}
//
//CreatureDeleteHandler
//

int CreatureDeleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: creature.delete
	 Sent when a selected creature is deleted while in build mode (/togglebuilding)
	 Args: [0] = Creature ID
	 [1] = Operation Type (ex: "PERMANENT")
	 */

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	int CreatureID = query->GetInteger(0);
	CreatureInstance *creature = creatureInstance->actInst->GetNPCInstanceByCID(
			CreatureID);
	if (creature != NULL) {
		if (sim->HasPropEditPermission(NULL, (float) creature->CurrentX,
				(float) creature->CurrentZ) == true) {

			g_Scheduler.Submit(bind(&ActiveInstance::CreatureDelete, creatureInstance->actInst, CreatureID));
		}
	}

	/* OLD
	 if(pld->accPtr->CheckBuildPermissionAdv(pld->CurrentZoneID, pld->zoneDef->mPageSize, (float)creatureInstance->CurrentX, (float)creatureInstance->CurrentZ) == false)
	 return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Creature location is off limits.");
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

