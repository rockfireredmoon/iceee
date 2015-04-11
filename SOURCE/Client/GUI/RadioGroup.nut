this.require("GUI/GUI");
class this.GUI.RadioGroup extends this.MessageBroadcaster
{
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this.mSelected = null;
	}

	function _addButton( button )
	{
		if (button.mRadioGroup == this)
		{
			return;
		}

		button.mRadioGroup = this;

		if (this.mSelected == null)
		{
			this.setSelected(button);
		}
		else
		{
			button.setToggled(false);
		}
	}

	function _removeButton( button )
	{
		if (button.mRadioGroup != this)
		{
			return;
		}

		if (this.mSelected == button)
		{
			this.setSelected(null);
		}

		button.mRadioGroup = null;
	}

	function getSelected()
	{
		return this.mSelected;
	}

	function setSelected( b, ... )
	{
		local forceRetoggle = false;

		if (vargc > 0)
		{
			forceRetoggle = vargv[0];
		}

		local old = this.mSelected;

		if (b == this.mSelected && !forceRetoggle)
		{
			return;
		}

		if (b != null && this != b.mRadioGroup)
		{
			throw this.Exception("Button is not a member of this radio group");
		}

		if (old != null)
		{
			old.setToggled(false);
			this.broadcastMessage("itemDeselected", old);
		}

		this.mSelected = b;

		if (b != null)
		{
			b.setToggled(true);
			this.broadcastMessage("itemSelected", b);
		}
	}

	mSelected = null;
}

