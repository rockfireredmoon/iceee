this.require("GUI/FullScreenComponent");
class this.GUI.ButtonFlowLayout extends this.GUI.LayoutManager
{
	constructor( padding )
	{
		this.mPadding = padding;
	}

	function preferredLayoutSize( container )
	{
		return container._addInsets(this.layoutContainer(container, true));
	}

	function layoutContainer( container, ... )
	{
		local sim = vargc > 0 ? vargv[0] : false;
		local h = 0;
		local x = 0;

		foreach( c in container.components )
		{
			local prefsize = c.getPreferredSize();

			if (sim == false)
			{
				c.setSize(prefsize);
				c.setPosition(x, 0);
			}

			x += prefsize.width + this.mPadding;

			if (prefsize.height > h)
			{
				h = prefsize.height;
			}
		}

		if (x > 0)
		{
			x -= this.mPadding;
		}

		return {
			width = x,
			height = h
		};
	}

	mPadding = 0;
}

class this.IGIS.PopUpWindow extends this.GUI.FullScreenComponent
{
	constructor( notification )
	{
		this.mNotification = notification;
		::GUI.FullScreenComponent.constructor(null);
		this.mPanel = this.GUI.Frame(notification.type == this.IGIS.NOTIFY ? "Notification" : "Request");
		this.mPanel.setCloseMessage("close");
		this.mPanel.addActionListener(this);
		this.mPanel.setVisible(true);
		this.mPanel.setSize(250, 175);
		this.add(this.mPanel);
		this.mHTML = this.GUI.HTML("");
		this.mHTML.setInsets(20, 20, 5, 20);
		this.mHTML.setResize(true);
		this.mHTML.getLayoutManager().setAlignment("center");
		this.mHTML.setFont(this.GUI.Font("Maiandra", 24));
		this.mHTML.setText("No text set!");
		this.mPanel.add(this.mHTML, ::GUI.BorderLayout.CENTER);
		local bottomPanel = this.GUI.Panel(this.GUI.ListLayoutManager());
		this.mButtonHolder = this.GUI.Container(this.GUI.BoxLayout());
		this.mButtonHolder.setInsets(10, 0, 15, 0);
		this.mButtonHolder.getLayoutManager().setAlignment(0.5);
		this.mButtonHolder.getLayoutManager().setPackAlignment(0.5);
		bottomPanel.add(this.mButtonHolder);
		local label = ::GUI.Label("Do not pop up automatically");
		this.mCheckBox = ::GUI.CheckBox();
		this.mCheckBox.setChecked(::Pref.get("igis.AutoPopup") == false);

		if (notification.type == this.IGIS.NOTIFY)
		{
			this.mButtonHolder.add(this.mCheckBox);
			this.mButtonHolder.add(label);
			this.mButtonHolder.add(::GUI.Spacer(20, 0));
			this.setOptions([
				"Dismiss"
			]);
		}
		else
		{
			this.mButtonHolder.add(this.mCheckBox);
			this.mButtonHolder.add(label);
			this.mButtonHolder.add(::GUI.Spacer(20, 0));
			this.setOptions(notification.responses);
		}

		this.mPanel.add(this.mButtonHolder, ::GUI.BorderLayout.SOUTH);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setOverlay("GUI/Overlay");
		local pos = {
			x = ::Screen.getWidth() / 2 - this.mPanel.getWidth() / 2,
			y = 128
		};
		pos.x -= 15 * this._igisManager.mPopUps.len();
		pos.y += 15 * this._igisManager.mPopUps.len();
		this.mHTML.setMaximumSize(this.mPanel.getWidth(), null);
		this.mPanel.setPosition(pos);
	}

	function setOptions( options )
	{
		foreach( o in options )
		{
			local bigButton = ::GUI.Button(o);
			bigButton.setFont(this.GUI.Font("MaiandraOutline", 22));
			bigButton.setPressMessage("onOptionPressed");
			bigButton.addActionListener(this);
			bigButton.setFixedSize(100, 26);
			this.mButtonHolder.add(bigButton);
		}

		this.mPanel.setSize(this.mPanel.getWidth() + options.len() * 100, this.mPanel.getHeight());
	}

	function close()
	{
		::Pref.set("igis.AutoPopup", this.mCheckBox.getChecked() == false);
		this._igisManager._removePopUp(this);
		this.destroy();
	}

	function destroy()
	{
		this._igisManager.setPaused(false);
		::GUI.FullScreenComponent.destroy();
	}

	function getNotifications()
	{
		return this.mNotifications;
	}

	function onOptionPressed( button )
	{
		this._fireActionPerformed(button.getText());
		this.close();
	}

	function closeWindow()
	{
		this.setOverlay(null);
		return null;
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function _fireActionPerformed( response )
	{
		this.mMessageBroadcaster.broadcastMessage("onConfirmation", this, response, this.mNotification);
	}

	function setText( pString )
	{
		this.mHTML.setText(pString);
	}

	mPanel = null;
	mHTML = null;
	mButtonHolder = null;
	mConfirmationType = 0;
	mOKButton = null;
	mCancelButton = null;
	mNotification = null;
	mCheckBox = null;
	static mClassName = "ConfirmationWindow";
}

