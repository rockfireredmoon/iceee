this.require("GUI/AnchorPanel");
this.require("GUI/Component");
class this.GUI.IconScrollSelect extends this.GUI.Component
{
	constructor()
	{
		this.GUI.Component.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setAppearance("Container");
		this.mLeftButton = this.GUI.SmallButton("LineUp");
		this.mLeftButton.setLayoutExclude(true);
		this.mLeftButton.setPosition(0, 20);
		this.mLeftButton.setPressMessage("onLeftButton");
		this.mLeftButton.addActionListener(this);
		this.add(this.mLeftButton);
		this.mRightButton = this.GUI.SmallButton("LineDown");
		this.mRightButton.setLayoutExclude(true);
		this.mRightButton.setPosition(100, 20);
		this.mRightButton.setPressMessage("onRightButton");
		this.mRightButton.addActionListener(this);
		this.add(this.mRightButton);
		this.mHolder = this.GUI.IconHolder();
		this.mHolder.setLayoutExclude(true);
		this.add(this.mHolder);
		this.mHolder.addActionListener(this);
		this.mIcons = [];
		this.mIconNames = [];
		this._recalc();
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function setChangeMessage( pString )
	{
		this.mChangeMessage = pString;
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function onHolderClick( h )
	{
		if (this.mRollout)
		{
			return;
		}
		else
		{
			this._openRollout();
		}
	}

	function _openRollout()
	{
		this.mRollout = this.GUI.IconScrollSelectRollout(this);
		local i;

		for( i = 0; i < this.mIcons.len(); i++ )
		{
			this.mRollout.addIcon(this.mIconNames[i], this.mIcons[i]);
		}
	}

	function onRolloutSelect( name )
	{
		local i;

		if (this.mRollout == null)
		{
			throw "IconScrollSelect got message from a phantom rollout!";
			return;
		}

		for( i = 0; i < this.mIcons.len(); i++ )
		{
			if (name == this.mIconNames[i])
			{
				this._selectIndex(i);
			}
		}

		this._closeRollout();
	}

	function _closeRollout()
	{
		this.mRollout.setOverlay(null);
		this.mRollout = null;
	}

	function onLeftButton( b )
	{
		if (this.mIcons.len() == 0)
		{
			return;
		}
		else if (this.mIndex <= 0)
		{
			this._selectIndex(this.mIcons.len() - 1);
		}
		else
		{
			this._selectIndex(this.mIndex - 1);
		}
	}

	function onRightButton( b )
	{
		if (this.mIcons.len() == 0)
		{
			return;
		}
		else if (this.mIndex >= this.mIcons.len() - 1)
		{
			this._selectIndex(0);
		}
		else
		{
			this._selectIndex(this.mIndex + 1);
		}
	}

	function setMargin( m )
	{
		this.mMargin = m;
	}

	function setHolderSize( ... )
	{
		if (vargc == 1)
		{
			this.mHolder.setSize(vargv[0]);
			this._recalc();
			return;
		}

		if (vargc == 2)
		{
			this.mHolder.setSize(vargv[0], vargv[1]);
			this._recalc();
			return;
		}
	}

	function _recalc()
	{
		this.mLeftButton.setPosition(this.mMargin, (this.getHeight() - this.mLeftButton.getHeight()) / 2);
		this.mHolder.setPosition((this.getWidth() - this.mHolder.getWidth()) / 2, (this.getHeight() - this.mHolder.getHeight()) / 2);
		this.mRightButton.setPosition(this.getWidth() - this.mMargin - this.mRightButton.getWidth(), (this.getHeight() - this.mRightButton.getHeight()) / 2);
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();
		this._recalc();
	}

	function addIcon( name, icon )
	{
		local istr;

		if (typeof icon == "instance")
		{
			istr = icon.getMaterial();
		}
		else
		{
			istr = icon;
		}

		this.mIconNames.append(name);
		this.mIcons.append(istr);
	}

	function _selectIndex( index )
	{
		this.mIndex = index;
		this.mHolder.setIcon(this.mIcons[this.mIndex]);

		if (this.mChangeMessage)
		{
			this._fireActionPerformed(this.mChangeMessage);
		}
	}

	function getCurrent()
	{
		if (this.mIndex < 0)
		{
			return null;
		}
		else
		{
			return this.mIconNames[this.mIndex];
		}
	}

	mMargin = 3;
	mLeftButton = null;
	mRightButton = null;
	mHolder = null;
	mRollout = null;
	mIndex = -1;
	mIcons = null;
	mIconNames = null;
	mChangeMessage = null;
}

class this.GUI.IconScrollSelectRollout extends this.GUI.AnchorPanel
{
	constructor( par )
	{
		this.mIcons = [];
		this.mIconNames = [];
		this.mGridPane = this.GUI.Component();
		this.mGridPane.setLayoutExclude(true);
		this.mGridPane.setAppearance("Container");
		this.GUI.AnchorPanel.constructor(par);
		this.add(this.mGridPane);
		this.setGridSlots(1, 1);
		this.setAlignment({
			horz = "center",
			vert = "bottom"
		});
		this.mParent = par;
	}

	function setIconSize( w, h )
	{
		this.mIconWidth = w;
		this.mIconHeight = h;
	}

	function setColumnCount( c )
	{
		this.mRowCount = 0;
		this.mColumnCount = this.Math.min(1, c);
	}

	function setRowCount( c )
	{
		this.mColumnCount = 0;
		this.mRowCount = this.Math.min(1, c);
	}

	function setGridSlots( iconswide, iconshigh )
	{
		local x = this.Math.max(1, iconswide);
		local y = this.Math.max(1, iconshigh);
		this.setSize(this.mHGap + (this.mHGap + this.mIconWidth) * x, this.mVGap + (this.mVGap + this.mIconHeight) * y);
	}

	function onIconSelect( b )
	{
		local i;

		for( i = 0; i < this.mIcons.len(); i++ )
		{
			if (this.mIcons[i] == b)
			{
				if (this.mParent && "onRolloutSelect" in this.mParent)
				{
					this.mParent.onRolloutSelect(this.mIconNames[i]);
				}

				return;
			}
		}
	}

	function setHolderSize( w, h )
	{
		this.mHolderWidth = w;
		this.mHolderHeight = h;
	}

	function addIcon( name, icon )
	{
		local ibut;
		ibut = this.GUI.Icon();
		ibut.setMaterial(icon);
		ibut.setPressMessage("onIconSelect");
		ibut.addActionListener(this);
		this.mIconNames.append(name);
		this.mIcons.append(ibut);
		this.mGridPane.add(ibut);

		if (this.mColumnCount > 0)
		{
			if (this.mIcons.len() <= this.mColumnCount)
			{
				this.setGridSlots(this.mIcons.len(), 1);
			}
			else
			{
				this.setGridSlots(this.mColumnCount, (this.mIcons.len() + this.mColumnCount - 1) / this.mColumnCount);
			}
		}
		else if (this.mRowCount > 0)
		{
			if (this.mIcons.len() <= this.mRowCount)
			{
				this.setGridSlots(1, this.mIcons.len());
			}
			else
			{
				local wid = (this.mIcons.len() + this.mRowCount - 1) / this.mRowCount;
				this.print("two choices " + this.mRowCount + ", " + (this.mIcons.len() + wid - 1) / wid);
				this.setGridSlots(wid, this.Math.min(this.mRowCount, (this.mIcons.len() + wid - 1) / wid));
			}
		}
	}

	function _reshapeNotify()
	{
		this.GUI.AnchorPanel._reshapeNotify();
		this._recalc();
	}

	function _recalc()
	{
		local xicons = this.getWidth();
		xicons -= this.mHGap;
		xicons /= this.mIconWidth + this.mHGap;

		if (xicons < 1)
		{
			xicons = 1;
		}

		local yicons = this.getHeight();
		yicons -= this.mVGap;
		yicons /= this.mIconHeight + this.mVGap;

		if (yicons < 1)
		{
			yicons = 1;
		}

		local newsize = {
			width = this.mHGap + (this.mHGap + this.mIconWidth) * xicons,
			height = this.mVGap + (this.mVGap + this.mIconHeight) * yicons
		};

		if (newsize.width != this.getWidth() || newsize.height != this.getHeight())
		{
			this.setSize(newsize);
		}

		this.mGridPane.setSize(newsize);
		this.mGridPane.setLayoutManager(this.GUI.GridLayout(yicons, xicons));
		this.mGridPane.getLayoutManager().setGaps(this.mHGap, this.mVGap);
	}

	mHGap = 3;
	mVGap = 3;
	mIconWidth = 42;
	mIconHeight = 42;
	mColumnCount = 3;
	mRowCount = 0;
	mGridPane = null;
	mIcons = null;
	mIconNames = null;
	mParent = null;
}

