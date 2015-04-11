this.require("EffectDef");
class this.EffectDef.HealingPotion extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingPotion";
	static mParticleCue = 0.5;
	static mParticleLife = 0.69999999;
	static mSpinSpeed = 1.2;
	static mSoundLife = 3.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local anim = this.createGroup("DrinkAnimation", this.getSource());
		anim.add("FFAnimation", {
			animation = "Potion_Drink",
			events = {
				[this.mParticleCue] = "onHeal"
			}
		});
		anim.add("Mesh", {
			mesh = "Item-Bottle.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		local sound = this.createGroup("Sound", this.getSource());
		sound.add("Sound", {
			sound = "Sound-Spell-Healingpotion.ogg"
		});
		this.fireIn(this.mSoundLife, "onDone");
	}

	function onHeal( ... )
	{
		local spinner = this.createGroup("GreenSpinner");
		spinner.add("ParticleSystem", {
			particleSystem = "Par-Healing-2GlowingGlobes",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		spinner.add("Spin", {
			speed = this.mSpinSpeed
		});
		local heal = this.createGroup("GreenEnergy", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-Healing-Imbue",
			emitterPoint = "core",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		heal.add("ParticleSystem", {
			particleSystem = "Par-Healing-Cross",
			emitterPoint = "secondary",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		this.fireIn(this.mParticleLife, "onParticleDone");
	}

	function onParticleDone( ... )
	{
		this.get("DrinkAnimation").finish();
		this.get("GreenSpinner").finish();
		this.get("GreenEnergy").finish();
		this.get("Sound").finish();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.HealingPotion1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingPotion1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local drink = this.createGroup("Drink", this.getSource());
		drink.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		drink.add("Mesh", {
			mesh = "Item-Bottle.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onBuff");
		this.fireIn(1.0, "onDone");
	}

	function onBuff( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealingPotion1_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealingPotion1_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealingPotion1_Symbol",
			emitterPoint = "node"
		});
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

class this.EffectDef.HealingPotion2 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingPotion2";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local drink = this.createGroup("Drink", this.getSource());
		drink.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		drink.add("Mesh", {
			mesh = "Item-Bottle.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onBuff");
		this.fireIn(1.0, "onDone");
	}

	function onBuff( ... )
	{
		local buff = this.createGroup("Buff");
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealingPotion2_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealingPotion2_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealingPotion2_Symbol",
			emitterPoint = "node"
		});
		buff.add("Spin", {
			axis = "y",
			speed = 0.75,
			extraStopTime = 1.0
		});
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

class this.EffectDef.ConstitutionPotion1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ConstitutionPotion1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local drink = this.createGroup("Drink", this.getSource());
		drink.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		drink.add("Mesh", {
			mesh = "Item-PotionConstitution.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onBuff");
		this.fireIn(1.0, "onDone");
	}

	function onBuff( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstitutionPotion1_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstitutionPotion1_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstitutionPotion1_Symbol",
			emitterPoint = "node"
		});
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

class this.EffectDef.HealthRegenPotion1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealthRegenPotion1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local drink = this.createGroup("Drink", this.getSource());
		drink.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		drink.add("Mesh", {
			mesh = "Item-PotionHealthRegen.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onBuff");
		this.fireIn(1.0, "onDone");
	}

	function onBuff( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealthRegenPotion1_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealthRegenPotion1_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-HealthRegenPotion1_Symbol",
			emitterPoint = "node"
		});
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

class this.EffectDef.ArmorBoostPotion1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ArmorBoostPotion1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local drink = this.createGroup("Drink", this.getSource());
		drink.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		drink.add("Mesh", {
			mesh = "Item-PotionArmor.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onBuff");
		this.fireIn(1.0, "onDone");
	}

	function onBuff( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ArmorBoostPotion1_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ArmorBoostPotion1_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ArmorBoostPotion1_Symbol",
			emitterPoint = "node"
		});
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

class this.EffectDef.SpeedPotion1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpeedPotion1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local drink = this.createGroup("Drink", this.getSource());
		drink.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		drink.add("Mesh", {
			mesh = "Item-PotionSpeed.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onBuff");
		this.fireIn(1.0, "onDone");
	}

	function onBuff( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-SpeedPotion1_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-SpeedPotion1_Sparkle",
			emitterPoint = "node"
		});
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

