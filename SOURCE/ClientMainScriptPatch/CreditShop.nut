this.require("UI/Screens");
this.require("GUI/CreditShopComponents");
this.require("UI/CreditShopDef");
class this.Screens.CreditShop extends this.GUI.Frame
{
	static mScreenName = "CreditShop";
	static C_BASE_HEIGHT = 600;
	static C_BASE_WIDTH = 800;
	static TOP_CENTER_WIDTH = 786;
	static TOP_CENTER_HEIGHT = 480;
	static BASE_WIDTH = 246;
	static BASE_HEIGHT = 138;
	static BOTTOM_SELECT_WIDTH = 606;
	static BOTTOM_SELECT_HEIGHT = 80;
	static CREDITS_PANEL_WIDTH = 172;
	static TOP_POSITION_OFFSET_X = 7;
	static TOP_POSITION_OFFSET_Y = 3;
	mNineBoxPanel = null;
	mPreviewPanel = null;
	mNameChangePanel = null;
	mLastViewType = null;
	mCurrentCreditLabel = null;
	mCurrentButtonSelected = null;
	mUpdateItemListEvent = null;
	CreditMainPanels = null;
	mPackageWaiter = null;
	mSelectionPanel = null;
	constructor()
	{
		this.GUI.Frame.constructor("Credit Shop");
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		local baseInsetSize = 7;
		local baseComponent = this.GUI.Container(null);
		this.setContentPane(baseComponent);
		this.mNineBoxPanel = this.GUI.NineItemPanel(this.TOP_CENTER_WIDTH, this.TOP_CENTER_HEIGHT, this.TOP_POSITION_OFFSET_X, this.TOP_POSITION_OFFSET_Y, this.BASE_HEIGHT, this.BASE_WIDTH);
		this.mNineBoxPanel.setVisible(false);
		baseComponent.add(this.mNineBoxPanel);
		this.mPreviewPanel = this.GUI.CreditPreviewPanel(this.TOP_CENTER_WIDTH, this.TOP_CENTER_HEIGHT, this.TOP_POSITION_OFFSET_X, this.TOP_POSITION_OFFSET_Y, this.BASE_HEIGHT, this.BASE_WIDTH);
		this.mPreviewPanel.setVisible(false);
		baseComponent.add(this.mPreviewPanel);
		this.mNameChangePanel = this.GUI.CreditNameChangePanel(this.TOP_CENTER_WIDTH, this.TOP_CENTER_HEIGHT, this.TOP_POSITION_OFFSET_X, this.TOP_POSITION_OFFSET_Y, this.BASE_HEIGHT, this.BASE_WIDTH);
		this.mNameChangePanel.setVisible(false);
		baseComponent.add(this.mNameChangePanel);
		this.CreditMainPanels = {
			[this.CreditScreenTypes.DEFAULT] = {
				screen = this.mNineBoxPanel
			},
			[this.CreditScreenTypes.PREVIEW] = {
				screen = this.mPreviewPanel
			},
			[this.CreditScreenTypes.NAME] = {
				screen = this.mNameChangePanel
			}
		};
		this.mSelectionPanel = this._buildSelectionPanel();
		baseComponent.add(this.mSelectionPanel);
		local buyCreditPanel = this._buildbuyCreditsPanel();
		baseComponent.add(buyCreditPanel);
		::_creditShopManager.addListener(this);
		::_Connection.addListener(this);
		this.addActionListener(this);
		this.setCached(::Pref.get("video.UICache"));
	}

	function _invalidate()
	{
		if (!this.mPreviewPanel.getPreviewsLoaded())
		{
			::_eventScheduler.fireIn(1.5, this, "_invalidate");
			local cached = ::Pref.get("video.UICache");

			if (cached)
			{
				this.setCached(false);
				this.setCached(true);
			}
		}
	}

	function setVisible( value )
	{
		if (value)
		{
			::_tutorialManager.onScreenOpened("CreditShop");			
			::_creditShopManager.requestItemMarketList();
			local callback = {
				previewPanel = this.mPreviewPanel,
				function onPackageComplete( pkgName )
				{
					this.previewPanel.setPreviewsLoaded(true);
				}

				function onPackageError( pkg, error )
				{
					this.log.debug("Error loading package " + pkg + " - " + error);
					this.onPackageComplete(pkg);
				}

			};
			this.mPackageWaiter = this.Util.waitForAssets("Preview-Armor", callback, this.ContentLoader.PRIORITY_FETCH);
		}

		this.GUI.Frame.setVisible(value);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
		this.mUpdateItemListEvent = ::_eventScheduler.fireIn(30.0, this, "_requestMarketList");
		::_eventScheduler.fireIn(1.5, this, "_invalidate");
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);

		if (this.mNameChangePanel)
		{
			this.mNameChangePanel.onClose();
		}

		if (this.mUpdateItemListEvent)
		{
			::_eventScheduler.cancel(this.mUpdateItemListEvent);
		}

		this.GUI.Component._removeNotify();
	}

	function _requestMarketList()
	{
		::_creditShopManager.requestItemMarketList();
		this.mUpdateItemListEvent = ::_eventScheduler.fireIn(30.0, this, "_requestMarketList");
	}

	function _buildSelectionPanel()
	{
		local GAP_SIZE = 12;
		local BUTTON_WIDTH = 62;
		local BUTTON_HEIGHT = 61;
		local bottomSelectPanel = this.GUI.InnerPanel(this.GUI.BoxLayout());
		bottomSelectPanel.setSize(this.BOTTOM_SELECT_WIDTH, this.BOTTOM_SELECT_HEIGHT);
		bottomSelectPanel.setPreferredSize(this.BOTTOM_SELECT_WIDTH, this.BOTTOM_SELECT_HEIGHT);
		bottomSelectPanel.setInsets(2, 0, 2, 8);
		bottomSelectPanel.getLayoutManager().setGap(GAP_SIZE);
		bottomSelectPanel.setPosition(this.TOP_POSITION_OFFSET_X, this.TOP_POSITION_OFFSET_Y + this.TOP_CENTER_HEIGHT + 5);
		local pageRadioGroup = this.GUI.RadioGroup();
		pageRadioGroup.addListener(this);

		foreach( key, creditSelectItems in this.CreditSelectInfo )
		{
			// TODO - Pets are hidden for now
			if(key != CreditSelectType.PETS) {
				local button = this.GUI.ImageButton();
				button.setRadioGroup(pageRadioGroup);
				button.setData(key);
				button.setTooltip(creditSelectItems.name);
				button.setSize(BUTTON_WIDTH, BUTTON_HEIGHT);
				button.setPreferredSize(BUTTON_WIDTH, BUTTON_HEIGHT);
				button.setAppearance("CreditShop/" + creditSelectItems.imageName);
				button.setGlowImageName("CreditShop/Glow");
				button.addActionListener(this);
				button.setPressMessage("_selectedIcons");
	
				if (key == this.CreditSelectType.CONSUMABLES)
				{
					button.setSelection(true);
					this._selectedIcons(button, false);
				}
	
				bottomSelectPanel.add(button);
			}
		}

		return bottomSelectPanel;
	}

	function _buildbuyCreditsPanel()
	{
		local GAP_SIZE = 10;
		local buyCreditPanel = this.GUI.InnerPanel(this.GUI.BoxLayoutV());
		buyCreditPanel.setSize(this.CREDITS_PANEL_WIDTH, this.BOTTOM_SELECT_HEIGHT);
		buyCreditPanel.setPreferredSize(this.CREDITS_PANEL_WIDTH, this.BOTTOM_SELECT_HEIGHT);
		buyCreditPanel.setInsets(10, 8, 0, 8);
		buyCreditPanel.getLayoutManager().setGap(GAP_SIZE);
		buyCreditPanel.setPosition(this.TOP_POSITION_OFFSET_X + this.BOTTOM_SELECT_WIDTH + 5, this.TOP_POSITION_OFFSET_Y + this.TOP_CENTER_HEIGHT + 5);
		local creditAmt = 0;

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		this.mCurrentCreditLabel = this.GUI.Label("Current Credits: " + creditAmt);
		this.mCurrentCreditLabel.setFontColor(this.Colors.white);
		this.mCurrentCreditLabel.setFont(::GUI.Font("Maiandra", 22, false));
		buyCreditPanel.add(this.mCurrentCreditLabel);
		
		/*
		local buyCreditsButton = this.GUI.BlueNarrowButton("Buy Credits!");
		buyCreditsButton.setFixedSize(115, 32);
		buyCreditsButton.setFont(::GUI.Font("Maiandra", 22, false));
		buyCreditsButton.addActionListener(this);
		buyCreditsButton.setPressMessage("_buyCreditsPressed");
		buyCreditsButton.setTooltip("Buy credtis at www.eartheternal.com");
		buyCreditPanel.add(buyCreditsButton);
		*/
		
		return buyCreditPanel;
	}

	function selectPanel( metaData )
	{
		local selectType;

		foreach( type, creditItemType in ::CreditSelectInfo )
		{
			if (creditItemType.key == metaData)
			{
				selectType = type;
			}
		}

		if (selectType != null)
		{
			local buttons = this.mSelectionPanel.components;

			foreach( button in buttons )
			{
				local buttonData = button.getData();

				if (buttonData != null && buttonData == selectType)
				{
					button.setSelection(true);
					this._selectedIcons(button, false);
				}
			}
		}
	}

	function _selectedIcons( button, ... )
	{
		local serverSpoofUpdate = false;

		if (vargc > 0)
		{
			serverSpoofUpdate = vargv[0];
		}

		this.mCurrentButtonSelected = button;

		if (button == null || button.getData() == null)
		{
			return;
		}

		local viewType = this.CreditSelectInfo[button.getData()].screenType;

		if (this.mLastViewType != null && this.mLastViewType != viewType)
		{
			local currentScreenShowing = this.CreditMainPanels[this.mLastViewType].screen;

			if (currentScreenShowing)
			{
				currentScreenShowing.setVisible(false);
			}
		}

		local newScreenShowing = this.CreditMainPanels[viewType].screen;

		if (newScreenShowing)
		{
			newScreenShowing.setVisible(true);

			if (viewType == this.CreditScreenTypes.DEFAULT || viewType == this.CreditScreenTypes.PREVIEW)
			{
				newScreenShowing.resetItems(this.CreditSelectInfo[button.getData()].key, serverSpoofUpdate);
			}
		}

		this.mLastViewType = viewType;
	}

	function _buyCreditsPressed( button )
	{
		local callback = {
			function onActionSelected( mb, alt )
			{
				if (alt == "Ok")
				{
					this.System.openURL("http://www.eartheternal.com/credits");
				}
			}

		};
		this.GUI.MessageBox.showOkCancel("This will open a new browser window.", callback);
	}

	function onCreditItemsUpdate( creditShopManager )
	{
		this._selectedIcons(this.mCurrentButtonSelected, true);
	}

	function onCreditUpdated( value )
	{
		this.mCurrentCreditLabel.setText("Current Credits: " + value);

		if (this.mNameChangePanel)
		{
			this.mNameChangePanel.updatePurchaseButton(value);
		}
	}

	function onFrameClosed()
	{
		if (this.mUpdateItemListEvent)
		{
			::_eventScheduler.cancel(this.mUpdateItemListEvent);
		}

		if (this.mNameChangePanel)
		{
			this.mNameChangePanel.onClose();
		}
	}

}

