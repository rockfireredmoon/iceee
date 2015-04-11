this.require("States/StateManager");
this.require("GUI/ConfirmationWindow");
this.require("GUI/GUI");
class this.States.MessageState extends this.State
{
	static mClassName = "MessageState";
	mWindow = null;
	mImage = null;
	mText = null;
	mType = null;
	constructor( text, ... )
	{
		this.mText = text;

		if (vargc > 0)
		{
			this.mType = vargv[0];
		}

		if (vargc > 1)
		{
			this.mImage = vargv[1];
		}
	}

	function onEnter()
	{
		this.mWindow = this.GUI.ConfirmationWindow();

		if (this.mType)
		{
			this.mWindow.setConfirmationType(this.mType);
		}
		else
		{
			this.mWindow.setConfirmationType(this.GUI.ConfirmationWindow.NONE);
		}

		this.mWindow.setText(this.mText);

		if (this.mImage)
		{
			this.mWindow.setImage(this.mImage);
		}

		this.mWindow.addActionListener(this);
	}

	function getHTMLComp()
	{
		return this.mWindow.getHTML();
	}

	function updateText( text )
	{
		this.mText = text;
		this.mWindow.setText(this.mText);
	}

	function onConfirmation( sender, bool )
	{
		this.States.pop(bool);
	}

	function onDestroy()
	{
		this.mWindow.destroy();
	}

	function onScreenResize( width, height )
	{
		this.mWindow.setSize(width, height);
	}

	function event_onUpdateLoadScreen( data )
	{
		::_loadScreenManager.setLoadScreenVisible(false);
	}

}

