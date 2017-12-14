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

#include "CivetServer.h"
#include "CAR.h"

#include "HTTP.h"
#include "HTTPService.h"
#include "../Util.h"
#include "../Config.h"
#include "../DirectoryAccess.h"

#include "../util/Log.h"

#include <sys/stat.h>
#include <string.h>

using namespace HTTPD;

/* Send bytes from the opened file to the client. */
static void send_file_data(struct mg_connection *conn, FileResource *filep) {
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

int CARHandler::openFile(const struct mg_request_info * req_info, FileResource *file) {
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

bool CARHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	/* Simple protection against drive-by penetration attempts uses user-agent */
	if(!isUserAgent(server, conn))
		return false;

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);

	//
	int status = 304;
	FileResource file;
	std::string newChecksum;

	/* Get the full path of the file */
	std::string nativePath = ruri;
	Util::Replace(nativePath,
			std::string(1, PLATFORM_FOLDERINVALID).c_str()[0],
			std::string(1, PLATFORM_FOLDERVALID).c_str()[0]);
	file.filePath = std::string(g_HTTPBaseFolder) + nativePath;

	/* Process headers */
	const char * checksum = CivetServer::getHeader(conn, "If-None-Match");
	if (checksum == NULL) {
		checksum = CivetServer::getHeader(conn, "If-Modified-Since");
	}

	// Get size and last modified details regardless
	struct stat st;
	if (!stat(file.filePath.c_str(), &st)) {
		file.fileSize = st.st_size;
		file.lastModified = st.st_mtime;
	} else {
		file.lastModified = (std::time_t) 0;
	}

	// Open the file if either there was no condition, or the checksum doesn't match
	if (checksum == NULL) {
		// No condition
		status = openFile(req_info, &file);
		newChecksum = g_FileChecksum.MatchChecksum(ruri, "__");
	} else {
		//If the checksum does not match, we need to update.
		newChecksum = g_FileChecksum.MatchChecksum(ruri, checksum);
		if (newChecksum.size() > 0) {
			status = openFile(req_info, &file);
		}
	}

	mg_set_status(conn, status);

	switch (status) {
	case 200:
	{
		g_Logs.http->info("Sending %v (%v bytes)", ruri.c_str(), file.fileSize);
		mg_printf(conn,	"HTTP/1.1 200 OK\r\n");
		mg_printf(conn, "Content-Type: application/octet-stream\r\n");
		mg_printf(conn,	"Content-Length: %lu\r\n", file.fileSize);
		mg_printf(conn, "Accept-Range: bytes\r\n");
		std::string fn;
		fn = Platform::Filename(ruri);
		mg_printf(conn,
				"Content-Disposition: attachment; filename=\"%s\";\r\n",
				fn.c_str());
		mg_printf(conn, "Last-Modified: %s\r\n", formatTime(&file.lastModified).c_str());
		mg_printf(conn, "\r\n");
		send_file_data(conn, &file);
	    fclose(file.fd);
		mg_increase_sent_bytes(conn, file.fileSize);
		break;
	}
	case 304: {
		g_Logs.http->info("Not modified %v (%v bytes)", ruri.c_str(), file.fileSize);
		std::time_t now = std::time(NULL);
		mg_printf(conn, "HTTP/1.1 304 Not Modified\r\n");
		mg_printf(conn, "Expires: Tue, 1 Nov 2011 00:00:00 GMT\r\n");
		mg_printf(conn, "Date: %s\r\n", formatTime(&now).c_str());
		mg_printf(conn, "Last-Modified: %s\r\n", formatTime(&file.lastModified).c_str());
		mg_printf(conn, "Cache-Control: max-age=0\r\n\r\n");
		mg_set_as_close(conn);
		break;
	}
	default:
		g_Logs.http->info("Could not find %v", ruri.c_str());
		mg_printf(conn, "HTTP/1.1 404 Not Found\r\n");
		mg_printf(conn, "Content-Length: %d\r\n",
				(int) g_HTTP404Message.size());
		mg_printf(conn, "Content-Type: text/html\r\n\r\n");
		mg_printf(conn, "%s", g_HTTP404Message.c_str());
		mg_set_as_close(conn);
		break;
	}
	return true;
}
