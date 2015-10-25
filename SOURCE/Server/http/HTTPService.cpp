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

#include "HTTPService.h"

#include "CivetServer.h"
#include "CAR.h"
#include "TAWApi.h"
#include "OAuth2.h"
#include "LegacyAccounts.h"
#include "WebControlPanel.h"

#include "../Config.h"
#include "../Util.h"
#include "../StringList.h"
#include "../FileReader.h"
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <string>
#include <vector>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace HTTPD;

//
// FileChecksum
//

FileChecksum g_FileChecksum;

void FileChecksum :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[WARNING] Could not open file: %s", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	std::string readfilename;
	std::string readchecksum;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r >= 2)
		{
			readfilename = lfr.BlockToStringC(0, 0);
			readchecksum = lfr.BlockToStringC(1, 0);
			mChecksumData[readfilename] = readchecksum;
			//g_Log.AddMessageFormat("SET: %s=%s (%s)", readfilename.c_str(), mChecksumData[readfilename].c_str(), readchecksum.c_str());
		}
	}
	lfr.CloseCurrent();
}

std::string FileChecksum :: MatchChecksum(const std::string &filename, const std::string &checksum)
{
	CHECKSUM_MAP::iterator it = mChecksumData.find(filename);

	//If it doesn't appear in the list, assume it's valid so the client doesn't redownload
	if(it == mChecksumData.end()) {
		g_Log.AddMessageFormat("[WARNING] File %s is not in the index, so it's checksum is unknown. Assuming no download required.", filename.c_str());
		return "";
	}

	if(it->second.compare(checksum) == 0) {
		return "";
	}

	return it->second;
}


//
// HTTPService
//

HTTPService g_HTTPService;

HTTPService::HTTPService() {
	civetServer = NULL;
}

HTTPService::~HTTPService() {
}

bool HTTPService::Shutdown() {
	if(civetServer != NULL) {
		delete civetServer;
		return true;
	}
	return false;
}

bool HTTPService::Start() {

	const char * zzOptions[32];
	unsigned int idx = 0;

	std::vector<const char *> opts;

	// Document root
	zzOptions[idx++] = "document_root";
	zzOptions[idx++] = g_HTTPBaseFolder;

	bool http = g_HTTPListenPort > 0;
#ifndef NO_SSL
	if(g_HTTPSListenPort > 0) {
		http = true;
	}
#endif

	// HTTP
	if(http) {

		zzOptions[idx++] = "listening_ports";
		char portbuf[6];

		std::string *ports = new std::string();
		if(g_HTTPListenPort > 0) {
			g_Log.AddMessageFormat("CivetWeb HTTP configured on port %d", g_HTTPListenPort);
			if(strlen(g_BindAddress) > 0) {
				ports->append(g_BindAddress);
				ports->append(":");
			}
			Util::SafeFormat(portbuf, sizeof(portbuf), "%d", g_HTTPListenPort);
			ports->append(portbuf);
		}
#ifndef NO_SSL
		if(g_HTTPSListenPort > 0) {
			if(g_SSLCertificate.size() < 1) {
				g_Log.AddMessageFormat("[WARNING] SSL port has been set (%d), but no SSLCertificate has been set. SSL server cannot be started.", g_HTTPSListenPort);
			}
			else {
				if(ports->size()> 0) {
					ports->append(",");
				}
				g_Log.AddMessageFormat("CivetWeb HTTPS configured on port %d", g_HTTPSListenPort);
				if(strlen(g_BindAddress) > 0) {
					ports->append(g_BindAddress);
					ports->append(":");
				}
				Util::SafeFormat(portbuf, sizeof(portbuf), "%ds", g_HTTPSListenPort);
				ports->append(portbuf);
			}
		}
#endif

		zzOptions[idx++] = ports->c_str();

		// Logs
		zzOptions[idx++] = "access_log_file";
		zzOptions[idx++] = "AccessLog.txt";
		zzOptions[idx++] = "error_log_file";
		zzOptions[idx++] = "ErrorLog.txt";
		if(g_Config.HTTPKeepAlive) {
			zzOptions[idx++] = "enable_keep_alive";
			zzOptions[idx++] = "yes";
		}
		zzOptions[idx++] = "enable_directory_listing";
		zzOptions[idx++] = g_Config.DirectoryListing ? "yes" : "no";
#ifndef NO_SSL
		zzOptions[idx++] = "ssl_certificate";
		zzOptions[idx++] = g_SSLCertificate.c_str();
#endif
		zzOptions[idx] = 0;

		g_Log.AddMessageFormat("Starting CivetWeb");
		civetServer = new CivetServer(zzOptions);
		if(!civetServer->isConfigured()) {
			g_Log.AddMessageFormat("[WARNING] CivetWeb HTTP server disabled due to misconfiguration.");
			return false;
		}

		// CAR files
		civetServer->addHandler("**.car$", new CARHandler());

		// API
		if(g_Config.PublicAPI) {
			civetServer->addHandler("/api/who", new WhoHandler());
			civetServer->addHandler("/api/chat", new ChatHandler());
			civetServer->addHandler("/api/user/*", new UserHandler());
			civetServer->addHandler("/api/character/*", new CharacterHandler());
			civetServer->addHandler("/api/user/*/groves", new UserGrovesHandler());
			civetServer->addHandler("/api/zone/*", new ZoneHandler());
			civetServer->addHandler("/api/scenery/*", new SceneryHandler());
			civetServer->addHandler("/api/leaderboard", new LeaderboardHandler());
		}

		// OAuth - Used to authenticate external services
		if(g_Config.OAuth2Clients.size() > 0) {
			civetServer->addHandler("/oauth/authorize", new AuthorizeHandler());
			civetServer->addHandler("/oauth/login", new LoginHandler());
			civetServer->addHandler("/oauth/token", new TokenHandler());
			civetServer->addHandler("/oauth/self", new SelfHandler());
		}

		// Legacy account maintenannce
		if(g_Config.LegacyAccounts) {
			civetServer->addHandler("/newaccount", new NewAccountHandler());
			civetServer->addHandler("/resetpassword", new ResetPasswordHandler());
			civetServer->addHandler("/accountrecover", new AccountRecoverHandler());
		}

		// Legacy web control panel
		civetServer->addHandler("/remoteaction", new RemoteActionHandler());

		return true;
	}
	else {
		g_Log.AddMessageFormat("[WARNING] CivetWeb HTTP server disabled. No HTTP requests will be served.");

		return false;
	}
}
