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

#ifndef OAUTH2_H
#define OAUTH2_H

#include "CivetServer.h"
#include "HTTP.h"

namespace HTTPD {

/*
 * Handles /oauth/auth requests, which returns HTML to allow login via
 * OAuth2. This chains to /api/login.
 */
class AuthHandler: public AbstractCivetHandler {
public:
	virtual bool handleGet(CivetServer *server, struct mg_connection *conn);
};
/*
 * Handles /oauth/login requests (a POST), which in turn redirects back
 * to the client site with temporary authentication it may use for the
 * next stage, which is a call to /api/token.
 */
class LoginHandler: public AbstractCivetHandler {
public:
	virtual bool handlePost(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /oauth/token requests (a GET), which returns a JSON response containing
 * the token.
 */
class TokenHandler: public AbstractCivetHandler {
public:
	virtual bool handlePost(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/self requests (a GET), which returns a JSON response containing
 * user details.
 */
class SelfHandler: public AbstractCivetHandler {
public:
	virtual bool handleGet(CivetServer *server, struct mg_connection *conn);
};

}

#endif

