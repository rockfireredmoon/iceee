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

#ifndef TAWCLIENT_H
#define TAWCLIENT_H

#include "HTTPClient.h"
#include "../ZoneDef.h"
#include <vector>
#include <curl/curl.h>

/*
 * Communicates with another TAW server to retrieve some player specific
 * data (used for account migration).
 */
class TAWClient  {
public:
	TAWClient(std::string url);
	bool getAccountByName(std::string accountName, AccountData &account);
	bool getScenery(int zoneID, SceneryPage &page);
	bool getZone(int zoneID, ZoneDefInfo &zone, std::vector<SceneryPageKey> &pages);
	bool enumerateGroves(int accountID, std::vector<ZoneDefInfo> &zones);
private:
	std::string mUrl;
};

#endif

