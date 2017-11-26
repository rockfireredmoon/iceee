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

#include "ServiceAuthentication.h"
#include "../Config.h"

#include "../Account.h"
#include "../Scenery2.h"
#include <curl/curl.h>
#include "../md5.hh"
#include "../json/json.h"
#include "../http/TAWClient.h"
#include "../http/SiteClient.h"

ServiceAuthenticationHandler::ServiceAuthenticationHandler() {

}

ServiceAuthenticationHandler::~ServiceAuthenticationHandler() {

}

void ServiceAuthenticationHandler::copyVeteranPlayerDetails(std::string pfUsername, AccountData *data) {

	g_Logs.server->info("Importing veteran details for account %v", data->Name);

	/*
	 * Now contact the partner server and see if there are any groves to transfer
	 */
	TAWClient client(g_Config.LegacyServer);
	std::vector<ZoneDefInfo> zones;
	AccountData otherAccount;
	if (client.getAccountByName(pfUsername, otherAccount)) {
		std::vector<ZoneDefInfo> groves;

		// Get the highest level character (for veteran rewards)
		std::vector<CharacterCacheEntry> cd = otherAccount.characterCache.cacheData;
		g_Logs.server->info("Imported account for %v has %v characters", data->Name, cd.size());
		for (std::vector<CharacterCacheEntry>::iterator it = cd.begin();
				it != cd.end(); ++it) {
			data->VeteranLevel = max(data->VeteranLevel, (*it).level);
			g_Logs.server->info("Character %v is level %v (max now %v)", (*it).level, (*it).display_name.c_str());
		}

		if (data->GroveName.size() > 0) {
			if (client.enumerateGroves(otherAccount.ID, groves)) {
				g_Logs.server->info("Copying %v groves from source account.",
						groves.size());
				for (std::vector<ZoneDefInfo>::iterator it = groves.begin();
						it != groves.end(); ++it) {

					std::string nextGroveName =
							g_ZoneDefManager.GetNextGroveName(data->GroveName);

					ZoneDefInfo zd = *it;

					// Change to the players new account
					zd.mAccountID = data->ID;
					zd.mEditPermissions.clear();
					zd.mWarpName = nextGroveName;
					zd.mGroveName = data->GroveName;

					// Just to be sure ..
					zd.mArena = false;
					zd.mGrove = true;
					zd.mGuildHall = false;

					// Player IDs will be different, might as well just clear them
					zd.mPlayerFilterID.clear();
					zd.mPlayerFilterType = 0;

					g_Logs.server->info("Copying grove '%v' to '%v'.",
							zd.mWarpName.c_str(), nextGroveName.c_str());

					const GroveTemplate *gt =
							g_GroveTemplateManager.GetTemplateByTerrainCfg(
									zd.mTerrainConfig.c_str());
					if (gt == NULL) {
					g_Logs.server->warn(
								"No grove template for %v, will not be able to add default build permission for user %v (%v).",
								zd.mTerrainConfig.c_str(), data->Name, pfUsername.c_str());
					}

					// Create the zone
					int zoneID = g_ZoneDefManager.CreateZone(zd);
					zd.mID = zoneID;

					if (gt != NULL) {
						BuildPermissionArea bp;
						bp.ZoneID = zoneID;
						bp.x1 = gt->mTileX1;
						bp.y1 = gt->mTileY1;
						bp.x2 = gt->mTileX2;
						bp.y2 = gt->mTileY2;

						data->BuildPermissionList.push_back(bp);
						data->PendingMinorUpdates++;
					}

					g_Logs.server->info("New grove zone ID is %v", zd.mID);

					std::vector<SceneryPageKey> pages;
					if (client.getZone((*it).mID, *it, pages)) {
						for (std::vector<SceneryPageKey>::iterator sit =
								pages.begin(); sit != pages.end(); ++sit) {
							SceneryPageKey spk = *sit;
							SceneryPage sp;
							sp.mTileX = spk.x;
							sp.mTileY = spk.y;
							g_Logs.server->info("   Copying tile %v x %v.",
									spk.x, spk.y);
							client.getScenery((*it).mID, sp);

							g_SceneryManager.GetThread(
									"ServiceAuthentication::transferGroves");

							SceneryPage::SCENERY_IT pit;
							for (pit = sp.mSceneryList.begin();
									pit != sp.mSceneryList.end(); ++pit) {
								SceneryObject prop = pit->second;
								SceneryPage * page =
										g_SceneryManager.GetOrCreatePage(zoneID,
												spk.x, spk.y);

								// Give prop new ID
								int newPropID = g_SceneryVars.BaseSceneryID
										+ g_SceneryVars.SceneryAdditive++;
								g_Logs.server->info(
										"       Copying object %v to %v",
										prop.ID, newPropID);
								prop.ID = newPropID;
								SessionVarsChangeData.AddChange();

								page->AddProp(prop, true);
							}

							g_SceneryManager.ReleaseThread();
						}
					} else {
						g_Logs.server->warn("Failed to get zone %v.", (*it).mID);
					}
				}
			} else {
				g_Logs.server->warn("Failed to enumerate groves.");
			}
		}
	} else {
		g_Logs.server->warn("Failed to retrieve legacy account.");
	}

	data->VeteranImported = true;
	data->PendingMinorUpdates++;
}

AccountData * ServiceAuthenticationHandler::onAuthenticate(SimulatorThread *sim,
		std::string loginName, std::string authorizationHash) {

	AccountData *accPtr = NULL;

	/* The web service has different rules for usernames to the server, so we escape
	 * the problematic characters, space, comma, semi-colon, pipe and apersand. It should
	 * be rare we need to decode this (perhaps for some of the new integrated web services)
	 */
	std::string un = loginName;
	Util::ReplaceAll(un, " ", "%20");
	Util::ReplaceAll(un, ",", "%2c");
	Util::ReplaceAll(un, ";", "%3b");
	Util::ReplaceAll(un, "|", "%7c");
	Util::ReplaceAll(un, "&", "%26");
	loginName = un;

	std::vector<std::string> prms;
	Util::Split(authorizationHash, ":", prms);
	if (prms.size() < 4) {
	g_Logs.server->info(
				"Unexpected number of elements in login string for SERVICE authentication. %v",
				authorizationHash.c_str());
	} else {
		/* Now try the integrated website authentication. This is hosted by the Drupal module 'Services' and is
		 * JSON based. The flow is roughly ..
		 *
		 * 1. The client makes an HTTP request to the website asking for a 'CSRF' token.
		 * 2. The website responds with a token.
		 * 3. The client makes a 2nd HTTP request with this token and the username and password.
		 * 4. If username/password OK, the website responds with a session ID and cookie.
		 * 5. The client sends the token, session ID and cookie to the game server (this is the point THIS code comes into play)
		 * 6. The server contacts the website using the token, session ID and cookie and requests full user details.
		 * 7. The website responds with user details including roles (used to configure permissions)
		 * 8. The server looks for a local account with the same username, creating one if required
		 * 9. The server responds to the client saying auth is OK and the user may login
		 */

		// Session - Kept in the user, may be used in game at some point (e.g. private message)
		HTTPD::SiteSession session;
		session.xCSRF = prms[0];
		session.sessionName = prms[2];
		session.sessionID = prms[1];
		session.uid = atoi(prms[3].c_str());

		// The client does the actual communication
		SiteClient sc(g_Config.ServiceAuthURL);

		// Get an X-CSRF-Token
//		if (session.xCSRF.compare("NONE") == 0) {
			sc.refreshXCSRF(&session);
//		}

		std::string readBuffer;
		char url[256];
		Util::SafeFormat(url, sizeof(url), "user/%s", prms[3].c_str());
		int res;

		res = sc.sendRequest(&session, url, readBuffer);

		if (res == 200) {
			// Parse the JSON response from service.

			Json::Value root;
			Json::Reader reader;

			bool parsingSuccessful = reader.parse(readBuffer.c_str(), root);
			if (!parsingSuccessful) {
				g_Logs.server->warn(
						"Invalid data from authentication request.");
				sim->ForceErrorMessage("Account information is not valid data.",
						INFOMSG_ERROR);
				sim->Disconnect("ServiceAuthentication::authenticate");
				return NULL;
			}

			/*
			 * If no roles were returned, authentication failed
			 */
			if (!root.isMember("roles")) {
				sim->ForceErrorMessage(
						"Please sign in through the game website.",
						INFOMSG_ERROR);
				sim->Disconnect("ServiceAuthentication::authenticate");
				g_Logs.server->warn("A likely attempt to login using the client directly.");
				return NULL;
			}

			/* Examine the roles the user has. We can use these to set up the account with
			 * the appropriate permissions
			 */
			const Json::Value roles = root["roles"];

			bool player = false;
			bool sage = false;
			bool admin = false;
			bool builder = false;
			bool developer = false;
			bool tweaker = false;
			bool veteran = false;

			Json::Value::Members members = roles.getMemberNames();

			for (Json::Value::Members::iterator it = members.begin();
					it != members.end(); ++it) {
				std::string mem = *it;
				Json::Value val = roles.get(mem, "");
				if (strcmp(val.asCString(), "players") == 0) {
					player = true;
				} else if (strcmp(val.asCString(), "sages") == 0) {
					sage = true;
				} else if (strcmp(val.asCString(), "developers") == 0) {
					developer = true;
				} else if (strcmp(val.asCString(), "administrator") == 0) {
					admin = true;
				} else if (strcmp(val.asCString(), "builders") == 0) {
					builder = true;
				} else if (strcmp(val.asCString(), "tweakers") == 0) {
					tweaker = true;
				} else if (strcmp(val.asCString(), "veterans") == 0) {
					veteran = true;
				}
			}

			if (!player && !sage && !admin && !developer && !builder) {
				sim->ForceErrorMessage(
						"User is valid, but does not have permission to play game.",
						INFOMSG_ERROR);
				sim->Disconnect("ServiceAuthentication::authenticate");
				g_Logs.server->info("User %v is valid, but does not any permission that allows play.",
						loginName.c_str());
				return NULL;
			}

			// Get the grove name if it's provided
			std::string grove;


			// Grove name
			if (root.isMember("field_grove")) {
				const Json::Value fieldGrove = root["field_grove"];
				if (fieldGrove.size() > 0 && fieldGrove.isMember("und")) {
					const Json::Value fieldUnd = fieldGrove["und"];
					if (fieldUnd.size() > 0) {
						const Json::Value fieldArr = fieldUnd[0];
						grove = fieldArr.get("value", "").asString();
					}
				}
				g_Logs.server->info("Player has grove name of %v", grove.c_str());
			}

			// Planetforever username (for account migration)
			std::string pfUsername = "";
			if (root.isMember("field_pf_username")) {
				const Json::Value fieldPfUsername = root["field_pf_username"];
				if (fieldPfUsername.size() > 0 && fieldPfUsername.isMember("und")) {
					const Json::Value fieldUnd = fieldPfUsername["und"];
					if (fieldUnd.size() > 0) {
						const Json::Value fieldArr = fieldUnd[0];
						pfUsername = fieldArr.get("value", "").asString();
					}
				}
				g_Logs.server->info("Player has PF account name of %v", pfUsername.c_str());
			}

			// Some characters are unacceptable in grove names (i.e. separators in index and data files)
			un = grove;
			Util::ReplaceAll(un, "=", "%61");
			Util::ReplaceAll(un, ";", "%3b");
			Util::ReplaceAll(un, "\"", "%34");
			grove = un;

			/*
			 * Look up the account locally. If it already exists, just load it, otherwise
			 * create a groveless account (without a registration key).
			 */
			AccountQuickData *aqd =
					g_AccountManager.GetAccountQuickDataByUsername(
							loginName.c_str());
			if (aqd != NULL) {
				g_Logs.server->info("External service authenticated %v OK.",
						loginName.c_str());
				accPtr = g_AccountManager.FetchIndividualAccount(aqd->mID);

			} else {
				g_Logs.server->info(
						"Service authenticated OK, but there was no local account with name %v, creating one",
						loginName.c_str());
				g_AccountManager.cs.Enter("ServiceAccountCreation");
				int retval = g_AccountManager.CreateAccountFromService(
						loginName.c_str());
				g_AccountManager.cs.Leave();
				if (retval == g_AccountManager.ACCOUNT_SUCCESS) {
					AccountQuickData *aqd =
							g_AccountManager.GetAccountQuickDataByUsername(
									loginName.c_str());
					accPtr = g_AccountManager.FetchIndividualAccount(aqd->mID);
					accPtr->GroveName = grove;
					accPtr->PendingMinorUpdates++;
				} else {
					char buf[128];
					Util::SafeFormat(buf, sizeof(buf),
							"Failed to create account on game server. %s",
							g_AccountManager.GetErrorMessage(retval));
					sim->ForceErrorMessage(buf, INFOMSG_ERROR);
					sim->Disconnect("ServiceAuthentication::authenticate");
					return NULL;
				}
			}

			/*
			 * Make sure the account has the permissions as set on the external service. This means
			 * for example sages on the website are also sages in the game (saves having to manually
			 * set permissions)
			 */

			//
			// TODO should allow this to be determined by a configuration file, I am forever
			// tweaking role->permission mappings
			//
			if (accPtr != NULL) {
				if (accPtr->GroveName.length() > 0
						&& grove.compare(accPtr->GroveName) != 0) {
					g_Logs.server->info(
							"Attempting to rename player groves from '%v' to '%v'",
							accPtr->GroveName.c_str(), grove.c_str());

					ZoneDefInfo *def = g_ZoneDefManager.GetPointerByGroveName(
							grove.c_str());
					if (def != NULL) {
						char buf[128];
						Util::SafeFormat(buf, sizeof(buf),
								"The Grove name '%s' is already in use by another player. Please choose another. ",
								grove.c_str());
						sim->ForceErrorMessage(buf, INFOMSG_ERROR);
						sim->Disconnect("ServiceAuthentication::authenticate");
						return NULL;
					}

					// TODO prevent rename if grove name already exists under different player
					char buf[256];

					std::vector<int> groveList;
					g_ZoneDefManager.EnumerateGroveIds(accPtr->ID, 0,
							groveList);
					int idx = 1;
					for (std::vector<int>::iterator it = groveList.begin();
							it != groveList.end(); ++it) {
						ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(
								*it);
						if (zone == NULL)
							g_Logs.server->info("Unknown grove %v", *it);
						else {
							Util::SafeFormat(buf, sizeof(buf), "%s%d",
									grove.c_str(), idx);
							g_Logs.server->info("Grove %v is now %v",
									zone->mWarpName.c_str(), buf);
							zone->mWarpName = buf;
							zone->mGroveName = grove;
							zone->PendingChanges++;

							g_ZoneDefManager.UpdateZoneIndex(zone->mID,
									accPtr->ID, zone->mWarpName.c_str(),
									zone->mGroveName.c_str(), false);

							idx++;
						}
					}
					accPtr->PendingMinorUpdates++;
				}

				accPtr->GroveName = grove;

				// Sages and admins get sage
				bool needSage = sage || admin || builder || developer;
				if (needSage
						!= accPtr->HasPermission(Perm_Account,
								Permission_Sage)) {
					accPtr->SetPermission(Perm_Account, "sage", needSage);
					accPtr->PendingMinorUpdates++;
				}

				// Admin and developer gets admin

				bool needAdmin = admin || developer;
				if (needAdmin
						!= accPtr->HasPermission(Perm_Account,
								Permission_Admin)) {
					accPtr->SetPermission(Perm_Account, "admin", needAdmin);
					accPtr->PendingMinorUpdates++;
				}

				// Sages and admins get debug
				bool needDebug = admin || builder || developer;
				if (needDebug
						!= accPtr->HasPermission(Perm_Account,
								Permission_Debug)) {
					accPtr->SetPermission(Perm_Account, "debug", needDebug);
					accPtr->PendingMinorUpdates++;
				}

				// Builders
				bool needBuilder = builder || admin;
				if (needBuilder
						!= accPtr->HasPermission(Perm_Account,
								Permission_Builder)) {
					accPtr->SetPermission(Perm_Account, "builder", needBuilder);
					accPtr->PendingMinorUpdates++;
				}

				// Item give
				bool needItemGive = sage || admin;
				if (needItemGive
						!= accPtr->HasPermission(Perm_Account,
								Permission_ItemGive)) {
					accPtr->SetPermission(Perm_Account, "itemgive",
							needItemGive);
					accPtr->PendingMinorUpdates++;
				}

				// Tweakers
				bool needClientTweak = tweaker || admin || sage || builder
						|| developer;
				if (needClientTweak
						!= accPtr->HasPermission(Perm_Account,
								Permission_TweakClient)) {
					accPtr->SetPermission(Perm_Account, "tweakclient",
							needClientTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needSelfTweak = admin || builder || developer;
				if (needSelfTweak
						!= accPtr->HasPermission(Perm_Account,
								Permission_TweakSelf)) {
					accPtr->SetPermission(Perm_Account, "tweakself",
							needSelfTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needNPCTweak = admin || builder || developer;
				if (needNPCTweak
						!= accPtr->HasPermission(Perm_Account,
								Permission_TweakNPC)) {
					accPtr->SetPermission(Perm_Account, "tweaknpc",
							needNPCTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needOtherTweak = admin;
				if (needOtherTweak
						!= accPtr->HasPermission(Perm_Account,
								Permission_TweakOther)) {
					accPtr->SetPermission(Perm_Account, "tweakother",
							needOtherTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needSysChat = admin;
				if (needSysChat
						!= accPtr->HasPermission(Perm_Account,
								Permission_SysChat)) {
					accPtr->SetPermission(Perm_Account, "syschat", needSysChat);
					accPtr->PendingMinorUpdates++;
				}
				bool needGMChat = admin;
				if (needGMChat
						!= accPtr->HasPermission(Perm_Account,
								Permission_GMChat)) {
					accPtr->SetPermission(Perm_Account, "gmchat", needGMChat);
					accPtr->PendingMinorUpdates++;
				}

				// Get the number of unread private messages waiting
				session.unreadMessages = sc.getUnreadPrivateMessages(&session);
				g_Logs.server->info("Account has %v unread messages",
						session.unreadMessages);

				if (!accPtr->VeteranImported) {
					copyVeteranPlayerDetails(pfUsername, accPtr);
				}

				accPtr->SiteSession.CopyFrom(&session);
			}
		} else {
			sim->ForceErrorMessage(
					"User not found on external service, please contact site administrator for assistance.",
					INFOMSG_ERROR);
			sim->Disconnect("ServiceAuthentication::authenticate");
			g_Logs.server->info(
					"Service returned error when confirming authentication. Status %v",
					res);
			return NULL;
		}
	}

	return accPtr;
}
