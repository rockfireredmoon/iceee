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
#include "../Util.h"

#include <ctime>
#include <string>
#include <map>
#include <vector>

#define MG_BUF_LEN 16384
#define MAX_PARAMETER_SIZE 16384
#define MAX_MULTIPART_HEADERS 10
#define MAX_PARAMETERS 1024

namespace HTTPD {

inline static void extractPair(std::string pair, std::string &key,
		std::string &value) {
	size_t eidx = pair.find("=");
	key = eidx == std::string::npos ? pair : pair.substr(0, eidx);
	value = eidx == std::string::npos ? "" : pair.substr(eidx + 1);
}


/* Protect against directory disclosure attack by removing '..',
 excessive '/' and '\' characters */
inline static std::string removeDoubleDotsAndDoubleSlashes(std::string str) {
	std::string &c = str;
	Util::ReplaceAll(c, "..", "");
	Util::ReplaceAll(c, "\\\\", "");
	Util::ReplaceAll(c, "//", "");
	return c;
}

inline static std::string removeEndSlash(std::string str) {
	while(Util::HasEnding(str, "/"))
		str = str.substr(0, str.length() - 1);
	return str;
}

inline static std::string removeStartSlash(std::string str) {
	while(Util::HasBeginning(str, "/"))
		str = str.substr(1);
	return str;
}

/*
 * Tracks the session obtained at authentication time
 */

class SiteSession {
public:
	std::string xCSRF;
	std::string sessionName;
	std::string sessionID;
	int uid;
	int unreadMessages;
	void CopyFrom(SiteSession *session);
	void Clear();
};

class FileResource {
public:
	FileResource(const std::string &path);
	~FileResource();
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
	bool requiresAuthentication;
};


class AbstractCivetHandler: public CivetHandler {
public:
	std::string formatTime(std::time_t *now);

	bool isUserAgent(CivetServer *server, struct mg_connection *conn);

	bool isAuthorized(CivetServer *server, struct mg_connection *conn, std::string credentials);

	bool parseMultiPart(CivetServer *server, struct mg_connection *conn, MultiPart *multipart);

	bool parseForm(CivetServer *server, struct mg_connection *conn,
			std::map<std::string, std::string> &parms);

	void send_file_data(struct mg_connection *conn, FileResource *filep);

	void writeWWWAuthenticate(CivetServer *server, struct mg_connection *conn, std::string realm);

	void writeJSON200(CivetServer *server, struct mg_connection *conn,
			std::string data);

	void writeStatusPlain(CivetServer *server, struct mg_connection *conn, int code,
			std::string msg, std::string data);

	void writeStatus(CivetServer *server, struct mg_connection *conn, int code,
			std::string msg, std::string data);

	void writeResponse(CivetServer *server, struct mg_connection *conn, std::string data, std::string contentType);

	int openFile(const struct mg_request_info * req_info, FileResource *file);

	void sendStatusFile(struct mg_connection *conn, const struct mg_request_info * req_info, int code, const std::string &codeText, const std::string &defaultMessage);
};
class PageOptions {
public:
	unsigned int count;
	unsigned int top;
	unsigned int start;
	std::string sort;
	bool desc;

	PageOptions();
	void Init(CivetServer *server, struct mg_connection *conn);
};


}


#endif

