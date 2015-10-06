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
#include "HTTP.h"

namespace HTTPD {

class FileChecksum
{
public:
	typedef std::map<std::string, std::string> CHECKSUM_MAP;
	CHECKSUM_MAP mChecksumData;
	void LoadFromFile(const char *filename);
	bool MatchChecksum(const std::string &filename, const std::string &checksum);
};

class HTTPService
{
public:
	HTTPService();
	~HTTPService();
	bool Start();
	bool Shutdown();
private:
	CivetServer *civetServer;
};

}

extern HTTPD::FileChecksum g_FileChecksum;
extern HTTPD::HTTPService g_HTTPService;

#endif

