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
#include "../StringList.h"
#include "../Util.h"
#include <stdlib.h>
#include <string.h>
#include "../util/base64.h"

using namespace HTTPD;

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
// MultiPart
Part MultiPart::getPartWithName(std::string name) {
	for (std::vector<Part>::iterator it = parts.begin(); it != parts.end();
			++it) {
		Part p = *it;
		std::map<std::string, std::string> hv = p.getHeaderValues(
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
		g_Log.AddMessage("[WARNING] Exceeded multipart header limit. Rejecting entire post.");
		return false;
	}
	return true;
}

std::map<std::string, std::string> Part::getHeaderValues(std::string name) {
	std::string v = headers[name];
	std::vector<std::string> p;
	Util::Split(v, ";", p);
	std::map<std::string, std::string> m;
	size_t eidx;
	for (std::vector<std::string>::iterator it = p.begin(); it != p.end();
			++it) {
		std::string v = *it;
		eidx = v.find("=");
		if (eidx == std::string::npos) {
			Util::Trim(v);
			m[v] = "";
		} else {
			std::string s = v.substr(eidx + 1);
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

std::string AbstractCivetHandler::formatTime(std::time_t *now) {
	std::tm * ptm = std::localtime(now);
	char nowBuffer[64];
	std::strftime(nowBuffer, 64, "%a, %d %b %Y %H:%M:%S GMT", ptm);
	return nowBuffer;
}

bool AbstractCivetHandler::parseMultiPart(CivetServer *server,
		struct mg_connection *conn, MultiPart *multiPart) {

	char postData[MG_BUF_LEN + 1];
	std::string contentType = CivetServer::getHeader(conn, "Content-Type");
	int contentLength = atoi(CivetServer::getHeader(conn, "Content-Length"));
	if (contentType.find("multipart/form-data") == 0) {
		size_t sep = contentType.find(";");
		if (sep == std::string::npos) {
			// No boundary
			return false;
		}
		std::string boundaryPair = contentType.substr(sep + 1);
		Util::Trim(boundaryPair);
		std::string k, v;
		extractPair(boundaryPair, k, v);
		std::string eob = "\r\n--" + v + "--";
		v = "\r\n--" + v + "\r\n";

		size_t dataLen, eidx, toRead = contentLength, matches = 2;
		std::vector<std::string> prms;
		const char * boundary = v.c_str();
		const char * boundary2 = eob.c_str();
		std::string nlstr = "\r\n\r\n";
		const char * nl = nlstr.c_str();
		unsigned int boundaryLen = strlen(boundary);

		std::vector<std::string> headerLines;
		std::string line;
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
										g_Log.AddMessage("[WARNING] Exceeded multipart header limit. Rejecting entire post.");
										return false;
									}
									line.clear();
								}
							}
							if (matches == 4) {
								// Parse the headers
								part = Part();
								for (std::vector<std::string>::iterator it =
										headerLines.begin();
										it != headerLines.end(); ++it) {
									std::string s = *it;
									eidx = s.find(":");
									if (eidx != std::string::npos) {
										std::string hk = s.substr(0, eidx);
										Util::Trim(hk);
										std::string hv = s.substr(eidx + 1);
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
		struct mg_connection *conn, std::map<std::string, std::string> &parms) {

	char postData[MG_BUF_LEN + 1];
	std::string contentType = CivetServer::getHeader(conn, "Content-Type");
	int contentLength = atoi(CivetServer::getHeader(conn, "Content-Length"));

	//application/x-www-form-urlencoded
	if (contentType.find("application/x-www-form-urlencoded") == 0) {

		size_t dataLen, eidx, ptr;
		unsigned long toRead = contentLength;
		std::vector<std::string> prms;

		do {
			dataLen = mg_read(conn, postData, sizeof(postData) - 1);
			std::string val;
			if (dataLen > 0) {
				postData[dataLen] = 0;
				std::string str = postData;
				ptr = 0;
				while (ptr < dataLen) {
					eidx = str.find("&", ptr);
					if (eidx == std::string::npos) {
						std::string rem = str.substr(ptr);
						val.append(rem);

						if(val.length() > MAX_PARAMETER_SIZE || prms.size() > MAX_PARAMETERS) {
							g_Log.AddMessage("[WARNING] Exceeded limits. Rejecting entire post.");
							return false;
						}

//#define MAX_PARAMETER_SIZE 16384
//#define MAX_MULTIPART_HEADERS 10
						break;
					} else {
						std::string sec = str.substr(ptr, eidx - ptr);
						val.append(sec);
						prms.push_back(val);


						if(val.length() > MAX_PARAMETER_SIZE || prms.size() > MAX_PARAMETERS) {
							g_Log.AddMessage("[WARNING] Exceeded limits. Rejecting entire post.");
							return false;
						}

						val = std::string();
					}
					ptr = eidx + 1;
				}

				if (val.size() > 0) {
					prms.push_back(val);
					if(val.length() > MAX_PARAMETER_SIZE || prms.size() > MAX_PARAMETERS) {
						g_Log.AddMessage("[WARNING] Exceeded limits. Rejecting entire post.");
						return false;
					}
				}
			}
			toRead -= dataLen;
		} while (dataLen > 0
				&& (contentLength == 0 || (contentLength != 0 && toRead > 0)));

		// Turn strings into parameters
		for (std::vector<std::string>::iterator it = prms.begin();
				it != prms.end(); ++it) {
			std::string p = *it;
			eidx = p.find("=");
			std::string v = eidx == std::string::npos ? "" : p.substr(eidx + 1);
			Util::URLDecode(v);
			parms[p.substr(0, eidx)] = v;
		}
	} else {
		return false;
	}

	return true;
}

bool AbstractCivetHandler::isAuthorized(CivetServer *server, struct mg_connection *conn, std::string credentials) {
	const char* h = server->getHeader(conn, "Authorization");
	if(h != NULL) {
		std::vector<std::string> l;
		Util::Split(h, " ", l);
		if(l.size() >= 2) {
			std::string type = l[0];
			std::string auth = l[1];
			std::string enc = base64_encode(reinterpret_cast<const unsigned char*>(credentials.c_str()), credentials.length());
			if(type.compare("Basic") == 0 && enc.compare(auth) == 0) {
				return true;
			}
		}
	}
	return false;
}

void AbstractCivetHandler::writeWWWAuthenticate(CivetServer *server,
		struct mg_connection *conn, std::string realm) {
	mg_printf(conn, "HTTP/1.1 401 Unauthorized\r\n");
	mg_printf(conn, "WWW-Authenticate: Basic realm=\"%s\"\r\n\r\n", realm.c_str());
	mg_set_status(conn, 401);
	mg_set_as_close(conn);
}

void AbstractCivetHandler::writeJSON200(CivetServer *server,
		struct mg_connection *conn, std::string data) {
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n", data.size());
	mg_printf(conn, "Content-Type: application/json\r\n\r\n%s", data.c_str());
	mg_set_status(conn, 200);
}

void AbstractCivetHandler::writeStatus(CivetServer *server,
		struct mg_connection *conn, int code, std::string msg,
		std::string data) {
	std::string content = "<html><body><h1>" + msg + "</h1></body></html>";
	mg_printf(conn, "HTTP/1.1 %d %s\r\nContent-Length: %lu\r\n", code,
			msg.c_str(), content.size());
	mg_printf(conn, "Content-Type: text/html\r\n\r\n%s", content.c_str());
	mg_set_status(conn, code);
}

void AbstractCivetHandler::writeResponse(CivetServer *server,
		struct mg_connection *conn, std::string data, std::string contentType) {
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n", data.size());
	mg_printf(conn, "Content-Type: %s\r\n\r\n%s", contentType.c_str(),
			data.c_str());
	mg_set_status(conn, 200);
}

