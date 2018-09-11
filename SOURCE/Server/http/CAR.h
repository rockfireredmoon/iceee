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

#ifndef CAR_H
#define CAR_H

#include "CivetServer.h"
#include "HTTP.h"

namespace HTTPD {

/*
 * Handles requests for CAR files in Sparkplayers weird HTTP dialect.
 */
class CARHandler: public AbstractCivetHandler {
public:
	virtual bool handleGet(CivetServer *server, struct mg_connection *conn);
};

}

#endif

