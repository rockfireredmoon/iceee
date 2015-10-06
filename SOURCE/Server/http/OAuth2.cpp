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

#include "OAuth2.h"
#include "../Config.h"
#include "../Util.h"
#include "../Account.h"
#include "../StringList.h"
#include <vector>

using namespace HTTPD;

static OAuth2Client * findClient(std::string clientID) {
	OAuth2Client *client = NULL;
	for (std::vector<OAuth2Client*>::iterator it =
			g_Config.OAuth2Clients.begin(); it != g_Config.OAuth2Clients.end();
			++it) {
		OAuth2Client *c = *it;
		if (c->ClientId.compare(clientID) == 0) {
			client = c;
			break;
		}
	}
	return client;
}

//
// AuthHandler
//

bool AuthHandler::handleGet(CivetServer *server, struct mg_connection *conn) {

	std::string encodedRedirectURI;
	std::string clientID;
	std::string scope;
	std::string responseType;
	std::string loginError;

	bool err = CivetServer::getParam(conn, "e", loginError);

	if (CivetServer::getParam(conn, "redirect_uri", encodedRedirectURI)
			&& CivetServer::getParam(conn, "client_id", clientID)
			&& CivetServer::getParam(conn, "scope", scope)
			&& CivetServer::getParam(conn, "response_type", responseType)) {

		std::string response;
		std::string redirectURI;

		CivetServer::urlDecode(encodedRedirectURI, redirectURI, false);

		OAuth2Client *client = findClient(clientID);
		if (client == NULL) {
			writeStatus(server, conn, 403, "Forbidden", "Unknown client.");
		} else {
			response += "<html><head>";
			response += "<link rel=\"stylesheet\" href=\"../main.css\">\n";
			response += "<script src=\"../md5.js\" type=\"text/javascript\">\n";
			response += "</script>\n";
			response += "<script type=\"text/javascript\">\n";
			response += "function pf_HashAuth(frm) { \n ";
			response += "document.getElementById(\"hash\").value = ";
			response += "md5(document.getElementById(\"username\").value + ";
			response +=
					"\":\" + md5(document.getElementById(\"password\").value) + \":"
							+ std::string(g_AuthKey) + "\");\n";
			response += "document.getElementById(\"password\").value = \"\";\n";
			response += "return true;\n }\n";
			response += "</script>\n";
			response += "</head>\n";
			response += "<body>\n";
			response += "<h1>Planet Forever Authentication</h1>\n";
			if (err) {
				response +=
						"<div class=\"errmsg\">Invalid username or password</div>\n";
			}
			response +=
					"<form onsubmit=\"return pf_HashAuth(this);\" action=\"login\" method=\"post\">\n";
			response +=
					"<div class=\"usernameRow\"><label class=\"usernameLabel\">Username:</label>\n";
			response +=
					"<input type=\"text\" id=\"username\" name=\"username\" size=\"20\"/></div>\n";
			response +=
					"<div class=\"passwordRow\"><label class=\"passwordLabel\">Password:</label>\n";
			response +=
					"<input type=\"password\" id=\"password\" name=\"password\" size=\"20\"/></div>\n";
			response += "<div class=\"actionRow\">\n";
			response += "<input type=\"hidden\" name=\"client_id\" value=\""
					+ clientID + "\"/>\n";
			response += "<input type=\"hidden\" name=\"redirect_uri\" value=\""
					+ redirectURI + "\"/>\n";
			response += "<input type=\"hidden\" name=\"response_type\" value=\""
					+ responseType + "\"/>\n";
			response += "<input type=\"hidden\" name=\"scope\" value=\"" + scope
					+ "\"/>\n";
			response += "<input type=\"hidden\" name=\"hash\" id=\"hash\"/>\n";
			response += "<input type=\"submit\" value=\"Login\"/>\n";
			response += "</div>\n</form>\n</html>\n";
		}

	} else {
		writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
	}

	return true;
}

//
// LoginHandler
//

bool LoginHandler::handlePost(CivetServer *server, struct mg_connection *conn) {

	char post_data[1024];
	int post_data_len;
	char encodedRedirectURI[1024];
	char redirectURI[1024];
	char clientID[64];
	char scope[64];
	char hash[256];
	char username[64];
	char responseType[64];

	post_data_len = mg_read(conn, post_data, sizeof(post_data));

	if (mg_get_var(post_data, post_data_len, "redirect_uri", encodedRedirectURI,
			sizeof(encodedRedirectURI)) > 0
			&& mg_get_var(post_data, post_data_len, "client_id", clientID,
					sizeof(clientID)) > 0
			&& mg_get_var(post_data, post_data_len, "scope", scope,
					sizeof(scope)) > 0
			&& mg_get_var(post_data, post_data_len, "response_type",
					responseType, sizeof(responseType)) > 0
			&& mg_get_var(post_data, post_data_len, "hash", hash, sizeof(hash))
					> 0
			&& mg_get_var(post_data, post_data_len, "username", username,
					sizeof(username)) > 0) {

		OAuth2Client *client = findClient(clientID);
		mg_url_decode(encodedRedirectURI, sizeof(encodedRedirectURI),
				redirectURI, sizeof(redirectURI), false);

		mg_printf(conn, "HTTP/1.1 301 Moved Permanently\r\n);");
		if (client == NULL
				|| !Util::HasBeginning(redirectURI, client->RedirectURL)) {
			g_Log.AddMessageFormat(
					"[WARNING] Attempt to use login api from invalid client (%s for %s against %s)",
					clientID, redirectURI,
					client == NULL ? "None" : client->RedirectURL.c_str());
			mg_printf(conn,
					"Location: auth?e=2&client_id=%s&redirect_uri=%s&scope=%s&response_type=%s\r\n\r\n",
					clientID, redirectURI, scope, responseType);
		} else {
			g_Log.AddMessageFormat(
					"[REMOVEME] redir: %s   client: %s   scope: %s   responsetype: %s   username: %s   hash: %s",
					redirectURI, clientID, scope, responseType, username, hash);
			std::string hashed;
			AccountData::GenerateSaltedHash(hash, hashed);

			g_AccountManager.cs.Enter(
					"SimulatorThread::handle_web_authenticate");
			AccountData *accPtr = g_AccountManager.GetValidLogin(username,
					hash);
			g_AccountManager.cs.Leave();
			if (accPtr == NULL) {
				mg_printf(conn,
						"Location: auth?e=1&client_id=%s&redirect_uri=%s&scope=%s&response_type=%s\r\n\r\n",
						clientID, redirectURI, scope, responseType);
			} else {
				std::string authCode = Util::RandomStr(22, true);
				AccountQuickData *aqd =
						g_AccountManager.GetAccountQuickDataByUsername(
								accPtr->Name);
				aqd->mAuthCode = authCode;
				aqd->mAuthCodeExpire = g_ServerTime + 30000; // 30 seconds
				mg_printf(conn, "Location: %s&code=%s\r\n\r\n", redirectURI,
						authCode.c_str());
			}
		}
	} else {
		writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
	}

	return true;
}

//
// TokenHandler
//

bool TokenHandler::handleGet(CivetServer *server, struct mg_connection *conn) {

	std::string encodedRedirectURI;
	std::string clientID;
	std::string grantType;
	std::string code;
	std::string clientSecret;

	if (CivetServer::getParam(conn, "redirect_uri", encodedRedirectURI)
			&& CivetServer::getParam(conn, "grant_type", grantType)
			&& CivetServer::getParam(conn, "client_id", clientID)
			&& CivetServer::getParam(conn, "code", code)) {

		std::string response;
		std::string redirectURI;

		CivetServer::urlDecode(encodedRedirectURI, redirectURI, false);
		OAuth2Client *client = findClient(clientID);

		if (client == NULL
				|| !Util::HasBeginning(redirectURI, client->RedirectURL)) {
			g_Log.AddMessageFormat(
					"[WARNING] Attempt to use token api from invalid client (%s for %s against %s)",
					clientID.c_str(), redirectURI.c_str(),
					client == NULL ? "None" : client->RedirectURL.c_str());
			writeJSON200(server, conn, "{ \"error\":\"invalid_request\"}");
		} else {
			g_Log.AddMessageFormat(
					"[REMOVEME] redir: %s   client: %s   code: %s   clientSecret: %s   grantType: %s",
					redirectURI.c_str(), clientID.c_str(), code.c_str(),
					clientSecret.c_str(), grantType.c_str());


			if(client->ClientSecret.size() > 0 && !CivetServer::getParam(conn, "redirect_uri", encodedRedirectURI)) {
				g_Log.AddMessageFormat(
									"[WARNING] Invalid client secret used  (%s for %s against %s)",
									clientID.c_str(), redirectURI.c_str(),
									client == NULL ? "None" : client->RedirectURL.c_str());
				writeJSON200(server, conn, "{ \"error\":\"invalid_request\"}");
			}
			else {
				AccountData *accPtr = g_AccountManager.FetchAccountWithAuthCode(
						code.c_str());
				if (accPtr == NULL) {
					g_Log.AddMessageFormat(
							"[WARNING] Attempt to get non-existing auth code.");
					writeJSON200(server, conn, "{ \"error\":\"invalid_request\"}");
				} else
					writeJSON200(server, conn, "{ \"access_token\":\"" + Util::RandomStr(22, true) + "\"}");
			}
		}
	}
	else {
		writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
	}

	return true;
}
