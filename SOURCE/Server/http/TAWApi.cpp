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
#include "../GameConfig.h"
#include "../Creature.h"
#include "../Simulator.h"
#include "../Character.h"
#include "../Clan.h"
#include "../Guilds.h"
#include "../ZoneDef.h"
#include "../Scenery2.h"
#include "../Config.h"
#include "../Chat.h"
#include "../Instance.h"
#include "../DirectoryAccess.h"
#include "../PlayerStats.h"
#include "../Leaderboard.h"
#include "../Item.h"
#include "../json/json.h"
#include "../VirtualItem.h"
#include "../EliteMob.h"
#include "../Ability2.h"
#include "HTTPService.h"
#include <algorithm>
#include <filesystem>

namespace fs = filesystem;
using namespace HTTPD;

//
// AuthenticatedHandler
//

bool AuthenticatedHandler::handleGet(CivetServer *server,
		struct mg_connection *conn) {
	if(g_Config.APIAuthentication.length() == 0) {
		g_Logs.server->warn(
				"API request made, but 'APIAuthentication' has not be set in configuration. All API requests must be authenticated.");
		writeWWWAuthenticate(server, conn, "TAWD");
		return true;
	}
	else if (!isAuthorized(server, conn, g_Config.APIAuthentication)) {
		writeWWWAuthenticate(server, conn, "TAWD");
		return true;
	}
	return handleAuthenticatedGet(server, conn);
}

bool AuthenticatedHandler::handlePost(CivetServer *server,
		struct mg_connection *conn) {
	if (!isAuthorized(server, conn, g_Config.APIAuthentication)) {
		writeWWWAuthenticate(server, conn, "TAWD");
		return true;
	}
	return handleAuthenticatedPost(server, conn);
}

bool AuthenticatedHandler::handleAuthenticatedPost(CivetServer *server,
		struct mg_connection *conn) {
	return false;
}

//
// UpHandler
//

bool UpHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {
	Json::Value root;
	Json::StyledWriter writer;
	root["status"] = "up";
	root["currentTime"] = Json::UInt64(g_ServerTime);
	root["launchTime"] = Json::UInt64(g_ServerLaunchTime);
	root["upTime"] = Json::UInt64(g_ServerTime - g_ServerLaunchTime);
	writeJSON200(server, conn, writer.write(root));
	return true;
}

//
// WhoHandler
//

bool WhoHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {
	string response;
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
						zd->mName.c_str(), cd->Shard.c_str());
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

void CreditShopHandler::writeCreditShopItemToJSON(CS::CreditShopItem* item,
		Json::Value &c) {

	item->WriteToJSON(c);

	if (item->mItemId > 0) {
		ItemDef *itemDef = g_ItemManager.GetSafePointerByID(item->mItemId);
		if (itemDef != NULL) {
			if (item->mTitle.length() == 0)
				c["computedTitle"] = itemDef->mDisplayName;
			vector<string> l;
			Util::Split(itemDef->mIcon, "|", l);
			switch (l.size()) {
			case 1:
				c["icon1"] = "TODO";
				c["icon2"] = l[0];
				break;
			case 2:
				c["icon1"] = l[0];
				c["icon2"] = l[1];
				break;
			}
		}
	}
	if (!c.isMember("computedTitle")) {
		c["computedTitle"] = item->mTitle;
	}
}

bool CreditShopHandler::handleAuthenticatedPost(CivetServer *server,
		struct mg_connection *conn) {
	map<string, string> parms;
	if (parseForm(server, conn, parms)) {
		if (parms.find("qty") == parms.end()
				|| parms.find("item") == parms.end()
				|| parms.find("character") == parms.end())
			writeStatusPlain(server, conn, 403, "Forbidden", "Missing parameters.");
		else {
			unsigned int qty = atoi(parms["qty"].c_str());
			int csItemId = atoi(parms["item"].c_str());

			CS::CreditShopItem csItem = g_CreditShopManager.GetItem(csItemId);
			if (csItem.mId == 0) {
				writeStatusPlain(server, conn, 404, "Not found.",
						"The item could not be found.");
				return true;
			}

			string characterName = parms["character"];
			int cdefID = g_UsedNameDatabase.GetIDByName(characterName);

			g_ActiveInstanceManager.cs.Enter(
					"CreditShopHandler::handleAuthenticatedPost");
			CreatureInstance *creatureInstance =
					g_ActiveInstanceManager.GetPlayerCreatureByDefID(cdefID);
			g_ActiveInstanceManager.cs.Leave();

			CharacterData *cd = NULL;
			CharacterStatSet *css = NULL;

			if (creatureInstance != NULL) {
				cd = creatureInstance->charPtr;
				css = &creatureInstance->css;
			}
			if (cd == NULL) {
				cd = g_CharacterManager.RequestCharacter(cdefID, true);
				css = &cd->cdef.css;
			}
			if (cd == NULL) {
				writeStatusPlain(server, conn, 404, "Not found.",
						"Your character could not be found.");
				return true;
			}
			AccountData *accPtr = g_AccountManager.FetchIndividualAccount(
					cd->AccountID);
			if (accPtr == NULL) {
				writeStatusPlain(server, conn, 404, "Not found.",
						"Your account could not be found.");
				return true;
			}

			int errCode = g_CreditShopManager.ValidateItem(&csItem, accPtr, css,
					cd);

			if (errCode != CreditShopError::NONE) {
				writeStatusPlain(server, conn, 403, "Forbidden.",
						CreditShopError::GetDescription(errCode));
				return true;
			}
			ItemDef *itemDef = g_ItemManager.GetSafePointerByID(
					csItem.mItemId);
			if (itemDef == NULL) {
				writeStatusPlain(server, conn, 404, "Not found.",
						"The item could not be found.");
				return true;
			}
			if(itemDef->mActionAbilityId != 0) {
				writeStatusPlain(server, conn, 403, "Forbidden.",
						"You must be playing the game in order to be able to buy these items.");
				return true;
			}

			if (csItem.mQuantityLimit > 0 && (csItem.mQuantitySold + qty) > csItem.mQuantityLimit) {
				writeStatusPlain(server, conn, 403, "Forbidden.",
						"There are not enough of this restricted item left.");
				return true;
			}

			InventorySlot *sendSlot =
					cd->inventory.AddItem_Ex(INV_CONTAINER, itemDef->mID, csItem.mIv1 + 1);
			if (sendSlot == NULL) {
				int err = creatureInstance->charPtr->inventory.LastError;
				if (err == InventoryManager::ERROR_ITEM) {
					writeStatusPlain(server, conn, 403, "Forbidden.",
							"Server error: item does not exist.");
					return true;
				} else if (err == InventoryManager::ERROR_SPACE) {
					writeStatusPlain(server, conn, 403, "Forbidden.",
							"You do not have any free inventory space.");
					return true;
				} else if (err == InventoryManager::ERROR_LIMIT) {
					writeStatusPlain(server, conn, 403, "Forbidden.",
							"You already the maximum amount of these items.");
					return true;
				} else {
					writeStatusPlain(server, conn, 403, "Forbidden.",
							"Server error: undefined error.");
					return true;
				}
			}

			g_CharacterManager.GetThread("Simulator::MarketBuy");
			cd->pendingChanges++;

			if(creatureInstance != NULL && creatureInstance->simulatorPtr != NULL && creatureInstance->simulatorPtr->ProtocolState !=0 && creatureInstance->simulatorPtr->isConnected) {
				creatureInstance->simulatorPtr->ActivateActionAbilities(sendSlot);
			}

			if (csItem.mPriceCurrency == Currency::COPPER
					|| csItem.mPriceCurrency == Currency::COPPER_CREDITS) {
				css->copper -= csItem.mPriceCopper;
				cd->pendingChanges++;
				if(creatureInstance != NULL)
					creatureInstance->SendStatUpdate(STAT::COPPER);
			}

			if (csItem.mPriceCurrency == Currency::CREDITS
					|| csItem.mPriceCurrency == Currency::COPPER_CREDITS) {
				css->credits -= csItem.mPriceCredits;
				if (g_GameConfig.UseAccountCredits) {
					accPtr->Credits = css->credits;
					accPtr->PendingMinorUpdates++;
				}
				if(creatureInstance != NULL)
					creatureInstance->SendStatUpdate(STAT::CREDITS);
			}

			if (csItem.mQuantityLimit > 0) {
				csItem.mQuantitySold++;
				g_CreditShopManager.UpdateItem(&csItem);
			}

			if(creatureInstance != NULL && creatureInstance->simulatorPtr != NULL && creatureInstance->simulatorPtr->ProtocolState !=0 && creatureInstance->simulatorPtr->isConnected) {
				char Aux2[128];
				char SendBuf[256];
				int wpos = AddItemUpdate(SendBuf, Aux2, sendSlot);
				creatureInstance->simulatorPtr->AttemptSend(SendBuf,wpos);
			}

			g_CharacterManager.ReleaseThread();
			writeStatusPlain(server, conn, 200, "OK", "Item purchased.");
		}
	} else {
		writeStatusPlain(server, conn, 403, "Forbidden", "Encoding not allowed.");
	}
	return true;
}

bool CreditShopHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	const struct mg_request_info * req_info = mg_get_request_info(conn);
	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeStartSlash(removeEndSlash(ruri));

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	PageOptions opts;
	opts.count = 10;
	opts.Init(server, conn);

	Json::Value root;
	if (pathParts.size() > 0
			&& pathParts[pathParts.size() - 1].compare("cs") == 0) {
		int category = Category::UNDEFINED;

		// Parse parameters
		string p;
		if (CivetServer::getParam(conn, "category", p)) {
			category = Category::GetIDByName(p.c_str());
		}

		Json::Value root;
		vector<CS::CreditShopItem> allItems;
		g_CreditShopManager.GetItems(allItems);
		vector<CS::CreditShopItem> items;
		for (auto it = allItems.begin(); it != allItems.end(); ++it) {
			if (it->mCategory == category) {
				items.push_back(*it);
			}
		}

		Json::Value data;
		int didx = 0;
		for (size_t i = opts.start;
				i < opts.start + opts.count && i < items.size(); i++) {
			Json::Value jv;
			writeCreditShopItemToJSON(&items[i], jv);

			data[didx++] = jv;
		}

		root["data"] = data;
		root["total"] = Json::UInt64(items.size());

		Json::StyledWriter writer;
		writeJSON200(server, conn, writer.write(root));
	} else {
		int id = atoi(pathParts[pathParts.size() - 1].c_str());
		CS::CreditShopItem item = g_CreditShopManager.GetItem(id);
		if (item.mId == 0) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The item could not be found.");
			return true;
		}
		writeCreditShopItemToJSON(&item, root);
		Json::StyledWriter writer;
		writeJSON200(server, conn, writer.write(root));
	}

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

bool LeaderboardHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	PageOptions opts;
	opts.Init(server, conn);

	string board = "character";

	// Parse parameters
	string p;
	if (CivetServer::getParam(conn, "board", p)) {
		board = p.c_str();
	}

	Leaderboard *leaderboard = g_LeaderboardManager.GetBoard(board);
	if (leaderboard == NULL)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {

		Json::Value root;

		leaderboard->cs.Enter("LeaderboardHandler::handleAuthenticatedGet");
		vector<Leader> l(leaderboard->mLeaders);
		leaderboard->cs.Leave();

		if (opts.sort.compare("deaths") == 0)
			sort(l.begin(), l.end(), deathsSort);
		else if (opts.sort.compare("pvpKills") == 0)
			sort(l.begin(), l.end(), pvpKillsSort);
		else if (opts.sort.compare("pvpDeaths") == 0)
			sort(l.begin(), l.end(), pvpDeathsSort);
		else
			sort(l.begin(), l.end(), killsSort);

		if (opts.desc)
			reverse(l.begin(), l.end());

		Json::Value data;
		int didx = 0;
		for (size_t i = opts.start; i < opts.start + opts.count && i < l.size();
				i++) {
			Json::Value jv;
			l[i].WriteToJSON(jv);
			jv["rank"] = (int)(i + 1);
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

	vector<Clans::ClanMember> l(clan.mMembers);
	sort(l.begin(), l.end(), rankSort);

	for (vector<Clans::ClanMember>::iterator it2 = l.begin();
			it2 != l.end(); ++it2) {
		CharacterData *cd = g_CharacterManager.RequestCharacter((*it2).mID,
				true);
		if (cd != NULL) {
			char buf[12];
			Util::SafeFormat(buf, sizeof(buf), "%d", it2->mID);
			c["members"][buf]["name"] = cd->cdef.css.display_name;
			c["members"][buf]["level"] = cd->cdef.css.level;
			c["members"][buf]["profession"] = cd->cdef.css.profession;

			Json::Value stats;
			cd->PlayerStats.WriteToJSON(stats);
			total.Add(cd->PlayerStats);

			c["members"][buf]["playerStats"] = stats;

			if ((*it2).mRank == Clans::Rank::LEADER) {
				Json::Value l;
				l["name"] = cd->cdef.css.display_name;
				l["id"] = cd->cdef.CreatureDefID;
				c["leader"] = l;
			}
		} else {
			g_Logs.server->warn(
					"Clan %v (%v) contains member %v that does not exist.",
					clan.mName.c_str(), clan.mId, (*it2).mID);
		}
	}

	c["size"] = Json::UInt64(clan.mMembers.size());

	Json::Value totalStats;
	total.WriteToJSON(totalStats);
	c["playerStats"] = totalStats;
}

bool ClanHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeStartSlash(removeEndSlash(ruri));

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	PageOptions opts;
	opts.Init(server, conn);

	Json::Value root;
	if (pathParts.size() > 0
			&& pathParts[pathParts.size() - 1].compare("clans") == 0) {
		// List clans
		vector<Clans::Clan> clns = g_ClanManager.GetClans();
		for (auto it = clns.begin(); it != clns.end(); ++it) {
			Json::Value c;
			writeClanToJSON(*it, c);
			root[(*it).mName] = c;
		}
	} else {
		// TODO get clan
		string name = pathParts[pathParts.size() - 1];
		Util::URLDecode(name);
		int clanID = g_ClanManager.FindClanID(name);
		if (clanID == -1) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The clan could not be found.");
			return true;
		}
		Clans::Clan c = g_ClanManager.GetClan(clanID);
		writeClanToJSON(c, root);
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

//	vector<Clans::ClanMember> l(clan.mMembers);
//	sort(l.begin(), l.end(), rankSort);
//
//	for(vector<Clans::ClanMember>::iterator it2 = l.begin(); it2 != l.end(); ++it2) {
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

bool GuildHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeStartSlash(removeEndSlash(ruri));

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	PageOptions opts;
	opts.Init(server, conn);

	Json::Value root;
	if (pathParts.size() > 0
			&& pathParts[pathParts.size() - 1].compare("guilds") == 0) {
		// List clans
		vector<GuildDefinition> m = g_GuildManager.defList;

		for (vector<GuildDefinition>::iterator it = m.begin();
				it != m.end(); ++it) {
			Json::Value c;
			GuildDefinition guild = *it;
			writeGuildToJSON(&guild, c);
			root[guild.defName] = c;
		}
	} else {
		string name = pathParts[pathParts.size() - 1];
		Util::URLDecode(name);
		GuildDefinition *guild = g_GuildManager.FindGuildDefinition(name);
		if (guild == NULL) {
			writeStatusPlain(server, conn, 404, "Not found.",
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
	deque<ChatMessage>::iterator it;
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

bool ChatHandler::handleAuthenticatedPost(CivetServer *server,
		struct mg_connection *conn) {
	map<string, string> parms;
	if (parseForm(server, conn, parms)) {
		if (parms.find("msg") == parms.end()
				|| parms.find("from") == parms.end())
			writeStatusPlain(server, conn, 403, "Forbidden", "Missing parameters.");
		else {
			string msg = parms["msg"];
			string from = parms["from"];
			string channel =
					parms.find("channel") == parms.end() ?
							"rc/" : parms["channel"];
			int count =
					parms.find("count") == parms.end() ?
							20 : atoi(parms["count"].c_str());

			ChatMessage cm(msg);
			if (channel.compare("clan") == 0) {
				int cdefID = g_UsedNameDatabase.GetIDByName(from);
				if (cdefID != -1) {
					CharacterData *cd = g_CharacterManager.RequestCharacter(
							cdefID, true);
					if (cd != NULL) {
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
		writeStatusPlain(server, conn, 403, "Forbidden", "Encoding not allowed.");
	}
	return true;
}

bool ChatHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	// Parse parameters
	string p;
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
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {
		int accountId = atoi(pathParts[pathParts.size() - 1].c_str());
		if (accountId == 0) {
			string username = pathParts[pathParts.size() - 1];
			Util::URLDecode(username);
			accountId = g_AccountManager.GetAccountQuickDataByUsername(username).mID;
		}

		string p;
		bool detailed = false;
		CivetServer::getParam(conn, "detailed", p);
		if (p.compare("true") == 0) {
			detailed = true;
		}

		AccountData *ad = NULL;
		if (accountId != 0) {
			g_AccountManager.cs.Enter(
					"TAWApi::UserHandler::handleAuthenticatedGet");
			ad = g_AccountManager.FetchIndividualAccount(accountId);
			g_AccountManager.cs.Leave();
		}

		if (ad == NULL) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The account could not be found.");
		} else {
			Json::Value root;
			ad->WriteToJSON(root);

			if(detailed) {
				for(int i = 0 ; i < AccountData::MAX_CHARACTER_SLOTS; i++) {
					if(ad->CharacterSet[i] != 0) {
						CharacterData *cd = g_CharacterManager.RequestCharacter(ad->CharacterSet[i], true);
						if(cd != NULL) {
							Json::Value cv;
							cd->WriteToJSON(cv);
							root["characters"][cd->cdef.css.display_name]["details"] = cv;
						}
					}
				}
			}

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
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The account could not be found.");
	else {
		CharacterData *cd = NULL;
		int cdefID = atoi(pathParts[pathParts.size() - 1].c_str());
		if (cdefID == 0) {
			string characterName = pathParts[pathParts.size() - 1];
			Util::URLDecode(characterName);
			cdefID = g_UsedNameDatabase.GetIDByName(characterName);
		}
		if (cdefID != 0) {
			cd = g_CharacterManager.RequestCharacter(cdefID, true);
		}

		if (cd == NULL) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The character could not be found.");
		} else {
			Json::Value root;
			cd->WriteToJSON(root);
			if (cd->clan > 0) {
				Clans::Clan c = g_ClanManager.GetClan(cd->clan);
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
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The zone could not be found.");
	else {
		int accountId = atoi(pathParts[pathParts.size() - 2].c_str());
		if (accountId == 0) {
			accountId = g_AccountManager.GetAccountQuickDataByUsername(pathParts[pathParts.size() - 2]).mID;
		}

		AccountData * ad = g_AccountManager.FetchIndividualAccount(accountId);
		if (ad == NULL) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The grove could not be found.");
		} else {
			vector<int> groveList;
			g_ZoneDefManager.EnumerateGroveIds(accountId, 0, groveList);
			Json::Value root;
			for (vector<int>::iterator it = groveList.begin();
					it != groveList.end(); ++it) {
				int id = *it;
				ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(id);
				if (zone == NULL)
					g_Logs.server->error("Unknown grove %v", id);
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

bool ZoneHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters
	if (pathParts.size() < 1)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The zone could not be found.");
	else {
		int zoneDefId = atoi(pathParts[pathParts.size() - 1].c_str());
		ZoneDefInfo *zone = g_ZoneDefManager.GetPointerByID(zoneDefId);
		if (zone == NULL)
			writeStatusPlain(server, conn, 404, "Not found.",
					"The zone could not be found.");
		else {
			Json::Value root;
			zone->WriteToJSON(root);

			fs::path dir;
			char buf[16];
			Util::SafeFormat(buf, sizeof(buf), "%d", zone->mID);
			if (zone->mGrove) {
				// TODO read from cluster
			}
			else {
				dir = g_Config.ResolveVariableDataPath() / "Scenery" / buf;

				Json::Value tiles;
				int xx = 0;
				int yy = 0;
				for(const fs::directory_entry& entry : fs::directory_iterator(dir)) {
					auto file = entry.path();
					if (sscanf(file.string().c_str(), "x%03dy%03d.txt", &xx, &yy) == 2) {
						Json::Value tile;
						tile["x"] = xx;
						tile["y"] = yy;
						tiles.append(tile);
					}
				}
				root["tiles"] = tiles;
			}

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
	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);
	if (pathParts.size() < 3)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The scenery could not be found.");
	else {
		// Parse parameters
		int zoneDefId = atoi(pathParts[pathParts.size() - 3].c_str());
		int x = atoi(pathParts[pathParts.size() - 2].c_str());
		int y = atoi(pathParts[pathParts.size() - 1].c_str());

		SceneryPage *page = g_SceneryManager.GetOrCreatePage(zoneDefId, x, y);
		if (page == NULL) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The scenery could not be found.");
		} else {
			Json::Value root;
			Json::Value props;
			SceneryPage::SCENERY_IT it;
			char id[10];
			for (it = page->mSceneryList.begin();
					it != page->mSceneryList.end(); ++it) {
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


//
// ItemHandler
//
bool ItemHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	const struct mg_request_info * req_info = mg_get_request_info(conn);

	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeEndSlash(ruri);

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	// Parse parameters

	// Parse parameters
	if (pathParts.size() < 2)
		writeStatusPlain(server, conn, 404, "Not found.",
				"The item could not be found.");
	else {
		ItemDef *itemDef = NULL;
		int itemID = atoi(pathParts[pathParts.size() - 1].c_str());
		if (itemID == 0) {
			string itemName = pathParts[pathParts.size() - 1];
			Util::URLDecode(itemName);
			itemDef = g_ItemManager.GetSafePointerByExactName(itemName.c_str());
		}
		else {
			itemDef = g_ItemManager.GetSafePointerByID(itemID);
		}

		if (itemDef == NULL) {
			writeStatusPlain(server, conn, 404, "Not found.",
					"The item could not be found.");
		} else {
			Json::Value root;
			itemDef->WriteToJSON(root);
			Json::StyledWriter writer;
			writeJSON200(server, conn, writer.write(root));
		}
	}

	return true;
}

//
// AuctionHouseHandler
//

void AuctionHandler::writeAuctionItemToJSON(AuctionHouseItem * item,
		Json::Value &c) {
	item->WriteToJSON(c);
}

bool AuctionHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {
	char buf[256];

	const struct mg_request_info * req_info = mg_get_request_info(conn);
	string ruri;

	/* Prepare the URI */
	CivetServer::urlDecode(req_info->local_uri, strlen(req_info->local_uri), ruri, false);
	ruri = removeDoubleDotsAndDoubleSlashes(ruri);
	ruri = removeStartSlash(removeEndSlash(ruri));

	vector<string> pathParts;
	Util::Split(ruri, "/", pathParts);

	PageOptions opts;
	opts.count = 10;
	opts.Init(server, conn);

	Json::Value root;
	if (pathParts.size() > 0
			&& pathParts[pathParts.size() - 1].compare("auction") == 0) {

		AuctionHouseSearch srch;
		vector<AuctionHouseItem> ahis;
		g_AuctionHouseManager.Search(srch, ahis);
		vector<int> auctioneers;
		for(auto it = ahis.begin(); it != ahis.end(); ++it) {
			if(find(auctioneers.begin(), auctioneers.end(), it->mAuctioneer) == auctioneers.end()) {
				auctioneers.push_back(it->mAuctioneer);
			}
		}

		Json::StyledWriter writer;
		Json::Value root(Json::arrayValue);
		for(vector<int>::iterator it = auctioneers.begin(); it != auctioneers.end(); ++it) {
			CreatureDefinition *def = CreatureDef.GetPointerByCDef(*it);
			if(def != NULL) {
				Json::Value c;
				def->WriteToJSON(c);
				root.append(c);
			}
		}
		writeJSON200(server, conn, writer.write(root));
	} else {
		int id = atoi(pathParts[pathParts.size() - 1].c_str());
		if(id == 0) {
			string an = pathParts[pathParts.size() - 1];
			Util::URLDecode(an);
			CreatureDefinition *def = CreatureDef.GetPointerByName(an.c_str());
			if(def != NULL) {
				id = def->CreatureDefID;
			}
		}

		AuctionHouseSearch srch;
		vector<AuctionHouseItem> ahis;
		g_AuctionHouseManager.Search(srch, ahis);

		for(auto it = ahis.begin(); it != ahis.end(); ++it) {
			if(it->mAuctioneer == id) {
				Json::Value item;
				it->WriteToJSON(item);
				ItemDef *itemDef = g_ItemManager.GetSafePointerByID(it->mItemId);
				if(itemDef != NULL) {
					Json::Value ij;
					itemDef->WriteToJSON(ij);
					item["item"] = ij;
				}
				CharacterData *data = g_CharacterManager.RequestCharacter(it->mSeller, true);
				if(data != NULL) {
					Json::Value cj;
					cj["name"] = data->cdef.css.display_name;
					item["sellerDetail"] = cj;
				}
				Util::SafeFormat(buf, sizeof(buf),"%d", it->mId);
				root[buf] = item;
			}
		}
		g_AuctionHouseManager.cs.Leave();
		Json::StyledWriter writer;
		writeJSON200(server, conn, writer.write(root));
	}

	return true;

}


//
// DashboardHandler
//

bool DashboardHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {
	Json::Value root;
	Json::StyledWriter writer;
//	root["status"] = "up";
	writeJSON200(server, conn, writer.write(root));
	return true;
}

//
// ReloadHandler
//

bool ReloadHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	auto paths = g_Config.ResolveLocalConfigurationPath();
	for (auto it = paths.begin(); it != paths.end(); ++it) {
		auto dir = *it;
		auto filename = dir / "ServerConfig.txt";
		if(!g_Config.LoadConfig(filename) && it == paths.begin())
			g_Logs.data->error("Could not open server configuration file: %v", filename);
	}

	g_ClusterManager.GameConfigChanged("","");
	g_GameConfig.Reload();

	g_VirtualItemModSystem.LoadSettings();
	g_AbilityManager.LoadData();
	g_FileChecksum.LoadFromFile();
	g_EliteManager.LoadData();
	writeJSON200(server, conn, "");
	return true;
}


//
// ShutdownHandler
//

bool ShutdownHandler::handleAuthenticatedGet(CivetServer *server,
		struct mg_connection *conn) {

	string restart;
	if(CivetServer::getParam(conn, "restart", restart, 0)) {
		g_ExitStatus = g_Config.RestartExitStatus;
	}

	g_ServerStatus = SERVER_STATUS_STOPPED;
	writeJSON200(server, conn, "");
	return true;
}
