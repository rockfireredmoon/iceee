#include "Mail.h"
#include "FileReader.h"
#include "Util.h"
#include "StringList.h"
#include "Config.h"
#include <string.h>
#include <curl/curl.h>

MailManager g_MailManager;

MailManager::MailManager() {
}

MailManager::~MailManager() {
}

bool MailManager::Mail(const char *subject, const char *recipient,
		const char *body) {
	CURL *curl;
	CURLcode res = CURLE_OK;
	//struct curl_slist *recipients = NULL;
	//struct upload_status upload_ctx;

	//upload_ctx.lines_read = 0;

	curl = curl_easy_init();
	if (curl) {
	    curl_easy_setopt(curl, CURLOPT_USERNAME, g_Config.SMTPUsername.c_str());
	    curl_easy_setopt(curl, CURLOPT_PASSWORD, g_Config.SMTPPassword.c_str());
	    if(!g_Config.SSLVerifyPeer) {
	    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	    }
	    if(!g_Config.SSLVerifyHostname) {
	    	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	    }
//	    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, g_Config.SMTPSender.c_str());
	}

	return false;
}

