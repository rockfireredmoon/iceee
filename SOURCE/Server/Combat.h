//Some helper functions for combat.

#ifndef COMBAT_H
#define COMBAT

namespace Combat
{
	int GetBlockStatBonus(int dexterity);
	int GetParryStatBonus(int dexterity);
	int GetDodgeStatBonus(int dexterity);
	int GetSpiritResistReduction(int damage, int spirit);
	int GetPsycheResistReduction(int damage, int psyche);
} //namespace Combat

#endif //#ifndef COMBAT_H