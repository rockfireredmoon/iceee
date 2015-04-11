this.require("GUI/QuestComponents");
this.require("UI/Screens");
class this.Screens.QuestJournal extends this.GUI.Frame
{
	static mScreenName = "QuestJournal";
	mMessageBroadcaster = null;
	mQuestTitleBodyComponent = null;
	mObjectiveComponent = null;
	mLevelPartyComp = null;
	mExpCostComp = null;
	mRewardComp = null;
	mQuestItemComps = null;
	mQuestItemHighlightComps = null;
	mQuestNumLabel = null;
	mShareButton = null;
	mAbandonButton = null;
	mCurrentlySelectedQuest = -1;
	mQuestList = null;
	mLoadedList = false;
	mLastJoinedQuest = 0;
	mBootstrapQuest = null;
	static MAX_MARKERS = 4;
	static C_QUEST_LIST_WIDTH = 250;
	static C_BASE_HEIGHT = 420;
	static C_BASE_WIDTH = 665;
	static C_ICON_SIZE = 18;
	static C_MAX_QUESTS = 15;
	static C_QUEST_ITEM_TITLE_STR = "QuestItemTitleLabel";
	static C_QUEST_PARTY_SIZE_STR = "QuestItemPartySize";
	static C_MARKER_CHECKBOX_STR = "MarkerCheckBox";
	static C_INSETS_SIZE = 5;
	constructor()
	{
		this.GUI.Frame.constructor("Quest Journal");
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.mQuestList = [];
		this.mQuestItemComps = [];
		this.mQuestItemHighlightComps = [];
		this.mLoadedList = false;
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this._exitFrameRelay.addListener(this);
		::_questManager.addQuestListener(this);
		local baseComponent = this.GUI.Container(this.GUI.GridLayout(1, 2));
		baseComponent.getLayoutManager().setRows("*");
		baseComponent.getLayoutManager().setColumns(this.C_QUEST_LIST_WIDTH, "*");
		baseComponent.setInsets(this.C_INSETS_SIZE, this.C_INSETS_SIZE, this.C_INSETS_SIZE, this.C_INSETS_SIZE);
		baseComponent.getLayoutManager().setGaps(this.C_INSETS_SIZE, 0);
		local questListComponent = this._buildQuestList();
		local rightComp = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mQuestTitleBodyComponent = this.GUI.QuestTitleBodyComponent("", "");
		rightComp.getLayoutManager().setGap(-2);
		rightComp.add(this.mQuestTitleBodyComponent);
		this.mObjectiveComponent = this.GUI.QuestObjectives();
		rightComp.add(this.mObjectiveComponent);
		local bottomComp = this.GUI.Component(this.GUI.BoxLayout());
		bottomComp.getLayoutManager().setGap(-2);
		this.mRewardComp = this.GUI.QuestRewards();
		bottomComp.add(this.mRewardComp);
		local bottomRightComp = this.GUI.Component(this.GUI.BoxLayoutV());
		bottomRightComp.getLayoutManager().setAlignment(0.0);
		bottomRightComp.getLayoutManager().setGap(-2);
		this.mLevelPartyComp = this.GUI.QuestLevelPartySize();
		bottomRightComp.add(this.mLevelPartyComp);
		this.mExpCostComp = this.GUI.QuestExpGold();
		bottomRightComp.add(this.mExpCostComp);
		bottomComp.add(bottomRightComp);
		rightComp.add(bottomComp);
		baseComponent.add(questListComponent);
		baseComponent.add(rightComp);
		this.setContentPane(baseComponent);
		this.centerOnScreen();

		if (::_avatar)
		{
			::_questManager.requestQuestList();
		}

		this.setCached(::Pref.get("video.UICache"));
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function onExitFrame()
	{
		if (::_avatar && !this.mLoadedList)
		{
			::_questManager.requestQuestList();
			::_exitFrameRelay.removeListener(this);
		}
	}

	function _buildQuestList()
	{
		local questListComponent = this.GUI.InnerPanel(this.GUI.BoxLayoutV());
		questListComponent.setSize(this.C_QUEST_LIST_WIDTH, this.C_BASE_HEIGHT - 30);
		questListComponent.setPreferredSize(this.C_QUEST_LIST_WIDTH, this.C_BASE_HEIGHT - 30);
		local topTitleComponent = this.GUI.Component(this.GUI.BorderLayout());
		topTitleComponent.setSize(this.C_QUEST_LIST_WIDTH - 20, 30);
		topTitleComponent.setPreferredSize(this.C_QUEST_LIST_WIDTH - 20, 30);
		questListComponent.add(topTitleComponent);
		local titleLabel = this.GUI.Label("Quest Title");
		titleLabel.setFont(this.GUI.Font("Maiandra", 20));
		titleLabel.setTextAlignment(0.5, 0.5);
		topTitleComponent.add(titleLabel, this.GUI.BorderLayout.CENTER);
		local iconComp = this.GUI.Component(this.GUI.BoxLayout());
		topTitleComponent.add(iconComp, this.GUI.BorderLayout.EAST);
		local trackingIcon = this.GUI.Component(null);
		trackingIcon.setAppearance("TrackerIcon");
		trackingIcon.setSize(this.C_ICON_SIZE, this.C_ICON_SIZE);
		trackingIcon.setPreferredSize(this.C_ICON_SIZE, this.C_ICON_SIZE);
		trackingIcon.setTooltip("Track quest");
		iconComp.add(trackingIcon);
		local partyIcon = this.GUI.Component(null);
		partyIcon.setAppearance("PartyIcon");
		partyIcon.setTooltip("Suggested party size");
		partyIcon.setSize(this.C_ICON_SIZE, this.C_ICON_SIZE);
		partyIcon.setPreferredSize(this.C_ICON_SIZE, this.C_ICON_SIZE);
		iconComp.add(partyIcon);
		local questComponentHolder = this.GUI.Component(this.GUI.BoxLayoutV());
		local extraPadding = 30;
		questComponentHolder.setSize(this.C_QUEST_LIST_WIDTH, this.C_ICON_SIZE * this.C_MAX_QUESTS + extraPadding);
		questComponentHolder.setPreferredSize(this.C_QUEST_LIST_WIDTH, this.C_ICON_SIZE * this.C_MAX_QUESTS + extraPadding);

		for( local i = 0; i < this.C_MAX_QUESTS; i++ )
		{
			local questItem = this._buildQuestListItem(i);
			questItem.setVisible(false);
			this.mQuestItemComps.append(questItem);
			questComponentHolder.add(questItem);
		}

		questListComponent.add(questComponentHolder);
		local bottomComponent = this.GUI.Component(this.GUI.GridLayout(1, 2));
		bottomComponent.getLayoutManager().setRows("*");
		bottomComponent.getLayoutManager().setColumns(this.C_QUEST_LIST_WIDTH - 120 - 40, 145);
		this.mQuestNumLabel = this.GUI.Label("Quests /15");
		bottomComponent.add(this.mQuestNumLabel);
		this._updateNumQuestListItemsLabel();
		local shareAbandonComponent = this.GUI.Component(this.GUI.BoxLayout());
		this.mAbandonButton = this.GUI.RedNarrowButton("Abandon");
		this.mAbandonButton.setFixedSize(70, 32);
		this.mAbandonButton.addActionListener(this);
		this.mAbandonButton.setReleaseMessage("_abandonQuestPress");
		shareAbandonComponent.add(this.mAbandonButton);
		this.mShareButton = this.GUI.NarrowButton("Share");
		this.mShareButton.setFixedSize(70, 32);
		this.mShareButton.addActionListener(this);
		this.mShareButton.setReleaseMessage("_shareQuestPress");
		shareAbandonComponent.add(this.mShareButton);
		local spacer = this.GUI.Spacer(10, 10);
		questListComponent.add(spacer);
		bottomComponent.add(shareAbandonComponent);
		questListComponent.add(bottomComponent);
		return questListComponent;
	}

	function _buildQuestListItem( count )
	{
		local questListItem = this.GUI.Component(this.GUI.GridLayout(1, 3));
		questListItem.getLayoutManager().setRows(this.C_ICON_SIZE);
		questListItem.getLayoutManager().setColumns(this.C_QUEST_LIST_WIDTH - this.C_ICON_SIZE * 2 - 20, this.C_ICON_SIZE, this.C_ICON_SIZE);
		local titleComp = this.GUI.Component();
		local questTitleLabel = this.GUI.Label();
		questTitleLabel.setTextAlignment(0.0, 0.5);
		questTitleLabel.setSize(this.C_QUEST_LIST_WIDTH - (this.C_ICON_SIZE * 2 + 10), this.C_ICON_SIZE);
		questTitleLabel.setPreferredSize(this.C_QUEST_LIST_WIDTH - (this.C_ICON_SIZE * 2 + 10), this.C_ICON_SIZE);
		questTitleLabel.setData(this.C_QUEST_ITEM_TITLE_STR);
		titleComp.add(questTitleLabel);
		local highlightSelection = this._createHighlightSelection(this.C_QUEST_LIST_WIDTH - (this.C_ICON_SIZE * 2 + 10), this.C_ICON_SIZE);
		highlightSelection.setData(count);
		this.mQuestItemHighlightComps.append(highlightSelection);
		titleComp.add(highlightSelection);
		questListItem.add(titleComp);
		local defaultIcon = this.GUI.CheckBox();
		defaultIcon.addActionListener(this);
		local markerData = {
			name = this.C_MARKER_CHECKBOX_STR,
			questId = -1
		};
		defaultIcon.setData(markerData);
		defaultIcon.setReleaseMessage("_selectedMarkerButton");
		local partyComp = this.GUI.Component(this.GUI.BorderLayout());
		partyComp.setSize(this.C_ICON_SIZE, this.C_ICON_SIZE);
		partyComp.setPreferredSize(this.C_ICON_SIZE, this.C_ICON_SIZE);
		partyComp.setAppearance("CheckBox");
		local partySize = this.GUI.Label("0");
		partySize.setData(this.C_QUEST_PARTY_SIZE_STR);
		partySize.setTextAlignment(0.5, 0.5);
		partyComp.add(partySize, this.GUI.BorderLayout.CENTER);
		questListItem.add(defaultIcon);
		questListItem.add(partyComp);
		return questListItem;
	}

	function _createHighlightSelection( width, height )
	{
		local r = this.GUI.ColumnListRow("ColumnList/Row", width, height, [
			0,
			0
		]);
		r.addActionListener(this);
		return r;
	}

	function clearAllSelectedQuest()
	{
		foreach( highlightComp in this.mQuestItemHighlightComps )
		{
			highlightComp.setSelectionVisible(false);
		}
	}

	function onRowSelect( row, evt )
	{
		this.clearAllSelectedQuest();
		row.setSelectionVisible(true);
		local selectedRow = row.getData();
		this.mCurrentlySelectedQuest = selectedRow;
		::Pref.set("quest.CurrentSelectedQuest", this.mCurrentlySelectedQuest);

		if (this.mCurrentlySelectedQuest != -1 && this.mCurrentlySelectedQuest < this.mQuestList.len())
		{
			this.mAbandonButton.setEnabled(false);
			this.mShareButton.setEnabled(false);
			local questData = ::_questManager.getPlayerQuestDataById(this.mQuestList[this.mCurrentlySelectedQuest].id);
			this.onQuestDataReceived(questData);
		}
	}

	function _selectedMarkerButton( button, checked )
	{
		local markerData = button.getData();

		if (!checked)
		{
			foreach( key, marker in ::QuestMarkerType )
			{
				if (markerData.questId == marker.questId)
				{
					marker.questId = -1;
					marker.isSelected = false;
					this.mMessageBroadcaster.broadcastMessage("onRemoveQuestTrackerItem", markerData.questId);
					local mapWindow = this.Screens.get("MapWindow", true);

					if (mapWindow)
					{
						mapWindow.updateQuestMarkers();
					}

					button.setChecked(false);
					local questMarkerSelectedTable = this.deepClone(::Pref.get("quest.QuestMarkerType"));
					questMarkerSelectedTable[key].questId = marker.questId;
					questMarkerSelectedTable[key].isSelected = marker.isSelected;
					::Pref.set("quest.QuestMarkerType", questMarkerSelectedTable);
				}
			}
		}
		else
		{
			foreach( key, marker in ::QuestMarkerType )
			{
				if (marker.questId == markerData.questId)
				{
					return;
				}
			}

			local markerKey = this._getFreeMarker();

			if (markerKey != null)
			{
				local currMarker = ::QuestMarkerType[markerKey];
				button.setCheckMarkAppearance(currMarker.iconType);
				button.setChecked(true);
				currMarker.isSelected = true;
				currMarker.questId = markerData.questId;
				this.mMessageBroadcaster.broadcastMessage("onAddQuestTrackerItem", currMarker.iconType, markerData.questId);
				local mapWindow = this.Screens.get("MapWindow", true);

				if (mapWindow)
				{
					mapWindow.updateQuestMarkers();
				}

				local questMarkerSelectedTable = this.deepClone(::Pref.get("quest.QuestMarkerType"));
				questMarkerSelectedTable[markerKey].questId = currMarker.questId;
				questMarkerSelectedTable[markerKey].isSelected = currMarker.isSelected;
				::Pref.set("quest.QuestMarkerType", questMarkerSelectedTable);
			}
			else
			{
				button.setChecked(false);
			}
		}
	}

	function _getFreeMarker()
	{
		foreach( key, marker in ::QuestMarkerType )
		{
			if (!marker.isSelected)
			{
				return key;
			}
		}

		return null;
	}

	function _clearInvalidQuestFromQuestTracker()
	{
		if (!this.mLoadedList)
		{
			return;
		}

		foreach( key, marker in ::QuestMarkerType )
		{
			local found = false;

			if (marker.isSelected)
			{
				foreach( index, questItem in this.mQuestList )
				{
					if (marker.questId == questItem.id)
					{
						found = true;
					}
				}

				if (!found)
				{
					this.mMessageBroadcaster.broadcastMessage("onRemoveQuestTrackerItem", marker.questId);
					marker.questId = -1;
					marker.isSelected = false;
					local mapWindow = this.Screens.get("MapWindow", true);

					if (mapWindow)
					{
						mapWindow.updateQuestMarkers();
					}

					local questMarkerSelectedTable = this.deepClone(::Pref.get("quest.QuestMarkerType"));
					questMarkerSelectedTable[key].questId = marker.questId;
					questMarkerSelectedTable[key].isSelected = marker.isSelected;
					::Pref.set("quest.QuestMarkerType", questMarkerSelectedTable);
				}
			}
		}
	}

	function _shareQuestPress( button )
	{
		if (this.mCurrentlySelectedQuest != -1 && this.mCurrentlySelectedQuest < this.mQuestList.len())
		{
			::_Connection.sendQuery("party", this, [
				"quest.invite",
				this.mQuestList[this.mCurrentlySelectedQuest].id
			]);
		}
	}

	function _abandonQuestPress( button )
	{
		local callback = {
			questJournal = this,
			function onActionSelected( mb, alt )
			{
				if (alt == "Yes")
				{
					this.questJournal._requestLeaveQuest();
				}
			}

		};

		if (this.mCurrentlySelectedQuest != -1 && this.mCurrentlySelectedQuest >= 0 && this.mCurrentlySelectedQuest < this.mQuestList.len())
		{
			this.GUI.MessageBox.showYesNo("Are you sure you want to abandon " + this.mQuestList[this.mCurrentlySelectedQuest].title + "?", callback);
		}
	}

	function _updateQuestListTitleItem( questIndex, title, partySize )
	{
		if (questIndex > this.mQuestItemComps.len())
		{
			return;
		}

		local questListItem = this.mQuestItemComps[questIndex];
		questListItem.setVisible(true);

		foreach( questItemComp in questListItem.components )
		{
			foreach( comp in questItemComp.components )
			{
				local data = comp.getData();

				if (data && data == this.C_QUEST_ITEM_TITLE_STR)
				{
					comp.setText(title);
				}

				if (data && data == this.C_QUEST_PARTY_SIZE_STR)
				{
					comp.setText(partySize);
				}
			}
		}
	}

	function _resetQuestListItems()
	{
		foreach( questItemComp in this.mQuestItemComps )
		{
			questItemComp.setVisible(false);

			foreach( item in questItemComp.components )
			{
				local compData = item.getData();

				if (compData && compData.name == this.C_MARKER_CHECKBOX_STR)
				{
					item.setChecked(false);
				}
			}
		}
	}

	function _autoTrackQuestMarker( questId )
	{
		foreach( index, questItem in this.mQuestList )
		{
			if (questItem && questId == questItem.id)
			{
				this.updateQuestListMarkerItem(index, true);
				return;
			}
		}
	}

	function removeQuestMarker( questId )
	{
		foreach( index, questItem in this.mQuestList )
		{
			if (questItem && questId == questItem.id)
			{
				this.updateQuestListMarkerItem(index, false);
				return;
			}
		}
	}

	function updateQuestListMarkerItem( questIndex, markerSelected )
	{
		if (questIndex > this.mQuestItemComps.len())
		{
			return;
		}

		local questListItem = this.mQuestItemComps[questIndex];

		foreach( questItemComp in questListItem.components )
		{
			local compData = questItemComp.getData();

			if (compData && compData.name == this.C_MARKER_CHECKBOX_STR)
			{
				questItemComp.setChecked(markerSelected);
				this._selectedMarkerButton(questItemComp, markerSelected);
				break;
			}
		}
	}

	function _setMarkerData( questIndex )
	{
		if (questIndex > this.mQuestItemComps.len())
		{
			return;
		}

		local questListItem = this.mQuestItemComps[questIndex];

		foreach( questItemComp in questListItem.components )
		{
			local compData = questItemComp.getData();

			if (compData && compData.name == this.C_MARKER_CHECKBOX_STR)
			{
				local markerData = {
					name = this.C_MARKER_CHECKBOX_STR,
					questId = this.mQuestList[questIndex].id
				};
				questItemComp.setData(markerData);
				break;
			}
		}
	}

	function _updateNumQuestListItemsLabel()
	{
		local numQuestItems = 0;

		foreach( questComp in this.mQuestItemComps )
		{
			local isChecked = questComp.isVisible();

			if (isChecked)
			{
				numQuestItems = numQuestItems + 1;
			}
		}

		this.mQuestNumLabel.setText("Quests " + numQuestItems + "/15");
	}

	function _requestLeaveQuest()
	{
		if (this.mCurrentlySelectedQuest != -1 && this.mCurrentlySelectedQuest >= 0 && this.mCurrentlySelectedQuest < this.mQuestList.len())
		{
			this.updateQuestListMarkerItem(this.mCurrentlySelectedQuest, false);
			::_questManager.requestQuestLeave(this.mQuestList[this.mCurrentlySelectedQuest].id);
			this.mCurrentlySelectedQuest = -1;
			::Pref.set("quest.CurrentSelectedQuest", this.mCurrentlySelectedQuest);
			this.clearAllSelectedQuest();
		}
	}

	function hasQuestId( questList, data )
	{
		foreach( questdata in questList )
		{
			if (questdata.id == data.id)
			{
				return true;
			}
		}

		return false;
	}

	function onQuestList( rows )
	{
		this.print("On quest list update");
		this.mLoadedList = true;
		this._resetQuestListItems();
		this.mQuestList.clear();
		this.clearQuestDataPanel();
		local count = 0;

		foreach( key, questData in rows )
		{
			local partySize = "0";

			if (questData.len() > 2)
			{
				partySize = questData[2];
			}

			local mData = {
				id = questData[0].tointeger(),
				title = questData[1],
				partySize = partySize
			};

			if (mData.id == 378)
			{
				this.mBootstrapQuest = mData.id;
			}

			if (mData.id == 648)
			{
				this.mBootstrapQuest = mData.id;
			}

			if (!this.hasQuestId(this.mQuestList, mData))
			{
				this.mQuestList.append(mData);
				this._updateQuestListTitleItem(count, mData.title, partySize);
				this._setMarkerData(count);
				count = count + 1;
			}
		}

		this._updateNumQuestListItemsLabel();

		if (this.mCurrentlySelectedQuest == -1 && this.mQuestList.len() > 0)
		{
			this.mCurrentlySelectedQuest = 0;
			this.setSelectedQuest(this.mCurrentlySelectedQuest);
		}
		else if (this.mCurrentlySelectedQuest != -1 && this.mCurrentlySelectedQuest < this.mQuestList.len())
		{
			local questData = ::_questManager.getPlayerQuestDataById(this.mQuestList[this.mCurrentlySelectedQuest].id);
			this.onQuestDataReceived(questData);
		}
		else
		{
			this.mCurrentlySelectedQuest = -1;
			this.clearAllSelectedQuest();
			this.mQuestTitleBodyComponent.setTitleText("You currently have no quests.");
		}

		this.updateCharacterSavedQuestMarkers();

		if (this.gPreferenceCharacterUpdate && this.mBootstrapQuest != null)
		{
			this._autoTrackQuestMarker(this.mBootstrapQuest);
			this.mBootstrapQuest = null;
		}

		if (this.mLastJoinedQuest != 0)
		{
			this._autoTrackQuestMarker(this.mLastJoinedQuest);
			this.mLastJoinedQuest = 0;
		}

		this._clearInvalidQuestFromQuestTracker();
	}

	function clearQuestDataPanel()
	{
		local questData = ::QuestData(0, "", "", "", [], [], 0, false, 0, 0, 0, 0, null);
		this.updateQuestDataPanel(questData);
	}

	function onQuestDataReceived( questData )
	{
		if (this.mCurrentlySelectedQuest != -1 && this.mCurrentlySelectedQuest < this.mQuestList.len() && this.mQuestList[this.mCurrentlySelectedQuest].id == questData.getId())
		{
			this.mAbandonButton.setEnabled(!questData.mUnabandonable);
			this.mShareButton.setEnabled(true);
			this.updateQuestDataPanel(questData);
		}
	}

	function updateQuestDataPanel( questData )
	{
		this.mQuestTitleBodyComponent.setTitleText(questData.getTitle());
		this.mQuestTitleBodyComponent.setBodyText(questData.getBodyText());
		this.mObjectiveComponent.clearObjectiveData();
		local objectives = questData.getObjectives();

		foreach( key, objective in objectives )
		{
			if (objective.getItemId() == -1)
			{
				this.mObjectiveComponent.updateObjectiveComponent(key, objective.getDescription(), objective.isCompleted());
			}
			else
			{
				this.mObjectiveComponent.updateObjectiveComponent(key, objective.getDescription(), objective.isCompleted(), objective.getItemId());
			}
		}

		this.mLevelPartyComp.updateLevel(questData.getSuggestedLevel());
		this.mLevelPartyComp.updatePartySize(questData.getSuggestedPartySize());
		this.mExpCostComp.updateExperience(questData.getExperience());
		this.mExpCostComp.updateCost(questData.getCurrencyGained());
		this.mRewardComp.updateRewardCountText(questData.getNumRewards());
		this.mRewardComp.clearRewardItems();
		local rewardChoices = questData.getRewardChoices();

		foreach( reward in rewardChoices )
		{
			this.mRewardComp.addRewardItem(reward.itemId, reward.numItems, reward.required);
		}
	}

	function setSelectedQuestMarkerType( value )
	{
		foreach( key, questMarkerType in value )
		{
			::QuestMarkerType[key].questId = questMarkerType.questId;
			::QuestMarkerType[key].isSelected = questMarkerType.isSelected;
		}

		this.updateCharacterSavedQuestMarkers();

		if (this.gPreferenceCharacterUpdate && this.mBootstrapQuest != null)
		{
			this._autoTrackQuestMarker(this.mBootstrapQuest);
			this.mBootstrapQuest = null;
		}

		this._clearInvalidQuestFromQuestTracker();
	}

	function _hasTrackedQuest()
	{
		foreach( key, questMarker in ::QuestMarkerType )
		{
			if (questMarker.questId != -1)
			{
				return true;
			}
		}

		return false;
	}

	function updateCharacterSavedQuestMarkers()
	{
		local found = false;

		foreach( key, questMarker in ::QuestMarkerType )
		{
			if (questMarker.questId != -1)
			{
				foreach( questListItem in this.mQuestItemComps )
				{
					foreach( questItemComp in questListItem.components )
					{
						local compData = questItemComp.getData();

						if (compData && compData.name == this.C_MARKER_CHECKBOX_STR && compData.questId == questMarker.questId)
						{
							if (questMarker.isSelected)
							{
								this._updateQuestMarkerSelected(questItemComp, questMarker);
							}
						}
					}
				}
			}
		}
	}

	function _updateQuestMarkerSelected( button, marker )
	{
		button.setCheckMarkAppearance(marker.iconType);
		button.setChecked(true);
		this.mMessageBroadcaster.broadcastMessage("onAddQuestTrackerItem", marker.iconType, marker.questId);
		local mapWindow = this.Screens.get("MapWindow", true);

		if (mapWindow)
		{
			mapWindow.updateQuestMarkers();
		}
	}

	function setSelectedQuest( value )
	{
		this.mCurrentlySelectedQuest = value;

		if (this.mCurrentlySelectedQuest != -1)
		{
			foreach( highlightComp in this.mQuestItemHighlightComps )
			{
				if (highlightComp.getData() == this.mCurrentlySelectedQuest)
				{
					this.onRowSelect(highlightComp, null);
				}
			}
		}
	}

	function setVisible( value )
	{
		this.GUI.Panel.setVisible(value);

		if (value)
		{
			::Audio.playSound("Sound-QuestLogOpen.ogg");
		}
		else
		{
			::Audio.playSound("Sound-QuestLogClose.ogg");
		}
	}

	function destroy()
	{
		::_questManager.removeQuestListener(this);
		::_exitFrameRelay.removeListener(this);
		::GUI.Frame.destroy();
	}

	function setLastJoinedQuest( questId )
	{
		this.mLastJoinedQuest = questId;
	}

}

