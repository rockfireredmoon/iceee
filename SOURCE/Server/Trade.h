#ifndef TRADE_H
#define TRADE_H

#include <vector>
#include <map>
#include "Character.h"

class CreatureInstance;

class TradePlayerData
{
public:
	TradePlayerData();
	~TradePlayerData();
	void Clear(void);

	int selfPlayerID;                      //Creature Instance ID of the player.
	TradePlayerData *otherPlayerData;      //Pointer to the other player's trade data.
	std::vector<InventorySlot> itemList;   //Items offered for trade.
	int coin;                              //Coin offered for trade.
	bool accepted;                         //The other player's offer has been accepted by this player.
	CreatureInstance *cInst;               //Pointer back to this creature.
	bool tradeWindowOpen;

	void SetAccepted(bool status);
	void SetCoin(int amount);
};

class TradeTransaction
{
public:
	TradeTransaction();
	~TradeTransaction();
	bool init;
	void Clear(void);
	TradePlayerData player[2];
	void SetPlayers(CreatureInstance *initPlayer, CreatureInstance *targetPlayer);
	TradePlayerData * GetPlayerData(int playerID);
	bool MutualAccept(void);
	//std::vector<TradeItemData> * GetItemList(int CreatureID);
};

class TradeManager
{
public:
	TradeManager();
	~TradeManager();

	std::map<int, TradeTransaction> tradeList;

	TradeTransaction * GetNewTransaction(int playerID);
	TradeTransaction * GetExistingTradeForPlayer(int playerID);
	TradeTransaction * GetExistingTransaction(int tradeID);
	int CancelTransaction(int playerID, int tradeID, char *buffer);
	void RemoveTransaction(int tradeID);

	//TradeTransaction* GetTransactionPtrByID(int CreatureID);
	void Clear(void);
};

#endif //TRADE_H