this.require("GUI/AnchorPanel");
this.require("GUI/GridLayout");
this.require("GUI/Label");
class this.GUI.ClothingColorRollout extends this.GUI.Component
{
	mCurrentSelections = null;
	mType = null;
	mTypeLabel = null;
	mColorSelectors = null;
	mMessageBroadcaster = null;
	mSlot = null;
	mRollout = null;
	static mClassName = "ClothingColorRollout";
	constructor( type, ... )
	{
		this.GUI.Component.constructor();
		local row = 0;
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mColorSelectors = [];
		this.mCurrentSelections = [];

		if (vargc > 0)
		{
			local selectionArray = vargv[0];
			this.mCurrentSelections = vargv[0];
		}

		this.setLayoutManager(this.GUI.GridLayout(1, row));
		this.setSize(this.getPreferredSize());
		this.mType = type;

		if (vargc > 0)
		{
			this.setType(type, vargv[0]);
		}
		else
		{
			this.setType(type);
		}
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function _createColorSelector( colorArea, palette, colorDefault, selection )
	{
		local sc = this.GUI.ColorPicker("", "", this.ColorRestrictionType.DEVELOPMENT);
		sc.setLayoutManager(this.GUI.GridLayout(1, 1));
		sc.getLayoutManager().setColumns(25);
		sc.getLayoutManager().setRows(18);
		this.mColorSelectors.append(sc);
		this.print("selection: " + selection);
		this.print("colorDefault: " + colorDefault);

		if (selection != null)
		{
			sc.setColor(selection);
		}
		else
		{
			sc.setColor(colorDefault);
		}

		sc.addActionListener(this);
		sc.setChangeMessage("onColorChange");
		this.add(sc);
		this.setLayoutManager(this.GUI.GridLayout(1, this.mColorSelectors.len()));
		this.setSize(this.getPreferredSize());
	}

	function onColorChange( sender )
	{
		this.mCurrentSelections = [];

		foreach( i, x in this.mColorSelectors )
		{
			this.mCurrentSelections.append(x.getCurrent().tolower());
		}

		this._fireActionPerformed("onSelectionChange");
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function removeAllSelectors()
	{
		if (this.mColorSelectors.len() == 0)
		{
			return;
		}

		this.print("mColorSelectors Length: " + this.mColorSelectors.len());

		foreach( i, x in this.mColorSelectors )
		{
			this.print(x);
			x.removeActionListener(this);
			this.remove(x);
		}

		this.mColorSelectors = [];
		this.setLayoutManager(this.GUI.GridLayout(1, 2));
		this.setSize(this.getPreferredSize());
	}

	function setType( type, ... )
	{
		if (type in ::ClothingDef)
		{
			this.removeAllSelectors();
			local cdef = ::ClothingDef[type];

			if ("colors" in cdef)
			{
				local colorDef = cdef.colors;

				foreach( i, color in colorDef )
				{
					local palette = this.ColorPalette.unionPalettes(this.ColorPalette[cdef.palette[i]], color);
					local selections = [];

					if (vargc > 0)
					{
						selections = vargv[0];
					}

					this._createColorSelector(this.ColorSlotNames[i], palette, color, selections.len() > i ? selections[i] : null);
				}
			}
		}
	}

	function getCurrent()
	{
		return this.mCurrentSelections;
	}

	function setCurrent( current )
	{
		this.mCurrentSelections = current;
		this.setType(this.mType, this.mCurrentSelections);
	}

	function destroy()
	{
		this.removeAllSelectors();
		this.setOverlay(null);
		return null;
	}

}

