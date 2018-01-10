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

#include <CompilerEnvironment.h>

#include <algorithm>
#include <Cluster.h>
#include <Config.h>
#include <Components.h>
#include <Ability2.h>
#include <VirtualItem.h>
#include <util/Log.h>
#include <curl/curl.h>
#include <dirent.h>
#include "Item.h"
#include "Account.h"
#include "StringUtil.h"

int main(int argc, char *argv[]) {

	if (PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
		return 0;
	}

	el::Level lvl = el::Level::Warning;
	std::vector<std::string> options;
	std::vector<std::string> switches;
	std::string command;
	for (int i = 1; i < argc; i++) {
		if (command == "") {
			if (strcmp(argv[i], "-c") == 0) {
				g_Config.LocalConfigurationPath = argv[++i];
			} else if (strcmp(argv[i], "-d") == 0) {
				lvl = el::Level::Debug;
			} else if (strcmp(argv[i], "-i") == 0) {
				lvl = el::Level::Info;
			} else if (strcmp(argv[i], "-q") == 0) {
				lvl = el::Level::Unknown;
			} else {
				command = argv[i];
			}
		} else {
			if (Util::HasBeginning(argv[i], "--"))
				switches.push_back(argv[i]);
			else
				options.push_back(argv[i]);
		}
	}

	g_Logs.Init(lvl, true);
	g_Logs.data->info("EEAccount");

	curl_global_init(CURL_GLOBAL_DEFAULT);
	g_PlatformTime.Init();

	LoadConfig(
			Platform::JoinPath(g_Config.ResolveLocalConfigurationPath(),
					"ServerConfig.txt"));

	if (g_ClusterManager.Init(
			Platform::JoinPath(g_Config.ResolveLocalConfigurationPath(),
					"Cluster.txt")) < 0) {
		g_Logs.data->error("Failed to connect to cluster.");
		return 1;
	}

	if (command == "create") {
		/* Create account */
		g_ZoneDefManager.LoadData();

		if (options.size() != 3) {
			g_Logs.data->error(
					"'create' requires at least 3 arguments. <userName> <password> <groveName> [--administrator|--sage|--builder|--developer|--tweaker].");
			return 1;
		}
		STRINGLIST roles;
		for (auto it = switches.begin(); it != switches.end(); ++it) {
			if (Util::HasBeginning(*it, "--roles=")) {
				STRINGLIST l;
				Util::Split((*it).substr(8), ",", l);
				roles.insert(roles.end(), l.begin(), l.end());
			}
		}

		std::string regkey = Util::RandomStr(16, false);
		std::string username = options[0].c_str();
		int retval = g_AccountManager.CreateAccount(username.c_str(),
				options[1].c_str(), regkey.c_str(), options[2].c_str());
		if (retval == AccountManager::ACCOUNT_SUCCESS) {
			AccountQuickData aqd =
					g_AccountManager.GetAccountQuickDataByUsername(username);
			AccountData *accPtr = g_AccountManager.FetchIndividualAccount(
					aqd.mID);
			if (accPtr == NULL) {
				g_Logs.data->error("Could not find new account %v [%v]",
						username, aqd.mID);
			} else {
				accPtr->SetRoles(roles);
			}
			g_AccountManager.SaveIndividualAccount(accPtr);
			g_Logs.data->info("Created account %v", username);
		} else {
			g_Logs.data->error("Failed to create account %v [%v]. %v",
					options[0], options[2],
					g_AccountManager.GetErrorMessage(retval));
			return 1;
		}
	} else if (command == "password") {
		/* Set password */
		if (options.size() != 2) {
			g_Logs.data->error(
					"'password' requires at 2 arguments. <userName> <password>.");
			return 1;
		}
		std::string username = options[0].c_str();
		AccountQuickData aqd = g_AccountManager.GetAccountQuickDataByUsername(
				username);
		AccountData *accPtr = g_AccountManager.FetchIndividualAccount(aqd.mID);
		if (accPtr == NULL) {
			g_Logs.data->error("Could not find account %v [%v]", username,
					aqd.mID);
		} else {
			accPtr->SetNewPassword(username.c_str(), options[1].c_str());
			g_AccountManager.SaveIndividualAccount(accPtr);
		}
	} else if (command == "roles") {
		/* Roles */
		if (options.size() < 2) {
			g_Logs.data->error(
					"'roles' requires at least 2 arguments. <userName> <role1> [<role2> [<role3> ..]].");
			return 1;
		}
		std::string username = options[0].c_str();
		options.erase(options.begin());
		AccountQuickData aqd = g_AccountManager.GetAccountQuickDataByUsername(
				username);
		AccountData *accPtr = g_AccountManager.FetchIndividualAccount(aqd.mID);
		if (accPtr == NULL) {
			g_Logs.data->error("Could not find account %v [%v]", username,
					aqd.mID);
		} else {
			accPtr->SetRoles(options);
			g_AccountManager.SaveIndividualAccount(accPtr);
		}
	} else if (command == "delete") {
		/* Roles */
		if (options.size() != 1) {
			g_Logs.data->error(
					"'delete' requires 1 argument. <userName> [--remove-grove].");
			return 1;
		}
		std::string username = options[0].c_str();
		options.erase(options.begin());
		AccountQuickData aqd = g_AccountManager.GetAccountQuickDataByUsername(
				username);
		AccountData *accPtr = g_AccountManager.FetchIndividualAccount(aqd.mID);
		if (accPtr == NULL) {
			g_Logs.data->error("Could not find account %v [%v]", username,
					aqd.mID);
		} else {
			int sessions = g_ClusterManager.CountAccountSessions(aqd.mID, true,
					true);
			if (sessions == 0) {

				std::vector<int> ids;
				g_ZoneDefManager.EnumerateGroveIds(accPtr->ID, 0, ids);

				AccountQuickData aqd =
						g_AccountManager.GetAccountQuickDataByUsername(
								username);
				if (aqd.mID > 0)
					g_ClusterManager.RemoveEntity(&aqd);
				std::vector<CharacterCacheEntry> cces =
						accPtr->characterCache.cacheData;
				for (int i = 0 ; i < MAX_CHAR ; i++) {
					g_AccountManager.DeleteCharacter(i, accPtr);
				}
				/* Need to force actual save of account so the character keys are deleted */
				g_AccountManager.SaveIndividualAccount(accPtr);

				g_ClusterManager.RemoveEntity(accPtr);
				g_ClusterManager.RemoveKey(
						StringUtil::Format("%s:%d",
								LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(),
								aqd.mID));
				g_ClusterManager.RemoveKey(
						StringUtil::Format("%s:%d",
								KEYPREFIX_ACCOUNT_SESSIONS.c_str(), aqd.mID));
				if (aqd.mGroveName.length() > 0)
					g_ClusterManager.RemoveKey(
							StringUtil::Format("%s:%%",
									LISTPREFIX_GROVE_NAME_TO_ZONE_ID.c_str(),
									aqd.mGroveName.c_str()));

				if (std::find(switches.begin(), switches.end(),
						"--remove-grove") != switches.end()
						&& accPtr->GroveName.length() > 0) {

					for (auto it = ids.begin(); it != ids.end(); ++it) {
						if(!g_ZoneDefManager.DeleteZone(*it))
							g_Logs.data->error("Failed to remove zone %v", *it);
					}
				}

				g_Logs.data->info("Removed account %v", username);
			} else {
				g_Logs.data->error(
						"Cannot remove account %v. There are %v active sessions",
						aqd.mID, username, sessions);
			}
		}
	} else {
		g_Logs.data->error("Unknown operation '%v'", command);
		return 1;
	}

	//
	g_ClusterManager.Shutdown(true);
}
