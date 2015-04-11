this.require("GUI/Component");
this.require("GUI/AnchorPanel");
this.require("GUI/StretchproofPanel");
class this.GUI.ColorSplotch extends this.GUI.Component
{
	mColor = null;
	mTexture = null;
	mHSplotch = null;
	mLSplotch = null;
	mFrame = null;
	mSelected = null;
	mReleaseMessage = null;
	mMouseInside = false;
	mDragAndDropEnabled = false;
	constructor( vColor, ... )
	{
		this.GUI.Component.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setAppearance("ColorSplotch");
		this.setSize(25, 25);
		this.setPreferredSize(25, 20);

		if (vargc == 0 || vargv[0] == true)
		{
			this.mFrame = this.GUI.Component();
			this.mFrame.setAppearance("ColorSplotch/Frame");
			this.mFrame.setSize(this.getWidth(), this.getHeight());
			this.mFrame.setPosition(0, 0);
			this.mFrame.setLayoutExclude(true);
			this.add(this.mFrame);
		}

		this.setColor(vColor);
	}

	function getColor()
	{
		return this.mColor;
	}

	function setColor( vColor )
	{
		if (typeof vColor == "string")
		{
			this.mColor = ::Color(vColor);
		}
		else
		{
			this.mColor = vColor;
		}

		this.setBlendColor(this.mColor);
	}

	function addActionListener( vListener )
	{
		this.mMessageBroadcaster.addListener(vListener);
	}

	function setPressMessage( message )
	{
		this.log.warn("ColorSplotch.setPressMessage is deprecated, use setReleaseMessage");
		this.DumpStackTrace();
		this.setReleaseMessage(message);
	}

	function setReleaseMessage( vMessage )
	{
		this.mReleaseMessage = vMessage;
	}

	function _fireActionPerformed( vMessage )
	{
		this.log.debug("Splotch Action Performed: " + vMessage);

		if (vMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(vMessage, this);
		}
	}

	function onMouseReleased( vEvent )
	{
		if (vEvent.button == this.MouseEvent.LBUTTON)
		{
			if (this.mMouseInside)
			{
				this._fireActionPerformed(this.mReleaseMessage);
			}

			vEvent.consume();
		}
	}

	function onMouseEnter( evt )
	{
		this.mMouseInside = true;
	}

	function onMouseExit( evt )
	{
		this.mMouseInside = false;
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();

		if (this.mFrame)
		{
			this.mFrame.setSize(this.getWidth(), this.getHeight());
		}
	}

	function isDragAndDropEnabled()
	{
		return this.mDragAndDropEnabled;
	}

	function setDragAndDropEnabled( value )
	{
		this.mDragAndDropEnabled = value;
	}

	function onDragRequested( dndevt )
	{
		if (this.mDragAndDropEnabled)
		{
			dndevt.acceptDrag(this, this.GUI.DnDEvent.ACTION_COPY_OR_MOVE);
		}
	}

	function onDragOver( dndevt )
	{
		local t = dndevt.getTransferable();

		if (t && (t instanceof this.GUI.ColorSplotch))
		{
			dndevt.acceptDrop(this.GUI.DnDEvent.ACTION_COPY_OR_MOVE);
		}
	}

	function onDrop( dndevt )
	{
		local t = dndevt.getTransferable();

		if (!t || !(t instanceof this.GUI.ColorSplotch))
		{
			throw this.Exception("Bad drop target");
		}

		local action = dndevt.getAction();
		this.log.debug("Dropping : " + t.getColor() + " with action = " + action);

		if (dndevt.isControlDown() && action & this.GUI.DnDEvent.ACTION_COPY)
		{
			this.setColor(t.getColor());
			this._fireActionPerformed("onSplotchUpdated");
			dndevt.consume();
		}
		else if (action & this.GUI.DnDEvent.ACTION_MOVE)
		{
			local c = t.getColor();
			t.setColor(this.getColor());
			this.setColor(c);
			t._fireActionPerformed("onSplotchUpdated");
			this._fireActionPerformed("onSplotchUpdated");
			dndevt.consume();
		}
	}

}

class this.GUI.ColorSelector extends this.GUI.Component
{
	mSplotch = null;
	mLabel = null;
	mCurrent = null;
	mName = null;
	mPalette = null;
	mShowGray = false;
	mDefault = null;
	mRollout = null;
	mChangeMessage = null;
	constructor( vName, ... )
	{
		this.GUI.Component.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setAppearance("Panel");
		this.setLayoutManager(this.GUI.GridLayout(1, 2));
		this.getLayoutManager().setColumns(25, "*");
		this.getLayoutManager().setRows(18);
		this.getLayoutManager().setGaps(5, 5);
		this.setInsets(3, 3, 3, 3);
		this.mName = vName;
		this.mPalette = [];
		this.mShowGray = false;
		this.mDefault = null;

		if (vargc > 0)
		{
			this.mPalette = vargv[0];
		}

		if (vargc > 1)
		{
			this.mShowGray = vargv[1];
		}

		if (vargc > 2)
		{
			this.mDefault = vargv[2];
		}

		this.mSplotch = this.GUI.ColorSplotch(null);
		this.mSplotch.setDragAndDropEnabled(true);
		this.mSplotch.setReleaseMessage("onSplotchClick");
		this.mSplotch.addActionListener(this);
		this.add(this.mSplotch);
		this.mLabel = this.GUI.Label(this.mName);
		this.mLabel.setPosition(31, 10);
		this.add(this.mLabel);
	}

	function setShowGray( vWhich )
	{
		this.mShowGray = vWhich;
	}

	function getShowGray()
	{
		return this.mShowGray;
	}

	function setPalette( vPalette )
	{
		this.mPalette = vPalette;
	}

	function getPalette()
	{
		return this.mPalette;
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

		this.mRollout = this.GUI.ColorSelectRollout(this, this.mPalette, this.mShowGray, this.mDefault);
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
		local i;

		if (this.mRollout == null)
		{
			throw "ColorSelect got message from a phantom rollout!";
			return;
		}

		this.setColor(vColor);
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

	function setName( vName )
	{
		this.mName = vName;
		this.mLabel.setText(vName);
	}

	function setFont( vFont )
	{
		this.mLabel.setFont(vFont);
	}

}

class this.GUI.ColorSelectRollout extends this.GUI.Panel
{
	mParent = null;
	mPalette = null;
	mShowGray = false;
	mDefault = null;
	mPaletteGroupDimentions = {
		x = 3,
		y = 3,
		size = 9
	};
	mSwatchSize = {
		x = 25,
		y = 20
	};
	mGroupColumns = 3;
	mPrimaryGrid = null;
	mGrayScaleGrid = null;
	mDefaultContainer = null;
	constructor( vParent, vPalette, vShowGrey, vDefault )
	{
		this.GUI.Panel.constructor();
		local layout = this.GUI.BoxLayoutV();
		this.setLayoutManager(layout);
		layout.setGap(5);
		layout.setAlignment(0);
		layout.setExpand(true);
		this.mParent = vParent;
		this.mPalette = vPalette;
		this.mShowGray = vShowGrey;
		this.mDefault = vDefault;
		this._build();
	}

	function onOutsideClick( vEvent )
	{
		this.mParent._closeRollout();
	}

	function onColorSelect( vSplotch )
	{
		if (this.mParent && "onRolloutSelect" in this.mParent)
		{
			this.mParent.onRolloutSelect(vSplotch.getColor());
		}
	}

	function _build()
	{
		local groups = this.mPalette.len() / this.mPaletteGroupDimentions.size;
		local rows = (groups / this.mGroupColumns).tointeger();

		if (rows < groups / this.mGroupColumns.tofloat())
		{
			rows++;
		}

		local height = rows * this.mPaletteGroupDimentions.y * this.mSwatchSize.y + 5;

		if (this.mShowGray)
		{
			height += 22;
		}

		if (this.mDefault != null)
		{
			height += 22;
		}

		this.setSize(this.mGroupColumns * this.mPaletteGroupDimentions.x * this.mSwatchSize.x + 4, height);
		this.mPrimaryGrid = this.GUI.Container(this.GUI.GridLayout(rows, this.mGroupColumns));
		this.mPrimaryGrid.getLayoutManager().setGaps(4, 5);
		this.mPrimaryGrid.setPreferredSize(this.mGroupColumns * this.mPaletteGroupDimentions.y * this.mSwatchSize.x, rows * this.mPaletteGroupDimentions.x * this.mSwatchSize.y - 5);
		this.add(this.mPrimaryGrid);

		for( local i = 0; i < groups; i++ )
		{
			local groupcontainer = this.GUI.Container(this.GUI.GridLayout(this.mPaletteGroupDimentions.y, this.mPaletteGroupDimentions.x));

			for( local x = 0; x < this.mPaletteGroupDimentions.x; x++ )
			{
				for( local y = 0; y < this.mPaletteGroupDimentions.x; y++ )
				{
					local colindex = i * this.mPaletteGroupDimentions.size + x * this.mPaletteGroupDimentions.y + y;

					if (colindex > this.mPalette.len())
					{
						break;
					}

					local swatch = this.GUI.ColorSplotch(this.mPalette[colindex]);
					swatch.setReleaseMessage("onColorSelect");
					swatch.addActionListener(this);
					groupcontainer.add(swatch);
				}
			}

			this.mPrimaryGrid.add(groupcontainer);
		}

		if (this.mShowGray)
		{
			local greys = this.ColorPalette.grayscale2d.len();
			this.mGrayScaleGrid = this.GUI.Container(this.GUI.GridLayout(1, this.mGroupColumns));
			this.mGrayScaleGrid.getLayoutManager().setGaps(4, 5);
			this.mGrayScaleGrid.setPreferredSize(this.mGroupColumns * this.mPaletteGroupDimentions.x * this.mSwatchSize.x, this.mSwatchSize.y - 3);
			local grouper = 0;
			local groupcontainer = this.GUI.Container(this.GUI.GridLayout(1, this.mPaletteGroupDimentions.x));

			for( local y = 0; y < greys; y++ )
			{
				if (grouper >= this.mPaletteGroupDimentions.x)
				{
					this.mGrayScaleGrid.add(groupcontainer);
					groupcontainer = this.GUI.Container(this.GUI.GridLayout(1, this.mPaletteGroupDimentions.x));
					grouper = 0;
				}

				local swatch = this.GUI.ColorSplotch(this.ColorPalette.grayscale2d[y]);
				swatch.setReleaseMessage("onColorSelect");
				swatch.addActionListener(this);
				groupcontainer.add(swatch);
				grouper++;
			}

			this.mGrayScaleGrid.add(groupcontainer);
			this.add(this.mGrayScaleGrid);
		}

		if (this.mDefault != null)
		{
			this.mDefaultContainer = this.GUI.Container(this.GUI.GridLayout(1, this.mGroupColumns));
			this.mDefaultContainer.getLayoutManager().setGaps(4, 5);
			this.mDefaultContainer.setPreferredSize(this.mGroupColumns * this.mPaletteGroupDimentions.x * this.mSwatchSize.x, this.mSwatchSize.y - 3);
			this.mDefaultContainer.add(this.GUI.Container());
			this.mDefaultContainer.add(this.GUI.Label("Default Color:"));
			local groupcontainer = this.GUI.Container(this.GUI.GridLayout(1, 3));
			groupcontainer.getLayoutManager().setColumns("*", 22, "*");
			local swatch = this.GUI.ColorSplotch(this.mDefault);
			swatch.setReleaseMessage("onColorSelect");
			swatch.addActionListener(this);
			groupcontainer.add(this.GUI.Container());
			groupcontainer.add(swatch);
			groupcontainer.add(this.GUI.Container());
			this.mDefaultContainer.add(groupcontainer);
			this.add(this.mDefaultContainer);
		}
	}

}

