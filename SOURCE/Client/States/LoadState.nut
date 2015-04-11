this.require("States/StateManager");
this.require("GUI/ConfirmationWindow");
this.require("UI/UI");
class this.States.LoadState extends this.State
{
	static mClassName = "LoadState";
	mNextState = null;
	mPackage = null;
	constructor( package, ... )
	{
		this.mPackage = package;

		if (vargc > 0)
		{
			this.mNextState = vargv[0];
		}
	}

	function onEnter()
	{
		::LoadGate.Require(this.mPackage, this);
		::_contentLoader.resetProgress(false);
	}

	function onPackageComplete( name )
	{
		this.log.debug("LoadState Completed: (" + this.mPackage + "): " + name);

		if (name == this.mPackage)
		{
			if (this.mNextState != null)
			{
				this.log.debug("LoadState advancing to new state: " + this.mNextState);
				this.States.set(this.mNextState);
			}
			else
			{
				this.log.debug("LoadState popping because no next state.");
				this.States.pop();
			}
		}
	}

	function onPackageError( name, error )
	{
		this.log.error("Could not load all required media for + " + name + " - " + error);
		this.onPackageComplete(name);
	}

	function onDestroy()
	{
	}

	function onScreenResize( width, height )
	{
	}

	function event_onUpdateLoadScreen( data )
	{
		this._loadScreenManager.setLoadScreenVisible(true);
		this._debug.setText("Loading", "[LoadState: " + this.mPackage + "] Packages still loading:\n" + this._contentLoader.getRequestStatusDebug());
		::_contentLoader.updateLoadScreen();
	}

	function _tostring()
	{
		return "LoadState(" + this.mPackage + ")";
	}

}

