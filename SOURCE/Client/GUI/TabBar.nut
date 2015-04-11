this.require("GUI/Component");
class this.GUI.TabBar extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.mTabNames = [];
		this.mTabs = [];
		this.mAttachPoint = "top";
		this.mIndex = 5;
		this.setSize(25, 25);
		this.setAppearance("TabBar");
	}

	function setAppearance( a )
	{
		this.GUI.Component.setAppearance(a);

		foreach( b in this.mTabs )
		{
			b.setAppearance(this.mAppearance + "/TabButton/" + this.mAttachPoint);
		}
	}

	function addTab( name )
	{
		this.mTabNames.append(name);
		local nt;
		nt = this.GUI.Button(name);
		nt.setSize(200, this.mHeight);
		nt.setInsets(7);
		nt.setPressMessage("onTabPress");
		nt.addActionListener(this);
		nt.setAppearance(this.mAppearance + "/TabButton/Inactive/" + this.mAttachPoint);
		nt.mUseOffsetEffect = false;
		this.mTabs.append(nt);
		this.add(nt);
		this.setLayoutManager(this.GUI.BoxLayout());
		this.getLayoutManager().setExpand(false);
		this.getLayoutManager().setGaps(5);
		this.invalidate();
	}

	function setAttachPoint( p )
	{
		this.mAttachPoint = p;
		this._recalc();
	}

	function selectTab( n )
	{
		foreach( t in this.mTabs )
		{
			if (t.getText() == n)
			{
				this.onTabPress(t);
				break;
			}
		}
	}

	function onTabPress( button )
	{
		if (this.mCurrentTab == button)
		{
			return;
		}

		if (this.mCurrentTab != null)
		{
			this.mCurrentTab.setAppearance(this.mAppearance + "/TabButton/Inactive/" + this.mAttachPoint);
		}

		button.setAppearance(this.mAppearance + "/TabButton/Active/" + this.mAttachPoint);
		this.mCurrentTab = button;

		if (this.mAttachParent && "onTabSwitch" in this.mAttachParent)
		{
			this.mAttachParent.onTabSwitch(button.getText());
		}
	}

	function _recalc()
	{
		if (this.mAttachParent)
		{
			local container = this.mAttachParent.mParentComponent;
			local sz = this.mAttachParent.getSize();
			local pt = this.mAttachParent.getPosition();
			this.setSize(sz.width, this.mHeight);

			if (this.mAttachPoint == "top")
			{
				this.setPosition(pt.x + this.mIndent, pt.y - this.getHeight());
			}
			else if (this.mAttachPoint == "bottom")
			{
				this.setPosition(pt.x + this.mIndent, pt.y + sz.height);
			}

			if (this.mLayoutManager)
			{
				this.mLayoutManager.layoutContainer(this);
			}

			foreach( foo in this.mTabs )
			{
				foo.invalidate();
				foo.validate();
			}
		}
	}

	function attach( pAttachParent )
	{
		this.mAttachParent = pAttachParent;
		local container = this.mAttachParent.mParentComponent;

		if (container)
		{
			container.add(this);
		}
		else
		{
			this.setOverlay(this.mAttachParent.getOverlay());
		}

		this.invalidate();
	}

	function validate()
	{
		this.GUI.Component.validate();
		this._recalc();
	}

	function setGap( pGap )
	{
		this.mGap = pGap;
		this.invalidate();
	}

	function setIndent( pIndent )
	{
		this.mIndent = pIndent;
		this.invalidate();
	}

	mGap = 0;
	mIndent = 5;
	mTabNames = null;
	mTabs = null;
	mCurrentTab = null;
	mAttachParent = null;
	mAttachPoint = null;
	static mClassName = "TabBar";
}

