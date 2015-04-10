class Screens.AdvCraft extends GUI.Frame
{
	/* Screen class name */
	static mClassName = "Screens.AdvCraft";

	mInventoryContainer = null;
	mScreenContainer = null;
	mButtonCraft = null;
	mButtonCancel = null;
	mButtonHelp = null;

	constructor()
	{
		GUI.Frame.constructor("Advanced Crafting");

		mInventoryContainer = GUI.ActionContainer("morph_stats", 1, 5, 0, 0, this, false);
		mInventoryContainer.setItemPanelVisible(false);
		mInventoryContainer.setValidDropContainer(true);
		mInventoryContainer.setAllowButtonDisownership(false);
		mInventoryContainer.setSlotDraggable(false, 0);
		mInventoryContainer.addListener(this);
		mInventoryContainer.addAcceptingFromProperties("inventory", AcceptFromProperties(this));

		mButtonCraft = GUI.NarrowButton("Craft");
		mButtonCraft.addActionListener(this);
		mButtonCraft.setReleaseMessage("onButtonPressed");

		mButtonCancel = GUI.RedNarrowButton("Cancel");
		mButtonCancel.addActionListener(this);
		mButtonCancel.setReleaseMessage("onButtonPressed");

		mButtonHelp = GUI.NarrowButton("Crafting Help Reference");
		mButtonHelp.addActionListener(this);
		mButtonHelp.setReleaseMessage("onButtonPressed");
		mButtonHelp.setFixedSize(220, 32);

		local buttonRow = GUI.Container();
		buttonRow.add(GUI.Spacer(2, 0));  //Seems to need a left margin?
		buttonRow.add(mButtonCraft);
		buttonRow.add(mButtonCancel);
		buttonRow.setPreferredSize(220, 32);

		mScreenContainer = GUI.Container(GUI.BoxLayoutV());

		local text = "Drag ingredients into the slots, then click <font color=\"00FF00\">Craft</font>.<br>" +
		             "If your recipe is correct, they will be removed, and<br>" +
		             "you will be granted the resulting item(s).<br>" + 
		             "For regular crafting (weapon and armor plans) use<br>" + 
                             "the crafting NPCs found in towns.";

		local label = GUI.HTML();
		label.setText(text);
		mScreenContainer.add(label);
		mScreenContainer.add(GUI.Spacer(0, 10));
		mScreenContainer.add(mInventoryContainer);
		mScreenContainer.add(GUI.Spacer(0, 10));
		mScreenContainer.add(buttonRow);
		mScreenContainer.add(mButtonHelp);

		setContentPane(mScreenContainer);
		setSize(280, 235);
	}
	function onButtonPressed(button)
	{
		if(button == mButtonCraft)
		{
			local items = mInventoryContainer.getAllActionButtons(true);
			local queryArgument = [];
			foreach(item in items)
			{
				queryArgument.append(item.mAction.mItemId);
			}
			if(queryArgument.len() == 0)
			{
				IGIS.info("You have not provided any items.");
				return;
			}
			disableButton();
			::_Connection.sendQuery("mod.craft", this, queryArgument);
		}
		else if(button == mButtonCancel)
		{
			close();
		}
		else if(button == mButtonHelp)
		{
			_URLManager.LaunchURL("Advanced Crafting");
		}
	}
	function disableButton()
	{
		mButtonCraft.setEnabled(false);
		::_eventScheduler.fireIn(3.0, this, "enableButton");
	}
	function enableButton()
	{
		mButtonCraft.setEnabled(true);
	}

	function setVisible(visible)
	{
		//This function derived from relevant parts of the trade screen.

		GUI.Frame.setVisible(visible);
		//Apparently this is called while the window is created, before the containers are
		//initialized.
		if(!visible && mInventoryContainer)
		{
			RestoreItems();
		}
	}
	function RestoreItems()
	{
		mInventoryContainer.removeAllActions();
		//This will unlock any slots that had been locked from the player moving items from the inventory
		local inv = ::Screens.get("Inventory",false)
		if(inv)
			inv.unlockAllActions();
	}

	function onQueryError(qa, error)
	{
		IGIS.error(error);
		RestoreItems();
		enableButton();

	}
	function onQueryComplete(qa, results)
	{
		RestoreItems();
		enableButton();
	}
}

function InputCommands::Craft(args)
{
	Screens.show("AdvCraft");
}