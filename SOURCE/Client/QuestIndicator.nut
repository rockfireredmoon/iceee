class this.QuestIndicator 
{
	mId = -1;
	mCreatureId = -1;
	mNode = null;
	mIndicator = null;
	mIndicatorType = "NONE";
	mNeedToUpdate = false;
	mChildNode = null;
	mBillBoardNode = null;
	mHeight = 0;
	mObject = null;
	static mIndicators = {
		GRAY_ICON = "WILL_HAVE_QUEST",
		WILL_HAVE_QUEST = "Manipulator-Quest_Soon.mesh",
		GREEN_ICON = "HAVE_QUEST",
		HAVE_QUEST = "Manipulator-Quest_Ready.mesh",
		YELLOW_ICON = "PLAYER_ON_QUEST",
		PLAYER_ON_QUEST = "Manipulator-Quest_Incomplete.mesh",
		STAR_ICON = "QUEST_COMPLETED",
		QUEST_COMPLETED = "Manipulator-Quest_Complete.mesh",
		QUEST_INTERACT = "Manipulator-Quest_Complete.mesh"
	};
	constructor( object, pId, so, ... )
	{
		this.mId = pId;
		this.mNode = so;
		this.mObject = object;

		if (vargc > 0)
		{
			local pos = vargv[0];

			if (pos)
			{
				this.mHeight = pos.y;
			}
		}
	}

	function setShowingQuestIndicator()
	{
		if (!this.mNeedToUpdate)
		{
			return;
		}

		if (!this.mChildNode && this.mNode)
		{
			this.mChildNode = this.mNode.createChildSceneNode();

			if (this.mChildNode)
			{
				this.mChildNode.setAutoTracking(true, this._camera.getParentSceneNode());
				this.mChildNode.setFixedYawAxis(true);
			}
		}

		if (this.mIndicator && this.mChildNode)
		{
			this.mChildNode.detachObject(this.mIndicator);
			this.mIndicator.destroy();
			this.mIndicator = null;
		}

		if (this.mIndicatorType != "NONE" && this.mNode)
		{
			if (this._scene.hasEntity(this.mId + "QuestIndicator"))
			{
				this.mIndicator = this._scene.getEntity(this.mId + "QuestIndicator");
			}

			if (!this.mIndicator)
			{
				this.mIndicator = this._scene.createEntity(this.mId + "QuestIndicator", this.mIndicators[this.mIndicatorType]);
			}

			this.mIndicator.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
			local yScale = this.mNode.getScale().y;
			yScale = yScale == 0 ? 1 : yScale;
			local newPos = this.Vector3(0, 0, 0);
			newPos.x = 0;
			newPos.y = this.mHeight / yScale + 7;
			newPos.z = 0;
			this.mChildNode.setPosition(newPos);
			this.mChildNode.attachObject(this.mIndicator);
		}

		this.mNeedToUpdate = false;
	}

	function hasValidQuest()
	{
		if (this.mIndicatorType == "HAVE_QUEST")
		{
			return true;
		}

		return false;
	}

	function hasCompletedNotTurnInQuest()
	{
		if (this.mIndicatorType == "QUEST_COMPLETED")
		{
			return true;
		}

		return false;
	}

	function isInteractive()
	{
		return this.mIndicatorType == "QUEST_INTERACT";
	}

	function requestQuestIndicator()
	{
		if (this.mNode != ::_avatar)
		{
			this._Connection.sendQuery("quest.indicator", this, this.mCreatureId);
		}
	}

	function onQueryComplete( qa, row )
	{
		foreach( item in row )
		{
			local resultString = item[0];

			if (resultString != this.mIndicatorType)
			{
				this.mIndicatorType = resultString;
				this.mNeedToUpdate = true;
				this.setShowingQuestIndicator();

				if (::_questManager)
				{
					::_questManager.tryRequestingNextQuest(this.mCreatureId);
				}
			}

			if (resultString == "QUEST_INTERACT")
			{
				::_useableCreatureManager.addCreatureDef(this.mCreatureId, "Q");
			}
		}
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

	function setCreatureId( id )
	{
		this.mCreatureId = id;
	}

	function getCreatureId()
	{
		return this.mCreatureId;
	}

	function destroy()
	{
		this.mId = -1;
		this.mCreatureId = -1;
		this.mIndicatorType = "NONE";
		this.mNeedToUpdate = false;

		if (this.mIndicator)
		{
			this.mChildNode.detachObject(this.mIndicator);
			this.mIndicator.destroy();
			this.mIndicator = null;
		}

		if (this.mNode && this.mChildNode)
		{
			this.mNode.removeChild(this.mChildNode);
			this.mChildNode.destroy();
			this.mChildNode = null;
		}

		this.mNode = null;
	}

	function _show()
	{
		if (this.mIndicator)
		{
			this.mChildNode.attachObject(this.mIndicator);
		}
	}

	function _hide()
	{
		if (this.mIndicator)
		{
			this.mChildNode.detachObject(this.mIndicator);
		}
	}

	static function updateCreatureIndicators()
	{
		local creatureList = ::_sceneObjectManager.getCreatures();

		foreach( creature in creatureList )
		{
			local questIndicator = creature.getQuestIndicator();

			if (questIndicator)
			{
				questIndicator.requestQuestIndicator();
				questIndicator.setShowingQuestIndicator();
			}
		}
	}

	static function indicatorsVisibility( val )
	{
		local creatureList = ::_sceneObjectManager.getCreatures();

		foreach( creature in creatureList )
		{
			local questIndicator = creature.getQuestIndicator();

			if (questIndicator)
			{
				if (val)
				{
					questIndicator._show();
				}
				else
				{
					questIndicator._hide();
				}
			}
		}
	}

}

