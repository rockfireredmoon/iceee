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

#ifndef HTTPSERVICE_H
#define HTTPSERVICE_H

#include <stdio.h>
#include <ctime>
#include <string>
#include <map>
#include "HTTP.h"
#include "../Components.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

namespace HTTPD {

class FileChecksum
{
public:
	FileChecksum();
	typedef map<string, string> CHECKSUM_MAP;
	CHECKSUM_MAP mChecksumData;
	void LoadFromFile();
	string MatchChecksum(const fs::path &filename, const string checksum);
private:
	fs::path GetFilename();
	void ScheduleCheck();
	int mChecksumUpdateTimer;
	fs::file_time_type mChecksumUpdate;
	int mChecked;
	//Platform_CriticalSection cs;
};

class HTTPService
{
public:
	HTTPService();
	~HTTPService();
	bool Start();
	bool Shutdown();
	void RegisterHandler(string name, CivetHandler *handler);
private:
	CivetServer *civetServer;
	map<string, CivetHandler*> handlers;
};

}

extern HTTPD::FileChecksum g_FileChecksum;
extern HTTPD::HTTPService g_HTTPService;

#endif

