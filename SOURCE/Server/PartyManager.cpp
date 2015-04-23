#include "PartyManager.h"
#include "ByteBuffer.h"
#include "Simulator.h"
#include "StringList.h"
#include "Util.h"
#include "Globals.h"

PartyManager g_PartyManager;

static int lootSeq = 0;

LootTag :: LootTag()
{
	Clear();
}

void LootTag :: Clear(void)
{
	slotIndex = 0;
	itemId = 0;
	creatureId = 0;
	lootCreatureId = 0;
	lootTag = 0;
	needed = false;
}

void ActiveParty :: AddMember(CreatureInstance* member)
{
	if(HasMember(member->CreatureDefID) == true)
		return;

	PartyMember newMember;
	newMember.mCreatureDefID = member->CreatureDefID;
	newMember.mCreatureID = member->CreatureID;
	newMember.mDisplayName = member->css.display_name;
	newMember.mCreaturePtr = member;
	newMember.mSocket = member->simulatorPtr->sc.ClientSocket;
	mMemberList.push_back(newMember);

	member->PartyID = mPartyID;
}

bool ActiveParty :: HasMember(int memberDefID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
		if(mMemberList[i].mCreatureDefID == memberDefID)
			return true;
	return false;
}

void ActiveParty :: RemoveMember(int memberDefID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreatureDefID == memberDefID)
		{
			if(mMemberList[i].mCreaturePtr != NULL)
				mMemberList[i].mCreaturePtr->PartyID = 0;

			mMemberList.erase(mMemberList.begin() + i);
			return;
		}
	}
}

bool ActiveParty :: UpdateLeaderDropped(int memberID)
{
	//Check if the given member is the leader.  If so, find a new leader
	//from the member list.  The leader must have already been removed from
	//the list.
	if(memberID != mLeaderID)
		return false;
	if(mMemberList.size() == 0)
		return false;
	mLeaderDefID = mMemberList[0].mCreatureDefID;
	mLeaderID = mMemberList[0].mCreatureID;
	mLeaderName = mMemberList[0].mDisplayName;
	return true;
}

bool ActiveParty :: SetLeader(int newLeaderDefID)
{
	int fi = -1;
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreatureDefID == newLeaderDefID)
		{
			fi = i;
			break;
		}
	}
	if(fi == -1)
		return false;
	mLeaderDefID = mMemberList[fi].mCreatureDefID;
	mLeaderID = mMemberList[fi].mCreatureID;
	mLeaderName = mMemberList[fi].mDisplayName;
	return true;
}

PartyMember* ActiveParty :: GetMemberByID(int memberID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
		if(mMemberList[i].mCreatureID == memberID)
			return &mMemberList[i];
	return NULL;
}

PartyMember* ActiveParty :: GetNextLooter()
{
	if(mNextToGetLoot >= mMemberList.size()) {
		mNextToGetLoot = 0;
	}
	if(mNextToGetLoot >= mMemberList.size()) {
		return NULL;
	}
	mNextToGetLoot++;
	return &mMemberList[mNextToGetLoot - 1];
}

PartyMember* ActiveParty :: GetMemberByDefID(int memberDefID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
		if(mMemberList[i].mCreatureDefID == memberDefID)
			return &mMemberList[i];
	return NULL;
}

bool ActiveParty :: UpdatePlayerReferences(CreatureInstance* member)
{
	if(mLeaderDefID == member->CreatureDefID)
		mLeaderID = member->CreatureID;
	
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreatureDefID == member->CreatureDefID)
		{
			mMemberList[i].mCreatureID = member->CreatureID;
			mMemberList[i].mCreaturePtr = member;
			mMemberList[i].mSocket = member->simulatorPtr->sc.ClientSocket;
			return true;
		}
	}
	return false;
}

bool ActiveParty :: RemovePlayerReferences(int memberDefID, bool disconnect)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreatureDefID == memberDefID)
		{
			mMemberList[i].mCreaturePtr = NULL;
			if(disconnect == true)
				mMemberList[i].mSocket = SocketClass::Invalid_Socket;
			return true;
		}
	}
	return false;
}

void ActiveParty :: RebroadCastMemberList(char *buffer)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mSocket == SocketClass::Invalid_Socket)
			continue;
		int wpos = PartyManager::WriteMemberList(buffer, this, mMemberList[i].mCreatureID);
		g_PacketManager.ExternalAddPacket(mMemberList[i].mSocket, buffer, wpos);
	}
}

void ActiveParty :: DebugDestroyParty(const char *buffer, int length)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mSocket != SocketClass::Invalid_Socket)
			g_PacketManager.ExternalAddPacket(mMemberList[i].mSocket, buffer, length);
		if(mMemberList[i].mCreaturePtr != NULL)
			mMemberList[i].mCreaturePtr->PartyID = 0;
	}
	mMemberList.clear();
}

void ActiveParty :: Disband(char *buffer)
{
	if(mMemberList.size() == 0)
		return;
	int wpos = PartyManager::WriteLeftParty(buffer);
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreaturePtr != NULL)
			mMemberList[i].mCreaturePtr->PartyID = 0;
	}
	BroadCast(buffer, wpos);
	mMemberList.clear();
}

int ActiveParty :: GetMaxPlayerLevel(void)
{
	if(mMemberList.size() == 0)
		return 0;
	int highest = 0;
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreaturePtr != NULL)
		{
			int level = mMemberList[i].mCreaturePtr->css.level;
			if(level > highest)
				highest = level;
		}
	}
	return highest;
}

void ActiveParty:: BroadcastInfoMessageToAllMembers(const char *buffer)
{
	int wpos = 0;
	char SendBuf[24576];     //Holds data that is being prepared for sending
	wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], buffer, INFOMSG_INFO);
	BroadCast(SendBuf, wpos);
}

void ActiveParty :: BroadCast(const char *buffer, int length)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
		if(mMemberList[i].mSocket != SocketClass::Invalid_Socket)
			g_PacketManager.ExternalAddPacket(mMemberList[i].mSocket, buffer, length);
}

void ActiveParty :: BroadCastExcept(const char *buffer, int length, int excludeDefID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
		if(mMemberList[i].mCreatureDefID != excludeDefID)
			if(mMemberList[i].mSocket != SocketClass::Invalid_Socket)
				g_PacketManager.ExternalAddPacket(mMemberList[i].mSocket, buffer, length);
}

void ActiveParty :: BroadCastTo(const char *buffer, int length, int creatureDefID)
{
	for(size_t i = 0; i < mMemberList.size(); i++)
	{
		if(mMemberList[i].mCreatureDefID == creatureDefID)
		{
			if(mMemberList[i].mSocket != SocketClass::Invalid_Socket)
			{
				g_PacketManager.ExternalAddPacket(mMemberList[i].mSocket, buffer, length);
				return;
			}
		}
	}
}

PartyManager :: PartyManager()
{
	nextPartyID = 1;
}

int PartyManager :: GetNextPartyID(void)
{
	return nextPartyID++;
}

ActiveParty :: ActiveParty() {
	mLeaderDefID = 0;
	mLeaderID = 0;
	mLeaderName = "";
	mPartyID = 0;
	mNextToGetLoot = 0;
	mLootFlags = 0;
	mLootMode = FREE_FOR_ALL;
	lootTags.clear();
}

ActiveParty* PartyManager :: GetPartyByLeader(int leaderDefID)
{
	for(size_t i = 0; i < mPartyList.size(); i++)
		if(mPartyList[i].mLeaderDefID == leaderDefID)
			return &mPartyList[i];
	return NULL;
}

ActiveParty* PartyManager :: GetPartyByID(int partyID)
{
	for(size_t i = 0; i < mPartyList.size(); i++)
		if(mPartyList[i].mPartyID == partyID)
			return &mPartyList[i];
	return NULL;
}

ActiveParty* PartyManager :: GetPartyWithMember(int memberDefID)
{
	for(size_t i = 0; i < mPartyList.size(); i++)
		if(mPartyList[i].HasMember(memberDefID) == true)
			return &mPartyList[i];
	return NULL;
}

ActiveParty* PartyManager :: CreateParty(CreatureInstance* leader)
{
	ActiveParty newParty;
	newParty.mLeaderDefID = leader->CreatureDefID;
	newParty.mLeaderID = leader->CreatureID;
	newParty.mLeaderName = leader->css.display_name;
	mPartyList.push_back(newParty);
	if(mPartyList.size() > 0)
		return &mPartyList.back();
	return NULL;
}

int PartyManager :: AcceptInvite(CreatureInstance* member, CreatureInstance* leader)
{
	ActiveParty *party = GetPartyByLeader(leader->CreatureDefID);
	if(party == NULL)
	{
		party = CreateParty(leader);
		if(party != NULL)
		{
			party->mPartyID = GetNextPartyID();  //leader->CreatureDefID;
			party->AddMember(leader);
			party->AddMember(member);
			party->RebroadCastMemberList(WriteBuf);
			return 1;
		}
	}
	else
	{
		party->AddMember(member);
		return 2;
	}
	return 0;

	/*
	//A player has accepted a party invitation.
	ActiveParty *party = GetPartyByLeader(leader->CreatureDefID);
	if(party == NULL)
		return 0;
	party->AddMember(member);
	return party->mPartyID;
	*/
}

void PartyManager :: BroadcastAddMember(CreatureInstance* member)
{
	//Generates a packet for a new member notification and broadcasts it to
	//all other players in the party.
	ActiveParty *party = GetPartyByID(member->PartyID);
	if(party == NULL)
		return;

	int wpos = 0;
	wpos += PutByte(&WriteBuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&WriteBuf[wpos], 0);
	wpos += PutByte(&WriteBuf[wpos], PartyUpdateOpTypes::ADD_MEMBER);
	wpos += PutInteger(&WriteBuf[wpos], member->CreatureID);
	wpos += PutStringUTF(&WriteBuf[wpos], member->css.display_name);
	PutShort(&WriteBuf[1], wpos - 3);       //Set message size

	party->BroadCastExcept(WriteBuf, wpos, member->CreatureDefID);
}

void PartyManager :: DoQuit(CreatureInstance* member)
{
	//Save the ID locally and remove the party just in case the other stuff fails.
	int PartyID = member->PartyID;
	member->PartyID = 0;

	ActiveParty *party = GetPartyByID(PartyID);
	if(party == NULL)
		return;

	party->RemoveMember(member->CreatureDefID);

	if(party->mMemberList.size() <= 1)
	{
		party->Disband(WriteBuf);
		DeletePartyByID(PartyID);
		return;
	}

	int wpos = WriteRemoveMember(WriteBuf, member->CreatureID);
	party->BroadCast(WriteBuf, wpos);

	if(party->UpdateLeaderDropped(member->CreatureID) == true)
	{
		wpos = PartyManager::WriteInCharge(WriteBuf, party);
		party->BroadCast(WriteBuf, wpos);
	}

	if(party->mMemberList.size() == 0)
		DeletePartyByID(PartyID);
}

void PartyManager :: DoRejectInvite(int leaderDefID, const char* nameDenied)
{
	/*
	ActiveParty *party = GetPartyByLeader(leaderDefID);
	if(party == NULL)
		return;
	
	int wpos = WriteRejectInvite(WriteBuf, nameDenied);
	party->BroadCastTo(WriteBuf, wpos, leaderDefID);
	*/
}

void PartyManager :: DoSetLeader(CreatureInstance *callMember, int newLeaderID)
{
	ActiveParty *party = GetPartyByID(callMember->PartyID);
	if(party == NULL)
		return;
	if(party->mLeaderID != callMember->CreatureID)
		return;

	PartyMember* member = party->GetMemberByID(newLeaderID);
	if(member == NULL)
		return;

	if(party->SetLeader(member->mCreatureDefID) == true)
	{
		int wpos = WriteInCharge(WriteBuf, party);
		party->BroadCast(WriteBuf, wpos);
	}
}

void PartyManager :: DoKick(CreatureInstance *caller, int memberID)
{
	int PartyID = caller->PartyID;

	ActiveParty *party = GetPartyByID(PartyID);
	if(party == NULL)
		return;
	if(party->mLeaderID != caller->CreatureID)
		return;

	// Note: players may be offline so a server lookup may not find them.
	// Need to resolve the member DefID from within the party data itself.

	PartyMember* member = party->GetMemberByID(memberID);
	if(member == NULL)
		return;

	int wpos = WriteRemoveMember(WriteBuf, member->mCreatureID);
	party->BroadCastExcept(WriteBuf, wpos, member->mCreatureDefID);

	wpos = WriteLeftParty(WriteBuf);
	party->BroadCastTo(WriteBuf, wpos, member->mCreatureDefID);

	party->RemoveMember(member->mCreatureDefID);

	if(party->mMemberList.size() <= 1)
	{
		party->Disband(WriteBuf);
		DeletePartyByID(PartyID);
		return;
	}

	if(party->UpdateLeaderDropped(member->mCreatureID) == true)
	{
		wpos = WriteInCharge(WriteBuf, party);
		party->BroadCast(WriteBuf, wpos);
	}

	if(party->mMemberList.size() == 0)
		DeletePartyByID(PartyID);
}

void PartyManager :: DoQuestInvite(CreatureInstance *caller, const char *questName, int questID)
{
	ActiveParty *party = GetPartyByID(caller->PartyID);
	if(party == NULL)
		return;
	int wpos = WriteQuestInvite(WriteBuf, questName, questID);
	party->BroadCastExcept(WriteBuf, wpos, caller->CreatureDefID);
}

void PartyManager :: DeletePartyByID(int partyID)
{
	for(size_t i = 0; i < mPartyList.size(); i++)
	{
		if(mPartyList[i].mPartyID == partyID)
		{
			g_Log.AddMessageFormat("Deleting party: %d", partyID);
			mPartyList.erase(mPartyList.begin() + i);
			return;
		}
	}
}

void PartyManager :: UpdatePlayerReferences(CreatureInstance* member)
{
	//Called whenever the player's zone is changed, or a player logs in.
	//Search all parties for any player matching the CreatureDefID.
	//Update the pointer, socket, and CreatureID to match.
	for(size_t i = 0; i < mPartyList.size(); i++)
	{
		if(mPartyList[i].UpdatePlayerReferences(member) == true)
		{
			member->PartyID = mPartyList[i].mPartyID;
			return;
		}
	}
}

void PartyManager :: RemovePlayerReferences(int memberDefID, bool disconnect)
{
	for(size_t i = 0; i < mPartyList.size(); i++)
		if(mPartyList[i].RemovePlayerReferences(memberDefID, disconnect) == true)
			return;
}

void PartyManager :: BroadCastPacket(int partyID, int callDefID, const char *buffer, int buflen)
{
	ActiveParty* party = GetPartyByID(partyID);
	if(party == NULL)
		return;
	party->BroadCast(buffer, buflen);
}

void PartyManager :: CheckMemberLogin(CreatureInstance* member)
{
	for(size_t i = 0; i < mPartyList.size(); i++)
	{
		if(mPartyList[i].UpdatePlayerReferences(member) == true)
		{
			member->PartyID = mPartyList[i].mPartyID;
			mPartyList[i].RebroadCastMemberList(WriteBuf);
			return;
		}
	}
}

void PartyManager :: DebugForceRemove(CreatureInstance *caller)
{
	caller->PartyID = 0;
	for(size_t i = 0; i < mPartyList.size(); i++)
	{
		if(mPartyList[i].HasMember(caller->CreatureDefID) == false)
		{
			//For debugging purposes
			if(mPartyList[i].mLeaderDefID == caller->CreatureDefID)
			{
				g_Log.AddMessageFormat("[PARTY] Empty party was removed for (%d)", caller->CreatureDefID);
				mPartyList[i].Disband(WriteBuf);
				mPartyList.erase(mPartyList.begin() + i);
				return;
			}

			continue;
		}

		mPartyList[i].RemoveMember(caller->CreatureDefID);
		if(mPartyList[i].mMemberList.size() <= 1)
		{
			mPartyList[i].Disband(WriteBuf);
			mPartyList.erase(mPartyList.begin() + i);
			return;
		}
	}		
}

int PartyManager :: PrepMemberList(char *outbuf, int partyID, int memberID)
{
	ActiveParty *party = GetPartyByID(partyID);
	if(party == NULL)
		return 0;
	return WriteMemberList(outbuf, party, memberID);
}

void PartyManager :: DebugDestroyParties(void)
{
	int wpos = WriteLeftParty(WriteBuf);
	for(size_t i = 0; i < mPartyList.size(); i++)
		mPartyList[i].DebugDestroyParty(WriteBuf, wpos);

	mPartyList.clear();
}

int PartyManager :: StrategyChange(char *outbuf, LootMode newLootMode)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::STRATEGY_CHANGE);
	wpos += PutInteger(&outbuf[wpos], newLootMode);

	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: StrategyFlagsChange(char *outbuf, int newFlags)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::STRATEGYFLAGS_CHANGE);
	wpos += PutInteger(&outbuf[wpos], newFlags);

	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}


int PartyManager :: OfferLoot(char *outbuf, int itemDefID, const char *lootTag, bool needed)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::OFFER_LOOT);
	wpos += PutStringUTF(&outbuf[wpos], lootTag);
	wpos += PutInteger(&outbuf[wpos], itemDefID);;
	wpos += PutByte(&outbuf[wpos], needed ? 1 : 0);

	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteLootRoll(char *outbuf, const char *itemDefName, char roll, const char *bidder)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::LOOT_ROLL);
	wpos += PutStringUTF(&outbuf[wpos], itemDefName);
	wpos += PutByte(&outbuf[wpos], roll);
	wpos += PutStringUTF(&outbuf[wpos], bidder);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}



LootTag * ActiveParty :: GetTag(int itemId, int creatureId)
{
	typedef std::map<int, LootTag>::iterator it_type;
	for(it_type iterator = lootTags.begin(); iterator != lootTags.end(); ++iterator) {
		if(iterator->second.itemId == itemId && iterator->second.creatureId == creatureId) {
			return &iterator->second;
		}
	}
	return NULL;
}

void ActiveParty :: RemoveTagsForLootCreatureId(int lootCreatureId, int itemId)
{
	std::map<int, LootTag>::iterator itr = lootTags.begin();
	while (itr != lootTags.end()) {
		if (itr->second.lootCreatureId == lootCreatureId && itr->second.itemId == itemId) {
			std::map<int, LootTag>::iterator toErase = itr;
			++itr;
			lootTags.erase(toErase);
		} else {
			++itr;
		}
	}
}

bool ActiveParty:: HasTags(int itemId)
{
	typedef std::map<int, LootTag>::iterator it_type;
	for(it_type iterator = lootTags.begin(); iterator != lootTags.end(); ++iterator) {
		if(iterator->second.itemId == itemId) {
			return true;
		}
	}
	return false;
}

void ActiveParty :: RemoveCreatureTags(int itemId, int creatureId)
{
	std::map<int, LootTag>::iterator itr = lootTags.begin();
	while (itr != lootTags.end()) {
		if (itr->second.itemId == itemId && itr->second.creatureId == creatureId) {
			std::map<int, LootTag>::iterator toErase = itr;
			++itr;
			lootTags.erase(toErase);
		} else {
			++itr;
		}
	}
}

LootTag ActiveParty :: TagItem(int itemId, int creatureId, int lootCreatureId)
{
	LootTag tag;
	tag.lootTag = ++lootSeq;
	tag.creatureId = creatureId;
	tag.itemId = itemId;
	tag.lootCreatureId = lootCreatureId;
	lootTags[tag.lootTag] = tag;
	g_Log.AddMessageFormat("Tagged item %d for loot creature %d to creature %d. Tag is %d", tag.itemId, tag.lootCreatureId, tag.creatureId, tag.lootTag);
	return tag;
}

int PartyManager :: WriteLootWin(char *outbuf, const char *lootTag, const char *originalTag, const char *winner, int creatureId, int slotIndex)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::LOOT_WIN);
	wpos += PutStringUTF(&outbuf[wpos], lootTag);
	wpos += PutStringUTF(&outbuf[wpos], originalTag);
	wpos += PutStringUTF(&outbuf[wpos], winner);
	char buf[34];
	Util::SafeFormat(buf, sizeof(buf), "%d:%d", creatureId, slotIndex);
	wpos += PutStringUTF(&outbuf[wpos], buf);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteInvite(char *outbuf, int leaderId, const char *leaderName)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::INVITE);
	
	wpos += PutInteger(&outbuf[wpos], leaderId);
	wpos += PutStringUTF(&outbuf[wpos], leaderName);

	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteProposeInvite(char *outbuf, int proposeeId, const char *proposeeName, int proposerId, const char *proposerName)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::PROPOSE_INVITE);
	
	wpos += PutInteger(&outbuf[wpos], proposeeId);
	wpos += PutStringUTF(&outbuf[wpos], proposeeName);

	wpos += PutInteger(&outbuf[wpos], proposerId);
	wpos += PutStringUTF(&outbuf[wpos], proposerName);

	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteMemberList(char *outbuf, ActiveParty *party, int memberID)
{
	//memberID is the creature ID that is requesting the list.
	//The client needs this to determine if it's leading the party.
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);

	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::JOINED_PARTY);

	//wpos += PutByte(&outbuf[wpos], party->mMemberList.size());
	int countLoc = wpos;
	wpos += PutByte(&outbuf[wpos], 0);
	wpos += PutInteger(&outbuf[wpos], party->mLeaderID);
	wpos += PutInteger(&outbuf[wpos], memberID);  //memberId

	int count = 0;
	for(size_t i = 0; i < party->mMemberList.size(); i++)
	{
		if(party->mMemberList[i].mCreatureID != memberID)
		{
			wpos += PutInteger(&outbuf[wpos], party->mMemberList[i].mCreatureID);
			wpos += PutStringUTF(&outbuf[wpos], party->mMemberList[i].mDisplayName.c_str());
			count++;
		}
	}
	PutByte(&outbuf[countLoc], count);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteLeftParty(char *outbuf)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);
	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::LEFT_PARTY);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteRemoveMember(char *outbuf, int memberID)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);
	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::REMOVE_MEMBER);
	wpos += PutInteger(&outbuf[wpos], memberID);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteInCharge(char *outbuf, ActiveParty *party)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);
	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::IN_CHARGE);
	wpos += PutInteger(&outbuf[wpos], party->mLeaderID);
	wpos += PutStringUTF(&outbuf[wpos], party->mLeaderName.c_str());
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteQuestInvite(char *outbuf, const char* questName, int questID)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);
	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::QUEST_INVITE);
	wpos += PutStringUTF(&outbuf[wpos], questName);
	wpos += PutInteger(&outbuf[wpos], questID);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}

int PartyManager :: WriteRejectInvite(char *outbuf, const char *memberDenied)
{
	int wpos = 0;
	wpos += PutByte(&outbuf[wpos], 6);     //_handlePartyUpdateMsg
	wpos += PutShort(&outbuf[wpos], 0);
	wpos += PutByte(&outbuf[wpos], PartyUpdateOpTypes::INVITE_REJECTED);
	wpos += PutStringUTF(&outbuf[wpos], memberDenied);
	PutShort(&outbuf[1], wpos - 3);       //Set message size
	return wpos;
}
	
