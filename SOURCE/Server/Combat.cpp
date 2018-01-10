#include "Combat.h"
#include "Config.h"

namespace Combat
{

int GetBlockStatBonus(int dexterity)
{
	if(g_Config.DexBlockDivisor == 0.0F)
		return 0;
	float calc = (dexterity / g_Config.DexBlockDivisor) * 10.0F;  //10 = 1% chance
	return static_cast<int>(calc);
}

int GetParryStatBonus(int dexterity)
{
	if(g_Config.DexParryDivisor == 0.0F)
		return 0;
	float calc = (dexterity / g_Config.DexParryDivisor) * 10.0F;  //10 = 1% chance
	return static_cast<int>(calc);
}

int GetDodgeStatBonus(int dexterity)
{
	if(g_Config.DexDodgeDivisor == 0.0F)
		return 0;
	float calc = (dexterity / g_Config.DexDodgeDivisor) * 10.0F;  //10 = 1% chance
	return static_cast<int>(calc);
}

int GetSpiritResistReduction(int damage, int spirit)
{
	if(g_Config.SpiResistDivisor == 0.0F)
		return 0;
	float calc = ((spirit / g_Config.SpiResistDivisor) / 100.0F);
	if(calc < 0.0F)
		calc = 0.0F;
	else if(calc > 1.0F)
		calc = 1.0F;
	calc = damage * calc;
	return static_cast<int>(calc);
}

int GetPsycheResistReduction(int damage, int psyche)
{
	if(g_Config.PsyResistDivisor == 0.0F)
		return 0;
	float calc = ((psyche / g_Config.PsyResistDivisor) / 100.0F);
	if(calc < 0.0F)
		calc = 0.0F;
	else if(calc > 1.0F)
		calc = 1.0F;
	calc = damage * calc;
	return static_cast<int>(calc);
}

} //namespace Combat

