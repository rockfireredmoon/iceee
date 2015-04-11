this.require("GUI/Component");
class this.GUI.ChatBubbleManager extends this.GUI.Component
{
	constructor()
	{
		this.GUI.Component.constructor();
		this.mChatStacks = [];
		::_ChatManager.addChatListener("bubbleChat", this);
		this.log.debug("Bubble Chat Manager Initialized.");
	}

	function setAppearance( a )
	{
		this.GUI.Component.setAppearance(a);

		foreach( s in this.mChatStacks )
		{
			s.setAppearance(a + "/ChatStack");
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

	function addBubble( so, text, channel )
	{
		local bub;
		local l;

		if (so == null || text == null)
		{
			return;
		}

		local color = this.getColor(channel);
		local pos = so.getPosition();

		if (pos.x == 0 && pos.y == 0 && pos.z == 0)
		{
			return;
		}

		foreach( s in this.mChatStacks )
		{
			if (so == s.getSceneObject())
			{
				s.addBubble(so, text, color);
				return;
			}
		}

		local nst = this.GUI.ChatStack(so);
		this.mChatStacks.append(nst);
		nst.addBubble(so, text, color);
	}

	mChatStacks = null;
}

