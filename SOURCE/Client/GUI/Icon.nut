this.require("GUI/Button");
class this.GUI.Icon extends this.GUI.Button
{
	mImage = null;
	mImageName = null;
	mBackground = null;
	mBackgroundName = null;
	mSelectedBackground = null;
	mCountLabel = null;
	mHotKeyLabel = null;
	mTimer = null;
	mTime = 0;
	static mClassName = "Icon";
	static ICON_SIZE = 42;
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor("", vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor("");
		}

		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = true;
		this.setAppearance("Icon");
		this.setBackground("StandardGrey");
		this.setLayoutManager(null);
		this.setFixedSize(this.ICON_SIZE, this.ICON_SIZE);
		this.mImage = this.GUI.Component(null);
		this.mImage.setAppearance("Icon");
		this.mImage.setPosition(0, 0);
		this.mImage.setSize(this.ICON_SIZE, this.ICON_SIZE);
		this.setIconImage("QuestionMark");
		this.add(this.mImage);
		this.setSelection(true);
		this.mSelection.setPosition(0, 0);
		this.mSelection.setSize(this.ICON_SIZE, this.ICON_SIZE);
		this.remove(this.mSelection);
		this.mImage.add(this.mSelection);
		local font = this.GUI.Font("MaiandraOutline", 16);
		this.mLabel.setFont(font);
		this.mLabel.setFontColor(this.GUI.DefaultButtonFontColor);
		this.remove(this.mLabel);
		this.mImage.add(this.mLabel);
		this.mCountLabel = this.GUI.Label("");
		this.mCountLabel.setVisible(true);
		this.mCountLabel.setFont(font);
		this.mImage.add(this.mCountLabel);
		this.setCountLabel(null);
		font = this.GUI.Font("MaiandraOutline", 12);
		this.mHotKeyLabel = this.GUI.Label("");
		this.mHotKeyLabel.setVisible(true);
		this.mHotKeyLabel.setFont(font);
		this.mHotKeyLabel.setPosition(0, 0);
		this.mImage.add(this.mHotKeyLabel);
		this.setHotKeyLabel(null);
	}

	function setIconSize( width, height )
	{
		this.setFixedSize(width, height);
		this.mImage.setSize(width, height);
		this.mSelection.setSize(width, height);
	}

	function setText( pText )
	{
	}

	function getText()
	{
		return null;
	}

	function setTimeLabel( pMillisec )
	{
		local Time = 0;

		if (pMillisec == 0)
		{
			this.mLabel.setText("");
			return;
		}

		local timeText = "";

		if (pMillisec > 60000)
		{
			Time = pMillisec / 60000;
			timeText = Time.tointeger().tostring() + "m";
		}
		else
		{
			Time = pMillisec / 1000;

			if (Time.tointeger() == 0)
			{
				timeText = "";
			}
			else
			{
				timeText = Time.tointeger().tostring() + "s";
			}
		}

		if (timeText != this.mLabel.getText())
		{
			this.mLabel.setText(timeText);
		}

		local sz = this.mLabel.getPreferredSize();
		local l = this.getPreferredSize().width / 2;
		local t = this.getPreferredSize().height / 2 + sz.height / 2 * -1 - 1;
		this.mLabel.setPosition(l.tointeger(), t.tointeger());
	}

	function setIcon( image, background )
	{
		this.setIconImage(image);
		this.setBackground(background);
	}

	function setTime( pMillisec )
	{
		this.mTimer = ::Timer();
		this.mTimer.reset();
		this.mTime = pMillisec;
		this._enterFrameRelay.addListener(this);
	}

	function setCountLabel( pCount )
	{
		if (pCount == null)
		{
			this.mCountLabel.setVisible(false);
			this.mCountLabel.setText("");
		}
		else
		{
			this.mCountLabel.setVisible(true);
			this.mCountLabel.setText(pCount.tostring());
		}

		local sz = this.mCountLabel.getPreferredSize();
		local l = this.getPreferredSize().width + sz.width * -1 - 2;
		local t = this.getPreferredSize().height + sz.height * -1;
		this.mCountLabel.setPosition(l.tointeger(), t.tointeger());
	}

	function setHotKeyLabel( pKeyName )
	{
		if (pKeyName == null)
		{
			this.mHotKeyLabel.setVisible(false);
			this.mHotKeyLabel.setText("");
		}
		else
		{
			this.mHotKeyLabel.setVisible(true);
			this.mHotKeyLabel.setText(pKeyName);
		}

		local sz = this.mHotKeyLabel.getPreferredSize();
		local l = 2;
		local t = -2;
		this.mHotKeyLabel.setPosition(l.tointeger(), t.tointeger());
	}

	function setBackground( name )
	{
		this.setMaterial(this.mAppearance + "/BG/" + name);
		this.mBackgroundName = name;
	}

	function getBackground()
	{
		return this.mBackgroundName;
	}

	function setSelectedBackground( background )
	{
		this.mSelectedBackground = background;
	}

	function getSelectedBackground()
	{
		return this.mSelectedBackground;
	}

	function setIconImage( name )
	{
		this.mImage.setMaterial(this.mAppearance + "/" + name);
		this.mImageName = name;
	}

	function getIconImage()
	{
		return this.mImageName;
	}

	function onEnterFrame()
	{
		if (this.mTimer)
		{
			if (this.mTime <= this.mTimer.getMilliseconds())
			{
				this.setTimeLabel(0);
				this.mTimer = null;
				this.mTime = 0;
				this._root.removeListener(this);
			}
			else
			{
				local time = this.mTime - this.mTimer.getMilliseconds();
				this.setTimeLabel(time);
			}
		}
	}

	function setToggled( value )
	{
		if (this.mSelectedBackground)
		{
			if (value)
			{
				this.setMaterial(this.mAppearance + "/BG/" + this.mSelectedBackground);
			}
			else
			{
				this.setMaterial(this.mAppearance + "/BG/" + this.mBackgroundName);
			}
		}
		else
		{
			this.mToggled = value;
			this.setSelectionVisible(this.mWidget != null && (this.mToggled || this.mHover));
		}
	}

}

