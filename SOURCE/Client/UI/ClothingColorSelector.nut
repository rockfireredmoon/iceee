this.require("GUI/Button");
class this.GUI.ClothingColorSelector extends this.GUI.Button
{
	constructor( name, slot, type, ... )
	{
		if (name == null || typeof name != "string")
		{
			throw "GUI.ClothingColorSelector was passed a invalid name value during construction";
		}

		if (type == null || typeof type != "string")
		{
			throw "GUI.ClothingColorSelector was passed a invalid type value during construction";
		}

		this.mCurrentSelections = [];

		if (vargc > 0 && typeof vargv[0] == "array")
		{
			this.mCurrentSelections = vargv[0];
		}

		this.GUI.Button.constructor(name);
		this.mSlot = slot;
		this.setType(type);
		this.addActionListener(this);
		this.setReleaseMessage("onRollout");
	}

	function _openRollout()
	{
		if (this.mRollout)
		{
			return;
		}

		this.mRollout = this.GUI.ClothingColorRollout(this.mType, this.mCurrentSelections);
		this.GUI._Manager.addTransientToplevel(this.mRollout);
	}

	function _closeRollout()
	{
		if (!this.mRollout)
		{
			return;
		}

		this.mRollout = this.mRollout.destroy();
	}

	function _removeNotify()
	{
		this._closeRollout();
		this.GUI.Button._removeNotify();
	}

	function onColorChange( sender )
	{
		if (this.mRollout)
		{
			this.mCurrentSelections = [];

			foreach( i, x in this.mRollout.mColorSelectors )
			{
				this.mCurrentSelections.append(x.getCurrent().tolower());
			}

			this._fireActionPerformed("onSelectionChange");
		}
	}

	function onRollout( sender )
	{
		if (this.mRollout)
		{
			this._closeRollout();
		}
		else if (!this.mRollout)
		{
			this._openRollout();
		}
	}

	function getCurrent()
	{
		return this.mCurrentSelections;
	}

	function setCurrent( current )
	{
		this.mCurrentSelections = current;

		if (this.mRollout)
		{
			this.mRollout.setType(this.mType, this.mCurrentSelections);
		}
	}

	function setType( type )
	{
		if (type in this.ClothingDef)
		{
			local oldSelections = this.mCurrentSelections;
			this.mCurrentSelections = [];
			this.mType = type;
			local cdef = this.ClothingDef[type];

			if ("colors" in cdef)
			{
				local c = cdef.colors;

				foreach( i, x in c )
				{
					if ((i in oldSelections) && oldSelections[i] != null)
					{
						this.mCurrentSelections.append(oldSelections[i]);
					}
					else
					{
						this.mCurrentSelections.append(x);
					}

					this.print("selection set: " + i + ", " + x[0]);
				}
			}

			if (this.mRollout)
			{
				this.mRollout.setType(type, this.mCurrentSelections);
			}
		}
	}

	mSlot = null;
	mType = null;
	mCurrentSelections = null;
	mRollout = null;
	static mClassName = "ClothingColorSelector";
}

