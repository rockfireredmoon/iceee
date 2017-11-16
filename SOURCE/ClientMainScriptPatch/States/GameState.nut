this.require("States/StateManager");
this.require("UI/UI");
this.require("Tools/PlayTool");
class this.AccountPermissionGroupChecker extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, rows )
	{
		::_accountPermissionGroup = "admin";
	}

	function onQueryError( qa, error )
	{
		::_accountPermissionGroup = "player";
	}

}

class this.States.GameState extends this.State
{
	static mClassName = "GameState";
	static MAX_FAKE_DIFF = 20.0;
	static MAX_GLITCH_CATCHER = 5000;
	static MAX_JUMP = 20;
	static END_CUSHION = 97;
	mRandom = this.Random();
	mPersonaIndex = 0;
	mPersona = null;
	mManualDisconnect = false;
	mInitialized = true;
	mLoading = false;
	mPendingRequiredAssemblyCount = null;
	mRequiredMediaCount = null;
	mRequiredPages = null;
	mCurrentPercentage = 0;
	mRealPercentage = 0;
	mGlitchCatcher = 0;
	mBasicSetupTask = null;
	mReadyToSetupQuickbars = false;
	mQuickbarsSetup = false;
	mRequestedPreferences = false;
	mSetupDefaultAbilities = false;
	mResourcesToPrepare = null;
	mResourcePreparationTask = null;
	constructor( personaIndex, persona )
	{
		this.mPersonaIndex = personaIndex;
		this.mPersona = persona;
		::_Connection.addListener(this);
		::_quickBarManager.addListener(this);
		this.mResourcesToPrepare = [
			"Biped-Male.mesh",
			"Biped-Male_mesh.skeleton",
			"Biped-Male2.mesh",
			"Biped-Male2_mesh.skeleton",
			"Biped-Female.mesh",
			"Biped-Female_mesh.skeleton",
			"Manipulator-ClickBox.png",
			"Manipulator-ClickBox.mesh",
			"Particles/Burst2",
			"Sound-Ambient-Cloudyday2.ogg",
			"Env-WhiteClouds"
		];
	}

	function onPackageComplete( pkg )
	{
		if (::States.top() != this)
		{
			return;
		}

		::KeyBindingDef.Load();
		::_Environment.setOverride(null);
		::_Environment.setAutoUpdate(true);
		::_chatBubbleManager <- this.GUI.ChatBubbleManager();
		this.Screens.show("ChatWindow");
		this.Screens.show("SelfTargetWindow");
		this.Screens.show("MainScreen");
		::_quickBarManager.initialize();
		::_quickBarManager.show();
		::sizeMult <- 1;
		::_scene.setVisibilityMask(this.VisibilityFlags.DEFAULT);
		::gToolMode <- "play";
		::_playTool <- this.PlayTool();
		::_buildTool <- this.BuildTool();
		::_groundTargetTool <- this.GroundTargetTool();
		::Screens.show("MiniMapScreen");
		this.Screens.show("PlayerBuffDebuff");
		::Screens.show("QuestTracker");
		::_tutorialManager <- this.TutorialManager();
		this.Screen.setOverlayVisible("GUI/TutorialOverlay", true);
		::IGIS.show();
		this.mInitialized = true;
		::_Connection.sendSelectPersona(this.mPersonaIndex);
		::Pref.setCharacter(this.mPersonaIndex);
	}

	function onProtocolChanged( newProto )
	{
		if (newProto != "Play")
		{
			throw this.Exception("Unexpected protocol switch!");
		}

		::_AbilityManager.handleUpdatingAbilities(::AbilityIndex, false);
		::_AbilityManager.getAbilityCooldowns();
		::_ChatManager.loadSwearWordList(::SwearWordIndex);
		::LoadScreen.setInGame(true);
		local socialWindow = ::Screens.get("SocialWindow", true);

		if (("gGodmodeAfterConnect" in this.getroottable()) && this.gGodmodeAfterConnect)
		{
			::EvalCommand("/godmode");
		}

		::_Connection.sendQuery("admin.check", this.AccountPermissionGroupChecker());
	}

	function onEnter()
	{
		this.mInitialized = false;
		this.LoadGate.Require(this.GateTrigger.GateTypes.PLAY_STAGE_1_DEP, this);
		this.mBasicSetupTask = ::_eventScheduler.fireIn(1.0, this, "_basicSetup");
		this.mResourcePreparationTask = ::_eventScheduler.repeatIn(0.1, 60.0, this, "_reprepareResources");
	}

	function onDestroy()
	{
		::_Connection.removeListener(this);
		::LoadScreen.setInGame(false);
		::Screen.resetForceInvisible();
		::_sceneObjectManager.reset();
		::_playTool <- null;
		::_buildTool <- null;

		if (::_groundTargetTool)
		{
			::_groundTargetTool.destroy();
			::_groundTargetTool <- null;
		}

		if (this.mBasicSetupTask)
		{
			::_eventScheduler.cancel(this.mBasicSetupTask);
			this.mBasicSetupTask = null;
		}

		if (this.mResourcePreparationTask)
		{
			::_eventScheduler.cancel(this.mResourcePreparationTask);
			this.mResourcePreparationTask = null;
		}

		::_tools.reset();
		::gToolMode <- null;
		::_quickBarManager.shutdown();
		::_quickBarManager.hide();
		this.Screens.close("ChatWindow");
		::_ChatManager.reset();
		this.Screen.setOverlayVisible("GUI/TutorialOverlay", false);
		::Screens.close("Queue");

		if (::_tutorialManager)
		{
			::_tutorialManager.onRemoveAndSaveTutorial();
		}

		::Screens.close("MiniMapScreen");
		::partyManager.clear();
		::Screens.clear();
		::_Environment.setAutoUpdate(false);
		::_exitGameStateRelay.gameExited();
		::_avatar <- null;
		this.mInitialized = false;
	}

	function _reprepareResources()
	{
		if (!this.gPrepareResources)
		{
			return;
		}

		this._root.prepareResources(this.mResourcesToPrepare, "CommonResources");
	}

	function _basicSetup()
	{
		if (::_avatar && ::_avatar.getID())
		{
			if (!this.mRequestedPreferences)
			{
				::Pref.download(this.Pref.CHARACTER);
				this.mRequestedPreferences = true;
			}

			if (this.mReadyToSetupQuickbars && !this.mQuickbarsSetup)
			{
				local quickbar0 = ::_quickBarManager.getQuickBar(0);

				if (quickbar0)
				{
					this.setupQuickbarDefaults();
					this.mQuickbarsSetup = true;
				}
			}
		}

		if (this.mQuickbarsSetup && this.mRequestedPreferences)
		{
		}
		else
		{
			this.mBasicSetupTask = ::_eventScheduler.fireIn(1.0, this, "_basicSetup");
		}
	}

	function _continueWithDisconnect()
	{
		::gQuickLogin <- false;
		this.States.set(this.States.LoginState());

		if (this.mManualDisconnect == false)
		{
			this.States.push(this.States.MessageState("The server has disconnected.", this.GUI.ConfirmationWindow.OK));
		}
	}

	function onActionSelected( mb, alt )
	{
		if (alt == "Yes")
		{
			::_buildTool.getPaintTool().saveTerrain();
		}
		else
		{
		}

		this._continueWithDisconnect();
	}

	function event_Disconnect( manual )
	{
		this.mManualDisconnect = manual;

		if ("dev" in ::_args || Util.hasTerrainPermission())
		{
			if (::_buildTool && ::_buildTool.getPaintTool().hasUnsavedTerrainData())
			{
				this.GUI.MessageBox.showYesNo("Save terrain changes? Select Yes to save changes and disconnect, No to cancel disconnect.", this);
			}
			else
			{
				this._continueWithDisconnect();
			}
		}
		else
		{
			this._continueWithDisconnect();
		}
	}

	function _fakeProgress()
	{
		local d = (this.mCurrentPercentage - this.mRealPercentage).tofloat();

		if (d >= this.MAX_FAKE_DIFF)
		{
			return this.mCurrentPercentage;
		}

		local prob = 1.0;

		if (d > 0)
		{
			prob = d / (d * d);
		}

		prob /= 6.0;
		local r = this.mRandom.nextFloat();

		if (prob > r)
		{
			return this.mCurrentPercentage + 1;
		}

		return this.mCurrentPercentage;
	}

	function onQuickbarUnserialized( quickbarManager, quickbar )
	{
		local quickbar0 = ::_quickBarManager.getQuickBar(0);

		if (quickbar0 == quickbar)
		{
			this.mReadyToSetupQuickbars = true;
		}
	}

	function onAvatarChanged( oldAvatar, avatar )
	{
		::Screens.get("Equipment", true);
	}

	function setupQuickbarDefaults()
	{
		local quickbar = ::_quickBarManager.getQuickBar(0);
		local actionContainer = quickbar.getActionContainer();
		local abilityAction = this.MeleeAbility("Icon-Question_Mark.png");
		local actionButton = actionContainer.addAction(abilityAction, false, 0);
		actionContainer.setSlotDraggableReplaceable(0, false);
		abilityAction = this.RangedAbility("Icon-Question_Mark.png");
		actionButton = actionContainer.addAction(abilityAction, true, 1);
		actionContainer.setSlotDraggableReplaceable(1, false);
		local eq = ::Screens.get("Equipment", false);

		if (eq)
		{
			eq._updateQuickbarWithEquipment();
		}
	}

	function event_queueChanged( data )
	{
		if (data[0] > 0)
		{
			local queue = ::Screens.show("Queue");
			queue.setQueuePosition(data[0], data[1]);
		}
		else
		{
			::Screens.close("Queue");
		}
	}

	function event_onUpdateLoadScreen( data )
	{
		local showScreen = false;
		local debug = true;
		local reasons = "";

		if (::_loadNode == null)
		{
			if (::_avatar == null)
			{
				reasons += "  !!! Avatar not set !!!\n";
				showScreen = true;
			}
		}

		local pendingRequiredAssemblyCount = this._sceneObjectManager.getPendingRequiredAssemblyCount();

		if (pendingRequiredAssemblyCount == null || pendingRequiredAssemblyCount > 0)
		{
			reasons += "  !!! Nearby scenery pages are not ready (" + pendingRequiredAssemblyCount + ")!!!\n";
			showScreen = true;
		}

		local count = this._contentLoader.getUnloadedRequestCount(true);
		local requiredMediaCount = count;

		if (count > 0)
		{
			reasons += "  !!! Required media not loaded !!!\n";
			showScreen = true;
		}

		if (showScreen && !this.mLoading)
		{
			this.mPendingRequiredAssemblyCount = pendingRequiredAssemblyCount;
			this.mRequiredMediaCount = requiredMediaCount;
			this.mRequiredPages = null;
			this.mCurrentPercentage = 0;
			this.mRealPercentage = 0;
			this.mLoading = true;
		}

		local pendingTerrain = this._scene.getUnloadedTileCount();

		if (debug && showScreen)
		{
			reasons += "  --- Load Queue Stats ---\n";
			reasons += "    Required Terrain Tiles: " + this._scene.getUnloadedTileCount() + "\n";
			local count = 0;
			local requiredmedia = "";

			foreach( r in this._contentLoader.mRequests )
			{
				if (!r.isLoaded() && r.mPriority >= ::ContentLoader.PRIORITY_REQUIRED)
				{
					if (count < 5)
					{
						requiredmedia += "    " + r + " : " + r.mState + "\n";
					}
					else if (count == 6)
					{
						requiredmedia += "    ...\n";
					}

					count++;
				}
			}

			reasons += "    Required Package Requests: " + count + "\n";
			local normalmedia = "";
			count = 0;

			foreach( r in this._contentLoader.mRequests )
			{
				if (!r.isLoaded() && r.mPriority >= ::ContentLoader.PRIORITY_NORMAL && r.mPriority < ::ContentLoader.PRIORITY_REQUIRED)
				{
					if (count < 5)
					{
						normalmedia += "    " + r + " : " + r.mState + "\n";
					}
					else if (count == 6)
					{
						normalmedia += "    ...\n";
					}

					count++;
				}
			}

			reasons += "    Normal Package Requests: " + count + "\n";
			local lowmedia = "";
			count = 0;

			foreach( r in this._contentLoader.mRequests )
			{
				if (!r.isLoaded() && r.mPriority >= ::ContentLoader.PRIORITY_LOW && r.mPriority < ::ContentLoader.PRIORITY_NORMAL)
				{
					if (count < 5)
					{
						lowmedia += "    " + r + " : " + r.mState + "\n";
					}
					else if (count == 6)
					{
						lowmedia += "    ...\n";
					}

					count++;
				}
			}

			reasons += "    Low Package Requests: " + count + "\n";
			local fetchmedia = "";
			count = 0;

			foreach( r in this._contentLoader.mRequests )
			{
				if (!r.isLoaded() && r.mPriority < ::ContentLoader.PRIORITY_LOW && r.mState != "FETCHEDONLY")
				{
					if (count < 5)
					{
						fetchmedia += "    " + r + " : " + r.mState + "\n";
					}
					else if (count == 6)
					{
						fetchmedia += "    ...\n";
					}

					count++;
				}
			}

			reasons += "    Fetch-Only Package Requests: " + count + "\n";
			count = 0;

			foreach( r in this._contentLoader.mRequests )
			{
				if (r.mState == "FETCHEDONLY")
				{
					count++;
				}
			}

			reasons += "    Pre-fetched Package Requests: " + count + "\n";
			local errormedia = "";
			count = 0;

			foreach( r in this._contentLoader.mRequests )
			{
				if (r.mState == "ERROR" && (r.mMedia.len() < 7 || r.mMedia.slice(0, 7).tolower() != "terrain"))
				{
					if (count < 5)
					{
						errormedia += "    " + r + " : " + r.mState + " - " + r.mError + "\n";
					}
					else if (count == 6)
					{
						errormedia += "    ...\n";
					}

					count++;
				}
			}

			reasons += "    Failed Package Requests: " + count + "\n";

			if (requiredmedia != "")
			{
				reasons += "  --- Required Package Requests ---\n";
				reasons += requiredmedia;
			}

			if (normalmedia != "")
			{
				reasons += "  --- Normal Package Requests ---\n";
				reasons += normalmedia;
			}

			if (lowmedia != "")
			{
				reasons += "  --- Low Package Requests ---\n";
				reasons += lowmedia;
			}

			if (fetchmedia != "")
			{
				reasons += "  --- Fetch-Only Package Requests ---\n";
				reasons += fetchmedia;
			}

			if (errormedia != "")
			{
				reasons += "  --- Failed Package Requests ---\n";
				reasons += errormedia;
			}

			this._debug.setText("Loading", reasons);
		}
		else
		{
			this._debug.setText("Loading", null);
		}

		if (this.gPreSimWaiting)
		{
			showScreen = true;
		}

		if (showScreen == true && !this._loadScreenManager.getLoadScreenVisible())
		{
			::Audio.setForceAmbientMuted(true);
			this._loadScreenManager.setLoadScreenVisible(true, ::_avatar && ::_avatar.isAssembled() ? false : true);
		}
		else if (showScreen == false && this._loadScreenManager.getLoadScreenVisible())
		{
			::Audio.setForceAmbientMuted(false);
			this._loadScreenManager.setLoadScreenVisible(false);
			this._loadScreenManager.update(100, 100);
			this.mLoading = false;
		}

		if (this._loadScreenManager.getLoadScreenVisible() && this.mLoading)
		{
			if (this.mPendingRequiredAssemblyCount == null)
			{
				this.mPendingRequiredAssemblyCount = pendingRequiredAssemblyCount;
			}

			if (this.mRequiredMediaCount == null)
			{
				this.mRequiredMediaCount = requiredMediaCount;
			}

			local incomplete = this.mRequiredMediaCount == null || this.mPendingRequiredAssemblyCount == null;
			local pendingPages = this._sceneObjectManager.getUnassembledPageCount();

			if (this.mRequiredPages == null || this.mRequiredPages < pendingPages)
			{
				this.mRequiredPages = pendingPages;
			}

			local total;
			local remaining;

			if (this.mRequiredPages != null && this.mRequiredPages > 0)
			{
				total = this.mRequiredPages;
				remaining = pendingPages;
			}
			else
			{
				remaining = pendingRequiredAssemblyCount != null ? pendingRequiredAssemblyCount : 0;

				if (requiredMediaCount != null)
				{
					remaining += requiredMediaCount;
				}

				total = this.mPendingRequiredAssemblyCount != null ? this.mPendingRequiredAssemblyCount : 0;

				if (this.mRequiredMediaCount != null)
				{
					total += this.mRequiredMediaCount;
				}
			}

			local nextPercentage = this.mCurrentPercentage;

			if (total > 0)
			{
				if (incomplete)
				{
					nextPercentage = this._fakeProgress();
				}
				else
				{
					nextPercentage = remaining.tofloat() / total;
					nextPercentage = ((1.0 - nextPercentage) * 100).tointeger();

					if (nextPercentage - this.mRealPercentage > this.MAX_JUMP)
					{
						this.mGlitchCatcher++;

						if (this.mGlitchCatcher > this.MAX_GLITCH_CATCHER)
						{
							this.mGlitchCatcher = 0;
							this.mRealPercentage = nextPercentage;
						}
						else
						{
							nextPercentage = this.mRealPercentage + this.mGlitchCatcher / this.MAX_GLITCH_CATCHER.tofloat() * this.MAX_JUMP;
						}
					}
					else
					{
						this.mGlitchCatcher = 0;

						if (nextPercentage <= this.mRealPercentage)
						{
							nextPercentage = this._fakeProgress();
						}
						else
						{
							this.mRealPercentage = nextPercentage;
						}
					}
				}

				if (nextPercentage < this.mCurrentPercentage)
				{
					nextPercentage = this.mCurrentPercentage;
				}

				if (nextPercentage >= this.END_CUSHION)
				{
					nextPercentage = this.END_CUSHION;
				}
			}

			this._loadScreenManager.update(nextPercentage, 100);
			this.mCurrentPercentage = nextPercentage;
		}
	}

}

