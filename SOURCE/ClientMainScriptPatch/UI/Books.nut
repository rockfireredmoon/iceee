this.require("UI/Screens");
this.require("UI/ActionContainer");

class Book {
	mTitle = null;
	mID = 0;
	mTotalPages = 0;
	mPages = [];
	
	constructor() {
	}
}

class BookObject extends this.GUI.Component {
	mBook = null;
	mTitleLabel = null;
	mInfoLabel = null;

	constructor( book )	{
		GUI.Component.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setExpand(true);
		setInsets(0, 5, -1, 5);
		mBook = book;
		
		mTitleLabel = GUI.HTML("");
		mTitleLabel.setInsets(2, 0, -5, 0);
		
		add(mTitleLabel);

		mInfoLabel = GUI.HTML("(No Information)");
		mInfoLabel.setInsets(0, 0, 2, 0);
		
		add(mInfoLabel);
		
		_buildDisplayString();
	}

	function _addNotify() {
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify() {
		if (mWidget != null) {
			mWidget.removeListener(this);
		}
		GUI.Component._removeNotify();
	}

	function _buildDisplayString() {
		mTitleLabel.setText("<b>" + mBook.mTitle + "</b>");
		mInfoLabel.setText("<i>" + mBook.mTotalPages + " pages</i>");
	}

}

class Screens.Books extends GUI.Frame {

	/* Screen class name */
	static mClassName = "Screens.Books";
	

	mScreenContainer = null;
	mButtonNext = null;
	mButtonPrevious = null;
	mBookText = null;
	mPages = [];
	mBookList = null;
	mBooks = [];
	mSelectedPage = 1;
	mSelectedBook = null;
	mBookLabel = null;
	mPageLabel = null;
	mSelectOnLoad = null;
	mSelectPageOnLoad = null;
	mLoaded = false;
	mAdjusting = false;
	mInited = false;
	mTabPane = null; 
	mBookShelf = null;
	mBookInventory = null;
	mBindingContainer = null;
	mButtonBind = null;	
	mBindingAgentsContainer = null;
	mCompleteBookContainer = null;
	
	constructor() {
		GUI.Frame.constructor("Books");
		
		local font = ::GUI.Font("MaiandraOutline", 24);

		mButtonNext = GUI.NarrowButton("Next");
		mButtonNext.addActionListener(this);
		mButtonNext.setReleaseMessage("onButtonPressed");

		mButtonPrevious = GUI.NarrowButton("Previous");
		mButtonPrevious.addActionListener(this);
		mButtonPrevious.setReleaseMessage("onButtonPressed");
		
		mPageLabel = GUI.Label("Page: 1");
		mPageLabel.setFont(font);
		mPageLabel.setAutoFit(true);

		local buttonRow = GUI.Container(GUI.BoxLayout());
		buttonRow.getLayoutManager().setPackAlignment(0.5);
		buttonRow.add(mButtonPrevious);
		buttonRow.add(mButtonNext);
		buttonRow.add(mPageLabel);
		buttonRow.setInsets(6, 0, 6, 0);
		
		mBookText = ::GUI.HTML();
		mBookText.setAppearance("PaperBackBorder");
		mBookText.setFont( GUI.Font( "Maiandra", 16 ) );
		mBookText.setFontColor("2b1b00");
		mBookText.setLinkStaticColor("004183");
		mBookText.setLinkHoverColor("007b83");
		mBookText.setChangeColorOnHover(true);
		mBookText.addActionListener(this);
		mBookText.setVisible(true);
		mBookText.setLayoutManager(::GUI.FlowLayout());
		mBookText.getLayoutManager().setAlignment("left");
		mBookText.getLayoutManager().setGaps(0.0, 0.0);
		mBookText.setInsets(10, 10, 10, 10);

		local scrollArea = ::GUI.ScrollPanel();
		scrollArea.setPreferredSize(310, 398);	
		scrollArea.attach(mBookText);
		
		mBookList = GUI.ColumnList();
		mBookList.setPreferredSize(180, 338);
		mBookList.addColumn("Name", 100);
		mBookList.setShowingHeaders(false);
		mBookList.setAppearance("DarkBorder");
		mBookList.setRowAppearance("ColumnSelection");
		mBookList.setSelectionInsets([
			3,
			3
		]);	
		mBookList.addActionListener(this);	
		
		mBookLabel = GUI.Label("");
		mBookLabel.setFont(font);
		mBookLabel.setAutoFit(true);
		
		local readerBox = GUI.Container(GUI.GridLayout(3, 1));
		readerBox.getLayoutManager().setColumns("*");
		readerBox.getLayoutManager().setRows(24, "*", 34);
		readerBox.setInsets(4);
		readerBox.add(mBookLabel);
		readerBox.add(scrollArea);
		readerBox.add(buttonRow);

		local listScrollArea = ::GUI.ScrollPanel();
		listScrollArea.setPreferredSize(180, 338);	
		listScrollArea.attach(mBookList);
		
		mScreenContainer = GUI.Container(GUI.GridLayout(1, 2));
		mScreenContainer.setInsets(5);
		mScreenContainer.getLayoutManager().setColumns(180, "*");
		mScreenContainer.getLayoutManager().setRows("*");
		mScreenContainer.add(listScrollArea);
		mScreenContainer.add(readerBox);
		
		// Binding
		mBindingAgentsContainer = GUI.ActionContainer("book_binding_mats", 4, 1, 0, 0, this, true);
		mBindingAgentsContainer.setItemPanelVisible(false);
		mBindingAgentsContainer.setValidDropContainer(true);
		mBindingAgentsContainer.setAllowButtonDisownership(false);
		mBindingAgentsContainer.addListener(this);
		mBindingAgentsContainer.addAcceptingFromProperties("inventory", AcceptFromProperties(this));	
		mBindingAgentsContainer.addAcceptingFromProperties("vault", AcceptFromProperties(this));
		
		mCompleteBookContainer = GUI.ActionContainer("book", 1, 1, 0, 0, this, true);
		mCompleteBookContainer.setItemPanelVisible(true);
		mCompleteBookContainer.setValidDropContainer(false);
		mCompleteBookContainer.setAllowButtonDisownership(false);
		mCompleteBookContainer.addListener(this);
		
		mBindingContainer = GUI.ActionContainer("book_binding", 4, 5, 0, 0, this, true);
		mBindingContainer.setItemPanelVisible(false);
		mBindingContainer.setValidDropContainer(true);
		mBindingContainer.setAllowButtonDisownership(false);
		mBindingContainer.addListener(this);
		mBindingContainer.addAcceptingFromProperties("inventory", AcceptFromProperties(this));	
		mBindingContainer.addAcceptingFromProperties("vault", AcceptFromProperties(this));

		mButtonBind = GUI.NarrowButton("Bind");
		mButtonBind.addActionListener(this);
		mButtonBind.setReleaseMessage("onButtonPressed");
		
		local addMarker = this.GUI.Component(null);
		addMarker.setSize(44, 44);
		addMarker.setPreferredSize(44, 44);
		addMarker.setAppearance("Crafting/AddMarker");
		
		local resultMarker = this.GUI.Component(null);
		resultMarker.setSize(33, 44);
		resultMarker.setPreferredSize(33, 44);
		resultMarker.setAppearance("Crafting/ResultMarkerVertical");
		
		local mTopBindContainer = GUI.Container(GUI.BoxLayout());
		mTopBindContainer.add(mBindingAgentsContainer);
		mTopBindContainer.add(addMarker);
		mTopBindContainer.add(mBindingContainer);

		local text = "Drag book pages and binding materials into the slots, then click <font color=\"00FF00\">Bind</font>.<br>" +
		             "The pages will be combined into a book, and placed in your <i>Bookshelf</i>.";
		local label = GUI.HTML();
		label.setText(text);

		local mBindContainer = GUI.Container(GUI.BoxLayoutV());
		mBindContainer.add(label);
		mBindContainer.add(GUI.Spacer(0, 10));
		mBindContainer.add(mTopBindContainer);
		mBindContainer.add(GUI.Spacer(0, 10));
		mBindContainer.add(resultMarker);
		mBindContainer.add(GUI.Spacer(0, 10));
		mBindContainer.add(mCompleteBookContainer);
		mBindContainer.add(GUI.Spacer(0, 10));
		mBindContainer.add(mButtonBind);
		
		// Bookshelf
		mBookShelf = this.GUI.Container(this.GUI.BoxLayoutV());
		mBookInventory = this.GUI.ActionContainer("bookshelf", 12, 16, 0, 0, this, false);
		mBookInventory.addListener(this);
		//mBookInventory.setCallback(this);
		mBookInventory.setValidDropContainer(true);
		mBookInventory.setShowEquipmentComparison(false);
		mBookInventory.setAllowButtonDisownership(false);
		mBookInventory.addAcceptingFromProperties("inventory", AcceptFromProperties(this));		
		mBookInventory.addAcceptingFromProperties("vault", AcceptFromProperties(this));		
		mBookInventory.addMovingToProperties("inventory", MoveToProperties(MovementTypes.MOVE));		
		mBookInventory.addMovingToProperties("vault", MoveToProperties(MovementTypes.MOVE));
		
		mBookShelf.add(mBookInventory);
		
		
		// This
		mTabPane = this.GUI.TabbedPane();
		mTabPane.setTabPlacement("top");
		mTabPane.setTabFontColor("E4E4E4");
		mTabPane.addTab("Reading Room", mScreenContainer);
		mTabPane.addTab("Book Binding", mBindContainer);
		mTabPane.addTab("Bookshelf", mBookShelf);
		mTabPane.addActionListener(this);
		mTabPane.setInsets(3);
		
		setContentPane(mTabPane);
		setSize(520, 440);
		
		::_ItemDataManager.addListener(this);

	}
	
	function getBookshelfContainer()
		return this.mBookshelfInventory;
	
	function getBookBindingContainer()
		return this.mBindingContainer;
	
	function getBookBindingMatsContainer()
		return this.mBindingAgentsContainer;
	
	
	function onValidDropSlot( newSlot, oldSlot )
	{
		local button = oldSlot.getActionButton();
		local action = button.getAction();
		local itemData = action.mItemData;
		
		if(newSlot.getActionContainer().getContainerName() == "book_binding_mats") 
		{
			if (itemData && itemData.getType() == this.ItemType.SPECIAL)
			{
				local itemDef = ::_ItemDataManager.getItemDef(itemData.mItemDefId); 
				if(itemDef && itemDef.getDynamicMax(ItemIntegerType.BOOK_MATERIAL) != null)
					return true;
			}
			this.IGIS.error("You can only place book binding materials in here.");
		}
		else if(newSlot.getActionContainer().getContainerName() == "book_binding") 
		{
			if (itemData && itemData.getType() == this.ItemType.SPECIAL)
			{
				local itemDef = ::_ItemDataManager.getItemDef(itemData.mItemDefId); 
				if(itemDef && itemDef.getDynamicMax(ItemIntegerType.BOOK_PAGE) != null)
					return true;
			}
			this.IGIS.error("You can only place book pages in here.");
		}
		else if(newSlot.getActionContainer().getContainerName() == "bookshelf") 
		{
			if (itemData && itemData.getType() == this.ItemType.SPECIAL)
			{
				local itemDef = ::_ItemDataManager.getItemDef(itemData.mItemDefId); 
				if(itemDef && itemDef.getDynamicMax(ItemIntegerType.BOOK) != null )
					return true;
			}
			this.IGIS.error("You can only place book pages or complete books in here.");
		}
		else 
		{
			if (itemData && itemData.getType() == this.ItemType.SPECIAL)
			{
				local itemDef = ::_ItemDataManager.getItemDef(itemData.mItemDefId); 
				if(itemDef && itemDef.getDynamicMax(ItemIntegerType.BOOK_PAGE) != null)
					return true;
			}
	
			this.IGIS.error("You can only place book pages in here.");
		}
		
		return false;
	}

	function onActionButtonDropped( actionContainer, actionButton )
	{
		print("ICE! onActionButtonDropped( " + actionContainer + "," +  actionButton + "\n");
		local equipScreen = this.Screens.get("Equipment", false);
		if (equipScreen)
		{
			equipScreen.handleClearHighlight();
		}
		
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )
	{
		print("ICE! onItemMovedInContainer( " + container + "," +  slotIndex + "," +  oldSlotsButton + "\n");
		
		local item = container.getSlotContents(slotIndex);
		local itemID = item.getActionButton().getAction().mItemId;
		local queryArgument = [];
		local previousSlotContainer = oldSlotsButton.getPreviousActionContainer();
		local oldActionButtonSlot = oldSlotsButton.getActionButtonSlot();
		local oldSlotContainerName = "";
		local oldSlotIndex = 0;

		if (previousSlotContainer && oldActionButtonSlot)
		{
			oldSlotContainerName = previousSlotContainer.getContainerName();
			oldSlotIndex = previousSlotContainer.getIndexOfSlot(oldActionButtonSlot);
		}

		if (container == mBookInventory)
		{
			if (item.getSwapped() == true)
			{
				item.setSwapped(false);
			}
			else
			{
				queryArgument.append(itemID);
				queryArgument.append("bookshelf");
				queryArgument.append(slotIndex);

				if (::_Connection.getProtocolVersionId() >= 19)
				{
					queryArgument.append(oldSlotContainerName);
					queryArgument.append(oldSlotIndex);
				}

				this._Connection.sendQuery("item.move", this, queryArgument);
			}
			this.onActionButtonDropped(null, null);
		}
		
		if(container == mBindingContainer)
		{
			print("ICE! onActionButtonDropped mBindingContainer\n");
			if(mCompleteBookContainer.isContainerEmpty()) 
			{
				/* If empty, query which book item this book page item produces */
				print("ICE! onActionButtonDropped book.item " + itemID + "\n");
				this._Connection.sendQuery("book.item", this, [itemID]);
			}
		}
	}
	
	function onLinkClicked( message, data )	{
		if("href" in data) {
			this.System.openURL(data.href);
		}
	}
	
	function onRowSelectionChanged(sender, index, add) {
		if(!mAdjusting) {
			mSelectedPage = 0;
			selectionChanged();
		}
	}
	
	function onRowSelect(evt) {		
		mSelectedPage = 0;
		selectionChanged();
	}
	
	function selectionChanged() {
		local idx = mBookList.getSelectedRow();
		mButtonNext.setEnabled(false);			
		mButtonPrevious.setEnabled(false);
		if(idx != -1) {
			mSelectedBook = mBooks[idx];
			mBookLabel.setText(mSelectedBook.mTitle);
			::_Connection.sendQuery("book.get", this, [
				mSelectedBook.mID
			]);
		}
		else {
			mBookLabel.setText("No Book Selected");
			mPageLabel.setText("");
			mBookText.setText("Select one of the available books");
			mSelectedBook = null;
		}
	}
	
	function refresh() {
		mLoaded = false;
		if(mSelectedBook != null) {			
			mSelectOnLoad = mSelectedBook.mID;
			mSelectPageOnLoad = mSelectedPage;
		}
		else {
			mSelectOnLoad = null;
			mSelectPageOnLoad = null;
		}
		mSelectedPage = 1;
		mSelectedBook = null;
		::_Connection.sendQuery("book.list", this);
	}
	
	function refreshAndShowBookPage(bookId, pageNumber) {
		mLoaded = false;
		showBookPage(bookId, pageNumber)
		::_Connection.sendQuery("book.list", this);
	}
	
	function showBookPage(bookId, pageNumber) {
		if(!mLoaded) {
			// Not loaded yet?
			mSelectOnLoad = bookId;
			mSelectPageOnLoad  = pageNumber;
			return;	
		}
		else {
			for(local i = 0 ; i < mBooks.len(); i++) {
				local bk = mBooks[i];
				if(bk.mID == bookId) {
					mAdjusting = true;
					mBookList.setSelectedRows([i]);
					mAdjusting = false;
					mSelectedPage = pageNumber;
					selectionChanged();
					return;
				}
			}		
		}
		
		IGIS.error("Could not show book " + bookId + ", page " + ( pageNumber + 1 ) );
	}
	
	function onButtonPressed(button) {
		if(button == mButtonBind) {
			local queryArgument = [];
			foreach(item in mBindingContainer.getAllActionButtons(true))
				queryArgument.append(item.mAction.mItemId);
				
			foreach(item in mBindingAgentsContainer.getAllActionButtons(true))
				queryArgument.append(item.mAction.mItemId);
				
			if(queryArgument.len() == 0)
			{
				IGIS.info("You have not provided any items.");
				return;
			}
			mButtonBind.setEnabled(false);
			::_Connection.sendQuery("mod.craft", this, queryArgument);
		}
		else if(mSelectedBook != null) {
			if(button == mButtonNext) {
				if(mSelectedPage < mSelectedBook.mTotalPages - 1)
					mSelectedPage++;		
			}
			else if(button == mButtonPrevious) {
				if(mSelectedPage > 0)
					mSelectedPage--;			
			}
			redisplayPage();	
		}
	}
	
	function setVisible(visible) {
		if (visible) {
			::Audio.playSound("Sound-QuestLogOpen.ogg");
		}
		else {
			::Audio.playSound("Sound-QuestLogClose.ogg");
		}
		if(visible && !mInited) {
			mInited = true;
			::_Connection.sendQuery("book.list", this);		
			refreshBookshelf();
		}
		if(!visible && mBindingContainer)
			_restoreBindingContainer();
		this.GUI.Component.setVisible(visible);
	}
	
	function _restoreBindingContainer() {
		mButtonBind.setEnabled(true);
		mBindingContainer.removeAllActions();
		mBindingAgentsContainer.removeAllActions();
		mCompleteBookContainer.removeAllActions();
		local inv = ::Screens.get("Inventory",false)
		if(inv)
			inv.unlockAllActions();
	}
	
	function redisplayPage() {
		if(mSelectedBook == null) {		
			mBookText.setText("Collect <b>Books</b> when looting monsters or during quests, and they may be read here.");
			mPageLabel.setText("No Pages");			
			mButtonNext.setEnabled(false);			
			mButtonPrevious.setEnabled(false);
		}
		else {
			if(mSelectedPage > mSelectedBook.mTotalPages) 
				mSelectedPage = mSelectedBook.mTotalPages - 1;
			else if(mSelectedPage < 0)
				mSelectedPage = 0;
				
			mButtonNext.setEnabled(mSelectedPage < mSelectedBook.mPages.len() - 1);			
			mButtonPrevious.setEnabled(mSelectedPage > 0);
			
			mPageLabel.setText("Page: " + ( mSelectedPage + 1));			
				
			if(mSelectedBook.mPages[mSelectedPage] == null || mSelectedBook.mPages[mSelectedPage].len() == 0) {
				mBookText.setText("You do not yet have page <b>" + ( mSelectedPage + 1 ) + "</b>.");
			}
			else {
				mBookText.setText(mSelectedBook.mPages[mSelectedPage]);
			}
		}
	}

	function onQueryError(qa, error) {
		switch(qa.query) {
		case "mod.craft":
			_restoreBindingContainer();
			break;
		}
		IGIS.error(error);
	}
	
	function onQueryComplete( qa, results )	{
		switch(qa.query) {
		case "mod.craft":
			_restoreBindingContainer();
			break;
		case "book.item":
			local item = ::_ItemManager.getItem(results[0][0].tointeger());
			mCompleteBookContainer.removeAllActions();
			mCompleteBookContainer.addAction(item, false, item.mItemData.mContainerSlot);
			break;
		case "book.list":
			_handleBookList(qa, results);
			break;
		case "book.get":
			_handleBook(qa, results);
			break;
		default:
			break;
		}
	}
	
	function refreshBookshelf()
	{
		local bookshelf = ::_ItemDataManager.getContents("bookshelf");
		this.onContainerUpdated("bookshelf", ::_avatar.getID(), bookshelf);
	}
	
	function onContainerUpdated( containerName, creatureId, container )
	{
		if (!container.hasAllItems())
		{
			return;
		}

		if (creatureId == ::_avatar.getID())
		{
			if (containerName == "bookshelf")
			{
				if (mBookInventory)
				{
					mBookInventory.removeAllActions();
					foreach( itemId in container.mContents )
					{
						local item = ::_ItemManager.getItem(itemId);
						mBookInventory.addAction(item, false, item.mItemData.mContainerSlot);
					}
					mBookInventory.updateContainer();
				}
			}
			
		}
	}
	
	function _handleBook(qa, results) {
		if(mSelectedBook != null) {
			for(local i = 0 ; i < mSelectedBook.mPages.len(); i++)
				mSelectedBook.mPages[i] = "";
			foreach( item in results ) {
				local idx = item[0].tointeger();
				if(idx < 0)
					idx = 0;
				if(idx >= mSelectedBook.mPages.len())
					log.error("Want book page " + idx + " but there are only " + mSelectedBook.mPages.len() + " pages in the book object");
				else
					mSelectedBook.mPages[idx] = item[1];
			}
		}
		redisplayPage();
	}
	
	function _handleBookList(qa, results) {
		local selRow = mBookList.getSelectedRow();
		mBookList.removeAllRows();
		mBooks = [];
		local row = 0;
		mAdjusting = true;
		local selPage = mSelectedPage;
		foreach( item in results ) {
			local book = Book();
			book.mID = item[0].tointeger();
			book.mTitle = item[1];
			book.mTotalPages = item[2].tointeger();
			while(book.mPages.len() < book.mTotalPages) 
				book.mPages.append("");

			local obj = BookObject(book);
			mBookList.addRow([obj]);
			mBooks.append(book);
			if(mSelectOnLoad != null && mSelectOnLoad == book.mID) {
				selRow = row;
				selPage = mSelectPageOnLoad;
				mSelectOnLoad = null;
				mSelectPageOnLoad = null;
			}
			row++;
		}
		mAdjusting = false;
		mLoaded = true;
		if(mBooks.len() > 0) {
			mAdjusting = true;
			mSelectedPage = selPage;
			mBookList.setSelectedRows([selRow == -1 ? 0 : selRow]);
			selectionChanged();
			mAdjusting = false;
		}	
	}
}

function InputCommands::Books(args) {
	Screens.show("Books");
}
