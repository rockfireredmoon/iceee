
#include "GameConfig.h"
#include "Combat.h"

namespace Combat
{

int GetBlockStatBonus(int dexterity)
{
	if(g_GameConfig.DexBlockDivisor == 0.0F)
		return 0;
	float calc = (dexterity / g_GameConfig.DexBlockDivisor) * 10.0F;  //10 = 1% chance
	return static_cast<int>(calc);
}

int GetParryStatBonus(int dexterity)
{
	if(g_GameConfig.DexParryDivisor == 0.0F)
		return 0;
	float calc = (dexterity / g_GameConfig.DexParryDivisor) * 10.0F;  //10 = 1% chance
	return static_cast<int>(calc);
}

int GetDodgeStatBonus(int dexterity)
{
	if(g_GameConfig.DexDodgeDivisor == 0.0F)
		return 0;
	float calc = (dexterity / g_GameConfig.DexDodgeDivisor) * 10.0F;  //10 = 1% chance
	return static_cast<int>(calc);
}

int GetSpiritResistReduction(int damage, int spirit)
{
	if(g_GameConfig.SpiResistDivisor == 0.0F)
		return 0;
	float calc = ((spirit / g_GameConfig.SpiResistDivisor) / 100.0F);
	if(calc < 0.0F)
		calc = 0.0F;
	else if(calc > 1.0F)
		calc = 1.0F;
	calc = damage * calc;
	return static_cast<int>(calc);
}

int GetPsycheResistReduction(int damage, int psyche)
{
	if(g_GameConfig.PsyResistDivisor == 0.0F)
		return 0;
	float calc = ((psyche / g_GameConfig.PsyResistDivisor) / 100.0F);
	if(calc < 0.0F)
		calc = 0.0F;
	else if(calc > 1.0F)
		calc = 1.0F;
	calc = damage * calc;
	return static_cast<int>(calc);
}

} //namespace Combat

