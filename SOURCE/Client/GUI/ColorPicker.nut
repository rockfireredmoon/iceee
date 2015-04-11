this.require("GUI/Component");
this.require("GUI/Panel");
this.require("GUI/ColorSelector");
this.ColorRestrictionType <- {
	DEVELOPMENT = 0,
	REFASHION = 1,
	CHARACTER_CREATION = 2
};
this.ColorRestrictionInfo <- {
	[this.ColorRestrictionType.DEVELOPMENT] = {
		wheelLevel = 0,
		valueBar = 0
	},
	[this.ColorRestrictionType.REFASHION] = {
		wheelLevel = 1,
		valueBar = 1
	},
	[this.ColorRestrictionType.CHARACTER_CREATION] = {
		wheelLevel = 1,
		valueBar = 3
	}
};
class this.GUI.ColorPicker extends this.GUI.Component
{
	mSplotch = null;
	mLabel = null;
	mCurrent = null;
	mName = null;
	mDefault = null;
	mRollout = null;
	mChangeMessage = null;
	mRestrictType = this.ColorRestrictionType.DEVELOPMENT;
	constructor( vTitle, vName, vRestricted, ... )
	{
		this.GUI.Component.constructor();

		if (this.Util.hasPermission("dev"))
		{
			this.mRestrictType = this.ColorRestrictionType.DEVELOPMENT;
		}
		else
		{
			this.mRestrictType = vRestricted;
		}

		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setInsets(3, 3, 3, 3);
		this.mName = vName;
		this.mDefault = null;
		this.mSplotch = this.GUI.ColorSplotch(this.Color("00000000"));
		this.mSplotch.setDragAndDropEnabled(true);
		this.mSplotch.setReleaseMessage("onSplotchClick");
		this.mSplotch.addActionListener(this);
		this.add(this.mSplotch, this.GUI.BorderLayout.WEST);

		if (vargc > 0)
		{
			local color = vargv[0];
			this.mDefault = this.Color(vargv[0]);
			this.mCurrent = this.mDefault;
			this.setColor(this.mCurrent);
		}
		else
		{
			this.mCurrent = this.mSplotch.getColor();
			this.mDefault = this.mCurrent;
		}

		this.mLabel = this.GUI.Label(vTitle);
		this.mLabel.setInsets(0, 0, 0, 4);
		this.add(this.mLabel, this.GUI.BorderLayout.CENTER);
		this.mLabel.setVisible(false);
	}

	function setLabelVisible( showLabel )
	{
		this.mLabel.setVisible(showLabel);
	}

	function setLabelFont( font )
	{
		this.mLabel.setFont(font);
	}

	function setDefault( vDefault )
	{
		this.mDefault = vDefault;
	}

	function getDefault()
	{
		return this.mDefault;
	}

	function addActionListener( vListener )
	{
		this.mMessageBroadcaster.addListener(vListener);
	}

	function removeActionListener( vListener )
	{
		this.mMessageBroadcaster.removeListener(vListener);
	}

	function setChangeMessage( vMessage )
	{
		this.mChangeMessage = vMessage;
	}

	function _removeNotify()
	{
		this._closeRollout();
		this.GUI.Component._removeNotify();
	}

	function _fireActionPerformed( vMessage )
	{
		if (vMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(vMessage, this);
		}
	}

	function onSplotchClick( vSplotch )
	{
		this._openRollout();
	}

	function onSplotchUpdated( vSplotch )
	{
		this.setColor(vSplotch.getColor());
	}

	function _openRollout()
	{
		if (this.mRollout != null)
		{
			return;
		}

		this.mRollout = this.GUI.ColorPickerRollout(this, this.mCurrent, this.mRestrictType);
		this.mRollout.validate();
		local pos = this.getScreenPosition();
		pos.y += this.getHeight() - 1;
		pos.x += 15;
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;

		if (pos.x + this.mRollout.getWidth() > ::Screen.getWidth())
		{
			pos.x = ::Screen.getWidth() - this.mRollout.getWidth();
		}

		if (pos.y + this.mRollout.getHeight() > ::Screen.getHeight())
		{
			pos.y = ::Screen.getHeight() - this.mRollout.getHeight();
		}

		this.mRollout.setPosition(pos);
		this.GUI._Manager.addTransientToplevel(this.mRollout);
		this.mRollout.setOverlay(this.GUI.ROLLOUT_OVERLAY);
	}

	function onRolloutSelect( vColor )
	{
		this.setColor(vColor);
	}

	function getColor()
	{
		return this.mCurrent;
	}

	function setColor( vColor, ... )
	{
		if (this.type(vColor).tolower() == "string")
		{
			this.mCurrent = ::Color(vColor);
		}
		else
		{
			this.mCurrent = vColor;
		}

		this.mSplotch.setColor(this.mCurrent);

		if (!(vargc > 0 && vargv[0] == true))
		{
			this._fireActionPerformed(this.mChangeMessage);
		}
	}

	function _closeRollout()
	{
		if (this.mRollout)
		{
			this.mRollout.setOverlay(null);
			this.mRollout.destroy();
			this.mRollout = null;
			this._fireActionPerformed("onRolloutClosed");
		}
	}

	function getCurrent()
	{
		if (this.mCurrent != null)
		{
			return this.mCurrent.toHexString();
		}

		return this.mCurrent;
	}

	function getName()
	{
		return this.mName;
	}

}

class this.GUI.ColorPickerRollout extends this.GUI.Panel
{
	mBrightnessPalette = [];
	mHuePalette = [];
	mHueSelection = null;
	mBrightnessSelection = null;
	mBrightness = 1;
	mHue = 0;
	mSaturation = 1;
	mParent = null;
	mSwatchSize = {
		x = 15,
		y = 17,
		yspace = 12
	};
	mCloseButton = null;
	mRestrictType = this.ColorRestrictionType.DEVELOPMENT;
	mWidth = 210;
	mHeight = 227;
	mFudge = 5;
	MAX_BRIGHTNESS_BAR = 13.0;
	constructor( vParent, vCurrent, vRestricted )
	{
		this.GUI.Panel.constructor();
		this.setLayoutManager(null);
		this.mParent = vParent;
		this.mWidth = 210;
		this.mHeight = 227;
		this.mFudge = 5;
		this.setSize(this.mWidth, this.mHeight);
		this.mCloseButton = this.GUI.Button("Close");

		if (this.Util.hasPermission("dev"))
		{
			this.mRestrictType = this.ColorRestrictionType.DEVELOPMENT;
		}
		else
		{
			this.mRestrictType = vRestricted;
		}

		this._build();

		if (vCurrent != null)
		{
			this.findClosest(vCurrent);
		}
		else
		{
			this.findClosest(this.Color(1, 1, 1, 0));
		}
	}

	function destroy()
	{
		this.mBrightnessPalette.clear();
		this.mHuePalette.clear;
	}

	function onOutsideClick( vEvent )
	{
		this.mParent._closeRollout();
	}

	function onHueSelect( vSplotch )
	{
		if (this.mParent && "onRolloutSelect" in this.mParent)
		{
			this.setHueSelection(vSplotch);
			local cx = 6 * this.mSwatchSize.x - 6 * (this.mSwatchSize.x / 2);
			local cy = 6 * this.mSwatchSize.yspace;
			local pos = vSplotch.getPosition();
			local polar = this._getPolar(pos.x - 50 - cx, pos.y - 10 - cy);
			this.mHue = polar.a / (this.PI * 2);
			this.mSaturation = polar.d / (this.mSwatchSize.x * 5.5);
			this._updateBrightnessPalette();
			this.mParent.onRolloutSelect(vSplotch.getColor());
		}
	}

	function onBrightnessSelect( vSplotch )
	{
		if (this.mParent && "onRolloutSelect" in this.mParent)
		{
			this.setBrightnessSelection(vSplotch);
			local dat = vSplotch.getColor().getHSB();
			this.mBrightness = dat.l;
			this._updateHuePalette();
			this.mParent.onRolloutSelect(vSplotch.getColor());
		}
	}

	function _getPolar( vX, vY )
	{
		return {
			d = this.sqrt(vX * vX + vY * vY),
			a = this.atan2(vY, vX)
		};
	}

	function _build()
	{
		local palettecontainer = this.GUI.Container();
		local sz = this.getSize();
		sz.height -= this.mCloseButton.getHeight() + this.mFudge;
		palettecontainer.setSize(sz);
		this.add(palettecontainer);
		this.mCloseButton.setPosition((this.mWidth - this.mCloseButton.getWidth()) / 2, this.mHeight - this.mCloseButton.getHeight() - this.mFudge);
		this.mCloseButton.setReleaseMessage("onClose");
		this.mCloseButton.addActionListener(this);
		this.add(this.mCloseButton);
		local splotch;
		local cx = 6 * this.mSwatchSize.x - 6 * (this.mSwatchSize.x / 2);
		local cy = 6 * this.mSwatchSize.yspace;
		local startY = 0;
		local endY = 13;
		local midY = endY / 2;
		local wheelLevel = this.ColorRestrictionInfo[this.mRestrictType].wheelLevel;
		startY = startY + wheelLevel;
		endY = endY - wheelLevel;
		local startX = 0;
		startX = startX + wheelLevel;

		for( local y = startY; y < endY; y++ )
		{
			local m = midY + 1 + y - wheelLevel;

			if (y > midY)
			{
				m = endY - (y - midY);
			}

			for( local x = startX; x < m; x++ )
			{
				local fx;

				if (y < 6)
				{
					fx = x * this.mSwatchSize.x - y * (this.mSwatchSize.x / 2);
				}
				else
				{
					fx = x * this.mSwatchSize.x - (6 - (y - 6)) * (this.mSwatchSize.x / 2);
				}

				local fy = y * this.mSwatchSize.yspace;
				splotch = this.GUI.ColorSplotch(this.Color(1, 1, 1, 1), false);
				splotch.setReleaseMessage("onHueSelect");
				splotch.setAppearance("ColorSplotch/Hex");
				splotch.addActionListener(this);
				splotch.setSize(this.mSwatchSize.x, this.mSwatchSize.y);
				splotch.setLayoutExclude(true);
				splotch.setPosition(fx + 50, fy + 10);
				palettecontainer.add(splotch);
				this.mHuePalette.append(splotch);
			}
		}

		local start = 0;
		local end = this.MAX_BRIGHTNESS_BAR;
		start = start + this.ColorRestrictionInfo[this.mRestrictType].valueBar;
		end = end - this.ColorRestrictionInfo[this.mRestrictType].valueBar;

		for( local x = start; x < end; x++ )
		{
			local fx = x * this.mSwatchSize.x;
			splotch = this.GUI.ColorSplotch(this.Color(1, 1, 1, 1), false);
			splotch.setReleaseMessage("onBrightnessSelect");
			splotch.setAppearance("ColorSplotch/Hex");
			splotch.addActionListener(this);
			splotch.setSize(this.mSwatchSize.x, this.mSwatchSize.y);
			splotch.setLayoutExclude(true);
			splotch.setPosition(fx + 7, this.mSwatchSize.yspace * 14 + 10);
			palettecontainer.add(splotch);
			this.mBrightnessPalette.append(splotch);
		}
	}

	function onClose( evt )
	{
		this.mParent._closeRollout();
	}

	function _updateBrightnessPalette()
	{
		local startingInitialBrightness = 0;
		startingInitialBrightness = this.ColorRestrictionInfo[this.mRestrictType].valueBar;

		foreach( splotch in this.mBrightnessPalette )
		{
			local color = this.Color();
			color.setHSB(this.mHue, this.mSaturation, startingInitialBrightness / (this.MAX_BRIGHTNESS_BAR - 1.0));
			splotch.setColor(color);
			startingInitialBrightness++;
		}
	}

	function _updateHuePalette()
	{
		local cx = 6 * this.mSwatchSize.x - 6 * (this.mSwatchSize.x / 2);
		local cy = 6 * this.mSwatchSize.yspace;
		local v = 0;

		foreach( splotch in this.mHuePalette )
		{
			local pos = splotch.getPosition();
			local color = this.Color();
			local polar = this._getPolar(pos.x - 50 - cx, pos.y - 10 - cy);
			color.setHSB(polar.a / (this.PI * 2), polar.d / (this.mSwatchSize.x * 5.5), this.mBrightness);
			splotch.setColor(color);
			v++;
		}
	}

	function setHueSelection( component )
	{
		if (component == null)
		{
			this.remove(this.mHueSelection);
			this.mHueSelection.destroy();
			this.mHueSelection = null;
			return;
		}

		if (this.mHueSelection == null)
		{
			this.mHueSelection = this.GUI.Component();
			this.mHueSelection.setAppearance("ColorSplotch/Hex/Selection");
			this.mHueSelection.setSize(this.getWidth(), this.getHeight());
			this.mHueSelection.setBlendColor(this.Color(1, 1, 0.80000001, 1));
			this.mHueSelection.setLayoutExclude(true);
			this.mHueSelection.setSize(this.mSwatchSize.x, this.mSwatchSize.y);
			this.add(this.GUI.ZOffset(1, this.mHueSelection));
		}

		this.mHueSelection.setPosition(component.getPosition());
	}

	function setBrightnessSelection( component )
	{
		if (component == null)
		{
			this.remove(this.mBrightnessSelection);
			this.mBrightnessSelection.destroy();
			this.mBrightnessSelection = null;
			return;
		}

		if (this.mBrightnessSelection == null)
		{
			this.mBrightnessSelection = this.GUI.Component();
			this.mBrightnessSelection.setAppearance("ColorSplotch/Hex/Selection");
			this.mBrightnessSelection.setSize(this.getWidth(), this.getHeight());
			this.mBrightnessSelection.setBlendColor(this.Color(1, 1, 0.80000001, 1));
			this.mBrightnessSelection.setLayoutExclude(true);
			this.mBrightnessSelection.setSize(this.mSwatchSize.x, this.mSwatchSize.y);
			this.add(this.GUI.ZOffset(1, this.mBrightnessSelection));
		}

		this.mBrightnessSelection.setPosition(component.getPosition());
	}

	function findClosest( vColor )
	{
		local currenthsb;
		currenthsb = vColor.getHSB();
		local closesthue;
		local closesthuedist = 0;
		local closestbrightness;
		local closestbrightnessdist = 1000;
		local cx = 6 * this.mSwatchSize.x - 6 * (this.mSwatchSize.x / 2);
		local cy = 6 * this.mSwatchSize.yspace;

		foreach( splotch in this.mHuePalette )
		{
			local pos = splotch.getPosition();
			local color = this.Color();
			local polar = this._getPolar(pos.x - 50 - cx, pos.y - 10 - cy);
			local splotchHSB = splotch.getColor().getHSB();
			local hue = splotchHSB.h;
			local sat = splotchHSB.s;
			local huedist = hue - currenthsb.h;
			local satdist = sat - currenthsb.s;
			huedist = this.sqrt(huedist * huedist + satdist * satdist);

			if (closesthue == null)
			{
				closesthue = splotch;
				closesthuedist = huedist;
			}
			else if (currenthsb.h == 0 && currenthsb.s == 0 && hue == 0 && sat == 0)
			{
				closesthue = splotch;
				break;
			}
			else if (huedist <= closesthuedist)
			{
				closesthue = splotch;
				closesthuedist = huedist;
			}
		}

		local startingInitialBrightness = 0;
		startingInitialBrightness = this.ColorRestrictionInfo[this.mRestrictType].valueBar;

		foreach( splotch in this.mBrightnessPalette )
		{
			local color = this.Color();
			local bright = startingInitialBrightness / (this.MAX_BRIGHTNESS_BAR - 1.0);

			if (this.fabs(bright - currenthsb.l) <= closestbrightnessdist)
			{
				closestbrightness = splotch;
				closestbrightnessdist = this.fabs(bright - currenthsb.l);
			}

			startingInitialBrightness++;
		}

		this.setHueSelection(closesthue);
		local cx = 6 * this.mSwatchSize.x - 6 * (this.mSwatchSize.x / 2);
		local cy = 6 * this.mSwatchSize.yspace;
		local pos = closesthue.getPosition();
		local polar = this._getPolar(pos.x - 50 - cx, pos.y - 10 - cy);
		this.mHue = polar.a / (this.PI * 2);
		this.mSaturation = polar.d / (this.mSwatchSize.x * 5.5);
		this._updateBrightnessPalette();
		this.setBrightnessSelection(closestbrightness);
		local dat = closestbrightness.getColor().getHSB();
		this.mBrightness = dat.l;
		this._updateHuePalette();
		return closestbrightness;
	}

}

