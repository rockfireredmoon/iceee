this.require("GUI/QuestComponents");
this.require("UI/Screens");
class this.Screens.QuestGiver extends this.GUI.Frame
{
	mQuestTitleBodyComponent = null;
	mObjectiveComponent = null;
	mLevelPartyComp = null;
	mExpCostComp = null;
	mRewardComp = null;
	mQuestItemComps = [];
	mQuestId = 0;
	mCreatureId = null;
	mAcceptDecline = null;
	mCheckDistanceEvent = null;
	static C_BASE_HEIGHT = 450;
	static C_BASE_WIDTH = 415;
	static C_ICON_SIZE = 18;
	static C_QUEST_ITEM_TITLE_STR = "QuestItemTitleLabel";
	static C_MARKER_CHECKBOX_STR = "MarkerCheckBox";
	static C_INSETS_SIZE = 5;
	constructor()
	{
		this.GUI.Frame.constructor("Quest");
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
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
		borderComp.setInsets(this.C_INSETS_SIZE, 0, 0, 0);
		this.mAcceptDecline = this.GUI.QuestAcceptDecline();
		this.mAcceptDecline.addQuestListener(this);
		borderComp.add(this.mAcceptDecline, this.GUI.BorderLayout.EAST);
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

	function onQuestDecline()
	{
		this.close();
	}

	function close()
	{
		::GUI.Frame.close();
		this.mCreatureId = null;
		this.mQuestId = null;
	}

	function onQuestAccept()
	{
		::_questManager.requestJoinQuest(this.mQuestId, this.mCreatureId);
	}

	function onGetQuestOffer( questId )
	{
		this.mQuestId = questId;
	}

	function onGenericQuestDataReceived( questData )
	{
		if (this.mQuestId == questData.getId())
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
			this.mExpCostComp.updateValour(questData.getValour());
			this.mExpCostComp.updateCost(questData.getCurrencyGained());
			this.mRewardComp.updateRewardCountText(questData.getNumRewards());
			this.mRewardComp.clearRewardItems();
			local rewardChoices = questData.getRewardChoices();

			foreach( key, reward in rewardChoices )
			{
				this.mRewardComp.addRewardItem(reward.itemId, reward.numItems, reward.required);
			}

			this.mAcceptDecline.setEnabled(true);
			this.Screens.show("QuestGiver");
		}
	}

	function screenDistanceCheck()
	{
		local MAX_DISTANCE = 90;
		local questGiver = ::_sceneObjectManager.getCreatureByID(this.mCreatureId);

		if (questGiver)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), questGiver.getPosition()) > MAX_DISTANCE)
			{
				this.IGIS.error("You are too far away from the quest giver to get a quest.");
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
			this.IGIS.error("This quest giver no longer exists.");
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
			::Audio.playSound("Sound-QuestAppear.ogg");
		}
	}

	function destroy()
	{
		::_questManager.removeQuestListener(this);
		::GUI.Frame.destroy();
	}

}

