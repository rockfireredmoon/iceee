this.QuestMarkerType <- {
	[0] = {
		iconType = "QuestTracker1",
		questId = -1,
		isSelected = false
	},
	[1] = {
		iconType = "QuestTracker2",
		questId = -1,
		isSelected = false
	},
	[2] = {
		iconType = "QuestTracker3",
		questId = -1,
		isSelected = false
	},
	[3] = {
		iconType = "QuestTracker4",
		questId = -1,
		isSelected = false
	}
};
class this.QuestManager extends this.DefaultQueryHandler
{
	mGenericQuests = null;
	mQuests = null;
	mMessageBroadcaster = this.MessageBroadcaster();
	mLastCompletedCreatureId = null;
	mTryGetNextQuest = false;
	constructor()
	{
		::_Connection.addListener(this);
		this.mGenericQuests = {};
		this.mQuests = {};
	}

	function reset()
	{
		local tracker = ::Screens.get("QuestTracker", true);
		tracker.reset();
		this.mGenericQuests = {};
		this.mQuests = {};
	}

	function getSelectedQuestMarkerType()
	{
		local questMarkerType = [];

		foreach( questMarker in this.QuestMarkerType )
		{
			if (questMarker.questId != -1)
			{
				questMarkerType.append(questMarker);
			}
		}

		return questMarkerType;
	}

	function getPlayerQuestDataById( questId )
	{
		if (!(questId in this.mQuests) || this.mQuests[questId] == 0)
		{
			local questData = ::QuestData(0, "", "", "", [], [], 0, false, 0, 0, 0, 0, null, 0);
			this.mQuests[questId] <- questData;
			this._Connection.sendQuery("quest.data", this, questId);
		}
		else
		{
		}

		return this.mQuests[questId];
	}

	function getGenericQuestDataById( questId )
	{
		if (!(questId in this.mGenericQuests))
		{
			local questData = ::QuestData(0, "", "", "", [], [], 0, false, 0, 0, 0, 0, null, 0);
			this.mGenericQuests[questId] <- questData;
			this._Connection.sendQuery("quest.genericdata", this, questId);
		}
		else
		{
			this.mMessageBroadcaster.broadcastMessage("onGenericQuestDataReceived", this.mGenericQuests[questId]);
		}

		return this.mGenericQuests[questId];
	}

	function addQuestListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeQuestListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function requestQuestList()
	{
		this.print("Request quest list");
		this._Connection.sendQuery("quest.list", this);
	}

	function _removeQuestData( questId )
	{
		if (questId in this.mQuests)
		{
			delete this.mQuests[questId];
		}
	}

	function requestQuestLeave( questId )
	{
		this._Connection.sendQuery("quest.leave", this, questId);
	}

	function requestQuestOffer( creatureId )
	{
		local questGiver = this.Screens.get("QuestGiver", true);

		if (questGiver.getCreatureId() == null)
		{
			questGiver.setCreatureId(creatureId);
			this._Connection.sendQuery("quest.getquestoffer", this, creatureId);
		}
	}

	function requestCompleteNotTurnInQuest( creatureId )
	{
		local questEnder = this.Screens.get("QuestEnder", true);

		if (questEnder.getCreatureId() == null)
		{
			questEnder.setCreatureId(creatureId);
			this._Connection.sendQuery("quest.getcompletequest", this, creatureId);
		}
	}

	function requestJoinQuest( questId, mCreatureId )
	{
		this._Connection.sendQuery("quest.join", this, questId, mCreatureId);
	}

	function requestQuestComplete( questId, mCreatureId, rewardsSelected )
	{
		local args = [
			questId,
			mCreatureId
		];

		foreach( a in rewardsSelected )
		{
			args.append(a);
		}

		this._Connection.sendQuery("quest.complete", this, args);
	}

	function tryRequestingNextQuest( creatureId )
	{
		if (this.mTryGetNextQuest && this.mLastCompletedCreatureId != null && creatureId == this.mLastCompletedCreatureId && ::_sceneObjectManager.hasCreature(this.mLastCompletedCreatureId))
		{
			local questGiver = ::_sceneObjectManager.getCreatureByID(this.mLastCompletedCreatureId);

			if (questGiver.getQuestIndicator() && questGiver.getQuestIndicator().hasValidQuest())
			{
				::_questManager.requestQuestOffer(this.mLastCompletedCreatureId);
			}

			this.mTryGetNextQuest = false;
			this.mLastCompletedCreatureId = null;
		}
	}

	function onQueryComplete( qa, rows )
	{
		local questJournal = ::Screens.get("QuestJournal", true);

		switch(qa.query)
		{
		case "quest.list":
			this.print("onQueryComplete quest list");
			questJournal.onQuestList(rows);
			break;

		case "quest.data":
			local rowLenght = rows.len();

			if (rowLenght > 0)
			{
				this._handleQuestDataReceive(rows[0]);
			}
			else
			{
				local questId = qa.args[0];
			}

			break;

		case "quest.leave":
			this.requestQuestList();
			local questId = qa.args[0];
			this._removeQuestData(questId);
			::QuestIndicator.updateCreatureIndicators();
			break;

		case "quest.genericdata":
			if (rows)
			{
				this._handleGenericQuestDataReceive(rows[0]);
			}

			break;

		case "quest.getquestoffer":
			foreach( item in rows )
			{
				local questId = item[0].tointeger();
				this.mMessageBroadcaster.broadcastMessage("onGetQuestOffer", questId);
				local questData = this.getGenericQuestDataById(questId);
			}

			break;

		case "quest.getcompletequest":
			foreach( item in rows )
			{
				local questId = item[0].tointeger();
				this.mMessageBroadcaster.broadcastMessage("onGetQuestComplete", questId);
				local questData = this.getPlayerQuestDataById(questId);
				this.mMessageBroadcaster.broadcastMessage("onQuestDataReceived", questData);
			}

			break;

		case "quest.join":
			local questId = qa.args[0];
			this.questJoined(questId);
			::_useableCreatureManager.refreshCache();
			break;

		case "quest.complete":
			this.Screens.close("QuestEnder");
			this.mLastCompletedCreatureId = qa.args[1];
			this.mTryGetNextQuest = true;
			break;
		}
	}

	function onQuestTurnedIn( questId, creatureId )
	{
		local questJournal = ::Screens.get("QuestJournal", true);
		questJournal.removeQuestMarker(questId);
		this._removeQuestData(questId);
		::Audio.playSound("Sound-CompleteQuest.ogg");
		::QuestIndicator.updateCreatureIndicators();
		::_tutorialManager.questCompleted(questId);
		this.LoadGate.AutoFetchLoadGate(this.GateTrigger.TriggerTypes.QUEST_COMPLETE, questId);
		this.requestQuestList();
		::_useableCreatureManager.refreshCache();
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
		this.log.error("" + qa.query + " timed out, re-requesting");
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error(error);

		if (qa.query == "quest.complete")
		{
			this.Screens.close("QuestEnder");
		}
		else if (qa.query == "quest.join")
		{
			this.Screens.close("QuestGiver");
		}
	}
	
	function onQuestRemoteAbandoned(questId) 
	{
		this.requestQuestList();
		this._removeQuestData(questId);
		::QuestIndicator.updateCreatureIndicators();
	}

	function _handleQuestObjectiveMessage( objective )
	{
		local objectiveTypes = [
			"Gather",
			"Kill",
			"Kill and Gather"
		];
		local objectiveMessage = [
			"Gathered",
			"Killed",
			"Killed and Gathered"
		];

		foreach( key, type in objectiveTypes )
		{
			local objectRegExp = this.regexp(type + " ([0-9]+)");
			local res = objectRegExp.search(objective);

			if (res)
			{
				local lastPartOfData = objective.slice(res.end, objective.len());
				this.IGIS.info(objectiveMessage[key] + " 1" + lastPartOfData);
				this.print("Objective " + lastPartOfData);
			}
		}
	}

	function onQuestObjectiveUpdate( questId, objective, complete, text )
	{
		if (questId in this.mQuests)
		{
			local questData = this.mQuests[questId];
			local questObjectives = questData.getObjectives();

			if (objective in questObjectives)
			{
				local currObjective = questObjectives[objective];
				currObjective.setCompleteText(text);
				currObjective.setCompleted(complete);

				if (complete)
				{
					local mapWindow = this.Screens.get("MapWindow", false);

					if (mapWindow)
					{
						mapWindow.updateQuestMarkers();
					}
				}

				this._handleQuestObjectiveMessage(currObjective.getDescription());
				this.mMessageBroadcaster.broadcastMessage("onQuestDataReceived", questData);
			}
		}

		::_tutorialManager.questObjectiveUpdated(questId, objective);
	}

	function onQuestActCompleted( questId, act )
	{
		local questTracker = ::Screens.get("QuestTracker", true);
		questTracker.clearQuestObjectives(questId);
		this._Connection.sendQuery("quest.data", this, questId);
		::_tutorialManager.questActCompleted(questId, act);
		local loadGateData = {
			questId = questId,
			actNumber = act
		};
		::LoadGate.AutoFetchLoadGate(this.GateTrigger.TriggerTypes.QUEST_ACT_COMPLETE, loadGateData);
		::QuestIndicator.updateCreatureIndicators();
	}

	function onQuestCompleted( questId )
	{
		::QuestIndicator.updateCreatureIndicators();
	}

	function onQuestJournal( questId, creatureId )
	{
		this.requestCompleteNotTurnInQuest(creatureId);
	}

	function _handleQuestDataReceiveV2( row )
	{
		local questId = row[0].tointeger();
		local title = row[1];
		local bodyText = row[2];
		local completionText = row[3];
		local level = row[4].tointeger();
		local experience = row[5].tointeger();
		local partySize = row[6].tointeger();
		local numRewards = row[7].tointeger();
		local coin = row[8].tointeger();
		local giverStringLoc = row[9];
		giverStringLoc = this.split(giverStringLoc, ",");
		local enderStringLoc = row[10];
		enderStringLoc = this.split(enderStringLoc, ",");
		local questEnderMarkerInfo;

		if (enderStringLoc.len() > 3)
		{
			questEnderMarkerInfo = {
				x = enderStringLoc[0].tointeger(),
				y = enderStringLoc[1].tointeger(),
				z = enderStringLoc[2].tointeger(),
				zoneId = enderStringLoc[3].tointeger()
			};
		}

		local objectives = [];

		for( local i = 11; i < 24; i = i + 6 )
		{
			if (row[i] != "")
			{
				local myItemId;

				if (row[i + 3] == "")
				{
					myItemId = -1;
				}
				else
				{
					myItemId = row[i + 3].tointeger();
				}

				local myCreatureDefId;

				if (row[i + 2] == "")
				{
					myCreatureDefId = -1;
				}
				else
				{
					myCreatureDefId = row[i + 2].tointeger();
				}

				local completeText = row[i + 4];
				local markerLocations = row[i + 5];
				local markers = [];

				if (markerLocations != "")
				{
					markerLocations = this.split(markerLocations, ";");

					foreach( markerLoc in markerLocations )
					{
						markerLoc = this.split(markerLoc, ",");
						local markerInfo = {
							x = markerLoc[0].tointeger(),
							y = markerLoc[1].tointeger(),
							z = markerLoc[2].tointeger(),
							zoneId = markerLoc[3].tointeger()
						};
						markers.append(markerInfo);
					}
				}

				local complete = ::Util.convertToType(row[i + 1], "bool");
				local objectivesObj = this.QuestObjectiveData(questId, row[i], completeText, complete, myItemId, myCreatureDefId, markers);
				objectives.append(objectivesObj);
			}
		}

		local rewards = [];

		for( local i = 29; i < 33; i++ )
		{
			if (row[i] != "")
			{
				local itemData = ::Util.replace(row[i], "id:", "");
				itemData = ::Util.replace(itemData, " count:", ",");
				itemData = ::Util.replace(itemData, " required:", ",");
				itemData = this.split(itemData, ",");
				local rewardData = {
					itemId = itemData[0].tointeger(),
					numItems = itemData[1].tointeger(),
					required = ::Util.atob(itemData[2])
				};
				rewards.append(rewardData);
			}
		}
		

		local questData;

		if (!(questId in this.mQuests))
		{
			questData = ::QuestData(questId, title, bodyText, "", objectives, rewards, numRewards, false, level, partySize, experience, coin, questEnderMarkerInfo, 0);
			this.mQuests[questId] <- questData;
		}
		else
		{
			questData = this.mQuests[questId];
			questData.mId = questId;
			questData.mTitle = title;
			questData.mCompletionBody = completionText;
			questData.mBody = bodyText;
			questData.mObjectives = objectives;
			questData.mRewardChoices = rewards;
			questData.mNumRewards = numRewards;
			questData.mSuggestedLevel = level;
			questData.mSuggestedPartySize = partySize;
			questData.mExperience = experience;
			questData.mCurrencyGained = coin;
			questData.mValour = 0;
			questData.mQuestEnderMarker = questEnderMarkerInfo;
		}

		this.mMessageBroadcaster.broadcastMessage("onQuestDataReceived", questData);
	}

	function _handleQuestDataReceiveV3( row )
	{
		local questId = row[0].tointeger();
		local title = row[1];
		local bodyText = row[2];
		local completionText = row[3];
		local level = row[4].tointeger();
		local experience = row[5].tointeger();
		local partySize = row[6].tointeger();
		local numRewards = row[7].tointeger();
		local coin = row[8].tointeger();
		local unabandonable = this.Util.atob(row[9]);
		local giverStringLoc = row[10];
		giverStringLoc = this.split(giverStringLoc, ",");
		local enderStringLoc = row[11];
		enderStringLoc = this.split(enderStringLoc, ",");
		local questEnderMarkerInfo;

		if (enderStringLoc.len() > 3)
		{
			questEnderMarkerInfo = {
				x = enderStringLoc[0].tointeger(),
				y = enderStringLoc[1].tointeger(),
				z = enderStringLoc[2].tointeger(),
				zoneId = enderStringLoc[3].tointeger()
			};
		}

		local objectives = [];

		for( local i = 12; i < 30; i = i + 6 )
		{
			if (row[i] != "")
			{
				local myItemId;

				if (row[i + 3] == "")
				{
					myItemId = -1;
				}
				else
				{
					myItemId = row[i + 3].tointeger();
				}

				local myCreatureDefId;

				if (row[i + 2] == "")
				{
					myCreatureDefId = -1;
				}
				else
				{
					myCreatureDefId = row[i + 2].tointeger();
				}

				local completeText = row[i + 4];
				local markerLocations = row[i + 5];
				local markers = [];

				if (markerLocations != "")
				{
					markerLocations = this.split(markerLocations, ";");

					foreach( markerLoc in markerLocations )
					{
						markerLoc = this.split(markerLoc, ",");
						local markerInfo = {
							x = markerLoc[0].tointeger(),
							y = markerLoc[1].tointeger(),
							z = markerLoc[2].tointeger(),
							zoneId = markerLoc[3].tointeger()
						};
						markers.append(markerInfo);
					}
				}

				local complete = ::Util.convertToType(row[i + 1], "bool");
				local objectivesObj = this.QuestObjectiveData(questId, row[i], completeText, complete, myItemId, myCreatureDefId, markers);
				objectives.append(objectivesObj);
			}
		}

		local rewards = [];

		for( local i = 30; i < 34; i++ )
		{
			if (row[i] != "")
			{
				local itemData = ::Util.replace(row[i], "id:", "");
				itemData = ::Util.replace(itemData, " count:", ",");
				itemData = ::Util.replace(itemData, " required:", ",");
				itemData = this.split(itemData, ",");
				local rewardData = {
					itemId = itemData[0].tointeger(),
					numItems = itemData[1].tointeger(),
					required = ::Util.atob(itemData[2])
				};
				rewards.append(rewardData);
			}
		}

		local questData;

		if (!(questId in this.mQuests))
		{
			questData = ::QuestData(questId, title, bodyText, "", objectives, rewards, numRewards, unabandonable, level, partySize, experience, coin, questEnderMarkerInfo, 0);
			this.mQuests[questId] <- questData;
		}
		else
		{
			questData = this.mQuests[questId];
			questData.mId = questId;
			questData.mTitle = title;
			questData.mUnabandonable = unabandonable;
			questData.mCompletionBody = completionText;
			questData.mBody = bodyText;
			questData.mObjectives = objectives;
			questData.mRewardChoices = rewards;
			questData.mNumRewards = numRewards;
			questData.mSuggestedLevel = level;
			questData.mSuggestedPartySize = partySize;
			questData.mExperience = experience;
			questData.mCurrencyGained = coin;
			questData.mQuestEnderMarker = questEnderMarkerInfo;
			questData.mValour = 0;
		}

		this.mMessageBroadcaster.broadcastMessage("onQuestDataReceived", questData);
	}
	
	function _handleQuestDataReceiveV4( row )
	{
		local questId = row[0].tointeger();
		local title = row[1];
		local bodyText = row[2];
		local completionText = row[3];
		local level = row[4].tointeger();
		local experience = row[5].tointeger();
		local partySize = row[6].tointeger();
		local numRewards = row[7].tointeger();
		local coin = row[8].tointeger();
		local unabandonable = this.Util.atob(row[9]);
		local valour = row[10].tointeger();
		local giverStringLoc = row[11];
		giverStringLoc = this.split(giverStringLoc, ",");
		local enderStringLoc = row[12];
		enderStringLoc = this.split(enderStringLoc, ",");
		local questEnderMarkerInfo;

		if (enderStringLoc.len() > 3)
		{
			questEnderMarkerInfo = {
				x = enderStringLoc[0].tointeger(),
				y = enderStringLoc[1].tointeger(),
				z = enderStringLoc[2].tointeger(),
				zoneId = enderStringLoc[3].tointeger()
			};
		}

		local objectives = [];

		for( local i = 13; i < 31; i = i + 6 )
		{
			if (row[i] != "")
			{
				local myItemId;

				if (row[i + 3] == "")
				{
					myItemId = -1;
				}
				else
				{
					myItemId = row[i + 3].tointeger();
				}

				local myCreatureDefId;

				if (row[i + 2] == "")
				{
					myCreatureDefId = -1;
				}
				else
				{
					myCreatureDefId = row[i + 2].tointeger();
				}

				local completeText = row[i + 4];
				local markerLocations = row[i + 5];
				local markers = [];

				if (markerLocations != "")
				{
					markerLocations = this.split(markerLocations, ";");

					foreach( markerLoc in markerLocations )
					{
						markerLoc = this.split(markerLoc, ",");
						local markerInfo = {
							x = markerLoc[0].tointeger(),
							y = markerLoc[1].tointeger(),
							z = markerLoc[2].tointeger(),
							zoneId = markerLoc[3].tointeger()
						};
						markers.append(markerInfo);
					}
				}

				local complete = ::Util.convertToType(row[i + 1], "bool");
				local objectivesObj = this.QuestObjectiveData(questId, row[i], completeText, complete, myItemId, myCreatureDefId, markers);
				objectives.append(objectivesObj);
			}
		}

		local rewards = [];

		for( local i = 31; i < 35; i++ )
		{
			if (row[i] != "")
			{
				local itemData = ::Util.replace(row[i], "id:", "");
				itemData = ::Util.replace(itemData, " count:", ",");
				itemData = ::Util.replace(itemData, " required:", ",");
				itemData = this.split(itemData, ",");
				local rewardData = {
					itemId = itemData[0].tointeger(),
					numItems = itemData[1].tointeger(),
					required = ::Util.atob(itemData[2])
				};
				rewards.append(rewardData);
			}
		}

		local questData;

		if (!(questId in this.mQuests))
		{
			questData = ::QuestData(questId, title, bodyText, "", objectives, rewards, numRewards, unabandonable, level, partySize, experience, coin, questEnderMarkerInfo, valour);
			this.mQuests[questId] <- questData;
		}
		else
		{
			questData = this.mQuests[questId];
			questData.mId = questId;
			questData.mTitle = title;
			questData.mUnabandonable = unabandonable;
			questData.mCompletionBody = completionText;
			questData.mBody = bodyText;
			questData.mObjectives = objectives;
			questData.mRewardChoices = rewards;
			questData.mNumRewards = numRewards;
			questData.mSuggestedLevel = level;
			questData.mSuggestedPartySize = partySize;
			questData.mExperience = experience;
			questData.mCurrencyGained = coin;
			questData.mQuestEnderMarker = questEnderMarkerInfo;
			questData.mValour = valour;
		}

		this.mMessageBroadcaster.broadcastMessage("onQuestDataReceived", questData);
	}

	function _handleQuestDataReceive( row )
	{
		if (row.len() == 33)
		{
			this._handleQuestDataReceiveV2(row);
			return;
		}
		else if (row.len() == 34)
		{
			this._handleQuestDataReceiveV3(row);
		}
		else if (row.len() == 35)
		{
			this._handleQuestDataReceiveV4(row);
		}
		else
		{
			throw this.Exception("No longer supported");
		}
	}

	function _handleGenericQuestDataReceiveV2( row )
	{
		local questId = row[0].tointeger();
		local title = row[1];
		local bodyText = row[2];
		local completionText = row[3];
		local level = row[4].tointeger();
		local experience = row[5].tointeger();
		local partySize = row[6].tointeger();
		local numRewards = row[7].tointeger();
		local coin = row[8].tointeger();
		local objectives = [];

		for( local i = 9; i < 16; i = i + 3 )
		{
			if (row[i] != "")
			{
				local myItemId;

				if (row[i + 2] == "")
				{
					myItemId = -1;
				}
				else
				{
					myItemId = row[i + 2].tointeger();
				}

				local objectivesObj = this.QuestObjectiveData(questId, row[i], "", ::Util.convertToType(row[i + 1], "bool"), myItemId, -1, []);
				objectives.append(objectivesObj);
			}
		}

		local rewards = [];

		for( local i = 18; i < 22; i++ )
		{
			if (row[i] != "")
			{
				local itemData = ::Util.replace(row[i], "id:", "");
				itemData = ::Util.replace(itemData, " count:", ",");
				itemData = ::Util.replace(itemData, " required:", ",");
				itemData = this.split(itemData, ",");
				local rewardData = {
					itemId = itemData[0].tointeger(),
					numItems = itemData[1].tointeger(),
					required = ::Util.atob(itemData[2])
				};
				rewards.append(rewardData);
			}
		}

		local questEnderMarkerInfo;
		local questData;

		if (!(questId in this.mGenericQuests))
		{
			questData = ::QuestData(questId, title, bodyText, completionText, objectives, rewards, numRewards, false, level, partySize, experience, coin, questEnderMarkerInfo, 0);
			this.mGenericQuests[questId] <- questData;
		}
		else
		{
			questData = this.mGenericQuests[questId];
			questData.mId = questId;
			questData.mTitle = title;
			questData.mBody = bodyText;
			questData.mCompletionBody = completionText;
			questData.mObjectives = objectives;
			questData.mRewardChoices = rewards;
			questData.mNumRewards = numRewards;
			questData.mSuggestedLevel = level;
			questData.mSuggestedPartySize = partySize;
			questData.mExperience = experience;
			questData.mCurrencyGained = coin;
			questData.mQuestEnderMarker = questEnderMarkerInfo;
			questData.mValour = 0;
		}

		this.mMessageBroadcaster.broadcastMessage("onGenericQuestDataReceived", questData);
	}

	function _handleGenericQuestDataReceiveV3( row )
	{
		local questId = row[0].tointeger();
		local title = row[1];
		local bodyText = row[2];
		local completionText = row[3];
		local level = row[4].tointeger();
		local experience = row[5].tointeger();
		local partySize = row[6].tointeger();
		local numRewards = row[7].tointeger();
		local coin = row[8].tointeger();
		local unabandonable = this.Util.atob(row[9]);
		local objectives = [];

		for( local i = 10; i < 19; i = i + 3 )
		{
			if (row[i] != "")
			{
				local myItemId;

				if (row[i + 2] == "")
				{
					myItemId = -1;
				}
				else
				{
					myItemId = row[i + 2].tointeger();
				}

				local objectivesObj = this.QuestObjectiveData(questId, row[i], "", ::Util.convertToType(row[i + 1], "bool"), myItemId, -1, []);
				objectives.append(objectivesObj);
			}
		}

		local rewards = [];

		for( local i = 19; i < 23; i++ )
		{
			if (row[i] != "")
			{
				local itemData = ::Util.replace(row[i], "id:", "");
				itemData = ::Util.replace(itemData, " count:", ",");
				itemData = ::Util.replace(itemData, " required:", ",");
				itemData = this.split(itemData, ",");
				local rewardData = {
					itemId = itemData[0].tointeger(),
					numItems = itemData[1].tointeger(),
					required = ::Util.atob(itemData[2])
				};
				rewards.append(rewardData);
			}
		}

		local questEnderMarkerInfo;
		local questData;

		if (!(questId in this.mGenericQuests))
		{
			questData = ::QuestData(questId, title, bodyText, completionText, objectives, rewards, numRewards, unabandonable, level, partySize, experience, coin, questEnderMarkerInfo, 0);
			this.mGenericQuests[questId] <- questData;
		}
		else
		{
			questData = this.mGenericQuests[questId];
			questData.mId = questId;
			questData.mTitle = title;
			questData.mBody = bodyText;
			questData.mCompletionBody = completionText;
			questData.mUnabandonable = unabandonable;
			questData.mObjectives = objectives;
			questData.mRewardChoices = rewards;
			questData.mNumRewards = numRewards;
			questData.mSuggestedLevel = level;
			questData.mSuggestedPartySize = partySize;
			questData.mExperience = experience;
			questData.mCurrencyGained = coin;
			questData.mQuestEnderMarker = questEnderMarkerInfo;
			questData.mValour = 0;
		}

		this.mMessageBroadcaster.broadcastMessage("onGenericQuestDataReceived", questData);
	}
	
	function _handleGenericQuestDataReceiveV4( row )
	{
		local questId = row[0].tointeger();
		local title = row[1];
		local bodyText = row[2];
		local completionText = row[3];
		local level = row[4].tointeger();
		local experience = row[5].tointeger();
		local partySize = row[6].tointeger();
		local numRewards = row[7].tointeger();
		local coin = row[8].tointeger();
		local unabandonable = this.Util.atob(row[9]);
		local valour = row[10].tointeger();
		local objectives = [];

		for( local i = 11; i < 20; i = i + 3 )
		{
			if (row[i] != "")
			{
				local myItemId;

				if (row[i + 2] == "")
				{
					myItemId = -1;
				}
				else
				{
					myItemId = row[i + 2].tointeger();
				}

				local objectivesObj = this.QuestObjectiveData(questId, row[i], "", ::Util.convertToType(row[i + 1], "bool"), myItemId, -1, []);
				objectives.append(objectivesObj);
			}
		}

		local rewards = [];

		for( local i = 20; i < 24; i++ )
		{
			if (row[i] != "")
			{
				local itemData = ::Util.replace(row[i], "id:", "");
				itemData = ::Util.replace(itemData, " count:", ",");
				itemData = ::Util.replace(itemData, " required:", ",");
				itemData = this.split(itemData, ",");
				local rewardData = {
					itemId = itemData[0].tointeger(),
					numItems = itemData[1].tointeger(),
					required = ::Util.atob(itemData[2])
				};
				rewards.append(rewardData);
			}
		}

		local questEnderMarkerInfo;
		local questData;

		if (!(questId in this.mGenericQuests))
		{
			questData = ::QuestData(questId, title, bodyText, completionText, objectives, rewards, numRewards, unabandonable, level, partySize, experience, coin, questEnderMarkerInfo, valour);
			this.mGenericQuests[questId] <- questData;
		}
		else
		{
			questData = this.mGenericQuests[questId];
			questData.mId = questId;
			questData.mTitle = title;
			questData.mBody = bodyText;
			questData.mCompletionBody = completionText;
			questData.mUnabandonable = unabandonable;
			questData.mObjectives = objectives;
			questData.mRewardChoices = rewards;
			questData.mNumRewards = numRewards;
			questData.mSuggestedLevel = level;
			questData.mSuggestedPartySize = partySize;
			questData.mExperience = experience;
			questData.mCurrencyGained = coin;
			questData.mQuestEnderMarker = questEnderMarkerInfo;
			questData.mValour = valour;
		}

		this.mMessageBroadcaster.broadcastMessage("onGenericQuestDataReceived", questData);
	}

	function _handleGenericQuestDataReceive( row )
	{
		if (row.len() == 22)
		{
			this._handleGenericQuestDataReceiveV2(row);
			return;
		}
		else if (row.len() == 23)
		{
			this._handleGenericQuestDataReceiveV3(row);
		}
		else if (row.len() == 24)
		{
			this._handleGenericQuestDataReceiveV4(row);
		}
		else
		{
			throw this.Exception("No longer supported");
		}
	}

	function questJoined( questId )
	{
		this.Screens.close("QuestGiver");
		local questJournal = ::Screens.get("QuestJournal", true);
		questJournal.setLastJoinedQuest(questId);
		this.requestQuestList();
		::QuestIndicator.updateCreatureIndicators();
	}

}

class this.QuestData 
{
	mId = -1;
	mTitle = null;
	mBody = null;
	mCompletionBody = null;
	mObjectives = null;
	mUnabandonable = false;
	mRewardChoices = null;
	mNumRewards = 0;
	mSuggestedLevel = 0;
	mSuggestedPartySize = 0;
	mExperience = 0;
	mValour = 0;
	mCurrencyGained = 0;
	mQuestEnderMarker = null;
	constructor( id, title, body, completionBody, objectives, rewardChoices, numRewards, unabandonable, suggestedLevel, partySize, experience, currencyGained, questEnderMarkerInfo, valour )
	{
		this.mObjectives = [];
		this.mRewardChoices = [];
		this.mUnabandonable = unabandonable;
		this.mId = id;
		this.mTitle = title;
		this.mBody = body;
		this.mCompletionBody = completionBody;
		this.mObjectives = objectives;
		this.mRewardChoices = rewardChoices;
		this.mNumRewards = numRewards;
		this.mSuggestedLevel = suggestedLevel;
		this.mSuggestedPartySize = partySize;
		this.mExperience = experience;
		this.mValour = valour;
		this.mCurrencyGained = currencyGained;
		this.mQuestEnderMarker = questEnderMarkerInfo;
	}

	function getId()
	{
		return this.mId;
	}

	function getTitle()
	{
		return this.mTitle;
	}

	function getBodyText()
	{
		return this.mBody;
	}

	function getCompletionText()
	{
		return this.mCompletionBody;
	}

	function getObjectives()
	{
		return this.mObjectives;
	}

	function getRewardChoices()
	{
		return this.mRewardChoices;
	}

	function getNumRewards()
	{
		return this.mNumRewards;
	}

	function getSuggestedLevel()
	{
		return this.mSuggestedLevel;
	}

	function getSuggestedPartySize()
	{
		return this.mSuggestedPartySize;
	}

	function getExperience()
	{
		return this.mExperience;
	}

	function getValour()
	{
		return this.mValour;
	}

	function getCurrencyGained()
	{
		return this.mCurrencyGained;
	}

	function getMaxRewardCount()
	{
		local count = 0;
		local hasCountedOptional = false;
		local choices = this.getRewardChoices();

		foreach( reward in choices )
		{
			if (!reward.required)
			{
				if (!hasCountedOptional)
				{
					hasCountedOptional = true;
					count++;
				}
			}
			else
			{
				count++;
			}
		}

		return count;
	}

	function getQuestEnderMarker()
	{
		if (!this.mQuestEnderMarker)
		{
			return null;
		}

		local marker = {
			name = "Return to Ender (" + this.mTitle + ")",
			zoneID = this.mQuestEnderMarker.zoneId,
			position = {
				x = this.mQuestEnderMarker.x,
				z = this.mQuestEnderMarker.z
			},
			iconType = ::LegendItemTypes.QUEST
		};
		return marker;
	}

	function isObjectivesComplete()
	{
		foreach( objective in this.mObjectives )
		{
			if (!objective.isCompleted())
			{
				return false;
			}
		}

		return true;
	}

	function isObjectiveComplete( number )
	{
		if (number > this.mObjectives.len() - 1)
		{
			return false;
		}

		return this.mObjectives[number].isCompleted();
	}

}

class this.QuestObjectiveData 
{
	mQuestId = -1;
	mDescription = null;
	mCompleteText = null;
	mCompleted = false;
	mItemId = -1;
	mCreatureDefId = -1;
	mMarkerLocs = null;
	constructor( questId, description, completeText, complete, itemId, creatureDefId, markerLoc )
	{
		this.mQuestId = questId;
		this.mDescription = description;
		this.mCompleteText = completeText;
		this.mCompleted = complete;
		this.mItemId = itemId;
		this.mCreatureDefId = creatureDefId;
		this.mMarkerLocs = markerLoc;
	}

	function setQuestId( questId )
	{
		this.mQuestId = questId;
	}

	function setDescription( description )
	{
		this.mDescription = description;
	}

	function setCompleteText( completeText )
	{
		this.mCompleteText = completeText;
	}

	function setCompleted( complete )
	{
		this.mCompleted = complete;
	}

	function setItemId( itemId )
	{
		this.mItemId = itemId;
	}

	function setCreatureDefId( creatureDefId )
	{
		this.mCreatureDefId = creatureDefId;
	}

	function setMarkerLocs( markerLocs )
	{
		this.mMarkerLocs = markerLocs;
	}

	function getQuestId()
	{
		return this.mQuestId;
	}

	function getDescription()
	{
		return this.mDescription;
	}

	function getCompleteText()
	{
		return this.mCompleteText;
	}

	function isCompleted()
	{
		return this.mCompleted;
	}

	function getItemId()
	{
		return this.mItemId;
	}

	function getCreatureDefId()
	{
		return this.mCreatureDefId;
	}

	function getMarkerObjects()
	{
		local markerObjects = [];

		if (this.mMarkerLocs.len() <= 0 || this.mCompleted)
		{
			return markerObjects;
		}

		local questData = ::_questManager.getPlayerQuestDataById(this.mQuestId);
		local questTitle = this.Util.trim(questData.getTitle());

		foreach( markerLoc in this.mMarkerLocs )
		{
			local marker = {
				name = this.Util.trim(this.mDescription) + "(" + questTitle + ")",
				zoneID = markerLoc.zoneId,
				position = {
					x = markerLoc.x,
					z = markerLoc.z
				},
				iconType = ::LegendItemTypes.QUEST
			};
			markerObjects.append(marker);
		}

		return markerObjects;
	}

}

