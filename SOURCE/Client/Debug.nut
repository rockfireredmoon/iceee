this.require("EventScheduler");
class this.DebugManager 
{
	mUpdater = null;
	mText = {};
	mShowAssembly = true;
	mShowFPS = 0;
	mShowProfileText = true;
	mShowGameState = true;
	constructor()
	{
		this._eventScheduler.repeatIn(1.0, 1.0, this, "_calc");
	}

	function _calc()
	{
		if (!this.isShowingFPS())
		{
			return;
		}

		local str;

		if (this.mShowAssembly)
		{
			str = this._sceneObjectManager.getDebugStatusText("brief");
		}
		else
		{
			str = null;
		}

		this.setText("Assembly", str, false);

		if (this.mShowProfileText)
		{
			str = "( Camera.Far=" + this.gCamera.farClippingDistance + ", Shadows=" + this.gShadows + " )\n";
			local prof = this.Util.getProfileSnapshotByTime();

			foreach( pair in prof )
			{
				str += pair[0] + ": " + this.format("%.1f", pair[1] * 100.0) + "%\n";
			}
		}
		else
		{
			str = null;
		}

		this.setText("Profile", str, false);

		if (this.mShowGameState)
		{
			str = "[" + this._stateManager.mStack.len() + "]: " + this.Util.join(this._stateManager.mStack, " > ") + "\n";
		}
		else
		{
			str = null;
		}

		this.setText("GameState", str, false);
		this._update();
	}

	function _update()
	{
		if (this.mText.len() == 0 || !this.mShowFPS)
		{
			this.Screen.setOverlayVisible("EE/Debug", false);
			this.Screen.setOverlayVisible("EE/Debug/Shadow", false);
			return;
		}

		local out = "";

		foreach( section, text in this.mText )
		{
			out += "---[ " + section + " ]---\n";
			out += text;
		}

		this.Screen.setOverlayVisible("EE/Debug", true);
		this.Screen.setOverlayVisible("EE/Debug/Shadow", true);
		this.Widget("EE/Debug/Text").setText(out);
		this.Widget("EE/Debug/Text/Shadow").setText(out);
	}

	function setText( section, text, ... )
	{
		local updateNow = vargc > 0 ? vargv[0] : true;

		if (text == null)
		{
			if (section in this.mText)
			{
				delete this.mText[section];
			}
		}
		else
		{
			this.mText[section] <- text;
		}

		if (updateNow)
		{
			this._update();
		}
	}

	function isShowingFPS()
	{
		return this.getShowingFPS() != 0;
	}

	function getShowingFPS()
	{
		return this.mShowFPS;
	}

	function toggleShowingFPS()
	{
		this.setShowingFPS((this.getShowingFPS() + 1) % 3);
	}

	function setShowingFPS( value )
	{
		this.mShowFPS = value;

		if (this.mShowFPS == 1)
		{
			this.Screen.setOverlayVisible("Core/Debug", true);
			this.Screen.setOverlayVisible("Core/DebugMemory", false);
			this._calc();
		}
		else if (this.mShowFPS == 2)
		{
			this.Screen.setOverlayVisible("Core/Debug", false);
			this.Screen.setOverlayVisible("Core/DebugMemory", true);
			this._calc();
		}
		else
		{
			this.Screen.setOverlayVisible("Core/Debug", false);
			this.Screen.setOverlayVisible("Core/DebugMemory", false);
			this._update();
		}
	}

}

