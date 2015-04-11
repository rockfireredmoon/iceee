this.require("Relay");
this.require("GUI/FullScreenComponent");
this.require("GUI/Frame");
this.IGIS <- {
	LOW = 0,
	NORMAL = 1,
	HIGH = 2,
	ERROR = 0,
	INFO = 1,
	FLOATIE_BUFFON = 0,
	FLOATIE_BUFFOFF = 1,
	FLOATIE_DEFAULT = 2,
	FLOATIE_STATUS = 3,
	FLOATIE_STATUS_ORANGE = 4,
	FLOATIE_STATUS_ORANGE_MEDIUM = 5,
	FLOATIE_STATUS_ORANGE_BIG = 6,
	FLOATIE_STATUS_WHITE = 7,
	FLOATIE_STATUS_WHITE_MEDIUM = 8,
	FLOATIE_STATUS_WHITE_BIG = 9,
	FLOATIE_STATUS_RED = 10,
	FLOATIE_STATUS_RED_MEDIUM = 11,
	FLOATIE_STATUS_RED_BIG = 12,
	FLOATIE_STATUS_YELLOW = 13,
	FLOATIE_STATUS_YELLOW_MEDIUM = 14,
	FLOATIE_STATUS_YELLOW_BIG = 15,
	FLOATIE_STATUS_GREEN = 16,
	FLOATIE_STATUS_GREEN_MEDIUM = 17,
	FLOATIE_STATUS_GREEN_BIG = 18,
	FLOATIE_STATUS_BLUE = 19,
	FLOATIE_STATUS_BLUE_MEDIUM = 20,
	FLOATIE_STATUS_BLUE_BIG = 21,
	_ICON_FADETIME = 0.5,
	_ICON_LIFETIME = 30,
	REQUEST = 0,
	NOTIFY = 1
};
class this.IGISManager extends this.MessageBroadcaster
{
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this.Screen.setOverlayPassThru("GUI/IGISOverlay", true);
		this._screenResizeRelay.addListener(this);
		this._enterFrameRelay.addListener(this);
	}

	function onScreenResize()
	{
		foreach( t in this.mTransitoryMessages )
		{
			local pos = t.container.getPosition();
			t.container.setPosition(::Screen.getWidth() / 2 - t.container.getWidth() / 2, pos.y);
		}
	}

	function hide()
	{
		this.mRTRWindow.destroy();
		this.mRTRWindow = null;
		this.reset();
		this.mNotificationContainer.destroy();
		this.mNotificationContainer = null;
		this.mBinContainer.destroy();
		this.mBinContainer = null;
		this.mBinIcon.destroy();
		this.mBinIcon = null;
	}

	function reset()
	{
		foreach( n in this.mIcons )
		{
			this.mIcons.icon.destroy();
		}

		foreach( t in this.mTransitoryMessages )
		{
			t.container.destroy();
		}

		foreach( p in this.mPopUps )
		{
			p.destroy();
		}

		this.mTransitoryMessages = [];
		this.mNotifications = [];
		this.mPopUps = [];
		this.mIcons = [];
	}

	function _removePopUp( popup )
	{
		for( local i = 0; i < this.mPopUps.len(); i++ )
		{
			if (this.mPopUps[i] == popup)
			{
				this.mPopUps.remove(i);
				return;
			}
		}
	}

	function show()
	{
		this.mRTRWindow = this.IGIS.RTRWindow();
		this.mRTRWindow.setVisible(false);
		this.mPauseCount = 0;
		this.mNotificationContainer = ::GUI.Container(null);
		this.mNotificationContainer.setPosition(5, 5);
		this.mNotificationContainer.setSize(200, 32);
		this.mNotificationContainer.setVisible(false);
		this.mBinContainer = ::GUI.Container(null);
		this.mBinContainer.setOverlay("GUI/Overlay");
		this.mBinContainer.setSize(55, 43);
		this.mBinIcon = ::GUI.ImageButton();
		this.mBinIcon.setPressMessage("onBinPressed");
		local imageMaterialName = "IGIS/NotificationBin";
		this.mBinIcon.setImageName(imageMaterialName);
		this.mBinIcon.setGlowImageName("Glow/" + imageMaterialName);
		this.mBinIcon.addActionListener(this);
		this.mBinIcon.setSize(64, 54);
	}

	function getNotifications()
	{
		return this.mNotifications;
	}

	function onBinPressed( button )
	{
		this.mRTRWindow.buildNotificationList();
		this.mRTRWindow.setVisible(!this.mRTRWindow.isVisible());
	}

	function onEnterFrame()
	{
		this.updateNotificationIcons();
		this.updateTransitoryMessages();
		this.updateFloaties();
	}

	function updateNotificationIcons()
	{
		if (this.mIcons.len() == 0)
		{
			return;
		}

		local icons = this.mIcons;
		this.mIcons = [];

		foreach( n in icons )
		{
			n.age += this._deltat / 1000.0;

			if (n.age >= this.IGIS._ICON_LIFETIME)
			{
				this.updatePulsate();
				n.icon.destroy();
				n.icon = null;
				continue;
			}

			this.mIcons.append(n);
			local color = n.icon.getBlendColor();

			if (this.IGIS._ICON_LIFETIME - n.age <= this.IGIS._ICON_FADETIME)
			{
				color.a = 1.0 - (1.0 - (this.IGIS._ICON_LIFETIME - n.age) / this.IGIS._ICON_FADETIME);
			}
			else if (n.age < this.IGIS._ICON_FADETIME)
			{
				color.a = n.age / this.IGIS._ICON_FADETIME;
			}
			else
			{
				color.a = 1.0;
			}

			n.icon.setEnabled(color.a == 1.0);
			n.icon.setBlendColor(color);
			n.x = this.Math.GravitateValue(n.x, n.target_x, this._deltat / 1000.0, 2.5);
			n.icon.setPosition(n.x, n.icon.getPosition().y);
		}

		local x = this.mBinIcon.getWidth();
		local slide = 0;

		foreach( n in this.mIcons )
		{
			if (n.icon)
			{
				if (n.dismissed == false && this.IGIS._ICON_LIFETIME - n.age <= this.IGIS._ICON_FADETIME)
				{
					n.target_x = 0;
				}
				else
				{
					n.target_x = x;

					if (n.dismissed == false)
					{
						x += n.icon.getWidth();
					}
				}
			}
		}
	}

	function updateTransitoryMessages()
	{
		if (this.mTransitoryMessages.len() == 0)
		{
			return;
		}

		this.updateTransitoryPositions();
		local messages = this.mTransitoryMessages;
		this.mTransitoryMessages = [];

		foreach( m in messages )
		{
			local container = m.container;
			local label = m.label;
			local fadeTime = 1.0;
			local padding = 5;
			local offset = 128;
			m.age += this._deltat / 1000.0;

			if (m.age >= m.lifetime)
			{
				m.label.destroy();
				m.label = null;
				m.container.destroy();
				m.container = null;
				continue;
			}

			local color = label.getFontColor();

			if (m.lifetime - m.age <= fadeTime)
			{
				color.a = 1.0 - (1.0 - (m.lifetime - m.age) / fadeTime);
			}
			else if (m.age < fadeTime)
			{
				color.a = m.age / fadeTime;
			}
			else
			{
				color.a = 1.0;
			}

			label.setFontColor(color);
			local labelpos = container.getPosition();
			m.y = this.Math.GravitateValue(m.y, m.ydest, this._deltat / 1000.0, 2.0);
			container.setPosition(labelpos.x, m.y);
			this.mTransitoryMessages.append(m);
		}
	}

	function updateTransitoryPositions()
	{
		local padding = 10;
		local y = 128;

		foreach( t in this.mTransitoryMessages )
		{
			if (t.y == 0)
			{
				t.container.setPosition(t.container.getPosition().x, y);
				t.y = y;
			}

			t.ydest = y;
			y += t.container.getHeight() + padding;
		}
	}

	function addTransitory( message, type )
	{
		local lifetime = 5.0 + message.len().tofloat() / 20.0;
		local label = ::GUI.Label();
		label.setFont(::GUI.Font("MaiandraOutline", 24));
		label.setFontColor(type == this.IGIS.ERROR ? this.Color(1.0, 0.0, 0.0, 0.0) : this.Color(1.0, 1.0, 0.0, 0.0));
		local wrapWidth = this.Screen.getWidth() - 100;
		local face = label.getFont().getFullFace();
		local faceHeight = label.getFont().getHeight();
		local result = this.Screen.wordWrap(message, face, wrapWidth, faceHeight);
		local size = this.Screen.getTextMetrics(result.text, face, faceHeight);
		label.setText(result.text);
		label.setSize(size.width, size.height);
		local container = ::GUI.Container();
		container.setPosition(this.Screen.getWidth() / 2 - size.width / 2, 0);
		container.setSize(size.width, size.height);
		container.setOverlay("GUI/IGISOverlay");
		container.add(label);
		this.mTransitoryMessages.insert(0, {
			age = 0.0,
			lifetime = lifetime,
			container = container,
			label = label,
			y = 0,
			ydest = 0
		});
		this.broadcastMessage("onIGISTransitoryMessage", message, type);
	}

	function getIconFromNotification( id )
	{
		foreach( i in this.mIcons )
		{
			if (i.msgId == id)
			{
				return i;
			}
		}

		return null;
	}

	function hasIcon( id )
	{
		foreach( i in this.mIcons )
		{
			if (i.msgId == id)
			{
				return true;
			}
		}

		return false;
	}

	function updatePulsate()
	{
		foreach( n in this.mNotifications )
		{
			local icon = this.hasIcon(n.msgId);

			if (icon == false)
			{
				this.mBinIcon.setPulsate(true);
				return;
			}
		}

		this.mBinIcon.setPulsate(false);
	}

	function dismissIcon( id )
	{
		foreach( n in this.mIcons )
		{
			if (n.msgId == id && n.dismissed == false)
			{
				n.icon.setEnabled(false);
				n.dismissed = true;
				local time = this.IGIS._ICON_LIFETIME - this.IGIS._ICON_FADETIME;

				if (n.age < time)
				{
					n.age = time;
				}
			}
		}

		this.updatePulsate();
	}

	function dismiss( id )
	{
		local notifications = this.mNotifications;
		this.mNotifications = [];

		foreach( n in notifications )
		{
			if (n.msgId != id)
			{
				this.mNotifications.append(n);
			}
		}

		this.dismissIcon(id);
		this.broadcastMessage("onIGISNotificationDismissed", id);
	}

	function respond( id, response )
	{
		this.dismiss(id);
	}

	function onConfirmation( window, response, notification )
	{
		for( local i = 0; i < this.mPopUps.len(); i++ )
		{
			if (this.mPopUps[i] == window)
			{
				this.mPopUps.remove(i);
				break;
			}
		}

		this.dismiss(notification.msgId);
	}

	function getNotificationFromId( id )
	{
		foreach( n in this.mNotifications )
		{
			if (n.msgId == id)
			{
				return n;
			}
		}

		return null;
	}

	function getNotificationFromIcon( icon )
	{
		foreach( n in this.mIcons )
		{
			if (n.icon == icon)
			{
				return this.getNotificationFromId(n.msgId);
			}
		}

		return null;
	}

	function showPopUp( n )
	{
		local window = ::IGIS.PopUpWindow(n);
		window.addActionListener(this);
		window.setText(n.text);
		this.mPopUps.append(window);
		this.setPaused(true);
	}

	function onIconPressed( icon )
	{
		local n = this.getNotificationFromIcon(icon);

		if (n == null)
		{
			return;
		}

		this.showPopUp(n);
	}

	function addNotification( msgId, iconName, text, responses, priority, type )
	{
		local icon = ::GUI.ImageButton();
		icon.setPressMessage("onIconPressed");
		icon.addActionListener(this);
		icon.setImageName(iconName);
		icon.setGlowImageName("Glow/" + iconName);
		icon.setSize(42, 42);
		icon.setGlow(0.89999998);
		this.mNotificationContainer.add(icon);
		local x = this.mBinIcon.getWidth();

		foreach( n in this.mIcons )
		{
			if (this.IGIS._ICON_LIFETIME - n.age > this.IGIS._ICON_FADETIME)
			{
				x += n.icon.getWidth();
			}
		}

		icon.setBlendColor(this.Color(1.0, 1.0, 1.0, 0.0));
		icon.setPosition(x, 0);
		local i = {
			msgId = msgId,
			icon = icon,
			age = 0.0,
			x = x,
			target_x = x,
			dismissed = false
		};
		this.mIcons.append(i);
		local n = {
			msgId = msgId,
			text = text,
			responses = responses,
			type = type
		};
		this.mNotifications.append(n);

		if (this.mIcons.len() > 3)
		{
			for( local n = 0; n < this.mIcons.len() - 3; n++ )
			{
				local time = this.IGIS._ICON_LIFETIME - this.IGIS._ICON_FADETIME;

				if (this.mIcons[n].age < time)
				{
					this.mIcons[n].age = time;
				}
			}
		}

		if (priority > this.IGIS.LOW)
		{
			if (priority == this.IGIS.HIGH || ::Pref.get("igis.AutoPopup") == true)
			{
				this.showPopUp(n);
			}
		}

		this.updatePulsate();
		this.broadcastMessage("onIGISNotification", n);
	}

	function setFloatieVisible( visible )
	{
		this.mShowFloaties = visible;
	}

	function addFloatie( text, style, character )
	{
		if (!this.mShowFloaties)
		{
			return;
		}

		local pos;

		if (character == null)
		{
			if (::_avatar == null)
			{
				return;
			}

			pos = ::_avatar.getPosition();
			pos.y += ::_avatar.getBoundingBox().getSize().y;
		}
		else
		{
			pos = character.getPosition();
			pos.y += character.getNamePlatePosition().y;
		}

		local color;
		local size = 2.0;

		switch(style)
		{
		case this.IGIS.FLOATIE_DEFAULT:
			color = this.Color(1.0, 1.0, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_BUFFON:
			color = this.Color(0.0, 1.0, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_BUFFOFF:
			color = this.Color(1.0, 0.0, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS:
			color = this.Color(1.0, 0.0, 1.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_ORANGE:
			color = this.Color(1.0, 0.64999998, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_ORANGE_MEDIUM:
			color = this.Color(1.0, 0.64999998, 0.0, 0.0);
			size = 3.0;
			break;

		case this.IGIS.FLOATIE_STATUS_ORANGE_BIG:
			color = this.Color(1.0, 0.64999998, 0.0, 0.0);
			size = 4.0;
			break;

		case this.IGIS.FLOATIE_STATUS_WHITE:
			color = this.Color(1.0, 1.0, 1.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_WHITE_MEDIUM:
			color = this.Color(1.0, 1.0, 1.0, 0.0);
			size = 3.0;
			break;

		case this.IGIS.FLOATIE_STATUS_WHITE_BIG:
			color = this.Color(1.0, 1.0, 1.0, 0.0);
			size = 4.0;
			break;

		case this.IGIS.FLOATIE_STATUS_RED:
			color = this.Color(1.0, 0.0, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_RED_MEDIUM:
			color = this.Color(1.0, 0.0, 0.0, 0.0);
			size = 3.0;
			break;

		case this.IGIS.FLOATIE_STATUS_RED_BIG:
			color = this.Color(1.0, 0.0, 0.0, 0.0);
			size = 4.0;
			break;

		case this.IGIS.FLOATIE_STATUS_YELLOW:
			color = this.Color(1.0, 1.0, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_YELLOW_MEDIUM:
			color = this.Color(1.0, 1.0, 0.0, 0.0);
			size = 3.0;
			break;

		case this.IGIS.FLOATIE_STATUS_YELLOW_BIG:
			color = this.Color(1.0, 1.0, 0.0, 0.0);
			size = 4.0;
			break;

		case this.IGIS.FLOATIE_STATUS_GREEN:
			color = this.Color(0.0, 1.0, 0.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_GREEN_MEDIUM:
			color = this.Color(0.0, 1.0, 0.0, 0.0);
			size = 3.0;
			break;

		case this.IGIS.FLOATIE_STATUS_GREEN_BIG:
			color = this.Color(0.0, 1.0, 0.0, 0.0);
			size = 4.0;
			break;

		case this.IGIS.FLOATIE_STATUS_BLUE:
			color = this.Color(0.0, 0.0, 1.0, 0.0);
			break;

		case this.IGIS.FLOATIE_STATUS_BLUE_MEDIUM:
			color = this.Color(0.0, 0.0, 1.0, 0.0);
			size = 3.0;
			break;

		case this.IGIS.FLOATIE_STATUS_BLUE_BIG:
			color = this.Color(0.0, 0.0, 1.0, 0.0);
			size = 4.0;
			break;

		default:
			throw this.Exception("Invalid floatie type");
		}

		local color2 = this.Color(color.r * 0.69999999, color.g * 0.69999999, color.b * 0.69999999, 0.0);
		local node = this._scene.getRootSceneNode().createChildSceneNode();
		local b = this._scene.createTextBoard(node.getName() + "/Floatie", "MaiandraOutline_16", size, text);
		b.setVisibilityFlags(this.VisibilityFlags.ANY | this.VisibilityFlags.FEEDBACK);
		b.setColorTop(color);
		b.setColorBottom(color2);
		node.attachObject(b);
		node.setPosition(pos);
		this.mFloaties.append({
			node = node,
			board = b,
			age = 0.0,
			target_y = pos.y + 1.5
		});
	}

	function updateFloaties()
	{
		if (this.mFloaties.len() == 0)
		{
			return;
		}

		local floaties = this.mFloaties;
		local fadeTime = 0.25;
		local lifetime = 1.5;
		this.mFloaties = [];

		foreach( f in floaties )
		{
			f.age += this._deltat / 1000.0;

			if (f.age >= lifetime)
			{
				f.node.destroy();
				f.node = null;
				continue;
			}

			this.mFloaties.append(f);
			local color = f.board.getColorTop();

			if (lifetime - f.age <= fadeTime)
			{
				color.a = 1.0 - (1.0 - (lifetime - f.age) / fadeTime);
			}
			else if (f.age < fadeTime)
			{
				color.a = f.age / fadeTime;
			}
			else
			{
				color.a = 1.0;
			}

			f.board.setColorTop(color);
			f.board.setColorBottom(this.Color(color.r * 0.69999999, color.g * 0.69999999, color.b * 0.69999999, color.a));
			local newpos = f.node.getPosition();
			newpos.y = this.Math.GravitateValue(newpos.y, f.target_y, this._deltat / 1000.0, 2.5);
			f.node.setPosition(newpos);
		}
	}

	function setPaused( which )
	{
		this.mPauseCount += which == true ? 1 : -1;
		this.mPauseCount = this.Math.max(0, this.mPauseCount);
	}

	mNotificationContainer = null;
	mBinContainer = null;
	mBinIcon = null;
	mPopUps = [];
	mIcons = [];
	mRTRWindow = null;
	mNotifications = [];
	mTransitoryMessages = [];
	mFloaties = [];
	mShowFloaties = true;
	mPauseCount = 0;
}

this.IGIS.notify <- function ( msgId, icon, text, priority )
{
	this._igisManager.addNotification(msgId, "IGIS/Notify", text, null, priority, this.IGIS.NOTIFY);
};
this.IGIS.request <- function ( msgId, icon, text, responses, priority )
{
	this._igisManager.addNotification(msgId, "IGIS/Request", text, responses, priority, this.IGIS.REQUEST);
};
this.IGIS.addListener <- function ( object )
{
	this._igisManager.addListener(object);
};
this.IGIS.removeListener <- function ( object )
{
	this._igisManager.removeListener(object);
};
this.IGIS.info <- function ( text )
{
	this.log.info(text);
	this._igisManager.addTransitory(text, this.IGIS.INFO);
	::_ChatManager.addMessage("sys/info", text);
};
this.IGIS.error <- function ( text )
{
	this.log.error(text);
	this._igisManager.addTransitory(text, this.IGIS.ERROR);
	::_ChatManager.addMessage("err/error", text);
};
this.IGIS.nonLoggingError <- function ( text )
{
	this._igisManager.addTransitory(text, this.IGIS.ERROR);
};
this.IGIS.floatie <- function ( text, style, ... )
{
	this._igisManager.addFloatie(text, style, vargc > 0 ? vargv[0] : null);
};
this.IGIS.dismiss <- function ( msgId )
{
	this._igisManager.dismiss(msgId);
};
this.IGIS.respond <- function ( msgId, response )
{
	this._igisManager.response(msgId, response);
};
this.IGIS.show <- function ()
{
	this._igisManager.show();
};
this.IGIS.hide <- function ()
{
	this._igisManager.hide();
};
