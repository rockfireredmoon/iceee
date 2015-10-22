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

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <vector>
#include <curl/curl.h>
#include <string>
#include "../StringList.h"
#include <sstream>

struct Writeable {
	const char *readptr;
	long sizeleft;
};

inline static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

inline static size_t readCallback(void *ptr, size_t size, size_t nmemb,
		void *userp) {
	struct Writeable *wrt = (struct Writeable *) userp;

	if (size * nmemb < 1)
		return 0;

	if (wrt->sizeleft) {
		*(char *) ptr = wrt->readptr[0];
		wrt->readptr++;
		wrt->sizeleft--;
		return 1;
	}

	return 0;
}

inline static int readJSONFromUrl(std::string url, std::string *readBuffer) {
	g_Log.AddMessageFormat("Reading JSON from %s", url.c_str());

	CURL *curl;
	curl = curl_easy_init();
	if(curl) {
		std::ostringstream completeUrl;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "EETAW");

		struct curl_slist *headers = NULL;

		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		// TODO might need config item to disable SSL verification
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		CURLcode res;
		res = curl_easy_perform(curl);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		return res;
	}
	return CURLE_FAILED_INIT;
}

#endif

