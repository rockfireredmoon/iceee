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
	mSelectedPage = 1;
	mSelectedBook = null;
	mBookLabel = null;
	mPageLabel = null;
	mSelectOnLoad = null;
	mLoaded = false;
	mAdjusting = false;
	mInited = false; 
	
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

		local buttonRow = GUI.Container();
		
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
		scrollArea.setPreferredSize(310, 368);	
		scrollArea.attach(mBookText);
		
		mBookList = GUI.ColumnList();
		mBookList.setPreferredSize(180, 368);
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
		
		mScreenContainer = GUI.Container(GUI.GridLayout(1, 2));
		mScreenContainer.setInsets(5);
		mScreenContainer.getLayoutManager().setColumns(180, "*");
		mScreenContainer.getLayoutManager().setRows("*");
		mScreenContainer.add(mBookList);
		mScreenContainer.add(readerBox);
		
		setContentPane(mScreenContainer);
		setSize(520, 440);
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
			mSelectedBook = mBookList.mRowContents[idx][0].mBook;
			print("ICE! selectionChanged " + idx + " sel: " + mSelectedBook);
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
	
	function refreshAndShowBookPage(bookId, pageNumber) {
		mLoaded = false;
		showBookPage(bookId, pageNumber)
		::_Connection.sendQuery("book.list", this);
	}
	
	function showBookPage(bookId, pageNumber) {
		print("ICE! showBookPage " + bookId + " / " + pageNumber + "\n");
		if(!mLoaded) {
			// Not loaded yet?
			print("ICE! showBookPage not loaded yet");
			mSelectOnLoad = bookId;
			mSelectedPage = pageNumber;		
			return;	
		}
		else {
			print("ICE! showBookPage loaded!");
			for(local i = 0 ; i < mBookList.mRowContents.len(); i++) {
				local bk = mBookList.mRowContents[i][0].mBook;
				if(bk.mID == bookId) {
					print("ICE! Found book at row " + i + "\n");
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
		if(mSelectedBook != null) {
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
		if(visible && !mInited) {
			mInited = true;
			::_Connection.sendQuery("book.list", this);
		}
		GUI.Frame.setVisible(visible);
	}
	
	function redisplayPage() {
		print("ICE! redisplay page " + mSelectedPage + "\n");
		if(mSelectedBook == null) {		
			mBookText.setText("Collect <b>Books</b> when looting monsters or during quests, and they may be read here.");
			mPageLabel.setText("No Pages");			
			mButtonNext.setEnabled(false);			
			mButtonPrevious.setEnabled(false);
		}
		else {
			
			if(mSelectedPage > mSelectedBook.mTotalPages) 
				mSelectedPage = mSelectedBook.mTotalPages - 1;
				
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
		IGIS.error(error);
	}
	
	function onQueryComplete( qa, results )	{
		print("ICE! Books: onQueryComplete\n");
		switch(qa.query) {
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
	
	function _handleBook(qa, results) {
		print("ICE! Book: _handleBook\n");
		if(mSelectedBook != null) {
			foreach( item in results ) {
				print("ICE! Book " + item[0].tointeger() + " = " + item[1]);
				mSelectedBook.mPages[item[0].tointeger()] = item[1];
			}
		}
		redisplayPage();
	}
	
	function _handleBookList(qa, results) {
		print("ICE! Books: _handleBookList " + mBookList + "\n");
		local selRow = mBookList.getSelectedRow();
		mBookList.removeAllRows();
		local row = 0;
		foreach( item in results ) {
			print("ICE! A book\n");
			local book = Book();
			book.mID = item[0].tointeger();
			book.mTitle = item[1];
			book.mTotalPages = item[2].tointeger();
			book.mPages.resize(book.mTotalPages);
			print("ICE! A book " + book.mID + " " + book.mTitle + " "  + book.mTotalPages + "\n");
			local obj = BookObject(book);
			mBookList.addRow([obj]);
			if(mSelectOnLoad != null && mSelectOnLoad == book.mID) {
				selRow = row;
				mSelectOnLoad = null;
			}
			row++;
		}
		mLoaded = true;
		if(mBookList.mRowContents.len() > 0) {
			mAdjusting = true;
			mBookList.setSelectedRows([selRow == -1 ? 0 : selRow]);
			selectionChanged();
			mAdjusting = false;
		}	
	}
}

function InputCommands::Books(args) {
	Screens.show("Books");
}
