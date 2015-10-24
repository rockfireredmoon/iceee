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

#ifndef SITECLIENT_H
#define SITECLIENT_H

#include "HTTP.h"
#include "HTTPClient.h"
#include <string>
#include <curl/curl.h>

/*
 * Client that communicates with the website using Drupal services.
 */
class SiteClient  {
public:
	SiteClient(std::string url);
	int refreshXCSRF(HTTPD::SiteSession *session);
	int postJSON(HTTPD::SiteSession *session, std::string path, std::string &content, std::string &reply);
	int sendRequest(HTTPD::SiteSession *session, std::string path, std::string &content);
	int getUnreadPrivateMessages(HTTPD::SiteSession *session);
	bool sendPrivateMessage(HTTPD::SiteSession *session, std::string recipient, std::string subject, std::string message);
private:
	std::string mUrl;
};

#endif

