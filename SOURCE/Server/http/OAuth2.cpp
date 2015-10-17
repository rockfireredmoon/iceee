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

bool AuthorizeHandler::handleGet(CivetServer *server, struct mg_connection *conn) {

	std::string encodedRedirectURI;
	std::string clientID;
	std::string scope;
	std::string responseType;
	std::string loginError;

	bool err = CivetServer::getParam(conn, "e", loginError);

	if (CivetServer::getParam(conn, "redirect_uri", encodedRedirectURI)
			&& CivetServer::getParam(conn, "client_id", clientID)
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
			writeResponse(server, conn, response, "text/html");
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

	std::map<std::string, std::string> parms;
	if (parseForm(server, conn, parms)) {
		if (parms.find("redirect_uri") == parms.end()
				|| parms.find("client_id") == parms.end()
				|| parms.find("scope") == parms.end()
				|| parms.find("response_type") == parms.end()
				|| parms.find("hash") == parms.end()
				|| parms.find("username") == parms.end()) {
			writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
		}
		else {

			std::string encodedRedirectURI = parms["redirect_uri"];
			std::string redirectURI = parms["redirect_uri"];
			std::string clientID = parms["client_id"];
			std::string scope = parms["scope"];
			std::string responseType = parms["response_type"];
			std::string hash = parms["hash"];
			std::string username = parms["username"];

			OAuth2Client *client = findClient(clientID);
			CivetServer::urlDecode(encodedRedirectURI.c_str(), encodedRedirectURI.size(), redirectURI, false);

			mg_printf(conn, "HTTP/1.1 301 Moved Permanently\r\n");
			if (client == NULL
					|| !Util::HasBeginning(redirectURI, client->RedirectURL)) {
				g_Log.AddMessageFormat(
						"[WARNING] Attempt to use login api from invalid client (%s for %s against %s)",
						clientID.c_str(), redirectURI.c_str(),
						client == NULL ? "None" : client->RedirectURL.c_str());
				mg_printf(conn,
						"Location: authorize?e=2&client_id=%s&redirect_uri=%s&scope=%s&response_type=%s\r\n\r\n",
						clientID.c_str(), redirectURI.c_str(), scope.c_str(), responseType.c_str());
			} else {
				std::string hashed;
				AccountData::GenerateSaltedHash(hash.c_str(), hashed);

				g_AccountManager.cs.Enter(
						"SimulatorThread::handle_web_authenticate");
				AccountData *accPtr = g_AccountManager.GetValidLogin(username.c_str(),
						hashed.c_str());
				g_AccountManager.cs.Leave();
				if (accPtr == NULL) {
					mg_printf(conn,
							"Location: authorize?e=1&client_id=%s&redirect_uri=%s&scope=%s&response_type=%s\r\n\r\n",
							clientID.c_str(), redirectURI.c_str(), scope.c_str(), responseType.c_str());
				} else {
					std::string authCode = g_AccountManager.GenerateToken(accPtr->ID, 30000, AccessToken::AUTHENTICATION_CODE, 1);
					std::string decodedURI = redirectURI;
					Util::URLDecode(decodedURI);
					mg_printf(conn, "Location: %s&code=%s\r\n\r\n", decodedURI.c_str(),
							authCode.c_str());
				}
			}
		}
	} else {
		writeStatus(server, conn, 403, "Forbidden", "Encoding not allowed.");
	}

	return true;
}

//
// TokenHandler
//

bool TokenHandler::handlePost(CivetServer *server, struct mg_connection *conn) {

	std::map<std::string, std::string> parms;
	MultiPart mp;
	if (parseMultiPart(server, conn, &mp)) {


		std::string encodedRedirectURI = mp.getPartWithName("redirect_uri").content;
		std::string clientID = mp.getPartWithName("client_id").content;
		std::string grantType = mp.getPartWithName("grant_type").content;
		std::string code = mp.getPartWithName("code").content;
		std::string clientSecret = mp.getPartWithName("client_secret").content;

		if (encodedRedirectURI.size() == 0 || clientID.size() == 0 || grantType.size() == 0 || code.size() == 0) {
			writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
		}
		else {

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
				if(client->ClientSecret.size() > 0 && clientSecret.compare(client->ClientSecret) != 0) {
					g_Log.AddMessageFormat(
										"[WARNING] Invalid client secret used  (%s for %s against %s)",
										clientID.c_str(), redirectURI.c_str(),
										client == NULL ? "None" : client->RedirectURL.c_str());
					writeJSON200(server, conn, "{ \"error\":\"invalid_request\"}");
				}
				else {
					AccessToken *tkn = g_AccountManager.GetToken(code);
					if (tkn == NULL) {
						g_Log.AddMessageFormat(
								"[WARNING] Attempt to get non-existing auth code '%s'.", code.c_str());
						writeJSON200(server, conn, "{ \"error\":\"invalid_request\"}");
					} else {
						writeJSON200(server, conn, "{\"access_token\":\"" + g_AccountManager.GenerateToken(tkn->accountID, 60000, AccessToken::ACCESS_TOKEN, -1) + "\"}");
					}
				}
			}
		}
	}
	else {
		writeStatus(server, conn, 403, "Forbidden", "Invalid encoding.");
	}

	return true;
}

//
// SelfHandler
//

bool SelfHandler::handleGet(CivetServer *server, struct mg_connection *conn) {

	// Parse parameters
	std::string p;
	if(CivetServer::getParam(conn, "access_token", p)) {
		AccessToken *tkn = g_AccountManager.GetToken(p);
		if(tkn == NULL) {
			writeStatus(server, conn, 403, "Forbidden", "Invalid access token.");
		}
		else {
			AccountData *ad = g_AccountManager.FetchIndividualAccount(tkn->accountID);
			if(ad == NULL) {
				writeStatus(server, conn, 403, "Forbidden", "Invalid account.");
			}
			else {
				char buf[256];
				std::string name = ad->Name;
				Util::SafeFormat(buf, sizeof(buf),
						"{ \"data\": { \"id\": \"%d\", \"username\": \"%s\" } }", ad->ID,
						Util::EncodeJSONString(name).c_str());
				writeJSON200(server, conn, buf);
			}
		}
	}
	else {
		writeStatus(server, conn, 403, "Forbidden", "No access token.");
	}
	return true;
}


