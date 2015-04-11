class this.GUI.ExperienceBar extends this.GUI.Component
{
	static nClassName = "ExperienceBar";
	mBarContainerWidth = 642;
	mBarContainerHeight = 8;
	mExperienceBarMaxWidth = 642;
	mExperienceBarHeight = 8;
	mExperiencePointsLabel = null;
	mExperienceBarStartingX = 0;
	mExperienceBarStartingY = 0;
	mExperienceBarMiddleXPos = 642 / 2;
	mExperienceBarMiddleYPos = 2;
	mExperienceBar = null;
	mConstructed = false;
	mStartingX = 0;
	constructor()
	{
		this.GUI.Container.constructor(null);
		this.mExperienceBar = this.GUI.Container();
		this.mExperienceBar.setPreferredSize(this.mExperienceBarMaxWidth, this.mExperienceBarHeight);
		this.mExperienceBar.setSize(this.mExperienceBarMaxWidth, this.mExperienceBarHeight);
		this.mExperienceBar.setPosition(this.mExperienceBarStartingX, this.mExperienceBarStartingY);
		this.mExperienceBar.setAppearance("ExperienceBar");
		this.setSize(this.mBarContainerWidth, this.mBarContainerHeight);
		local screenHeight = ::Screen.getHeight();
		this.setPosition(this.mStartingX, screenHeight - this.getSize().height);
		local font = ::GUI.Font("Maiandra", 12);
		this.mExperiencePointsLabel = this.GUI.Label("50 / 100");
		this.mExperiencePointsLabel.setFontColor("ffffff");
		this.mExperiencePointsLabel.setFont(font);
		this.add(this.mExperienceBar);
		this.add(this.mExperiencePointsLabel);
		this.mConstructed = true;
		this.setChildrenInheritTooltip(true);
		this.fillOut();
		this.setCached(::Pref.get("video.UICache"));
	}

	function fillOut()
	{
		if (!::_avatar)
		{
			return;
		}

		local avatarLevel = ::_avatar.getStat(this.Stat.LEVEL);

		if (!avatarLevel || avatarLevel > 70)
		{
			return;
		}

		local totalExpLastLevel = 0;

		for( local i = 1; i < avatarLevel; i++ )
		{
			totalExpLastLevel += this.LevelRequirements[i];
		}

		local xpToLevel = this.LevelRequirements[avatarLevel];
		local curXP = ::_avatar.getStat(this.Stat.EXPERIENCE) - totalExpLastLevel;
		this.setTooltip("Experience for next level: " + curXP + "/" + xpToLevel);
		this.setExperience(curXP, xpToLevel);
	}

	function setExperience( currentXP, xpNeededToLevel )
	{
		local percent = currentXP.tofloat() / xpNeededToLevel.tofloat();

		if (percent > 1)
		{
			percent = 1.0;
		}

		this.mExperienceBar.setPreferredSize(this.mExperienceBarMaxWidth * percent, this.mExperienceBarHeight);
		this.mExperienceBar.setSize(this.mExperienceBarMaxWidth * percent, this.mExperienceBarHeight);
		this.mExperiencePointsLabel.setText(currentXP + " / " + xpNeededToLevel);
		this._centerXPText();
	}

	function setVisible( value )
	{
		this.GUI.Container.setVisible(value);

		if (this.mConstructed)
		{
			this.fillOut();
		}
	}

	function _centerXPText()
	{
		local font = this.mExperiencePointsLabel.getFont();
		local fontMetrics = font.getTextMetrics(this.mExperiencePointsLabel.getText());
		this.mExperiencePointsLabel.setPosition(this.mExperienceBarMiddleXPos - fontMetrics.width / 2, this.mExperienceBarMiddleYPos - fontMetrics.height / 2);
	}

}

