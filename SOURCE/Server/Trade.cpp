#include "Trade.h"
#include "Creature.h"

#include "Globals.h"
#include "Instance.h"

#include "StringList.h"
#include "Util.h"

TradePlayerData :: TradePlayerData()
{
	Clear();
}

TradePlayerData :: ~TradePlayerData()
{
	Clear();
}

void TradePlayerData :: Clear(void)
{
	//Note: otherPlayerData needs to be handled by the TradeTransaction class.
	selfPlayerID = 0;
	itemList.clear();
	coin = 0;
	accepted = false;
	cInst = NULL;
	tradeWindowOpen = false;
}

void TradePlayerData :: SetAccepted(bool status)
{
	accepted = status;
}

void TradePlayerData :: SetCoin(int amount)
{
	coin = amount;
}

TradeTransaction :: TradeTransaction()
{
	init = false;
	Clear();
	player[0].otherPlayerData = &player[1];
	player[1].otherPlayerData = &player[0];
}

TradeTransaction :: ~TradeTransaction()
{
	Clear();
}

void TradeTransaction :: Clear(void)
{
	init = false;
	player[0].Clear();
	player[1].Clear();

	player[0].otherPlayerData = &player[1];
	player[1].otherPlayerData = &player[0];
}

void TradeTransaction :: SetPlayers(CreatureInstance *initPlayer, CreatureInstance *targetPlayer)
{
	player[0].selfPlayerID = initPlayer->CreatureID;
	player[0].cInst = initPlayer;

	player[1].selfPlayerID = targetPlayer->CreatureID;
	player[1].cInst = targetPlayer;
}

TradePlayerData * TradeTransaction :: GetPlayerData(int playerID)
{
	if(playerID == player[0].selfPlayerID)
		return &player[0];

	if(playerID == player[1].selfPlayerID)
		return &player[1];

	return NULL;
}

bool TradeTransaction :: MutualAccept(void)
{
	return ((player[0].accepted == true) && (player[1].accepted == true));
}


/*
std::vector<TradeItemData>* TradeTransaction :: GetItemList(int playerID)
{
	//Return the item list of the corresponding player.
	if(playerID == player[0].selfPlayerID)
		return &player[0].itemList;

	if(playerID == player[1].selfPlayerID)
		return &player[1].itemList;

	return NULL;
}
*/


/*
int TradeTransaction :: GetOppositePlayerID(int ID)
{
	// Since both players share the same instance, actions by one player
	// must update the other.  This function determines the opposite
	// player.
	if(ID == originID)
		return targetID;
	else if(ID == targetID)
		return originID;

	return 0;
}


void TradeTransaction :: SetCoin(int playerID, int amount)
{
	if(playerID == originID)
		originCoin = amount;
	else if(playerID = targetID)
		targetCoin = amount;
}

void TradeTransaction :: AcceptOffer(int playerID)
{
	if(playerID == originID)
		originAccepted = true;
	else if(playerID = targetID)
		targetAccepted = true;
}
*/

TradeManager :: TradeManager()
{
}

TradeManager :: ~TradeManager()
{
	tradeList.clear();
}

void TradeManager :: Clear(void)
{
	tradeList.clear();
}

TradeTransaction * TradeManager :: GetNewTransaction(int playerID)
{
	std::map<int, TradeTransaction>::iterator it;
	TradeTransaction *ptr = &tradeList[playerID];
	if(ptr->init == false)
		ptr->Clear();
	return ptr;
}

TradeTransaction * TradeManager :: GetExistingTradeForPlayer(int playerID)
{
	std::map<int, TradeTransaction>::iterator it;
	for(it = tradeList.begin(); it != tradeList.end(); ++it)
	{
		if(it->second.player[0].selfPlayerID == playerID || it->second.player[1].selfPlayerID == playerID)
			return &it->second;
	}
	return NULL;
}

TradeTransaction * TradeManager :: GetExistingTransaction(int tradeID)
{
	//Note: transactions IDs match the creature instance ID of
	//the player who initiated the trade.
	std::map<int, TradeTransaction>::iterator it;
	it = tradeList.find(tradeID);
	if(it == tradeList.end())
		return NULL;

	return &tradeList[tradeID];
}

int TradeManager :: CancelTransaction(int playerID, int tradeID, char *buffer)
{
	//Removes a transaction from the list.  Prepares and sends a close message
	//to the players involved.

	//The return value needs to be a generic error, so the calling function can
	//easily exit by do something like "return CancelTransaction()";

	TradeTransaction *tradeData = GetExistingTransaction(tradeID);
	if(tradeData == NULL)
		return -1;

	if(playerID == tradeData->player[1].selfPlayerID)
		playerID = tradeData->player[0].selfPlayerID;

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&buffer[wpos], playerID);   //traderID
	wpos += PutByte(&buffer[wpos], TradeEventTypes::REQUEST_CLOSED);     //eventType
	wpos += PutByte(&buffer[wpos], CloseReasons::CANCELED);     //eventType
	PutShort(&buffer[1], wpos - 3);       //Set message size

	//Send to both players.
	SendToOneSimulator(buffer, wpos, tradeData->player[0].cInst->simulatorPtr);
	SendToOneSimulator(buffer, wpos, tradeData->player[1].cInst->simulatorPtr);

	//tradeData->player[0].cInst->actInst->LSendToOneSimulator(buffer, wpos, tradeData->player[0].cInst->SimulatorIndex);
	//tradeData->player[1].cInst->actInst->LSendToOneSimulator(buffer, wpos, tradeData->player[1].cInst->SimulatorIndex);

	//Remove trade data
	tradeData->player[0].cInst->activeLootID = 0;
	tradeData->player[1].cInst->activeLootID = 0;

	//Remove this trade object entirely.
	RemoveTransaction(tradeID);
	return -1;
}

/*
TradeTransaction * TradeManager :: GetTransactionPtrByID(int CreatureID)
{
	//Search for an open transaction where the given ID matches an existing
	//origin or target player.
	std::map<int, TradeTransaction>::iterator it;
	for(it = tradeList.begin(); it != tradeList.end(); ++it)
	{
		if(it->second.originID == CreatureID)
			return &it->second;
		if(it->second.targetID == CreatureID)
			return &it->second;
	}
	return NULL;
}
*/

void TradeManager :: RemoveTransaction(int tradeID)
{
	std::map<int, TradeTransaction>::iterator it;
	it = tradeList.find(tradeID);
	if(it != tradeList.end())
		tradeList.erase(it);
}
