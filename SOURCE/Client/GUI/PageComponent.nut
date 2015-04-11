class this.GUI.PageComponent extends this.GUI.Component
{
	static mClassName = "PageComponent";
	static C_BUTTON_HEIGHT = 23;
	mNextPage = null;
	mPreviousPage = null;
	mPageLabel = null;
	mCurrentPage = 1;
	mMaxPages = 1;
	mMessageBroadcaster = null;
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.GridLayout(1, 3));
		this.getLayoutManager().setRows(this.C_BUTTON_HEIGHT);
		this.getLayoutManager().setColumns(this.C_BUTTON_HEIGHT + 2, "*", this.C_BUTTON_HEIGHT + 2);
		this.mPreviousPage = this.GUI.SmallButton("LeftArrow");
		this.mPreviousPage.setPressMessage("onPreviousPressed");
		this.mPreviousPage.addActionListener(this);
		this.add(this.mPreviousPage);
		this.mPageLabel = this.GUI.Label("PAGE " + this.mCurrentPage + "/" + this.mMaxPages);
		this.mPageLabel.setFontColor(this.Colors.white);
		this.mPageLabel.setFont(::GUI.Font("Maiandra", 30, false));
		this.mPageLabel.setTextAlignment(0.5, 0.5);
		this.add(this.mPageLabel);
		this.mNextPage = this.GUI.SmallButton("RightArrow");
		this.mNextPage.setPressMessage("onNextPressed");
		this.mNextPage.addActionListener(this);
		this.add(this.mNextPage);
		this._updateButtonVisible();
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function setTotalPages( pageNumber )
	{
		this.mMaxPages = pageNumber;

		if (this.mMaxPages == 0)
		{
			this.mCurrentPage = 0;
		}

		this._updateButtonVisible();
		this.updateLabelText();
	}

	function setCurrentPage( pageNumber )
	{
		if (pageNumber > 0 && pageNumber <= this.mMaxPages)
		{
			this.mCurrentPage = pageNumber;
			this._updateButtonVisible();
			this.updateLabelText();
		}
	}

	function getCurrentPage()
	{
		return this.mCurrentPage;
	}

	function onPreviousPressed( button )
	{
		this.mCurrentPage = this.mCurrentPage - 1;
		this._updateButtonVisible();
		this.updateLabelText();
		this.mMessageBroadcaster.broadcastMessage("onPreviousButtonPressed", this);
	}

	function onNextPressed( button )
	{
		this.mCurrentPage = this.mCurrentPage + 1;
		this._updateButtonVisible();
		this.updateLabelText();
		this.mMessageBroadcaster.broadcastMessage("onNextButtonPressed", this);
	}

	function updateLabelText()
	{
		this.mPageLabel.setText("PAGE " + this.mCurrentPage + "/" + this.mMaxPages);
	}

	function _updateButtonVisible()
	{
		if (this.mCurrentPage <= 1)
		{
			this.mPreviousPage.setVisible(false);
		}
		else
		{
			this.mPreviousPage.setVisible(true);
		}

		if (this.mCurrentPage < this.mMaxPages)
		{
			this.mNextPage.setVisible(true);
		}
		else
		{
			this.mNextPage.setVisible(false);
		}

		if (this.mMaxPages == 0)
		{
			this.mPageLabel.setVisible(false);
		}
		else
		{
			this.mPageLabel.setVisible(true);
		}
	}

}

