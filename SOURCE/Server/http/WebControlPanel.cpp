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

#include "WebControlPanel.h"
#include "../Config.h"
#include "../Report.h"
#include "../RemoteAction.h"

#include "../Account.h"
#include "../EliteMob.h"
#include "../Ability2.h"
#include "../VirtualItem.h"
#include "../DirectoryAccess.h"

#include "HTTPService.h"
#include "../util/Log.h"
#include <map>

using namespace HTTPD;

//
// RemoteActionHandler
//

bool RemoteActionHandler::handlePost(CivetServer *server,
		struct mg_connection *conn) {
	std::map<std::string, std::string> parms;

	/* Simple protection against drive-by penetration attempts uses user-agent */
	if(!isUserAgent(server, conn))
		return false;

	if (parseForm(server, conn, parms)) {

		if (parms.find("action") == parms.end()
				|| parms.find("authtoken") == parms.end())
			writeStatus(server, conn, 200, "OK",
					"Invalid or malformed request.");
		else if (g_Config.RemotePasswordMatch(parms["authtoken"].c_str())
				== false)
			writeStatus(server, conn, 200, "OK", "Permission denied.");
		else {
			std::string action = parms["action"];

			// Actions
			if (action.compare("shutdown") == 0) {
				g_Logs.event->info(
						"[NOTICE] The server was remotely shut down.");
				g_ServerStatus = SERVER_STATUS_STOPPED;
			} else if (action.compare("reloadvi") == 0) {
				g_VirtualItemModSystem.LoadSettings();
			} else if (action.compare("reloadchecksum") == 0) {
				g_FileChecksum.LoadFromFile();
			} else if (action.compare("reloadconfig") == 0) {
				LoadConfig("ServerConfig.txt");
			} else if (action.compare("reloadability") == 0) {
				g_AbilityManager.LoadData();
			} else if (action.compare("reloadelite") == 0) {
				g_EliteManager.LoadData();
			} else if (action.compare("setmotd") == 0) {
				if (parms.find("data") == parms.end()
						|| parms["data"].size() == 0) {
					writeStatus(server, conn, 200, "OK", "Operation failed.");
					return true;
				}
				g_MOTD_Message = parms["data"];
			} else if (action.compare("importkeys") == 0) {
				g_AccountManager.ImportKeys();
			} else {
				// Reports

				ReportBuffer report(65535, ReportBuffer::NEWLINE_N);
				if (action.compare("refreshthreads") == 0) {
					Report::RefreshThreads(report);
				} else if (action.compare("refreshtime") == 0) {
					Report::RefreshTime(report);
				} else if (action.compare("refreshmods") == 0) {
					Report::RefreshMods(report, parms["sim_id"].c_str());
				} else if (action.compare("refreshplayers") == 0) {
					Report::RefreshPlayers(report);
				} else if (action.compare("refreshinstance") == 0) {
					Report::RefreshInstance(report);
				} else if (action.compare("refreshscripts") == 0) {
					Report::RefreshScripts(report);
				} else if (action.compare("refreshhateprofile") == 0) {
					Report::RefreshHateProfile(report);
				} else if (action.compare("refreshcharacter") == 0) {
					Report::RefreshCharacter(report);
				} else if (action.compare("refreshsim") == 0) {
					Report::RefreshSim(report);
				} else if (action.compare("refreshprofiler") == 0) {
					Report::RefreshProfiler(report);
				} else if (action.compare("refreshitem") == 0) {
					Report::RefreshItem(report, parms["sim_id"].c_str());
				} else if (action.compare("refreshitemdetailed") == 0) {
					Report::RefreshItemDetailed(report,
							parms["sim_id"].c_str());
				} else if (action.compare("refreshpacket") == 0) {
					Report::RefreshPacket(report);
				} else {
					writeStatus(server, conn, 200, "OK", "Unknown action.");
					return true;
				}

				// Write report
				mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n",
						report.getLength());
				mg_printf(conn, "Content-Type: text/html\r\n\r\n");
				mg_write(conn, report.getData(), report.getLength());

				return true;
			}

			// Operation OK
			writeStatus(server, conn, 200, "OK", "Operation successful.");

		}
	} else {
		writeStatus(server, conn, 200, "OK", "Failed to parse parameters.");
	}
	return true;
}

