this.require("Combat/CombatConstants");
class this.CombatEquations 
{
	function calcBlockRate( baseBlock, dex, str, mod )
	{
		if (null == baseBlock || null == dex || null == str)
		{
			return 0.0;
		}

		local block = (baseBlock + dex * this.BlockDexterityMod + str * this.BlockStrengthMod) / this.BlockMod;
		block += mod;
		return this.Util.limitSignificantDigits(block, 2);
	}

	function calcDamageReduction( armorRating, level )
	{
		if (!armorRating || null == level)
		{
			return 0.0;
		}

		local dr = armorRating.tofloat() / (armorRating + this.DamageReductionModiferPerLevel * level + this.DamageReductionAdditive).tofloat();
		return this.Util.limitSignificantDigits(dr, 2);
	}

	function calcDPS( damage, speed )
	{
		if (null == damage || !speed)
		{
			return 0.0;
		}

		local damage = damage / (speed * this.SecondsPerRoundInverse);
		return this.Util.limitSignificantDigits(damage, 2);
	}

	function calcWeaponDPS( weapon )
	{
		local itemDef;

		if (weapon.isValid())
		{
			if (weapon instanceof this.ItemDefData)
			{
				itemDef = weapon;
			}
			else
			{
				itemDef = weapon.mItemDefData;
			}

			if (itemDef.mWeaponType != this.WeaponType.NONE && itemDef.mWeaponType != this.WeaponType.ARCANE_TOTEM)
			{
				local avgdamage = (itemDef.mWeaponDamageMax - itemDef.mWeaponDamageMin) * 0.5 + itemDef.mWeaponDamageMin;
				local dps = avgdamage / itemDef.mWeaponSpeed + itemDef.mWeaponExtraDamangeRating;
				return this.Util.limitSignificantDigits(dps, 2);
			}
		}
		else
		{
			return 0.0;
		}
	}

	function calcDodge( baseDodge, dexterity )
	{
		if (null == dexterity || baseDodge == null)
		{
			return 0.0;
		}

		return baseDodge + dexterity * this.DodgeDexMod;
	}

	function calcHealthRegen( inCombat, spirit, con, level, mod )
	{
		if (null == spirit || null == con || null == level)
		{
			return 0.0;
		}

		local regen = spirit * this.RegenSpiritMod + con * this.RegenConMod + level * this.RegenLevelMod;

		if (inCombat)
		{
			regen *= this.RegenCombatModifier;
		}

		regen += regen * mod;
		regen /= 2.0;
		return this.Util.limitSignificantDigits(regen, 2);
	}

	function calcMagicCritRate( baseMagicCrit, psyche, mod )
	{
		if (null == baseMagicCrit || null == psyche)
		{
			return 0.0;
		}

		local critRate = baseMagicCrit + psyche * this.CritMagicPsycheMod;
		critRate += mod;

		if (critRate > 25.0)
		{
			critRate = 25.0;
		}

		return critRate;
	}

	function calcMagicHitRate( baseMagicHitRate, psyche, mod )
	{
		if (null == baseMagicHitRate || null == psyche)
		{
			return 0.0;
		}

		local hitChance = baseMagicHitRate * 0.80000001 + psyche * this.ToHitMagicPsycheMod;
		hitChance += mod;
		return this.Util.limitSignificantDigits(hitChance, 2);
	}

	function calcMaxHealth( baseHealth, constitution, bonusHealth, rarity )
	{
		if (null == baseHealth || null == constitution)
		{
			return 0;
		}

		if (bonusHealth == null)
		{
			bonusHealth = 0;
		}

		local rarityMultiplier = 1.0;

		switch(rarity)
		{
		case this.CreatureRarityType.NORMAL:
			break;

		case this.CreatureRarityType.HEROIC:
			rarityMultiplier = this.RarityTypeHealthModifier.HEROIC;
			break;

		case this.CreatureRarityType.EPIC:
			rarityMultiplier = this.RarityTypeHealthModifier.EPIC;
			break;

		case this.CreatureRarityType.LEGEND:
			rarityMultiplier = this.RarityTypeHealthModifier.LEGEND;
			break;
		}

		return (baseHealth + constitution * this.HealthConModifier) * rarityMultiplier + bonusHealth;
	}

	function calcMeleeCritRate( baseMeleeCrit, dex, mod )
	{
		if (null == baseMeleeCrit || null == dex)
		{
			return 0.0;
		}

		local critRate = baseMeleeCrit + dex * this.CritMeleeDexterityMod;
		critRate += mod;

		if (critRate > 25.0)
		{
			critRate = 25.0;
		}

		return critRate;
	}

	function calcMeleeHitRate( baseMeleeHitRate, dex, mod )
	{
		if (null == baseMeleeHitRate || null == dex)
		{
			return 0.0;
		}

		local hitChance = baseMeleeHitRate * 0.80000001 + dex * this.ToHitMeleeDexMod;
		hitChance += mod;
		return this.Util.limitSignificantDigits(hitChance, 2);
	}

	function calcParry( baseParry, dex, str, mod )
	{
		if (null == baseParry || null == dex || null == str)
		{
			return 0.0;
		}

		local parry = (baseParry + dex * this.ParryDexMod + str * this.ParryStrMod) / this.ParryMod;
		parry += mod;
		return this.Util.limitSignificantDigits(parry, 2);
	}

	function calcMagicDeflectRate( baseDeflect, psyche )
	{
		if (null == baseDeflect || null == psyche)
		{
			return 0.0;
		}

		return baseDeflect + psyche * this.DeflectPsycheMod;
	}

}

this._combatEquations <- this.CombatEquations();
