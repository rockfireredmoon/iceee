this.require("UI/ActionContainer");
this.require("UI/Screens");
class this.Screens.MacroCreator extends this.GUI.Frame
{
	static mClassName = "Screens.MacroCreator";
	mMacroContainer = null;
	mNameInput = null;
	mMacroCommandsInput = null;
	mSaveButton = null;
	mExitButton = null;
	mDeleteButton = null;
	mNewButton = null;
	mChangeIcon = null;
	mIconBrowser = null;
	mRows = 3;
	mColumns = 8;
	mMaxMacros = 0;
	constructor()
	{
		this.mMaxMacros = this.mRows * this.mColumns;
		this.GUI.Frame.constructor("MacroCreator");
		local content = this.GUI.Container(this.GUI.GridLayout(3, 1));
		this.setSize(300, 350);
		this.mMacroContainer = this.GUI.ActionContainer("macro_container", this.mRows, this.mColumns, 0, 0, this);
		this.mMacroContainer.getLayoutManager().setExpand(false);
		this.mMacroContainer.setHighlightSelectedIndex(true);
		this.mMacroContainer.addListener(this);
		content.add(this.mMacroContainer);
		local font = ::GUI.Font("Maiandra", 14);
		local nameLabel = this.GUI.Label("Name");
		this.mNameInput = this.GUI.InputArea();
		local commandLabel = this.GUI.Label("Commands");
		this.mMacroCommandsInput = this.GUI.InputArea();
		this.mMacroCommandsInput.setMultiLine(true);
		this.mDeleteButton = this.GUI.Button("Delete");
		this.mDeleteButton.setFont(font);
		this.mDeleteButton.setReleaseMessage("onDelete");
		this.mDeleteButton.addActionListener(this);
		this.mChangeIcon = this.GUI.Button("Change Icon");
		this.mChangeIcon.setFont(font);
		this.mChangeIcon.setReleaseMessage("onChangeIcon");
		this.mChangeIcon.addActionListener(this);
		this.mSaveButton = this.GUI.Button("Save");
		this.mSaveButton.setFont(font);
		this.mSaveButton.setReleaseMessage("onSave");
		this.mSaveButton.addActionListener(this);
		this.mNewButton = this.GUI.Button("New");
		this.mNewButton.setFont(font);
		this.mNewButton.setReleaseMessage("onNew");
		this.mNewButton.addActionListener(this);
		this.mExitButton = this.GUI.Button("Exit");
		this.mExitButton.setFont(font);
		this.mExitButton.setReleaseMessage("onExit");
		this.mExitButton.addActionListener(this);
		local commandLabelCombo = this.GUI.Container(this.GUI.GridLayout(1, 5));
		commandLabelCombo.add(commandLabel);
		commandLabelCombo.add(this.mMacroCommandsInput, {
			span = 4
		});
		content.add(commandLabelCombo);
		local nameLabelCombo = this.GUI.Container(this.GUI.GridLayout(1, 5));
		nameLabelCombo.add(nameLabel);
		nameLabelCombo.add(this.mNameInput, {
			span = 4
		});
		local bottomHalf = this.GUI.Container(this.GUI.GridLayout(3, 5));
		bottomHalf.add(this.GUI.Spacer(1, 1), {
			span = 5
		});
		bottomHalf.add(nameLabelCombo, {
			span = 5
		});
		bottomHalf.add(this.mDeleteButton);
		bottomHalf.add(this.mChangeIcon);
		bottomHalf.add(this.mNewButton);
		bottomHalf.add(this.mSaveButton);
		bottomHalf.add(this.mExitButton);
		content.add(bottomHalf);
		this.setContentPane(content);
		this.resetSelection();
		this.setContainerMoveProperties();
	}

	function displayMacros( macroList )
	{
		foreach( macro in macroList )
		{
			this.mMacroContainer.addAction(macro, false);
		}

		this.mMacroContainer.updateContainer();

		if (macroList.len() == this.mMaxMacros)
		{
			this.mNewButton.setEnabled(false);
		}
	}

	function onNewSelection( actionContainer, slot )
	{
		if (actionContainer == this.mMacroContainer)
		{
			local actionButton = slot.getActionButton();

			if (!actionButton)
			{
				return;
			}

			local action = actionButton.getAction();

			if (!action)
			{
				return;
			}

			this.mNameInput.setText(action.getName());
			this.mMacroCommandsInput.setText(action.getCommands());
			this.mDeleteButton.setEnabled(true);
			this.mChangeIcon.setEnabled(true);
		}
	}

	function onNew( evt )
	{
		local macroCount = ::_macroManager.getMacroCount();

		if (macroCount < this.mMaxMacros)
		{
			local newMacro = this.Macro("Icon/QuestionMark");
			this.mMacroContainer.addAction(newMacro, true);
			::_macroManager.addMacro(newMacro);
		}

		if (macroCount >= this.mMaxMacros - 1)
		{
			this.mNewButton.setEnabled(false);
		}
	}

	function onChangeIcon( evt )
	{
		this.mIconBrowser = this.Screens.IconBrowserScreen();
		this.mIconBrowser.setIconSelectionListener(this);
		this.mIconBrowser.setOverlay(this.GUI.POPUP_OVERLAY);
		this.mIconBrowser.setVisible(true);
	}

	function onIconSelected( icon )
	{
		local actionSlot = this.mMacroContainer.getSelectedSlot();

		if (!actionSlot)
		{
			return;
		}

		local actionButton = actionSlot.getActionButton();

		if (!actionButton)
		{
			return;
		}

		actionButton.setImageName(icon);
	}

	function onDelete( evt )
	{
		local actionSlot = this.mMacroContainer.getSelectedSlot();

		if (!actionSlot)
		{
			return;
		}

		local actionButton = actionSlot.getActionButton();

		if (!actionButton)
		{
			return;
		}

		local action = actionButton.getAction();

		if (!action)
		{
			return;
		}

		::_macroManager.removeMacro(action);
		::_macroManager.serializeMacros();
		this.mMacroContainer.removeAction(action);
		this.resetSelection();
	}

	function resetSelection()
	{
		this.mNameInput.setText("");
		this.mMacroCommandsInput.setText("");
		this.mDeleteButton.setEnabled(false);
		this.mChangeIcon.setEnabled(false);
		this.mMacroContainer.clearActiveSelection();
	}

	function onExit( evt )
	{
		this.setVisible(false);
	}

	function onSave( evt )
	{
		local actionSlot = this.mMacroContainer.getSelectedSlot();

		if (actionSlot)
		{
			local actionButton = actionSlot.getActionButton();

			if (actionButton)
			{
				local action = actionButton.getAction();

				if (action)
				{
					action.setName(this.mNameInput.getText());
					action.setCommands(this.mMacroCommandsInput.getText());
				}
			}
		}

		::_macroManager.serializeMacros();
	}

	function setContainerMoveProperties()
	{
		this.mMacroContainer.addMovingToProperties("quickbar", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar1", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar1", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar3", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar4", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar5", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar6", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar7", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("quickbar8", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mMacroContainer.addMovingToProperties("macro_container", this.MoveToProperties(this.MovementTypes.MOVE));
	}

	function setVisible( visible )
	{
		this.GUI.Frame.setVisible(visible);

		if (visible)
		{
			this.mMacroContainer.removeAllActions();
			this.mMacroCommandsInput.setText("");
			this.mNameInput.setText("");
			this.displayMacros(::_macroManager.getMacros());
		}
		else
		{
		}
	}

	function _addNotify()
	{
		this.GUI.Frame._addNotify();
		::_root.addListener(this);
	}

	function _removeNotify()
	{
		this.GUI.Frame._removeNotify();
		::_root.removeListener(this);
	}

}

class this.MacroManager 
{
	mMacroList = [];
	constructor()
	{
		this.mMacroList = [];
		this._unserializeMacros();
	}

	function addMacro( macro )
	{
		local newID = this.mMacroList.len();
		this.mMacroList.append(macro);
		macro.setID(newID);
	}

	function getMacros()
	{
		return this.mMacroList;
	}

	function getMacroCount()
	{
		return this.mMacroList.len();
	}

	function getMacroByID( id )
	{
		if (this.mMacroList.len() > id && id >= 0)
		{
			return this.mMacroList[id];
		}
		else
		{
			return null;
		}
	}

	function removeMacro( macro )
	{
		for( local i = 0; i < this.mMacroList.len(); i++ )
		{
			if (macro == this.mMacroList[i])
			{
				this.mMacroList.remove(i);
				return true;
			}
		}

		return false;
	}

	function serializeMacros()
	{
		local save = [];

		foreach( macro in this.mMacroList )
		{
			save.append([
				{
					name = macro.getName()
				},
				{
					image = macro.getForegroundImage()
				},
				{
					commands = macro.getCommands()
				}
			]);
		}

		this._cache.setCookie("Macros", this.serialize(save));
	}

	function _unserializeMacros()
	{
		this.mMacroList.clear();
		local macroData = this.unserialize(this._cache.getCookie("Macros"));

		if (!macroData)
		{
			return;
		}

		foreach( macro in macroData )
		{
			local newMacro = this.Macro("");

			if ("name" in macro[0])
			{
				newMacro.setName(macro[0].name);
			}

			if ("image" in macro[1])
			{
				newMacro.setImage(macro[1].image);
			}

			if ("commands" in macro[2])
			{
				newMacro.setCommands(macro[2].commands);
			}

			this.addMacro(newMacro);
		}
	}

}

class this.Macro extends this.Action
{
	mName = "";
	mCommands = "";
	mID = -1;
	constructor( image )
	{
		this.setImage(image);
	}

	function sendActivationRequest()
	{
		::EvalCommand(this.mCommands);
	}

	function getCommands()
	{
		return this.mCommands;
	}

	function getID()
	{
		return this.mID;
	}

	function getName()
	{
		return this.mName;
	}

	function getQuickbarString()
	{
		return "MACRO" + "id:" + this.mID;
	}

	function setCommands( commands )
	{
		this.mCommands = commands;
	}

	function setID( id )
	{
		this.mID = id;
	}

	function setName( name )
	{
		this.mName = name;
	}

}

this._macroManager <- this.MacroManager();
