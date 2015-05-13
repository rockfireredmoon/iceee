#ifndef ABILITYTIME_H
#define ABILITYTIME_H

//Manages buffs and cooldowns.
#include <vector>
#include <stdio.h>

struct ActiveBuff
{
	unsigned char tier;
	unsigned char buffType;
	short abID;
	short abgID;
	int durationS;
	unsigned long castEndTimeMS;
};

struct ActiveBuffManager
{
	std::vector<ActiveBuff> buffList;
	int HasBuff(unsigned char tier, unsigned char buffType);
	int HasBuffNot(unsigned char tier, unsigned char buffType, int abilityGroup);
	void UpdateBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration);
	void SaveToStream(FILE *output);
	void AddBuff(unsigned char tier, unsigned char buffType, short abID, short abgID, double duration);
	void RemoveBuff(int abilityID);
	void DebugLogBuffs(const char *label);
	void CopyFrom(const ActiveBuffManager &source);
	void Clear(void);
};

struct ActiveCooldown
{
	int category;
	int durationMS;
	unsigned long castStartTimeMS;
	unsigned long castEndTimeMS;
	int GetRemainTimeMS(void);
	int GetElapsedTimeMS(void);
};

struct ActiveCooldownManager
{
	std::vector<ActiveCooldown> cooldownList;
	int HasCooldown(int category);
	void AddCooldown(int category, int durationMS, int timeElapsedMS);
	void Clear(void);
	void CopyFrom(const ActiveCooldownManager &source);
	void SaveToStream(FILE *output);
	void LoadEntry(const char *name, int remainTimeMS, int timeElapsedMS);
};

#endif //#ifndef ABILITYTIME_H
