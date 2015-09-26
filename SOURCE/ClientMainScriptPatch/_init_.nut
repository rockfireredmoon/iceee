this.ClothingIndex <- {};
this.ComponentIndex <- {};
this.CreatureIndex <- {};
this.AttachableIndex <- {};
this.MediaIndex <- {};
this.AbilityIndex <- {};
this.MediaCaseIndex <- {};
this.SwearWordIndex <- {};
this._ArchiveAlias <- {};
this.require("Relay");
this._scene.setShadowTechnique(this.Scene.SHADOWTYPE_NONE);
this._scene.setShadowFarDistance(0.0);
::PageAssets <- {};

if (!("gPositionDebugObjects" in this.getroottable()))
{
	this.gPositionDebugObjects <- false;
}

::TerrainPageDef <- {};
::TerrainEnvDef <- {};
function ReplaceDeps()
{
	foreach( k, v in ::AssetDependencies )
	{
		local changed = false;

		foreach( dep in v )
		{
			local n = 0;

			for( local n = 0; n < v.len(); n++ )
			{
				foreach( a, l in ::_ArchiveAlias )
				{
					foreach( p in l )
					{
						if (v[n] == p)
						{
							this.log.debug("Changing " + p + " to " + a);
							changed = true;
							v[n] = a;
						}
					}
				}
			}
		}

		if (changed)
		{
			local out = "require( \"AssetDependencies\" );\r\n";
			out += "::AssetDependencies[\"" + k + "\"] <- " + this.serialize(v) + ";";
			::System.writeToFile("../../../EE/Client/Earth Eternal/Catalogs/AssetDependencies/" + k + ".deps.nut", out);
		}
	}
}


			
/*
 * Move the extended args from the temporary cookie into the _args table. These
 * are setup in the preloader and used as a way to get command line arguments into
 * the args list on earlier clients (I can't find the 'proper' way to do this).
 */
local tmpCache = MediaCache("tmpcache", "http://localhost");
try {
	foreach(k, v in unserialize(tmpCache.getCookie("xargs"))) {
		::_args[k] <- v;
	}
}
catch(e) {
	print("[WARNING] Could not load extended arguments cache. Web authentication will not be possible.\n");
}

// Clear the temporary cookie, we don't need it anymore
try {
	tmpCache.setCookie("xargs", "");
}
catch(e) {
}

foreach( k, v in ::_args )
{
	this.log.debug("ARGS: " + k + ", " + v);
	print("ICE! ARGS: " + k + ", " + v + "\n");
}

::Screen.setTitle("Earth Eternal - The Anubian War");
this.System.setLoggingLevel(3);
this.MovableObject.setDefaultQueryFlags(this.QueryFlags.ANY | this.QueryFlags.LIGHT_OCCLUDER);
this._scene.setTerrainFlags(this.QueryFlags.ANY | this.QueryFlags.LIGHT_OCCLUDER | this.QueryFlags.FLOOR, this.VisibilityFlags.ANY | this.VisibilityFlags.SCENERY | this.VisibilityFlags.LIGHT_GROUP_0 | this.VisibilityFlags.LIGHT_GROUP_2 | this.VisibilityFlags.LIGHT_GROUP_3);
this._scene.setWaterVisibilityFlagValue(this.VisibilityFlags.WATER);
this._camera.setQueryFlags(this.QueryFlags.CAMERA);
this._camera.setAutoAspectRatio(true);
this.Light.setLightingGroupsMask(this.VisibilityFlags.LIGHT_GROUP_0 | this.VisibilityFlags.LIGHT_GROUP_1 | this.VisibilityFlags.LIGHT_GROUP_2 | this.VisibilityFlags.LIGHT_GROUP_3);
this.InitAttachmentPointSets();
this.InitEmitterSets();
::Screen.setBackgroundColor(this.Color(0.0, 0.0, 0.0));
this._scene.setLinkVisibilityFlags(this.VisibilityFlags.HELPER_GEOMETRY);
::_eventScheduler <- this.EventScheduler();
::_messageTimerManager <- ::MessageTimer.Manager();
::_sceneObjectManager <- this.SceneObjectManager();
::_contentLoader <- this.SuperContentLoader();
::_Connection <- this.Connection();
::_ItemDataManager <- this.ItemDataManager();
::_ItemManager <- this.ItemManager();
::_loadScreenManager <- this.LoadScreenManager();
::_LightingManager <- this.LightingManager();
::_Environment <- this.Environment();
::_igisManager <- this.IGISManager();
::_ChatManager <- this.UI.ChatManager();
::_chatBubbleManager <- this.GUI.ChatBubbleManager();
::_stateManager <- this.StateManager();
::_audioManager <- this.AudioManager();
::_quickBarManager <- this.UI.QuickBarManager();
::_debug <- this.DebugManager();
::_actionManager <- this.ActionManager();
::_AbilityHelper <- this.AbilityHelper();
::_AbilityManager <- this.AbilityManager();
::_questManager <- this.QuestManager();
::_useableCreatureManager <- this.UsableCreatureManager();
::partyManager <- this.PartyManager();
::_gameTime <- this.GameTime();
::_TradeManager <- this.TradeManager();
::_creditShopManager <- this.CreditShopManager();
::_specialOfferManager <- this.SpecialOfferManager();
::_root.addTargetCompositor("Bloom");
::Pref.setDefaults();
::_loadPreferences("Preferences_SystemLocal", ::Pref.SYSTEM_LOCAL);
this.Util.updateFromVideoSettings(this.Pref.get("video.Settings"));
::Screen.setOverlayPassThru("GUI/TooltipOverlay", true);
::Screen.setOverlayPassThru("GUI/EditBorderOverlay", true);
::Screen.setOverlayVisible("GUI/EditBorderOverlay", true);
::_demo <- null;
this._stateManager.setState(this.States.LoadState("Bootstrap", this.States.BootstrapState()));
class this.BugScreenShow 
{
	constructor()
	{
		::_root.addListener(this);
	}

	function _onDebugKeyPressed( evt )
	{
		if (evt.keyCode == this.Key.VK_F11)
		{
			::Screens.show("BugReport");
		}
		else if (evt.keyCode == this.Key.VK_F12)
		{
			local archives = ::_contentLoader.mArchives;
			local current = ::_contentLoader.mCurrentArchive;
			this.log.debug("------------------------------------------");
			this.log.debug("Current archive: " + (current != null ? current.getName() : "<NONE>"));
			this.log.debug("------------------------------------------");
			this.log.debug("------------------------------------------");
			this.log.debug("Loading archives: ");
			this.log.debug("------------------------------------------");

			foreach( m in archives )
			{
				this.log.debug(m.getName());
			}

			this.log.debug("------------------------------------------");
		}
	}

}

this.BugScreenShow();

if (this.gPrepareResources)
{
	this._root.initStandardResidentResources();
}
