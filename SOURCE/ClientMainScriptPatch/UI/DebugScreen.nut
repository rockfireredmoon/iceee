this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.DebugScreen extends this.GUI.Container
{
	static mClassName = "DebugScreen";
	mZoneDef = null;
	mWorldPos = null;
	mUpdateTimer = null;
	mTerrainPage = null;
	mGodModeStatus = null;
	mVersion = null;
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.BorderLayout());
		this.setAppearance("ChatWindow");
		this.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.25));
		this.add(this._createInfoPage());

		if (this.Util.isDevMode())
		{
			this.setSize(300, 15);
		}
		else
		{
			this.setSize(300, 30);
		}

		this.setSticky("left", "top");
		this.setPosition(90, 0);
		this.mUpdateTimer = this.GUI.Timer("update", 500, 500);
		this.mUpdateTimer.addListener(this);
		this.setPassThru(true);
		this.setCached(false);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mUpdateTimer.setDelay(10);
		this.setOverlay("GUI/DebugScreen");
		this.Screen.setOverlayVisible("GUI/DebugScreen", true);
	}

	function _removeNotify()
	{
		this.GUI.Component._removeNotify();
		this.mUpdateTimer.setDelay(null);
	}

	function _createInfoPage()
	{
		local layout = this.GUI.Container(this.GUI.BorderLayout());
		local p = this.GUI.Container(this.GUI.BoxLayout());
		p.setInsets(1);
		this.mWorldPos = this.GUI.Label("XXXXXX XXXXXX XXX XXXXXXXX");
		this.mGodModeStatus = this.GUI.Label("Gaia Mode: OFF");
		this.mVersion = this.GUI.Label();
		this.mVersion.setText("Version: " + this.gVersion);
		p.add(this.mGodModeStatus);
		p.add(this.GUI.Spacer());
		p.add(this.mWorldPos);
		layout.add(p, this.GUI.BorderLayout.NORTH);

		if (!this.Util.isDevMode())
		{
			layout.add(this.mVersion, this.GUI.BorderLayout.SOUTH);
		}

		return layout;
	}

	function update()
	{
		local str = "";

		if (this._sceneObjectManager)
		{
			local worldInstanceId = this._sceneObjectManager.getCurrentZoneID();
			str += "Z:" + this._sceneObjectManager.getCurrentZoneDefID() + "(" + worldInstanceId + ") ";
			this.mWorldPos.setText(str);
		}

		if (::gToolMode == "build" && ::_loadNode != null)
		{
			local tpos = this.Util.getTerrainPageIndex(::_loadNode.getPosition());

			if (tpos)
			{
				str += "(T: " + tpos.x + ", " + tpos.z + ") ";
			}

			str += "Pos: " + ::_buildTool.getBuildNodePosition().x.tointeger() + " " + ::_buildTool.getBuildNodePosition().y.tointeger() + " " + ::_buildTool.getBuildNodePosition().z.tointeger();
			this.mWorldPos.setText(str);
		}
		else if (::_avatar)
		{
			local tpos = ::_avatar.getTerrainPageCoords();

			if (tpos)
			{
				str += "(T: " + tpos.x + ", " + tpos.z + ") ";
			}

			str += "Pos: " + ::_avatar.getPosition().x.tointeger() + " " + ::_avatar.getPosition().y.tointeger() + " " + ::_avatar.getPosition().z.tointeger();

			if (::_avatar.hasStatusEffect(this.StatusEffects.INVINCIBLE))
			{
				this.mGodModeStatus.setText("Gaia mode : ON");
				this.mGodModeStatus.setFontColor(this.Color(this.Colors.red));
			}
			else if (::_avatar.hasStatusEffect(this.StatusEffects.UNKILLABLE))
			{
				this.mGodModeStatus.setText("Gaia mode : UNKILLABLE");
				this.mGodModeStatus.setFontColor(this.Color(this.Colors.yellow));
			}
			else
			{
				this.mGodModeStatus.setText("Gaia mode : OFF");
				this.mGodModeStatus.setFontColor(this.Color(this.Colors.green));
			}

			this.mWorldPos.setText(str);
		}
		else
		{
			this.mWorldPos.setText("");
		}
	}

}

