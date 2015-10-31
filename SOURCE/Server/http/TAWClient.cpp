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

#include "TAWClient.h"
#include "../StringList.h"
#include <sstream>
#include <iostream>

//
// TAWClient
//

TAWClient::TAWClient(std::string url) {
	mUrl = url;
}


bool TAWClient::getAccountByName(std::string accountName, AccountData &account) {
	std::ostringstream completeUrl;
	completeUrl << mUrl << "/user/" << accountName << std::endl;
	std::string readBuffer;
	int res = readJSONFromUrl(completeUrl.str(), &readBuffer);
	if(res == CURLE_OK) {
		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( readBuffer.c_str(), root );
		if ( !parsingSuccessful) {
			g_Log.AddMessageFormat("Failed to parse JSON account data for %d (on %s)", accountName.c_str(), completeUrl.str().c_str());
			return false;
		}
		account.ReadFromJSON(root);
		return true;
	}
	return false;
}

bool TAWClient::getScenery(int zoneID, SceneryPage &page) {
	std::ostringstream completeUrl;
	completeUrl << mUrl << "/scenery/" << zoneID << "/" << page.mTileX << "/" << page.mTileY << std::endl;
	std::string readBuffer;
	int res = readJSONFromUrl(completeUrl.str(), &readBuffer);
	if(res == CURLE_OK) {
		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( readBuffer.c_str(), root );
		if ( !parsingSuccessful) {
			g_Log.AddMessageFormat("Failed to parse JSON scenery data for %d (on %s)", zoneID, completeUrl.str().c_str());
			return false;
		}
		Json::Value objects = root["objects"];
		for(Json::Value::iterator it = objects.begin(); it != objects.end(); ++it) {
			Json::Value prop = *it;
			SceneryObject obj;
			obj.ReadFromJSON(prop);
			page.AddProp(obj, false);
		}

		return true;
	}
	return false;
}

bool TAWClient::getZone(int zoneID, ZoneDefInfo &zone, std::vector<SceneryPageKey> &pages) {
	std::ostringstream completeUrl;
	completeUrl << mUrl << "/zone/" << zoneID << std::endl;
	std::string readBuffer;
	int res = readJSONFromUrl(completeUrl.str(), &readBuffer);
	if(res == CURLE_OK) {
		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( readBuffer.c_str(), root );
		if ( !parsingSuccessful) {
			g_Log.AddMessageFormat("Failed to parse JSON zone data for %d (on %s)", zoneID, completeUrl.str().c_str());
			return false;
		}
		zone.ReadFromJSON(root);
		Json::Value tiles = root["tiles"];
		for(Json::Value::iterator it = tiles.begin(); it != tiles.end(); ++it) {
			Json::Value tile = *it;
			SceneryPageKey k;
			k.ReadFromJSON(tile);
			pages.push_back(k);
		}

		return true;
	}
	return false;
}


bool TAWClient::enumerateGroves(int accountID, std::vector<ZoneDefInfo> &zones) {
	std::ostringstream completeUrl;
	completeUrl << mUrl << "/user/" << accountID << "/groves" << std::endl;
	std::string readBuffer;
	int res = readJSONFromUrl(completeUrl.str(), &readBuffer);
	if(res == CURLE_OK) {
		Json::Value root;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( readBuffer.c_str(), root );
		if ( !parsingSuccessful) {
			g_Log.AddMessageFormat("Failed to parse JSON account data for %d (on %s)", accountID, completeUrl.str().c_str());
			return false;
		}
		for(Json::Value::iterator it = root.begin(); it != root.end(); ++it) {
			Json::Value grove = *it;
			ZoneDefInfo zDi;
			zDi.ReadFromJSON(grove);
			zones.push_back(zDi);
		}

		return true;
	}
	return false;
}
