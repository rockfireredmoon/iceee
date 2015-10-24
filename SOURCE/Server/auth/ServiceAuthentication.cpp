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
#include "../StringList.h"
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

void ServiceAuthenticationHandler::transferGroves(AccountData *data) {
	/*
	 * Now contact the partner server and see if there are any groves to transfer
	 */
	TAWClient client(g_Config.LegacyServer);
	std::vector<ZoneDefInfo> zones;
	AccountData otherAccount;
	if(client.getAccountByName(data->Name, otherAccount)) {
		std::vector<ZoneDefInfo> groves;
		if(client.enumerateGroves(otherAccount.ID, groves)) {
			g_Log.AddMessageFormat("Copying %d groves from source account.", groves.size());
			for(std::vector<ZoneDefInfo>::iterator it = groves.begin(); it != groves.end(); ++it) {

				std::string nextGroveName = g_ZoneDefManager.GetNextGroveName(data->GroveName);

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

				g_Log.AddMessageFormat("Copying grove '%s' to '%s'.", zd.mWarpName.c_str(), nextGroveName.c_str());

				const GroveTemplate *gt = g_GroveTemplateManager.GetTemplateByTerrainCfg(zd.mTerrainConfig.c_str());
				if(gt == NULL) {
					g_Log.AddMessageFormat("No grove template for %s, will not be able to add default build permission for user %s.", zd.mTerrainConfig.c_str(), data->Name);
				}

				// Create the zone
				int zoneID = g_ZoneDefManager.CreateZone(zd);
				zd.mID = zoneID;

				if(gt != NULL){
					BuildPermissionArea bp;
					bp.ZoneID = zoneID;
					bp.x1 = gt->mTileX1;
					bp.y1 = gt->mTileY1;
					bp.x2 = gt->mTileX2;
					bp.y2 = gt->mTileY2;

					data->BuildPermissionList.push_back(bp);
					data->PendingMinorUpdates++;
				}

				g_Log.AddMessageFormat("New grove zone ID is %d", zd.mID);

				std::vector<SceneryPageKey> pages;
				if(client.getZone((*it).mID, *it, pages)) {
					for(std::vector<SceneryPageKey>::iterator sit = pages.begin(); sit != pages.end(); ++sit) {
						SceneryPageKey spk = *sit;
						SceneryPage sp;
						sp.mTileX = spk.x;
						sp.mTileY = spk.y;
						g_Log.AddMessageFormat("   Copying tile %d x %d.", spk.x, spk.y);
						client.getScenery((*it).mID, sp);

						g_SceneryManager.GetThread("ServiceAuthentication::transferGroves");

						SceneryPage::SCENERY_IT pit;
						for(pit = sp.mSceneryList.begin(); pit != sp.mSceneryList.end(); ++pit) {
							SceneryObject prop = pit->second;
							SceneryPage * page = g_SceneryManager.GetOrCreatePage(zoneID, spk.x, spk.y);

							// Give prop new ID
							int newPropID = g_SceneryVars.BaseSceneryID + g_SceneryVars.SceneryAdditive++;
							g_Log.AddMessageFormat("       Copying object %d to %d", prop.ID, newPropID);
							prop.ID = newPropID;
							SessionVarsChangeData.AddChange();

							page->AddProp(prop, true);
						}

						g_SceneryManager.ReleaseThread();
					}
				}
				else {
					g_Log.AddMessageFormat("[WARNING] Failed to get zone %d.", (*it).mID);
				}
			}
		}
		else {
			g_Log.AddMessageFormat("[WARNING] Failed to enumerate groves.");
		}
	}
	else {
		g_Log.AddMessageFormat("[WARNING] Failed to retrieve legacy account.");
	}
}

AccountData * ServiceAuthenticationHandler::onAuthenticate(SimulatorThread *sim, std::string loginName, std::string authorizationHash) {

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
	if(prms.size() < 4) {
		g_Log.AddMessageFormat("Unexpected number of elements in login string for SERVICE authentication. %s", authorizationHash.c_str());
	}
	else {
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
		if(session.xCSRF.compare("NONE") == 0) {
			sc.refreshXCSRF(&session);
		}

		std::string readBuffer;
		char url[256];
		Util::SafeFormat(url, sizeof(url), "user/%s", prms[3].c_str());
		int res;

		res = sc.sendRequest(&session, url, readBuffer);

		if(res == 200) {
			// Parse the JSON response from service.

			Json::Value root;
			Json::Reader reader;
			bool parsingSuccessful = reader.parse( readBuffer.c_str(), root );
			if ( !parsingSuccessful )
			{
				g_Log.AddMessageFormat("[WARNING] Invalid data from authentication request.");
				sim->ForceErrorMessage("Account information is not valid data.", INFOMSG_ERROR);
				sim->Disconnect("ServiceAuthentication::authenticate");
				return NULL;
			}

			/*
			 * If no roles were returned, authentication failed
			 */
			if(!root.isMember("roles")) {
				sim->ForceErrorMessage("Please sign in through the game website.", INFOMSG_ERROR);
				sim->Disconnect("ServiceAuthentication::authenticate");
				g_Log.AddMessageFormat("[WARNING] A likely attempt to login using the client directly.");
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

			for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it) {
				std::string mem = *it;
				Json::Value val = roles.get(mem, "");
				if(strcmp(val.asCString(), "players") == 0) {
					player = true;
				}
				else if(strcmp(val.asCString(), "sages") == 0) {
					sage = true;
				}
				else if(strcmp(val.asCString(), "developers") == 0) {
					developer = true;
				}
				else if(strcmp(val.asCString(), "administrator") == 0) {
					admin = true;
				}
				else if(strcmp(val.asCString(), "builders") == 0) {
					builder = true;
				}
				else if(strcmp(val.asCString(), "tweakers") == 0) {
					tweaker = true;
				}
				else if(strcmp(val.asCString(), "veterans") == 0) {
					veteran = true;
				}
			}

			if(!player && !sage && !admin && !developer && !builder) {
				sim->ForceErrorMessage("User is valid, but does not have permission to play game.", INFOMSG_ERROR);
				sim->Disconnect("ServiceAuthentication::authenticate");
				g_Log.AddMessageFormat("User %s is valid, but does not any permission that allows play.", loginName.c_str());
				return NULL;
			}

			// Get the grove name if it's provided
			std::string grove;

			// Broken?

//			if(root.isMember("field_grove")) {
//				const Json::Value fieldGrove = root["field_grove"];
//				if(fieldGrove.isMember("und")) {
//					const Json::Value fieldUnd = fieldGrove["und"];
//					const Json::Value fieldArr = fieldUnd[0];
//					grove = fieldArr.get("value","").asString();
//				}
//			}


			/*
			 * Look up the account locally. If it already exists, just load it, otherwise
			 * create a groveless account (without a registration key).
			 */
			AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(loginName.c_str());
			if(aqd != NULL) {
				g_Log.AddMessageFormat("External service authenticated %s OK.", loginName.c_str());
				accPtr = g_AccountManager.FetchIndividualAccount(aqd->mID);

				// TODO temp
				if(veteran) {
					if(grove.size() > 0)
						transferGroves(accPtr);
					else
						g_Log.AddMessageFormat("No grove name supplied, groves will not be imported.", loginName.c_str());
				}
			}
			else {
				g_Log.AddMessageFormat("Service authenticated OK, but there was no local account with name %s, creating one", loginName.c_str());
				g_AccountManager.cs.Enter("ServiceAccountCreation");
				int retval = g_AccountManager.CreateAccountFromService(loginName.c_str());
				g_AccountManager.cs.Leave();
				if(retval == g_AccountManager.ACCOUNT_SUCCESS) {
					AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(loginName.c_str());
					accPtr = g_AccountManager.FetchIndividualAccount(aqd->mID);
					accPtr->GroveName = grove;
					accPtr->PendingMinorUpdates++;
					if(veteran) {
						if(grove.size() > 0)
							transferGroves(accPtr);
						else
							g_Log.AddMessageFormat("No grove name supplied, groves will not be imported.", loginName.c_str());
					}
					else {
						g_Log.AddMessageFormat("Not a veteran, no groves will be imported.", loginName.c_str());
					}
				}
				else {
					char buf[128];
					Util::SafeFormat(buf, sizeof(buf), "Failed to create account on game server. %s", g_AccountManager.GetErrorMessage(retval));
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
			if(accPtr != NULL) {

				// Sages and admins get sage
				bool needSage = sage || admin || builder || developer;
				if(needSage != accPtr->HasPermission(Perm_Account, Permission_Sage)) {
					accPtr->SetPermission(Perm_Account, "sage", needSage);
					accPtr->PendingMinorUpdates++;
				}

				// Admin and developer gets admin

				bool needAdmin = admin || developer;
				if(needAdmin != accPtr->HasPermission(Perm_Account, Permission_Admin)) {
					accPtr->SetPermission(Perm_Account, "admin", needAdmin);
					accPtr->PendingMinorUpdates++;
				}

				// Sages and admins get debug
				bool needDebug = admin || builder || developer;
				if(needDebug != accPtr->HasPermission(Perm_Account, Permission_Debug)) {
					accPtr->SetPermission(Perm_Account, "debug", needDebug);
					accPtr->PendingMinorUpdates++;
				}

				// Builders
				bool needBuilder = builder || admin;
				if(needBuilder != accPtr->HasPermission(Perm_Account, Permission_Builder)) {
					accPtr->SetPermission(Perm_Account, "builder", needBuilder);
					accPtr->PendingMinorUpdates++;
				}

				// Item give
				bool needItemGive = sage || admin;
				if(needItemGive != accPtr->HasPermission(Perm_Account, Permission_ItemGive)) {
					accPtr->SetPermission(Perm_Account, "itemgive", needItemGive);
					accPtr->PendingMinorUpdates++;
				}

				// Tweakers
				bool needClientTweak = tweaker || admin || sage || builder || developer;
				if(needClientTweak != accPtr->HasPermission(Perm_Account, Permission_TweakClient)) {
					accPtr->SetPermission(Perm_Account, "tweakclient", needClientTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needSelfTweak = admin || builder || developer;
				if(needSelfTweak != accPtr->HasPermission(Perm_Account, Permission_TweakSelf)) {
					accPtr->SetPermission(Perm_Account, "tweakself", needSelfTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needNPCTweak = admin || builder || developer;
				if(needNPCTweak != accPtr->HasPermission(Perm_Account, Permission_TweakNPC)) {
					accPtr->SetPermission(Perm_Account, "tweaknpc", needNPCTweak);
					accPtr->PendingMinorUpdates++;
				}
				bool needOtherTweak = admin;
				if(needOtherTweak != accPtr->HasPermission(Perm_Account, Permission_TweakOther)) {
					accPtr->SetPermission(Perm_Account, "tweakother", needOtherTweak);
					accPtr->PendingMinorUpdates++;
				}

				// Get the number of unread private messages waiting
				session.unreadMessages = sc.getUnreadPrivateMessages(&session);
				g_Log.AddMessageFormat("Account has %d unread messages", session.unreadMessages);

				accPtr->SiteSession.CopyFrom(&session);
			}
		}
		else {
			sim->ForceErrorMessage("User not found on external service, please contact site administrator for assistance.", INFOMSG_ERROR);
			sim->Disconnect("ServiceAuthentication::authenticate");
			g_Log.AddMessageFormat("Service returned error when confirming authentication. Status %d", res);
			return NULL;
		}
	}

	return accPtr;
}
