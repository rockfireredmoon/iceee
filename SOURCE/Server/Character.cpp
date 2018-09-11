#include <vector>
#include <algorithm>   //For sort
#include "Character.h"
#include "FileReader.h"
#include "Config.h"       //For global variables holding default positions
   //To report errors
#include "ItemSet.h"
#include "Info.h"
#include "Quest.h"
#include <limits>
#include "Account.h"  //For upgrade conversion only.
#include "DebugTracer.h"
#include "Ability2.h"
#include "Util.h"
#include "Debug.h"
#include "Globals.h"
#include "PVP.h"
#include "Cluster.h"
#include "StringUtil.h"
#include "InstanceScale.h"
#include "Inventory.h"
#include "DirectoryAccess.h"
#include "util/SquirrelObjects.h"
#include "util/JsonHelpers.h"
#include "util/Log.h"

const int SERVER_CHARACTER_VERSION = 1;

//vector<CharacterData> CharList;
CharacterManager g_CharacterManager;

AbilityContainer :: AbilityContainer()
{
}

AbilityContainer :: ~AbilityContainer()
{
	AbilityList.clear();
}

int AbilityContainer :: GetAbilityIndex(int value)
{
	int a;
	for(a = 0; a < (int)AbilityList.size(); a++)
		if(AbilityList[a] == value)
			return a;

	return -1;
}

int AbilityContainer :: AddAbility(int value)
{
	int r = GetAbilityIndex(value);
	if(r == -1)
	{
		AbilityList.push_back(value);
		r = AbilityList.size() - 1;
	}
	return r;
}

IntContainer :: IntContainer()
{
}

IntContainer :: ~IntContainer()
{
	List.clear();
}

int IntContainer :: GetIndex(int value)
{
	int a;
	for(a = 0; a < (int)List.size(); a++)
		if(List[a] == value)
			return a;

	return -1;
}

int IntContainer :: Add(int value)
{
	int r = GetIndex(value);
	if(r == -1)
	{
		List.push_back(value);
		r = List.size() - 1;
	}
	return r;
}

int IntContainer :: Remove(int value)
{
	int r = GetIndex(value);
	if(r >= 0)
	{
		List.erase(List.begin() + r);
		return 1;
	}

	return 0;
}

void IntContainer :: Clear(void)
{
	List.clear();
}

PreferenceContainer :: PreferenceContainer()
{
}

PreferenceContainer :: ~PreferenceContainer()
{
	PrefList.clear();
}

int PreferenceContainer :: GetPrefIndex(const char *name)
{
	for(size_t a = 0; a < PrefList.size(); a++)
		if(PrefList[a].name.compare(name) == 0)
			return a;

	return -1;
}

const char * PreferenceContainer :: GetPrefValue(const char *name)
{
	int r = GetPrefIndex(name);
	if(r >= 0)
		return PrefList[r].value.c_str();

	return NULL;
}

int PreferenceContainer :: SetPref(const char *name, const char *value)
{
	int r = GetPrefIndex(name);
	if(r >= 0)
	{
		PrefList[r].value = value;
	}
	else
	{
		_PreferencePair newPref;
		newPref.name = name;
		newPref.value = value;
		PrefList.push_back(newPref);
		r = PrefList.size() - 1;
	}

	return r;
}

CharacterData :: CharacterData()
{
	ClearAll();
}

CharacterData :: ~CharacterData()
{
	abilityList.AbilityList.clear();
	preferenceList.PrefList.clear();
	SidekickList.clear();
	originalAppearance.clear();
}

void CharacterData :: ClearAll(void)
{
	AccountID = 0;
	characterVersion = 0;
	pendingChanges = 0;
	expireTime = 0;
	clan = 0;

	cdef.css.Clear();
	memset(&activeData, 0, sizeof(activeData));
	StatusText.clear();
	PrivateChannelName.clear();
	PrivateChannelPassword.clear();
	abilityList.AbilityList.clear();
	preferenceList.PrefList.clear();
	friendList.clear();
	guildList.clear();
	SidekickList.clear();
	MaxSidekicks = MAX_SIDEKICK;
	Mode = PVP::GameMode::PVE;

	//eq.slotList.clear();
	//inv.slotList.clear();
	inventory.ClearAll();
	localCharacterVault = true;

	hengeList.clear();

	SecondsLogged = 0;
	CreatedTimeSec = 0;
	SessionsLogged = 0;
	TimeLogged = "";
	LastSession = "";
	LastLogOn = "";
	LastLogOff = "";
	Shard = "";

	questJournal.Clear();

	InstanceScaler.clear();

	memset(groveReturnPoint, 0, sizeof(groveReturnPoint));
	memset(bindReturnPoint, 0, sizeof(bindReturnPoint));
	memset(pvpReturnPoint, 0, sizeof(pvpReturnPoint));
	LastWarpTime = 0;
	UnstickCount = 0;
	LastUnstickTime = 0;

	memset(PermissionSet, 0, sizeof(PermissionSet));

	CurrentVaultSize = 0;
	CreditsPurchased = 0;
	CreditsSpent = 0;
	ExtraAbilityPoints = 0;

	PlayerStats.Clear();
}

void CharacterData :: CopyFrom(CharacterData &source)
{
	//Don't need to copy base stuff, so just set to zero
	activeData.CopyFrom(&source.activeData);
	cdef.Clear();

	MaxSidekicks = source.MaxSidekicks;
	Mode = source.Mode;

	//Assign the vector lists
	abilityList.AbilityList.assign(source.abilityList.AbilityList.begin(), source.abilityList.AbilityList.end());
	preferenceList.PrefList.assign(source.preferenceList.PrefList.begin(), source.preferenceList.PrefList.end());
}



bool CharacterData :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key("CharacterData", StringUtil::Format("%d", cdef.CreatureDefID), true);
	return true;
}

bool CharacterData :: ReadEntity(AbstractEntityReader *reader) {
	if(!reader->Exists())
		return false;

	AccountID = reader->ValueInt("AccountID");
	characterVersion = reader->ValueInt("CharacterVersion");
	activeData.CurInstance = reader->ValueInt("Instance");
	clan = reader->ValueInt("Clan");
	activeData.CurZone = reader->ValueInt("Zone");
	if(activeData.CurZone <= 0)
		activeData.CurZone = g_InfoManager.GetStartZone();
	activeData.CurX = reader->ValueInt("X");
	if(activeData.CurX == -1)
		activeData.CurX = g_InfoManager.GetStartX();
	activeData.CurY = reader->ValueInt("Y");
	if(activeData.CurY == -1)
		activeData.CurY = g_InfoManager.GetStartY();
	activeData.CurZ = reader->ValueInt("Z");
	if(activeData.CurZ == -1)
		activeData.CurZ = g_InfoManager.GetStartZ();
	activeData.CurRotation = reader->ValueInt("Rotation");
	StatusText = reader->Value("StatusText");
	SecondsLogged = reader->ValueULong("SecondsLogged");
	CreatedTimeSec = reader->ValueULong("CreatedTime");
	SessionsLogged = reader->ValueULong("SessionsLogged");
	TimeLogged = reader->Value("TimeLogged");
	LastSession = reader->Value("LastSession");
	LastLogOn = reader->Value("LastLogOn");
	LastLogOff = reader->Value("LastLogOff");
	Shard = reader->Value("Shard");
	originalAppearance = reader->Value("OriginalAppearance");
	originalAppearance = reader->Value("OriginalAppearance");
	CurrentVaultSize = reader->ValueInt("CurrentVaultSize");
	CreditsPurchased = reader->ValueInt("CreditsPurchased");
	CreditsSpent = reader->ValueInt("CreditsSpent");
	ExtraAbilityPoints = reader->ValueInt("ExtraAbilityPoints");
	STRINGLIST l;
	Util::Split(reader->Value("GroveReturn"), ",", l);
	if(l.size() > 3) {
		groveReturnPoint[0] = atoi(l[0].c_str());
		groveReturnPoint[1] = atoi(l[1].c_str());
		groveReturnPoint[2] = atoi(l[2].c_str());
		groveReturnPoint[3] = atoi(l[3].c_str());
	}
	l.clear();
	Util::Split(reader->Value("BindReturn"), ",", l);
	if(l.size() > 3) {
		bindReturnPoint[0] = atoi(l[0].c_str());
		bindReturnPoint[1] = atoi(l[1].c_str());
		bindReturnPoint[2] = atoi(l[2].c_str());
		bindReturnPoint[3] = atoi(l[3].c_str());
	}
	LastWarpTime = reader->ValueULong("LastWarpTime");
	UnstickCount = reader->ValueInt("UnstickCount");
	LastUnstickTime = reader->ValueULong("LastUnstickTime");

	l = reader->ListValue("HengeList", ",");
	for(auto a = l.begin(); a != l.end(); ++a)
		HengeAdd(atoi((*a).c_str()));

	l = reader->ListValue("Abilities", ",");
	for(auto a = l.begin(); a != l.end(); ++a) {
		abilityList.AddAbility(atoi((*a).c_str()));
	}

	l = reader->ListValue("FriendList");
	for(auto a = l.begin(); a != l.end(); ++a) {
		STRINGLIST l2;
		Util::Split(*a, ",", l2);
		if(l2.size() > 1) {
			int CDefID = atoi(l2[0].c_str());
			if(CDefID != 0 && l2[1].length() > 0) {
				AddFriend(CDefID, l2[1].c_str());
			}
		}
	}
	l = reader->ListValue("GuildList");
	for(auto a = l.begin(); a != l.end(); ++a) {
		STRINGLIST l2;
		Util::Split(*a, ",", l2);
		if(l2.size() > 1) {
			int GuildDefID = atoi(l2[0].c_str());
			int valour = atoi(l2[1].c_str());
			if(GuildDefID != 0) {
				JoinGuild(GuildDefID);
				AddValour(GuildDefID, valour);
			}
		}
	}
	l = reader->ListValue("Sidekick");
	for(auto a = l.begin(); a != l.end(); ++a) {
		STRINGLIST l2;
		Util::Split(*a, ",", l2);
		if(l2.size() > 2) {
			SidekickObject skobj;
			skobj.CDefID = atoi(l2[0].c_str());
			skobj.summonType = atoi(l2[1].c_str());
			skobj.summonParam = atoi(l2[2].c_str());
			SidekickList.push_back(skobj);
		}
	}
	MaxSidekicks = reader->ValueInt("MaxSidekicks");
	Mode = reader->ValueInt("Mode");
	PrivateChannelName = reader->Value("PrivateChannelName");
	PrivateChannelPassword = reader->Value("PrivateChannelPassword");
	InstanceScaler = reader->Value("InstanceScaler");

	PlayerStats.ReadEntity(reader);

	STRINGLIST perms = reader->ListValue("Permissions", ",");
	for(auto a = perms.begin(); a != perms.end(); ++a) {
		if(SetPermission(Perm_Account, StringUtil::LowerCase(*a).c_str(), true) == false)
			g_Logs.data->warn("Unknown permission identifier [%v] in Character %v.", cdef.CreatureDefID);
	}

	//
	// Stats
	//
	reader->Section("STATS");
	STRINGLIST keys = reader->Keys();
	for(auto a = keys.begin(); a != keys.end(); ++a) {
		int res = WriteStatToSetByName(*a, reader->Value(*a), &cdef.css);
		if(res == -1)
			g_Logs.data->warn("Unknown stat name in Character list: %v", *a);
	}

	//
	// Preferences
	//
	reader->Section("PREFS");
	keys = reader->Keys();
	for(auto a = keys.begin(); a != keys.end(); ++a)
		preferenceList.SetPref((*a).c_str(), reader->Value((*a)).c_str());

	//
	// Inventory
	//
	reader->Section("INV");
	keys = reader->Keys();
	for(auto i = keys.begin(); i != keys.end(); ++i) {
		STRINGLIST l = reader->ListValue(*i);
		for(std::vector<std::string>::iterator a = l.begin(); a != l.end(); ++a) {
			std::string v = *a;
			if(ReadInventory(StringUtil::LowerCase(*i), v, inventory, "CharacterData",  cdef.css.display_name, "Character") == -2) {
				g_Logs.data->warn("Character [%v] invalid inventory item [%v]", cdef.css.display_name, v);
			}
		}
	}

	//
	// Quest
	//
	reader->Section("QUEST");
	keys = reader->Keys();
	for(auto i = keys.begin(); i != keys.end(); ++i) {
		STRINGLIST l = reader->ListValue(*i);
		for(auto a = l.begin(); a != l.end(); ++a) {
			STRINGLIST l;
			Util::Split(*a, ",", l);

			QuestReference newItem;
			newItem.Reset();

			int complete, count;
			if((*i) == "active") {
				if(l.size() > 1) {
					// Index: 0   1  2   3        4         5        6         7        8
					//    active=id,act,obj1comp,obj1count,obj2comp,obj2count,obj3comp,obj3count
					newItem.QuestID = atoi(l[0].c_str());
					newItem.CurAct = atoi(l[1].c_str());
					int o = 0;
					for(size_t a = 2; a < l.size() - 1; a++)
					{
						complete = atoi(l[a++].c_str());
						count = atoi(l[a].c_str());
						newItem.ObjComplete[o] = complete;
						newItem.ObjCounter[o] = count;
						o++;
					}
					questJournal.activeQuests.AddItem(newItem);
				}
				else {
					g_Logs.data->warn("Invalid quest active spec [%v] in character [%v]", *a, cdef.CreatureDefID);
				}
			}
			else if((*i) == "complete") {
				for(size_t a = 0; a < l.size(); a++) {
					newItem.QuestID = atoi(l[a].c_str());
					questJournal.completedQuests.AddItem(newItem);
				}
			}
			else if((*i) == "repeat") {
				if(l.size() > 2) {
					int ID = atoi(l[0].c_str());
					unsigned long startMinute = strtoul(l[1].c_str(), NULL, 0);
					unsigned long waitMinute = strtoul(l[2].c_str(), NULL, 0);
					questJournal.AddQuestRepeatDelay(ID, startMinute, waitMinute);
				}
				else {
					g_Logs.data->warn("Invalid quest repeat spec [%v] in character [%v]", *a, cdef.CreatureDefID);
				}
			}
		}
	}

	//
	// Cooldown
	//
	reader->Section("COOLDOWN");
	keys = reader->Keys();
	for(auto i = keys.begin(); i != keys.end(); ++i) {
		STRINGLIST l = reader->ListValue(*i);
		for(auto a = l.begin(); a != l.end(); ++a) {

			STRINGLIST l;
			Util::Split(*a, ",", l);

			if(l.size() > 1) {
				long remain = strtoul(l[0].c_str(), NULL, 0);  //For these, don't use the copy form so it doesn't overwrite the name block.
				long elapsed = strtoul(l[1].c_str(), NULL, 0);
				if(remain < 0)
					remain = 0;
				if(elapsed < 0)
					elapsed = 0;

				if(remain > 0) {
					cooldownManager.LoadEntry((*i).c_str(), remain, elapsed);
				}
			}
			else {
				g_Logs.data->warn("Invalid cooldown spec [%v] in character [%v]", *a, cdef.CreatureDefID);
			}
		}
	}

	//
	// Abilities
	//
	if(g_Config.PersistentBuffs) {
		reader->Section("ABILITIES");
		keys = reader->Keys();
		for(auto i = keys.begin(); i != keys.end(); ++i) {
			STRINGLIST l = reader->ListValue(*i);
			for(auto a = l.begin(); a != l.end(); ++a) {

				STRINGLIST l;
				Util::Split(*a, ",", l);

				if(l.size() > 4) {
					//Expected formats:
						//  Ability=tier,buffType,ability ID,ability group ID,remain

					unsigned char tier = atoi(l[0].c_str());
					unsigned char buffType = atoi(l[1].c_str());
					short abID = atoi(l[2].c_str());
					short abgID = atoi(l[3].c_str());
					double remainS = (double)strtoul(l[4].c_str(), NULL, 0) / 1000.0;
					buffManager.AddPersistentBuff(tier, buffType, abID, abgID, remainS);
				}
				else {
					g_Logs.data->warn("Invalid abilities spec [%v] in character [%v]", *a, cdef.CreatureDefID);
				}
			}
		}
	}

	return true;
}

bool CharacterData :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key("CharacterData", StringUtil::Format("%d", cdef.CreatureDefID));
	writer->Value("characterVersion", characterVersion);
	writer->Value("AccountID", AccountID);
	writer->Value("Clan", clan);
	writer->Value("Instance", activeData.CurInstance);
	writer->Value("Zone", activeData.CurZone);
	writer->Value("X", activeData.CurX);
	writer->Value("Y", activeData.CurY);
	writer->Value("Z", activeData.CurZ);
	writer->Value("Rotation", activeData.CurRotation);
	writer->Value("StatusText", StatusText);
	writer->Value("InstanceScaler", InstanceScaler);
	writer->Value("PrivateChannelName", PrivateChannelName);
	writer->Value("PrivateChannelPassword", PrivateChannelPassword);
	writer->Value("CreatedTime", CreatedTimeSec);
	writer->Value("SecondsLogged", SecondsLogged);
	writer->Value("SessionsLogged", SessionsLogged);
	writer->Value("TimeLogged", TimeLogged);
	writer->Value("LastSession", LastSession);
	writer->Value("LastLogOn", LastLogOn);
	writer->Value("LastLogOff", LastLogOff);
	writer->Value("Shard", Shard);
	writer->Value("OriginalAppearance", originalAppearance);
	writer->Value("CurrentVaultSize", CurrentVaultSize);
	writer->Value("CreditsPurchased", CreditsPurchased);
	writer->Value("CreditsSpent", CreditsSpent);
	writer->Value("ExtraAbilityPoints", ExtraAbilityPoints);
	writer->Value("GroveReturn", StringUtil::Format("%d,%d,%d,%d", groveReturnPoint[0], groveReturnPoint[1], groveReturnPoint[2], groveReturnPoint[3]));
	writer->Value("BindReturn", StringUtil::Format("%d,%d,%d,%d", bindReturnPoint[0], bindReturnPoint[1], bindReturnPoint[2], bindReturnPoint[3]));
	writer->Value("LastWarpTime", LastWarpTime);
	writer->Value("UnstickCount", UnstickCount);
	writer->Value("LastUnstickTime", LastUnstickTime);

	PlayerStats.WriteEntity(writer);
	STRINGLIST l;
	for(auto a = hengeList.begin(); a != hengeList.end(); ++a)
		l.push_back(StringUtil::Format("%d", *a));
	writer->ListValue("HengeList", l);

	l.clear();
	for(int a = 0; a < MaxPermissionDef; a++)
		if((PermissionSet[PermissionDef[a].index] & PermissionDef[a].flag) == PermissionDef[a].flag)
			l.push_back(PermissionDef[a].name);
	writer->ListValue("Permissions", l);

	l.clear();
	for(auto a = abilityList.AbilityList.begin(); a != abilityList.AbilityList.end(); ++a)
		l.push_back(StringUtil::Format("%d", *a));
	writer->ListValue("Abilities", l);

	l.clear();
	for(auto a = guildList.begin(); a != guildList.end(); ++a)
		l.push_back(StringUtil::Format("%d,%d", (*a).GuildDefID, (*a).Valour));
	writer->ListValue("GuildList", l);

	l.clear();
	for(auto a = friendList.begin(); a != friendList.end(); ++a)
		l.push_back(StringUtil::Format("%d,%s", (*a).CDefID, (*a).Name.c_str()));
	writer->ListValue("FriendList", l);

	writer->Value("MaxSidekicks", MaxSidekicks);
	writer->Value("Mode", Mode);

	l.clear();
	for(auto a = SidekickList.begin(); a != SidekickList.end(); ++a)
		l.push_back(StringUtil::Format("%d,%d,%d", (*a).CDefID, (*a).summonType, (*a).summonParam));
	writer->ListValue("Sidekicks", l);

	//
	// Stats
	//
	writer->Section("STATS");
	for(int a = 0; a < NumStats; a++)
		if(isStatZero(a, &cdef.css) == false)
			WriteStatToEntity(a, &cdef.css, writer);
	//
	// Preferences
	//
	writer->Section("PREFS");
	for(auto a = preferenceList.PrefList.begin(); a != preferenceList.PrefList.end(); ++a)
		writer->Value((*a).name, (*a).value);

	//
	// Inventory
	//
	writer->Section("INV");
	for(int a = 0; a < MAXCONTAINER; a++) {
		if(IsContainerIDValid(a)) {
			STRINGLIST l;
			for(int b = 0; b < (int)inventory.containerList[a].size(); b++) {
				InventorySlot *slot = &inventory.containerList[a][b];

				std::string s = StringUtil::Format("%lu,%d", slot->CCSID & CONTAINER_SLOT,
						slot->IID );

				if(slot->count > 0 || slot->customLook != 0 || slot->bindStatus != 0 || slot->secondsRemaining != -1)
					s += StringUtil::Format(",%d,%d,%d,%ld", slot->count, slot->customLook, slot->bindStatus, slot->AdjustTimes());

				l.push_back(s);
			}
			writer->ListValue(GetContainerNameFromID(a), l);
		}
	}

	//
	// Quests
	//
	writer->Section("QUEST");
	l.clear();
	for(auto a = questJournal.activeQuests.itemList.begin(); a != questJournal.activeQuests.itemList.end() ; ++a) {
		QuestReference &qref = *a;
		std::string s = StringUtil::Format("%d,%d", qref.QuestID, qref.CurAct);
		for(int b = 0; b < MAXOBJECTIVES; b++) 		{
			int comp = qref.ObjComplete[b];
			int count = qref.ObjCounter[b];
			s += StringUtil::Format(",%d,%d", comp, count);
		}
		l.push_back(s);
	}
	if(l.size() > 0)
		writer->ListValue("active", l);

	l.clear();
	for(auto a = questJournal.completedQuests.itemList.begin(); a != questJournal.completedQuests.itemList.end() ; ++a) {
		l.push_back(StringUtil::Format("%d", (*a).QuestID));
	}
	if(l.size() > 0)
		writer->ListValue("complete", l);

	l.clear();
	for(auto a = questJournal.delayedRepeat.begin(); a != questJournal.delayedRepeat.end() ; ++a) {
		l.push_back(StringUtil::Format("%d,%lu,%lu", (*a).QuestID, (*a).StartTimeMinutes, (*a).WaitTimeMinutes));
	}
	if(l.size() > 0)
		writer->ListValue("repeat", l);

	//
	// Cooldown
	//
	writer->Section("COOLDOWN");
	cooldownManager.WriteEntity(writer);

	//
	// Abilities
	//
	if(g_Config.PersistentBuffs) {
		writer->Section("ABILITIES");
		buffManager.WriteEntity(writer);
	}

	return true;
}

void CharacterData :: EraseExpirationTime(void)
{
	expireTime = 0;
}

void CharacterData :: SetExpireTime(void)
{
	expireTime = g_ServerTime + CharacterManager::TEMP_EXPIRE_TIME;
}

void CharacterData :: ExtendExpireTime(void)
{
	if(expireTime != 0)
		SetExpireTime();
}

void CharacterData :: AddValour(int GuildDefID, int valour)
{
	for(size_t i = 0; i < guildList.size(); i++) {
		if(guildList[i].GuildDefID == GuildDefID) {
			guildList[i].Valour += valour;
			return;
		}
	}
}

int CharacterData :: GetValour(int GuildDefID)
{
	for(size_t i = 0; i < guildList.size(); i++) {
		if(guildList[i].GuildDefID == GuildDefID) {
			return guildList[i].Valour;
		}
	}
	return 0;
}

bool CharacterData :: IsInGuildAndHasValour(int GuildDefID, int valour) {
	for(size_t i = 0; i < guildList.size(); i++)
		if(guildList[i].GuildDefID == GuildDefID && guildList[i].Valour >= valour)
			return true;
	return false;
}

void CharacterData :: LeaveGuild(int GuildDefID)
{
	for(size_t i = 0; i < guildList.size(); i++) {
		if(guildList[i].GuildDefID == GuildDefID) {
			guildList.erase(guildList.begin() + i);
			break;
		}
	}
	OnRankChange(0);
}

void CharacterData :: JoinGuild(int GuildDefID)
{
	for(size_t i = 0; i < guildList.size(); i++)
		if(guildList[i].GuildDefID == GuildDefID)
			return;

	GuildListObject newObject;
	newObject.GuildDefID = GuildDefID;
	guildList.push_back(newObject);
}

void CharacterData :: AddFriend(int CDefID, const char *name)
{
	for(size_t i = 0; i < friendList.size(); i++)
		if(friendList[i].CDefID == CDefID)
			return;

	FriendListObject newObject;
	newObject.CDefID = CDefID;
	newObject.Name = name;
	friendList.push_back(newObject);
}

int CharacterData :: RemoveFriend(const char *name)
{
	for(size_t i = 0; i < friendList.size(); i++)
	{
		if(friendList[i].Name.compare(name) == 0)
		{
			friendList.erase(friendList.begin() + i);
			return 1;
		}
	}
	return 0;
}

int CharacterData :: GetFriendIndex(int CDefID)
{
	for(size_t a = 0; a < friendList.size(); a++)
		if(friendList[a].CDefID == CDefID)
			return a;

	return -1;
}

void CharacterData :: UpdateBaseStats(CreatureInstance *destData, bool setStats)
{
	//if setStats is true, it communicates back the adjusted base stats to the
	//Creature Instance, allowing external functions to update buffs if necessary,
	//such as when a character levels.

	CharacterStatSet *css;
	if(destData != NULL)
		css = &destData->css;
	else
		css = &cdef.css;

	int level = css->level;
	if(level < 0)
		level = 0;
	else if(level > MAX_LEVEL)
		level = MAX_LEVEL;

	int prof = css->profession;
	if(prof < 0)
		prof = 0;
	else if(prof > MAX_PROFESSION)
		prof = MAX_PROFESSION;

	/*
	equipStat[Stat_Str][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Str]];
	equipStat[Stat_Dex][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Dex]];
	equipStat[Stat_Con][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Con]];
	equipStat[Stat_Psy][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Psy]];
	equipStat[Stat_Spi][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Spi]];
	sprintf(css->base_stats, "%d,%d,%d,%d,%d",
		equipStat[Stat_Str][Stat_Base],
		equipStat[Stat_Dex][Stat_Base],
		equipStat[Stat_Con][Stat_Base],
		equipStat[Stat_Psy][Stat_Base],
		equipStat[Stat_Spi][Stat_Base] );
	*/

	if(setStats == true)
	{
		/*
		css->strength = equipStat[Stat_Str][Stat_Base];
		css->dexterity = equipStat[Stat_Dex][Stat_Base];
		css->constitution = equipStat[Stat_Con][Stat_Base];
		css->psyche = equipStat[Stat_Psy][Stat_Base];
		css->spirit = equipStat[Stat_Spi][Stat_Base];
		*/
		short baseStr = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Str]];
		short baseDex = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Dex]];
		short baseCon = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Con]];
		short basePsy = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Psy]];
		short baseSpi = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Spi]];
		cdef.css.strength = baseStr;
		cdef.css.dexterity = baseDex;
		cdef.css.constitution = baseCon;
		cdef.css.psyche = basePsy;
		cdef.css.spirit = baseSpi;
		Util::SafeFormat(css->base_stats, sizeof(css->base_stats), "%d,%d,%d,%d,%d", baseStr, baseDex, baseCon, basePsy, baseSpi);

		if(destData != NULL)
		{
			if(destData->css.strength < baseStr) destData->css.strength = baseStr;
			if(destData->css.dexterity < baseDex) destData->css.dexterity = baseDex;
			if(destData->css.constitution < baseCon) destData->css.constitution = baseCon;
			if(destData->css.psyche < basePsy) destData->css.psyche = basePsy;
			if(destData->css.spirit < baseSpi) destData->css.spirit = baseSpi;

			destData->UpdateBaseStatMinimum(STAT::STRENGTH, baseStr);
			destData->UpdateBaseStatMinimum(STAT::DEXTERITY, baseDex);
			destData->UpdateBaseStatMinimum(STAT::CONSTITUTION, baseCon);
			destData->UpdateBaseStatMinimum(STAT::PSYCHE, basePsy);
			destData->UpdateBaseStatMinimum(STAT::SPIRIT, baseSpi);
		}
	}
}

void CharacterData :: UpdateEquipStats(CreatureInstance *destData)
{
	if(destData != NULL)
	{
		destData->MainDamage[0] = 0;
		destData->MainDamage[1] = 0;
		destData->RangedDamage[0] = 0;
		destData->RangedDamage[1] = 0;
		destData->OffhandDamage[0] = 0;
		destData->OffhandDamage[1] = 0;
		memset(destData->EquippedWeapons, 0, sizeof(destData->EquippedWeapons));
	}

	int armorClassMult = 1;
	switch(cdef.css.profession)
	{
	case Professions::KNIGHT: armorClassMult = 4; break;
	case Professions::ROGUE: armorClassMult = 2; break;
	case Professions::MAGE: armorClassMult = 1; break;
	case Professions::DRUID: armorClassMult = 3; break;
	}

	ItemSetTally itemSetTally;

	bool hasMeleeWeapon = false;
	bool hasShield = false;
	for(size_t a = 0; a < inventory.containerList[EQ_CONTAINER].size(); a++)
	{
		int slot = inventory.containerList[EQ_CONTAINER][a].GetSlot();
		int ID = inventory.containerList[EQ_CONTAINER][a].IID;
		VirtualItem *vitem = NULL;
		if(ID >= ItemManager::BASE_VIRTUAL_ITEM_ID)
			vitem = g_ItemManager.GetVirtualItem(ID);

		ItemDef *itemDef = inventory.containerList[EQ_CONTAINER][a].ResolveSafeItemPtr();

		g_ItemSetManager.CheckItem(ID, itemSetTally);

		if(itemDef->mEquipType == ItemEquipType::ARMOR_SHIELD)
			hasShield = true;
		switch(itemDef->mWeaponType)
		{
		case WeaponType::SMALL:
		case WeaponType::ONE_HAND:
		case WeaponType::TWO_HAND:
		case WeaponType::POLE:
		case WeaponType::WAND:
		case WeaponType::ARCANE_TOTEM:
			hasMeleeWeapon = true;
		}

		if(destData != NULL)
		{
			if(itemDef->mWeaponType != 0)
			{
				int wslot = slot;
				if(slot < 0 || slot >= 3)
				{
					g_Logs.server->error("UpdateEquipStats unexpected weapon slot: %v", slot);
					wslot = 0;
				}
				destData->EquippedWeapons[wslot] = itemDef->mWeaponType;
			}

			if(itemDef->mBonusStrength > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::STRENGTH, itemDef->mBonusStrength);
			if(itemDef->mBonusDexterity > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DEXTERITY, itemDef->mBonusDexterity);
			if(itemDef->mBonusConstitution > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::CONSTITUTION, itemDef->mBonusConstitution);
			if(itemDef->mBonusPsyche > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::PSYCHE, itemDef->mBonusPsyche);
			if(itemDef->mBonusSpirit > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::SPIRIT, itemDef->mBonusSpirit);

			if(itemDef->mArmorResistMelee > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_MELEE, itemDef->mArmorResistMelee * armorClassMult);
			if(itemDef->mArmorResistFire > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_FIRE, itemDef->mArmorResistFire);
			if(itemDef->mArmorResistFrost > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_FROST, itemDef->mArmorResistFrost);
			if(itemDef->mArmorResistMystic > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_MYSTIC, itemDef->mArmorResistMystic);
			if(itemDef->mArmorResistDeath > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_DEATH, itemDef->mArmorResistDeath);

			if(vitem != NULL)
			{
				STRINGLIST modentry;
				STRINGLIST moddata;
				std::string combine;
				vitem->MergeStats(vitem->mModString, combine);
				Util::Split(combine, "&", modentry);
				for(size_t i = 0; i < modentry.size(); i++)
				{
					Util::Split(modentry[i], "=", moddata);
					if(moddata.size() >= 2)
					{
						int index = GetStatIndexByName(moddata[0]);
						if(index >= 0)
						{
							if(StatList[index].isNumericalType() == true)
							{
								int statID = StatList[index].ID;
								float value = static_cast<float>(atof(moddata[1].c_str()));
								if(vitem->isItemDefStat(statID) == false)
									destData->AddItemStatMod(itemDef->mID, statID, value);
							}
						}
					}
				}
			}
			else if(itemDef->Params.size() > 0)
			{
				destData->ApplyItemStatModFromConfig(itemDef->mID, itemDef->Params);
			}
		}

		if(destData != NULL)
		{
			int slot = inventory.containerList[EQ_CONTAINER][a].CCSID & CONTAINER_SLOT;
			if(slot == ItemEquipSlot::WEAPON_MAIN_HAND)
			{
				destData->MainDamage[0] = itemDef->mWeaponDamageMin;
				destData->MainDamage[1] = itemDef->mWeaponDamageMax;
			}
			else if(slot == ItemEquipSlot::WEAPON_RANGED)
			{
				destData->RangedDamage[0] = itemDef->mWeaponDamageMin;
				destData->RangedDamage[1] = itemDef->mWeaponDamageMax;
			}
			else if(slot == ItemEquipSlot::WEAPON_OFF_HAND)
			{
				destData->OffhandDamage[0] = itemDef->mWeaponDamageMin;
				destData->OffhandDamage[1] = itemDef->mWeaponDamageMax;
			}
		}
	}

	if(destData != NULL)
	{
		g_ItemSetManager.UpdateCreatureBonuses(itemSetTally, destData);

		destData->SetServerFlag(ServerFlags::HasMeleeWeapon, hasMeleeWeapon);
		destData->SetServerFlag(ServerFlags::HasShield, hasShield);

		//g_Log.AddMessageFormat("Melee: %d, Shield: %d", hasMeleeWeapon, hasShield);
	}
}

void CharacterData :: UpdateEqAppearance()
{
	std::string str;
	str.append("{");
	for(size_t a = 0; a < inventory.containerList[EQ_CONTAINER].size(); a++)
	{
		InventorySlot *slot = &inventory.containerList[EQ_CONTAINER][a];
		if(a > 0)
			str.append(",");
		int appearanceID = slot->GetLookID();
		str.append("[");
		Util::StringAppendInt(str, slot->CCSID & CONTAINER_SLOT);
		str.append("]=");
		Util::StringAppendInt(str, appearanceID);
	}
	str.append("}");
	cdef.css.SetEqAppearance(str.c_str());
}

void CharacterData :: BackupAppearance(void)
{
	if(originalAppearance.size() == 0)
		originalAppearance = cdef.css.appearance;
}

void CharacterData :: BuildAvailableQuests(QuestDefinitionContainer &questList)
{
	questJournal.availableQuests.itemList.clear();
	questJournal.availableSoonQuests.itemList.clear();

	//The iterators below would crash the program if attempting to iterate across an empty quest list
	//when trying to set up the default null (zero ID) character.
	if(questList.mQuests.size() == 0)
		return;

	QuestReference qr;
	bool soon = false;
	
	QuestDefinitionContainer::ITERATOR it;
	for(it = questList.mQuests.begin(); it != questList.mQuests.end(); ++it)
	{
		QuestDefinition *qd = &it->second;

		soon = false;
		if(cdef.css.level < qd->levelMin)
		{
			if(cdef.css.level >= qd->levelMin - QuestJournal::QUEST_SOON_TOLERANCE)
				soon = true;
			else
				continue;
		}
		if(qd->levelMax != 0)
			if(cdef.css.level > qd->levelMax)
				continue;
		if(qd->profession != 0)
			if(cdef.css.profession != qd->profession)
				continue;

		// Guild start quest
		if(qd->guildStart && IsInGuildAndHasValour(qd->guildId, 0))
			continue;

		// Guild requirements
		if(!qd->guildStart && qd->guildId != 0 && !IsInGuildAndHasValour(qd->guildId, qd->valourRequired))
			continue;

		//
		if(qd->accountQuest) {
			AccountData *acc = g_AccountManager.GetActiveAccountByID(AccountID);
			if(acc != NULL && std::find(acc->AccountQuests.begin(), acc->AccountQuests.end(), qd->questID) != acc->AccountQuests.end())
				continue;
		}

		//If we get here, the quest is good to add to at least one availability list.
		qr.QuestID = qd->questID;
		qr.DefPtr = qd;
		qr.CreatureDefID = qd->QuestGiverID;

		//Add to the pending list.  This function performs the necessary checks.
		if(soon == false)
		{
			questJournal.AddPendingQuest(qr);
		}
		else
			questJournal.availableSoonQuests.AddItem(qr);
	}

	if(questJournal.availableSoonQuests.itemList.size() > 0)
		questJournal.availableSoonQuests.Sort();
}

void CharacterData :: OnFinishedLoading(void)
{
	//If the character file loaded any items into the vault, then all vault processing must
	//be local only.  Do not overwrite the account vault when logging out.
	localCharacterVault = true;
	
	/*
	localCharacterVault = (inventory.containerList[BANK_CONTAINER].size() > 0);
	g_Log.AddMessageFormat("Local vault data: %d.", localCharacterVault);
	*/

	//Reset the important stats here.  Just in case the character file has incorrect values.
	//Base stats and health will be recalculated after when the equipment bonuses are rechecked.
	cdef.css.strength = 0;
	cdef.css.dexterity = 0;
	cdef.css.constitution = 0;
	cdef.css.psyche = 0;
	cdef.css.spirit = 0;
	cdef.css.damage_resist_melee = 0;
	cdef.css.damage_resist_fire = 0;
	cdef.css.damage_resist_frost = 0;
	cdef.css.damage_resist_mystic = 0;
	cdef.css.damage_resist_death = 0;
	cdef.css.dmg_mod_death = 0;
	cdef.css.dmg_mod_fire = 0;
	cdef.css.dmg_mod_frost = 0;
	cdef.css.dmg_mod_mystic = 0;
	cdef.css.dmg_mod_melee = 0;
	cdef.css.dr_mod_death = 0;
	cdef.css.dr_mod_fire = 0;
	cdef.css.dr_mod_frost = 0;
	cdef.css.dr_mod_mystic = 0;
	cdef.css.dr_mod_melee = 0;
	cdef.css.casting_setback_chance = 500;
	cdef.css.channeling_break_chance = 500;
	cdef.css.offhand_weapon_damage = 500;
	cdef.css.base_healing = 0;
	cdef.css.base_damage_death = 0;
	cdef.css.base_damage_fire = 0;
	cdef.css.base_damage_frost = 0;
	cdef.css.base_damage_mystic = 0;
	cdef.css.base_damage_melee = 0;
	cdef.css.extra_damage_death = 0;
	cdef.css.extra_damage_fire = 0;
	cdef.css.extra_damage_frost = 0;
	cdef.css.extra_damage_mystic = 0;
	cdef.css.melee_attack_speed = 0;
	cdef.css.magic_attack_speed = 0;
	cdef.css.mod_melee_to_crit = 0.0F;
	cdef.css.mod_magic_to_crit = 0.0F;
	cdef.css.mod_attack_speed = 0;
	cdef.css.mod_movement = 0;
	cdef.css.experience_gain_rate = 0;
	cdef.css.base_movement = 0;
	cdef.css.weapon_damage_1h = 0;
	cdef.css.weapon_damage_2h = 0;
	cdef.css.weapon_damage_box = 0;
	cdef.css.weapon_damage_pole = 0;
	cdef.css.weapon_damage_small = 0;
	cdef.css.weapon_damage_thrown = 0;
	cdef.css.weapon_damage_wand = 0;
	cdef.css.base_block = 0;
	cdef.css.base_parry = 0;
	cdef.css.base_dodge = 0;
	cdef.css.mod_attack_speed = ABGlobals::MINIMAL_FLOAT;
	cdef.css.mod_casting_speed = ABGlobals::MINIMAL_FLOAT;
	cdef.css.mod_health_regen = 0.0F;
	cdef.css.might_regen = ABGlobals::DEFAULT_MIGHT_REGEN;
	cdef.css.will_regen = ABGlobals::DEFAULT_WILL_REGEN;
	cdef.css.mod_luck = 0.0F;
	cdef.css.bonus_health = 0;
	cdef.css.damage_shield = 0;

	UpdateBaseStats(NULL, true);

	inventory.FixBuyBack();
	BackupAppearance();
	BuildAvailableQuests(QuestDef);

	questJournal.ResolveLoadedQuests();
	inventory.CountInventorySlots();
	
	cdef.css.total_ability_points = Global::GetAbilityPointsLevelCumulative(cdef.css.level) + ExtraAbilityPoints;
	CheckVersion();

	VersionUpgradeCharacterItems();

	//Check if the scaler is valid since it may have changed through updates.
	if(InstanceScaler.size() > 0)
	{
		if(g_InstanceScaleManager.GetProfile(InstanceScaler) == NULL)
			InstanceScaler.clear();
	}
}

void CharacterData :: CheckVersion(void)
{
	if(characterVersion >= SERVER_CHARACTER_VERSION)
		return;

	if(characterVersion < 2)
		VersionUpgradeCharacterItems();

	characterVersion = SERVER_CHARACTER_VERSION;
	pendingChanges++;
	g_Logs.data->info("Character Upgraded: %v (%v)", cdef.css.display_name, cdef.CreatureDefID);
}

void CharacterData :: VersionUpgradeCharacterItems(void)
{
	if(g_Config.Upgrade == 0)
		return;

	for(int c = 0; c < MAXCONTAINER; c++)
	{
		for(size_t i = 0; i < inventory.containerList[c].size(); i++)
		{
			ItemDef *itemDef = inventory.containerList[c][i].ResolveItemPtr();
			if(itemDef != NULL)
			{
				if(itemDef->mBindingType == BIND_ON_PICKUP && inventory.containerList[c][i].bindStatus == 0)
				{
					inventory.containerList[c][i].bindStatus = 1;
					g_Logs.data->info("Updating bind status %v (%v):%v", cdef.css.display_name, cdef.CreatureDefID, itemDef->mDisplayName.c_str());
				}
			}
		}
	}
	size_t pos = 0;
	while(pos < inventory.containerList[EQ_CONTAINER].size())
	{
		bool del = false;
		ItemDef *itemDef = inventory.containerList[EQ_CONTAINER][pos].ResolveItemPtr();
		if(itemDef != NULL)
		{
			int slot = inventory.containerList[EQ_CONTAINER][pos].GetSlot();
			int res = inventory.VerifyEquipItem(itemDef, slot, cdef.css.level, cdef.css.profession);
			if(res != InventoryManager::EQ_ERROR_NONE)
			{
				int slot = inventory.GetFreeSlot(INV_CONTAINER);
				if(slot >= 0)
				{
					InventorySlot copy;
					copy.CopyFrom(inventory.containerList[EQ_CONTAINER][pos], true);
					copy.CCSID = inventory.GetCCSID(INV_CONTAINER, slot);
					inventory.containerList[INV_CONTAINER].push_back(copy);
					del = true;
				}
				else
					g_Logs.server->warn("%v: No inventory space for: %v", cdef.css.display_name, itemDef->mDisplayName.c_str());
			}
		}
		if(del == false)
			pos++;
		else {
			g_Logs.server->info("%v: Moving unequippable item: %v", cdef.css.display_name, itemDef->mDisplayName.c_str());
			inventory.containerList[EQ_CONTAINER].erase(inventory.containerList[EQ_CONTAINER].begin() + pos);
		}
	}
	pendingChanges++;
}

//Called by the character creation functions to initialize some specific defaults.
void CharacterData :: OnCharacterCreation(void)
{
	CurrentVaultSize = g_Config.VaultInitialPurchaseSize;

	if(cdef.css.level <= 1)
	{
		//Add the default quest.
		QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(378);
		if(qdef != NULL)
			questJournal.activeQuests.AddItem(378, qdef);

		/*
		activeData.CurX = 5682;
		activeData.CurY = 682;
		activeData.CurZ = 5890;
		activeData.CurZone = 59;
		activeData.CurRotation = 225;
		*/


		activeData.CurZone = g_InfoManager.GetStartZone();
		activeData.CurX = g_InfoManager.GetStartX();
		activeData.CurY = g_InfoManager.GetStartY();
		activeData.CurZ = g_InfoManager.GetStartZ();
		activeData.CurRotation = g_InfoManager.GetStartRotation();


		std::string quickbar0;
		quickbar0 = "\"{[\\\"slotsY\\\"]=1,[\\\"slotsX\\\"]=8,[\\\"snapY\\\"]=null,[\\\"y\\\"]=0.863333,[\\\"x\\\"]=0.33625,[\\\"positionX\\\"]=84,[\\\"positionY\\\"]=26,[\\\"snapX\\\"]=null,[\\\"locked\\\"]=true,[\\\"visible\\\"]=true,[\\\"buttons\\\"]=[null,null,";
		switch(cdef.css.profession)
		{
		case Professions::KNIGHT:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("224\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("288\\\",");
			break;
		case Professions::ROGUE:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("232\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("292\\\",");
			break;
		case Professions::MAGE:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("240\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("297\\\",");
			break;
		case Professions::DRUID:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("248\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("301\\\",");
			break;
		default:
			quickbar0.append("null,null,");
		}
		quickbar0.append("null,null,null,null]}\"");
		preferenceList.SetPref("quickbar.0", quickbar0.c_str());
	}
}

void CharacterData :: OnRankChange(int newRank)
{
	//This function is intended to be called from the main thread.
	//Since we're altering the character data, need to lock access.
	g_CharacterManager.GetThread("CharacterData::OnRankChange");
	BuildAvailableQuests(QuestDef);
	g_CharacterManager.ReleaseThread();
}

void CharacterData :: OnLevelChange(int newLevel)
{
	//This function is intended to be called from the main thread.
	//Since we're altering the character data, need to lock access.
	g_CharacterManager.GetThread("CharacterData::OnLevelChange");

	cdef.css.level = newLevel;
	BuildAvailableQuests(QuestDef);

	g_CharacterManager.ReleaseThread();
}

void CharacterData :: SetPlayerDefaults(void)
{
	cdef.DefHints = 1;
	cdef.css.mod_casting_speed = numeric_limits<float>::denorm_min();
}

void CharacterData :: AbilityRespec(CreatureInstance *ptr)
{
	//cdef.css.total_ability_points = cdef.css.level * ABILITY_POINTS_PER_LEVEL;
	cdef.css.total_ability_points = Global::GetAbilityPointsLevelCumulative(cdef.css.level);
	cdef.css.total_ability_points += ExtraAbilityPoints;

	cdef.css.current_ability_points = cdef.css.total_ability_points;

	//Allows this function to update an instantiated creature with the correct
	//ability point data.
	if(ptr != NULL)
	{
		ptr->css.total_ability_points = cdef.css.total_ability_points;
		ptr->css.current_ability_points = cdef.css.current_ability_points;
	}

	abilityList.AbilityList.clear();

	abilityList.AddAbility(188);  // Bind : 1
	abilityList.AddAbility(189);  // Bind : 1

	switch(cdef.css.profession)
	{
	case Professions::KNIGHT:
		abilityList.AddAbility(224);  //Assault : 1
		abilityList.AddAbility(288);  //Bash : 1
		abilityList.AddAbility(1);    //Two Hand Weapons : 1
		abilityList.AddAbility(2);    //One Hand Weapons : 1
		abilityList.AddAbility(5);    //Bow Weapons : 1
		abilityList.AddAbility(9);    //Parry : 1
		abilityList.AddAbility(10);   //Block : 1
		break;
	case Professions::ROGUE:
		abilityList.AddAbility(232);  //Assail : 1
		abilityList.AddAbility(292);  //Disembowel : 1
		abilityList.AddAbility(8);    //Dual Wield : 1
		abilityList.AddAbility(2);    //One Hand Weapons : 1
		abilityList.AddAbility(3);    //Small Weapons : 1
		abilityList.AddAbility(6);    //Thrown Weapons : 1
		abilityList.AddAbility(9);    //Parry : 1
		break;
	case Professions::MAGE:
		abilityList.AddAbility(240);  //Firebolt : 1
		abilityList.AddAbility(297);  //Pyro Blast : 1
		abilityList.AddAbility(3);    //Small Weapons : 1
		abilityList.AddAbility(7);    //Wand Weapons : 1
		break;
	case Professions::DRUID:
		abilityList.AddAbility(248);  //Sting : 1
		abilityList.AddAbility(301);  //Deadly Shot : 1
		abilityList.AddAbility(4);    //Pole Weapons : 1
		abilityList.AddAbility(2);    //One Hand Weapons : 1
		abilityList.AddAbility(5);    //Bow Weapons : 1
		abilityList.AddAbility(9);    //Parry : 1
		break;
	}
}

void CharacterData :: SetLastChannel(const char *name, const char *password)
{
	if(name == NULL)
		PrivateChannelName.clear();
	else
		PrivateChannelName = name;

	if(password == NULL)
		PrivateChannelPassword.clear();
	else
		PrivateChannelPassword = password;
}

bool CharacterData :: HengeHas(int creatureDefID)
{
	for(size_t i = 0; i < hengeList.size(); i++)
		if(hengeList[i] == creatureDefID)
			return true;
	return false;
}

void CharacterData :: HengeAdd(int creatureDefID)
{
	hengeList.push_back(creatureDefID);
}

void CharacterData :: HengeSort(void)
{
	std::sort(hengeList.begin(), hengeList.end());
}

bool CharacterData :: SetPermission(short filterType, const char *name, bool value)
{
	int a;
	for(a = 0; a < MaxPermissionDef; a++)
	{
		if(PermissionDef[a].type == filterType)
		{
			if(strcmp(PermissionDef[a].name, name) == 0)
			{
				if(value == true)
					PermissionSet[PermissionDef[a].index] |= PermissionDef[a].flag;
				else
					PermissionSet[PermissionDef[a].index] &= (~(PermissionDef[a].flag));
				return true;
			}
		}
	}
	return false;
}

bool CharacterData :: HasPermission(short permissionSet, unsigned int permissionFlag)
{
	if(PermissionSet[permissionSet] & permissionFlag)
		return true;

	return false;
}

bool CharacterData :: QualifyGarbage(void)
{
	//Return true if the character data is garbage and can be safely deleted.
	if(expireTime == 0)      //Character is still registered within the game.
		return false;

//	if(pendingChanges == 0)  //Character needs to be saved before it can be deleted.
//		return false;

	if(g_ServerTime >= expireTime)
		return true;

	return false;
}


void CharacterData :: RemoveSidekicks(int sidekickType)
{
	size_t pos = 0;
	while(pos < SidekickList.size())
	{
		if(SidekickList[pos].summonType == sidekickType)
			SidekickList.erase(SidekickList.begin() + pos);
		else
			pos++;
	}
}

void CharacterData :: AddSidekick(SidekickObject& skobj)
{
	SidekickList.push_back(skobj);
}

int CharacterData :: CountSidekick(int sidekickType)
{
	int count = 0;
	for(size_t i = 0; i < SidekickList.size(); i++)
		if(SidekickList[i].summonType == sidekickType)
			count++;

	return count;
}

void CharacterData :: AddAbilityPoints(int abilityPointCount)
{
	ExtraAbilityPoints += abilityPointCount;
}

bool CharacterData :: NotifyUnstick(bool peek)
{
	int minutesPassed = g_PlatformTime.getAbsoluteMinutes() - LastUnstickTime;
	if(peek == true)
		return (minutesPassed < 5);

	LastUnstickTime = g_PlatformTime.getAbsoluteMinutes();
	UnstickCount++;
	if(minutesPassed < 5)
		return true;

	return false;
}

void CharacterData :: Debug_CountItems(int *intArr)
{
	for(size_t c = 0; c < 6; c++)
	{
		for(size_t i = 0; i < inventory.containerList[c].size(); i++)
		{
			InventorySlot *slot = &inventory.containerList[c][i];
			ItemDef *itemDef = slot->ResolveItemPtr();
			if(itemDef != NULL)
			{
				intArr[(int)itemDef->mQualityLevel]++;
			}
		}
	}
}

//Return the total vault capacity by combining the default space with the character-specific
//expanded space.
int CharacterData :: VaultGetTotalCapacity(void)
{
	int slots = g_Config.VaultDefaultSize + CurrentVaultSize;
	return Util::ClipInt(slots, 0, MAX_VAULT_SIZE);
}

//Return true if the vault has reached its maximum limit (default + extra).  If the limit
//is reached, it should allow any more expansions.
bool CharacterData :: VaultIsMaximumCapacity(void)
{
	return ((g_Config.VaultDefaultSize + CurrentVaultSize) >= MAX_VAULT_SIZE);
}

//Perform a vault expansion.  Assumes that all conditions are met.  Adjusts the relevant information
//in the character data (NOTE: does not subtract credits, those are part of the operational creature stat
//set).
void CharacterData :: VaultDoPurchaseExpand(void)
{
	CurrentVaultSize += VAULT_EXPAND_SIZE;
	CreditsSpent += VAULT_EXPAND_CREDIT_COST;
}

/*
void CharacterData :: NamedLocationUpdate(const NamedLocation &location)
{
	NamedLocation *exist = NamedLocationGetPtr(location.mName.c_str());
	if(exist == NULL)
		namedLocation.push_back(location);
	else
		exist->CopyFrom(location);
}

void CharacterData :: NamedLocationGetPtr(const char *name)
{
	for(size_t i = 0; i < namedLocation.size(); i++)
		if(namedLocation[i].mName.compare(name) == 0)
			return &namedLocation[i];
}
*/

enum CharacterDataFileSections
{
	CDFS_None = 0,
	CDFS_General,
	CDFS_Stats,
	CDFS_Prefs,
	CDFS_Inv,
	CDFS_Quest,
	CDFS_Cooldown,
	CDFS_Abilities
};

void CharacterData::WritePrivateToJSON(Json::Value &value)
{

	value["privateChannelName"] = PrivateChannelName;
	value["privateChannelPassword"] = PrivateChannelPassword;
}

void CharacterData::WriteToJSON(Json::Value &value)
{
	value["account"] = AccountID;
	value["characterVersion"] = characterVersion;
	value["clan"] = clan;
	value["status"] = StatusText;
	value["secondsLogged"] = Json::UInt64(SecondsLogged);
	value["createdTime"] = Json::UInt64(CreatedTimeSec);
	value["sessionsLogged"] = SessionsLogged;
	value["timeLogged"] = TimeLogged;
	value["lastSession"] = LastSession;
	value["lastLogon"] = LastLogOn;
	value["lastLogoff"] = LastLogOff;
	value["shard"] = Shard;
	value["originalAppearance"] = originalAppearance;
	value["instanceScale"] = InstanceScaler;
	value["currentVaultSize"] = CurrentVaultSize;
	value["creditsPurchased"] = CreditsPurchased;
	value["creditsSpent"] = CreditsSpent;
	value["extraAbilityPoints"] = ExtraAbilityPoints;
	value["lastWarpTime"] = Json::UInt64(LastWarpTime);
	value["unstickCount"] = UnstickCount;
	value["lastUnstickTime"] = Json::UInt64(LastUnstickTime);

	// JSON version of appear
	Json::Value appearance;
	JsonHelpers::SquirrelAppearanceToJSONAppearance(cdef.css.appearance, appearance);
	value["appearance"] = appearance;

	Json::Value henges(Json::arrayValue);
	for(std::vector<int>::iterator it = hengeList.begin(); it != hengeList.end(); ++it) {
		henges.append(*it);
	}
	value["henges"] = henges;

	Json::Value stats;
	PlayerStats.WriteToJSON(stats);
	value["playerStats"] = stats;

//	fprintf(output, "GroveReturn=%d,%d,%d,%d\r\n", cd.groveReturnPoint[0], cd.groveReturnPoint[1], cd.groveReturnPoint[2], cd.groveReturnPoint[3]);
//	fprintf(output, "BindReturn=%d,%d,%d,%d\r\n", cd.bindReturnPoint[0], cd.bindReturnPoint[1], cd.bindReturnPoint[2], cd.bindReturnPoint[3]);

	Json::Value c;
	cdef.WriteToJSON(c);
	value["cdef"] = c;


	Json::Value a;
	a["instance"] = activeData.CurInstance;
	a["zone"] = activeData.CurZone;
	a["x"] = activeData.CurX;
	a["y"] = activeData.CurY;
	a["z"] = activeData.CurZ;
	a["rot"] = activeData.CurRotation;
	value["activeData"] = a;


	value["maxSidekicks"] = MaxSidekicks;
	value["mode"] = Mode;

	Json::Value guilds(Json::objectValue);
	for(std::vector<GuildListObject>::iterator it = guildList.begin(); it != guildList.end(); ++it) {
		GuildListObject ob = *it;
		Json::Value g;
		ob.WriteToJSON(g);
		guilds[ob.GuildDefID] = g;
	}
	value["guilds"] = guilds;

}


void GuildListObject :: WriteToJSON(Json::Value &value)
{
	value["id"] = GuildDefID;
	value["valour"] = Valour;
}

void GuildListObject :: Clear(void)
{
	GuildDefID = 0;
	Valour = 0;
}

//

FriendListObject :: FriendListObject()
{
	Clear();
}

FriendListObject :: ~FriendListObject()
{
	Name.clear();
}

void FriendListObject :: Clear(void)
{
	Name.clear();
	CDefID = 0;
}

CharacterLeaderboard :: CharacterLeaderboard() {
	SetName("character");
}

CharacterLeaderboard :: ~CharacterLeaderboard() {
}

void CharacterLeaderboard :: OnBuild(std::vector<Leader> *leaders)
{
	g_ClusterManager.Scan([this, leaders](const std::string &v){
		STRINGLIST l;
		Util::Split(v, ":", l);
		CharacterData *cd = g_CharacterManager.RequestCharacter(atoi(l[1].c_str()), true);
		if(cd != NULL) {
			Leader l;
			l.mId = cd->cdef.CreatureDefID;
			l.mName = cd->cdef.css.display_name;
			l.mStats.CopyFrom(&cd->PlayerStats);
			leaders->push_back(l);
		}
	}, StringUtil::Format("%s:*", KEYPREFIX_CHARACTER_NAME_TO_ID.c_str()));
}


CharacterManager :: CharacterManager()
{
	cs.SetDebugName("CS_CHARMGR");
	cs.disabled = true;
}

CharacterManager :: ~CharacterManager()
{
	Clear();
}

void CharacterManager :: Clear(void)
{
	charList.clear();
}

int CharacterManager :: LoadCharacter(int CDefID, bool tempResource)
{
	CharacterData newChar;
	newChar.cdef.CreatureDefID = CDefID;
	if(!g_ClusterManager.ReadEntity(&newChar))
	{
		return -1;
	}

	if(tempResource == true)
		newChar.expireTime = g_ServerTime + CharacterManager::TEMP_EXPIRE_TIME;

	GetThread("CharacterManager::LoadCharacter");

	bool newInstance = false;
	CHARACTER_MAP::iterator it;
	it = charList.lower_bound(CDefID);
	if(it == charList.end())
	{
		charList.insert(charList.begin(), CHARACTER_PAIR(CDefID, newChar));
		newInstance = true;
	}
	else if(it->first != CDefID)
	{
		charList.insert(it, CHARACTER_PAIR(CDefID, newChar));
		newInstance = true;
	}
	else
		g_Logs.data->warn("LoadCharacter() ID [%v] already exists.", CDefID);

	if(newInstance == true)
	{
		charList[CDefID].SetPlayerDefaults();
		charList[CDefID].OnFinishedLoading();
	}

	ReleaseThread();
//	g_Log.AddMessageFormatW(MSG_SHOW, "Successfully loaded creature: %d [%s]", CDefID, charList[CDefID].cdef.css.display_name);
	return 0;
}

void CharacterManager :: GetThread(const char *request)
{
	cs.Enter(request);
}

void CharacterManager :: ReleaseThread(void)
{
	cs.Leave();
}

CharacterData * CharacterManager :: GetPointerByID(int CDefID)
{
	CHARACTER_MAP::iterator it;
	it = charList.find(CDefID);
	if(it == charList.end())
		return NULL;

	//Since we're accessing it, there must be some demand.
	//Extend the expiration timer, if it has one.
	it->second.ExtendExpireTime();
	return &it->second;
}

CharacterData * CharacterManager :: GetCharacterByName(const char *name)
{
	//TODO: not thread safe?
	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
		if(strcmp(it->second.cdef.css.display_name, name) == 0)
			return &it->second;
	return NULL;
}

CharacterData * CharacterManager :: GetDefaultCharacter(void)
{
	CreateDefaultCharacter();
	return &charList[0];
}

void CharacterManager :: CreateDefaultCharacter(void)
{
	CharacterData *ret = RequestCharacter(0, false);
	if(ret != NULL)
		return;

	CharacterData newObj;
	charList.insert(CHARACTER_PAIR(0, newObj));
	g_CharacterManager.SaveCharacter(0);
}

void CharacterManager :: AddExternalCharacter(int CDefID, CharacterData &newChar)
{
	CHARACTER_MAP::iterator it;
	it = charList.lower_bound(CDefID);
	if(it == charList.end())
		charList.insert(charList.begin(), CHARACTER_PAIR(CDefID, newChar));
	else if(it->first != CDefID)
		charList.insert(it, CHARACTER_PAIR(CDefID, newChar));
	else
		g_Logs.data->warn("AddExternalCharacter() ID [%v] already exists.", CDefID);
}

void CharacterManager :: Compatibility_SaveList(FILE *output)
{
	/*
	CHARACTER_MAP::iterator it;
	CharacterData empty;
	CharacterData *defChar = g_CharacterManager.GetDefaultCharacter();
	SaveCharacterToStream(output, *defChar, empty);

	for(it = charList.begin(); it != charList.end(); ++it)
	{
		SaveCharacterToStream(output, it->second, *defChar);
		fprintf(output, "\r\n\r\n");
	}
	*/
}

void CharacterManager :: Compatibility_ResolveCharacters(void)
{
	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
	{
		it->second.BuildAvailableQuests(QuestDef);
		it->second.questJournal.ResolveLoadedQuests();
		it->second.inventory.CountInventorySlots();
	}
}

CharacterData * CharacterManager :: RequestCharacter(int CDefID, bool tempOnly)
{
	CharacterData *retPtr = GetPointerByID(CDefID);
	if(retPtr != NULL)
	{
		if(tempOnly == true && retPtr->expireTime > 0)
			retPtr->SetExpireTime();
		else if(tempOnly == false)
			retPtr->EraseExpirationTime();

		return retPtr;
	}

	int r = LoadCharacter(CDefID, tempOnly);
	if(r >= 0)
	{
		retPtr = &charList[CDefID];
		if(tempOnly == true && retPtr->expireTime > 0)
			retPtr->SetExpireTime();
		else if(tempOnly == false)
			retPtr->EraseExpirationTime();

		return retPtr;
	}

	return NULL;
}

void CharacterManager :: CheckGarbageCharacters(void)
{
	if(GarbageTimer.ReadyWithUpdate(CharacterManager::TEMP_GARBAGE_INTERVAL) == true)
		RemoveGarbageCharacters();
}

void CharacterManager :: RemoveGarbageCharacters(void)
{
	GetThread("CharacterManager::RemoveGarbageCharacters");

	/*
	CHARACTER_MAP::iterator it;
	it = charList.begin();
	bool qualify = false;
	while(it != charList.end())
	{
		qualify = RemoveSingleGarbage(it->second);
		if(qualify == true)
		{
			g_Log.AddMessageFormat("Removing garbage character: %s (%d)", it->second.cdef.css.display_name, it->first);
			charList.erase(it++);
		}
		else
			++it;
	}*/

	while(RemoveSingleGarbage() == true);

	ReleaseThread();
}

bool CharacterManager :: RemoveSingleGarbage(void)  //CharacterData &charData)
{

	/*
	//Return true if the character can be safely removed.
	if(charData.QualifyGarbage() == false)
		return false;

	if(charData.pendingChanges > 0)
	{
		if(SaveCharacter(charData.cdef.CreatureDefID) == false)
		{
			g_Log.AddMessageFormat("[ERROR] RemoveSingleGarbage() failed to save character");
			return false;
		}
	}
	return true;
	*/

	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
	{
		if(it->second.expireTime > 0)
		{
			if(g_ServerTime >= it->second.expireTime)
			{
				if(it->second.pendingChanges != 0)
				if(SaveCharacter(it->first) == false)
				{
					g_Logs.server->debug("RemoveSingleGarbage failed to save");
					return true;
				}

				UnloadCharacter(it->first);
				return true;
			}
		}
	}
	return false;
}

void CharacterManager :: UnloadAllCharacters(void)
{
	GetThread("CharacterManager::UnloadAllCharacters");

	while(RemoveSingleCharacter() == true);

	ReleaseThread();
}

bool CharacterManager :: RemoveSingleCharacter(void)
{
	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
	{
		if(it->second.pendingChanges != 0)
			SaveCharacter(it->first);

		UnloadCharacter(it->first);
		return true;
	}
	return false;
}

bool CharacterManager :: SaveCharacter(int CDefID, bool sync)
{
	CharacterData *ptr = GetPointerByID(CDefID);
	if(ptr == NULL)
	{
		g_Logs.data->error("SaveCharacter() invalid ID: %v", CDefID);
		return false;
	}

	if(!g_ClusterManager.WriteEntity(ptr, sync)) {
		g_Logs.data->error("SaveCharacter() could not save [%v]", CDefID);
		return false;
	}

	ptr->pendingChanges = 0;

	g_Logs.data->info("Saved character %v [%v]", ptr->cdef.CreatureDefID, ptr->cdef.css.display_name);
	return true;
}

void CharacterManager :: UnloadCharacter(int CDefID)
{
	CHARACTER_MAP::iterator it;
	it = charList.find(CDefID);
	if(it != charList.end())
	{
		if(it->second.pendingChanges != 0)
			g_Logs.server->warn("Unloading a character with pending changes:", CDefID);

//		g_Log.AddMessageFormat("Unloading character: %d [%s]", CDefID, it->second.cdef.css.display_name);
		charList.erase(it);
	}
}

void NamedLocation :: SetData(const char *data)
{
	STRINGLIST params;
	Util::Split(data, ",", params);
	if(params.size() >= 5)
	{
		mName = params[0];
		mX = atoi(params[1].c_str());
		mY = atoi(params[2].c_str());
		mZ = atoi(params[3].c_str());
		mZone = atoi(params[4].c_str());
	}
};

void NamedLocation :: CopyFrom(const NamedLocation &source)
{
	mName = source.mName;
	mX = source.mX;
	mY = source.mY;
	mZ = source.mZ;
	mZone = source.mZone;
}


int PrepExt_FriendsAdd(char *buffer, CharacterData *charData)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 43);       //_handleFriendNotificationMsg
	wpos += PutShort(&buffer[wpos], 0);       //Placeholder for size
	wpos += PutByte(&buffer[wpos], 4);        //Notification for friend added
	wpos += PutStringUTF(&buffer[wpos], charData->cdef.css.display_name);
	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}


int PrepExt_FriendsLogStatus(char *buffer, CharacterData *charData, int logStatus)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 43);       //_handleFriendNotificationMsg
	wpos += PutShort(&buffer[wpos], 0);       //Placeholder for size

	//Notifications: 2 = logout, 1 = login
	wpos += PutByte(&buffer[wpos], (logStatus == 0) ? 2 : 1);
	wpos += PutStringUTF(&buffer[wpos], charData->cdef.css.display_name);
	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}
