this.require("UI/Screens");
class this.Screens.CreatureDebugScreen extends this.GUI.Frame
{
	static mScreenName = "CreatureDebugScreen";
	mStatsPage = null;
	mDebugStats = null;
	mCurrentCreature = null;
	mCurrentTypeId = null;
	mScreenInitialized = false;
	constructor()
	{
		this.GUI.Frame.constructor("Creature Debug");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		::_Connection.addListener(this);
		::_ItemDataManager.addListener(this);
		this.setSize(500, 310);
		this.setPreferredSize(500, 310);
		this.mStatsPage = this._buildStatsPage();
		this.add(this.mStatsPage);
	}

	function destroy()
	{
		::_ItemDataManager.removeListener(this);
		::_Connection.removeListener(this);

		if (::_avatar)
		{
			::_avatar.removeListener(this);
		}

		this.GUI.Frame.destroy();
	}

	function setVisible( value )
	{
		if (value && !this.isVisible())
		{
			if (!this.mScreenInitialized)
			{
				this.mScreenInitialized = true;

				if (this._avatar)
				{
					this._avatar.addListener(this);
				}
			}

			this._sendCreatureDebugQuery();
		}

		this.GUI.Frame.setVisible(value);
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "creature.debug")
		{
			this.IGIS.error("Error attempting to retrieve creature.debug info: " + error);
		}
	}

	function _buildStatsPage()
	{
		local page = this.GUI.Container(this.GUI.BorderLayout());
		this.mDebugStats = this.GUI.ColumnList();
		this.mDebugStats.addColumn("Key", 75);
		this.mDebugStats.addColumn("Value", 100);
		page.add(this.GUI.ScrollPanel(this.mDebugStats), this.GUI.BorderLayout.CENTER);
		return page;
	}

	function _sendCreatureDebugQuery()
	{
		if (!this.mCurrentCreature)
		{
			this.mCurrentCreature = this._avatar.getTargetObject();
		}

		if (this.mCurrentCreature)
		{
			this._Connection.sendQuery("creature.debug", this, [
				this.mCurrentCreature.getID()
			]);
		}
		else if (this._avatar)
		{
			this._Connection.sendQuery("creature.debug", this, [
				this._avatar.getID()
			]);
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "creature.debug")
		{
			this.mDebugStats.removeAllRows();

			foreach( row in results )
			{
				if (this.mDebugStats)
				{
					this.mDebugStats.addRow(row);
				}
			}
		}
	}

	function onAvatarChanged( oldAvatar, avatar )
	{
		if (oldAvatar)
		{
			oldAvatar.removeListener(this);
		}

		avatar.addListener(this);
		this._sendCreatureDebugQuery();
	}

	function onTargetObjectChanged( creature, target )
	{
		this.mCurrentCreature = target;

		if (target)
		{
			this.setCurrentType(target.getType());
			this._Connection.sendQuery("creature.debug", this, [
				this.mCurrentCreature.getID()
			]);
		}
		else if (this._avatar)
		{
			this.setCurrentType(this._avatar.getType());
			this._Connection.sendQuery("creature.debug", this, [
				this._avatar.getID()
			]);
		}
		else
		{
			this.setVisible(false);
		}

		this._sendCreatureDebugQuery();
	}

	function isTweakingAvatarType()
	{
		return this._avatar && this._avatar.getType() == this.mCurrentTypeId;
	}

	function setCurrentType( id, ... )
	{
		this.log.debug("CreatureDebug.setCurrentType(" + id + ")");
		this.mCurrentTypeId = id;
		local title = "Creature Debug: ";

		if (vargc > 0)
		{
			title += "" + vargv[0] + " ";
		}

		title += "[DEF#" + id + "]";

		if (this.isTweakingAvatarType())
		{
			title += " (AVATAR)";
		}
		else if (this._avatar && this._avatar.getTargetObject() && this._avatar.getTargetObject().getType() == id)
		{
			title += " (SELECTED)";
		}

		this.setTitle(title);
	}

}

