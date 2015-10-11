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
#include <curl/curl.h>
#include "../md5.hh"
#include "../json/json.h"


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

ServiceAuthenticationHandler::ServiceAuthenticationHandler() {

}

ServiceAuthenticationHandler::~ServiceAuthenticationHandler() {

}


AccountData * ServiceAuthenticationHandler::onAuthenticate(SimulatorThread *sim, std::string loginName, std::string authorizationHash) {

	AccountData *accPtr = NULL;

	/* The web service has different rules for usernames to the server, so we escape
	 * the problematic characters, space, comma, semi-colon, pipe and apersand. It should
	 * be rare we need to decode this (perhaps for some of the new integrate web services)
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
		/* Now try the integrated website authentication. This is host by the Drupal module 'Services' and is
		 * JSON based. The flow is roughly ..
		 *
		 * 1. The client makes an HTTP request to the website asking for a 'CSRF' token.
		 * 2. The website responds with a token.
		 * 3. The client makes a 2nd HTTP request with this token and the username and password.
		 * 4. If username/password OK, the website responds with a session ID and cookie.
		 * 5. The client sends the token, session ID and cookie to the game server (this is the point THIS code comes into play)
		 * 6. The server contacts the website using the token, session ID and cookie and requests full user details.
		 * 7. The website responsds with user details including roles (used to configure permissions)
		 * 8. The server looks for a local account with the same username, creating one if required
		 * 9. The server responds to the client saying auth is OK and the user may login
		 */

		struct curl_slist *headers = NULL;
		CURL *curl;
		curl = curl_easy_init();
		if(curl) {
			char url[256];
			char token[256];
			char cookie[256];

			std::string readBuffer;

			Util::SafeFormat(url, sizeof(url), "%s/user/%s.json", g_Config.ServiceAuthURL.c_str(), prms[3].c_str());
			Util::SafeFormat(token, sizeof(token), "X-CSRF-Token: %s", prms[0].c_str());
			Util::SafeFormat(cookie, sizeof(cookie), "Cookie: %s=%s", prms[2].c_str(),prms[1].c_str());

			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "EETAW");

			headers = curl_slist_append(headers, token);
			headers = curl_slist_append(headers, cookie);
			headers = curl_slist_append(headers, "Content-Type: application/json");

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

			// TODO might need config item to disable SSL verification
			//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

			CURLcode res;
			res = curl_easy_perform(curl);

			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			if(res == CURLE_OK) {

				// Parse the JSON response from service.

				Json::Value root;
				Json::Reader reader;
				bool parsingSuccessful = reader.parse( readBuffer.c_str(), root );
				if ( !parsingSuccessful )
				{
					sim->ForceErrorMessage("Account information is not valid data.", INFOMSG_ERROR);
					sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
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
				bool veterans = false;

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
						veterans = true;
					}
				}

				if(!player && !sage && !admin && !developer && !builder) {
					sim->ForceErrorMessage("User is valid, but does not have permission to play game.", INFOMSG_ERROR);
					sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
					g_Log.AddMessageFormat("User %s is valid, but does not have player or sage permissions.", loginName.c_str());
					return NULL;
				}

				/*
				 * Look up the account locally. If it already exists, just load it, otherwise
				 * create a groveless account (without a registration key).
				 */
				AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(loginName.c_str());
				if(aqd != NULL) {
					g_Log.AddMessageFormat("External service authenticated %s OK.", loginName.c_str());
					accPtr = g_AccountManager.FetchIndividualAccount(aqd->mID);
				}
				else {
					g_Log.AddMessageFormat("Service authenticated OK, but there was no local account with name %s, creating one", loginName.c_str());
					g_AccountManager.cs.Enter("ServiceAccountCreation");
					int retval = g_AccountManager.CreateAccountFromService(loginName.c_str());
					g_AccountManager.cs.Leave();
					if(retval == g_AccountManager.ACCOUNT_SUCCESS) {
						AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(loginName.c_str());
						accPtr = g_AccountManager.FetchIndividualAccount(aqd->mID);
					}
					else {
						char buf[128];
						Util::SafeFormat(buf, sizeof(buf), "Failed to create account on game server. %s", g_AccountManager.GetErrorMessage(retval));
						sim->ForceErrorMessage(buf, INFOMSG_ERROR);
						sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
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

					// Only admin gets admin
					if(admin != accPtr->HasPermission(Perm_Account, Permission_Admin)) {
						accPtr->SetPermission(Perm_Account, "admin", admin);
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
				}
			}
			else {
				sim->ForceErrorMessage("User not found on external service, please contact site administrator for assistance.", INFOMSG_ERROR);
				sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
				g_Log.AddMessageFormat("Service returned error when confirming authentication. Status %d", res);
				return NULL;
			}
		}
	}

	return accPtr;
}
