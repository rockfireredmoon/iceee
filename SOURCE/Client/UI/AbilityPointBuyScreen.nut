this.require("UI/Screens");
class this.Screens.AbilityPointBuyScreen extends this.GUI.Frame
{
	static WindowSize = {
		DEFAULT = {
			width = 800,
			height = 240
		},
		EXPANDED = {
			width = 800,
			height = 435
		}
	};
	ButtonInfo = {
		[0] = {
			point = 1,
			price = 2
		},
		[1] = {
			point = 5,
			price = 30
		},
		[2] = {
			point = 10,
			price = 110
		},
		[3] = {
			point = 25,
			price = 250
		}
	};
	mAbilityPricingComp = null;
	mNameHTML = null;
	mButtons = null;
	mPendingPointBuy = false;
	mAbScreen = null;
	mLoadingContents = null;
	mLoadedContents = null;
	mLastButtonSelected = 0;
	mConfirmPopup = null;
	mCurrentCreditLabel = null;
	mExpanded = false;
	mCost = 0;
	mAbilityPurchased = 0;
	mCornerLeftImage = null;
	mCornerRightImage = null;
	constructor()
	{
		this.GUI.BigFrame.constructor("Buy Ability Points");
		this.mButtons = [];
		this.mAbScreen = this.GUI.Container(null);
		this.mAbScreen.setSize(this.WindowSize.DEFAULT.width, this.WindowSize.DEFAULT.height);
		this.mAbScreen.setPreferredSize(this.WindowSize.DEFAULT.width, this.WindowSize.DEFAULT.height);
		this.mLoadedContents = this._buildMainScreen();
		this.mLoadedContents.setVisible(false);
		this.mLoadedContents.setPosition(20, 0);
		this.mLoadingContents = this.GUI.Container();
		this.mLoadingContents.add(this.GUI.Label("Waiting for server to respond..."));
		this.mLoadingContents.setPosition(0, 0);
		this.mAbScreen.add(this.mLoadingContents);
		this.mAbScreen.add(this.mLoadedContents);
		this.mCornerLeftImage = this.GUI.Component(null);
		this.mCornerLeftImage.setAppearance("CornerElementLeft");
		this.mCornerLeftImage.setSize(93, 90);
		this.mCornerLeftImage.setPreferredSize(93, 90);
		this.mCornerLeftImage.setPosition(4, this.WindowSize.DEFAULT.height - 145);
		this.mAbScreen.add(this.mCornerLeftImage);
		this.mCornerRightImage = this.GUI.Component(null);
		this.mCornerRightImage.setAppearance("CornerElementRight");
		this.mCornerRightImage.setSize(93, 90);
		this.mCornerRightImage.setPreferredSize(93, 90);
		this.mCornerRightImage.setPosition(this.WindowSize.DEFAULT.width - 98, this.WindowSize.DEFAULT.height - 145);
		this.mAbScreen.add(this.mCornerRightImage);
		this.mAbilityPricingComp = this.GUI.AbilityPointPricingComp();
		this.mAbilityPricingComp.setVisible(false);
		this.mAbilityPricingComp.setPosition(15, this.WindowSize.DEFAULT.height - 60);
		this.mAbScreen.add(this.mAbilityPricingComp);
		this._getTotalAbilityPurchased();
		this.setContentPane(this.mAbScreen);
		this.setSize(this.WindowSize.DEFAULT.width, this.WindowSize.DEFAULT.height);
		this.setPosition(::Screen.getWidth() / 2 - 200, ::Screen.getHeight() / 2 - 95);
		::_Connection.addListener(this);
	}

	function setVisible( value )
	{
		if (value)
		{
			this.GUI.Frame.setVisible(true);

			if (!this.mPendingPointBuy)
			{
				::_Connection.sendQuery("ab.point.listamount", this);
			}
		}
		else
		{
			this.GUI.Frame.setVisible(false);
		}

		this.mLastButtonSelected = 0;
	}

	function onAbilityPointBought()
	{
		this.mPendingPointBuy = false;
		::_Connection.sendQuery("ab.point.listamount", this);
		this._getTotalAbilityPurchased();
	}

	function _getTotalAbilityPurchased()
	{
		::_Connection.sendQuery("ab.point.gettotalpurchased", this);
	}

	function onQueryComplete( qa, results )
	{
		switch(qa.query)
		{
		case "ab.point.listamount":
			this.mLoadedContents.setVisible(true);
			this.mLoadingContents.setVisible(false);

			for( local i = 0; i < results.len(); i++ )
			{
				local notEnoughCredits = false;
				local tooLowLevel = false;
				local buttonInfo = this.ButtonInfo[i];
				local cost = 0;
				local splitParam = this.Util.split(results[i][1], "NEC_");
				local abPoints = results[i][0].tointeger();

				if (splitParam.len() > 1)
				{
					notEnoughCredits = true;
				}
				else
				{
					splitParam = this.Util.split(splitParam[0], "LTL");

					if (splitParam.len() > 1)
					{
						tooLowLevel = true;
					}
				}

				if (tooLowLevel || notEnoughCredits)
				{
					if (!tooLowLevel)
					{
						cost = splitParam[1].tointeger();
					}
				}
				else
				{
					cost = splitParam[0].tointeger();
				}

				this.mButtons[i].setEnabled(true);

				if (abPoints == 1)
				{
					this.mCost = cost;
					this._updateFirstLine();
				}

				this._setAbilityPointText(this.mButtons[i], abPoints);
				this._setCreditText(this.mButtons[i], cost);
				buttonInfo.price = cost;
				buttonInfo.point = abPoints;
			}

			break;

		case "ab.point.price":
			local notEnoughCredits = false;
			local tooLowLevel = false;
			local abPoints = results[0][0].tointeger();
			local cost = 0;
			local splitParam = this.Util.split(results[0][1], "NEC_");

			if (splitParam.len() > 1)
			{
				notEnoughCredits = true;
			}
			else
			{
				splitParam = this.Util.split(splitParam[0], "LTL");

				if (splitParam.len() > 1)
				{
					tooLowLevel = true;
				}
			}

			if (tooLowLevel || notEnoughCredits)
			{
				if (!tooLowLevel)
				{
					cost = splitParam[1].tointeger();
				}
			}
			else
			{
				cost = splitParam[0].tointeger();
			}

			if (tooLowLevel)
			{
				this.mConfirmPopup = this.GUI.MessageBox.show("You are not a high enough level to purchase this many points.");
				this.GUI._Manager.addTransientToplevel(this.mConfirmPopup);
			}
			else if (notEnoughCredits)
			{
				local messageBox = this.GUI.MessageBox.showEx("You currently do not have enough Credits to make<br>this purchase.  " + "Would you like to buy more Credits?<br>Current credits: " + this.creditAmt, [
					"Buy Credits!",
					"Decline"
				], this, "_onBuyCredits");
				messageBox.setButtonColor(0, "Blue");
				messageBox.setButtonColor(1, "Red");
				messageBox.setButtonSize(0, 100, 32);
				messageBox.setButtonSize(1, 70, 32);
				this.GUI._Manager.addTransientToplevel(messageBox);
			}
			else
			{
				this._abSpecificPurchase(abPoints, cost);
			}

			break;

		case "ab.point.gettotalpurchased":
			local purchased = results[0][0].tointeger();
			this.mAbilityPurchased = purchased;
			this._updateFirstLine();
			break;
		}
	}

	function _abPurchase( button )
	{
		local index = button.getData();

		if (index < this.mButtons.len())
		{
			this.mLastButtonSelected = index;
		}

		local itemCost = this.ButtonInfo[this.mLastButtonSelected].price;
		local creditAmt = 1000000;

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		if (creditAmt >= itemCost)
		{
			local purchaseMessageBox = this.GUI.MessageBox.showEx("You want to purchase " + this.ButtonInfo[this.mLastButtonSelected].point + " Ability Points for " + this.ButtonInfo[this.mLastButtonSelected].price + " Credits?", [
				"Accept",
				"Decline"
			], this, "_onConfirmPurchase");
			purchaseMessageBox.setButtonColor(0, "Blue");
			purchaseMessageBox.setButtonColor(1, "Red");
			purchaseMessageBox.setButtonSize(0, 90, 32);
			purchaseMessageBox.setButtonSize(1, 70, 32);
		}
		else
		{
			local messageBox = this.GUI.MessageBox.showEx("You currently do not have enough Credits to make<br>this purchase.  " + "Would you like to buy more Credits?<br>Current credits: " + creditAmt, [
				"Buy Credits!",
				"Decline"
			], this, "_onBuyCredits");
			messageBox.setButtonColor(0, "Blue");
			messageBox.setButtonColor(1, "Red");
			messageBox.setButtonSize(0, 100, 32);
			messageBox.setButtonSize(1, 70, 32);
		}
	}

	function _onBuyCredits( source, action )
	{
		switch(action)
		{
		case "Buy Credits!":
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
			break;

		default:
			if (action == "Decline")
			{
			}
		}
	}

	function _abSpecificPurchase( numPoints, cost )
	{
		local creditAmt = 1000000;

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		if (creditAmt >= cost)
		{
			local purchaseMessageBox = this.GUI.MessageBox.showEx("You want to purchase " + numPoints + " Ability Points for " + cost + " Credits?", [
				"Accept",
				"Decline"
			], this, "_onConfirmPurchase");
			purchaseMessageBox.setButtonColor(0, "Blue");
			purchaseMessageBox.setButtonColor(1, "Red");
			purchaseMessageBox.setButtonSize(0, 90, 32);
			purchaseMessageBox.setButtonSize(1, 70, 32);
		}
		else
		{
			local messageBox = this.GUI.MessageBox.showEx("You currently do not have enough Credits to make<br>this purchase.  " + "Would you like to buy more Credits?<br>Current credits: " + creditAmt, [
				"Buy Credits!",
				"Decline"
			], this, "_onBuyCredits");
			messageBox.setButtonColor(0, "Blue");
			messageBox.setButtonColor(1, "Red");
			messageBox.setButtonSize(0, 100, 32);
			messageBox.setButtonSize(1, 70, 32);
		}
	}

	function _buildMainScreen()
	{
		local creditOptions = this.GUI.Container(this.GUI.GridLayout(7, 4));
		creditOptions.getLayoutManager().setColumns(182, 182, 182, 182);
		creditOptions.getLayoutManager().setRows(30, 5, 62, 5, 25, 15, 40);
		creditOptions.setSize(this.WindowSize.DEFAULT.width - 10, this.WindowSize.DEFAULT.height);
		creditOptions.setPreferredSize(this.WindowSize.DEFAULT.width - 10, this.WindowSize.DEFAULT.height);
		creditOptions.getLayoutManager().setGaps(10, 0);
		local totalPointsPurchased = 0;
		local firstPointCost = 2;
		local playerName = "Default";

		if (this._avatar)
		{
			playerName = this._avatar.getStat(this.Stat.DISPLAY_NAME);
		}

		this.mNameHTML = this.GUI.HTML("<font color=\"" + this.Colors.white + "\"> " + playerName + " you purchased " + totalPointsPurchased + " Ability Points.  Your next ability Point will cost </font>" + "<font color=\"" + this.Colors.mint + "\">" + firstPointCost + " Credits.</font>");
		this.mNameHTML.setSize(this.WindowSize.DEFAULT.width, 30);
		this.mNameHTML.setPreferredSize(this.WindowSize.DEFAULT.width, 30);
		this.mNameHTML.setFont(::GUI.Font("Maiandra", 28, false));
		creditOptions.add(this.mNameHTML, {
			span = 4
		});
		creditOptions.add(this.GUI.Spacer(0, 0), {
			span = 4
		});

		foreach( index, buttonData in this.ButtonInfo )
		{
			local button = this.GUI.BlueFrameButton(this._createButtonComponent(buttonData.point));
			button.setReleaseMessage("_abPurchase");
			button.addActionListener(this);
			button.setData(index);
			this.mButtons.append(button);
			creditOptions.add(button);
		}

		creditOptions.add(this.GUI.Spacer(5, 5), {
			span = 4
		});
		creditOptions.add(this.GUI.Spacer(5, 5), {
			span = 1
		});
		local creditAndLink = this.GUI.Component(this.GUI.BoxLayout());
		creditAndLink.setSize(270, 38);
		creditAndLink.getLayoutManager().setGap(15);
		creditAndLink.setPreferredSize(270, 38);
		creditAndLink.setInsets(10, 0, 0, 40);
		creditOptions.add(creditAndLink, {
			span = 3
		});
		local creditAmt = 0;

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		this.mCurrentCreditLabel = this.GUI.Label("Current Credits: " + creditAmt);
		this.mCurrentCreditLabel.setFontColor(this.Colors.mint);
		this.mCurrentCreditLabel.setFont(::GUI.Font("Maiandra", 28, false));
		creditAndLink.add(this.mCurrentCreditLabel);
		local buyCreditButton = this.GUI.BuyCreditButton("");
		buyCreditButton.addActionListener(this);
		buyCreditButton.setPressMessage("onBuyCreditLinkPressed");
		creditAndLink.add(buyCreditButton);
		creditOptions.add(this.GUI.Spacer(5, 5), {
			span = 4
		});
		creditOptions.add(this.GUI.Spacer(5, 5), {
			span = 1
		});
		local narrowButtonHolder = this.GUI.Component(this.GUI.BoxLayoutV());
		narrowButtonHolder.setSize(300, 32);
		narrowButtonHolder.setPreferredSize(300, 32);
		creditOptions.add(narrowButtonHolder, {
			span = 2
		});
		local narrowButton = this.GUI.NarrowButton("Ability Point Pricing");
		narrowButton.setFixedSize(170, 32);
		narrowButton.addActionListener(this);
		narrowButton.setPressMessage("_expandSelected");
		narrowButtonHolder.add(narrowButton);
		return creditOptions;
	}

	function _createButtonComponent( abilityPoint )
	{
		local creditPriceComp = this.GUI.Container(this.GUI.BoxLayoutV());
		creditPriceComp.setSize(180, 62);
		creditPriceComp.setPreferredSize(180, 62);
		creditPriceComp.getLayoutManager().setGap(-2);
		creditPriceComp.setInsets(3, 0, 0, 0);
		local textHTML;

		if (abilityPoint > 1)
		{
			textHTML = this.GUI.HTML("<font color=\"" + this.Colors.white + "\"> " + abilityPoint + " Ability Points</font>");
		}
		else
		{
			textHTML = this.GUI.HTML("<font color=\"" + this.Colors.white + "\"> " + abilityPoint + " Ability Point</font>");
		}

		textHTML.setSize(180, 28);
		textHTML.setPreferredSize(180, 28);
		textHTML.setFont(::GUI.Font("Maiandra", 28, false));
		textHTML.getLayoutManager().setAlignment("center");
		creditPriceComp.add(textHTML);
		local credits = this.GUI.Credits(9999);
		credits.setFont(::GUI.Font("Maiandra", 26, false));
		credits.setSize(180, 30);
		credits.setPreferredSize(180, 30);
		credits.setAlignment(0.40000001);
		creditPriceComp.add(credits);
		return creditPriceComp;
	}

	function onBuyCreditLinkPressed( button )
	{
		this.System.openURL("http://www.eartheternal.com/credits");
	}

	function onCreditUpdated( value )
	{
		this.mCurrentCreditLabel.setText("Current Credits: " + value);
	}

	function _onConfirmPurchase( messageBox, button )
	{
		if (button == "Accept")
		{
			if (this.mLastButtonSelected < this.ButtonInfo.len())
			{
				::_Connection.sendQuery("ab.point.buy", this, this.ButtonInfo[this.mLastButtonSelected].point);
			}

			this.mPendingPointBuy = true;
		}
	}

	function _setAbilityPointText( button, abilityPoint )
	{
		if (button.components && button.components[0].components)
		{
			local components = button.components[0].components;

			foreach( comp in components )
			{
				if (comp instanceof this.GUI.HTML)
				{
					if (abilityPoint > 1)
					{
						comp.setText("<font color=\"" + this.Colors.white + "\"> " + abilityPoint + " Ability Points</font>");
					}
					else
					{
						comp.setText("<font color=\"" + this.Colors.white + "\"> " + abilityPoint + " Ability Point</font>");
					}
				}
			}
		}
	}

	function _setCreditText( button, creditAmount )
	{
		if (button.components && button.components[0].components)
		{
			local components = button.components[0].components;

			foreach( comp in components )
			{
				if (comp instanceof this.GUI.Credits)
				{
					comp.setCurrentValue(creditAmount);
				}
			}
		}
	}

	function _updateFirstLine()
	{
		local playerName = "Default";

		if (this._avatar)
		{
			playerName = this._avatar.getStat(this.Stat.DISPLAY_NAME);
		}

		this.mNameHTML.setText("<font color=\"" + this.Colors.white + "\"> " + playerName + " you purchased " + this.mAbilityPurchased + " Ability Points.  Your next ability Point will cost </font>" + "<font color=\"" + this.Colors.mint + "\">" + this.mCost + " Credits.</font>");
	}

	function _expandSelected( button )
	{
		this.mExpanded = !this.mExpanded;

		if (this.mExpanded)
		{
			this.mAbilityPricingComp.setVisible(true);
			this.mCornerLeftImage.setVisible(false);
			this.mCornerRightImage.setVisible(false);
			this.mAbScreen.setSize(this.WindowSize.EXPANDED.width, this.WindowSize.EXPANDED.height);
			this.mAbScreen.setPreferredSize(this.WindowSize.EXPANDED.width, this.WindowSize.EXPANDED.height);
			this.setSize(this.WindowSize.EXPANDED.width, this.WindowSize.EXPANDED.height);
		}
		else
		{
			if (this.mAbilityPricingComp)
			{
				this.mAbilityPricingComp.setVisible(false);
			}

			this.mCornerLeftImage.setVisible(true);
			this.mCornerRightImage.setVisible(true);
			this.mAbScreen.setSize(this.WindowSize.DEFAULT.width, this.WindowSize.DEFAULT.height);
			this.mAbScreen.setPreferredSize(this.WindowSize.DEFAULT.width, this.WindowSize.DEFAULT.height);
			this.setSize(this.WindowSize.DEFAULT.width, this.WindowSize.DEFAULT.height);
		}
	}

}

class this.GUI.AbilityPointPricingComp extends this.GUI.Component
{
	static mScreenName = "AbilityPointPricing";
	static C_BASE_WIDTH = 770;
	static C_BASE_HEIGHT = 195;
	mAbilityPointData = {
		[0] = {
			point = 1,
			cost = 2,
			type = "square"
		},
		[1] = {
			point = 2,
			cost = 4,
			type = "square"
		},
		[2] = {
			point = 3,
			cost = 6,
			type = "square"
		},
		[3] = {
			type = "arrow"
		},
		[4] = {
			point = 50,
			cost = 100,
			type = "square"
		},
		[5] = {
			point = 51,
			cost = 100,
			type = "square"
		}
	};
	constructor()
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		local titleHTML = this.GUI.HTML("Ability Point Pricing");
		titleHTML.setSize(this.C_BASE_WIDTH - 40, 30);
		titleHTML.setPreferredSize(this.C_BASE_WIDTH - 40, 30);
		titleHTML.setFont(::GUI.Font("Maiandra", 28, false));
		titleHTML.setFontColor(this.Colors.white);
		titleHTML.getLayoutManager().setAlignment("center");
		this.add(titleHTML);
		local abilityDescHTML = this.GUI.HTML("Your first Ability Points cost is 2 Credits.  Each additional Ability Point will cost 2 Credits more than" + " the previous Ability Point.  This continues until the cost of your next Ability Point is 100 Credits.  Each additional" + " Ability Point purchased will then cost 100 Credits.");
		abilityDescHTML.setSize(this.C_BASE_WIDTH - 40, 75);
		abilityDescHTML.setPreferredSize(this.C_BASE_WIDTH - 40, 75);
		abilityDescHTML.setFont(::GUI.Font("Maiandra", 28, false));
		abilityDescHTML.setFontColor(this.Colors["Soft Grey"]);
		abilityDescHTML.getLayoutManager().setGaps(0, -5);
		this.add(abilityDescHTML);
		local bottomComp = this._buildSquareComp();
		this.add(bottomComp);
		this.setOverlay(this.GUI.OVERLAY);
	}

	function _buildSquareComp()
	{
		local baseComp = this.GUI.Component(this.GUI.BoxLayout());
		baseComp.setSize(this.C_BASE_WIDTH - 20, 85);
		baseComp.setPreferredSize(this.C_BASE_WIDTH - 20, 85);
		baseComp.getLayoutManager().setGap(7);

		foreach( data in this.mAbilityPointData )
		{
			if (data.type == "square")
			{
				local imageSquare = this.GUI.Component(this.GUI.BoxLayoutV());
				imageSquare.setSize(124, 67);
				imageSquare.setPreferredSize(124, 67);
				imageSquare.setInsets(10, 0, 0, 0);
				imageSquare.setAppearance("PurchaseAbilityGreySquare");
				baseComp.add(imageSquare);
				local squareHTML = this.GUI.HTML("<font size=\"26\" font color=\"" + this.Colors.white + "\">Ability Point " + data.point + "<br></font><font size=\"24\"Cost: </font>" + "<font size=\"24\" font color=\"" + this.Colors.mint + "\">" + data.cost + " Credits</font>");
				squareHTML.setSize(124, 67);
				squareHTML.setPreferredSize(124, 67);
				squareHTML.setFont(::GUI.Font("Maiandra", 24, false));
				squareHTML.getLayoutManager().setGaps(0, -5);
				squareHTML.getLayoutManager().setAlignment("center");
				imageSquare.add(squareHTML);
			}
			else if (data.type == "arrow")
			{
				local imageArrow = this.GUI.Component(null);
				imageArrow.setSize(88, 54);
				imageArrow.setPreferredSize(88, 54);
				imageArrow.setAppearance("PurchaseAbilityArrow");
				baseComp.add(imageArrow);
			}
		}

		return baseComp;
	}

}

