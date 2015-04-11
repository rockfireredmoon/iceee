this.Screens <- {
	mSingletons = {},
	mScreenPositions = {}
};
this.Screens.show <- function ( className, ... )
{
	local s;

	if (vargc > 0)
	{
		s = this.Screens.get(className, true, vargv[0]);
	}
	else
	{
		s = this.Screens.get(className, true);
	}

	s.setVisible(true);

	if ("onShow" in s)
	{
		s.onShow();
	}

	if (!s.getOverlay())
	{
		s.setOverlay(this.GUI.OVERLAY);
	}

	return s;
};
this.Screens.hide <- function ( className )
{
	local s = this.Screens.get(className, false);

	if (s)
	{
		s.setVisible(false);
	}

	return s;
};
this.Screens.toggle <- function ( className, ... )
{
	local s = this.Screens.get(className, true);

	if (s.isVisible() && s.getOverlay())
	{
		if (vargc > 0 && vargv[0])
		{
			this.Screens.close(className);
		}
		else
		{
			this.Screens.hide(className);
		}
	}
	else
	{
		this.Screens.show(className);
	}

	return s;
};
this.Screens.get <- function ( className, createIfNeeded, ... )
{
	if (!(className in this.mSingletons))
	{
		if (!createIfNeeded)
		{
			return null;
		}

		if (vargc > 0)
		{
			this.mSingletons[className] <- this.Screens[className](vargv[0]);
		}
		else
		{
			this.mSingletons[className] <- this.Screens[className]();
		}

		if (className in this.mScreenPositions)
		{
			local screenData = this.mScreenPositions[className];

			if (("oldScreenWidth" in screenData) && ("oldScreenHeight" in screenData) && !this.mSingletons[className].isSticky())
			{
				local position = ::Util.getUpdateResizePosition(screenData.oldScreenWidth.tofloat(), screenData.oldScreenHeight.tofloat(), screenData.x.tofloat(), screenData.y.tofloat(), this.mSingletons[className].getWidth().tofloat(), this.mSingletons[className].getHeight().tofloat());
				this.mSingletons[className].setPosition(position.x, position.y);
			}
		}
	}

	return this.mSingletons[className];
};
this.Screens.getAllScreens <- function ()
{
	return this.mSingletons;
};
this.Screens.close <- function ( name )
{
	if (name in this.mSingletons)
	{
		try
		{
			this.log.debug("Removing screen: " + name);
			this.mSingletons[name].destroy();
		}
		catch( err )
		{
			this.log.error("Error destroying Screen " + name + ": " + err);
		}

		delete this.mSingletons[name];
		return true;
	}

	return false;
};
this.Screens.clear <- function ()
{
	::Screens.saveAllScreenPosition();

	foreach( key, val in this.mSingletons )
	{
		this.log.debug("Removing screen: " + key);
		val.destroy();
	}

	this.mSingletons = {};
};
this.Screens.loadSavePosition <- function ( value )
{
	this.mScreenPositions = this.unserialize(value);

	if (this.mScreenPositions == null)
	{
		this.mScreenPositions = {};
	}

	foreach( className, screenData in this.mScreenPositions )
	{
		if (className in this.mSingletons)
		{
			this.mSingletons[className].setPosition(screenData.x, screenData.y);
		}
	}
};
this.Screens.saveAllScreenPosition <- function ()
{
	foreach( key, screen in this.mSingletons )
	{
		local position = screen.getPosition();
		this.mScreenPositions[key] <- {
			x = position.x,
			y = position.y,
			oldScreenWidth = ::Screen.getWidth().tofloat(),
			oldScreenHeight = ::Screen.getHeight().tofloat()
		};
	}

	::Pref.set("screens.Positions", this.serialize(this.mScreenPositions));
};
this.Screens.clearScreenSavePositions <- function ()
{
	foreach( key, screenData in this.mScreenPositions )
	{
		delete this.mScreenPositions[key];
	}

	this.mScreenPositions = {};
	::Pref.set("screens.Positions", this.serialize(this.mScreenPositions));
};
