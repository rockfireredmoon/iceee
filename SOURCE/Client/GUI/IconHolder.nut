this.require("GUI/Component");
this.require("GUI/GridLayout");
class this.GUI.IconHolder extends this.GUI.Component
{
	constructor()
	{
		this.GUI.Component.constructor();
		this.setSize(46, 46);
		this.setLayoutManager(this.GUI.GridLayout(1, 1));
		this.setInsets(2);
		this.setAppearance("IconHolder");
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
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

	function getPreferredSize()
	{
		return this.getSize();
	}

	function setIcon( icon )
	{
		if (icon)
		{
			this.mIcon = icon;
			this.add(icon);
			icon.setPosition(0, 0);
		}

		this._fireActionPerformed("onIconAdded");
	}

	function getIcon()
	{
		return this.mIcon;
	}

	mIcon = null;
}

