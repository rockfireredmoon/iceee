#ifndef ABILITYTIME_H
#define ABILITYTIME_H

//Manages buffs and cooldowns.
#include <vector>
#include <stdio.h>
#include "Entities.h"

struct ActiveBuff
{
	unsigned char tier;
	unsigned char buffType;
	short abID;
	short abgID;
	double durationS;
	unsigned long castEndTimeMS;
};

struct ActiveBuffManager
{
	std::vector<ActiveBuff> buffList;
	std::vector<ActiveBuff> persistentBuffList;

	void ActiveToPersistent();
	int HasBuff(unsigned char tier, unsigned char buffType);
	int HasBuffNot(unsigned char tier, unsigned char buffType, int abilityGroup);
	ActiveBuff * UpdateBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration, bool initialising);
	void WriteEntity(AbstractEntityWriter *writer);
	ActiveBuff * GetPersistentBuff(unsigned char tier, short abID);
	ActiveBuff * AddPersistentBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration);
	ActiveBuff * AddBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration, bool initialising);
	void RemoveBuff(int abilityID);
	void DebugLogBuffs(const char *label);
	void CopyFrom(const ActiveBuffManager &source);
	void Clear(void);
	void ClearPersistent(void);
};

struct ActiveCooldown
{
	int category;
	int durationMS;
	unsigned long castStartTimeMS;
	unsigned long castEndTimeMS;
	unsigned long GetRemainTimeMS(void);
	unsigned long GetElapsedTimeMS(void);
};

struct ActiveCooldownManager
{
	std::vector<ActiveCooldown> cooldownList;
	int HasCooldown(int category);
	void AddCooldown(int category, unsigned long durationMS, unsigned long timeElapsedMS);
	void Clear(void);
	void CopyFrom(const ActiveCooldownManager &source);
	void WriteEntity(AbstractEntityWriter *writer);
	void LoadEntry(const char *name, unsigned long remainTimeMS, unsigned long timeElapsedMS);
};

#endif //#ifndef ABILITYTIME_H
