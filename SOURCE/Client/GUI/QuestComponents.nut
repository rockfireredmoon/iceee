class this.GUI.QuestTitleBodyComponent extends this.GUI.Component
{
	mTitleLabel = null;
	mBodyText = null;
	mScrollPage = null;
	static C_BASE_WIDTH = 400;
	static C_BASE_HEIGHT = 185;
	static C_TITLE_HEIGHT = 28;
	static C_BUTTON_HEIGHT = 23;
	static C_INSET_SIZE = 2;
	constructor( titleText, bodyText )
	{
		local C_BODY_HEIGHT = this.C_BASE_HEIGHT - this.C_TITLE_HEIGHT;
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayout());
		this.setInsets(this.C_INSET_SIZE);
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		local baseComp = this.GUI.Component(this.GUI.GridLayout(2, 1));
		baseComp.setSize(this.C_BASE_WIDTH - this.C_INSET_SIZE * 2, this.C_BASE_HEIGHT - this.C_INSET_SIZE * 2);
		baseComp.setPreferredSize(this.C_BASE_WIDTH - this.C_INSET_SIZE * 2, this.C_BASE_HEIGHT - this.C_INSET_SIZE * 2);
		baseComp.setAppearance("PaperBackBorder");
		this.add(baseComp);
		baseComp.getLayoutManager().setRows(this.C_TITLE_HEIGHT, C_BODY_HEIGHT - this.C_INSET_SIZE * 2);
		baseComp.getLayoutManager().setColumns(this.C_BASE_WIDTH - this.C_INSET_SIZE * 2);
		local tempComp = this.GUI.Component(this.GUI.BoxLayout());
		tempComp.setInsets(0, 10, 0, 10);
		baseComp.add(tempComp);
		this.mTitleLabel = this.GUI.Label(titleText);
		this.mTitleLabel.setFontColor(this.Colors.black);
		tempComp.add(this.mTitleLabel);
		this.mTitleLabel.setFont(this.GUI.Font("Maiandra", this.C_TITLE_HEIGHT));
		local scrollBarBody = this._createBodyScrollBar(bodyText);
		baseComp.add(scrollBarBody);
	}

	function _createBodyScrollBar( bodyText )
	{
		local C_BODY_HEIGHT = this.C_BASE_HEIGHT - this.C_TITLE_HEIGHT - this.C_INSET_SIZE * 2;
		local HTML_HEIGHT = C_BODY_HEIGHT - this.C_BUTTON_HEIGHT;
		bodyText = ::Util.replace(bodyText, "\r", "<br/>");
		bodyText = ::Util.replace(bodyText, "\n", "<br/>");
		this.mBodyText = this.GUI.HTML(bodyText);
		this.mBodyText.setSize(this.C_BASE_WIDTH - 20 - this.C_INSET_SIZE * 2, HTML_HEIGHT);
		this.mBodyText.setPreferredSize(this.C_BASE_WIDTH - 20 - this.C_INSET_SIZE * 2, HTML_HEIGHT);
		this.mBodyText.setInsets(0, 10, 0, 10);
		this.mBodyText.setFontColor(this.Colors.black);
		this.mScrollPage = this.GUI.VerticalScrollPageComponent(this.mBodyText);
		this.mScrollPage.setSize(this.C_BASE_WIDTH - 20 - this.C_INSET_SIZE * 2, C_BODY_HEIGHT);
		this.mScrollPage.setPreferredSize(this.C_BASE_WIDTH - 20 - this.C_INSET_SIZE * 2, C_BODY_HEIGHT);
		this.mScrollPage.setLabelsColor(this.Colors.black);
		return this.mScrollPage;
	}

	function setBodyText( bodyText )
	{
		bodyText = ::Util.replace(bodyText, "\r", "<br/>");
		bodyText = ::Util.replace(bodyText, "\n", "<br/>");
		this.mBodyText.setText(bodyText);
		this.mBodyText.getPreferredSize();
		this.mScrollPage.refreshToFirstPage();
	}

	function setTitleText( titleText )
	{
		this.mTitleLabel.setText(titleText);
	}

}

class this.GUI.QuestObjectives extends this.GUI.Component
{
	mObjectiveStatus = null;
	mObjectiveComponents = null;
	static C_MAX_OBJECTIVES = 3;
	static C_OBJECTIVE_HEIGHT = 32;
	static C_BASE_WIDTH = 400;
	static C_BASE_HEIGHT = 140;
	static C_OBJECTIVE_LABEL_WIDTH = 400 - 60;
	static QuestButtonType = {
		DEFAULT_EMPTY_BOX = 0,
		[0] = {
			iconName = "CheckBox"
		},
		FINISHED_QUEST = 1,
		[1] = {
			iconName = "CheckBox/CheckMark"
		}
	};
	constructor( ... )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.mObjectiveStatus = [];
		this.mObjectiveComponents = [];
		local objectiveTitleLabel = this.GUI.Label("Objectives");
		objectiveTitleLabel.setFont(this.GUI.Font("Maiandra", 20));
		this.add(objectiveTitleLabel);

		for( local i = 0; i < this.C_MAX_OBJECTIVES; i++ )
		{
			local objComp = this._createObjectiveComponent(i);
			this.mObjectiveComponents.append(objComp);
			this.add(objComp);
		}
	}

	function _createObjectiveComponent( index )
	{
		local objectiveComp = this.GUI.Component(this.GUI.GridLayout(1, 3));
		objectiveComp.getLayoutManager().setRows(this.C_OBJECTIVE_HEIGHT);
		objectiveComp.getLayoutManager().setColumns(10, this.C_OBJECTIVE_LABEL_WIDTH, 50);
		objectiveComp.add(this.GUI.Spacer(10, 10));
		local objectStatusLabel = this.GUI.HTML();
		objectStatusLabel.setData("StatusLabel");
		objectStatusLabel.setTextAlignment(0.0, 0.5);
		objectiveComp.add(objectStatusLabel);
		local rightSideComp = this.GUI.Component(null);
		rightSideComp.setData("RightSideComp");
		local checkBoxComp = this.GUI.Component(null);
		checkBoxComp.setSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		checkBoxComp.setPreferredSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		local defaultIcon = this.GUI.Component(null);
		defaultIcon.setSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		defaultIcon.setPreferredSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		defaultIcon.setAppearance(this.QuestButtonType[this.QuestButtonType.DEFAULT_EMPTY_BOX].iconName);
		checkBoxComp.add(defaultIcon);
		local checkIcon = this.GUI.Component(null);
		checkIcon.setSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		checkIcon.setPreferredSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		checkIcon.setAppearance(this.QuestButtonType[this.QuestButtonType.FINISHED_QUEST].iconName);
		checkIcon.setData("CheckMarkIcon");
		checkIcon.setVisible(false);
		checkBoxComp.add(checkIcon);
		rightSideComp.add(checkBoxComp);
		local itemIconContainer = this.GUI.ActionContainer("item_objective_container_" + index, 1, 1, 0, 0, this, false);
		itemIconContainer.setSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		itemIconContainer.setPreferredSize(this.C_OBJECTIVE_HEIGHT, this.C_OBJECTIVE_HEIGHT);
		itemIconContainer.setPagingInfoEnabled(false);
		itemIconContainer.setAllButtonsDraggable(false);
		itemIconContainer.setData("ItemActionContainer");
		itemIconContainer.setVisible(false);
		rightSideComp.add(itemIconContainer);
		objectiveComp.add(rightSideComp);
		objectiveComp.setVisible(false);
		return objectiveComp;
	}

	function clearObjectiveData()
	{
		foreach( comp in this.mObjectiveComponents )
		{
			comp.setVisible(false);
		}
	}

	function updateObjectiveComponent( objectiveIndex, text, isFinished, ... )
	{
		if (objectiveIndex < 0 || objectiveIndex >= this.C_MAX_OBJECTIVES)
		{
			return;
		}

		local itemId = -1;

		if (vargc > 0)
		{
			itemId = vargv[0];
		}

		local objectiveComponent = this.mObjectiveComponents[objectiveIndex];
		objectiveComponent.setVisible(true);

		foreach( comp in objectiveComponent.components )
		{
			local data = comp.getData();

			if (data && data == "StatusLabel")
			{
				comp.setText("+ " + text);
			}
			else if (comp.getData() && comp.getData() == "RightSideComp")
			{
				foreach( iconComp in comp.components )
				{
					foreach( checkBoxComp in iconComp.components )
					{
						local iconData = checkBoxComp.getData();

						if (iconData && iconData == "CheckMarkIcon")
						{
							checkBoxComp.setVisible(isFinished);
							break;
						}
					}

					local iconCompData = iconComp.getData();

					if (iconCompData && iconCompData == "ItemActionContainer" && !isFinished && itemId != -1)
					{
						iconComp.setVisible(true);
						local item = ::_ItemManager.getItemDef(itemId);
						iconComp.removeAllActions();
						local rewardAction = iconComp.addAction(item, true);
					}
					else if (iconCompData && iconCompData == "ItemActionContainer")
					{
						iconComp.setVisible(false);
					}
				}
			}
		}
	}

}

class this.RewardAction extends this.ItemDefAction
{
	mItemId = 0;
	mNumStacks = 1;
	mItemDefData = null;
	constructor( itemId, count )
	{
		local itemDef = ::_ItemDataManager.getItemDef(itemId);
		local itemDefAct = ::ItemDefAction.constructor(itemDef.getDisplayName(), itemDef.getIcon(), itemId, itemDef);
		this.mItemId = itemId;
		this.mNumStacks = count;

		if (itemDef)
		{
			this.setImage(itemDef.mIcon);
		}

		::_ItemDataManager.addListener(this);
	}

	function onItemDefUpdated( itemDefId, itemDef )
	{
		if (itemDefId == this.mItemId)
		{
			this.mItemDefData = itemDef;

			if (itemDef)
			{
				this.mName = itemDef.mDisplayName;
				this.setImage(itemDef.mIcon);
			}
		}
	}

	function updateStackCount()
	{
		return;
	}

	function getId()
	{
		return this.mItemId;
	}

	function getNumStacks()
	{
		return this.mNumStacks;
	}

	function setStackCount( count )
	{
		this.mNumStacks = count;
	}

	function destroy()
	{
		::_ItemDataManager.removeListener(this);
	}

}

class this.RewardItem 
{
	mButton = null;
	mOptional = false;
	mCheckOrder = 0;
	constructor( button, optional )
	{
		this.mButton = button;
		this.mOptional = optional;
	}

	function getButton()
	{
		return this.mButton;
	}

	function getAction()
	{
		return this.getButton().getAction();
	}

	function getItemId()
	{
		return this.getAction().getId();
	}

	function isOptional()
	{
		return this.mOptional;
	}

	function setChecked( checked )
	{
		this.getButton().setChecked(checked);
	}

	function isChecked()
	{
		return this.getButton().isChecked();
	}

	function setCheckOrder( order )
	{
		if (order < 0)
		{
			order = 0;
		}

		this.mCheckOrder = order;

		if (this.isOptional())
		{
			this.setChecked(this.mCheckOrder > 0);
		}
	}

	function getCheckOrder()
	{
		return this.mCheckOrder;
	}

}

class this.GUI.QuestRewards extends this.GUI.Component
{
	mRewardLabel = null;
	mRewardContainer = null;
	mIsQuestCompleteContainer = false;
	mNumRewards = 0;
	mRewardItemMap = {};
	static C_BASE_WIDTH = 157;
	static C_BASE_HEIGHT = 65;
	static C_ICON_SIZE = 32;
	static MAX_REWARD_ICONS = 4;
	constructor( ... )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.mRewardLabel = this.GUI.Label("Choose N Reward(s)");
		local numRows = 1;
		local spacing = 5;
		this.mRewardContainer = this.GUI.ActionContainer("quest_reward_container", numRows, this.MAX_REWARD_ICONS, spacing, spacing, this, false);
		this.mRewardContainer.setPagingInfoEnabled(false);
		this.mRewardContainer.setAllButtonsDraggable(false);
		this.add(this.mRewardLabel);
		this.add(this.mRewardContainer);
	}

	function testCode()
	{
		this.addRewardItem(4, 2);
		this.addRewardItem(10, 1);
		this.addRewardItem(15, 14);
	}

	function getRequiredRewardCount()
	{
		local requiredCount = 0;

		foreach( reward in this.mRewardItemMap )
		{
			if (!reward.isOptional())
			{
				requiredCount++;
			}
		}

		return requiredCount;
	}

	function getSelectedRewards()
	{
		local selectedRewards = [];

		foreach( slotIndex, reward in this.mRewardItemMap )
		{
			if (reward.isOptional() && reward.isChecked())
			{
				selectedRewards.append(slotIndex);
			}
		}

		return selectedRewards;
	}

	function setQuestCompleteContainer( value )
	{
		this.mIsQuestCompleteContainer = value;
	}

	function clearRewardItems()
	{
		this.mRewardItemMap.clear();
		this.mRewardContainer.removeAllActions();
	}

	function addRewardItem( itemId, count, required )
	{
		local action = this.RewardAction(itemId, count);
		local button = this.mRewardContainer.addAction(action, true);
		local slotIndex = this.mRewardContainer.findSlotIndexOfAction(action);
		this.mRewardItemMap[slotIndex] <- this.RewardItem(button, !required);
		local actionButtonSlot = this.mRewardContainer.mActionButtonSlotList[slotIndex];
		button.setRewardCheckBoxVisible(this.mIsQuestCompleteContainer && !required);
	}

	function updateRewardCountText( numRewards )
	{
		this.mNumRewards = numRewards;
		this.mRewardLabel.setText("Choose " + this.mNumRewards + (this.mNumRewards <= 1) ? " Reward" : " Rewards");
	}

	function getNumRewards()
	{
		return this.mNumRewards;
	}

	function setRewardChecked( itemId, checked )
	{
		local modifier = 0;
		local reward = this._getRewardItem(itemId);

		if (reward.isChecked() == checked)
		{
			return;
		}

		modifier = checked ? -1 : 1;
		reward.setCheckOrder(checked ? this.getNumRewards() : 0);

		foreach( index, rewardItem in this.mRewardItemMap )
		{
			if (rewardItem.getItemId() == itemId)
			{
				continue;
			}

			local order = rewardItem.getCheckOrder();

			if (order != 0 && order + modifier <= this.getNumRewards())
			{
				rewardItem.setCheckOrder(order + modifier);
			}
		}
	}

	function _getRewardItem( itemId )
	{
		foreach( reward in this.mRewardItemMap )
		{
			if (reward.getItemId() == itemId)
			{
				return reward;
			}
		}

		return null;
	}

	function onActionButtonMouseReleased( action, rewardCheckBox )
	{
		if (!rewardCheckBox || !this.mIsQuestCompleteContainer || rewardCheckBox && !rewardCheckBox.isVisible())
		{
			return;
		}

		local slot = this.mRewardContainer.findSlotIndexOfAction(action);

		if (slot != -1)
		{
			local reward = this.mRewardItemMap[slot];
			this.setRewardChecked(reward.getItemId(), !reward.isChecked());
		}
	}

}

class this.GUI.QuestLevelPartySize extends this.GUI.Component
{
	mLevelLabel = null;
	mPartySizeLabel = null;
	static C_BASE_WIDTH = 245;
	static C_BASE_HEIGHT = 33;
	static C_INSET_SIZE = 5;
	constructor( ... )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayout());
		this.getLayoutManager().setAlignment(0.0);
		this.getLayoutManager().setGap(10);
		this.setInsets(this.C_INSET_SIZE, this.C_INSET_SIZE, this.C_INSET_SIZE, this.C_INSET_SIZE);
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.mLevelLabel = this.GUI.Label("Suggested Level: ");
		this.mPartySizeLabel = this.GUI.Label("Suggested Party Size: ");
		this.add(this.mLevelLabel);
		this.add(this.mPartySizeLabel);
	}

	function updateLevel( level )
	{
		this.mLevelLabel.setText("Suggested Level: " + level);
	}

	function updatePartySize( partySize )
	{
		this.mPartySizeLabel.setText("Suggested Party Size: " + partySize);
	}

}

class this.GUI.QuestExpGold extends this.GUI.Component
{
	mExperienceLabel = null;
	mCostCurr = null;
	static C_BASE_WIDTH = 245;
	static C_BASE_HEIGHT = 33;
	static C_INSET_SIZE = 5;
	constructor( ... )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayout());
		this.getLayoutManager().setAlignment(0.0);
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.getLayoutManager().setGap(10);
		this.setInsets(this.C_INSET_SIZE, this.C_INSET_SIZE, this.C_INSET_SIZE, this.C_INSET_SIZE);
		this.mExperienceLabel = this.GUI.Label("Experience: ");
		this.mCostCurr = this.GUI.Currency();
		this.add(this.mExperienceLabel);
		this.add(this.mCostCurr);
	}

	function updateExperience( exp )
	{
		this.mExperienceLabel.setText("Experience: " + exp);
	}

	function updateCost( value )
	{
		this.mCostCurr.setCurrentValue(value);
	}

}

class this.GUI.QuestAcceptDecline extends this.GUI.Component
{
	mMessageBroadcaster = null;
	static C_BASE_WIDTH = 150;
	static C_BASE_HEIGHT = 36;
	mAcceptButton = null;
	mDeclineButton = null;
	constructor( ... )
	{
		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mAcceptButton = this.GUI.NarrowButton("Accept");
		this.mAcceptButton.setFixedSize(70, 32);
		this.mAcceptButton.addActionListener(this);
		this.mAcceptButton.setPressMessage("_acceptPress");
		this.mDeclineButton = this.GUI.RedNarrowButton("Decline");
		this.mDeclineButton.setFixedSize(70, 32);
		this.mDeclineButton.addActionListener(this);
		this.mDeclineButton.setPressMessage("_declinePress");
		this.add(this.mAcceptButton);
		this.add(this.mDeclineButton);
	}

	function _acceptPress( button )
	{
		button.setSelectionVisible(false);
		this.mMessageBroadcaster.broadcastMessage("onQuestAccept");
		this.mAcceptButton.setEnabled(false);
		this.mDeclineButton.setEnabled(false);
	}

	function _declinePress( button )
	{
		button.setSelectionVisible(false);
		this.mMessageBroadcaster.broadcastMessage("onQuestDecline");
		this.mAcceptButton.setEnabled(false);
		this.mDeclineButton.setEnabled(false);
	}

	function addQuestListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeQuestListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function setEnabled( which )
	{
		this.mAcceptButton.setEnabled(which);
		this.mDeclineButton.setEnabled(which);
	}

}

class this.GUI.QuestTrackerItem extends this.GUI.Component
{
	mQuestId = 0;
	mMarkerImage = null;
	mTitleLabel = null;
	mType = 0;
	mObjectiveComps = null;
	static ICON_SIZE = 18;
	static COMPLETED_LABEL_COLOR = "Light Blue";
	static NOT_COMPLETED_LABEL_COLOR = "Bright Green";
	static TITLE_LABEL_COLOR = "white";
	constructor( questId )
	{
		this.mQuestId = questId;
		this.mObjectiveComps = [];
		this.GUI.Container.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setAlignment(1.0);
		this.setPassThru(true);
		local markerTitleComp = this.GUI.Container(this.GUI.BoxLayout());
		local questData = ::_questManager.getPlayerQuestDataById(this.mQuestId);
		local labelData = "";

		if (questData)
		{
			labelData = questData.getTitle();
		}

		this.mTitleLabel = this.GUI.Label(labelData);
		this.mTitleLabel.setFontColor(this.Colors[this.TITLE_LABEL_COLOR]);
		local font = ::GUI.Font("MaiandraOutline", 20);
		this.mTitleLabel.setFont(font);
		this.mTitleLabel.setAutoFit(true);
		markerTitleComp.add(this.mTitleLabel);
		local imageHolder = this.GUI.Container(null);
		imageHolder.setSize(this.ICON_SIZE, this.ICON_SIZE);
		imageHolder.setPreferredSize(this.ICON_SIZE, this.ICON_SIZE);
		markerTitleComp.add(imageHolder);
		this.mMarkerImage = this.GUI.Component();
		this.mMarkerImage.setSize(this.ICON_SIZE, this.ICON_SIZE);
		this.mMarkerImage.setPreferredSize(this.ICON_SIZE, this.ICON_SIZE);
		this.mMarkerImage.setAppearance("Icon");
		imageHolder.add(this.mMarkerImage);
		this.add(markerTitleComp);
	}

	function getQuestId()
	{
		return this.mQuestId;
	}

	function updateTrackerItem( ... )
	{
		local markerImage = "";

		if (vargc > 0)
		{
			markerImage = vargv[0];
		}

		if (markerImage != "")
		{
			this.setMarkerImage(markerImage);
		}

		local questData = ::_questManager.getPlayerQuestDataById(this.mQuestId);

		if (questData)
		{
			this.setTitle(questData.getTitle());
			local objectives = questData.getObjectives();

			for( local i = 0; i < objectives.len(); i++ )
			{
				this.updateObjectiveItem(i, objectives[i]);
			}
		}
	}

	function addObjectiveItem( objectiveData )
	{
		local objectiveComp = this.GUI.Component(this.GUI.BoxLayout());
		objectiveComp.getLayoutManager().setAlignment(0.0);
		local descriptionLabel = this.GUI.Label(objectiveData.getDescription());
		descriptionLabel.setAutoFit(true);
		local text = objectiveData.getCompleteText();
		local completeLabel = this.GUI.Label(text != "" ? "(" + text + ")" : "");
		objectiveComp.add(descriptionLabel);
		objectiveComp.add(completeLabel);
		objectiveComp.setPassThru(true);
		local font = ::GUI.Font("MaiandraOutline", 16);
		descriptionLabel.setFont(font);
		completeLabel.setFont(font);
		completeLabel.setData("CompleteLabel");
		local labelColor = this.NOT_COMPLETED_LABEL_COLOR;

		if (objectiveData.isCompleted())
		{
			labelColor = this.COMPLETED_LABEL_COLOR;
		}

		descriptionLabel.setFontColor(this.Colors[labelColor]);
		completeLabel.setFontColor(this.Colors[labelColor]);
		this.add(objectiveComp);
		this.mObjectiveComps.append(objectiveComp);
	}

	function setMarkerImage( markerImage )
	{
		this.mMarkerImage.setAppearance(markerImage);
	}

	function setTitle( titleText )
	{
		this.mTitleLabel.setText(titleText);
	}

	function updateObjectiveItem( index, objectiveData )
	{
		if (index < 0)
		{
			return;
		}

		if (index >= this.mObjectiveComps.len())
		{
			this.addObjectiveItem(objectiveData);
			return;
		}

		local objectiveComp = this.mObjectiveComps[index];
		local labelColor = this.NOT_COMPLETED_LABEL_COLOR;

		if (objectiveData.isCompleted())
		{
			labelColor = this.COMPLETED_LABEL_COLOR;
		}

		foreach( label in objectiveComp.components )
		{
			label.setFontColor(this.Colors[labelColor]);
			local data = label.getData();

			if (data && data == "CompleteLabel")
			{
				local text = objectiveData.getCompleteText();
				label.setText(text != "" ? "(" + text + ")" : "");
			}
		}
	}

	function clearObjectiveComponents()
	{
		foreach( comp in this.mObjectiveComps )
		{
			this.remove(comp);
			comp.destroy();
			comp = null;
		}

		this.mObjectiveComps.clear();
	}

}

