require("UI/UI");
require("UI/Screens");
/**	
	A UI that displays the results of a /who command.
	Every few seconds that the UI remains open, it will
	requery the server and update the results.
	
	The user can also double click a cell to instantly
	jump to the location of the character selected.
	@author Ryne Anderson
*/

class Screens.ItemMarketEditScreen extends GUI.Frame {
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
	
	constructor() {
		GUI.Frame.constructor("Item Market Editor");
		
		local cmain = GUI.Container(GUI.GridLayout(1, 2));
		cmain.getLayoutManager().setColumns(250, "*");
		cmain.setInsets(5);
		
		setContentPane(cmain);
		setSize(600, 400);
		setPosition(50, 50);
		
		mOutputList = GUI.ColumnList();
		mOutputList.setForcedRowHeight(24);
		mOutputList.addColumn("ID", 25);
		mOutputList.addColumn("Title", 100);
		mOutputList.addColumn("Category", 50);
		mOutputList.addActionListener(this);
		
		mTitleEntry = GUI.InputArea();
		mTitleEntry.addActionListener(this);
		
		mDescEntry = GUI.InputArea();
		mDescEntry.addActionListener(this);
		
		mCategoryEntry = GUI.DropDownList();
		mCategoryEntry.addChoice("CONSUMABLES");
		mCategoryEntry.addChoice("CHARMS");
		mCategoryEntry.addChoice("ARMOR");
		mCategoryEntry.addChoice("BAGS");
		mCategoryEntry.addChoice("RECIPES");
		mCategoryEntry.addSelectionChangeListener(this);
		
		mStatusEntry = GUI.DropDownList();
		mStatusEntry.addChoice("HIDDEN");
		mStatusEntry.addChoice("NEW");
		mStatusEntry.addChoice("ACTIVE");
		mStatusEntry.addChoice("EXPIRED");
		mStatusEntry.addChoice("CANCELED");
		mStatusEntry.addChoice("HOT");
		mStatusEntry.addSelectionChangeListener(this);
		
		mBeginDateEntry = GUI.InputArea();
		mBeginDateEntry.addActionListener(this);
		
		mEndDateEntry = GUI.InputArea();
		mEndDateEntry.addActionListener(this);
		
		mPriceAmountEntry = GUI.InputArea();
		mPriceAmountEntry.addActionListener(this);
		
		mPriceCurrencyEntry = GUI.DropDownList();
		mPriceCurrencyEntry.addChoice("COPPER");
		mPriceCurrencyEntry.addChoice("CREDITS");
		mPriceCurrencyEntry.addChoice("COPPER+CREDITS");
		mPriceCurrencyEntry.addSelectionChangeListener(this);
		
		mQuantityLimitEntry = GUI.InputArea();
		mQuantityLimitEntry.addActionListener(this);
		
		mQuantitySoldEntry = GUI.InputArea();
		mQuantitySoldEntry.addActionListener(this);
		
		mItemProtoEntry = GUI.InputArea();
		mItemProtoEntry.addActionListener(this);
		
		local cont = GUI.Container(GUI.GridLayout(2, 1));
		cont.getLayoutManager().setRows("*", 25);
		
		local commands = GUI.Container(GUI.GridLayout(1, 4));
		commands.getLayoutManager().setRows(25);
		mSaveButton = GUI.Button("Save");
		mSaveButton.setReleaseMessage("onSavePressed");
		mSaveButton.addActionListener(this);
		mSaveButton.setEnabled(false);
		commands.add(mSaveButton);
		
		local button = GUI.Button("Refresh");
		button.setReleaseMessage("onRefreshPressed");
		button.addActionListener(this);
		commands.add(button);
		
		button = GUI.Button("New");
		button.setReleaseMessage("onNewPressed");
		button.addActionListener(this);
		commands.add(button);
		
		mDeleteButton = GUI.Button("Delete");
		mDeleteButton.setReleaseMessage("onDeletePressed");
		mDeleteButton.addActionListener(this);
		commands.add(mDeleteButton);
		
		local fields = GUI.Container(GUI.GridLayout(11, 2));
		fields.getLayoutManager().setColumns(128, "*");
		fields.add(GUI.Label("Title"));
		fields.add(mTitleEntry);
		fields.add(GUI.Label("Description"));
		fields.add(mDescEntry);
		fields.add(GUI.Label("Category"));
		fields.add(mCategoryEntry);
		fields.add(GUI.Label("Status"));
		fields.add(mStatusEntry);
		fields.add(GUI.Label("Begin Date"));
		fields.add(mBeginDateEntry);
		fields.add(GUI.Label("End Date"));
		fields.add(mEndDateEntry);
		fields.add(GUI.Label("Price Amount"));
		fields.add(mPriceAmountEntry);
		fields.add(GUI.Label("Price Currency"));
		fields.add(mPriceCurrencyEntry);
		fields.add(GUI.Label("Quantity Limit"));
		fields.add(mQuantityLimitEntry);
		fields.add(GUI.Label("Quantity Sold"));
		fields.add(mQuantitySoldEntry);
		
		mItemSlot = GUI.ActionContainer("market_edit", 1, 1, 0, 0, this, false);
		mItemSlot.addAcceptingFromProperties("inventory", AcceptFromProperties());
		mItemSlot.addListener(this);
		
		local tmpCont = GUI.Container(GUI.GridLayout(1, 2));
		tmpCont.getLayoutManager().setColumns("*", 25);
		tmpCont.add(mItemProtoEntry);
		tmpCont.add(mItemSlot);
		
		fields.add(GUI.Label("Item Proto"));
		fields.add(tmpCont);
		
		cont.add(fields);
		cont.add(commands);
		
		cmain.add(GUI.ScrollPanel(mOutputList));
		cmain.add(cont);
		
		reset();
		fillList();
	}

	function onRefreshPressed( button ) {
		fillList();
	}

	function onSavePressed( button ) {
		save();
	}

	function onNewPressed( button )	{
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
		fillList();
	}

	function onTextChanged( text ) {
		mSaveButton.setEnabled(true);
	}

	function onSelectionChange( list ) {
		mSaveButton.setEnabled(true);
	}

	function onDeletePressed( button ) {
		local entry = getSelected();

		if (entry == null)
			return;

		::_Connection.sendQuery("item.market.edit", this, [
			"DELETE",
			entry.id
		]);
	}

	function reset() {
		mTitleEntry.setText("");
		mDescEntry.setText("");
		mCategoryEntry.setCurrent("CONSUMABLE");
		mStatusEntry.setCurrent("NEW");
		mBeginDateEntry.setText("");
		mEndDateEntry.setText("");
		mPriceAmountEntry.setText("");
		mPriceCurrencyEntry.setCurrent("CREDITS");
		mQuantityLimitEntry.setText("");
		mQuantitySoldEntry.setText("");
		mItemProtoEntry.setText("");
		mItemSlot.removeAllActions();
		mDeleteButton.setEnabled(false);
		mSaveButton.setEnabled(false);
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )	{
		local item = container.getSlotContents(slotIndex);
		local itemID = item.getActionButton().getAction();
		local lookId = itemID.mLookDefData.mID;
		local defId = itemID.mItemDefData.mID;
		mItemProtoEntry.setText("item" + defId + ":" + (lookId != defId ? lookId : 0) + ":" + itemID.mItemData.mIv1 + ":" + itemID.mItemData.mIv2);
		mSaveButton.setEnabled(true);
	}

	function onRowSelectionChanged( list, row, selected ) {
		local entry = getSelected();

		if (entry == null) {
			reset();
			return;
		}

		mTitleEntry.setText(entry.title);
		mDescEntry.setText(entry.description);
		mCategoryEntry.setCurrent(entry.category);
		mStatusEntry.setCurrent(entry.status);
		mBeginDateEntry.setText(entry.beginDate);
		mEndDateEntry.setText(entry.endDate);
		mPriceAmountEntry.setText(entry.priceAmount);
		mPriceCurrencyEntry.setCurrent(entry.priceCurrency);
		mQuantityLimitEntry.setText(entry.quantityLimit);
		mQuantitySoldEntry.setText(entry.quantitySold);
		mItemProtoEntry.setText(entry.itemProto);
		mSaveButton.setEnabled(false);
		mDeleteButton.setEnabled(true);
	}

	function getSelected() {
		local rows = mOutputList.getSelectedRows();

		if (rows.len() == 0)
			return null;

		return mEntries[rows[0].tointeger()];
	}

	function save()	{
		local selected = getSelected();

		if (selected == null)
			return;

		::_Connection.sendQuery("item.market.edit", this, [
			selected.id,
			"title",
			mTitleEntry.getText(),
			"description",
			mDescEntry.getText(),
			"category",
			mCategoryEntry.getCurrent(),
			"status",
			mStatusEntry.getCurrent(),
			"beginDate",
			mBeginDateEntry.getText(),
			"endDate",
			mEndDateEntry.getText(),
			"priceAmount",
			mPriceAmountEntry.getText(),
			"priceCurrency",
			mPriceCurrencyEntry.getCurrent(),
			"quantityLimit",
			mQuantityLimitEntry.getText(),
			"quantitySold",
			mQuantitySoldEntry.getText(),
			"itemProto",
			mItemProtoEntry.getText()
		]);
	}

	function fillList()	{
		::_Connection.sendQuery("item.market.list", this, []);
	}

	function onQueryError( qa, err ) {
		::IGIS.error(err);
	}

	function onQueryComplete( qa, results )	{
		if (qa.query == "item.market.edit")	{
			reset();
			fillList();
		}
		else if (qa.query == "item.market.list") {
			local selected = mOutputList.getSelectedRows();
			mOutputList.removeAllRows();
			mEntries = [];

			foreach( r in results ) {
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
				mEntries.append(e);
				mOutputList.addRow([
					e.id,
					e.title,
					e.category
				]);
			}

			mOutputList.setSelectedRows(selected);
		}
	}

	function onQueryTimeout( qa ) {
		::IGIS.error("Timeout");
	}

	function isVisible() {
		return GUI.Frame.isVisible();
	}

	function setVisible( value ) {
		GUI.Frame.setVisible(value);
	}

	function destroy() {
		return GUI.Frame.destroy();
	}

}

