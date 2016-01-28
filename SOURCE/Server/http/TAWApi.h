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

#ifndef TAWAPI_H
#define TAWAPI_H

#include "CivetServer.h"
#include "HTTP.h"
#include "../Clan.h"
#include "../Guilds.h"
#include "../CreditShop.h"
#include "../AuctionHouse.h"


namespace HTTPD {

/*
 * All these API requests are authenticated.
 */
class AuthenticatedHandler: public AbstractCivetHandler {
public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	bool handlePost(CivetServer *server, struct mg_connection *conn);
	virtual bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) =0;
	virtual bool handleAuthenticatedPost(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/up requests, returning a JSON response containing uptime data.
 */
class UpHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/who requests, returning a JSON response containing all
 * logged in users.
 */
class WhoHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/chat requests, returning a JSON response containing historical
 * region chat messages.
 */
class ChatHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
	bool handleAuthenticatedPost(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/user/<accountIdOrName> requests, returning a JSON response containing user
 * details
 */
class UserHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/character/<characterIdOrName> requests, returning a JSON response containing character
 * details
 */
class CharacterHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/user/<accountId>/groves requests, returning a JSON response containing a list
 * of an accounts groves
 */
class UserGrovesHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/zone/<zoneDefId> requests, returning a JSON response containing zone
 * details and tiles.
 */
class ZoneHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/scenery/<zoneDefId>/<x>/<y> requests, returning a JSON response containing grove
 * details.
 */
class SceneryHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};

/*
 * Handles /api/leaderboard/<name> requests, returning a JSON response containing player
 * rankings.
 */
class LeaderboardHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};
/*
 * Handles /api/cs/<name> requests, returning a JSON response containing the credit shop.
 */
class CreditShopHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
	bool handleAuthenticatedPost(CivetServer *server, struct mg_connection *conn);
private:
	void writeCreditShopItemToJSON(CS::CreditShopItem* item, Json::Value &c);
};

/*
 * Handles /api/clan and /api/clan/<name> requests, returning a JSON response containing clan details.
 */
class ClanHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
private:
	void writeClanToJSON(Clans::Clan &clan, Json::Value &c);
};

/*
 * Handles /api/guild and /api/guild/<name> requests, returning a JSON response containing clan details.
 */
class GuildHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
private:
	void writeGuildToJSON(GuildDefinition *guild, Json::Value &c);
};

/*
 * Handles /api/item/<name> requests, returning a JSON response containing item details.
 */
class ItemHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
};


/*
 * Handles /api/auction requests, returning a JSON response containing auction details.
 */
class AuctionHandler: public AuthenticatedHandler {
public:
	bool handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn);
private:
	void writeAuctionItemToJSON(AuctionHouseItem * item, Json::Value &c);
};

}

#endif

