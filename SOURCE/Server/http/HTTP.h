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
#include <filesystem>

#include <chrono>

using namespace std;
namespace fs = filesystem;

#define MG_BUF_LEN 16384
#define MAX_PARAMETER_SIZE 16384
#define MAX_MULTIPART_HEADERS 10
#define MAX_PARAMETERS 1024

namespace HTTPD {

inline static void extractPair(string pair, string &key,
		string &value) {
	size_t eidx = pair.find("=");
	key = eidx == string::npos ? pair : pair.substr(0, eidx);
	value = eidx == string::npos ? "" : pair.substr(eidx + 1);
}


/* Protect against directory disclosure attack by removing '..',
 excessive '/' and '\' characters */
inline static string removeDoubleDotsAndDoubleSlashes(string str) {
	string &c = str;
	Util::ReplaceAll(c, "..", "");
	Util::ReplaceAll(c, "\\\\", "");
	Util::ReplaceAll(c, "//", "");
	return c;
}

inline static string removeEndSlash(string str) {
	while(Util::HasEnding(str, "/"))
		str = str.substr(0, str.length() - 1);
	return str;
}

inline static string removeStartSlash(string str) {
	while(Util::HasBeginning(str, "/"))
		str = str.substr(1);
	return str;
}

/*
 * Tracks the session obtained at authentication time
 */

class SiteSession {
public:
	string xCSRF;
	string sessionName;
	string sessionID;
	int uid;
	int unreadMessages;
	void CopyFrom(SiteSession *session);
	void Clear();
};

class FileResource {
public:
	FileResource(const fs::path &path);
	~FileResource();
	FILE *fd;
	fs::path filePath;
	unsigned long fileSize;
	fs::file_time_type lastModified;

};

class Part {
public:
	map<string, string> headers;
	string content;
	map<string, string> getHeaderValues(const string &header);
	bool write(const char *data, size_t offset, size_t len);

};

class MultiPart {
public:
	Part getPartWithName(const string &name);
	vector<Part> parts;
	bool requiresAuthentication;
};


class AbstractCivetHandler: public CivetHandler {
public:
	string formatTime(time_t now);

	bool isUserAgent(CivetServer *server, struct mg_connection *conn);

	bool isAuthorized(CivetServer *server, struct mg_connection *conn, const string &credentials);

	bool parseMultiPart(CivetServer *server, struct mg_connection *conn, MultiPart *multipart);

	bool parseForm(CivetServer *server, struct mg_connection *conn,
			map<string, string> &parms);

	void send_file_data(struct mg_connection *conn, FileResource *filep);

	void writeWWWAuthenticate(CivetServer *server, struct mg_connection *conn, const string &realm);

	void writeJSON200(CivetServer *server, struct mg_connection *conn,
			const string &data);

	void writeStatusPlain(CivetServer *server, struct mg_connection *conn, int code,
			const string &msg, const string &data);

	void writeStatus(CivetServer *server, struct mg_connection *conn, int code,
			const string &msg, const string &data);

	void writeResponse(CivetServer *server, struct mg_connection *conn, const string &data, const string &contentType);

	int openFile(const struct mg_request_info * req_info, FileResource *file);

	void sendStatusFile(struct mg_connection *conn, const struct mg_request_info * req_info, int code, const string &codeText, const string &defaultMessage);
};
class PageOptions {
public:
	unsigned int count;
	unsigned int top;
	unsigned int start;
	string sort;
	bool desc;

	PageOptions();
	void Init(CivetServer *server, struct mg_connection *conn);
};


}


#endif

