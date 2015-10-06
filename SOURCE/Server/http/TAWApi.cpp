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
#include "../Simulator.h"
#include "../Character.h"
#include "../ZoneDef.h"
#include "../Chat.h"

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
	if(CivetServer::getParam(conn, "count", p)) {
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
		Util::EncodeJSONString(msg);
		Util::SafeFormat(buf, sizeof(buf),
				"%s\"%lu\" : { \"message\": \"%s\", \"time\": \"%s\" }",
				no > 0 ? "," : "", cm.mTime, msg.c_str(), tbuf);
		response += buf;
		no++;
	}
	g_ChatManager.cs.Leave();
	response.append(" }");
	writeJSON200(server, conn, response);

	return true;
}

