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

#include "TAWApi.h"
#include "../Chat.h"
#include "../Util.h"
#include "../Account.h"
#include "../Simulator.h"
#include "../Character.h"
#include "../CreditShop.h"
#include "../Clan.h"
#include "../Guilds.h"
#include "../ZoneDef.h"
#include "../Scenery2.h"
#include "../Config.h"
#include "../Chat.h"
#include "../DirectoryAccess.h"
#include "../PlayerStats.h"
#include "../Leaderboard.h"
#include "../json/json.h"
#include <algorithm>

using namespace HTTPD;

//
// AuthenticatedHandler
//

bool AuthenticatedHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
	if(!isAuthorized(server, conn, g_Config.APIAuthentication)) {
		writeWWWAuthenticate(server, conn, "TAWD");
		return true;
	}
	return handleAuthenticatedGet(server, conn);
}

bool AuthenticatedHandler::handlePost(CivetServer *server, struct mg_connection *conn) {
	if(!isAuthorized(server, conn, g_Config.APIAuthentication)) {
		writeWWWAuthenticate(server, conn, "TAWD");
		return true;
	}
	return handleAuthenticatedPost(server, conn);
}

bool AuthenticatedHandler::handleAuthenticatedPost(CivetServer *server, struct mg_connection *conn) {
	return false;
}

//
// WhoHandler
//

bool WhoHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {
	std::string response;
	char buf[256];
	response.append("{ ");
	SIMULATOR_IT it;
	int no = 0;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1
				&& it->LoadStage == SimulatorThread::LOADSTAGE_GAMEPLAY) {
			if (it->IsGMInvisible() == true) //Hide GM Invisibility from the list.
				continue;
			CharacterData *cd = it->pld.charPtr;
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(
					it->pld.CurrentZoneID);
			if (cd != NULL && zd != NULL) {
				Util::SafeFormat(buf, sizeof(buf),
						"%s\"%s\" : { \"zone\": \"%s\", \"shard\": \"%s\" }",
						no > 0 ? "," : "", cd->cdef.css.display_name,
						zd->mName.c_str(), zd->mShardName.c_str());
				no++;
				response.append(buf);
			}
		}
	}
	response.append(" }");
	writeJSON200(server, conn, response);

	return true;
}

//
// CreditShopHandler
//

bool CreditShopHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {
	char buf[256];
	int no = 0;

	PageOptions opts;
	opts.count = 10;
	opts.Init(server, conn);

	int category = Category::UNDEFINED;

	// Parse parameters
	std::string p;
	if (CivetServer::getParam(conn, "category", p)) {
		category = Category::GetIDByName(p.c_str());
	}

	Json::Value root;
	std::vector<CS::CreditShopItem*> items;
	g_CreditShopManager.cs.Enter("LeaderboardHandler::g_CreditShopManager");
	std::map<int, CS::CreditShopItem*> itemMap = g_CreditShopManager.mItems;
	for(std::map<int, CS::CreditShopItem*>::iterator it = itemMap.begin(); it != itemMap.end(); ++it)	{
		if(it-> second->mCategory == category) {
			items.push_back(it->second);
		}
	}
	g_CreditShopManager.cs.Leave();



	Json::Value data;
	int didx = 0;
	for(int i = opts.start ; i < opts.start + opts.count && i < items.size() ; i++) {
		Json::Value jv;
		items[i]->WriteToJSON(jv);

		if(items[i]->mItemId > 0) {
			ItemDef *item = g_ItemManager.GetSafePointerByID(items[i]->mItemId);
			if(item != NULL) {
				if(items[i]->mTitle.length() == 0)
					jv["computedTitle"] = item->mDisplayName;
				std::vector<std::string> l;
				Util::Split(item->mIcon, "|", l);
				switch(l.size()) {
				case 1:
					jv["icon1"] = "TODO";
					jv["icon2"] = l[0];
					break;
				case 2:
					jv["icon1"] = l[0];
					jv["icon2"] = l[1];
					break;
				}
			}
		}
		if(!jv.isMember("computedTitle")) {
			jv["computedTitle"] = items[i]->mTitle;
		}

		data[didx++] = jv;
	}

	root["data"] = data;
	root["total"] = Json::UInt64(items.size());

	Json::StyledWriter writer;
	writeJSON200(server, conn, writer.write(root));

	return true;

}

//
// LeaderboardHandler
//

bool killsSort(const Leader &l1, const Leader &l2) {
	return l1.mStats.TotalKills > l2.mStats.TotalKills;
}

bool deathsSort(const Leader &l1, const Leader &l2) {
	return l1.mStats.TotalDeaths > l2.mStats.TotalDeaths;
}

bool pvpKillsSort(const Leader &l1, const Leader &l2) {
	return l1.mStats.TotalPVPKills > l2.mStats.TotalPVPKills;
}

bool pvpDeathsSort(const Leader &l1, const Leader &l2) {
	return l1.mStats.TotalPVPDeaths > l2.mStats.TotalPVPDeaths;
}

bool LeaderboardHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {
	char buf[256];
	int no = 0;

	PageOptions opts;
	opts.Init(server, conn);

	std::string board = "character";

	// Parse parameters
	std::string p;
	if (CivetServer::getParam(conn, "board", p)) {
		board = p.c_str();
	}

	Leaderboard *leaderboard = g_LeaderboardManager.GetBoard(board);
	if(leaderboard == NULL)
		writeStatus(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {

		Json::Value root;

		leaderboard->cs.Enter("LeaderboardHandler::handleAuthenticatedGet");
		std::vector<Leader> l(leaderboard->mLeaders);
		leaderboard->cs.Leave();

		if(opts.sort.compare("deaths") == 0)
			sort(l.begin(), l.end(), deathsSort);
		else if(opts.sort.compare("pvpKills") == 0)
			sort(l.begin(), l.end(), pvpKillsSort);
		else if(opts.sort.compare("pvpDeaths") == 0)
			sort(l.begin(), l.end(), pvpDeathsSort);
		else
			sort(l.begin(), l.end(), killsSort);

		if(opts.desc)
			std::reverse(l.begin(),l.end());

		Json::Value data;
		int didx = 0;
		for(int i = opts.start ; i < opts.start + opts.count && i < l.size() ; i++) {
			Json::Value jv;
			l[i].WriteToJSON(jv);
			jv["rank"] = i + 1;
			data[didx++] = jv;
		}

		root["data"] = data;
		root["total"] = Json::UInt64(l.size());
		root["collected"] = Json::UInt64(leaderboard->mCollected);

		Json::StyledWriter writer;
		writeJSON200(server, conn, writer.write(root));
	}

	return true;

}

//
// ClanHandler
//

bool rankSort(const Clans::ClanMember &l1, const Clans::ClanMember &l2) {
	return l1.mRank > l2.mRank;
}

void ClanHandler::writeClanToJSON(Clans::Clan &clan, Json::Value &c) {

	PlayerStatSet total;
	clan.WriteToJSON(c);

	std::vector<Clans::ClanMember> l(clan.mMembers);
	sort(l.begin(), l.end(), rankSort);

	for(std::vector<Clans::ClanMember>::iterator it2 = l.begin(); it2 != l.end(); ++it2) {
		CharacterData *cd = g_CharacterManager.RequestCharacter((*it2).mID, true);
		if(cd != NULL) {
			char buf[12];
			Util::SafeFormat(buf,sizeof(buf),"%d", it2->mID);
			c["members"][buf]["name"] = cd->cdef.css.display_name;
			c["members"][buf]["level"] = cd->cdef.css.level;
			c["members"][buf]["profession"] = cd->cdef.css.profession;

			Json::Value stats;
			cd->PlayerStats.WriteToJSON(stats);
			total.Add(cd->PlayerStats);

			c["members"][buf]["playerStats"] = stats;

			if((*it2).mRank == Clans::Rank::LEADER) {
				Json::Value l;
				l["name"] =  cd->cdef.css.display_name;
				l["id"] =  cd->cdef.CreatureDefID;
				c["leader"] = l;
			}
		}
		else {
			g_Log.AddMessageFormat("[WARNING] Clan %s (%d) contains member %d that does not exist.", clan.mName.c_str(), clan.mId, (*it2).mID);
		}
	}

	c["size"] = Json::UInt64(clan.mMembers.size());

	Json::Value totalStats;
	total.WriteToJSON(totalStats);
	c["playerStats"] = totalStats;
}

bool ClanHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {

	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeStartSlash(removeEndSlash(ruri));

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	PageOptions opts;
	opts.Init(server, conn);


	Json::Value root;
	if(pathParts.size() > 0 && pathParts[pathParts.size() - 1].compare("clans") == 0) {
		// List clans
		std::map<int, Clans::Clan> m = g_ClanManager.mClans;

		for(std::map<int, Clans::Clan>::iterator it = m.begin(); it != m.end(); ++it) {
			Json::Value c;
			Clans::Clan clan = it->second;
			writeClanToJSON(clan, c);
			root[clan.mName] = c;
		}
	}
	else {
		// TODO get clan
		std::string name = pathParts[pathParts.size() - 1];
		Util::URLDecode(name);
		int clanID = g_ClanManager.FindClanID(name);
		if(clanID == -1) {
			writeStatus(server, conn, 404, "Not found.",
					"The clan could not be found.");
			return true;
		}
		writeClanToJSON(g_ClanManager.mClans[clanID], root);
	}
	Json::StyledWriter writer;
	writeJSON200(server, conn, writer.write(root));

	return true;

}



//
// GuildHandler
//

//bool rankSort(const Clans::ClanMember &l1, const Clans::ClanMember &l2) {
//	return l1.mRank > l2.mRank;
//}

void GuildHandler::writeGuildToJSON(GuildDefinition *guild, Json::Value &c) {

	PlayerStatSet total;
	guild->WriteToJSON(c);

//	std::vector<Clans::ClanMember> l(clan.mMembers);
//	sort(l.begin(), l.end(), rankSort);
//
//	for(std::vector<Clans::ClanMember>::iterator it2 = l.begin(); it2 != l.end(); ++it2) {
//		CharacterData *cd = g_CharacterManager.RequestCharacter((*it2).mID, true);
//		if(cd != NULL) {
//			char buf[12];
//			Util::SafeFormat(buf,sizeof(buf),"%d", it2->mID);
//			c["members"][buf]["name"] = cd->cdef.css.display_name;
//			c["members"][buf]["level"] = cd->cdef.css.level;
//			c["members"][buf]["profession"] = cd->cdef.css.profession;
//
//			Json::Value stats;
//			cd->PlayerStats.WriteToJSON(stats);
//			total.Add(cd->PlayerStats);
//
//			c["members"][buf]["playerStats"] = stats;
//
//			if((*it2).mRank == Clans::Rank::LEADER) {
//				Json::Value l;
//				l["name"] =  cd->cdef.css.display_name;
//				l["id"] =  cd->cdef.CreatureDefID;
//				c["leader"] = l;
//			}
//		}
//		else {
//			g_Log.AddMessageFormat("[WARNING] Clan %s (%d) contains member %d that does not exist.", clan.mName.c_str(), clan.mId, (*it2).mID);
//		}
//	}
//
//	c["size"] = Json::UInt64(clan.mMembers.size());
//
//	Json::Value totalStats;
//	total.WriteToJSON(totalStats);
//	c["playerStats"] = totalStats;
}

bool GuildHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {

	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeStartSlash(removeEndSlash(ruri));

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	PageOptions opts;
	opts.Init(server, conn);


	Json::Value root;
	if(pathParts.size() > 0 && pathParts[pathParts.size() - 1].compare("guilds") == 0) {
		// List clans
		std::vector<GuildDefinition> m = g_GuildManager.defList;

		for(std::vector<GuildDefinition>::iterator it = m.begin(); it != m.end(); ++it) {
			Json::Value c;
			GuildDefinition guild = *it;
			writeGuildToJSON(&guild, c);
			root[guild.defName] = c;
		}
	}
	else {
		std::string name = pathParts[pathParts.size() - 1];
		Util::URLDecode(name);
		GuildDefinition *guild = g_GuildManager.FindGuildDefinition(name);
		if(guild == NULL) {
			writeStatus(server, conn, 404, "Not found.",
					"The guild could not be found.");
			return true;
		}
		writeGuildToJSON(guild, root);
	}
	Json::StyledWriter writer;
	writeJSON200(server, conn, writer.write(root));

	return true;

}

//
// ChatHandler
//

void WriteChat(Json::Value &chat, int count) {
	char buf[256];
	std::deque<ChatMessage>::iterator it;
	g_ChatManager.cs.Enter("HTTPDistribute::Chat");
	int start = g_ChatManager.CircularChatBuffer.size() - 1 - count;
	if (start < 0)
		start = 0;
	for (unsigned int i = start; i < g_ChatManager.CircularChatBuffer.size();
			i++) {
		ChatMessage cm = g_ChatManager.CircularChatBuffer[i];
		Json::Value jcm;
		cm.WriteToJSON(jcm);
		Util::SafeFormat(buf, sizeof(buf), "%lu", cm.mTime);
		chat[buf] = jcm;
	}
	g_ChatManager.cs.Leave();
}

bool ChatHandler::handleAuthenticatedPost(CivetServer *server, struct mg_connection *conn) {
	std::map<std::string, std::string> parms;
	if (parseForm(server, conn, parms)) {
		if (parms.find("msg") == parms.end() || parms.find("from") == parms.end())
			writeStatus(server, conn, 403, "Forbidden", "Missing parameters.");
		else {
			std::string msg = parms["msg"];
			std::string from = parms["from"];
			std::string channel = parms.find("channel") == parms.end() ? "rc/" : parms["channel"];
			int count = parms.find("count") == parms.end() ? 20 : atoi(parms["count"].c_str());

			ChatMessage cm(msg);
			if(channel.compare("clan") == 0) {
				int cdefID = g_AccountManager.GetCDefFromCharacterName(from.c_str());
				if(cdefID != -1) {
					CharacterData *cd = g_CharacterManager.RequestCharacter(cdefID, true);
					if(cd != NULL) {
						cm.mSenderClanID = cd->clan;
					}
				}
			}

			cm.mChannelName = channel;
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender = from;
			g_ChatManager.SendChatMessage(cm, NULL);

			Json::Value root;
			WriteChat(root, count);
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	} else {
		writeStatus(server, conn, 403, "Forbidden", "Encoding not allowed.");
	}
	return true;
}

bool ChatHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {


	// Parse parameters
	std::string p;
	int count = 20;
	if (CivetServer::getParam(conn, "count", p)) {
		count = atoi(p.c_str());
	}

	Json::Value root;
	WriteChat(root, count);
	Json::StyledWriter writer;
	writeJSON200(server, conn, writer.write(root));

	return true;
}

//
// UserHandler
//
bool UserHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {


	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatus(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {
		int accountId = atoi(pathParts[pathParts.size() - 1].c_str());
		if(accountId == 0) {
			std::string username = pathParts[pathParts.size() - 1];
			Util::URLDecode(username);
			AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(username.c_str());
			if(aqd != NULL) {
				accountId = aqd->mID;
			}
		}

		AccountData *ad = NULL;
		if(accountId != 0) {
			g_AccountManager.cs.Enter("TAWApi::UserHandler::handleAuthenticatedGet");
			ad = g_AccountManager.FetchIndividualAccount(accountId);
			g_AccountManager.cs.Leave();
		}

		if(ad == NULL) {
			writeStatus(server, conn, 404, "Not found.",
					"The account could not be found.");
		}
		else {
			Json::Value root;
			ad->WriteToJSON(root);
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}


//
// CharacterHandler
//
bool CharacterHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {


	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatus(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {
		CharacterData *cd = NULL;
		int cdefID = atoi(pathParts[pathParts.size() - 1].c_str());
		if(cdefID == 0) {
			std::string characterName = pathParts[pathParts.size() - 1];
			Util::URLDecode(characterName);
			cdefID = g_AccountManager.GetCDefFromCharacterName(characterName.c_str());
		}
		if(cdefID != 0) {
			cd = g_CharacterManager.RequestCharacter(cdefID, true);
		}

		if(cd == NULL) {
			g_AccountManager.cs.Leave();
			writeStatus(server, conn, 404, "Not found.",
					"The character could not be found.");
		}
		else {
			Json::Value root;
			cd->WriteToJSON(root);
			if(cd->clan > 0) {
				Clans::Clan c = g_ClanManager.mClans[cd->clan];
				root["clanName"] = c.mName;
			}
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// UserGrovesHandler
//

bool UserGrovesHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatus(server, conn, 404, "Not found.",
				"The zone could not be found.");
	else {
		int accountId = atoi(pathParts[pathParts.size() - 2].c_str());
		if(accountId == 0) {
			AccountQuickData *aqd = g_AccountManager.GetAccountQuickDataByUsername(pathParts[pathParts.size() - 2].c_str());
			if(aqd != NULL) {
				accountId = aqd->mID;
			}
		}

		AccountData * ad = g_AccountManager.FetchIndividualAccount(accountId);
		if (ad == NULL) {
			writeStatus(server, conn, 404, "Not found.",
					"The grove could not be found.");
		} else {
			std::vector<int> groveList;
			g_ZoneDefManager.EnumerateGroveIds(accountId, 0, groveList);
			Json::Value root;
			for (std::vector<int>::iterator it = groveList.begin();
					it != groveList.end(); ++it) {
				int id = *it;
				ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(id);
				if (zone == NULL)
					g_Log.AddMessageFormat("Unknown grove %d", id);
				else {
					Json::Value grove;
					zone->WriteToJSON(grove);
					root[zone->mWarpName] = grove;
				}
			}
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// GroveHandler
//

bool ZoneHandler::handleAuthenticatedGet(CivetServer *server, struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters
	if (pathParts.size() < 1)
		writeStatus(server, conn, 404, "Not found.",
				"The zone could not be found.");
	else {
		int zoneDefId = atoi(pathParts[pathParts.size() - 1].c_str());
		ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(zoneDefId);
		if (zone == NULL)
			writeStatus(server, conn, 404, "Not found.",
					"The zone could not be found.");
		else {
			Json::Value root;
			zone->WriteToJSON(root);

			char buf[128];
			if (zone->mGrove)
				Util::SafeFormat(buf, sizeof(buf), "Grove/%d", zone->mID);
			else
				Util::SafeFormat(buf, sizeof(buf), "Scenery/%d", zone->mID);
			Platform::FixPaths(buf);

			Platform_DirectoryReader dr;
			dr.SetDirectory(buf);
			dr.ReadFiles();

			Json::Value tiles;
			int xx = 0;
			int yy = 0;
			for (std::vector<std::string>::iterator it = dr.fileList.begin();
					it != dr.fileList.end(); ++it) {
				if (sscanf((*it).c_str(), "x%03dy%03d.txt", &xx, &yy) == 2) {
					Json::Value tile;
					tile["x"] = xx;
					tile["y"] = yy;
					tiles.append(tile);
				}
			}
			root["tiles"] = tiles;

			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// SceneryHandler
//

bool SceneryHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	struct mg_request_info * req_info = mg_get_request_info(conn);

	std::string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->uri, strlen(req_info->uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);

	std::vector<std::string> pathParts;
	Util::Split(ruri, "/", pathParts);
	if (pathParts.size() < 3)
		writeStatus(server, conn, 404, "Not found.",
				"The scenery could not be found.");
	else {
		// Parse parameters
		int zoneDefId = atoi(pathParts[pathParts.size() - 3].c_str());
		int x = atoi(pathParts[pathParts.size() - 2].c_str());
		int y = atoi(pathParts[pathParts.size() - 1].c_str());

		SceneryPage *page = g_SceneryManager.GetOrCreatePage(zoneDefId, x, y);
		if(page == NULL) {
			writeStatus(server, conn, 404, "Not found.",
					"The scenery could not be found.");
		}
		else {
			Json::Value root;
			Json::Value props;
			SceneryPage::SCENERY_IT it;
			char id[10];
			for(it = page->mSceneryList.begin(); it != page->mSceneryList.end(); ++it) {
				SceneryObject *so = &it->second;
				Json::Value object;
				Util::SafeFormat(id, sizeof(id), "%d", so->ID);
				so->WriteToJSON(object);
				props[id] = object;
			}
			root["objects"] = props;
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

