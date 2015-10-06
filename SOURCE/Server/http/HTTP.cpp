#include "HTTP.h"

using namespace HTTPD;

std::string AbstractCivetHandler::formatTime(std::time_t *now) {
	std::tm * ptm = std::localtime(now);
	char nowBuffer[64];
	std::strftime(nowBuffer, 64, "%a, %d %b %Y %H:%M:%S GMT", ptm);
	return nowBuffer;
}

bool AbstractCivetHandler::parsePOST(CivetServer *server, struct mg_connection *conn,
		std::map<std::string, std::string> &parms) {

	char postData[MG_BUF_LEN];

	std::string contentType = CivetServer::getHeader(conn, "Content-Type");
	int contentLength = atoi(CivetServer::getHeader(conn, "Content-Length"));

	if (contentType.find("application/x-www-form-urlencoded") != 0) {
		return false;
	}

	unsigned int dataLen, eidx, ptr;
	unsigned long toRead = contentLength;
	std::vector<std::string> prms;

	do {
		dataLen = mg_read(conn, postData, sizeof(postData));
		std::string val;
		if (dataLen > 0) {
			std::string str = postData;
			ptr = 0;
			while(ptr < dataLen) {
				eidx = str.find("&", ptr);
				if(eidx == std::string::npos) {
					val.append(str.substr(ptr));
					break;
				}
				else {
					val.append(str.substr(ptr, eidx));
					prms.push_back(val);
					val = std::string();
				}
				ptr = eidx + 1;
			}

			if(val.size() > 0)
				prms.push_back(val);
		}
		toRead -= dataLen;
	} while (dataLen > 0
			&& (contentLength == 0 || (contentLength != 0 && toRead > 0)));

	// Turn strings into parameters
	for(std::vector<std::string>::iterator it = prms.begin(); it != prms.end(); ++it) {
		std::string p = *it;
		eidx = p.find("=");
		parms[p.substr(0, eidx)] = eidx == std::string::npos ? "" : p.substr(eidx + 1);
	}

	return true;
}

void AbstractCivetHandler::writeJSON200(CivetServer *server, struct mg_connection *conn,
		std::string data) {
	mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n",
			data.size());
	mg_printf(conn, "Content-Type: text/json\r\n\r\n%s", data.c_str());
}

void AbstractCivetHandler::writeStatus(CivetServer *server, struct mg_connection *conn, int code,
		std::string msg, std::string data) {
	std::string content = "<html><body><h1>" + msg + "</h1></body></html>";
	mg_printf(conn, "HTTP/1.1 %d %s\r\nContent-Length: %lu\r\n", code,
			msg.c_str(), content.size());
	mg_printf(conn, "Content-Type: text/html\r\n\r\n%s", content.c_str());
}

