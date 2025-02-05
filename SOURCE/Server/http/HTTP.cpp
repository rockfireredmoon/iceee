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

#include "HTTP.h"

#include "../Util.h"
#include <stdlib.h>
#include <string.h>
#include "../util/base64.h"
#include "../util/Log.h"
#include "../Config.h"
#include "../DirectoryAccess.h"

using namespace HTTPD;\

//
// SiteSession
//
void SiteSession::CopyFrom(SiteSession *session) {
	xCSRF = session->xCSRF;
	sessionName = session->sessionName;
	sessionID = session->sessionID;
	uid = session->uid;
	unreadMessages = session->unreadMessages;
}

void SiteSession::Clear() {
	xCSRF.clear();
	sessionName.clear();
	sessionID.clear();
	uid = 0;
	unreadMessages = 0;
}

//
// FileResource
//
FileResource::FileResource(const fs::path &path) {
	fd = 0;
	filePath = path;
	fileSize = fs::file_size(path);
	lastModified = fs::last_write_time(path);
}
FileResource::~FileResource() { }

//
// MultiPart
Part MultiPart::getPartWithName(const string &name) {
	for (auto p : parts) {
		map<string, string> hv = p.getHeaderValues(
				"Content-Disposition");
		if (name.compare(hv["name"]) == 0) {
			return p;
		}
	}
	return Part();
}
//

//
// Part
//
bool Part::write(const char *data, size_t off, size_t len) {
	char c[len + 1];
	memcpy(c, data + off, len);
	c[len] = 0;
	content.append(c);

	if(content.length() > MAX_PARAMETER_SIZE) {
		g_Logs.http->warn("Exceeded multipart header limit. Rejecting entire post.");
		return false;
	}
	return true;
}

map<string, string> Part::getHeaderValues(const string &name) {
	string v = headers[name];
	vector<string> p;
	Util::Split(v, ";", p);
	map<string, string> m;
	size_t eidx;
	for (auto v : p) {
		eidx = v.find("=");
		if (eidx == string::npos) {
			Util::Trim(v);
			m[v] = "";
		} else {
			string s = v.substr(eidx + 1);
			if (Util::HasBeginning(s, "\"")) {
				s = s.substr(1);
			}
			if (Util::HasEnding(s, "\"")) {
				s = s.substr(0, s.length() - 1);
			}
			v = v.substr(0, eidx);
			Util::Trim(v);
			m[v] = s;
		}
	}
	return m;
}

//
// AbstractCivetHandler
//


/* Send bytes from the opened file to the client. */
void AbstractCivetHandler::send_file_data(struct mg_connection *conn, FileResource *filep) {
	char buf[MG_BUF_LEN];
	int to_read, num_read, num_written;
	long len = filep->fileSize;

	/* Sanity check the offset */

	if (len > 0 && filep->fd != NULL) {
		while (len > 0) {
			/* Calculate how much to read from the file in the buffer */
			to_read = sizeof(buf);
			if (to_read > len) {
				to_read = (int) len;
			}

			/* Read from file, exit the loop on error */
			if ((num_read = (int) fread(buf, 1, (size_t) to_read, filep->fd))
					<= 0) {
				break;
			}

			/* Send read bytes to the client, exit the loop on error */
			if ((num_written = mg_write(conn, buf, (size_t) num_read))
					!= num_read) {
				break;
			}

			/* Both read and were successful, adjust counters */
//            conn->num_bytes_sent += num_written;
			len -= num_written;
		}
	}
}

void AbstractCivetHandler::sendStatusFile(struct mg_connection *conn, const struct mg_request_info * req_info, int code, const string &codeText, const string &defaultMessage) {
	FileResource errfile(g_Config.ResolveHTTPBasePath() / "Errors" / Util::Format("%d.html", code));
	mg_printf(conn, "HTTP/1.1 %d %s\r\n", code, codeText.c_str());
	mg_printf(conn, "Content-Type: text/html\r\n");
	int errstatus = openFile(req_info, &errfile);
	if(errstatus == 200) {
		mg_printf(conn, "Content-Length: %d\r\n\r\n",
				(int) errfile.fileSize);
		send_file_data(conn, &errfile);
		fclose(errfile.fd);
	}
	else {
		mg_printf(conn, "Content-Length: %lu\r\n\r\n", defaultMessage.size());
		mg_printf(conn, "%s", defaultMessage.c_str());
	}
}

int AbstractCivetHandler::openFile(const struct mg_request_info * req_info, FileResource *file) {
	file->fd = fopen(file->filePath.c_str(), "rb");
	if (file->fd == NULL) {
		return 404;
	} else {
		if (file->fileSize > 0) {
			return 200;
		} else
			fclose(file->fd);
		return 0;
	}
}

string AbstractCivetHandler::formatTime(time_t now) {
	tm * ptm = localtime(&now);
	char nowBuffer[64];
	strftime(nowBuffer, 64, "%a, %d %b %Y %H:%M:%S GMT", ptm);
	return nowBuffer;
}

bool AbstractCivetHandler::parseMultiPart(CivetServer *server,
		struct mg_connection *conn, MultiPart *multiPart) {

	char postData[MG_BUF_LEN + 1];
	string contentType = CivetServer::getHeader(conn, "Content-Type");
	int contentLength = atoi(CivetServer::getHeader(conn, "Content-Length"));
	if (contentType.find("multipart/form-data") == 0) {
		size_t sep = contentType.find(";");
		if (sep == string::npos) {
			// No boundary
			return false;
		}
		string boundaryPair = contentType.substr(sep + 1);
		Util::Trim(boundaryPair);
		string k, v;
		extractPair(boundaryPair, k, v);
		string eob = "\r\n--" + v + "--";
		v = "\r\n--" + v + "\r\n";

		size_t dataLen, eidx, toRead = contentLength, matches = 2;
		vector<string> prms;
		const char * boundary = v.c_str();
		const char * boundary2 = eob.c_str();
		string nlstr = "\r\n\r\n";
		const char * nl = nlstr.c_str();
		unsigned int boundaryLen = strlen(boundary);

		vector<string> headerLines;
		string line;
		Part part;

		// States
		// 0 - Scanning for boundary
		// 1 - Have boundary, waiting for part header
		// 2 - Have part header, waiting for content and scanning for boundary

		unsigned int state = 0;

		do {
			dataLen = mg_read(conn, postData, sizeof(postData) - 1);
			if (dataLen > 0) {
				for (unsigned i = 0; i < dataLen; i++) {
					if (state == 0 || state == 2) {
						// Scanning for boundary
						if (postData[i] == boundary[matches] || postData[i] == boundary2[matches]) {
							matches++;
							if (matches == boundaryLen) {
								if(state == 2) {
									multiPart->parts.push_back(part);
								}
								headerLines.clear();
								state = 1;
								matches = 0;
							}
						} else {
							if (state == 2) {
								if (matches > 0) {
									// Write out matches that weren't actually a boundary
									if(!part.write(boundary, 0, matches)) {
										return false;
									}
								}
								// Write content
								if(!part.write(postData, i, 1)) {
									return false;
								}
							}
							matches = 0;
						}
					} else if (state == 1) {
						// Scanning for double newline signalling start of content
						if (postData[i] == nl[matches]) {
							matches++;
							if (matches == 2) {
								if (line.length() > 0) {
									// A header
									headerLines.push_back(line);
									if(headerLines.size() > MAX_MULTIPART_HEADERS) {
										g_Logs.http->warn("Exceeded multipart header limit. Rejecting entire post.");
										return false;
									}
									line.clear();
								}
							}
							if (matches == 4) {
								// Parse the headers
								part = Part();
								for (vector<string>::iterator it =
										headerLines.begin();
										it != headerLines.end(); ++it) {
									string s = *it;
									eidx = s.find(":");
									if (eidx != string::npos) {
										string hk = s.substr(0, eidx);
										Util::Trim(hk);
										string hv = s.substr(eidx + 1);
										Util::Trim(hv);
										part.headers[hk] = hv;
									}
								}

								// Start of content
								state = 2;
								matches = 0;
							}
						} else {
							if (matches > 0) {
								line += nlstr.substr(0, matches);
							}
							line += postData[i];
							matches = 0;
						}
					}
				}
			}
			toRead -= dataLen;
		} while (dataLen > 0
				&& (contentLength == 0 || (contentLength != 0 && toRead > 0)));

		return true;
	} else
		return false;
}

bool AbstractCivetHandler::parseForm(CivetServer *server,
		struct mg_connection *conn, map<string, string> &parms) {

	char postData[MG_BUF_LEN + 1];
	string contentType = CivetServer::getHeader(conn, "Content-Type");
	int contentLength = atoi(CivetServer::getHeader(conn, "Content-Length"));

	//application/x-www-form-urlencoded
	if (contentType.find("application/x-www-form-urlencoded") == 0) {

		size_t dataLen, eidx, ptr;
		unsigned long toRead = contentLength;
		vector<string> prms;

		do {
			dataLen = mg_read(conn, postData, sizeof(postData) - 1);
			string val;
			if (dataLen > 0) {
				postData[dataLen] = 0;
				string str = postData;
				ptr = 0;
				while (ptr < dataLen) {
					eidx = str.find("&", ptr);
					if (eidx == string::npos) {
						string rem = str.substr(ptr);
						val.append(rem);

						if(val.length() > MAX_PARAMETER_SIZE || prms.size() > MAX_PARAMETERS) {
							g_Logs.http->warn("Exceeded limits. Rejecting entire post.");
							return false;
						}

//#define MAX_PARAMETER_SIZE 16384
//#define MAX_MULTIPART_HEADERS 10
						break;
					} else {
						string sec = str.substr(ptr, eidx - ptr);
						val.append(sec);
						prms.push_back(val);


						if(val.length() > MAX_PARAMETER_SIZE || prms.size() > MAX_PARAMETERS) {
							g_Logs.http->warn("Exceeded limits. Rejecting entire post.");
							return false;
						}

						val = string();
					}
					ptr = eidx + 1;
				}

				if (val.size() > 0) {
					prms.push_back(val);
					if(val.length() > MAX_PARAMETER_SIZE || prms.size() > MAX_PARAMETERS) {
						g_Logs.http->warn("Exceeded limits. Rejecting entire post.");
						return false;
					}
				}
			}
			toRead -= dataLen;
		} while (dataLen > 0
				&& (contentLength == 0 || (contentLength != 0 && toRead > 0)));

		// Turn strings into parameters
		for (vector<string>::iterator it = prms.begin();
				it != prms.end(); ++it) {
			string p = *it;
			eidx = p.find("=");
			string v = eidx == string::npos ? "" : p.substr(eidx + 1);
			Util::URLDecode(v);
			parms[p.substr(0, eidx)] = v;
		}
	} else {
		return false;
	}

	return true;
}

bool AbstractCivetHandler::isUserAgent(CivetServer *server, struct mg_connection *conn) {
	if(!g_Config.UseUserAgentProtection)
		return true;
	const char* h = server->getHeader(conn, "User-Agent");
	bool ok = h != NULL && ( strcmp(h, "ire-3dply(VERSION)") == 0 ||  strcmp(h, "EETAW") == 0 );
	if(!ok)
		mg_set_as_close(conn);
	return ok;
}

bool AbstractCivetHandler::isAuthorized(CivetServer *server, struct mg_connection *conn,const  string &credentials) {
	const char* h = server->getHeader(conn, "Authorization");
	if(h != NULL) {
		vector<string> l;
		Util::Split(h, " ", l);
		if(l.size() >= 2) {
			string type = l[0];
			string auth = l[1];
			string enc = base64_encode(reinterpret_cast<const unsigned char*>(credentials.c_str()), credentials.length());
			if(type.compare("Basic") == 0 && enc.compare(auth) == 0) {
				return true;
			}
		}
	}
	return false;
}

void AbstractCivetHandler::writeWWWAuthenticate(CivetServer *server,
		struct mg_connection *conn, const string & realm) {
	mg_printf(conn, "HTTP/1.1 401 Unauthorized\r\n");
	mg_printf(conn, "WWW-Authenticate: Basic realm=\"%s\"\r\n\r\n", realm.c_str());
	mg_set_status(conn, 401);
	mg_set_as_close(conn);
}

void AbstractCivetHandler::writeJSON200(CivetServer *server,
		struct mg_connection *conn, const string &data) {
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n", data.size());
	mg_printf(conn, "Content-Type: application/json\r\n\r\n%s", data.c_str());
	mg_set_status(conn, 200);
}

void AbstractCivetHandler::writeStatusPlain(CivetServer *server,
		struct mg_connection *conn, int code, const string &msg,
		const string &data) {
	mg_printf(conn, "HTTP/1.1 %d %s\r\nContent-Length: %lu\r\n", code,
			msg.c_str(), data.size());
	mg_printf(conn, "Content-Type: text/html\r\n\r\n%s", data.c_str());
	mg_set_status(conn, code);
}

void AbstractCivetHandler::writeStatus(CivetServer *server,
		struct mg_connection *conn, int code, const string &msg,
		const string &data) {
	string content = "<html><body>" + data + "</body></html>";
	mg_printf(conn, "HTTP/1.1 %d %s\r\nContent-Length: %lu\r\n", code,
			msg.c_str(), content.size());
	mg_printf(conn, "Content-Type: text/html\r\n\r\n%s", content.c_str());
	mg_set_status(conn, code);
}

void AbstractCivetHandler::writeResponse(CivetServer *server,
		struct mg_connection *conn, const string &data, const string &contentType) {
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n", data.size());
	mg_printf(conn, "Content-Type: %s\r\n\r\n%s", contentType.c_str(),
			data.c_str());
	mg_set_status(conn, 200);
}

//
// PageOptions
//
PageOptions::PageOptions() {
	start = 0;
	sort = "";
	desc = false;
	count = 20;
	top = 0;
}

void PageOptions::Init(CivetServer *server, struct mg_connection *conn) {
	string p;
	if (CivetServer::getParam(conn, "count", p)) {
		count = atoi(p.c_str());
	}
	if (CivetServer::getParam(conn, "top", p)) {
		top = atoi(p.c_str());
	}
	if (CivetServer::getParam(conn, "start", p)) {
		start = atoi(p.c_str());
	}
	if (CivetServer::getParam(conn, "sort", p)) {
		sort = p.c_str();
	}
	if (CivetServer::getParam(conn, "desc", p)) {
		desc = p.compare("true") == 0;
	}
};

