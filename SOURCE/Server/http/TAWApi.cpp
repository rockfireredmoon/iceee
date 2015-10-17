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

#include "TAWApi.h"
#include "../Util.h"
#include "../Account.h"
#include "../Simulator.h"
#include "../Character.h"
#include "../ZoneDef.h"
#include "../Scenery2.h"
#include "../Chat.h"
#include "../DirectoryAccess.h"
#include "../json/json.h"

using namespace HTTPD;

//
// WhoHandler
//

bool WhoHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
	std::string response;
	char buf[256];
	response.append("{ ");
	SIMULATOR_IT it;
	int no = 0;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1
				&& it->LoadStage == SimulatorThread::LOADSTAGE_GAMEPLAY) {
			if (it->IsGMInvisible() == true) //Hide GM Invisibility from the list.
				continue;
			CharacterData *cd = it->pld.charPtr;
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(
					it->pld.CurrentZoneID);
			if (cd != NULL && zd != NULL) {
				Util::SafeFormat(buf, sizeof(buf),
						"%s\"%s\" : { \"zone\": \"%s\", \"shard\": \"%s\" }",
						no > 0 ? "," : "", cd->cdef.css.display_name,
						zd->mName.c_str(), zd->mShardName.c_str());
				no++;
				response.append(buf);
			}
		}
	}
	response.append(" }");
	writeJSON200(server, conn, response);

	return true;
}

//
// ChatHandler
//

bool ChatHandler::handleGet(CivetServer *server, struct mg_connection *conn) {

	std::string response;
	char buf[256];
	int no = 0;

	// Parse parameters
	std::string p;
	int count = 20;
	if (CivetServer::getParam(conn, "count", p)) {
		count = atoi(p.c_str());
	}

	response.append("{ ");
	std::deque<ChatMessage>::iterator it;
	g_ChatManager.cs.Enter("HTTPDistribute::Chat");
	int start = g_ChatManager.CircularChatBuffer.size() - 1 - count;
	if (start < 0)
		start = 0;
	for (unsigned int i = start; i < g_ChatManager.CircularChatBuffer.size();
			i++) {
		ChatMessage cm = g_ChatManager.CircularChatBuffer[i];
		std::string msg = cm.mMessage;
		struct tm * timeinfo;
		time_t tt = cm.mTime;
		timeinfo = localtime(&tt);
		char tbuf[64];
		strftime(tbuf, sizeof(tbuf), "%m/%d %H:%M", timeinfo);
		Util::SafeFormat(buf, sizeof(buf),
				"%s\"%lu\" : { \"message\": \"%s\", \"time\": \"%s\" }",
				no > 0 ? "," : "", cm.mTime,
				Util::EncodeJSONString(msg).c_str(), tbuf);
		response += buf;
		no++;
	}
	g_ChatManager.cs.Leave();
	response.append(" }");
	writeJSON200(server, conn, response);

	return true;
}

//
// UserHandler
//
bool UserHandler::handleGet(CivetServer *server,
		struct mg_connection *conn) {


	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatus(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {
		int accountId = atoi(pathParts[pathParts.size() - 1].c_str());
		if(accountId == 0) {
			AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(pathParts[pathParts.size() - 1].c_str());
			if(aqd != NULL) {
				accountId = aqd->mID;
			}
		}

		AccountData *ad = NULL;
		if(accountId != 0) {
			ad = g_AccountManager.FetchIndividualAccount(accountId);
		}

		if(ad == NULL)
			writeStatus(server, conn, 404, "Not found.",
					"The account could not be found.");
		else {
			Json::Value root;
			ad->WriteToJSON(root);
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// UserGrovesHandler
//

bool UserGrovesHandler::handleGet(CivetServer *server,
		struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatus(server, conn, 404, "Not found.",
				"The zone could not be found.");
	else {
		int accountId = atoi(pathParts[pathParts.size() - 2].c_str());
		if(accountId == 0) {
			AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(pathParts[pathParts.size() - 2].c_str());
			if(aqd != NULL) {
				accountId = aqd->mID;
			}
		}

		AccountData * ad = g_AccountManager.FetchIndividualAccount(accountId);
		if (ad == NULL) {
			writeStatus(server, conn, 404, "Not found.",
					"The grove could not be found.");
		} else {
			std::vector<int> groveList;
			g_ZoneDefManager.EnumerateGroveIds(accountId, 0, groveList);
			Json::Value root;
			for (std::vector<int>::iterator it = groveList.begin();
					it != groveList.end(); ++it) {
				int id = *it;
				ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(id);
				if (zone == NULL)
					g_Log.AddMessageFormat("Unknown grove %d", id);
				else {
					Json::Value grove;
					zone->WriteToJSON(grove);
					root[zone->mWarpName] = grove;
				}
			}
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// GroveHandler
//

bool ZoneHandler::handleGet(CivetServer *server, struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters
	if (pathParts.size() < 1)
		writeStatus(server, conn, 404, "Not found.",
				"The zone could not be found.");
	else {
		int zoneDefId = atoi(pathParts[pathParts.size() - 1].c_str());
		ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(zoneDefId);
		if (zone == NULL)
			writeStatus(server, conn, 404, "Not found.",
					"The zone could not be found.");
		else {
			Json::Value root;
			zone->WriteToJSON(root);

			char buf[128];
			if (zone->mGrove)
				Util::SafeFormat(buf, sizeof(buf), "Grove/%d", zone->mID);
			else
				Util::SafeFormat(buf, sizeof(buf), "Scenery/%d", zone->mID);
			Platform::FixPaths(buf);

			Platform_DirectoryReader dr;
			dr.SetDirectory(buf);
			dr.ReadFiles();

			Json::Value tiles;
			int xx = 0;
			int yy = 0;
			for (std::vector<std::string>::iterator it = dr.fileList.begin();
					it != dr.fileList.end(); ++it) {
				if (sscanf((*it).c_str(), "x%03dy%03d.txt", &xx, &yy) == 2) {
					Json::Value tile;
					tile["x"] = xx;
					tile["y"] = yy;
					tiles.append(tile);
				}
			}
			root["tiles"] = tiles;

			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// SceneryHandler
//

bool SceneryHandler::handleGet(CivetServer *server,
		struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);
	if (pathParts.size() < 3)
		writeStatus(server, conn, 404, "Not found.",
				"The scenery could not be found.");
	else {
		// Parse parameters
		int zoneDefId = atoi(pathParts[pathParts.size() - 3].c_str());
		int x = atoi(pathParts[pathParts.size() - 2].c_str());
		int y = atoi(pathParts[pathParts.size() - 1].c_str());

		SceneryPage *page = g_SceneryManager.GetOrCreatePage(zoneDefId, x, y);
		if(page == NULL) {
			writeStatus(server, conn, 404, "Not found.",
					"The scenery could not be found.");
		}
		else {
			Json::Value root;
			Json::Value props;
			SceneryPage::SCENERY_IT it;
			char id[10];
			for(it = page->mSceneryList.begin(); it != page->mSceneryList.end(); ++it) {
				SceneryObject *so = &it->second;
				Json::Value object;
				Util::SafeFormat(id, sizeof(id), "%d", so->ID);
				so->WriteToJSON(object);
				props[id] = object;
			}
			root["objects"] = props;
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

