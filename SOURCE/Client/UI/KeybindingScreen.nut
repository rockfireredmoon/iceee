this.require("UI/MainScreenElement");
this.require("Tools/PlayTool");
this.require("Tools/KeyConfigurationTool");
class this.KeybindingScreen extends this.GUI.Frame
{
	SCREEN_SIZE_X = 515;
	SCREEN_SIZE_Y = 535;
	mContent = null;
	mTabPane = null;
	mTabContents = null;
	mKeyConfigTool = null;
	mCurrentPage = 1;
	mCurrentTabName = "";
	mLowerSectionContainer = null;
	mDefaultsButton = null;
	mUnsetButton = null;
	mAcceptButton = null;
	mCancelButton = null;
	mPageFwButton = null;
	mPageLabel = null;
	mPageBkButton = null;
	mMainBindingOptions = null;
	mMovementBindingOptions = null;
	mTargetingBindingOptions = null;
	mQuickbarBindingOptions = null;
	mCurrentButtonInfo = null;
	mInstructionLabel = null;
	mTabAndBindingMapping = null;
	mPreviousActiveTool = null;
	mConfirmationMsgBox = null;
	mTemporaryBindings = null;
	constructor()
	{
		this.GUI.Frame.constructor("Key Configuration");
		this.mKeyConfigTool = this.KeyConfigurationTool();
		this.mKeyConfigTool.addListener(this);
		this.mTemporaryBindings = [];
		this.mCurrentButtonInfo = {
			button = null,
			command = null,
			primary = false
		};
		this.mTemporaryBindings = this.deepClone(::_playTool.getActiveKeybindings());
		this.mContent = this._buildContent();
		this.setContentPane(this.mContent);
		this.setSize(this.SCREEN_SIZE_X, this.SCREEN_SIZE_Y);
		this.mCurrentTabName = "Main";
		this.mTabAndBindingMapping = {};
		this.mTabAndBindingMapping.Main <- {
			options = this.mMainBindingOptions,
			startIndex = ::_playTool.MAIN_COMMAND_START,
			endIndex = ::_playTool.MAIN_COMMAND_END
		};
		this.mTabAndBindingMapping.Movement <- {
			options = this.mMovementBindingOptions,
			startIndex = ::_playTool.CONTROL_COMMAND_START,
			endIndex = ::_playTool.CONTROL_COMMAND_END
		};
		this.mTabAndBindingMapping.Targeting <- {
			options = this.mTargetingBindingOptions,
			startIndex = ::_playTool.TARGETING_COMMAND_START,
			endIndex = ::_playTool.TARGETING_COMMAND_END
		};
		this.mTabAndBindingMapping["Quick Bars"] <- {
			options = this.mQuickbarBindingOptions,
			startIndex = ::_playTool.QUICKBAR_COMMAND_START,
			endIndex = ::_playTool.QUICKBAR_COMMAND_END
		};
		this._updatePageLabel(this._getMaxPages(this.mCurrentTabName));
		this._refreshDisplayedBindings();
		this.mTabPane.addActionListener(this);
	}

	function onAcceptClicked( button )
	{
		this.serializeKeybindings();
		this.setVisible(false);
	}

	function onCancelClicked( button )
	{
		this.setVisible(false);
	}

	function onDefaultsClicked( button )
	{
		this.mTemporaryBindings = this.deepClone(::_playTool.getDefaultKeybindings());
		this._updateTab(this.mCurrentTabName);
	}

	function onPageForward( button )
	{
		this.mCurrentPage++;
		local maxPages = this._getMaxPages(this.mCurrentTabName);

		if (this.mCurrentPage > maxPages)
		{
			this.mCurrentPage = maxPages;
		}

		this.untoggleCurrentButton();
		this.mInstructionLabel.setText("");
		this._updatePageLabel(this._getMaxPages(this.mCurrentTabName));
		this._refreshDisplayedBindings();
	}

	function onPageBack( button )
	{
		this.mCurrentPage--;

		if (this.mCurrentPage < 1)
		{
			this.mCurrentPage = 1;
		}

		this.untoggleCurrentButton();
		this.mInstructionLabel.setText("");
		this._updatePageLabel(this._getMaxPages(this.mCurrentTabName));
		this._refreshDisplayedBindings();
	}

	function onKeyPressed( evt )
	{
		local key = this.KeyHelper.getKeyText(evt, true);
		local keyCodeCombo = [
			evt.keyCode,
			evt.isControlDown(),
			evt.isAltDown(),
			evt.isShiftDown()
		];
		local splitKeyBinding = this.Util.split(key, "+");
		local foundValidBindKey = false;

		foreach( key in splitKeyBinding )
		{
			if (key != this.KB(this.Key.VK_SHIFT) && key != this.KB(this.Key.VK_CONTROL) && key != this.KB(this.Key.VK_ALT))
			{
				foundValidBindKey = true;
			}
		}

		if (key.find("Unrecognized key") != null)
		{
			return;
		}

		if (key == "Esc" || key == "Enter")
		{
			return;
		}

		if (!foundValidBindKey)
		{
			return;
		}

		if (this.mCurrentButtonInfo && this.mCurrentButtonInfo.button)
		{
			if (this.mCurrentButtonInfo.button.getText() == key)
			{
				this._cancelBinding();
				return;
			}

			local keybindingUsage = "";
			local primaryScanButtons = [];
			local keyComboBoundTo;
			local optionsToSearch = [];
			optionsToSearch.extend(this.mMainBindingOptions);
			optionsToSearch.extend(this.mMovementBindingOptions);
			optionsToSearch.extend(this.mTargetingBindingOptions);
			optionsToSearch.extend(this.mQuickbarBindingOptions);

			foreach( option in optionsToSearch )
			{
				if (option.primaryButton.getText() == key || option.secondaryButton.getText() == key)
				{
					keyComboBoundTo = option;
					break;
				}
			}

			if (keyComboBoundTo != null)
			{
				local callback = {
					keyCombo = keyCodeCombo,
					currentButtonInfo = this.mCurrentButtonInfo,
					keyScreen = this,
					function onActionSelected( mb, alt )
					{
						if (alt == "Accept")
						{
							this.keyScreen._setBinding(this.keyCombo, this.currentButtonInfo);
						}
						else if (alt == "Cancel")
						{
							this.keyScreen._cancelBinding();
						}
					}

				};
				local newCommand = ::_playTool.Commands[this.mCurrentButtonInfo.command].kFunction;
				local message = "The " + key + " key is already configured to activate : " + keyComboBoundTo.label.getText() + "." + " Are you sure you want to change the " + key + " key to activate : " + newCommand + "?";
				this.mConfirmationMsgBox = this.GUI.MessageBox.showEx(message, [
					"Accept",
					"Cancel"
				], callback);
			}
			else
			{
				this._setBinding(keyCodeCombo, this.mCurrentButtonInfo);
			}
		}
	}

	function onKeybindingButtonPressed( button, primary )
	{
		this.untoggleCurrentButton();

		if (this.mCurrentButtonInfo && this.mCurrentButtonInfo.button)
		{
			this.mCurrentButtonInfo.button.setToggled(false);
			this.mCurrentButtonInfo.button = button;
			this.mCurrentButtonInfo.command = null;
			this.mCurrentButtonInfo.primary = false;
		}

		this.mCurrentButtonInfo.button = button;
		button.setToggled(true);
		local index = -1;
		local keybindings = this.mTabAndBindingMapping[this.mCurrentTabName].options;

		for( local i = 0; i < keybindings.len(); i++ )
		{
			local bindButton;

			if (primary)
			{
				bindButton = keybindings[i].primaryButton;
				this.mCurrentButtonInfo.primary = true;
			}
			else
			{
				bindButton = keybindings[i].secondaryButton;
				this.mCurrentButtonInfo.primary = false;
			}

			if (bindButton == button)
			{
				local startingOffset = this.mTabAndBindingMapping[this.mCurrentTabName].startIndex;
				index = i;
				this.mCurrentButtonInfo.command = startingOffset + index;
				break;
			}
		}

		this.mInstructionLabel.setText("Choose a key to activate: " + keybindings[index].label.getText());
		this.mPreviousActiveTool = ::_tools.getActiveTool();
		::_tools.setActiveTool(this.mKeyConfigTool);
	}

	function onPrimaryPressed( button )
	{
		this.onKeybindingButtonPressed(button, true);
	}

	function onSecondaryPressed( button )
	{
		this.onKeybindingButtonPressed(button, false);
	}

	function onTabSelected( tabBar, tab )
	{
		this.mCurrentTabName = tab.name;
		this.untoggleCurrentButton();
		this.mInstructionLabel.setText("");
		this.mCurrentPage = 1;
		this._updatePageLabel(this._getMaxPages(this.mCurrentTabName));
		this._refreshDisplayedBindings();
	}

	function onUnsetClicked( button )
	{
		if (this.mCurrentButtonInfo && this.mCurrentButtonInfo.button)
		{
			local currentButtonText = this.mCurrentButtonInfo.button.getText();

			if (currentButtonText != "None")
			{
				local vkCombo = this.mTemporaryBindings[currentButtonText].vkCombo;
				local keyCombo = [
					0,
					0,
					0,
					0
				];

				foreach( vk in vkCombo )
				{
					if (vk == this.Key.VK_CONTROL)
					{
						keyCombo[1] = true;
					}
					else if (vk == this.Key.VK_ALT)
					{
						keyCombo[2] = true;
					}
					else if (vk == this.Key.VK_SHIFT)
					{
						keyCombo[3] = true;
					}
					else
					{
						keyCombo[0] = vk;
					}
				}

				this._updateKeybinding(keyCombo, ::_playTool.Commands.UNBOUNDED, this.mCurrentButtonInfo.primary);
			}

			this.mInstructionLabel.setText("");
			this.untoggleCurrentButton();
			this._updateTab(this.mCurrentTabName);
		}
	}

	function restorePreviousTool()
	{
		if (this.mPreviousActiveTool)
		{
			::_tools.setActiveTool(this.mPreviousActiveTool);
			this.mPreviousActiveTool = null;
		}
	}

	function setVisible( value )
	{
		if (value == false)
		{
			this.untoggleCurrentButton();
		}
		else
		{
			this.mTemporaryBindings.clear();
			this.mTemporaryBindings = this.deepClone(::_playTool.getActiveKeybindings());
			this._updateTab(this.mCurrentTabName);
		}

		this.GUI.Frame.setVisible(value);
	}

	function serializeKeybindings()
	{
		local defaultKeybindings = ::_playTool.getDefaultKeybindings();
		local bindingsToSerialize = [];

		foreach( key, info in this.mTemporaryBindings )
		{
			local serializeInfo = false;

			if (!(key in defaultKeybindings))
			{
				serializeInfo = true;
			}
			else
			{
				local defaultBinding = defaultKeybindings[key];
				local vkCombosDifferent = false;

				if (defaultBinding.vkCombo.len() != info.vkCombo.len())
				{
					vkCombosDifferent = true;
				}
				else
				{
					for( local i = 0; i < info.vkCombo.len(); i++ )
					{
						if (defaultBinding.vkCombo[i] != info.vkCombo[i])
						{
							vkCombosDifferent = true;
							break;
						}
					}
				}

				if (defaultBinding.command != info.command || defaultBinding.primary != info.primary || vkCombosDifferent)
				{
					serializeInfo = true;
				}
			}

			if (serializeInfo)
			{
				local keyCombo = [
					0,
					0,
					0,
					0
				];

				foreach( vk in info.vkCombo )
				{
					if (vk == this.Key.VK_CONTROL)
					{
						keyCombo[1] = 1;
					}
					else if (vk == this.Key.VK_ALT)
					{
						keyCombo[2] = 1;
					}
					else if (vk == this.Key.VK_SHIFT)
					{
						keyCombo[3] = 1;
					}
					else
					{
						keyCombo[0] = vk;
					}
				}

				bindingsToSerialize.append([
					keyCombo[0],
					keyCombo[1],
					keyCombo[2],
					keyCombo[3],
					info.command,
					info.primary
				]);
			}
		}

		::Pref.set("control.Keybindings", bindingsToSerialize);
	}

	function untoggleCurrentButton()
	{
		this.restorePreviousTool();

		if (this.mCurrentButtonInfo && this.mCurrentButtonInfo.button)
		{
			this.mCurrentButtonInfo.button.setToggled(false);
			this.mCurrentButtonInfo.button = null;
			this.mCurrentButtonInfo.command = null;
			this.mCurrentButtonInfo.primary = false;
		}
	}

	function _buildContent()
	{
		local container = this.GUI.Container(this.GUI.BorderLayout());
		container.setInsets(10);
		this.mTabPane = this.GUI.TabbedPane(false);
		this.mTabPane.setTabPlacement("top");

		if (this.mTabPane.mTabsPane)
		{
			this.mTabPane.mTabsPane.setInsets(0, 5, 0, 0);
		}

		local mLowerSectionContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		local pageFwBwContainer = this.GUI.Container(this.GUI.GridLayout(1, 3));
		pageFwBwContainer.getLayoutManager().setColumns(40, "*", 40);
		this.mPageBkButton = this.GUI.SmallButton("LeftArrow");
		this.mPageBkButton.setSize(30, 15);
		this.mPageBkButton.addActionListener(this);
		this.mPageBkButton.setReleaseMessage("onPageBack");
		this.mPageFwButton = this.GUI.SmallButton("RightArrow");
		this.mPageFwButton.setSize(30, 15);
		this.mPageFwButton.addActionListener(this);
		this.mPageFwButton.setReleaseMessage("onPageForward");
		pageFwBwContainer.setAppearance("Panel");
		pageFwBwContainer.setSize(500, 35);
		pageFwBwContainer.setPreferredSize(500, 35);
		this.mPageLabel = this.GUI.Label("Page 1 / 1");
		this.mPageLabel.setTextAlignment(0.5, 0.5);
		local leftButtonContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		leftButtonContainer.add(this.mPageBkButton);
		leftButtonContainer.setInsets(6, 0, 0, 10);
		local rightButtonContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		rightButtonContainer.add(this.mPageFwButton);
		rightButtonContainer.setInsets(6, 10, 0, 0);
		pageFwBwContainer.add(leftButtonContainer);
		pageFwBwContainer.add(this.mPageLabel);
		pageFwBwContainer.add(rightButtonContainer);
		mLowerSectionContainer.add(pageFwBwContainer);
		this.mInstructionLabel = this.GUI.Label("");
		local instructionContainer = this.GUI.Container();
		instructionContainer.setSize(500, 50);
		instructionContainer.setPreferredSize(500, 50);
		instructionContainer.add(this.mInstructionLabel);
		mLowerSectionContainer.add(instructionContainer);
		mLowerSectionContainer.setSize(500, 120);
		mLowerSectionContainer.setPreferredSize(500, 120);
		mLowerSectionContainer.setInsets(5, 0, 0, 0);
		local buttonContainer = this.GUI.Container(this.GUI.BoxLayout());
		this.mDefaultsButton = this.GUI.NarrowButton("Defaults");
		this.mDefaultsButton.setReleaseMessage("onDefaultsClicked");
		this.mDefaultsButton.addActionListener(this);
		this.mUnsetButton = this.GUI.NarrowButton("Set to None");
		this.mUnsetButton.setReleaseMessage("onUnsetClicked");
		this.mUnsetButton.addActionListener(this);
		this.mAcceptButton = this.GUI.NarrowButton("Accept");
		this.mAcceptButton.setReleaseMessage("onAcceptClicked");
		this.mAcceptButton.addActionListener(this);
		this.mCancelButton = this.GUI.NarrowButton("Cancel");
		this.mCancelButton.setReleaseMessage("onCancelClicked");
		this.mCancelButton.addActionListener(this);
		buttonContainer.add(this.mDefaultsButton);
		buttonContainer.add(this.mUnsetButton);
		buttonContainer.add(this.GUI.Spacer(75, 1));
		buttonContainer.add(this.mAcceptButton);
		buttonContainer.add(this.mCancelButton);
		mLowerSectionContainer.add(buttonContainer);
		container.add(this.mTabPane, this.GUI.BorderLayout.CENTER);
		container.add(mLowerSectionContainer, this.GUI.BorderLayout.SOUTH);
		this.mTabPane.addTab("Main", this._buildMainKeyConfig());
		this.mTabPane.addTab("Movement", this._buildMovementKeyConfig());
		this.mTabPane.addTab("Targeting", this._buildTargetingKeyConfig());
		this.mTabPane.addTab("Quick Bars", this._buildQuickbarsKeyConfig());
		return container;
	}

	function _buildMainKeyConfig()
	{
		local container = this.GUI.Container(this.GUI.GridLayout(10, 6));
		container.getLayoutManager().setColumns(40, 165, 110, 10, 110, 30);
		container.getLayoutManager().setRows(32, 32, 32, 32, 32, 32, 32, 32, 32, 32);
		this.mMainBindingOptions = [];

		for( local i = ::_playTool.MAIN_COMMAND_START; i <= ::_playTool.MAIN_COMMAND_END; i++ )
		{
			local mainLabel = this.GUI.Label(::_playTool.Commands[i].kFunction);
			local primaryKey = "None";
			local secondaryKey = "None";
			local primaryMainButton = this.GUI.NarrowButton(primaryKey);
			primaryMainButton.addActionListener(this);
			primaryMainButton.setReleaseMessage("onPrimaryPressed");
			local secondaryMainButton = this.GUI.NarrowButton(secondaryKey);
			secondaryMainButton.addActionListener(this);
			secondaryMainButton.setReleaseMessage("onSecondaryPressed");
			this.mMainBindingOptions.append({
				label = mainLabel,
				primaryButton = primaryMainButton,
				secondaryButton = secondaryMainButton,
				command = i
			});
		}

		return container;
	}

	function _buildMovementKeyConfig()
	{
		local container = this.GUI.Container(this.GUI.GridLayout(10, 6));
		container.getLayoutManager().setColumns(40, 165, 110, 10, 110, 30);
		container.getLayoutManager().setRows(32, 32, 32, 32, 32, 32, 32, 32, 32, 32);
		this.mMovementBindingOptions = [];

		for( local i = ::_playTool.CONTROL_COMMAND_START; i <= ::_playTool.CONTROL_COMMAND_END; i++ )
		{
			local movementLabel = this.GUI.Label(::_playTool.Commands[i].kFunction);
			local primaryKey = "None";
			local secondaryKey = "None";
			local primaryMovementButton = this.GUI.NarrowButton(primaryKey);
			primaryMovementButton.addActionListener(this);
			primaryMovementButton.setReleaseMessage("onPrimaryPressed");
			local secondaryMovementButton = this.GUI.NarrowButton(secondaryKey);
			secondaryMovementButton.addActionListener(this);
			secondaryMovementButton.setReleaseMessage("onSecondaryPressed");
			this.mMovementBindingOptions.append({
				label = movementLabel,
				primaryButton = primaryMovementButton,
				secondaryButton = secondaryMovementButton,
				command = i
			});
		}

		return container;
	}

	function _buildTargetingKeyConfig()
	{
		local container = this.GUI.Container(this.GUI.GridLayout(10, 6));
		container.getLayoutManager().setColumns(40, 165, 110, 10, 110, 30);
		container.getLayoutManager().setRows(32, 32, 32, 32, 32, 32, 32, 32, 32, 32);
		this.mTargetingBindingOptions = [];

		for( local i = ::_playTool.TARGETING_COMMAND_START; i <= ::_playTool.TARGETING_COMMAND_END; i++ )
		{
			local targetingLabel = this.GUI.Label(::_playTool.Commands[i].kFunction);
			local primaryKey = "None";
			local secondaryKey = "None";
			local primaryTargetingButton = this.GUI.NarrowButton(primaryKey);
			primaryTargetingButton.addActionListener(this);
			primaryTargetingButton.setReleaseMessage("onPrimaryPressed");
			local secondaryTargetingButton = this.GUI.NarrowButton(secondaryKey);
			secondaryTargetingButton.addActionListener(this);
			secondaryTargetingButton.setReleaseMessage("onSecondaryPressed");
			this.mTargetingBindingOptions.append({
				label = targetingLabel,
				primaryButton = primaryTargetingButton,
				secondaryButton = secondaryTargetingButton,
				command = i
			});
		}

		return container;
	}

	function _buildQuickbarsKeyConfig()
	{
		local container = this.GUI.Container(this.GUI.GridLayout(9, 6));
		container.getLayoutManager().setColumns(40, 165, 110, 10, 110, 30);
		container.getLayoutManager().setRows(32, 32, 32, 32, 32, 32, 32, 32, 32);
		this.mQuickbarBindingOptions = [];

		for( local i = ::_playTool.QUICKBAR_COMMAND_START; i <= ::_playTool.QUICKBAR_COMMAND_END; i++ )
		{
			local quickbarLabel = this.GUI.Label(::_playTool.Commands[i].kFunction);
			local primaryKey = "None";
			local secondaryKey = "None";
			local primaryQuickbarButton = this.GUI.NarrowButton(primaryKey);
			primaryQuickbarButton.addActionListener(this);
			primaryQuickbarButton.setReleaseMessage("onPrimaryPressed");
			local secondaryQuickbarButton = this.GUI.NarrowButton(secondaryKey);
			secondaryQuickbarButton.addActionListener(this);
			secondaryQuickbarButton.setReleaseMessage("onSecondaryPressed");
			this.mQuickbarBindingOptions.append({
				label = quickbarLabel,
				primaryButton = primaryQuickbarButton,
				secondaryButton = secondaryQuickbarButton,
				command = i
			});
		}

		return container;
	}

	function _cancelBinding()
	{
		this.mInstructionLabel.setText("");
		this.untoggleCurrentButton();
		this._updateTab(this.mCurrentTabName);
	}

	function _setBinding( keyCodeCombo, buttonInfo )
	{
		this._updateKeybinding(keyCodeCombo, buttonInfo.command, buttonInfo.primary);
		this.mInstructionLabel.setText("");
		this.untoggleCurrentButton();
		this._updateTab(this.mCurrentTabName);
	}

	function _getKeybindingsPerPage( tabName )
	{
		if (tabName == "Main" || tabName == "Movement" || tabName == "Targeting")
		{
			return 9;
		}
		else if (tabName == "Quick Bars")
		{
			return 8;
		}
	}

	function _getMaxPages( tabName )
	{
		local optionsPerPage = this._getKeybindingsPerPage(tabName);
		local availableOptions = this.mTabAndBindingMapping[tabName].options;
		return (availableOptions.len().tofloat() / optionsPerPage.tofloat() + 0.99000001).tointeger();
	}

	function _refreshDisplayedBindings()
	{
		local currentTab = this.mTabPane.getSelectedTab().component;
		local maxPages = this._getMaxPages(this.mCurrentTabName);
		local keyBindingsPerPage = this._getKeybindingsPerPage(this.mCurrentTabName);
		currentTab.removeAll();
		local labelArray;
		local primaryButtonArray;
		local secondaryButtonArray;
		local keybindings = this.mTabAndBindingMapping[this.mCurrentTabName].options;
		this._updateTab(this.mCurrentTabName);
		local startingIndex = keyBindingsPerPage * (this.mCurrentPage - 1);
		local endingIndex = this.Math.min(startingIndex + keyBindingsPerPage, keybindings.len());
		currentTab.add(this.GUI.Spacer(1, 1));
		local label = this.GUI.Label("Key Function");
		label.setTextAlignment(0.5, 0.5);
		currentTab.add(label);
		label = this.GUI.Label("Primary");
		label.setTextAlignment(0.5, 0.5);
		currentTab.add(label);
		currentTab.add(this.GUI.Spacer(1, 1));
		local label = this.GUI.Label("Secondary");
		label.setTextAlignment(0.5, 0.5);
		currentTab.add(label);
		currentTab.add(this.GUI.Spacer(1, 1));

		for( local i = startingIndex; i < endingIndex; i++ )
		{
			currentTab.add(this.GUI.Spacer(1, 1));
			currentTab.add(keybindings[i].label);
			currentTab.add(keybindings[i].primaryButton);
			currentTab.add(this.GUI.Spacer(1, 1));
			currentTab.add(keybindings[i].secondaryButton);
			currentTab.add(this.GUI.Spacer(1, 1));
		}
	}

	function _updateKeybinding( keyCombo, newCommand, isprimary )
	{
		local newVkCombo = [];

		if (keyCombo[1] == true)
		{
			newVkCombo.append(this.Key.VK_CONTROL);
		}

		if (keyCombo[2] == true)
		{
			newVkCombo.append(this.Key.VK_ALT);
		}

		if (keyCombo[3] == true)
		{
			newVkCombo.append(this.Key.VK_SHIFT);
		}

		newVkCombo.append(keyCombo[0]);
		local keyText = this.KeyHelper.keyBindText(keyCombo[0], keyCombo[1], keyCombo[2], keyCombo[3], true);

		if (keyText in this.mTemporaryBindings)
		{
			this.mTemporaryBindings[keyText].command = newCommand;
			this.mTemporaryBindings[keyText].primary = isprimary;
			this.mTemporaryBindings[keyText].vkCombo = newVkCombo;
		}
		else
		{
			this.mTemporaryBindings[keyText] <- {
				command = newCommand,
				primary = isprimary,
				vkCombo = newVkCombo
			};
		}

		if (newCommand != ::_playTool.Commands.UNBOUNDED)
		{
			foreach( key, info in this.mTemporaryBindings )
			{
				if (key == keyText)
				{
					continue;
				}

				if (isprimary == true)
				{
					if (info.command == newCommand && info.primary == true)
					{
						info.command = ::_playTool.Commands.UNBOUNDED;
					}
				}
				else if (info.command == newCommand && info.primary == false)
				{
					info.command = ::_playTool.Commands.UNBOUNDED;
				}
			}
		}

		this._updateTab(this.mCurrentTabName);
	}

	function _updatePageLabel( maxPages )
	{
		this.mPageLabel.setText("Page " + this.mCurrentPage + " / " + maxPages);
	}

	function _updateTab( tabName )
	{
		local startIndex = 0;
		local endIndex = 0;
		local keybindings = this.mTabAndBindingMapping[tabName].options;
		startIndex = this.mTabAndBindingMapping[tabName].startIndex;
		endIndex = this.mTabAndBindingMapping[tabName].endIndex;

		for( local i = startIndex; i <= endIndex; i++ )
		{
			local commands = ::_playTool.getCommands();
			local primaryKey = "None";
			local secondaryKey = "None";
			local j = i - startIndex;
			local primaryMainButton = keybindings[j].primaryButton;
			local secondaryMainButton = keybindings[j].secondaryButton;
			local foundPrimary = false;
			local foundSecondary = false;

			foreach( k, v in this.mTemporaryBindings )
			{
				if (v.command == i)
				{
					if (v.primary == true)
					{
						foundPrimary = true;
						primaryKey = k;
					}
					else
					{
						foundSecondary = true;
						secondaryKey = k;
					}
				}

				if (foundPrimary && foundSecondary)
				{
					break;
				}
			}

			primaryMainButton.setText(primaryKey);
			secondaryMainButton.setText(secondaryKey);
		}
	}

}

