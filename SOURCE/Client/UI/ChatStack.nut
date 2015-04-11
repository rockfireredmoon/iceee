this.require("GUI/Component");
this.require("GUI/AnchorPanel");
this.require("UI/ChatBubble");
class this.GUI.ChatStack extends this.GUI.Component
{
	constructor( so )
	{
		this.GUI.Component.constructor();
		this.mBase = this.GUI.WorldPanel(so);
		this.mBase.update();
		this.mMessages = [];
	}

	function addBubble( so, text, textColor )
	{
		if (this.mChatBubble == null)
		{
			this.mChatBubble = this.GUI.ChatBubble(so, this.mBase);
			this.mChatBubble.setOverlay(this.GUI.POPUP_OVERLAY);
		}

		if (this.mTimer == null)
		{
			this.mTimer = ::Timer();
			this.mTimer.reset();
			this._enterFrameRelay.addListener(this);
		}

		if (this.mMessages.len() == 0)
		{
			this.mTimer.reset();
		}

		local mess = {
			originaltext = text,
			label = null,
			timestamp = 0,
			alpha = 0,
			color = textColor,
			fadein = true,
			fadeout = false
		};
		mess.label = this.GUI.Label();
		mess.label.setResize(false);
		mess.label.setFont(this.GUI.Font("Maiandra", 16));
		mess.label.setPosition(10, 5);
		mess.label.setFontColor(mess.color);
		this.mChatBubble.add(mess.label);
		mess.timestamp = this.mTimer.getMilliseconds();
		this.mMessages.append(mess);
		this.mChatBubble.fadeIn();
	}

	function _updateMessages()
	{
		local delta = ::_deltat / 1000.0;
		local curtime = this.mTimer.getMilliseconds();
		local newest = this.mMessages.len() - 1;
		local YOffset = 5;
		local nextsize = {
			width = 0,
			height = 0
		};

		for( local i = 0; i < this.mMessages.len(); i++ )
		{
			local alphachanged = false;

			if (curtime > this.mMessages[i].timestamp + (this.mDisplayTime - this.mFadeTime))
			{
				this.mMessages[i].fadeout = true;
			}

			if (this.mMessages[i].fadein)
			{
				this.mMessages[i].alpha += delta / (this.mFadeTime / 1000.0);
				alphachanged = true;

				if (this.mMessages[i].alpha > 0.99000001)
				{
					this.mMessages[i].alpha = 1;
					this.mMessages[i].fadein = false;
				}
			}

			if (this.mMessages[i].fadeout)
			{
				this.mMessages[i].alpha -= delta / (this.mFadeTime / 1000.0);
				alphachanged = true;

				if (this.mMessages[i].alpha < 0.0099999998)
				{
					this.mMessages[i].alpha = 0;
				}
			}

			if (alphachanged)
			{
				local calpha = (this.mMessages[i].alpha - 0.5) / 0.5;
				calpha = calpha > 0 ? calpha : 0;
				local color = this.mMessages[i].label.getFontColor();
				this.mMessages[i].label.setFontColor(this.Color(color.r, color.g, color.b, calpha));
			}

			local face = this.mMessages[i].label.getFont().getFullFace();
			local faceHeight = this.mMessages[i].label.getFont().getHeight();
			local result = this.Screen.wordWrap(this.mMessages[i].originaltext, face, this.mMaxBubbleWidth, faceHeight);
			local size = this.Screen.getTextMetrics(result.text, face, faceHeight);
			this.mMessages[i].label.setText(result.text);

			if (this.mMessages.len() == 1 || this.mMessages[i].fadein == false)
			{
				local yExtraOffset = result.height - faceHeight;
				local yTextOffset = YOffset + yExtraOffset / 2;
				this.mMessages[i].label.setPosition(10, yTextOffset);
			}

			if (this.mMessages.len() > 1 && (this.mMessages[i].fadeout || this.mMessages[i].fadein))
			{
				local calpha = this.mMessages[i].alpha / 0.5;
				calpha = calpha < 1 ? calpha : 1;
				YOffset -= (size.height + 7) * (1 - calpha);

				if (size.width > nextsize.width)
				{
					nextsize.width = (size.width - nextsize.width) * calpha + nextsize.width;
				}

				nextsize.height += (size.height + 7) * calpha;
			}
			else if (this.mMessages.len() > 1)
			{
				nextsize.width = nextsize.width > size.width ? nextsize.width : size.width;
				nextsize.height += size.height + 7;
			}
			else
			{
				nextsize.width = size.width;
				nextsize.height = size.height + 7;
			}

			if (this.mMessages[i].fadein == true)
			{
				this.mMessages[i].label.setPosition(10, YOffset);
			}

			YOffset += size.height + 7;
		}

		nextsize.width += 20;
		nextsize.height += 4;
		this.mChatBubble.setSize(nextsize);
	}

	function onEnterFrame()
	{
		if (!this.mTimer)
		{
			return;
		}

		this._updateMessages();

		if (this.mMessages.len() == 1)
		{
			local curtime = this.mTimer.getMilliseconds();

			if (curtime > this.mMessages[0].timestamp + (this.mDisplayTime - this.mFadeTime))
			{
				this.mChatBubble.fadeOut();
			}
		}

		if (this.mMessages.len() > 0)
		{
			local curtime = this.mTimer.getMilliseconds();

			if (curtime > this.mMessages[0].timestamp + this.mDisplayTime)
			{
				this.mChatBubble.remove(this.mMessages[0].label);
				this.mMessages.remove(0);

				if (this.mMessages.len() == 0)
				{
					this.mChatBubble.setOverlay(null);
					this.mChatBubble._removeNotify();
					this.mChatBubble.destroy();
					this.mChatBubble = null;
					this.mTimer = null;
				}
			}
		}
	}

	function getSceneObject()
	{
		return this.mBase.getAnchor().getSceneObject();
	}

	mDisplayTime = 7000;
	mBase = null;
	mMessages = null;
	mTimer = null;
	mChatBubble = null;
	mMaxBubbleWidth = 250;
	mFadeTime = 300;
	static mClassName = "ChatStack";
}

