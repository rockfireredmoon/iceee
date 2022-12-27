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
#include "../util/Log.h"
#include "../Scheduler.h"

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

FileChecksum::FileChecksum() {
	mChecked = 0;
	mChecksumUpdateTimer = 0;
}

void FileChecksum::LoadFromFile() {
//	cs.Enter("FileChecksum::LoadFromFile");
	mChecksumData.clear();
	auto filename = GetFilename();
	if (!fs::exists(filename)) {
		g_Logs.http->warn(
				"Could not find newer style checksum %v, trying old location",
				filename);
		filename = g_Config.ResolveStaticDataPath() / "Data" / "HTTPChecksum.txt";
	}

	g_Logs.http->info("Loading checksums from %v", filename);
	FileReader lfr;
	if (lfr.OpenText(filename) != Err_OK) {
		g_Logs.http->warn("Could not open file: %v", filename);
//		cs.Leave();
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	std::string readfilename;
	std::string readchecksum;
	while (lfr.FileOpen() == true) {
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if (r >= 2) {
			readfilename = lfr.BlockToStringC(0, 0);
			readchecksum = lfr.BlockToStringC(1, 0);
			mChecksumData[readfilename] = readchecksum;
			//g_Log.AddMessageFormat("SET: %s=%s (%s)", readfilename.c_str(), mChecksumData[readfilename].c_str(), readchecksum.c_str());
		}
	}
	lfr.CloseCurrent();
	mChecksumUpdate = fs::last_write_time(filename);
	ScheduleCheck();
//	cs.Leave();
}

fs::path FileChecksum::GetFilename() {
	return g_Config.ResolveHTTPCARPath() / "Release" / "Current"/ "HTTPChecksum.txt";
}

void FileChecksum::ScheduleCheck() {
	if(mChecksumUpdateTimer != 0) {
		mChecksumUpdateTimer = 0;
		g_Scheduler.Cancel(mChecksumUpdateTimer);
	}
	mChecksumUpdateTimer = g_Scheduler.ScheduleIn([this]() {
		std::string filename = GetFilename();
		if (fs::last_write_time(filename) != mChecksumUpdate) {
			// Different again, reset counter to 1 so we check checking
			mChecked = 1;
		}
		else if(mChecked > 3) {
			// Stayed same after a change for 3 cycles, has changed
			g_Logs.http->info("Checksum has changed: %v", filename);
			LoadFromFile();

			// Start again
			mChecked = 0;
		}
		else if(mChecked > 0) {
			// Had one change, but need more
			mChecked++;
		}
		mChecksumUpdate = fs::last_write_time(filename);
		ScheduleCheck();
	}, 10000);
}

std::string FileChecksum::MatchChecksum(const fs::path &filename,
		const std::string checksum) {
	CHECKSUM_MAP::iterator it = mChecksumData.find(filename.string());

	//If it doesn't appear in the list, assume it's valid so the client doesn't redownload
	if (it == mChecksumData.end()) {
		g_Logs.http->warn(
				"File %v is not in the index, so it's checksum is unknown. Assuming no download required.",
				filename);
		return "";
	}

	if (it->second.compare(checksum) == 0) {
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
	if (civetServer != NULL) {
		delete civetServer;
		return true;
	}
	return false;
}

void HTTPService::RegisterHandler(std::string name, CivetHandler *handler) {
	handlers[name] = handler;
}

bool HTTPService::Start() {

	std::vector<std::string> zzOptions;

	// Document root
	zzOptions.push_back("document_root");
	zzOptions.push_back((g_Config.ResolveHTTPBasePath()/ "Pages").string());

	bool http = g_HTTPListenPort > 0;
#ifndef NO_SSL
	if(g_HTTPSListenPort > 0) {
		http = true;
	}
#endif

	// HTTP
	if (http) {

		zzOptions.push_back("listening_ports");

		std::string ports;
		if (g_HTTPListenPort > 0) {
			if (strlen(g_BindAddress) > 0) {
				ports.append(g_BindAddress);
				ports.append(":");
			}
			ports.append(Util::Format("%d", g_HTTPListenPort));
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
				ports.append(Util::Format("%d", g_HTTPSListenPort));
				g_Logs.http->info("CivetWeb HTTPS configured on port %v", ports);
			}
		}
#endif

		zzOptions.push_back(ports);

		std::string alog = (g_Config.ResolveLogPath()/ 	"HTTPAccess.txt").string();
		std::string elog = (g_Config.ResolveLogPath()/ "HTTPError.txt").string();

		// Logs
		zzOptions.push_back("access_log_file");
		zzOptions.push_back(alog);
		zzOptions.push_back("error_log_file");
		zzOptions.push_back(elog);

		// Server
		zzOptions.push_back("num_threads");
		zzOptions.push_back(std::to_string(g_Config.HTTPThreads));

		zzOptions.push_back("listen_backlog");
		zzOptions.push_back(std::to_string(g_Config.HTTPBacklog));

		zzOptions.push_back("connection_queue");
		zzOptions.push_back(std::to_string(g_Config.HTTPConnectionQueue));

		zzOptions.push_back("enable_auth_domain_check");
		zzOptions.push_back(g_Config.HTTPAuthDomainCheck ? "yes" : "no");

		zzOptions.push_back("authentication_domain");
		if(g_Config.HTTPAuthDomain.length() == 0)
			zzOptions.push_back(g_Config.ResolveSimulatorAddress());
		else
			zzOptions.push_back(g_Config.HTTPAuthDomain);

		// Other configuration
		zzOptions.push_back("enable_keep_alive");
		zzOptions.push_back(g_Config.HTTPKeepAlive ? "yes" : "no");
		if(g_Config.HTTPKeepAlive) {
			g_Logs.http->warn("HTTP Keep Alive is enabled, but the client is known not to work correctly with this option.");
		}

		zzOptions.push_back("enable_directory_listing");
		zzOptions.push_back(g_Config.DirectoryListing ? "yes" : "no");

		// SSL
#ifndef NO_SSL
		zzOptions.push_back("ssl_certificate");
		zzOptions.push_back(g_SSLCertificate);
#endif

		g_Logs.http->info("Starting CivetWeb");
		BEGINTRY {
			civetServer = new CivetServer(zzOptions);
//			if (!civetServer->isConfigured()) {
//				g_Logs.http->warn(
//						"CivetWeb HTTP server disabled due to misconfiguration.");
//				return false;
//			}

			for (std::map<std::string, CivetHandler*>::iterator it =
					handlers.begin(); it != handlers.end(); ++it) {
				civetServer->addHandler((*it).first, (*it).second);
			}

			return true;
		}
		BEGINCATCH {
			g_Logs.http->error(
					"CivetWeb HTTP server threw exception on startup. This is likely due to configuration or other services running on the same interface and / or port.");
			return false;
		}
	} else {
		g_Logs.http->warn(
				"CivetWeb HTTP server disabled. No HTTP requests will be served.");

		return false;
	}
}
