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

#ifndef SCRIPTHANDLERS_H
#define SCRIPTHANDLERS_H

#include "Query.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

class AbstractScriptHandler: public QueryHandler {
public:
	~AbstractScriptHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);

	virtual int handleScriptQuery(bool ownPlayer,
			int instanceID,
			int questID,
			int type, string scriptName,
			ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
			ActiveInstance *instance, CreatureInstance *targetCreature,
			const fs::path &path, SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance)=0;
};

class ScriptLoadHandler: public AbstractScriptHandler {
public:
	~ScriptLoadHandler() {
	}

	int handleScriptQuery(bool ownPlayer,
			int instanceID,
			int questID,int type, string scriptName,
			ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
			ActiveInstance *instance, CreatureInstance *targetCreature,
			const fs::path &path, SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptListHandler: public QueryHandler {
public:
	~ScriptListHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptKillHandler: public AbstractScriptHandler {
public:
	~ScriptKillHandler() {
	}

	int handleScriptQuery(bool ownPlayer,
			int instanceID,
			int questID,int type, string scriptName,
			ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
			ActiveInstance *instance, CreatureInstance *targetCreature,
			const fs::path &path, SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptRunHandler: public AbstractScriptHandler {
public:
	~ScriptRunHandler() {
	}

	int handleScriptQuery(bool ownPlayer,
			int instanceID,
			int questID,int type, string scriptName,
			ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
			ActiveInstance *instance, CreatureInstance *targetCreature,
			const fs::path &path, SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptSaveHandler: public AbstractScriptHandler {
public:
	~ScriptSaveHandler() {
	}

	int handleScriptQuery(bool ownPlayer,
			int instanceID,
			int questID,int type, string scriptName,
			ScriptCore::NutPlayer *player, ScriptCore::ScriptPlayer *oldPlayer,
			ActiveInstance *instance, CreatureInstance *targetCreature,
			const fs::path &path, SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif

