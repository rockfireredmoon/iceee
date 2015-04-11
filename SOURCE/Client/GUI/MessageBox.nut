this.require("GUI/Panel");
this.require("GUI/Container");
this.require("GUI/BoxLayout");
this.require("GUI/HTML");
class this.GUI.MessageBox extends this.GUI.FullScreenComponent
{
	static mClassName = "MessageBox";
	mActionMessage = "onActionSelected";
	mContentComponent = null;
	mButtons = null;
	mHtml = null;
	constructor( message, alts, ... )
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());

		if (typeof alts != "array")
		{
			throw this.Exception("Alts must be an array of strings");
		}

		if (typeof message == "string")
		{
			this.mHtml = this.GUI.HTML();
			this.mHtml.setMaximumSize(500, null);
			this.mHtml.setResize(true);
			this.mHtml.setText("<font size=\"24\">" + message + "</font>");
			this.mContentComponent = this.GUI.Container(this.GUI.BorderLayout());
			this.mContentComponent.setInsets(5);
			this.mContentComponent.add(this.mHtml);
		}
		else if (message instanceof this.GUI.Component)
		{
			this.mContentComponent = message;
		}
		else
		{
			throw this.Exception("Message must be a string or a GUI.Component instance");
		}

		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mButtons = this.GUI.Container(this.GUI.BoxLayout());

		foreach( alt in alts )
		{
			this.mButtons.add(this._createButton("" + alt));
		}

		this.mButtons.setInsets(4);
		this.add(this.mContentComponent, this.GUI.BorderLayout.CENTER);
		local box = this.GUI.Container(this.GUI.BoxLayoutV());
		box.add(this.mButtons);
		this.add(box, this.GUI.BorderLayout.SOUTH);
		this.setVisible(false);
		this.setSize(this.getPreferredSize());
		this.validate();
		this.centerOnScreen();
		this.setOverlay(this.GUI.CONFIRMATION_OVERLAY);
	}

	function _createButton( text )
	{
		local b = this.GUI.NarrowButton(text);
		b.addActionListener(this);
		return b;
	}

	function setButtonColor( index, color )
	{
		local buttonComponents = this.mButtons.components;

		if (buttonComponents.len() > index)
		{
			switch(color)
			{
			case "Blue":
				buttonComponents[index].setAppearance("BlueNarrowButton");
				buttonComponents[index].setSelection(true);
				break;

			case "Red":
				buttonComponents[index].setAppearance("RedNarrowButton");
				buttonComponents[index].setSelection(true);
				break;

			default:
				break;
			}
		}
	}

	function setButtonSize( index, width, height )
	{
		local buttonComponents = this.mButtons.components;
		local buttonComponents = this.mButtons.components;

		if (buttonComponents.len() > index)
		{
			buttonComponents[index].setFixedSize(width, height);
		}
	}

	function getContentComponent()
	{
		return this.mContentComponent;
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function onActionPerformed( button )
	{
		this.setVisible(false);
		this.mMessageBroadcaster.broadcastMessage(this.mActionMessage, this, button.getText());
		this.setOverlay(null);
	}

	function onOutsideClick( evt )
	{
		this.close();
	}

	function close()
	{
		this.setVisible(false);
		this.setOverlay(null);
	}

	static function showEx( message, alts, callback, ... )
	{
		local mb = this.GUI.MessageBox(message, alts);

		if (callback != null)
		{
			mb.addActionListener(callback);
		}

		if (vargc > 0)
		{
			mb.mActionMessage = vargv[0];
		}

		mb.setVisible(true);
		return mb;
	}

	static function show( message )
	{
		return this.GUI.MessageBox.showEx(message, [
			"Ok"
		], null);
	}

	static function showOkCancel( message, callback, ... )
	{
		local mb = this.GUI.MessageBox.showEx(message, [
			"Ok",
			"Cancel"
		], callback);

		if (vargc > 0)
		{
			mb.mActionMessage = vargv[0];
		}

		return mb;
	}

	static function showYesNo( message, callback, ... )
	{
		local mb = this.GUI.MessageBox.showEx(message, [
			"Yes",
			"No"
		], callback);

		if (vargc > 0)
		{
			mb.mActionMessage = vargv[0];
		}

		return mb;
	}

}

