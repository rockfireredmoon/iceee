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

#include "LegacyAccounts.h"
#include "../Config.h"
#include "../Util.h"
#include "../Account.h"

#include <vector>

using namespace HTTPD;

//
// NewAccountHandler
//

bool NewAccountHandler::handlePost(CivetServer *server, struct mg_connection *conn) {

	char post_data[1024];

	char username[128];
	char password[128];
	char grove[128];
	char regkey[256];

	int post_data_len;

	post_data_len = mg_read(conn, post_data, sizeof(post_data));

	if (mg_get_var(post_data, post_data_len, "regkey", regkey,
			sizeof(regkey)) > 0
		&& mg_get_var(post_data, post_data_len, "username", username,
				sizeof(username)) > 0
		&& mg_get_var(post_data, post_data_len, "password", password,
				sizeof(password)) > 0
		&& mg_get_var(post_data, post_data_len, "grove",
				grove, sizeof(grove)) > 0) {

		int retval = 0;
		g_AccountManager.cs.Enter("CreateAccount");
		retval = g_AccountManager.CreateAccount(username, password, regkey, grove);
		g_AccountManager.cs.Leave();
		writeStatus(server, conn, 200, "OK", g_AccountManager.GetErrorMessage(retval));
	}
	else {
		writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
	}

	return true;
}

//
// ResetPasswordHandler
//

bool ResetPasswordHandler::handlePost(CivetServer *server, struct mg_connection *conn) {

	char post_data[1024];

	char username[128];
	char newpassword[128];
	char regkey[256];

	int post_data_len;

	post_data_len = mg_read(conn, post_data, sizeof(post_data));

	if (mg_get_var(post_data, post_data_len, "regkey", regkey,
			sizeof(regkey)) > 0
		&& mg_get_var(post_data, post_data_len, "username", username,
				sizeof(username)) > 0
		&& mg_get_var(post_data, post_data_len, "newpassword", newpassword,
				sizeof(newpassword)) > 0) {

		int retval = 0;
		g_AccountManager.cs.Enter("ResetPassword");
		retval = g_AccountManager.ResetPassword(username, newpassword, regkey);
		g_AccountManager.cs.Leave();
		writeStatus(server, conn, 200, "OK", g_AccountManager.GetErrorMessage(retval));
	}
	else {
		writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
	}

	return true;
}

//
// AccountRecover
//

bool AccountRecoverHandler::handlePost(CivetServer *server, struct mg_connection *conn) {

	char post_data[1024];

	char username[128];
	char keypass[128];
	char type[128];

	int post_data_len;

	post_data_len = mg_read(conn, post_data, sizeof(post_data));

	if (mg_get_var(post_data, post_data_len, "username", username,
			sizeof(username)) > 0
		&& mg_get_var(post_data, post_data_len, "keypass", keypass,
				sizeof(keypass)) > 0
		&& mg_get_var(post_data, post_data_len, "type", type,
				sizeof(type)) > 0) {

		int retval = 0;
		g_AccountManager.cs.Enter("AccountRecover");
		retval = g_AccountManager.AccountRecover(username, keypass, type);
		g_AccountManager.cs.Leave();
		writeStatus(server, conn, 200, "OK", g_AccountManager.GetErrorMessage(retval));
	}
	else {
		writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
	}

	return true;
}
