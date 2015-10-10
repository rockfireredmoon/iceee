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

#ifndef HTTP_H
#define HTTP_H

#include "CivetServer.h"

#include <ctime>
#include <string>
#include <map>
#include <vector>

#define MG_BUF_LEN 16384
#define MAX_PARAMETER_SIZE 16384
#define MAX_MULTIPART_HEADERS 10
#define MAX_PARAMETERS 1024

namespace HTTPD {

class FileResource {
public:
	FILE *fd;
	std::string filePath;
	unsigned long fileSize;
	std::time_t lastModified;

};

class Part {
public:
	std::map<std::string, std::string> headers;
	std::string content;
	std::map<std::string, std::string> getHeaderValues(std::string header);
	bool write(const char *data, size_t offset, size_t len);

};

class MultiPart {
public:
	Part getPartWithName(std::string name);
	std::vector<Part> parts;
};

class AbstractCivetHandler: public CivetHandler {
public:
	std::string formatTime(std::time_t *now);

	bool parseMultiPart(CivetServer *server, struct mg_connection *conn, MultiPart *multipart);

	bool parseForm(CivetServer *server, struct mg_connection *conn,
			std::map<std::string, std::string> &parms);

	void writeJSON200(CivetServer *server, struct mg_connection *conn,
			std::string data);

	void writeStatus(CivetServer *server, struct mg_connection *conn, int code,
			std::string msg, std::string data);

	void writeResponse(CivetServer *server, struct mg_connection *conn, std::string data, std::string contentType);
};

}

#endif

