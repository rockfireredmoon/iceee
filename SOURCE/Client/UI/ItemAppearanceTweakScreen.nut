this.require("UI/Screens");
class this.Screens.ItemAppearanceTweak extends this.GUI.Frame
{
	mItemList = null;
	mTooltipContainer = null;
	mAppearanceEntry = null;
	mAppearanceCopyButton = null;
	mAppearancePasteButton = null;
	mAppearanceDeleteButton = null;
	mPreviewButton = null;
	mMaxItemDefs = 200;
	mCurrentIndex = 0;
	mCurrentItemDefOffset = null;
	mIconBrowserButton = null;
	mBackgroundIconBrowserButton = null;
	mForegroundIconEntry = null;
	mBackgroundIconEntry = null;
	mForegroundIconPreview = null;
	mBackgroundIconPreview = null;
	mCombinedIconPreviewBG = null;
	mCombinedIconPreviewFG = null;
	mResetPreviewButton = null;
	mNameFilter = null;
	mFilterList = null;
	mTooltip = null;
	mAddBtn = null;
	mNameMap = null;
	mAppearanceList = null;
	mSelectingForeground = false;
	mSelectingBackground = false;
	constructor()
	{
		this.GUI.Frame.constructor("Item Appearance");
		local content = this.GUI.Panel(this.GUI.GridLayout(1, 4));
		content.getLayoutManager().setColumns(300, 265, 270, 48);
		local listContent = this.GUI.Container(this.GUI.GridLayout(3, 1));
		this.mItemList = this.GUI.ColumnList();
		this.mItemList.addColumn("ID", 45);
		this.mItemList.addColumn("Name", 200);
		this.mItemList.addActionListener(this);
		local listContainer = this.GUI.Container(this.GUI.GridLayout(1, 2));
		listContainer.getLayoutManager().setColumns(300, 25);
		listContainer.add(this.mItemList);
		listContainer.add(this.GUI.ScrollPanel(this.mItemList));
		this.mFilterList = this.GUI.DropDownList();
		this.mFilterList.addSelectionChangeListener(this);
		this.mFilterList.setChangeMessage("onFilterChanged");
		this.mFilterList.addChoice("All Items");
		this.mFilterList.addChoice("No Appearance or Icon");
		this.mFilterList.addChoice("No Appearance");
		this.mFilterList.addChoice("No Icon");
		this.mFilterList.addChoice("Consumable");
		this.mFilterList.addChoice("Weapons");
		this.mFilterList.addChoice("Armor");
		this.mFilterList.addChoice("Talisman");
		this.mFilterList.addChoice("Special");
		this.mFilterList.addChoice("Quest");
		this.mFilterList.addChoice("Basic");
		local filterContents = this.GUI.Container(this.GUI.GridLayout(1, 2));
		filterContents.getLayoutManager().setColumns(48, 250);
		filterContents.add(this.GUI.Label("Class:"));
		filterContents.add(this.mFilterList);
		this.mNameFilter = this.GUI.InputArea();
		this.mNameFilter.addActionListener(this);
		local nameFilterContents = this.GUI.Container(this.GUI.GridLayout(1, 2));
		nameFilterContents.getLayoutManager().setColumns(48, 250);
		nameFilterContents.add(this.GUI.Label("Name:"));
		nameFilterContents.add(this.mNameFilter);
		this.mAppearanceEntry = this.GUI.InputArea();
		this.mAppearanceEntry.addActionListener(this);
		this.mAppearanceEntry.setLocked(true);
		local appearanceContents = this.GUI.Container(this.GUI.GridLayout(1, 5));
		appearanceContents.getLayoutManager().setColumns(64, 69, 25, 25, 25);
		appearanceContents.add(this.GUI.Label("Appearance:"));
		appearanceContents.add(this.mAppearanceEntry);
		this.mAppearanceCopyButton = this.GUI.Button("C", this, "onCopyAppearance");
		this.mAppearancePasteButton = this.GUI.Button("P", this, "onPasteAppearance");
		this.mAppearanceDeleteButton = this.GUI.Button("X", this, "onDeleteAppearance");
		appearanceContents.add(this.mAppearanceCopyButton);
		appearanceContents.add(this.mAppearancePasteButton);
		appearanceContents.add(this.mAppearanceDeleteButton);
		this.mForegroundIconEntry = this.GUI.InputArea();
		this.mForegroundIconEntry.addActionListener(this);
		this.mBackgroundIconEntry = this.GUI.InputArea();
		this.mBackgroundIconEntry.addActionListener(this);
		this.mIconBrowserButton = this.GUI.Button("...", this, "onForegroundIconBrowse");
		this.mBackgroundIconBrowserButton = this.GUI.Button("...", this, "onBackgroundIconBrowse");
		local iconContents = this.GUI.Container(this.GUI.GridLayout(1, 3));
		iconContents.getLayoutManager().setColumns(68, 112, 30);
		iconContents.add(this.GUI.Label("Foreground:"));
		iconContents.add(this.mForegroundIconEntry);
		iconContents.add(this.mIconBrowserButton);
		content.add(iconContents);
		listContent.add(filterContents);
		listContent.add(nameFilterContents);
		listContent.add(listContainer);
		listContent.getLayoutManager().setRows(25, 25, 250);
		content.add(listContent);
		this.mTooltipContainer = this.GUI.Panel(this.GUI.BoxLayoutV(true));
		local appearanceEntryFields = this.GUI.Container(this.GUI.GridLayout(1, 1));
		appearanceEntryFields.add(iconContents);
		appearanceEntryFields.getLayoutManager().setRows(25);
		this.mForegroundIconPreview = this.GUI.ImageButton();
		this.mForegroundIconPreview.setGlowEnabled(false);
		this.mForegroundIconPreview.addActionListener(this);
		this.mForegroundIconPreview.setPressMessage("onForegroundIconBrowse");
		local appearanceTmp = this.GUI.Container(this.GUI.GridLayout(1, 2));
		appearanceTmp.getLayoutManager().setColumns(215, 48);
		appearanceTmp.add(appearanceEntryFields);
		appearanceTmp.add(this.mForegroundIconPreview);
		local appearancePanel = this.GUI.Container(this.GUI.GridLayout(2, 1));
		appearancePanel.getLayoutManager().setRows(50, 250);
		appearancePanel.add(appearanceTmp);
		appearancePanel.add(this.mTooltipContainer);
		content.add(appearancePanel);
		local rightSideContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		rightSideContainer.getLayoutManager().setRows(25, 70, 200);
		local backgroundIconContents = this.GUI.Container(this.GUI.GridLayout(1, 3));
		backgroundIconContents.getLayoutManager().setColumns(68, 112, 25);
		backgroundIconContents.add(this.GUI.Label("Background:"));
		backgroundIconContents.add(this.mBackgroundIconEntry);
		backgroundIconContents.add(this.mBackgroundIconBrowserButton);
		local backgroundIconContainer = this.GUI.Container(this.GUI.GridLayout(1, 1));
		backgroundIconContainer.getLayoutManager().setRows(50);
		backgroundIconContainer.getLayoutManager().setColumns(49);
		this.mBackgroundIconPreview = this.GUI.ImageButton();
		this.mBackgroundIconPreview.setGlowEnabled(false);
		this.mBackgroundIconPreview.addActionListener(this);
		this.mBackgroundIconPreview.setPressMessage("onBackgroundIconBrowse");
		local combinedIconContainer = this.GUI.Container(null);
		combinedIconContainer.setSize(49, 50);
		combinedIconContainer.setPreferredSize(49, 50);
		this.mCombinedIconPreviewBG = this.GUI.ImageButton();
		this.mCombinedIconPreviewBG.setGlowEnabled(false);
		this.mCombinedIconPreviewBG.setSize(49, 50);
		this.mCombinedIconPreviewBG.setPreferredSize(49, 50);
		this.mCombinedIconPreviewBG.setPosition(100, 20);
		this.mCombinedIconPreviewFG = this.GUI.ImageButton();
		this.mCombinedIconPreviewFG.setGlowEnabled(false);
		this.mCombinedIconPreviewFG.setSize(49, 50);
		this.mCombinedIconPreviewFG.setPreferredSize(49, 50);
		this.mCombinedIconPreviewFG.setPosition(100, 20);
		local combinedLabel = this.GUI.Label("Combined Preview:");
		combinedLabel.setPosition(0, 20);
		combinedIconContainer.add(combinedLabel);
		combinedIconContainer.add(this.mCombinedIconPreviewBG);
		combinedIconContainer.add(this.mCombinedIconPreviewFG);
		backgroundIconContainer.add(this.mBackgroundIconPreview);
		rightSideContainer.add(backgroundIconContents);
		rightSideContainer.add(combinedIconContainer);
		this.mAppearanceList = this.GUI.ColumnList();
		this.mAppearanceList.addColumn("Attach Point", 90);
		this.mAppearanceList.addColumn("Appearance", 180);
		this.mAppearanceList.addActionListener(this);
		local appearanceListContainer = this.GUI.Container(this.GUI.GridLayout(2, 1));
		appearanceListContainer.getLayoutManager().setRows(170, 25);
		appearanceListContainer.add(this.mAppearanceList);
		appearanceListContainer.add(appearanceContents);
		rightSideContainer.add(appearanceListContainer);
		content.add(rightSideContainer);
		content.add(backgroundIconContainer);
		this.setContentPane(content);
		this.setSize(900, 340);
		::_ItemDataManager.addListener(this);
		this.mNameMap = {};
		this.requestItemDefs();
		this.center();
	}

	function onPreviewReset( button )
	{
		::_Connection.sendQuery("item.spoof", this, [
			"reset"
		]);
	}

	function onPreview( button )
	{
		local itemDef = this.getCurrentItemDef();

		if (itemDef == null)
		{
			return;
		}

		local slot = ::EquipmentMapContainer[itemDef.getEquipType()][0];
		::_Connection.sendQuery("item.spoof", this, [
			slot,
			itemDef.getID()
		]);

		if (slot == this.ItemEquipSlot.WEAPON_MAIN_HAND || slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
		{
			::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
		}
		else if (slot == this.ItemEquipSlot.WEAPON_RANGED)
		{
			::_avatar.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
		}
		else
		{
			::_avatar.setVisibleWeapon(this.VisibleWeaponSet.NONE, false);
		}
	}

	function requestItemDefs()
	{
		::_Connection.sendQuery("item.def.list", this, [
			this.mCurrentIndex,
			this.mMaxItemDefs
		]);
	}

	function onBackgroundIconBrowse( button )
	{
		local browser = ::Screens.IconBrowserScreen();
		browser.setIconSelectionListener(this);
		browser.setVisible(true);
		this.mSelectingBackground = true;
		this.mSelectingForeground = false;
	}

	function onForegroundIconBrowse( button )
	{
		local browser = ::Screens.IconBrowserScreen();
		browser.setIconSelectionListener(this);
		browser.setVisible(true);
		this.mSelectingForeground = true;
		this.mSelectingBackground = false;
	}

	function onIconSelected( icon )
	{
		if (this.mSelectingForeground == true)
		{
			this.mForegroundIconPreview.setImageName(icon);
			this.mForegroundIconEntry.setText(icon);
			this.mCombinedIconPreviewFG.setImageName(icon);
			this.mSelectingForeground = false;
		}
		else if (this.mSelectingBackground == true)
		{
			this.setBackgroundImage(icon);
			this.mSelectingBackground = false;
		}

		this.sendIcon();
	}

	function onInputComplete( box )
	{
		if (box == this.mNameFilter)
		{
			this.fillItemList();
		}
		else if (box == this.mAppearanceEntry)
		{
			this.sendAppearance();
		}
		else if (box == this.mForegroundIconEntry || box == this.mBackgroundIconEntry)
		{
			this.sendIcon();
		}
	}

	function sendAppearance()
	{
		local def = this.getCurrentItemDef();

		if (def == null)
		{
			return;
		}

		::_Connection.sendQuery("item.def.edit", this, [
			def.getID().tostring(),
			"appearance",
			this.serialize(def.getAppearance())
		]);
	}

	function sendIcon()
	{
		local def = this.getCurrentItemDef();

		if (def == null)
		{
			return;
		}

		local iconImage = this.mForegroundIconEntry.getText() + "|" + this.mBackgroundIconEntry.getText();
		::_Connection.sendQuery("item.def.edit", this, [
			def.getID().tostring(),
			"icon",
			iconImage
		]);
	}

	function onQueryComplete( q, results )
	{
		if (q.query == "item.def.list")
		{
			foreach( v in results )
			{
				local def = ::_ItemDataManager.getItemDef(v[0].tointeger());
				this.mNameMap[v[0].tointeger()] <- v[1];

				if (def.isValid())
				{
					this.onItemDefUpdated(def.getID(), def);
				}
			}

			if (results.len() >= this.mMaxItemDefs)
			{
				this.log.debug("Got item results for index: " + this.mCurrentIndex);
				this.mCurrentIndex += this.mMaxItemDefs;
				this.requestItemDefs();
			}
			else
			{
				this.mCurrentIndex = 0;
			}
		}
	}

	function onQueryTimeout( q )
	{
		if (q.query == "item.def.list")
		{
			this.requestItemDefs();
		}
	}

	function onQueryError( q, msg )
	{
		this.log.debug(msg);
	}

	function onFilterChanged( list )
	{
		this.fillItemList();
	}

	function onDeleteAppearance( button )
	{
		this.mAppearanceEntry.setText("");
		this.applyAppearance(null);
	}

	function applyAppearance( text )
	{
		local def = this.getCurrentItemDef();

		if (def == null)
		{
			return;
		}

		local selected = this.mAppearanceList.getSelectedRows();

		if (selected.len() == 0)
		{
			return;
		}

		local appearance = def.getAppearance();

		if (typeof appearance != "array")
		{
			appearance = [
				appearance,
				null
			];
		}

		appearance[selected[0]] = text != null ? ::unserialize(text) : null;
		def.setAppearance(appearance);
		this.sendAppearance();
	}

	function onCopyAppearance( button )
	{
		::System.setClipboard(this.mAppearanceEntry.getText());
	}

	function onPasteAppearance( button )
	{
		this.mAppearanceEntry.setText(::System.getClipboard());
		this.applyAppearance(::System.getClipboard());
	}

	function onRowSelectionChanged( list, index, which )
	{
		if (list == this.mAppearanceList)
		{
			this.onAppearanceListChanged(index, which);
			return;
		}

		local def = this.getCurrentItemDef();

		if (def != null)
		{
			local tmp;
			local appearance = def.getAppearance();

			if (appearance != null)
			{
				try
				{
					tmp = this.serialize(appearance);
				}
				catch( err )
				{
					this.log.debug(err);
					tmp = "(Invalid Appearance)";
				}
			}
			else
			{
				tmp = "(Invalid Appearance)";
			}

			local image = def.getIcon();
			local foregroundBackgroundImages = this.splitIconIntoForegroundBackground(image);
			this.setForegroundImage(foregroundBackgroundImages[0]);
			this.setBackgroundImage(foregroundBackgroundImages[1]);
			this.mAppearanceEntry.setText(tmp != null ? tmp : null);
			this.updateAppearanceList();
		}

		this.updateTooltip();
	}

	function updateAppearanceEntry()
	{
		local rows = this.mAppearanceList.getSelectedRows();

		if (rows.len() == 0)
		{
			this.mAppearanceEntry.setText("");
			return;
		}

		local row = this.mAppearanceList.getRow(rows[0]);
		this.mAppearanceEntry.setText(row[1]);
	}

	function onAppearanceListChanged( index, which )
	{
		this.updateAppearanceEntry();
	}

	function updateAppearanceList()
	{
		local def = this.getCurrentItemDef();

		if (def == null)
		{
			return;
		}

		local appearance = def.getAppearance();
		local names = [
			"Primary Attachment",
			"Secondary Attachment"
		];

		if (typeof appearance != "array")
		{
			appearance = [
				appearance,
				null
			];
		}

		local selected = this.mAppearanceList.getSelectedRows();
		this.mAppearanceList.removeAllRows();
		local x;

		for( x = 0; x < names.len(); x++ )
		{
			this.mAppearanceList.addRow([
				names[x],
				appearance.len() > x && appearance[x] != null ? this.serialize(appearance[x]) : ""
			]);
		}

		this.mAppearanceList.setSelectedRows([]);
		this.updateAppearanceEntry();
	}

	function splitIconIntoForegroundBackground( iconName )
	{
		local splitImages = this.Util.split(iconName, "|");

		if (splitImages.len() > 1 && splitImages[1] != "")
		{
		}
		else
		{
			splitImages.append(this.BackgroundImages.GREY);
		}

		return splitImages;
	}

	function setBackgroundImage( image )
	{
		local imageFilename = image;

		if (image.find(".png") == null)
		{
			imageFilename = this.BackgroundImages[image.toupper()];
		}

		this.mBackgroundIconPreview.setImageName(imageFilename);
		this.mCombinedIconPreviewBG.setImageName(imageFilename);
		this.mBackgroundIconEntry.setText(image);
	}

	function setForegroundImage( image )
	{
		this.mForegroundIconPreview.setImageName(image);
		this.mForegroundIconEntry.setText(image);
		this.mCombinedIconPreviewFG.setImageName(image);
	}

	function getCurrentItemDef()
	{
		local rows = this.mItemList.getSelectedRows();

		if (rows.len() == 0)
		{
			return null;
		}

		local row = this.mItemList.getRow(rows[0]);

		if (row == false)
		{
			return null;
		}

		return ::_ItemDataManager.getItemDef(row[0].tointeger());
	}

	function updateTooltip()
	{
		if (this.mTooltip)
		{
			this.mTooltipContainer.remove(this.mTooltip);
			this.mTooltipContainer.remove(this.mPreviewButton);
			this.mTooltipContainer.remove(this.mResetPreviewButton);
			this.mTooltipContainer.remove(this.mAddBtn);
		}

		local itemDef = this.getCurrentItemDef();

		if (itemDef == null)
		{
			return;
		}

		this.mTooltip = itemDef.getTooltip(false, true);
		this.mAddBtn = this.GUI.Button("Add to inventory", this, "onAddToInventory");
		this.mPreviewButton = this.GUI.Button("Preview", this, "onPreview");
		this.mResetPreviewButton = this.GUI.Button("Reset Preview", this, "onPreviewReset");
		this.mTooltipContainer.add(this.mAddBtn);
		this.mTooltipContainer.add(this.mPreviewButton);
		this.mTooltipContainer.add(this.mResetPreviewButton);
		this.mTooltipContainer.add(this.mTooltip);
	}

	function onAddToInventory( button )
	{
		local itemDef = this.getCurrentItemDef();
		::_Connection.sendQuery("item.create", null, [
			itemDef.mID
		]);
	}

	function updatePosition()
	{
		local width = this.getWidth() / 2;
		local height = this.getHeight() / 2;
		this.setPosition(::Screen.getWidth() / 2 - width, ::Screen.getHeight() * 0.85000002 - height);
	}

	function destroy()
	{
		::_ItemDataManager.removeListener(this);
		::GUI.Frame.destroy();
	}

	function onItemDefUpdated( itemDefId, itemDef )
	{
		this.setTitle("Item Appearance Tweak (Downloaded: " + this.getDefName(itemDef) + ")");
		local valid = this.applyFilter(itemDef);
		local x;

		for( x = 0; x < this.mItemList.getRowCount(); x++ )
		{
			local row = this.mItemList.getRow(x);

			if (row[0].tointeger() == itemDefId)
			{
				local selected = this.mItemList.isRowSelected(x);

				if (valid)
				{
					this.mItemList.insertRow(x, [
						itemDefId.tostring(),
						this.getDefName(itemDef)
					]);
					this.mItemList.removeRow(x + 1);
				}
				else
				{
					this.mItemList.removeRow(x);
				}

				if (selected)
				{
					this.mItemList.setSelectedRows([
						x
					]);
				}

				return;
			}
		}

		if (valid)
		{
			this.mItemList.addRow([
				itemDefId.tostring(),
				this.getDefName(itemDef)
			]);
		}
	}

	function getDefName( itemDef )
	{
		local id = itemDef.getID();

		if (id in this.mNameMap)
		{
			return this.mNameMap[id];
		}

		return itemDef.getDisplayName();
	}

	function applyFilter( itemDef )
	{
		local name = this.getDefName(itemDef);

		if (name.find("<<SYSTEM>>") != null)
		{
			return false;
		}

		local txt = this.Util.trim(this.mNameFilter.getText());

		if (txt != "")
		{
			if (name.tolower().find(txt.tolower()) == null)
			{
				return false;
			}
		}

		local index = this.mFilterList.getCurrentIndex();

		if (index >= 0)
		{
			local name = this.mFilterList.getCurrent();

			if (name == "No Appearance")
			{
				if (itemDef.getAppearance() != null)
				{
					return false;
				}
			}
			else if (name == "No Icon")
			{
				if (itemDef.getIcon().find("Icon/QuestionMark") == null)
				{
					return false;
				}
			}
			else if (name == "No Appearance or Icon")
			{
				if (itemDef.getAppearance() != null && itemDef.getIcon().find("Icon/QuestionMark") == null)
				{
					return false;
				}
			}
			else if (name == "Consumable")
			{
				if (itemDef.getType() != this.ItemType.CONSUMABLE)
				{
					return false;
				}
			}
			else if (name == "Weapons")
			{
				if (itemDef.getType() != this.ItemType.WEAPON)
				{
					return false;
				}
			}
			else if (name == "Armor")
			{
				if (itemDef.getType() != this.ItemType.ARMOR)
				{
					return false;
				}
			}
			else if (name == "Talisman")
			{
				if (itemDef.getType() != this.ItemType.WEAPON || itemDef.getWeaponType() != this.WeaponType.ARCANE_TOTEM)
				{
					return false;
				}
			}
			else if (name == "Special")
			{
				if (itemDef.getType() != this.ItemType.CHARM)
				{
					return false;
				}
			}
			else if (name == "Quest")
			{
				if (itemDef.getType() != this.ItemType.QUEST)
				{
					return false;
				}
			}
			else if (name == "Basic")
			{
				if (itemDef.getType() != this.ItemType.BASIC)
				{
					return false;
				}
			}
			else if (name == "Charm")
			{
				if (itemDef.getType() != this.ItemType.CHARM)
				{
					return false;
				}
			}
		}

		return true;
	}

	function fillItemList()
	{
		this.mItemList.removeAllRows();
		this.setForegroundImage("Icon/QuestionMark");
		this.setBackgroundImage(this.BackgroundImages.GREY);
		local cache = ::_ItemDataManager.getItemDefCache();

		foreach( k, v in cache )
		{
			if (v.isValid() && this.applyFilter(v))
			{
				this.mItemList.addRow([
					k.tostring(),
					this.getDefName(v)
				]);
			}
		}
	}

}

