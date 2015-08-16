this.require("UI/UI");
class this.UI.ChatManager extends this.MessageBroadcaster
{
	mChatListeners = null;
	mMasterChatLog = null;
	mMaxEntries = 500;
	mLastTellFrom = null;
	ProfanityFilterPatternMatch = null;
	ProfanityFilterExactMatch = null;
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this.reset();
		this.ProfanityFilterPatternMatch = [];
		this.ProfanityFilterExactMatch = [];
	}

	function reset()
	{
		this.mChatListeners = {};
		this.mMasterChatLog = [];
	}

	function setProfanityFilter( value )
	{
		this.ProfanityFilterFlag = value;
	}

	function filterMessage( message )
	{
		if (this.ProfanityFilterFlag == false)
		{
			return message;
		}

		local newMessage = "";

		for( local i = 0; i < this.ProfanityFilterPatternMatch.len(); i++ )
		{
			local profanityWord = this.ProfanityFilterPatternMatch[i];
			local index = message.tolower().find(profanityWord, 0);

			while (index != null)
			{
				newMessage = message.slice(0, index) + "!@#$%";
				message = message.slice(index + profanityWord.len(), message.len());
				index = message.tolower().find(profanityWord, 0);
			}

			newMessage += message;
			message = newMessage;
			newMessage = "";
		}

		for( local i = 0; i < this.ProfanityFilterExactMatch.len(); i++ )
		{
			local profanityWord = this.ProfanityFilterExactMatch[i];
			local messageList = this.split(message, " ");
			message = "";

			for( local j = 0; j < messageList.len(); j++ )
			{
				if (messageList[j] == profanityWord)
				{
					message += "!@#$%";
				}
				else if (messageList[j] == profanityWord + "." || messageList[j] == profanityWord + "!" || messageList[j] == profanityWord + "?")
				{
					message += "!@#$%" + messageList[j].slice(messageList[j].len() - 1, messageList[j].len());
				}
				else
				{
					message += messageList[j];
				}

				message += " ";
			}
		}

		return message;
	}

	function ignorePlayer( player )
	{
		local ignoreMap = ::Pref.get("chat.ignoreList");

		if (player in ignoreMap)
		{
			return;
		}

		ignoreMap[player] <- true;
		::Pref.set("chat.ignoreList", ignoreMap);
		this.broadcastMessage("onPlayerIgnored", this, player);
	}

	function unignorePlayer( player )
	{
		local ignoreMap = ::Pref.get("chat.ignoreList");

		if (player in ignoreMap)
		{
			delete ignoreMap[player];
			::Pref.set("chat.ignoreList", ignoreMap);
			this.broadcastMessage("onPlayerUnignored", this, player);
		}
	}

	function _routeMessage( channel, message, ... )
	{
		if (channel == null || channel == "")
		{
			channel = "s";
		}

		if (message == "")
		{
			return;
		}

		local speakerName = "";
		local fullMessage = "";
		local creature;
		local exclamation = false;
		local question = false;
		message = this.filterMessage(message);
		message = message.slice(0, 1).toupper() + message.slice(1, message.len());

		while (message[0] == 32)
		{
			message = message.slice(1, message.len());
		}

		while (message[message.len() - 1] == 32)
		{
			message = message.slice(0, message.len() - 1);
		}

		if (message[message.len() - 1] == 33)
		{
			exclamation = true;
		}
		else if (message[message.len() - 1] == 63)
		{
			question = true;
		}
		else if (message[message.len() - 1] != 46)
		{
			message += ".";
		}

		if (vargc > 0)
		{
			local speakerID = vargv[0];
			speakerName = vargv[1];
			creature = ::_sceneObjectManager.getCreatureByID(speakerID);

			if (speakerName == null)
			{
				if (creature != null)
				{
					speakerName = creature.getName();
				}
			}
		}

		if (this.Util.isInIgnoreList(speakerName))
		{
			return;
		}

		if (!(channel in this.ChannelNoFilter))
		{
			if (this.Util.startsWith(channel, "yt/"))
			{
				local recipientName = channel.slice(3, channel.len());
				fullMessage += "You tell " + "<a info=\"speakerName\">" + recipientName + "</a>" + ": ";
				speakerName = recipientName;
			}
			else if (this.Util.startsWith(channel, "t/"))
			{
				this.mLastTellFrom = speakerName;
				fullMessage += "tells you: ";
			}
			else if (exclamation && speakerName != null && speakerName != "")
			{
				fullMessage += "exclaims: ";
			}
			else if (question && speakerName != null && speakerName != "")
			{
				fullMessage += "asks: ";
			}
			else if ((channel in this.ChannelHeaders) && "Consider" != this.ChannelHeaders[channel])
			{
				if (channel in this.ChannelHeaders)
				{
					fullMessage += this.ChannelHeaders[channel] + ": ";
				}
				else
				{
					fullMessage += this.ChannelHeaders.Default + ": ";
					this.IGIS.error("Channel ID " + channel);
				}
			}
			else
			{
				fullMessage += "says: ";
			}
		}

		local speakerPos;
		local avatarPos;

		if (creature)
		{
			speakerPos = creature.getPosition();
			avatarPos = ::_avatar.getPosition();
		}

		message = ::Util.convertHTMLtoText(message);
		fullMessage += message;
		local bubbleMessage = message;
		local chatMessage = "";

		if (speakerName != null && speakerName != "" && !this.Util.startsWith(channel, "yt/"))
		{
			fullMessage = "<a info=\"speakerName\">" + speakerName + "</a>" + " " + fullMessage;
			chatMessage = fullMessage;
		}
		else
		{
			chatMessage = fullMessage;
		}

		if (channel in this.ChannelBracket)
		{
			if (this.ChannelBracket[channel] != "")
			{
				chatMessage = "<a info=\"chatChannel\">" + this.ChannelBracket[channel] + "</a> " + chatMessage;
			}
		}
		else if (this.Util.startsWith(channel, "t/") || this.Util.startsWith(channel, "yt/"))
		{
			chatMessage = "[tell] " + chatMessage;
		}
		else if (this.Util.startsWith(channel, "ch/"))
		{
			local channelName = channel.slice(3, channel.len());
			chatMessage = "<a info=\"chatChannel\">[chan " + channelName + "]</a> " + chatMessage;
		}
		else if (this.Util.startsWith(channel, "gm/"))
		{
			chatMessage = "<a info=\"chatChannel\">[Earthsage]</a> " + chatMessage;
		}
		else if (this.Util.startsWith(channel, "tc/"))
		{
			chatMessage = "<a info=\"chatChannel\">[Trade]</a> " + chatMessage;
		}
		else if (this.Util.startsWith(channel, "rc/"))
		{
			chatMessage = "<a info=\"chatChannel\">[Region]</a> " + chatMessage;
		}
		else if (this.Util.startsWith(channel, "sys/"))
		{
			chatMessage = chatMessage;
		}
		else if (this.Util.startsWith(channel, "err/"))
		{
			chatMessage = chatMessage;
		}

		foreach( i, x in this.mChatListeners )
		{
			if (!(channel in x.blacklist))
			{
				if (i == "bubbleChat")
				{
					if (creature != null && ::_avatar != null)
					{
						local pos1 = ::_avatar.getPosition();
						local pos2 = creature.getPosition();
						bubbleMessage = this.Util.replace(bubbleMessage, "&amp;", "&");
						bubbleMessage = this.Util.replace(bubbleMessage, "&lt;", "<");
						bubbleMessage = this.Util.replace(bubbleMessage, "&gt;", ">");

						if (pos1 != null && pos2 != null && pos1.distance(pos2) < 200.0)
						{
							x.listener.addBubble(creature, bubbleMessage, channel);
						}
					}
				}
				else if ((channel in this.ChannelHeaders) && "Consider" == this.ChannelHeaders[channel])
				{
					x.listener.addMessage(channel, fullMessage, i, speakerName);
				}
				else
				{
					if (this.Pref.get("chat.BoldText") == true)
					{
						if (!(this.Util.startsWith(chatMessage, "<b>") && this.Util.endsWith(chatMessage, "</b>")))
						{
							chatMessage = "<b>" + chatMessage + "</b>";
						}
					}

					x.listener.addMessage(channel, chatMessage, i, speakerName);
				}
			}
		}
	}

	function addChatListener( name, listener )
	{
		if (listener == null)
		{
			return;
		}

		if (name in this.mChatListeners)
		{
			return;
		}

		this.mChatListeners[name] <- {};
		this.mChatListeners[name].listener <- listener;
		this.mChatListeners[name].blacklist <- [];
	}

	function removeChatListener( name )
	{
		if (name in this.mChatListeners)
		{
			delete this.mChatListeners[name];
		}
	}

	function _parseMessages()
	{
	}

	function _assembleListenerLog()
	{
	}

	function addMessage( channel, message, ... )
	{
		if (vargc > 0)
		{
			local speakerID = vargv[0];
			local speakerName = vargv[1];
			this.mMasterChatLog.append({
				message = message,
				channel = channel,
				speakerID = speakerID,
				speakerName = speakerName
			});
			this._routeMessage(channel, message, speakerID, speakerName);
		}
		else
		{
			this.mMasterChatLog.append({
				message = message,
				channel = channel
			});
			this._routeMessage(channel, message);
		}

		if (this.mMasterChatLog.len() > 500)
		{
			this.mMasterChatLog.pop();
		}
	}

	function getColor( channel )
	{
		if (channel in this.ChannelColors)
		{
			return this.ChannelColors[channel];
		}
		else if (this.Util.startsWith(channel, "t/") || this.Util.startsWith(channel, "yt/"))
		{
			return this.Colors.purple2;
		}
		else if (this.Util.startsWith(channel, "sys/"))
		{
			return this.Colors.yellow;
		}
		else if (this.Util.startsWith(channel, "err/"))
		{
			return this.Colors.red;
		}
		else if (this.Util.startsWith(channel, "ch/"))
		{
			return this.Colors.white;
		}
		else if (this.Util.startsWith(channel, "gm/"))
		{
			return this.Colors.coral;
		}
		else if (this.Util.startsWith(channel, "tc/"))
		{
			return this.Colors.coral;
		}
		else if (this.Util.startsWith(channel, "rc/"))
		{
			return this.Colors["Medium Grey"];
		}
		else
		{
			return this.ChannelColors.Default;
		}
	}

	function getScope( channel )
	{
		if (channel in this.ChannelScope)
		{
			return this.ChannelScope[channel];
		}
		else if (this.Util.startsWith(channel, "con"))
		{
			return "System";
		}
		else if (this.Util.startsWith(channel, "t/") || this.Util.startsWith(channel, "yt/"))
		{
			return "Tell";
		}
		else if (this.Util.startsWith(channel, "ch/"))
		{
			return "Private Channel";
		}
		else if (this.Util.startsWith(channel, "gm/"))
		{
			return "Private Channel";
		}
		else if (this.Util.startsWith(channel, "tc/"))
		{
			return "Trade";
		}
		else if (this.Util.startsWith(channel, "rc/"))
		{
			return "Region";
		}
		else if (this.Util.startsWith(channel, "sys/") || this.Util.startsWith(channel, "err/"))
		{
			return "System";
		}
		else
		{
			return "Default";
		}
	}

	function getName( channel )
	{
		if (channel in this.ChannelNames)
		{
			return this.ChannelNames[channel];
		}
		else
		{
			return this.ChannelNames.Default;
		}
	}

	function addChannelToListenerBlacklist( name, channel )
	{
		if (name in this.mChannelListeners)
		{
			if (channel in this.mChannelListeners[name].blacklist)
			{
				this.log.error("Attempted to add channel " + channel + " to chat listener " + name + " but it\'s already registered.");
			}
			else
			{
				this.mChannelListeners[name].blacklist[channel] <- true;
			}
		}
		else
		{
			this.log.error("Attempted to add a chat blacklist channel to a listener that isn\'t registered: " + this.listener);
		}
	}

	function removeChannelToListenerBlackList( name, channel )
	{
		if (name in this.mChannelListeners)
		{
			if (channel in this.mChannelListeners[name].blacklist)
			{
				delete this.mChannelListeners[name].blacklist[channel];
			}
			else
			{
				this.log.error("Attempted to remove channel " + channel + " from chat listener " + name + " but not on the list.");
			}
		}
		else
		{
			this.log.error("Attempted to remove a chat blacklist channel from a listener that isn\'t registered: " + this.listener);
		}
	}

	function getListenerLog()
	{
	}

	function getLastTellFrom()
	{
		return this.mLastTellFrom;
	}

	function onInputComplete( sender )
	{
		local text = sender.getText();
		::EvalCommand(text);
		sender.setText("");
	}

	function loadSwearWordList( swearWordIndex )
	{
		this.ProfanityFilterPatternMatch.clear();
		this.ProfanityFilterExactMatch.clear();

		if ("Dirty" in swearWordIndex)
		{
			this.ProfanityFilterPatternMatch.extend(swearWordIndex.Dirty);
		}

		if ("Exact" in swearWordIndex)
		{
			this.ProfanityFilterExactMatch.extend(swearWordIndex.Exact);
		}
	}

}

this.ProfanityFilterFlag <- true;
