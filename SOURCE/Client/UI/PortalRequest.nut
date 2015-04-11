class this.Screens.PortalRequest 
{
	mPopupBox = null;
	constructor()
	{
	}

	function showPortalRequest( message )
	{
		if (this.mPopupBox != null)
		{
			this.mPopupBox.destroy();
			this.mPopupBox = null;
		}

		local callback = {
			function onActionSelected( mb, alt )
			{
				if (alt == "Ok")
				{
					::_Connection.sendQuery("portal.acceptRequest", this);
					return;
				}
			}

		};
		this.mPopupBox = this.GUI.MessageBox.showOkCancel(message, callback);
	}

	function setVisible( value )
	{
		if (!value)
		{
			if (this.mPopupBox)
			{
				this.mPopupBox.destroy();
				this.mPopupBox = null;
			}
		}
	}

}

