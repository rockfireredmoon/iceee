require("SceneObject");

class Screens.PetScreen extends GUI.Frame
{
	static mClassName = "Screens.PetScreen";
	mColumnListBrowse = null;

	mButtonPreview = null;
	mButtonPurchase = null;
	mHTMLLabel = null;

	mQueriedPetList = {};
	constructor()
	{
		GUI.Frame.constructor("Pet Browser");
		mColumnListBrowse = GUI.ColumnList();
		mColumnListBrowse.setPreferredSize(410, 320);
		mColumnListBrowse.setWindowSize(100);
		mColumnListBrowse.addColumn("Name", 70);
		mColumnListBrowse.addColumn("Level", 20);
		mColumnListBrowse.addColumn("Cost", 30);
		mColumnListBrowse.addColumn("Type", 60);
		mColumnListBrowse.addActionListener(this);

		local panel = GUI.ScrollPanel(mColumnListBrowse);
		panel.setPreferredSize(410, 320);

		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(panel);
		cmain.add(_buildButtonList());
		setContentPane(cmain);
		setSize(440, 380);
		FetchList();
	}
	function _buildButtonList()
	{
		mButtonPreview = _createButton("Preview");
		mButtonPreview.setTooltip("Preview will spawn the pet at your standing<br>location, but will appear on your screen only.<br>It will disappear after about 30 seconds.");
		mButtonPurchase = _createButton("Purchase");

		mHTMLLabel = GUI.HTML("<font color=\"00FF00\">Pets are cosmetic only.</font>");

		local container = GUI.Container();
		container.add(mButtonPreview);
		container.add(GUI.Spacer(20, 0));
		container.add(mButtonPurchase);
		container.add(GUI.Spacer(10, 0));
		container.add(mHTMLLabel);
		return container;
	}
	function _createButton(name)
	{
		local button = GUI.Button(name);
		button.addActionListener(this);
		button.setReleaseMessage("onButtonPressed");
		return button;
	}
	function onButtonPressed(button)
	{
		if(button == mButtonPreview)
			DoPreview();
		else if(button == mButtonPurchase)
			DoPurchase();
	}
	function DoPreview()
	{
		local sel = GetSelectedRow();
		if(sel == null)
		{
			IGIS.info("You must select a pet from the list.");
			return;
		}
		::_Connection.sendQuery("mod.pet.preview", this, [sel.cdef] );
	}
	function DoPurchase()
	{
		local sel = GetSelectedRow();
		if(sel == null)
		{
			IGIS.info("You must select a pet from the list.");
			return;
		}

		local callback =
		{
			cdef = sel.cdef,
			handler = this,
			function onActionSelected(mb, alt)
			{
				if( alt == "Yes" )
				{
					::_Connection.sendQuery("mod.pet.purchase", handler, [cdef]);
					//handler.DisableButton();
				}
			}
		};
		local text = "Are you sure you want to purchase:<br>" + sel.name;
		GUI.MessageBox.showYesNo(text, callback);
	}
	function DisableButton()
	{
		mButtonPurchase.setEnabled(false);
		::_eventScheduler.fireIn(5.0, this, "EnableButton");
	}
	function EnableButton()
	{
		mButtonPurchase.setEnabled(true);
	}
	function FetchList()
	{
		::_Connection.sendQuery("mod.pet.list", this, [] );
	}
	function onQueryError(qa, error)
	{
		IGIS.error(error);
	}
	function onQueryComplete(qa, results)
	{
		if(qa.query == "mod.pet.list")
			HandleResultList(results);
	}
	function HandleResultList(results)
	{
		mQueriedPetList.clear();
		if(results.len() == 0)
			return;
		foreach(i, r in results)
		{
			//Since most of these are strictly for display purposes, leave them as strings.
			local cdef = r[0];      //ID of the creature definition
			local name = r[1];      //Display name of the sidekick
			local level = r[2];     //Level required to use
			local coinAmount = r[3].tointeger();      //Coin amount to purchase
			local type = r[4];      //Short descriptive name since the model type may not be inferred from the name

			local cost = ConvertCurrencyString(coinAmount);

			mQueriedPetList[i] <- {cdef = cdef, name = name, level = level, cost = cost, type = type};
		}
		RefreshPetList();
	}
	function ConvertCurrencyString(coinAmount)
	{
		if(coinAmount == 0)
			return "Free";
		//coinAmount = integer, copper amount to convert (ex: 10000)
		local gold = (coinAmount / gCopperPerGold).tointeger();
		coinAmount -= gold * gCopperPerGold;
		local silver = (coinAmount / gCopperPerSilver).tointeger();
		coinAmount -= silver * gCopperPerSilver;
		local copper = coinAmount;
		local conv = "";
		if(gold > 0) conv += gold + "g ";
		if(silver > 0) conv += silver + "s ";
		if(copper > 0) conv += copper + "c ";
		return conv;
	}
	function GetSelectedRow()
	{
		local rows = mColumnListBrowse.getSelectedRows();
		if(rows.len() == 0)
			return null;
		local index = rows[0].tointeger();
		if(!(index in mQueriedPetList))
			return null;

		return mQueriedPetList[index];
	}
	function RefreshPetList()
	{
		mColumnListBrowse.removeAllRows();
		foreach(i, d in mQueriedPetList)
		{
			mColumnListBrowse.addRow([d.name, d.level, d.cost, d.type]);
		}
	}
}

function InputCommands::pet(args)
{
	Screens.toggle("PetScreen");
}

/*
function SceneObject::useCreature_mod(so)
{
	if(so.getMeta("pet_vendor"))
	{
		Screens.toggle("PetScreen");
		return;
	}
	if(so.CDEF_HINT_CREDIT_SHOP
	useCreature_old(so);
}

::SceneObject["useCreature_old"] <- ::SceneObject["useCreature"];
::SceneObject["useCreature"] <- ::SceneObject["useCreature_mod"];
*/
