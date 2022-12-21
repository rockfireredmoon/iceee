#include "AbilityTime.h"
#include "Ability2.h"

#include "Config.h"
#include "GameConfig.h"
#include <string.h>
#include "util/Log.h"

extern unsigned long g_ServerTime;

void ActiveBuffManager :: ActiveToPersistent()
{
	g_Logs.server->info("Moving active %v buffs to persistent buffs (replacing %v)", buffList.size(), persistentBuffList.size());
	persistentBuffList.clear();
	persistentBuffList.assign(buffList.begin(), buffList.end());
	buffList.clear();
}

ActiveBuff * ActiveBuffManager :: GetPersistentBuff(unsigned char tier, short abID)
{

	for(size_t i = 0; i < persistentBuffList.size(); i++)
	{
		if(persistentBuffList[i].abID == abID && tier >= persistentBuffList[i].tier)
		{
			return &persistentBuffList[i];
		}
	}
	return NULL;
}

int ActiveBuffManager :: HasBuff(unsigned char tier, unsigned char buffType)
{
	for(size_t i = 0; i < buffList.size(); i++)
	{
		if(buffList[i].buffType == buffType)
		{
			bool del = false;
			if(g_ServerTime >= buffList[i].castEndTimeMS)
				del = true;
			else if(tier >= buffList[i].tier)
				del = true;

			if(del == true)
			{
				buffList.erase(buffList.begin() + i);
				return -1;
			}
			return i;
		}
	}
	return -1;
}

ActiveBuff * ActiveBuffManager :: UpdateBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration, bool initialising)
{
	int r = HasBuff(tier, buffType);
	if(r >= 0)
	{
		buffList[r].tier = tier;
		buffList[r].abID = abID;
		buffList[r].abgID = abgID;

		if(initialising)
		{
			ActiveBuff *persistentBuff = GetPersistentBuff(tier, abID);
			if(persistentBuff != NULL) {
				g_Logs.server->info("Overriding duration of %v with %v for ability %v because there was an active ability on logout", duration,
						persistentBuff->durationS, abID);
				buffList[r].durationS = (int)persistentBuff ->durationS;
			}
			else
				buffList[r].durationS = (int)duration;
		}
		else
			buffList[r].durationS = (int)duration;
		buffList[r].castEndTimeMS = g_ServerTime + (buffList[r].durationS * 1000);
		return &buffList[r];
	}
	else
	{
		return AddBuff(tier, buffType, abID, abgID, duration, initialising);
	}
}

ActiveBuff * ActiveBuffManager :: AddPersistentBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration)
{
	ActiveBuff buff;
	buff.tier = tier;
	buff.buffType = buffType;
	buff.abID = abID;
	buff.abgID = abgID;
	buff.durationS = (int)duration;
	buff.castEndTimeMS = g_ServerTime + (buff.durationS * 1000);
	persistentBuffList.push_back(buff);
	return &persistentBuffList.back();
}

ActiveBuff * ActiveBuffManager :: AddBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration, bool initialising)
{
	ActiveBuff buff;
	buff.tier = tier;
	buff.buffType = buffType;
	buff.abID = abID;
	buff.abgID = abgID;
	buff.durationS = (int)duration;

	if(initialising)
	{
		ActiveBuff *persistentBuff = GetPersistentBuff(tier, abID);
		if(persistentBuff != NULL) {
			g_Logs.server->info("Overriding duration of %v of with %v for ability %v because there was an active ability on logout", duration,
					persistentBuff->durationS, abID);
			buff.durationS = (int)persistentBuff ->durationS;
		}
	}

	buff.castEndTimeMS = g_ServerTime + (buff.durationS * 1000);
	buffList.push_back(buff);
	return &buffList.back();
}

// Remove a buff from the list, probably from an ability cancel.  Otherwise lower tier buffs
// cannot be recast.
void ActiveBuffManager :: RemoveBuff(int abilityID)
{
	size_t pos = 0;
	while(pos < buffList.size())
	{
		if(buffList[pos].abID == abilityID)
			buffList.erase(buffList.begin() + pos);
		else
			pos++;
	}
}

// Some old logging to assist with seeing which buffs exist when testing add/remove calls.
void ActiveBuffManager :: DebugLogBuffs(const char *label)
{
	g_Logs.server->debug("Active ability buffs (%v)", label);
	for(size_t i = 0; i < buffList.size(); i++)
	{
		const char *cat = g_AbilityManager.ResolveBuffCategoryName(buffList[i].buffType);
		g_Logs.server->debug("ab:%v,abgid:%v,tier:%v,cat:%v,timeleft:%v", buffList[i].abID, buffList[i].abgID, buffList[i].tier, cat, ( buffList[i].castEndTimeMS - g_ServerTime) / 1000);
	}
	if(g_GameConfig.UsePersistentBuffs) {
		g_Logs.server->debug("Persistent Active ability buffs (%v)", label);
		for(size_t i = 0; i < persistentBuffList.size(); i++)
		{
			const char *cat = g_AbilityManager.ResolveBuffCategoryName(persistentBuffList[i].buffType);
			g_Logs.server->debug("ab:%v,abgid:%v,tier:%v,cat:%v,timeleft:%v", persistentBuffList[i].abID, persistentBuffList[i].abgID, persistentBuffList[i].tier, cat, ( persistentBuffList[i].castEndTimeMS - g_ServerTime) / 1000);
		}
	}
}

void ActiveBuffManager :: CopyFrom(const ActiveBuffManager &source)
{
	buffList.assign(source.buffList.begin(), source.buffList.end());
	if(g_GameConfig.UsePersistentBuffs) {
		persistentBuffList.assign(source.persistentBuffList.begin(), source.persistentBuffList.end());
	}
}

void ActiveBuffManager :: WriteEntity(AbstractEntityWriter *writer)
{
	STRINGLIST l;
	for(size_t i = 0; i < buffList.size(); i++)
	{
		long remain = buffList[i].castEndTimeMS - g_ServerTime;
		const Ability2::AbilityEntry2* abEntry = g_AbilityManager.GetAbilityPtrByID(buffList[i].abID);
		// Only save abilities that have remaining time and are not passive
		if(abEntry != NULL && remain > 0 && !abEntry->IsPassive()) {
			g_Logs.server->info("Saving ability %v,%v,%v,%v,%v\r\n", buffList[i].tier,
					buffList[i].buffType, buffList[i].abID, buffList[i].abgID, remain);
			l.push_back(Util::Format("%d,%d,%d,%d,%lu", buffList[i].tier,
					buffList[i].buffType, buffList[i].abID, buffList[i].abgID, remain));
		}
	}
	if(l.size() > 0)
		writer->ListValue("Active", l);
}


void ActiveBuffManager :: Clear(void)
{
	buffList.clear();
}

void ActiveBuffManager :: ClearPersistent(void)
{
	persistentBuffList.clear();
}

unsigned long ActiveCooldown :: GetRemainTimeMS(void)
{
	if(g_ServerTime > castEndTimeMS)
		return 0;
	return (castEndTimeMS - g_ServerTime);
}

unsigned long ActiveCooldown :: GetElapsedTimeMS(void)
{
	if(castStartTimeMS > g_ServerTime)
		return 0;
	return (g_ServerTime - castStartTimeMS);
}

int ActiveCooldownManager :: HasCooldown(int category)
{
	for(size_t i = 0; i < cooldownList.size(); i++)
	{
		if(cooldownList[i].category == category)
		{
			//Since we're iterating the array, might as well check for end of time here and
			//delete it.  This ensures that a cooldown is always available if it has expired.
			if(g_ServerTime >= cooldownList[i].castEndTimeMS)
			{
				cooldownList.erase(cooldownList.begin() + i);
				return -1;
			}
			return i;
		}
	}
	return -1;
}

void ActiveCooldownManager :: AddCooldown(int category, unsigned long remainMS, unsigned long timeElapsedMS)
{
	ActiveCooldown acd;
	acd.category = category;
	acd.durationMS = remainMS;
	acd.castEndTimeMS = g_ServerTime + acd.durationMS;
	acd.castStartTimeMS = g_ServerTime - timeElapsedMS;
	cooldownList.push_back(acd);
}

void ActiveCooldownManager :: Clear(void)
{
	cooldownList.clear();
}

void ActiveCooldownManager :: CopyFrom(const ActiveCooldownManager &source)
{
	cooldownList.assign(source.cooldownList.begin(), source.cooldownList.end());
}

void ActiveCooldownManager :: WriteEntity(AbstractEntityWriter *writer)
{
	for(size_t i = 0; i < cooldownList.size(); i++)
	{
		long remain = cooldownList[i].castEndTimeMS - g_ServerTime;
		if(remain > 0)
		{
			const char *name = g_AbilityManager.ResolveCooldownCategoryName(cooldownList[i].category);
			if(name == NULL)
				continue;

			unsigned long rem = cooldownList[i].GetRemainTimeMS();
			unsigned long el = cooldownList[i].GetElapsedTimeMS();
			g_Logs.data->info("AbilityTyime WriteEntity %v = %v %v", name, rem, el);

			writer->Value(name, Util::Format("%lu,%lu", rem, el));
		}
	}
}

void ActiveCooldownManager :: LoadEntry(const char *name, unsigned long remainTimeMS, unsigned long timeElapsedMS)
{
	int category = g_AbilityManager.ResolveCooldownCategoryID(name);
	if(category != 0)
		AddCooldown(category, remainTimeMS, timeElapsedMS);
}
