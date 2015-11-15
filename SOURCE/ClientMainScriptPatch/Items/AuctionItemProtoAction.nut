class AuctionItemProtoAction extends ItemProtoAction {

	mAuctionItem = null;
	mReceivedDataTime = null;
	remainlabel = null;

	constructor(auctionItem) {
		mReceivedDataTime = ::System.currentTimeMillis() / 1000;
		ItemProtoAction.constructor(auctionItem.proto);
		mAuctionItem = auctionItem;
	}

	function getType() {
		return "auctionItemProto";
	}
	
	function updateTime() {
		_updateRemainLabel();
	}

	function getInfoPanel( mods ) {
		local itemdef = this._getItemDef();

		if (itemdef) {
			
			if (itemdef.mValid == true) {
			
				local hideValue = false;
				if (mods && "hideValue" in mods) {
					hideValue = mods.hideValue;
				}
				
				// Seller
				local sellerlabel = GUI.Label(mAuctionItem.sellerName);
				sellerlabel.setFont(GUI.Font("Maiandra", 16, true));
				sellerlabel.setFontColor(Colors.yellow);
				
				// Remaining time
				remainlabel = GUI.Label(); 
				remainlabel.setFont(GUI.Font("Maiandra", 16));
				remainlabel.setAutoFit(true);
				_updateRemainLabel();
					
				// Bids
				local bidsLabel = mAuctionItem.bids == 0 ? GUI.Label("No bids") : GUI.Label(mAuctionItem.bids + " Bid" + (mAuctionItem.bids > 1 ? "s" : ""));
				
				if(hideValue) {
					// 196
					local leftPanel = GUI.Container(GUI.GridLayout(3, 3));
					leftPanel.getLayoutManager().setColumns(54, 70, 70);
					
					// Row 1
					leftPanel.add(itemdef._buildName(), {
						span = 3,
						anchor = GUI.GridLayout.LEFT
					});
					
					// Row 2
					local level = GUI.Label(TXT("Level") + " " + itemdef.mLevel);
					level.setFont(GUI.Font("Maiandra", 16));
					leftPanel.add(level, {
						anchor = GUI.GridLayout.LEFT
					});
					leftPanel.add(itemdef._buildType(), {
						anchor = GUI.GridLayout.RIGHT
					});
					leftPanel.add(itemdef._buildSubtype(), {
						anchor = GUI.GridLayout.RIGHT
					});
					
					// Row 3
					leftPanel.add(remainlabel, {
						anchor = GUI.GridLayout.LEFT,
						span = 2
					});
					leftPanel.add(bidsLabel, {
						anchor = GUI.GridLayout.RIGHT
					});
					
					return leftPanel;
				}
				else {				
			
					local leftPanel = GUI.Container(GUI.GridLayout(3, 4));
					leftPanel.getLayoutManager().setColumns(99, 99, 99, 105);
					
					// Row 1
					leftPanel.add(itemdef._buildName(), {
						span = 2,
						anchor = GUI.GridLayout.LEFT
					});
					leftPanel.add(sellerlabel, {
						span = 2,
						anchor = GUI.GridLayout.RIGHT
					});
		
					// Row 2
					local level = GUI.Label(TXT("Level") + " " + itemdef.mLevel);
					level.setFont(GUI.Font("Maiandra", 16));
					leftPanel.add(level, {
						anchor = GUI.GridLayout.LEFT
					});
					leftPanel.add(itemdef._buildType(), {
						anchor = GUI.GridLayout.RIGHT
					});
					leftPanel.add(itemdef._buildSubtype(), {
						anchor = GUI.GridLayout.RIGHT
					});
					leftPanel.add(remainlabel, {
						anchor = GUI.GridLayout.RIGHT
					});
		
					// Row 3
					local moneyPanel = GUI.Container(GUI.GridLayout(1, 7));
					moneyPanel.getLayoutManager().setColumns(28, 82, 58, 30, 82, 58, 58);
					moneyPanel.getLayoutManager().setGaps( 2, 0 );
					local value;	
						
					if (mAuctionItem.copper > 0 || mAuctionItem.credits > 0) {
						value = GUI.Label("Buy:");
						value.setFont(GUI.Font("Maiandra", 16));
						moneyPanel.add(value);
						if (mAuctionItem.copper > 0) {
							value = GUI.Currency();
							value.setCurrentValue(mAuctionItem.copper);
							value.setAlignment(0);
							value.setFont(GUI.Font("Maiandra", 16));
							moneyPanel.add(value);
						}
						else {
							moneyPanel.add(GUI.Spacer());
						}
						if (mAuctionItem.credits > 0) {
							value = GUI.Credits();
							value.setCurrentValue(mAuctionItem.credits);
							value.setFont(GUI.Font("Maiandra", 16));
							moneyPanel.add(value);
						}
						else {
							moneyPanel.add(GUI.Spacer());
						}
					}
					else {
						moneyPanel.add(GUI.Spacer());
						moneyPanel.add(GUI.Spacer());
						moneyPanel.add(GUI.Spacer());
					}
					
					
					if (mAuctionItem.bidCopper > 0 || mAuctionItem.bidCredits > 0) {
						value = GUI.Label("High:");
						value.setFont(GUI.Font("Maiandra", 16));
						moneyPanel.add(value);
						if (mAuctionItem.bidCopper > 0) {
							value = GUI.Currency();
							value.setCurrentValue(mAuctionItem.bidCopper);
							value.setAlignment(0);
							value.setFont(GUI.Font("Maiandra", 16));
							moneyPanel.add(value);
						}
						else {
							moneyPanel.add(GUI.Spacer());
						}
						if (mAuctionItem.bidCredits > 0) {
							value = GUI.Credits();
							value.setCurrentValue(mAuctionItem.bidCredits);
							value.setFont(GUI.Font("Maiandra", 16));
							moneyPanel.add(value);
						}
						else {
							moneyPanel.add(GUI.Spacer());
						}
					}
					else {
						moneyPanel.add(GUI.Spacer());
						moneyPanel.add(GUI.Spacer());
						moneyPanel.add(GUI.Spacer());
					}
					moneyPanel.add(bidsLabel, {
						anchor = GUI.GridLayout.RIGHT
					});
		
					leftPanel.add(moneyPanel, {
						span = 4,
						anchor = GUI.GridLayout.LEFT
					});
					
					return leftPanel;
				}
			}
			else {
				return ItemProtoAction.getInfoPanel(mods);
			}
		}
		else {
			return ItemProtoAction.getInfoPanel(mods);
		}
	}
	
	function _updateRemainLabel() {
		if(!remainlabel) {
			return;
		}
		local now = ::System.currentTimeMillis() / 1000;
		local age = now - mReceivedDataTime;
		local rem = mAuctionItem.remainingSeconds - age;
		remainlabel.setText(rem <= 0 ? "Expired" : ( Util.parseMilliToShortTimeStr(rem * 1000)));
		if(mAuctionItem.remainingSeconds < 60) 
			remainlabel.setFontColor(Colors.red);
		else if(mAuctionItem.remainingSeconds < 600) 
			remainlabel.setFontColor(Colors.orange);
		else 
			remainlabel.setFontColor(Colors.white);
	}
}

