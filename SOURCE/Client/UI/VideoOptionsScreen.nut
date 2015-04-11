this.require("GUI/GUI");
this.require("UI/Screens");
class this.Screens.VideoOptionsScreen extends this.GUI.Frame
{
	static mClassName = "Screen.VideoOptionsScreen";
	mDefaultsButton = null;
	mVideoQuality = null;
	mTerrainDistance = null;
	mCharacterShadows = null;
	mClutterDistance = null;
	mClutterDensity = null;
	mClutterVisible = null;
	mCacheUI = null;
	mFSAA = null;
	mDeferUpdate = false;
	mAmbientSFXSlider = null;
	mCombatSFXSlider = null;
	mMusicSlider = null;
	mMusic = null;
	mBloom = null;
	mSounds = null;
	mProfanityFilter = null;
	mBoldChatText = null;
	mMute = null;
	mOverheadNames = null;
	mShowTutorials = null;
	mResetTutorials = null;
	mTabPane = null;
	mMessageBroadcaster = null;
	mClickToMove = null;
	mChatWindowSize = null;
	mMouseSensitivitySlider = null;
	mShowComparisons = null;
	mSplatting = null;
	mBindPopup = null;
	mKeybindingButton = null;
	constructor()
	{
		this.GUI.Frame.constructor("Options");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mDefaultsButton = this._createButton(this.TXT("Defaults"), "onDefaultsPressed");
		local videoSection = this.GUI.Container(this.GUI.BoxLayoutV());
		videoSection.add(this._createVideoSection());
		local audioSection = this.GUI.Container(this.GUI.BoxLayoutV());
		audioSection.add(this._createAudioSection());
		local otherSection = this.GUI.Container(this.GUI.BoxLayoutV());
		otherSection.add(this._createOtherSection());
		local chatSection = this.GUI.Container(this.GUI.BoxLayoutV());
		chatSection.add(this._createChatSection());
		this.mTabPane = this.GUI.TabbedPane();
		this.mTabPane.setSize(300, 320);
		this.mTabPane.setPreferredSize(300, 320);
		this.mTabPane.setTabPlacement("top");
		this.mTabPane.setTabFontColor("E4E4E4");
		this.mTabPane.addTab("Video", videoSection);
		this.mTabPane.addTab("Audio", audioSection);
		this.mTabPane.addTab("Other", otherSection);
		this.mTabPane.addTab("Chat", chatSection);
		this.mTabPane.addActionListener(this);
		local cmain = this.GUI.Container(this.GUI.BoxLayoutV());
		cmain.setInsets(5);
		cmain.getLayoutManager().setGap(10);
		cmain.add(this.mTabPane);
		cmain.add(this.mDefaultsButton);
		this._fetchSettings();
		this.setContentPane(cmain);
		this.setSize(320, 400);
		this.centerOnScreen();
	}

	function _createButton( label, msg )
	{
		local b = this.GUI.NarrowButton(label);
		b.setReleaseMessage(msg);
		b.addActionListener(this);
		return b;
	}

	function _createVideoSection()
	{
		this.mVideoQuality = this.GUI.DropDownList();
		this.mVideoQuality.addChoice("Low");
		this.mVideoQuality.addChoice("Medium");
		this.mVideoQuality.addChoice("High");
		this.mVideoQuality.addChoice("Custom");
		this.mVideoQuality.addSelectionChangeListener(this);
		this.mTerrainDistance = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 1500, 3000, 100);
		this.mTerrainDistance.addChangeListener(this);
		this.mClutterDensity = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 0.0, 2.0, 100);
		this.mClutterDensity.addChangeListener(this);
		this.mClutterDistance = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 150, 500, 100);
		this.mClutterDistance.addChangeListener(this);
		this.mCharacterShadows = this.GUI.CheckBox();
		this.mCharacterShadows.addActionListener(this);
		this.mClutterVisible = this.GUI.CheckBox();
		this.mClutterVisible.addActionListener(this);
		this.mCacheUI = this.GUI.CheckBox();
		this.mCacheUI.addActionListener(this);
		this.mSplatting = this.GUI.CheckBox();
		this.mSplatting.addActionListener(this);
		this.mBloom = this.GUI.CheckBox();
		this.mBloom.addActionListener(this);
		this.mFSAA = this.GUI.DropDownList();
		this.mFSAA.addChoice("No Anti-Aliasing");
		local modes = ::System.querySupportedMultisamples();

		foreach( m in modes )
		{
			this.mFSAA.addChoice(m.tostring() + "x Anti-Aliasing");
		}

		this.mFSAA.addSelectionChangeListener(this);
		local p = this.GUI.Container(this.GUI.GridLayout(10, 2));
		p.add(this.GUI.Label(this.TXT("Video Quality")));
		p.add(this.mVideoQuality, this.GUI.GridLayout.EXPAND_W);
		p.add(this.GUI.Label(this.TXT("High Detail Terrain")));
		p.add(this.mSplatting, 0);
		p.add(this.GUI.Label(this.TXT("Terrain Distance")));
		p.add(this.mTerrainDistance, this.GUI.GridLayout.EXPAND_W);
		p.add(this.GUI.Label(this.TXT("Clutter Density")));
		p.add(this.mClutterDensity, this.GUI.GridLayout.EXPAND_W);
		p.add(this.GUI.Label(this.TXT("Clutter Distance")));
		p.add(this.mClutterDistance, this.GUI.GridLayout.EXPAND_W);
		p.add(this.GUI.Label(this.TXT("Clutter")));
		p.add(this.mClutterVisible, 0);
		p.add(this.GUI.Label(this.TXT("Character Shadows")));
		p.add(this.mCharacterShadows, 0);
		p.add(this.GUI.Label(this.TXT("Fast UI")));
		p.add(this.mCacheUI, 0);
		p.add(this.GUI.Label(this.TXT("Bloom")));
		p.add(this.mBloom, 0);
		p.add(this.GUI.Label(this.TXT("Anti-Aliasing")));
		p.add(this.mFSAA, 0);
		return p;
	}

	function _createAudioSection()
	{
		this.mMusic = this.GUI.CheckBox();
		this.mMusic.addActionListener(this);
		this.mMute = this.GUI.CheckBox();
		this.mMute.addActionListener(this);
		this.mSounds = this.GUI.CheckBox();
		this.mSounds.addActionListener(this);
		this.mAmbientSFXSlider = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 0.0, 1.0, 100);
		this.mAmbientSFXSlider.addChangeListener(this);
		this.mCombatSFXSlider = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 0.0, 1.0, 100);
		this.mCombatSFXSlider.addChangeListener(this);
		this.mMusicSlider = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 0.0, 1.0, 100);
		this.mMusicSlider.addChangeListener(this);
		local p = this.GUI.Container(this.GUI.GridLayout(6, 2));
		p.add(this.GUI.Label(this.TXT("Mute All Audio")));
		p.add(this.mMute, 0);
		p.add(this.GUI.Label(this.TXT("Mute Music")));
		p.add(this.mMusic, 0);
		p.add(this.GUI.Label(this.TXT("Mute Sounds")));
		p.add(this.mSounds, 0);
		p.add(this.GUI.Label(this.TXT("Ambient Sound Level")));
		p.add(this.mAmbientSFXSlider);
		p.add(this.GUI.Label(this.TXT("Combat Sound Level")));
		p.add(this.mCombatSFXSlider);
		p.add(this.GUI.Label(this.TXT("Music Level")));
		p.add(this.mMusicSlider);
		return p;
	}

	function _createOtherSection()
	{
		this.mClickToMove = this.GUI.CheckBox();
		this.mClickToMove.addActionListener(this);
		this.mShowTutorials = this.GUI.CheckBox();
		this.mShowTutorials.addActionListener(this);
		this.mShowComparisons = this.GUI.CheckBox();
		this.mShowComparisons.addActionListener(this);
		this.mBindPopup = this.GUI.CheckBox();
		this.mBindPopup.addActionListener(this);
		local resetDefaultUIPosition = this.GUI.Button("Reset");
		resetDefaultUIPosition.setReleaseMessage("onResetDefaultPositionClicked");
		resetDefaultUIPosition.addActionListener(this);
		this.mResetTutorials = this.GUI.Button("Reset");
		this.mResetTutorials.setReleaseMessage("onResetTutorialsClicked");
		this.mResetTutorials.addActionListener(this);
		this.mOverheadNames = this.GUI.DropDownList();

		foreach( i, x in ::OverheadNames )
		{
			this.mOverheadNames.addChoice(x);
		}

		this.mOverheadNames.addSelectionChangeListener(this);
		local p = this.GUI.Container(this.GUI.GridLayout(9, 2));
		p.add(this.GUI.Label(this.TXT("Show Tutorials")));
		p.add(this.mShowTutorials, 0);
		p.add(this.GUI.Label(this.TXT("Click To Move")));
		p.add(this.mClickToMove, 0);
		p.add(this.GUI.Label(this.TXT("Equipment Tooltip Comparisons")));
		p.add(this.mShowComparisons, 0);
		p.add(this.GUI.Label(this.TXT("Show bind on pickup/equip popup")));
		p.add(this.mBindPopup, 0);
		p.add(this.GUI.Label(this.TXT("Reset Tutorials")));
		p.add(this.mResetTutorials, 0);
		p.add(this.GUI.Label(this.TXT("Reset Default UI Position")));
		p.add(resetDefaultUIPosition, 0);
		p.add(this.GUI.Label(this.TXT("Display Over-head Names")));
		p.add(this.mOverheadNames, 0);
		this.mMouseSensitivitySlider = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 0.050000001, 0.34999999, 100);
		this.mMouseSensitivitySlider.addChangeListener(this);
		p.add(this.GUI.Label(this.TXT("Mouse Sensitivity")));
		p.add(this.mMouseSensitivitySlider, 0);
		this.mKeybindingButton = this.GUI.Button("Change");
		this.mKeybindingButton.setReleaseMessage("onKeybindingPressed");
		this.mKeybindingButton.addActionListener(this);
		p.add(this.GUI.Label(this.TXT("Key Configuration")));
		p.add(this.mKeybindingButton, 0);
		return p;
	}

	function _createChatSection()
	{
		local p = this.GUI.Container(this.GUI.GridLayout(3, 2));
		this.mProfanityFilter = this.GUI.CheckBox();
		this.mProfanityFilter.addActionListener(this);
		p.add(this.GUI.Label(this.TXT("Profanity Filter")));
		p.add(this.mProfanityFilter, 0);
		this.mBoldChatText = this.GUI.CheckBox();
		this.mBoldChatText.addActionListener(this);
		p.add(this.GUI.Label(this.TXT("Bold Chat Text")));
		p.add(this.mBoldChatText, 0);
		this.mChatWindowSize = this.GUI.DropDownList();

		foreach( i, x in ::ChatWindowSizes )
		{
			this.mChatWindowSize.addChoice(x);
		}

		this.mChatWindowSize.addSelectionChangeListener(this);
		p.add(this.GUI.Label(this.TXT("Change Chat Window Size")));
		p.add(this.mChatWindowSize, 0);
		return p;
	}

	function onSelectionChange( list )
	{
		if (list == this.mFSAA)
		{
			local entry = list.getCurrent();

			if (entry == "No Anti-Aliasing")
			{
				::Pref.set("video.FSAA", 0);
			}
			else
			{
				local text = entry.slice(0, entry.find("x"));
				::Pref.set("video.FSAA", text.tointeger());
			}

			this.mVideoQuality.setValue("Custom");
			this.Pref.set("video.Settings", "Custom");
		}
		else if (list == this.mChatWindowSize)
		{
			local entry = list.getCurrent();

			if (::_ChatWindow)
			{
				::_ChatWindow.handleWindowResized(entry);
			}
		}
		else if (list == this.mVideoQuality)
		{
			local entry = list.getCurrent();
			this.Util.updateFromVideoSettings(entry);
			this.Pref.set("video.Settings", entry);
			this._fetchSettings();
		}
		else if (list == this.mOverheadNames)
		{
			local a = this.Util.indexOf(::OverheadNames, this.mOverheadNames.getCurrent());
			::Pref.set("igis.OverheadNames", a);
		}
	}

	function onSliderUpdated( slider )
	{
		if (slider == this.mTerrainDistance || slider == this.mClutterDensity || slider == this.mClutterDistance)
		{
			this.mVideoQuality.setValue("Custom");
			this.Pref.set("video.Settings", "Custom");
		}

		this._applySliderSettings();
	}

	function onActionPerformed( component, ... )
	{
		this._applySettings();

		if (component == this.mCharacterShadows || component == this.mClutterVisible || component == this.mBloom || component == this.mSplatting)
		{
			this.mVideoQuality.setValue("Custom");
			this.Pref.set("video.Settings", "Custom");
		}
	}

	function onInputComplete( component )
	{
		this._applySettings();
	}

	function onResetTutorialsClicked( button )
	{
		if (button == this.mResetTutorials)
		{
			::_tutorialManager.resetTutorials();
		}
	}

	function onResetDefaultPositionClicked( button )
	{
		::_root.setDefaultWindowPositions();
	}

	function onKeybindingPressed( button )
	{
		this.Screens.show("KeybindingScreen");
	}

	function onDefaultsPressed( button )
	{
		local selectedTab = this.mTabPane.getSelectedTab();
		local tabName = selectedTab.name;

		if (tabName == "Video")
		{
			::Pref.set("video.Splatting", true);
			::Pref.set("video.TerrainDistance", 2500);
			::Pref.set("video.ClutterDensity", 1.0);
			::Pref.set("video.ClutterDistance", 350.0);
			::Pref.set("video.UICache", true);
			::Pref.set("video.FSAA", 0);
			this.Util.updateFromVideoSettings("None");
		}
		else if (tabName == "Audio")
		{
			::Pref.set("audio.Music", true);
			::Pref.set("audio.Mute", false);
			::Pref.set("audio.Sounds", false);
			::Pref.set("audio.CombatSFXLevel", 1.0);
			::Pref.set("audio.AmbientSFXLevel", 1.0);
			::Pref.set("audio.MusicLevel", 1.0);
		}
		else if (tabName == "Other")
		{
			::Pref.set("tutorial.active", true);
			::Pref.set("gameplay.mousemovement", false);
			::Pref.set("gameplay.eqcomparisons", true);
			::Pref.set("other.BindPopup", true);
			::Pref.set("igis.OverheadNames", "a");
			::Pref.set("other.MouseSensitivity", 0.15000001);
		}
		else if (tabName == "Chat")
		{
			::Pref.set("chat.ProfanityFilter", true);
			::Pref.set("chat.BoldText", false);
			::Pref.set("chatwindow.windowSize", "Small");
		}

		this._fetchSettings();
	}

	function _applySettings()
	{
		if (this.mDeferUpdate)
		{
			return;
		}

		::Pref.set("video.Splatting", this.mSplatting.getChecked());
		::Pref.set("video.CharacterShadows", this.mCharacterShadows.getChecked());
		::Pref.set("video.ClutterVisible", this.mClutterVisible.getChecked());
		::Pref.set("video.Bloom", this.mBloom.getChecked());
		::Pref.set("video.UICache", this.mCacheUI.getChecked());
		::Pref.set("audio.Music", !this.mMusic.getChecked());
		::Pref.set("audio.Mute", this.mMute.getChecked());
		::Pref.set("audio.Sounds", this.mSounds.getChecked());
		::Pref.set("chat.ProfanityFilter", this.mProfanityFilter.getChecked());
		::Pref.set("chat.BoldText", this.mBoldChatText.getChecked());
		::Pref.set("gameplay.mousemovement", this.mClickToMove.getChecked());
		::Pref.set("tutorial.active", this.mShowTutorials.getChecked());
		::Pref.set("gameplay.eqcomparisons", this.mShowComparisons.getChecked());
		::Pref.set("other.BindPopup", this.mBindPopup.getChecked());
		local a = this.Util.indexOf(::OverheadNames, this.mOverheadNames.getCurrent());
		::Pref.set("igis.OverheadNames", a);
		this._applySliderSettings();
	}

	function _applySliderSettings()
	{
		::Pref.set("audio.CombatSFXLevel", this.mCombatSFXSlider.getValue());
		::Pref.set("audio.AmbientSFXLevel", this.mAmbientSFXSlider.getValue());
		::Pref.set("audio.MusicLevel", this.mMusicSlider.getValue());
		::Pref.set("video.TerrainDistance", this.mTerrainDistance.getValue());
		::Pref.set("video.ClutterDensity", this.mClutterDensity.getValue());
		::Pref.set("video.ClutterDistance", this.mClutterDistance.getValue());
		::Pref.set("other.MouseSensitivity", this.mMouseSensitivitySlider.getValue());
	}

	function _fetchSettings()
	{
		this.mDeferUpdate = true;
		this.mVideoQuality.setCurrent(::Pref.get("video.Settings"));
		this.mSplatting.setChecked(::Pref.get("video.Splatting"));
		this.mTerrainDistance.setValue(::Pref.get("video.TerrainDistance"));
		this.mClutterDistance.setValue(::Pref.get("video.ClutterDistance"));
		this.mClutterDensity.setValue(::Pref.get("video.ClutterDensity"));
		this.mClutterVisible.setChecked(::Pref.get("video.ClutterVisible"));
		this.mCharacterShadows.setChecked(::Pref.get("video.CharacterShadows"));
		this.mBloom.setChecked(::Pref.get("video.Bloom"));
		this.mCacheUI.setChecked(::Pref.get("video.UICache"));
		this.mMusic.setChecked(::Pref.get("audio.Music") == false);
		this.mMute.setChecked(::Pref.get("audio.Mute"));
		this.mSounds.setChecked(::Pref.get("audio.Sounds"));
		this.mProfanityFilter.setChecked(::Pref.get("chat.ProfanityFilter"));
		this.mBoldChatText.setChecked(::Pref.get("chat.BoldText"));
		this.mShowTutorials.setChecked(::Pref.get("tutorial.active"));
		this.mShowComparisons.setChecked(::Pref.get("gameplay.eqcomparisons"));
		this.mBindPopup.setChecked(::Pref.get("other.BindPopup"));
		this.mClickToMove.setChecked(::Pref.get("gameplay.mousemovement"));
		this.mCombatSFXSlider.setValue(::Pref.get("audio.CombatSFXLevel"));
		this.mAmbientSFXSlider.setValue(::Pref.get("audio.AmbientSFXLevel"));
		this.mMusicSlider.setValue(::Pref.get("audio.MusicLevel"));
		this.mOverheadNames.setCurrent(::OverheadNames[::Pref.get("igis.OverheadNames")]);
		local fsaa = ::Pref.get("video.FSAA");
		this.mFSAA.setCurrent("No Anti-Aliasing", false);
		this.mFSAA.setCurrent(fsaa.tostring() + "x Anti-Aliasing", false);
		local chatWindowSize = ::Pref.get("chatwindow.windowSize");
		this.mChatWindowSize.setCurrent(chatWindowSize, true);
		this.mMouseSensitivitySlider.setValue(::Pref.get("other.MouseSensitivity"));
		this.mDeferUpdate = false;
	}

	function setChatWindowDropdownOption( chatWindowSize )
	{
		if (this.mChatWindowSize)
		{
			this.mChatWindowSize.setCurrent(chatWindowSize, true);
		}
	}

	function setVisible( value )
	{
		this.GUI.Panel.setVisible(value);

		if (value)
		{
			::Audio.playSound("Sound-SettingsOpen.ogg");
		}
		else
		{
			::Audio.playSound("Sound-SettingsClose.ogg");
		}
	}

}

