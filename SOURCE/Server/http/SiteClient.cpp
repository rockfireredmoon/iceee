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

#include "SiteClient.h"
#include "../Config.h"
#include "../Util.h"
#include "../json/json.h"
#include "../util/Log.h"


//
// SiteClient
//

SiteClient::SiteClient(std::string url) {
	mUrl = url;
}

int SiteClient::sendRequest(HTTPD::SiteSession *session, std::string path, std::string &content) {
	struct curl_slist *headers = NULL;
	CURL *curl;
	curl = curl_easy_init();
	if(curl) {
		char url[256];
		char token[256];
		char cookie[256];

		Util::SafeFormat(url, sizeof(url), "%s/%s.json", mUrl.c_str(), path.c_str());
		if(session->xCSRF.size() > 0)
			Util::SafeFormat(token, sizeof(token), "X-CSRF-Token: %s", session->xCSRF.c_str());
		Util::SafeFormat(cookie, sizeof(cookie), "Cookie: %s=%s", session->sessionName.c_str(),session->sessionID.c_str());

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "EETAW");

		headers = curl_slist_append(headers, token);
		headers = curl_slist_append(headers, cookie);
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		// TODO might need config item to disable SSL verification
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		CURLcode res;
		res = curl_easy_perform(curl);

		long http_code = 0;
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return (int)http_code;
	}

	return CURLE_SEND_ERROR;
}



int SiteClient::postJSON(HTTPD::SiteSession *session, std::string path, std::string &content, std::string &reply) {
	struct curl_slist *headers = NULL;
	CURL *curl;
	curl = curl_easy_init();
	if(curl) {
		struct Writeable wrt;

		wrt.readptr = content.c_str();
		wrt.sizeleft = content.length();

		char url[256];
		char token[256];
		char cookie[256];

		Util::SafeFormat(url, sizeof(url), "%s/%s.json", mUrl.c_str(), path.c_str());
		if(session->xCSRF.size() > 0)
			Util::SafeFormat(token, sizeof(token), "X-CSRF-Token: %s", session->xCSRF.c_str());
		Util::SafeFormat(cookie, sizeof(cookie), "Cookie: %s=%s", session->sessionName.c_str(),session->sessionID.c_str());

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "EETAW");

		headers = curl_slist_append(headers, token);
		headers = curl_slist_append(headers, cookie);
		headers = curl_slist_append(headers, "Content-Type: application/json");

	    curl_easy_setopt(curl, CURLOPT_POST, 1L);
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content.size());

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
		curl_easy_setopt(curl, CURLOPT_READDATA, &wrt);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

#ifdef OUTPUT_TO_CONSOLE
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

		// TODO might need config item to disable SSL verification
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		CURLcode res;
		res = curl_easy_perform(curl);

		long http_code = 0;
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return (int)http_code;
	}

	return CURLE_SEND_ERROR;
}

bool SiteClient::login(HTTPD::SiteSession *session, std::string username, std::string password) {
	Json::Value body;
	body["username"] = username;
	body["password"] = password;
	Json::StyledWriter writer;
	std::string output = writer.write(body);
	int res;
	std::string replyBuffer;
	res = postJSON(session, "user/login", output, replyBuffer);
	if(res == 200) {
		Json::Value root;
		Json::Reader reader;
		if (reader.parse( replyBuffer.c_str(), root ) && root.size() > 0) {
			session->sessionID = root["sessid"].asCString();
			session->sessionName = root["session_name"].asCString();
			return true;
		}
	}
	else {
		g_Logs.server->warn("Failed to authenticate with error %v", res);
	}
	return false;
}

int SiteClient::refreshXCSRF(HTTPD::SiteSession *session) {
	std::string sendBuffer;
	std::string replyBuffer;
	int res;
	session->xCSRF = "";
	res = postJSON(session, "user/token", sendBuffer, replyBuffer);
	if(res == 200) {
		Json::Value root;
		Json::Reader reader;
		if (reader.parse( replyBuffer.c_str(), root ) && root.size() > 0) {
			session->xCSRF = root["token"].asCString();
		}
	}
	return res;
}

int SiteClient::getUnreadPrivateMessages(HTTPD::SiteSession *session) {
	std::string readBuffer;
	int res;
	res = sendRequest(session, "privatemsgunread", readBuffer);
	if(res == 200) {
		Json::Value root;
		Json::Reader reader;
		if (reader.parse( readBuffer.c_str(), root ) && root.size() > 0) {
			return atoi(root[0].asCString());
		}
	}
	return -1;
}

bool SiteClient::sendPrivateMessage(HTTPD::SiteSession *session, std::string recipient, std::string subject, std::string message) {
	std::string readBuffer;
	std::string replyBuffer;

	Json::Value root;
	root["subject"] = subject;
	root["body"] = message;
	root["recipients"] = recipient;

	Json::StyledWriter writer;
	std::string output = writer.write(root);
	int res = postJSON(session, "privatemsg", output, replyBuffer);
	return res == 200;
}
