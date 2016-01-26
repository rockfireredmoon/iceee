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

#include "ScriptHandlers.h"
#include "../Account.h"
#include "../Character.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Config.h"
#include "../AIScript.h"
#include "../AIScript2.h"
#include "../ScriptCore.h"
#include "../InstanceScript.h"
#include "../QuestScript.h"
#include <algorithm>
#include <fstream>

using namespace std;

//
// AbstractScriptHandler
//

int AbstractScriptHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: script.op
	 Get a script (instance script etc)
	 Args: [type]

	 0 = Instance
	 1 = Quest
	 2 = AI

	 0 = Load
	 1 = Kill
	 2 = Run
	 3 = Save
	 */
	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Incorrect arguments, require type and parameter.");

	int type = query->GetInteger(0);
	const char *parameter = query->GetString(1);

	g_Log.AddMessageFormat("Script type %d param: %s", type, parameter);

	bool admin = sim->CheckPermissionSimple(Perm_Account, Permission_Admin);
	bool ok = admin;
	bool ownGrove = pld->zoneDef->mGrove == true
			&& pld->zoneDef->mAccountID == pld->accPtr->ID;
	if (!ok) {
		// Players can edit their own grove scripts (some commands will be restricted)
		if (type == 0 && ownGrove)
			ok = true;
	}
	if (!ok)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	string path;
	int instanceId = 0;
	int questId = 0;
	string scriptName;
	ScriptCore::NutPlayer *player = NULL;
	ScriptCore::ScriptPlayer *oldPlayer = NULL;
	ActiveInstance *instance = NULL;
	CreatureInstance *targetCreature = NULL;
	bool ownPlayer = true;

	switch (type) {
	case 0: {
		// Instance script
		if (strcmp(parameter, "") == 0) {
			instance = creatureInstance->actInst;
			instanceId = instance->mZone;
			player = instance->nutScriptPlayer;
			oldPlayer = instance->scriptPlayer;
		} else if (admin) {
			instanceId = atoi(parameter);
			if (instanceId == 0) {
				sim->SendInfoMessage("Invalid zone ID", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Invalid zone ID.");
			}
		} else {
			sim->SendInfoMessage("Permission denied", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");
		}

		// If editing the current zone, always use that as active script
		if (instanceId == creatureInstance->actInst->mZone) {
			instance = creatureInstance->actInst;
			player = instance->nutScriptPlayer;
			oldPlayer = instance->scriptPlayer;
		} else {
			ownPlayer = false;
		}

		ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(instanceId);
		path = InstanceScript::InstanceNutDef::GetInstanceScriptPath(instanceId,
				true, zoneDef != NULL && zoneDef->mGrove);
		break;
	}
	case 1:
		// Quest script
		if (!admin) {
			sim->SendInfoMessage("Permission denied", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");
		}

		if (strcmp(parameter, "") == 0) {
			sim->SendInfoMessage("Unsupported, please provide quest ID",
					INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Unsupported.");
		}

		ownPlayer = false;
		questId = atoi(parameter);
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "QuestScripts/%d.nut",
				questId);
		path = sim->Aux1;
		break;
	case 2:
		// AI script
		if (strcmp(parameter, "") == 0) {
			targetCreature = creatureInstance->CurrentTarget.targ;
			if (targetCreature == NULL)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Must select a creature to edit or provide a CDefID.");

			scriptName = targetCreature->css.ai_package;
			if (scriptName.length() == 0)
				scriptName = targetCreature->charPtr->cdef.css.ai_package;

			player = targetCreature->aiNut;
			oldPlayer = targetCreature->aiScript;
		} else if (admin) {
			scriptName = parameter;
		} else {
			sim->SendInfoMessage("Permission denied", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");
		}

		// If editing a creatures AI, always use that as active script
		if (strcmp(parameter, "")
				!= 0&& creatureInstance->CurrentTarget.targ != NULL) {
			scriptName = creatureInstance->CurrentTarget.targ->css.ai_package;
			if (scriptName.length() == 0) {
				scriptName =
						creatureInstance->CurrentTarget.targ->charPtr->cdef.css.ai_package;
			}
			if (scriptName.compare(parameter) == 0) {
				targetCreature = creatureInstance->CurrentTarget.targ;
				player = targetCreature->aiNut;
				oldPlayer = targetCreature->aiScript;
			} else {
				ownPlayer = false;
			}
		}

		ScriptCore::NutScriptCallStringParser p(scriptName);

		AINutDef *def = aiNutManager.GetScriptByName(p.mScriptName.c_str());
		if (def == NULL) {
			AIScriptDef * oldDef = aiScriptManager.GetScriptByName(
					p.mScriptName.c_str());
			if (oldDef != NULL) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"AIScript/%s.txt", p.mScriptName.c_str());
				path = string(sim->Aux1);
			}

		} else
			path = def->mSourceFile;

		break;
	}

	return handleScriptQuery(ownPlayer, instanceId, questId, type, scriptName,
			player, oldPlayer, instance, targetCreature, path, sim, pld, query,
			creatureInstance);
}

//
// ScriptLoadHandler
//

int ScriptLoadHandler::handleScriptQuery(bool ownPlayer, int instanceID,
		int questID, int type, std::string scriptName,
		ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
		ActiveInstance *instance, CreatureInstance *targetCreature,
		std::string path, SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	if (path.length() == 0) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Unable to open script.");
		sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
		sim->LogMessageL(MSG_WARN,
				"[WARNING] Load script query unable to open script for zone: %d",
				creatureInstance->actInst->mZone);
		return 0;
	}
	FileReader lfr;
	std::vector<std::string> lines;
	if (Platform::FileExists(path.c_str())) {
		if (lfr.OpenText(path.c_str()) != Err_OK) {

			sim->LogMessageL(MSG_WARN,
					"[WARNING] Load script query unable to open file: %s",
					path.c_str());
			return 0;
		}

		while (lfr.FileOpen() == true) {
			lfr.ReadLine();
			lines.push_back(lfr.DataBuffer);
		}
	}
	else {
		lines.push_back("#!/bin/sq");
	}

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], lines.size() + 1);
	for (unsigned int i = 0; i < lines.size(); i++) {
		wpos += PutByte(&sim->SendBuf[wpos], 1);
		wpos += PutStringUTF(&sim->SendBuf[wpos], lines[i].c_str());
	}

	// The last record contains info about the instance itself for the editor UI
	wpos += PutByte(&sim->SendBuf[wpos], 2);

	if (player != NULL) {
		g_Log.AddMessageFormat("Using active new player %s",
				player->mActive ? "active" : "inactive");
		wpos += PutStringUTF(&sim->SendBuf[wpos],
				player->mActive ? "true" : "false"); // active
	} else if (oldPlayer != NULL) {
		g_Log.AddMessageFormat("Using active old player %s",
				oldPlayer->mActive ? "active" : "inactive");
		wpos += PutStringUTF(&sim->SendBuf[wpos],
				oldPlayer->mActive ? "true" : "false"); // active
	} else {
		g_Log.AddMessageFormat("No player %s!",
				ownPlayer ? "false" : "unknown");
		wpos += PutStringUTF(&sim->SendBuf[wpos],
				ownPlayer ? "false" : "unknown"); // active
	}

	switch (type) {
	case 0:
		sprintf(sim->Aux1, "%d", instanceID);
		break;
	case 1:
		sprintf(sim->Aux1, "%d", questID);
		break;
	case 2:
		sprintf(sim->Aux1, "%s", scriptName.c_str());
		break;
	}
	wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;

}

//
// ScriptKillHandler
//

int ScriptKillHandler::handleScriptQuery(bool ownPlayer, int instanceID,
		int questID, int type, std::string scriptName,
		ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
		ActiveInstance *instance, CreatureInstance *targetCreature,
		std::string path, SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	// Kill
	switch (type) {
	case 0:
		if (instance == NULL) {
			sim->SendInfoMessage("Can only kill scripts in current instance.",
					INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Can only kill scripts in current instance.");
		}
		if (!instance->KillScript()) {
			sim->SendInfoMessage("Script not running.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Script not running.");
		}
		sim->SendInfoMessage("Script killed.", INFOMSG_INFO);
		break;
	case 1:
		sim->SendInfoMessage("Not supported.", INFOMSG_ERROR);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Not supported.");
	default:
		// AI script
		if (targetCreature == NULL) {
			sim->SendInfoMessage("Must select a creature.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Must select a creature.");
		}
		if (!targetCreature->KillAI()) {
			sim->SendInfoMessage("Script not running.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Script not running.");
		}
		sim->SendInfoMessage("Script killed.", INFOMSG_INFO);
		break;
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ScriptRunHandler
//

int ScriptRunHandler::handleScriptQuery(bool ownPlayer, int instanceID,
		int questID, int type, std::string scriptName,
		ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
		ActiveInstance *instance, CreatureInstance *targetCreature,
		std::string path, SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	// Run
	sim->LogMessageL(MSG_SHOW, "Handling script run");

	std::string errors;

	switch (type) {
	case 0:
		if (instance == NULL) {
			sim->SendInfoMessage("Can only run scripts in current instance.",
					INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Can only run scripts in current instance.");
		}
		if (!instance->RunScript(errors)) {
			sim->SendInfoMessage("Script already running.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Script already running.");
		}
		break;
	case 1:
		sim->SendInfoMessage("Not supported.", INFOMSG_ERROR);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Not yet supported.");
	default:
		// AI script
		if (targetCreature == NULL) {
			sim->SendInfoMessage("Must select a creature.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Must select a creature.");
		}
		if (!targetCreature->StartAI(errors)) {
			sim->SendInfoMessage("Script already running.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Script already running.");
		}
		break;
	}

	if (errors.length() > 0) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Failed to run script %s", errors.c_str());
		sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Script already running.");
	} else {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Script now running.");
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ScriptSaveHandler
//

int ScriptSaveHandler::handleScriptQuery(bool ownPlayer, int instanceID,
		int questID, int type, std::string scriptName,
		ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
		ActiveInstance *instance, CreatureInstance *targetCreature,
		std::string path, SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount < 3)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Incorrect arguments, require script content.");

	std::string scriptText = query->GetString(2);

	std::string nutHeader = "#!/bin/sq";
	std::string tslHeader = "#!/bin/tsl";

	if (path.length() == 0) {

		switch (type) {
		case 0:
			if (scriptText.substr(0, nutHeader.size()) == nutHeader)
				path = InstanceScript::InstanceNutDef::GetInstanceNutScriptPath(
						creatureInstance->actInst->mZone,
						creatureInstance->actInst->mZoneDefPtr->mGrove);
			else if (scriptText.substr(0, tslHeader.size()) == tslHeader)
				path =
						InstanceScript::InstanceScriptDef::GetInstanceTslScriptPath(
								creatureInstance->actInst->mZone,
								creatureInstance->actInst->mZoneDefPtr->mGrove);
			break;
		case 2:

			if (scriptText.substr(0, nutHeader.size()) == nutHeader)
				path = InstanceScript::InstanceNutDef::GetInstanceNutScriptPath(
						creatureInstance->actInst->mZone,
						creatureInstance->actInst->mZoneDefPtr->mGrove);
			else if (scriptText.substr(0, tslHeader.size()) == tslHeader)
				path =
						InstanceScript::InstanceScriptDef::GetInstanceTslScriptPath(
								creatureInstance->actInst->mZone,
								creatureInstance->actInst->mZoneDefPtr->mGrove);
			break;
		}

		if (path.length() == 0) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Not supported.");
			sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Script not supported.");
		}
	}

	// Create the directory
	string dir = Platform::Dirname(path.c_str());
	Platform::FixPaths(dir);
	Platform::MakeDirectory(dir.c_str());

	g_Log.AddMessageFormat("Saving to %s in %s", path.c_str(), dir.c_str());

	// Save to temporary file first in case the save fails (leaving some hope of recovery)
	string tpath = path;
	tpath.append(".tmp");

	// If the script is empty, delete it
	if (scriptText.length() == 0) {
		Platform::Delete(tpath.c_str());
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Script for %d deleted.",
				creatureInstance->actInst->mZone);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	} else {
		std::ofstream out(tpath.c_str());
		out << scriptText;
		out.close();

		// If we wrote OK, delete the old file and swap in the new one
		if (!Platform::FileExists(path.c_str()) || remove(path.c_str()) == 0) {
			if (rename(tpath.c_str(), path.c_str()) == 0) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Script for %d saved.",
						creatureInstance->actInst->mZone);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
						"OK");
			} else {
				sim->LogMessageL(MSG_WARN,
						"[WARNING] Failed to rename %s to %s, old script will no longer be available, neither will new",
						tpath.c_str(), path.c_str());
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Failed to rename new file, the script may now be missing!");
			}
		} else {
			sim->LogMessageL(MSG_WARN,
					"[WARNING] Failed to remove %s, new script will not be available.",
					path.c_str());
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Failed to delete old script file, new one not swapped in.");
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

