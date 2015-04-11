this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.ItemMarketEditScreen extends this.GUI.Frame
{
	mOutputList = null;
	mUpdateEvent = null;
	mSelectedPlyId = null;
	mEntries = null;
	mStatusEntry = null;
	mTitleEntry = null;
	mDescEntry = null;
	mCategoryEntry = null;
	mBeginDateEntry = null;
	mEndDateEntry = null;
	mPriceAmountEntry = null;
	mPriceCurrencyEntry = null;
	mQuantityLimitEntry = null;
	mQuantitySoldEntry = null;
	mItemProtoEntry = null;
	mItemSlot = null;
	mDeleteButton = null;
	mSaveButton = null;
	constructor()
	{
		this.GUI.Frame.constructor("Item Market Editor");
		local cmain = this.GUI.Container(this.GUI.GridLayout(1, 2));
		cmain.getLayoutManager().setColumns(250, "*");
		cmain.setInsets(5);
		this.setContentPane(cmain);
		this.setSize(600, 400);
		this.setPosition(50, 50);
		this.mOutputList = this.GUI.ColumnList();
		this.mOutputList.addColumn("ID", 25);
		this.mOutputList.addColumn("Title", 100);
		this.mOutputList.addColumn("Category", 50);
		this.mOutputList.addActionListener(this);
		this.mTitleEntry = this.GUI.InputArea();
		this.mTitleEntry.addActionListener(this);
		this.mDescEntry = this.GUI.InputArea();
		this.mDescEntry.addActionListener(this);
		this.mCategoryEntry = this.GUI.DropDownList();
		this.mCategoryEntry.addChoice("CONSUMABLES");
		this.mCategoryEntry.addChoice("CHARMS");
		this.mCategoryEntry.addChoice("ARMOR");
		this.mCategoryEntry.addChoice("BAGS");
		this.mCategoryEntry.addChoice("RECIPES");
		this.mCategoryEntry.addSelectionChangeListener(this);
		this.mStatusEntry = this.GUI.DropDownList();
		this.mStatusEntry.addChoice("HIDDEN");
		this.mStatusEntry.addChoice("NEW");
		this.mStatusEntry.addChoice("ACTIVE");
		this.mStatusEntry.addChoice("EXPIRED");
		this.mStatusEntry.addChoice("CANCELED");
		this.mStatusEntry.addChoice("HOT");
		this.mStatusEntry.addSelectionChangeListener(this);
		this.mBeginDateEntry = this.GUI.InputArea();
		this.mBeginDateEntry.addActionListener(this);
		this.mEndDateEntry = this.GUI.InputArea();
		this.mEndDateEntry.addActionListener(this);
		this.mPriceAmountEntry = this.GUI.InputArea();
		this.mPriceAmountEntry.addActionListener(this);
		this.mPriceCurrencyEntry = this.GUI.DropDownList();
		this.mPriceCurrencyEntry.addChoice("CREDITS");
		this.mPriceCurrencyEntry.addChoice("COPPER");
		this.mPriceCurrencyEntry.addSelectionChangeListener(this);
		this.mQuantityLimitEntry = this.GUI.InputArea();
		this.mQuantityLimitEntry.addActionListener(this);
		this.mQuantitySoldEntry = this.GUI.InputArea();
		this.mQuantitySoldEntry.addActionListener(this);
		this.mItemProtoEntry = this.GUI.InputArea();
		this.mItemProtoEntry.addActionListener(this);
		local cont = this.GUI.Container(this.GUI.GridLayout(2, 1));
		cont.getLayoutManager().setRows("*", 25);
		local commands = this.GUI.Container(this.GUI.GridLayout(1, 4));
		commands.getLayoutManager().setRows(25);
		this.mSaveButton = this.GUI.Button("Save");
		this.mSaveButton.setReleaseMessage("onSavePressed");
		this.mSaveButton.addActionListener(this);
		this.mSaveButton.setEnabled(false);
		commands.add(this.mSaveButton);
		local button = this.GUI.Button("Refresh");
		button.setReleaseMessage("onRefreshPressed");
		button.addActionListener(this);
		commands.add(button);
		button = this.GUI.Button("New");
		button.setReleaseMessage("onNewPressed");
		button.addActionListener(this);
		commands.add(button);
		this.mDeleteButton = this.GUI.Button("Delete");
		this.mDeleteButton.setReleaseMessage("onDeletePressed");
		this.mDeleteButton.addActionListener(this);
		commands.add(this.mDeleteButton);
		local fields = this.GUI.Container(this.GUI.GridLayout(11, 2));
		fields.getLayoutManager().setColumns(128, "*");
		fields.add(this.GUI.Label("Title"));
		fields.add(this.mTitleEntry);
		fields.add(this.GUI.Label("Description"));
		fields.add(this.mDescEntry);
		fields.add(this.GUI.Label("Category"));
		fields.add(this.mCategoryEntry);
		fields.add(this.GUI.Label("Status"));
		fields.add(this.mStatusEntry);
		fields.add(this.GUI.Label("Begin Date"));
		fields.add(this.mBeginDateEntry);
		fields.add(this.GUI.Label("End Date"));
		fields.add(this.mEndDateEntry);
		fields.add(this.GUI.Label("Price Amount"));
		fields.add(this.mPriceAmountEntry);
		fields.add(this.GUI.Label("Price Currency"));
		fields.add(this.mPriceCurrencyEntry);
		fields.add(this.GUI.Label("Quantity Limit"));
		fields.add(this.mQuantityLimitEntry);
		fields.add(this.GUI.Label("Quantity Sold"));
		fields.add(this.mQuantitySoldEntry);
		this.mItemSlot = this.GUI.ActionContainer("market_edit", 1, 1, 0, 0, this, false);
		this.mItemSlot.addAcceptingFromProperties("inventory", this.AcceptFromProperties());
		this.mItemSlot.addListener(this);
		local tmpCont = this.GUI.Container(this.GUI.GridLayout(1, 2));
		tmpCont.getLayoutManager().setColumns("*", 25);
		tmpCont.add(this.mItemProtoEntry);
		tmpCont.add(this.mItemSlot);
		fields.add(this.GUI.Label("Item Proto"));
		fields.add(tmpCont);
		cont.add(fields);
		cont.add(commands);
		cmain.add(this.GUI.ScrollPanel(this.mOutputList));
		cmain.add(cont);
		this.reset();
		this.fillList();
	}

	function onRefreshPressed( button )
	{
		this.fillList();
	}

	function onSavePressed( button )
	{
		this.save();
	}

	function onNewPressed( button )
	{
		::_Connection.sendQuery("item.market.edit", this, [
			"NEW",
			"title",
			"New Offer",
			"description",
			"",
			"category",
			"ARMOR",
			"status",
			"NEW",
			"beginDate",
			"",
			"endDate",
			"",
			"priceAmount",
			"0",
			"priceCurrency",
			"CREDITS",
			"quantityLimit",
			"0",
			"quantitySold",
			"0",
			"itemProto",
			""
		]);
		this.fillList();
	}

	function onTextChanged( text )
	{
		this.mSaveButton.setEnabled(true);
	}

	function onSelectionChange( list )
	{
		this.mSaveButton.setEnabled(true);
	}

	function onDeletePressed( button )
	{
		local entry = this.getSelected();

		if (entry == null)
		{
			return;
		}

		::_Connection.sendQuery("item.market.edit", this, [
			"DELETE",
			entry.id
		]);
	}

	function reset()
	{
		this.mTitleEntry.setText("");
		this.mDescEntry.setText("");
		this.mCategoryEntry.setCurrent("CONSUMABLE");
		this.mStatusEntry.setCurrent("NEW");
		this.mBeginDateEntry.setText("");
		this.mEndDateEntry.setText("");
		this.mPriceAmountEntry.setText("");
		this.mPriceCurrencyEntry.setCurrent("CREDITS");
		this.mQuantityLimitEntry.setText("");
		this.mQuantitySoldEntry.setText("");
		this.mItemProtoEntry.setText("");
		this.mItemSlot.removeAllActions();
		this.mDeleteButton.setEnabled(false);
		this.mSaveButton.setEnabled(false);
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )
	{
		local item = container.getSlotContents(slotIndex);
		local itemID = item.getActionButton().getAction();
		local lookId = itemID.mLookDefData.mID;
		local defId = itemID.mItemDefData.mID;
		this.mItemProtoEntry.setText("item" + defId + ":" + (lookId != defId ? lookId : 0) + ":" + itemID.mItemData.mIv1 + ":" + itemID.mItemData.mIv2);
		this.mSaveButton.setEnabled(true);
	}

	function onRowSelectionChanged( list, row, selected )
	{
		local entry = this.getSelected();

		if (entry == null)
		{
			this.reset();
			return;
		}

		this.mTitleEntry.setText(entry.title);
		this.mDescEntry.setText(entry.description);
		this.mCategoryEntry.setCurrent(entry.category);
		this.mStatusEntry.setCurrent(entry.status);
		this.mBeginDateEntry.setText(entry.beginDate);
		this.mEndDateEntry.setText(entry.endDate);
		this.mPriceAmountEntry.setText(entry.priceAmount);
		this.mPriceCurrencyEntry.setCurrent(entry.priceCurrency);
		this.mQuantityLimitEntry.setText(entry.quantityLimit);
		this.mQuantitySoldEntry.setText(entry.quantitySold);
		this.mItemProtoEntry.setText(entry.itemProto);
		this.mSaveButton.setEnabled(false);
		this.mDeleteButton.setEnabled(true);
	}

	function getSelected()
	{
		local rows = this.mOutputList.getSelectedRows();

		if (rows.len() == 0)
		{
			return null;
		}

		return this.mEntries[rows[0].tointeger()];
	}

	function save()
	{
		local selected = this.getSelected();

		if (selected == null)
		{
			return;
		}

		::_Connection.sendQuery("item.market.edit", this, [
			selected.id,
			"title",
			this.mTitleEntry.getText(),
			"description",
			this.mDescEntry.getText(),
			"category",
			this.mCategoryEntry.getCurrent(),
			"status",
			this.mStatusEntry.getCurrent(),
			"beginDate",
			this.mBeginDateEntry.getText(),
			"endDate",
			this.mEndDateEntry.getText(),
			"priceAmount",
			this.mPriceAmountEntry.getText(),
			"priceCurrency",
			this.mPriceCurrencyEntry.getCurrent(),
			"quantityLimit",
			this.mQuantityLimitEntry.getText(),
			"quantitySold",
			this.mQuantitySoldEntry.getText(),
			"itemProto",
			this.mItemProtoEntry.getText()
		]);
	}

	function fillList()
	{
		::_Connection.sendQuery("item.market.list", this, []);
	}

	function onQueryError( qa, err )
	{
		::IGIS.error(err);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "item.market.edit")
		{
			this.reset();
			this.fillList();
		}
		else if (qa.query == "item.market.list")
		{
			local selected = this.mOutputList.getSelectedRows();
			this.mOutputList.removeAllRows();
			this.mEntries = [];

			foreach( r in results )
			{
				local e = {
					id = r[0],
					title = r[1],
					description = r[2],
					status = r[3],
					category = r[4],
					beginDate = r[5],
					endDate = r[6],
					priceAmount = r[7],
					priceCurrency = r[8],
					quantityLimit = r[9],
					quantitySold = r[10],
					itemProto = r[11]
				};
				this.mEntries.append(e);
				this.mOutputList.addRow([
					e.id,
					e.title,
					e.category
				]);
			}

			this.mOutputList.setSelectedRows(selected);
		}
	}

	function onQueryTimeout( qa )
	{
		::IGIS.error("Timeout");
	}

	function isVisible()
	{
		return this.GUI.Frame.isVisible();
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);
	}

	function destroy()
	{
		return this.GUI.Frame.destroy();
	}

}

