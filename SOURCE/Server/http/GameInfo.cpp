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

#include "CivetServer.h"
#include "GameInfo.h"

#include "HTTP.h"
#include "HTTPService.h"

#include "../Info.h"
#include "../Cluster.h"
#include "../util/Log.h"
#include "../StringUtil.h"
#include "../Config.h"

using namespace HTTPD;
using namespace std;

bool GameInfoHandler::handleGet(CivetServer *server,
		struct mg_connection *conn) {
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	/* Simple protection against drive-by penetration attempts uses user-agent */
	if (!isUserAgent(server, conn))
		return false;

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);

	int status = 404;
	string content;
	//string contentType = "application/octet-stream";
	string contentType = "text/plain";

	if (Util::HasEnding(ruri, "tips")) {
		status = 200;
		vector<Tip> tips = g_InfoManager.GetTips();
		for (auto it = tips.begin(); it != tips.end(); ++it) {
			content.append((*it).mText);
			content.append("<br/>\n");
		}
	} else if (Util::HasEnding(ruri, "in_game_news")) {
		status = 200;
		content = g_InfoManager.GetInGameNews();
		Util::ReplaceAll(content, "\n", "<br/>");
	} else if (Util::HasEnding(ruri, "loading_announcements")) {
		status = 200;
		vector<string> tips = g_InfoManager.GetLoadingAnnouncments();
		for (auto it = tips.begin(); it != tips.end(); ++it) {
			if(content.length() > 0)
				content.append("<hr/>");
			string c = *it;
			Util::ReplaceAll(c, "\n", "<br/>");
			content.append(c);
		}
	} else if (Util::HasEnding(ruri, "shards.json")) {
		status = 200;
		string contentType = "text/json";
		Json::Value root;
		for (std::map<std::string, Shard>::iterator it = g_ClusterManager.mActiveShards.begin(); it != g_ClusterManager.mActiveShards.end(); ++it) {
			Shard s = (*it).second;
			Json::Value c;
			s.WriteToJSON(c);
			root.append(c);
		}
		Json::StyledWriter writer;
		content = writer.write(root);
	}

	mg_set_status(conn, status);

	switch (status) {
	case 200: {
		g_Logs.http->info("Sending %v (%v bytes)", ruri.c_str(),
				content.length());
		mg_printf(conn, "HTTP/1.1 200 OK\r\n");
		mg_printf(conn, "Content-Type: %s\r\n", contentType.c_str());
		mg_printf(conn, "Content-Length: %lu\r\n", content.length());
		mg_printf(conn, "\r\n");
		mg_printf(conn, "%s", content.c_str());
		mg_increase_sent_bytes(conn, content.length());
		break;
	}
	default:
		g_Logs.http->info("Could not find %v", ruri.c_str());
		sendStatusFile(conn, req_info, status, "Not Found", "File not found.");
		mg_set_as_close(conn);
		break;
	}
	return true;
}
