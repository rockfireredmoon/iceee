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

#ifndef LEGACYACCOUNTS_H
#define LEGACYACCOUNTS_H

#include "CivetServer.h"
#include "HTTP.h"

namespace HTTPD {

/*
 * Handles /newaccount requests (a POST), which is used by the 'Legacy' account
 * registration system.
 */
class NewAccountHandler: public AbstractCivetHandler {
public:
	virtual bool handlePost(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /resetpassword requests (a POST), which is used by the 'Legacy' account
 * registration system.
 */
class ResetPasswordHandler: public AbstractCivetHandler {
public:
	virtual bool handlePost(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /accountrecover requests (a POST), which is used by the 'Legacy' account
 * registration system.
 */
class AccountRecoverHandler: public AbstractCivetHandler {
public:
	virtual bool handlePost(CivetServer *server, struct mg_connection *conn);
};

}

#endif

