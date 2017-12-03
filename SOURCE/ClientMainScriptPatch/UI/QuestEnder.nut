this.require("GUI/QuestComponents");
this.require("UI/Screens");
class this.Screens.QuestEnder extends this.GUI.Frame
{
	mQuestTitleBodyComponent = null;
	mObjectiveComponent = null;
	mLevelPartyComp = null;
	mExpCostComp = null;
	mRewardComp = null;
	mQuestItemComps = [];
	mQuestId = null;
	mCreatureId = null;
	mCompleteButton = null;
	mCheckDistanceEvent = null;
	mQuestData = null;
	static C_QUEST_LIST_WIDTH = 220;
	static C_BASE_HEIGHT = 358;
	static C_BASE_WIDTH = 415;
	static C_ICON_SIZE = 18;
	static C_INSETS_SIZE = 5;
	static C_QUEST_ITEM_TITLE_STR = "QuestItemTitleLabel";
	static C_MARKER_CHECKBOX_STR = "MarkerCheckBox";
	constructor()
	{
		this.GUI.Frame.constructor("Quest");
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT + (gMaxObjectives * 32));
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT + (gMaxObjectives * 32));
		::_questManager.addQuestListener(this);
		local baseComponent = this.GUI.Container(this.GUI.GridLayout(1, 1));
		baseComponent.getLayoutManager().setRows("*");
		baseComponent.getLayoutManager().setColumns("*");
		baseComponent.setInsets(this.C_INSETS_SIZE, this.C_INSETS_SIZE, 0, this.C_INSETS_SIZE);
		local rightComp = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mQuestTitleBodyComponent = this.GUI.QuestTitleBodyComponent("", "");
		rightComp.getLayoutManager().setGap(-2);
		rightComp.add(this.mQuestTitleBodyComponent);
		this.mObjectiveComponent = this.GUI.QuestObjectives();
		rightComp.add(this.mObjectiveComponent);
		local bottomComp = this.GUI.Component(this.GUI.BoxLayout());
		bottomComp.getLayoutManager().setGap(-2);
		this.mRewardComp = this.GUI.QuestRewards();
		this.mRewardComp.setQuestCompleteContainer(true);
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
		local borderComp = this.GUI.Component(this.GUI.BorderLayout());
		borderComp.setSize(this.C_BASE_WIDTH, 42);
		borderComp.setPreferredSize(this.C_BASE_WIDTH, 42);
		borderComp.setInsets(this.C_INSETS_SIZE, this.C_INSETS_SIZE, 0, 0);
		this.mCompleteButton = this.GUI.NarrowButton("Complete");
		this.mCompleteButton.setFixedSize(90, 32);
		this.mCompleteButton.addActionListener(this);
		this.mCompleteButton.setPressMessage("_onCompletePress");
		borderComp.add(this.mCompleteButton, this.GUI.BorderLayout.EAST);
		rightComp.add(borderComp);
		baseComponent.add(rightComp);
		this.setContentPane(baseComponent);
		this.centerOnScreen();
		this.setCached(::Pref.get("video.UICache"));
	}

	function setCreatureId( creatureId )
	{
		this.mCreatureId = creatureId;
	}

	function getCreatureId()
	{
		return this.mCreatureId;
	}

	function _onCompletePress( button )
	{
		if (this.mQuestId != null)
		{
			local maxRewards = this.mQuestData.getMaxRewardCount();
			local selectedRewards = this.mRewardComp.getSelectedRewards();
			local requiredRewardCount = this.mRewardComp.getRequiredRewardCount();
			local selectedRewardsCount = selectedRewards.len() + requiredRewardCount;

			if (selectedRewardsCount < maxRewards)
			{
				local numLeftToSelect = maxRewards - selectedRewardsCount;
				local errorMessage = "Please select " + numLeftToSelect + " more ";

				if (numLeftToSelect > 1)
				{
					errorMessage += "rewards.";
				}
				else
				{
					errorMessage += "reward.";
				}

				this.IGIS.error(errorMessage);
			}
			else
			{
				::_questManager.requestQuestComplete(this.mQuestId, this.mCreatureId, selectedRewards);
				this.mCompleteButton.setEnabled(false);
			}
		}
	}

	function close()
	{
		::GUI.Frame.close();
		this.mCreatureId = null;
		this.mQuestId = null;
	}

	function onGetQuestComplete( questId )
	{
		this.mQuestId = questId;
	}

	function setQuestData( questData )
	{
		this.mQuestData = questData;
	}

	function onQuestDataReceived( questData )
	{
		if (this.mQuestId == questData.getId())
		{
			this.setQuestData(questData);
			local completionText = questData.getCompletionText();
			this.mQuestTitleBodyComponent.setTitleText(questData.getTitle());
			this.mQuestTitleBodyComponent.setBodyText(completionText != "" ? completionText : questData.getBodyText());
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
			this.mExpCostComp.updateValour(questData.getValour());
			this.mExpCostComp.updateCost(questData.getCurrencyGained());
			this.mRewardComp.updateRewardCountText(questData.getNumRewards());
			this.mRewardComp.clearRewardItems();
			local rewardChoices = questData.getRewardChoices();
			local optionalRewards = [];

			foreach( reward in questData.mRewardChoices )
			{
				this.mRewardComp.addRewardItem(reward.itemId, reward.numItems, reward.required);

				if (!reward.required)
				{
					optionalRewards.append(reward.itemId);
				}
			}

			if (optionalRewards.len() == 1)
			{
				this.mRewardComp.setRewardChecked(optionalRewards[0], true);
			}

			this.mCompleteButton.setEnabled(true);
			this.Screens.show("QuestEnder");
		}
	}

	function screenDistanceCheck()
	{
		local MAX_DISTANCE = 90;
		local questEnder = ::_sceneObjectManager.getCreatureByID(this.mCreatureId);

		if (questEnder)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), questEnder.getPosition()) > MAX_DISTANCE)
			{
				this.IGIS.error("You are too far away from the quest ender to complete the quest.");
				this.setVisible(false);
				this.mCreatureId = null;
				this.mQuestId = null;
			}
			else
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "screenDistanceCheck");
			}
		}
		else
		{
			this.IGIS.error("This quest ender no longer exists.");
			this.setVisible(false);
			this.mCreatureId = null;
			this.mQuestId = null;
		}
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (!value)
		{
			if (this.mCheckDistanceEvent)
			{
				::_eventScheduler.cancel(this.mCheckDistanceEvent);
			}
		}
		else
		{
			this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "screenDistanceCheck");
		}
	}

	function destroy()
	{
		::_questManager.removeQuestListener(this);
		::GUI.Frame.destroy();
	}

}

