this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.IconBrowserScreen extends this.GUI.Frame
{
	static mClassName = "Screens.IconBrowserScreen";
	static mTotalIcons = 96;
	mContainer = null;
	mIconNameFilter = null;
	mIconNameFilterString = "";
	mIcons = null;
	mIconNames = null;
	mIconPageIndex = 0;
	mIconPageIndexLabel = null;
	mIconNamesDisplayed = null;
	mIconHighlighted = null;
	mIconHighlitedFilename = null;
	mIconSelection = null;
	mIconSelectionListener = null;
	constructor()
	{
		this.GUI.Frame.constructor("Icon Browser");
		this.mIcons = [];
		this.mIcons.resize(this.mTotalIcons, null);
		this.mIconNamesDisplayed = [];
		this.mIconNamesDisplayed.resize(this.mTotalIcons, null);
		this.mIconNames = [];
		local part1 = this.GUI.Container(this.GUI.GridLayout(1, 2));
		part1.getLayoutManager().setColumns(64, "*");
		part1.setInsets(4);
		part1.add(this.GUI.Label("Name filter:"));
		this.mIconNameFilter = this.GUI.InputArea();
		this.mIconNameFilter.addActionListener(this);
		part1.add(this.mIconNameFilter);
		local part21 = this.GUI.Container(this.GUI.GridLayout(8, 12));
		part21.getLayoutManager().setRows(42, 42, 42, 42, 42, 42, 42, 42);
		part21.getLayoutManager().setColumns(42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42);
		part21.setInsets(4);

		for( local i = 0; i < 96; ++i )
		{
			local b = this.GUI.ImageButton();
			b.addActionListener(this);
			b.setPressMessage("_onChangeSelection");
			b.setGlowImageName("Icon-Disable.png");
			b.setSelection(false);
			b.setVisible(false);
			part21.add(b);
			this.mIcons[i] = b;
		}

		local part22 = this.GUI.Container(this.GUI.GridLayout(3, 1));
		part22.getLayoutManager().setRows(25, "*", 25);
		part22.getLayoutManager().setColumns(25);
		part22.setInsets(4);
		local upButton = this.GUI.SmallButton("PageUp");
		upButton.setPressMessage("onPageUp");
		upButton.addActionListener(this);
		part22.add(upButton);
		this.mIconPageIndexLabel = this.GUI.Label("");
		part22.add(this.mIconPageIndexLabel);
		local downButton = this.GUI.SmallButton("PageDown");
		downButton.setPressMessage("onPageDown");
		downButton.addActionListener(this);
		part22.add(downButton);
		local part2 = this.GUI.Container(this.GUI.GridLayout(1, 2));
		part2.getLayoutManager().setRows(350);
		part2.getLayoutManager().setColumns("*", 32);
		part2.add(part21);
		part2.add(part22);
		local part3 = this.GUI.Container(this.GUI.GridLayout(1, 2));
		part3.getLayoutManager().setColumns(64, "*");
		part3.setInsets(4);
		part3.add(this.GUI.Label("Highlighted:"));
		this.mIconHighlitedFilename = this.GUI.InputArea();
		part3.add(this.mIconHighlitedFilename);
		local part4 = this.GUI.Container(this.GUI.GridLayout(1, 4));
		part4.getLayoutManager().setRows(32);
		part4.getLayoutManager().setColumns("*", 1, 1, 76);
		part4.setInsets(4);
		part4.add(this.GUI.Spacer(1, 1));
		part4.add(this.GUI.Spacer(1, 1));
		part4.add(this.GUI.Spacer(1, 1));
		part4.add(this.GUI.Button("Select Icon", this, "_onSelectIconPressed"));
		local layout = this.GUI.Container(this.GUI.GridLayout(4, 1));
		layout.getLayoutManager().setRows(32, "*", 32, 42);
		layout.add(part1);
		layout.add(part2);
		layout.add(part3);
		layout.add(part4);
		this.setContentPane(layout);
		this.setSize(575, 490);
		this.centerOnScreen();
		this._applyIconFilter();
	}

	function setIconSelectionListener( listener )
	{
		this.mIconSelectionListener = listener;
	}

	function _onSelectIconPressed( button )
	{
		this.mIconSelection = this.mIconHighlighted;

		if (this.mIconSelection != null)
		{
			this.close();
			this.log.debug("Icon Selected: " + this.mIconSelection);

			if (this.mIconSelectionListener)
			{
				this.mIconSelectionListener.onIconSelected(this.mIconSelection.getImageName());
			}
		}
	}

	function getSelection()
	{
		return this.mIconSelection;
	}

	function _onChangeSelection( button )
	{
		if (this.mIconHighlighted != null)
		{
			this.mIconHighlighted.setGlowImageName("Icon-Disable.png");
			this.mIconHighlighted.setSelection(false);
		}

		this.mIconHighlighted = button;

		if (this.mIconHighlighted)
		{
			this.mIconHighlighted.setGlowImageName("Icon-Selection.png");
			this.mIconHighlighted.setSelection(true);
			local i = 0;

			foreach( icon in this.mIcons )
			{
				if (icon == button)
				{
					this.mIconHighlitedFilename.setText(this.mIconNamesDisplayed[i]);
					return;
				}

				++i;
			}
		}
		else
		{
			this.mIconHighlitedFilename.setText("");
		}
	}

	function _applyIconFilter()
	{
		this.mIconNames = this.System.findFiles("Icon-*" + this.mIconNameFilterString + "*.png");
		this.mIconPageIndex = 0;
		this._updateIconPageDisplayed();
	}

	function _updateIconPageDisplayed()
	{
		local totDisplay = this.Math.min(this.mIconNames.len() - this.mIconPageIndex * this.mTotalIcons, this.mTotalIcons);

		for( local i = 0; i < totDisplay; i++ )
		{
			local iconNameIndex = this.mIconPageIndex * this.mTotalIcons + i;
			this.mIcons[i].setVisible(true);
			this.mIcons[i].setImageName(this.mIconNames[iconNameIndex]);
			this.mIcons[i].setGlowImageName("Icon-Disable.png");
			this.mIcons[i].setSelection(false);
			this.mIconNamesDisplayed[i] = this.mIconNames[iconNameIndex];
		}

		for( local i = totDisplay; i < this.mTotalIcons; ++i )
		{
			this.mIcons[i].setVisible(false);
			this.mIconNamesDisplayed[i] = "";
		}

		if (totDisplay)
		{
			this.mIconPageIndexLabel.setText(this.mIconPageIndex + 1 + "/" + this._getTotalPageCount());
		}
		else
		{
			this.mIconPageIndexLabel.setText("None");
		}
	}

	function _getTotalPageCount()
	{
		local totPages = this.mIconNames.len() / this.mTotalIcons;

		if (this.mIconNames.len() % this.mTotalIcons > 0)
		{
			++totPages;
		}

		return totPages;
	}

	function onPageUp( button )
	{
		local targetPageIndex = this.mIconPageIndex - 1;
		local pageCount = this._getTotalPageCount();

		if (pageCount == 0)
		{
			return;
		}

		if (targetPageIndex < 0)
		{
			targetPageIndex = pageCount - 1;
		}

		if (this.mIconPageIndex != targetPageIndex)
		{
			this.mIconPageIndex = targetPageIndex;
			this._updateIconPageDisplayed();
		}
	}

	function onPageDown( button )
	{
		local targetPageIndex = this.mIconPageIndex + 1;
		local pageCount = this._getTotalPageCount();

		if (pageCount == 0)
		{
			return;
		}

		targetPageIndex = targetPageIndex % pageCount;

		if (this.mIconPageIndex != targetPageIndex)
		{
			this.mIconPageIndex = targetPageIndex;
			this._updateIconPageDisplayed();
		}
	}

	function onInputComplete( inputarea )
	{
		if (inputarea == this.mIconNameFilter)
		{
			local filterTest = this.mIconNameFilter.getText().tolower();

			if (this.mIconNameFilterString != filterTest)
			{
				this.mIconNameFilterString = filterTest;
				this._applyIconFilter();
				this._onChangeSelection(null);
			}
		}
	}

}

