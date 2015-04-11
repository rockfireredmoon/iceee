this.require("GUI/Container");
class this.IGIS.RTREntry extends this.GUI.Container
{
	constructor( notification, listener )
	{
		::GUI.Container.constructor(null);
		this.mNotification = notification;
		this.mListener = listener;
		this.setInsets(0);
		local label = ::GUI.Label(notification.text);
		label.setAutoFit(true);
		this.add(label, ::GUI.BorderLayout.CENTER);
		local buttonPanel = ::GUI.Container(null);
		buttonPanel.setInsets(0);

		if (notification.responses)
		{
			foreach( r in notification.responses )
			{
				local button = ::GUI.Button(r);
				button.setData({
					id = notification.msgId,
					request = true
				});
				button.setPressMessage("onButtonPressed");
				button.addActionListener(listener);
				button.setInsets(2, 6, 2, 6);
				buttonPanel.add(button);
			}
		}
		else
		{
			local button = ::GUI.Button("Dismiss");
			button.setData({
				id = notification.msgId,
				request = false
			});
			button.setPressMessage("onButtonPressed");
			button.addActionListener(listener);
			button.setInsets(2, 6, 2, 6);
			buttonPanel.add(button);
		}

		buttonPanel.setLayoutManager(::GUI.ButtonFlowLayout(2));
		this.add(buttonPanel, ::GUI.BorderLayout.EAST);
		this.setLayoutManager(::GUI.BorderLayout());
		this.setMaterial("DefaultSkin/Selection");
	}

	function _addNotify()
	{
		::GUI.Container._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		::GUI.Container._removeNotify();
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount == 2)
		{
			this.mListener.onDoubleClicked(this.mNotification);
		}
	}

	mNotification = null;
	mListener = null;
}

class this.IGIS.RTRWindow extends this.GUI.Frame
{
	constructor()
	{
		::GUI.Frame.constructor("Requests & Notifications");
		this.setLayoutManager(::GUI.BorderLayout());
		this.setPosition(5, 55);
		this.setSize(300, 150);
		this.mContentPane = ::GUI.Container(::GUI.ListLayoutManager());
		this.mContentPane.setInsets(5, 5, 5, 5);
		this.add(this.mContentPane, ::GUI.BorderLayout.CENTER);
		this.mScrollBar = ::GUI.ScrollButtons();
		this.mScrollBar.setLayoutExclude(true);
		this.mScrollBar.setInsets(2);
		this.mScrollBar.attach(this.mContentPane);
		this._igisManager.addListener(this);
	}

	function destroy()
	{
		this._igisManager.removeListener(this);
		::GUI.Frame.destroy();
	}

	function buildNotificationList()
	{
		if (this.isVisible() == false)
		{
			return;
		}

		this.mContentPane.setLayoutManager(null);
		this.mContentPane.removeAll();

		foreach( n in this._igisManager.getNotifications() )
		{
			this.mContentPane.add(this.IGIS.RTREntry(n, this));
		}

		this.mContentPane.setLayoutManager(::GUI.ListLayoutManager(2));
	}

	function onButtonPressed( button )
	{
		local data = button.getData();

		if (data.request == true)
		{
			this._igisManager.respond(data.id, button.getText());
		}
		else
		{
			this._igisManager.dismiss(data.id);
		}

		if (this._igisManager.getNotifications().len() == 0)
		{
			this.close();
		}
	}

	function onDoubleClicked( notification )
	{
		this._igisManager.showPopUp(notification);
	}

	function onIGISNotification( notification )
	{
		this.buildNotificationList();
	}

	function onIGISNotificationDismissed( id )
	{
		if (this._igisManager.getNotifications().len() > 0)
		{
			this.buildNotificationList();
		}
		else
		{
			this.close();
		}
	}

	function _visibilityNotify()
	{
		::GUI.Frame._visibilityNotify();
		this.buildNotificationList();
	}

	function _reshapeNotify()
	{
		::GUI.Frame._reshapeNotify();
	}

	mContentPane = null;
	mScrollBar = null;
}

