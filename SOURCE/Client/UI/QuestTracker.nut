this.require("UI/Screens");
class this.Screens.QuestTracker extends this.GUI.Component
{
	mQuestTrackerItems = null;
	static MAX_QUEST_TRACKERS = 4;
	static C_BASE_WIDTH = 450;
	static C_BASE_HEIGHT = 420;
	constructor( ... )
	{
		this.mQuestTrackerItems = {};
		this.GUI.Container.constructor(this.GUI.BoxLayoutV());
		this.setInsets(0, 10, 0, 0);
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.getLayoutManager().setAlignment(1.0);
		this.setSticky("right", "top");
		local minimapScreen = this.Screens.get("MiniMapScreen", false);
		local miniMapHeight = 180;

		if (minimapScreen)
		{
			miniMapHeight = minimapScreen.getSize().height;
		}

		local windowWidth = this.getSize().width;
		this.setPosition(-windowWidth, miniMapHeight + 20);
		this.setPassThru(true);
		local emptyLabel = this.GUI.Label();
		this.add(emptyLabel);
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		this.mWidget.addListener(this);
		this.setOverlay("GUI/QuestTracker");
		this.Screen.setOverlayVisible("GUI/QuestTracker", true);
		local questJournalScreen = ::Screens.get("QuestJournal", true);

		if (questJournalScreen)
		{
			questJournalScreen.addActionListener(this);
		}

		::_questManager.addQuestListener(this);
	}

	function reset()
	{
		foreach( questId, questItem in this.mQuestTrackerItems )
		{
			local trackerItemComp = this.mQuestTrackerItems[questId];
			this.remove(trackerItemComp);
			delete this.mQuestTrackerItems[questId];
			trackerItemComp.destroy();
			trackerItemComp = null;
		}

		this.mQuestTrackerItems = {};
	}

	function _removeNotify()
	{
		local questJournalScreen = ::Screens.get("QuestJournal", true);

		if (questJournalScreen)
		{
			questJournalScreen.removeActionListener(this);
		}

		this.mWidget.removeListener(this);
		::_questManager.removeQuestListener(this);
		this.GUI.Container._removeNotify();
	}

	function onAddQuestTrackerItem( markerImage, questId )
	{
		local questTrackerItem;

		if (questId in this.mQuestTrackerItems)
		{
			questTrackerItem = this.mQuestTrackerItems[questId];
		}
		else
		{
			questTrackerItem = this.GUI.QuestTrackerItem(questId);
			this.mQuestTrackerItems[questId] <- questTrackerItem;
			this.add(questTrackerItem);
		}

		questTrackerItem.updateTrackerItem(markerImage);
	}

	function onQuestCompleted( questId )
	{
		this.onRemoveQuestTrackerItem(questId);
	}

	function onRemoveQuestTrackerItem( questId )
	{
		if (questId in this.mQuestTrackerItems)
		{
			local trackerItemComp = this.mQuestTrackerItems[questId];
			this.remove(trackerItemComp);
			delete this.mQuestTrackerItems[questId];
			trackerItemComp.destroy();
			trackerItemComp = null;
		}
	}

	function onQuestDataReceived( questData )
	{
		if (questData.getId() in this.mQuestTrackerItems)
		{
			local trackerItemComp = this.mQuestTrackerItems[questData.getId()];
			trackerItemComp.updateTrackerItem();
			local mapWindow = this.Screens.get("MapWindow", true);

			if (mapWindow)
			{
				mapWindow.updateQuestMarkers();
			}
		}
	}

	function clearQuestObjectives( questId )
	{
		if (questId in this.mQuestTrackerItems)
		{
			local trackerItemComp = this.mQuestTrackerItems[questId];
			trackerItemComp.clearObjectiveComponents();
		}
	}

}

