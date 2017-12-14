#ifndef SESSION_H
#define SESSION_H

#include "Scenery2.h"
#include "Account.h"
#include "FileReader.h"
#include "Item.h"
#include "GM.h"
#include "CreditShop.h"
#include "AuctionHouse.h"
#include "Clan.h"
#include "Quest.h"
#include "util/Log.h"

int SaveSession(const char *filename)
{
	FILE *output = fopen(filename, "wb");
	if(output == NULL)
	{
		g_Logs.server->error("Could not open session config file [%v] for writing.", filename);
		return -1;
	}
	char buffer[256] = "; This file stores session settings and is automatically saved on exit.\r\n";
	fwrite(buffer, strlen(buffer), 1, output);

	fprintf(output, "SceneryAdditive=%d\r\n", g_SceneryVars.SceneryAdditive);
	fprintf(output, "NextCharacterID=%d\r\n", g_AccountManager.NextCharacterID);
	fprintf(output, "NextZoneID=%d\r\n", g_ZoneDefManager.NextZoneID);
	fprintf(output, "NextAccountID=%d\r\n", g_AccountManager.NextAccountID);
	fprintf(output, "NextVirtualItemID=%d\r\n", g_ItemManager.nextVirtualItemID);
	fprintf(output, "NextMarketItemID=%d\r\n", g_CreditShopManager.nextMarketItemID);
	fprintf(output, "NextAuctionHouseItemID=%d\r\n", g_AuctionHouseManager.nextAuctionHouseItemID);
	fprintf(output, "NextClanID=%d\r\n", g_ClanManager.nextClanID);
	fprintf(output, "NextPetitionID=%d\r\n", g_PetitionManager.NextPetitionID);
	fprintf(output, "NextVirtualQuestID=%lu\r\n", QuestDef.mVirtualQuestID);
	fprintf(output, "\r\n");
	fclose(output);
	g_Logs.server->info("Saved session file %v.", filename);
	return 0;
}

int LoadSession(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.server->error("Could not open session config file [%v] for reading.", filename);
		return -1;
	}
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			lfr.SingleBreak("=");
			lfr.BlockToString(0);
			char *NameBlock = lfr.BlockToString(0);
			if(strcmp(NameBlock, "SceneryAdditive") == 0)
				g_SceneryVars.SceneryAdditive = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextCharacterID") == 0)
				g_AccountManager.NextCharacterID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextZoneID") == 0)
				g_ZoneDefManager.NextZoneID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextAccountID") == 0)
				g_AccountManager.NextAccountID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextPetitionID") == 0)
				g_PetitionManager.NextPetitionID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextVirtualItemID") == 0)
				g_ItemManager.nextVirtualItemID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextMarketItemID") == 0)
				g_CreditShopManager.nextMarketItemID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextAuctionHouseItemID") == 0)
				g_AuctionHouseManager.nextAuctionHouseItemID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextClanID") == 0)
				g_ClanManager.nextClanID = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "NextVirtualQuestID") == 0)
				QuestDef.mVirtualQuestID = lfr.BlockToInt(1);
			else
				g_Logs.data->error("Unknown identifier [%v] in file [%v]", NameBlock, filename);
		}
	}
	lfr.CloseCurrent();
	g_Logs.server->info("Loaded session file %v.", filename);
	return 0;
}

#endif //SESSION_H
