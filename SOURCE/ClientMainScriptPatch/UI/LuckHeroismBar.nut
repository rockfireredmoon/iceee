this.HeroismTypes <- {
	IRON = "Iron",
	COPPER = "Copper",
	BRONZE = "Bronze",
	STEEL = "Steel",
	GOLD = "Gold"
};
this.HeroismItems <- {
	[this.HeroismTypes.IRON] = {
		min = 0.0,
		max = 24.0
	},
	[this.HeroismTypes.COPPER] = {
		min = 25.0,
		max = 49.0
	},
	[this.HeroismTypes.BRONZE] = {
		min = 50.0,
		max = 74.0
	},
	[this.HeroismTypes.STEEL] = {
		min = 75.0,
		max = 99.0
	},
	[this.HeroismTypes.GOLD] = {
		min = 100.0,
		max = 100.0
	}
};
class this.GUI.LuckHeroismBar extends this.GUI.Component
{
	static nClassName = "LuckHeroismBar";
	static BASE_WIDTH = 58;
	static BASE_HEIGHT = 76;
	static HEROISM_WIDTH = 52;
	static HEROISM_HEIGHT = 72;
	static HEROISM_FILL_WIDTH = 20;
	static HEROISM_FILL_HEIGHT = 48;
	static LUCK_EMBLEM_WIDTH = 16;
	static LUCK_EMBLEM_HEIGHT = 14;
	static EXTRA_HEIGHT = 8;
	mConstructed = false;
	mHeroismFill = null;
	mLuckComponent = null;
	mHeroismContainer = null;
	mHeroismBGFill = null;
	constructor()
	{
		this.GUI.Container.constructor(null);
		this.setSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		this.setPreferredSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		this.mHeroismBGFill = this.GUI.Container(null);
		this.mHeroismBGFill.setSize(this.HEROISM_FILL_WIDTH, this.HEROISM_FILL_HEIGHT + this.EXTRA_HEIGHT);
		this.mHeroismBGFill.setPreferredSize(this.HEROISM_FILL_WIDTH, this.HEROISM_FILL_HEIGHT + this.EXTRA_HEIGHT);
		this.mHeroismBGFill.setPosition((this.BASE_WIDTH - this.HEROISM_FILL_WIDTH) / 2, 11);
		this.mHeroismBGFill.setTooltip("Heroism");
		this.mHeroismBGFill.setChildrenInheritTooltip(true);
		this.add(this.mHeroismBGFill);
		this.mHeroismFill = this.GUI.Container(null);
		this.mHeroismFill.setSize(this.HEROISM_FILL_WIDTH, this.HEROISM_FILL_HEIGHT + 10);
		this.mHeroismFill.setPreferredSize(this.HEROISM_FILL_WIDTH, this.HEROISM_FILL_HEIGHT + 10);
		this.mHeroismFill.setPosition(0, 0);
		this.mHeroismFill.setAppearance("HeroismBladeRed/Fill");
		this.mHeroismBGFill.add(this.mHeroismFill);
		this.mHeroismContainer = this.GUI.Container(null);
		this.mHeroismContainer.setSize(this.HEROISM_WIDTH, this.HEROISM_HEIGHT);
		this.mHeroismContainer.setPreferredSize(this.HEROISM_WIDTH, this.HEROISM_HEIGHT);
		this.mHeroismContainer.setPosition(-16, -13);
		this.mHeroismFill.add(this.mHeroismContainer);
		this.mLuckComponent = this.GUI.Component(null);
		this.mLuckComponent.setSize(this.LUCK_EMBLEM_WIDTH, this.LUCK_EMBLEM_HEIGHT);
		this.mLuckComponent.setPreferredSize(this.LUCK_EMBLEM_WIDTH, this.LUCK_EMBLEM_HEIGHT);
		this.mLuckComponent.setPosition(18.5, 58);
		this.mLuckComponent.setAppearance("LuckEmblem");
		this.mHeroismContainer.add(this.mLuckComponent);
		this.mConstructed = true;
		::_sceneObjectManager.addListener(this);
		this.fillOut();
	}

	function fillOut()
	{
		if (!::_avatar)
		{
			return;
		}

		local heroism = ::_avatar.getStat(this.Stat.HEROISM);
		local luck = ::_avatar.getStat(this.Stat.BASE_LUCK);
		local modLuck = ::_avatar.getStat(this.Stat.MOD_LUCK);

		if (modLuck && luck)
		{
			luck += luck * (modLuck / 100.0);
		}

		if (heroism)
		{
			this.setHeroism(heroism);
		}
		else
		{
			this.setHeroism(0);
		}

		if (luck)
		{
			this.setLuck((luck + 0.5).tointeger());
		}
		else
		{
			this.setLuck(0);
		}
	}

	function onAvatarSet()
	{
		this.fillOut();
		::_sceneObjectManager.removeListener(this);
	}

	function setHeroism( currentHeroism )
	{
		local percent = currentHeroism.tofloat() / this.MAX_HEROISM.tofloat() * 100.0;

		foreach( heroismType, heroismData in this.HeroismItems )
		{
			if (percent.tointeger() >= heroismData.min && percent.tointeger() <= heroismData.max)
			{
				this.mHeroismBGFill.setAppearance("HeroismBlade" + heroismType + "/Fill");
				this.mHeroismContainer.setAppearance("HeroismBlade" + heroismType);
				local difference = heroismData.max - heroismData.min;
				local fillPercent = 1.0;

				if (heroismData.max != 100)
				{
					fillPercent = (percent - heroismData.min) / (heroismData.max - heroismData.min);
				}

				local fillHeight = this.HEROISM_FILL_HEIGHT * fillPercent;
				local fillDiff = this.HEROISM_FILL_HEIGHT - fillHeight;
				this.mHeroismFill.setPreferredSize(this.HEROISM_FILL_WIDTH, fillHeight + this.EXTRA_HEIGHT);
				this.mHeroismFill.setSize(this.HEROISM_FILL_WIDTH, fillHeight + this.EXTRA_HEIGHT);
				this.mHeroismFill.setPosition(0, fillDiff);
				this.mHeroismContainer.setPosition(-16, -13 - fillDiff);
			}
		}
	}

	function setLuck( currentLuck )
	{
		this.mLuckComponent.setTooltip("Luck: " + currentLuck);
	}

	function setVisible( value )
	{
		this.GUI.Container.setVisible(value);

		if (this.mConstructed)
		{
			this.fillOut();
		}
	}

}

