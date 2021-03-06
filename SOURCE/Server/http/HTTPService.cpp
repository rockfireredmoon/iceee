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

#include "../Config.h"
#include "../Util.h"
#include "../DirectoryAccess.h"
#include "../StringUtil.h"
#include "../util/Log.h"

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

void FileChecksum :: LoadFromFile()
{
	std::string filename = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveHTTPCARPath(), "Release"),
			"Current"), "HTTPChecksum.txt");
	if(!Platform::FileExists(filename)) {
		g_Logs.http->warn("Could not find newer style checksum %v, trying old location", filename);
		filename = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveStaticDataPath(), "Data"), "HTTPChecksum.txt");
	}

	g_Logs.http->info("Loading checksums from %v", filename);
	FileReader lfr;
	if(lfr.OpenText(filename.c_str()) != Err_OK)
	{
		g_Logs.http->warn("Could not open file: %v", filename);
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

std::string FileChecksum :: MatchChecksum(const std::string filename, const std::string checksum)
{
	CHECKSUM_MAP::iterator it = mChecksumData.find(filename);

	//If it doesn't appear in the list, assume it's valid so the client doesn't redownload
	if(it == mChecksumData.end()) {
		g_Logs.http->warn("File %v is not in the index, so it's checksum is unknown. Assuming no download required.", filename.c_str());
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

void HTTPService::RegisterHandler(std::string name, CivetHandler *handler) {
	handlers[name] = handler;
}

bool HTTPService::Start() {

	const char * zzOptions[32];
	unsigned int idx = 0;

	std::vector<const char *> opts;

	// Document root
	zzOptions[idx++] = "document_root";
	zzOptions[idx++] = Platform::JoinPath(g_Config.ResolveHTTPBasePath(), "Pages").c_str();

	bool http = g_HTTPListenPort > 0;
#ifndef NO_SSL
	if(g_HTTPSListenPort > 0) {
		http = true;
	}
#endif

	// HTTP
	if(http) {

		zzOptions[idx++] = "listening_ports";

		std::string ports;
		if(g_HTTPListenPort > 0) {
			if(strlen(g_BindAddress) > 0) {
				ports.append(g_BindAddress);
				ports.append(":");
			}
			ports.append(StringUtil::Format("%d", g_HTTPListenPort));
			g_Logs.http->info("CivetWeb HTTP configured on port %v", ports);
		}
#ifndef NO_SSL
		if(g_HTTPSListenPort > 0) {
			if(g_SSLCertificate.size() < 1) {
				g_Logs.http->warn("SSL port has been set (%v), but no SSLCertificate has been set. SSL server cannot be started.", g_HTTPSListenPort);
			}
			else {
				if(ports.size()> 0) {
					ports.append(",");
				}
				if(strlen(g_BindAddress) > 0) {
					ports.append(g_BindAddress);
					ports.append(":");
				}
				ports.append(StringUtil::Format("%d", g_HTTPSListenPort));
				g_Logs.http->info("CivetWeb HTTPS configured on port %v", ports);
			}
		}
#endif

		zzOptions[idx++] = ports.c_str();

		std::string alog = Platform::JoinPath(g_Config.ResolveLogPath(), "HTTPAccess.txt");
		std::string elog = Platform::JoinPath(g_Config.ResolveLogPath(), "HTTPError.txt");

		// Logs
		zzOptions[idx++] = "access_log_file";
		zzOptions[idx++] = alog.c_str();
		zzOptions[idx++] = "error_log_file";
		zzOptions[idx++] = elog.c_str();
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

		g_Logs.http->info("Starting CivetWeb");
		BEGINTRY {
			civetServer = new CivetServer(zzOptions);
			if(!civetServer->isConfigured()) {
				g_Logs.http->warn("CivetWeb HTTP server disabled due to misconfiguration.");
				return false;
			}

			for(std::map<std::string, CivetHandler*>::iterator it = handlers.begin(); it != handlers.end(); ++it) {
				civetServer->addHandler((*it).first, (*it).second);
			}

			return true;
		}
		BEGINCATCH {
			g_Logs.http->error("CivetWeb HTTP server threw exception on startup. This is likely due to configuration or other services running on the same interface and / or port.");
			return false;
		}
	}
	else {
		g_Logs.http->warn("CivetWeb HTTP server disabled. No HTTP requests will be served.");

		return false;
	}
}
