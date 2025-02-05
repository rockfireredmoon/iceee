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
#include "json/json.h"
#include "Item.h"
#include "Account.h"

using namespace std;
namespace fs = filesystem;

int do_cmd(const char *this_exe, string command, vector<string> options, vector<string> switches);

int main(int argc, char *argv[]) {

	if (PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
		return 0;
	}

	el::Level lvl = el::Level::Warning;
	vector<string> options;
	vector<string> switches;
	string command;
	bool configSet = false;
	for (int i = 1; i < argc; i++) {
		if (command == "") {
			if (strcmp(argv[i], "-c") == 0) {
				if (!configSet) {
					configSet = true;
					g_Config.LocalConfigurationPath.clear();
				}
				g_Config.LocalConfigurationPath.push_back(argv[++i]);
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

	g_Logs.Init(lvl, true, "LogConfig.txt");
	g_Logs.data->info("EEAccount");

	curl_global_init(CURL_GLOBAL_DEFAULT);
	g_PlatformTime.Init();

	auto paths = g_Config.ResolveLocalConfigurationPath();

	for (auto it = paths.begin(); it != paths.end(); ++it) {
		auto dir = *it;
		auto filename = dir / "ServerConfig.txt";
		if (!g_Config.LoadConfig(filename) && it == paths.begin())
			g_Logs.data->error("Could not open server configuration file: %v",
					filename);
	}

	g_ClusterManager.mNoEvents = true;
	for (auto dir : paths) {
		g_ClusterManager.LoadConfiguration(dir / "Cluster.txt");
	}

	if (!g_ClusterManager.Init())
		return 1;

	//
	int retval = do_cmd(argv[0], command, options, switches);
	g_ClusterManager.Shutdown(true);
	g_Logs.FlushAll();
	g_Logs.CloseAll();
	return retval;
}

int do_cmd(const char *this_exe, string command, vector<string> options, vector<string> switches) {
	if (command == "help" || command == "") {
		g_Logs.data->verbose(0, "Manage local game accounts");
		g_Logs.data->verbose(0, "");
		g_Logs.data->verbose(0, "For most commands, if you supply no further arguments, help for that command will be printed.");
		g_Logs.data->verbose(0, "");
		g_Logs.data->verbose(0, Util::Format("%s create - create new accounts", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s password - change passwords", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s show - show account details", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s list - list", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s roles - show or change account roles", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s delete - delete accounts", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s groves - show account grove details", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s remove-grove - remove player groves", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s keys - list currently available registration keys", this_exe));
		g_Logs.data->verbose(0, Util::Format("%s genkeys - generate a new set registration keys", this_exe));
	} else if (command == "list") {
		std::string glob = "*";
		if (options.size() > 0) {
			glob = options[0];
		}
		auto matches = g_AccountManager.MatchAccountNames(glob);
		for (auto it = matches.begin(); it != matches.end(); ++it) {
			printf("%s\n", (*it).c_str());
		}
	} else if (command == "keys") {
		auto matches = g_AccountManager.GetRegistrationKeys();
		for (auto it = matches.begin(); it != matches.end(); ++it) {
			printf("%s\n", (*it).c_str());
		}
	} else if (command == "genkeys") {
		int keys = 50;
		if (options.size() > 0) {
			keys = Util::SafeParseInt(options[0].c_str());
			if (keys < 1 || keys > 500) {
				g_Logs.data->error("You may generate between 1 and 500 keys.");
				return 1;
			}
			for (int i = 0; i < keys; i++) {
				std::string genkey = Util::RandomStr(16, false);
				printf("%s\n", genkey.c_str());
				g_AccountManager.ImportKey(genkey.c_str());
			}
		} else {
			g_Logs.data->error(
					"'genkeys' requires a single argument. <numberOfKeys>.");
			return 1;
		}
	} else if (command == "create") {
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
			g_AccountManager.SaveIndividualAccount(accPtr, true);
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
					"'password' requires 2 arguments. <userName> <password>.");
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
			g_AccountManager.SaveIndividualAccount(accPtr, true);
			g_AccountManager.AppendQuickData(accPtr, true);
			g_Logs.data->info("Password changed for account %v", username);
		}
	} else if (command == "show") {
		/* Set password */
		if (options.size() != 1) {
			g_Logs.data->error("'show' requires 1 argument. <userName>");
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
			Json::Value str;
			accPtr->WriteToJSON(str);
			Json::StyledWriter writer;
			printf("%s\n", writer.write(str).c_str());
		}
	} else if (command == "groves") {
		/* Groves */
		if (options.size() < 1) {
			g_Logs.data->error("'groves' requires 1 argument. <userName>.");
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
			std::vector<int> ids;
			g_ZoneDefManager.EnumerateGroveIds(accPtr->ID, 0, ids);
			Json::Value str;
			for (auto it = ids.begin(); it != ids.end(); ++it) {
				ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(*it);
				if (zd == NULL)
					g_Logs.data->error("No such zone %v", *it);
				else {
					Json::Value zstr;
					zd->WriteToJSON(zstr);
					str.append(zstr);
				}
			}
			Json::StyledWriter writer;
			printf("%s\n", writer.write(str).c_str());
		}
	} else if (command == "remove-grove") {
		/* Groves */
		if (options.size() < 1) {
			g_Logs.data->error(
					"'remove-grove' requires 1 arguments. <groveId>.");
			return 1;
		}
		int gid = Util::SafeParseInt(options[0].c_str());
		ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(gid);
		if (zd == NULL) {
			g_Logs.data->error("Could not find zone %v", gid);
		} else {
			if (zd->mGrove) {
				if (!g_ZoneDefManager.DeleteZone(gid))
					g_Logs.data->error("Failed to remove zone %v", gid);
			} else
				g_Logs.data->error("Zone %v (%v) is not a grove", gid,
						zd->mName);
		}
	} else if (command == "roles") {
		/* Roles */
		if (options.size() < 1) {
			g_Logs.data->error(
					"'roles' requires at least 1 argument. <userName> [<role1> [<role2> [<role3> ..]].");
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
			if (options.size() == 0) {
				for (int a = 0; a < MaxPermissionDef; a++)
					if ((accPtr->PermissionSet[PermissionDef[a].index]
							& PermissionDef[a].flag) == PermissionDef[a].flag)
						printf("%s\n", PermissionDef[a].name);
			} else {
				accPtr->SetRoles(options);
				g_AccountManager.SaveIndividualAccount(accPtr, true);
			}
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
				for (int i = 0; i < MAX_CHAR; i++) {
					g_AccountManager.DeleteCharacter(i, accPtr);
				}
				/* Need to force actual save of account so the character keys are deleted */
				g_AccountManager.SaveIndividualAccount(accPtr, true);

				g_ClusterManager.RemoveEntity(accPtr);
				g_ClusterManager.RemoveKey(
						Util::Format("%s:%d",
								LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(),
								aqd.mID));
				g_ClusterManager.RemoveKey(
						Util::Format("%s:%d",
								KEYPREFIX_ACCOUNT_SESSIONS.c_str(), aqd.mID));
				if (aqd.mGroveName.length() > 0)
					g_ClusterManager.RemoveKey(
							Util::Format("%s:%%",
									LISTPREFIX_GROVE_NAME_TO_ZONE_ID.c_str(),
									aqd.mGroveName.c_str()));

				if (std::find(switches.begin(), switches.end(),
						"--remove-grove") != switches.end()
						&& accPtr->GroveName.length() > 0) {

					for (auto it = ids.begin(); it != ids.end(); ++it) {
						if (!g_ZoneDefManager.DeleteZone(*it))
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
	return 0;
}
