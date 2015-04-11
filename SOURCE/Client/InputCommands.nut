this.require("Interpolator");
this.require("UI/MapDef");
this.InputCommands <- {};
this.InputCommandHelpers <- {};
this.LastCommand <- "s";
this.EchoTrigger <- "";
this.InputRegExp <- {
	Command = ::regexp("^[/]([\\w]*)\\s*(.*)"),
	CommandHelp = ::regexp("^[/]([\\?]{1})"),
	CheckStartSlash = ::regexp("^([/]{1})"),
	OneQuestionMark = ::regexp("^([\\?]{1})"),
	OneAlphaNumOneQuestionMark = ::regexp("^([\\w\\-_]+)\\s+([\\?]{1})"),
	OneAlphaNum = ::regexp("^([\\w\\-_]+)"),
	OneAlphaNumOptional = ::regexp("^([\\w\\-_]*)"),
	OneAlphaNumOneColorHex = ::regexp("^([\\w\\-_]+)\\s+([a-fA-F0-9]{6})"),
	TwoAlphaNum = ::regexp("^([\\w\\-_]+)\\s+([\\w\\-_]+)"),
	OneAlphaNumOneAlphaNumOptional = ::regexp("^([\\w\\-_]+)\\s*([\\w\\-_]*)"),
	ThreeAlphaNum = ::regexp("^([\\w\\-_]+)\\s+([\\w\\-_]+)\\s+([\\w\\-_]+)"),
	TwoAlphaNumOneColorHex = ::regexp("^([\\w\\-_]+)\\s+([\\w\\-_]+)\\s+([a-fA-F0-9]{6})"),
	TwoAlphaNumOneQuestionMark = ::regexp("^([\\w\\-_]+)\\s+([\\w\\-_]+)\\s+([\\?]{1})"),
	ThreeAlphaNumOneColorHex = ::regexp("^([\\w\\-_]+)\\s+([\\w\\-_]+)\\s+([\\w\\-_]+)\\s+([a-fA-F0-9]{6})"),
	AlphaNumUnderScoreHyphen = ::regexp("([a-zA-Z0-9_\\-]*)")
};
this.InputCommands.CheckCommand <- function ( pString )
{
	local str = this.Util.trim(pString);

	if (str == "" || str == "/")
	{
		return true;
	}

	if (str[0] == 47)
	{
		if (this.Util.hasPermission("setappearance") && this.Util.startsWith(str.tolower(), "/setappearance"))
		{
			this.InputCommands.setAppearance([
				str.slice(14)
			]);
			return true;
		}

		if (this.Util.startsWith(str.tolower(), "/syschat "))
		{
			this.LastCommand = "*SysChat";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(9), this.LastCommand);
			return true;
		}

		if (this.Util.startsWith(str.tolower(), "/say "))
		{
			this.LastCommand = "s";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(5), this.LastCommand);
			return true;
		}

		if (this.Util.startsWith(str.tolower(), "/s "))
		{
			this.LastCommand = "s";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(3), this.LastCommand);
			return true;
		}

		if (this.Util.startsWith(str.tolower(), "/tell ") || this.Util.startsWith(str.tolower(), "/t "))
		{
			local result;

			if (this.Util.startsWith(str.tolower(), "/tell"))
			{
				result = this.Util.splitQuoteSafe(str.slice(6), " ");
			}
			else
			{
				result = this.Util.splitQuoteSafe(str.slice(3), " ");
			}

			if (result.len() < 2)
			{
				return false;
			}

			local s = result[1];

			for( local i = 2; i < result.len(); ++i )
			{
				s = s + " " + result[i];
			}

			this.LastCommand = "t/" + result[0];
			this._Connection.sendComm(s, this.LastCommand);

			if (this.Util.startsWith(result[0], "\""))
			{
				result[0] = result[0].slice(1, result[0].len());
			}

			if (result[0].slice(result[0].len() - 1, result[0].len()) == "\"")
			{
				result[0] = result[0].slice(0, result[0].len() - 1);
			}

			this.EchoTrigger = "yt/" + result[0];

			if (!::_avatar.hasStatusEffect(this.StatusEffects.GM_SILENCED))
			{
				::_ChatManager.addMessage("yt/" + result[0], s);
			}

			return true;
		}

		if (this.Util.startsWith(str.tolower(), "/p "))
		{
			this.LastCommand = "party";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(3), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/party "))
		{
			this.LastCommand = "party";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(7), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/clan "))
		{
			this.LastCommand = "clan";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(6), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/emote "))
		{
			this.LastCommand = "emote";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(7), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/em ") || this.Util.startsWith(str.tolower(), "/me"))
		{
			this.LastCommand = "emote";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(4), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/ch "))
		{
			local result = this.Util.splitQuoteSafe(str.slice(4), " ");

			if (result.len() < 2)
			{
				return false;
			}

			local s = result[1];

			for( local i = 2; i < result.len(); ++i )
			{
				s = s + " " + result[i];
			}

			this.LastCommand = "ch/" + result[0];
			this.EchoTrigger = "";
			this._Connection.sendComm(s, this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/gm "))
		{
			this.LastCommand = "gm/earthsages";
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(4), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/trade "))
		{
			this.LastCommand = "tc/" + ::_Connection.getCurrentRegionChannel();
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(7), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/region "))
		{
			this.LastCommand = "rc/" + ::_Connection.getCurrentRegionChannel();
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(8), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/chat "))
		{
			this.LastCommand = "rc/" + ::_Connection.getCurrentRegionChannel();
			this.EchoTrigger = "";
			this._Connection.sendComm(str.slice(6), this.LastCommand);
			return true;
		}
		else if (this.Util.startsWith(str.tolower(), "/announce "))
		{
			this._Connection.sendQuery("util.sysannounce", this.NullQueryHandler(), [
				str.slice(10)
			]);
			return true;
		}

		local parsed = ::InputCommandHelpers.parseCommand(str);

		if (!parsed)
		{
			return false;
		}

		local cmd = parsed.cmd.tolower();

		foreach( i, x in this.InputCommands )
		{
			if (i.tolower() == cmd)
			{
				this.InputCommands[i](parsed.args);
				return true;
			}
		}

		if (this.strcasecmp(cmd, "util.reprofession") == 0)
		{
			this._Connection.sendQuery(parsed.cmd, this.ReprofessionHandler(), parsed.args[0]);
		}
		else if (this.strcasecmp(cmd, "util.swear.list") == 0)
		{
			this._Connection.sendQuery(parsed.cmd, this.SwearWordListHandler());
		}
		else
		{
			::_Connection.sendAction(parsed.cmd, parsed.args);
		}

		return true;
	}

	this._Connection.sendComm(str, this.LastCommand);

	if (this.EchoTrigger != "")
	{
		::_ChatManager.addMessage(this.EchoTrigger, str);
	}

	return true;
};
class this.PubSubQueryHandler 
{
	function onQueryComplete( qa, results )
	{
		if (qa == null || qa.query == null)
		{
			return;
		}

		if (qa.query == "ps.join" && results[0][0] == "OK")
		{
			this.IGIS.info("Joined channel " + qa.args[0] + " successfully");
		}
		else if (qa.query == "ps.leave" && results[0][0] == "OK")
		{
			this.IGIS.info("Left channel " + qa.args[0] + " successfully");
		}
	}

	function onQueryError( qa, results )
	{
		if (results)
		{
			return;
		}

		this.IGIS.error(results);
	}

}

this.QueryHandler <- this.PubSubQueryHandler();
this.InputCommands.join <- function ( args )
{
	if (args.len() == 2)
	{
		this._Connection.sendQuery("ps.join", this.QueryHandler, [
			args[0],
			this.md5(args[1])
		]);
	}
	else if (args.len() == 1)
	{
		this._Connection.sendQuery("ps.join", this.QueryHandler, [
			args[0]
		]);
	}
	else
	{
		this.IGIS.error("Usage: /join <topic> [password]");
	}
};
this.InputCommands.leave <- function ( args )
{
	if (args.len() == 1)
	{
		this._Connection.sendQuery("ps.leave", this.QueryHandler, [
			args[0]
		]);
	}
	else
	{
		this.IGIS.error("Usage: /leave <topic>");
	}
};
this.InputCommands.auditlog <- function ( args )
{
	if (!this.Util.hasPermission("auditlog"))
	{
		return;
	}

	this.System.openURL("https://secure.sparkplaymedia.com/ee/auditlog/logview.php");
};
class this.VersionHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		local text = "--[ Version Info ]--\n";

		foreach( row in results )
		{
			text += row[0] + ": " + row[1] + "\n";
		}

		this.IGIS.info(text);
		this.log.info(text);
	}

}

class this.KickHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
	}

	function onQueryError( qa, reason )
	{
		this.IGIS.error(reason);
	}

}

class this.SimHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		local text = "Simulator name: " + results[0][0];
		this.IGIS.info(text);
		this.log.info(text);
	}

}

class this.ReprofessionHandler extends this.DefaultQueryHandler
{
	mUpdateAbilityWindow = false;
	constructor()
	{
		::_Connection.addListener(this);
	}

	function onQueryComplete( qa, results )
	{
		this.mUpdateAbilityWindow = true;
		local str = this.resultAsString(qa, results);
		this.IGIS.info(str);
		this.log.info(str);
	}

	function onProfessionUpdate( value )
	{
		if (this.mUpdateAbilityWindow)
		{
			::_AbilityManager.handleAbRespec();
			this.mUpdateAbilityWindow = false;
		}
	}

}

class this.PingHandler extends this.DefaultQueryHandler
{
	type = "";
	startTime = 0;
	constructor( txt )
	{
		this.type = txt;
		this.startTime = ::System.currentTimeMillis();
	}

	function onQueryComplete( qa, results )
	{
		local text = this.type + " ping time: " + (::System.currentTimeMillis() - this.startTime) + " milliseconds.";
		this.IGIS.info(text);
		this.log.info(text);
	}

}

class this.SwearWordListHandler extends this.DefaultQueryHandler
{
	mSwearList = null;
	constructor()
	{
		this.mSwearList = {};
	}

	function onQueryComplete( qa, results )
	{
		foreach( result in results )
		{
			local type = result[0];

			if (type in this.mSwearList)
			{
				this.mSwearList[type].append(result[1]);
			}
			else
			{
				local wordList = [];
				wordList.append(result[1]);
				this.mSwearList[type] <- wordList;
			}
		}

		this.saveAbilityData(this.mSwearList);
	}

	function getSwearWordPath()
	{
		local basePath = this._cache.getBaseURL();

		if (basePath.slice(0, 8) != "file:///")
		{
			throw this.Exception("Swear word list path unavailble for base URL: " + basePath);
		}

		basePath = basePath.slice(8);
		basePath += "/../../Media/Catalogs/";
		return basePath;
	}

	function saveAbilityData( results )
	{
		local name = "SwearWordList";
		local filename = this.getSwearWordPath() + name + ".nut";
		local out = "SwearWordIndex <- " + this.serialize(results) + ";";
		::System.writeToFile(filename, out);
		::_ChatManager.loadSwearWordList(results);
	}

}

this.InputCommands.version <- function ( args )
{
	if (this.Util.hasPermission("version"))
	{
		::_Connection.sendQuery("util.version", this.VersionHandler(), []);
	}
};
this.InputCommands.sim <- function ( args )
{
	if (this.Util.hasPermission("version"))
	{
		::_Connection.sendQuery("util.simaddress", this.SimHandler(), []);
	}
};
this.InputCommands.ping <- function ( args )
{
	::_Connection.sendQuery("util.pingrouter", this.PingHandler("Router"), []);
	::_Connection.sendQuery("util.pingsim", this.PingHandler("Simulator"), []);
};
this.InputCommands.pingSim <- function ( args )
{
	::_Connection.sendQuery("util.pingsim", this.PingHandler("Simulator"), []);
};
this.InputCommands.pingRouter <- function ( args )
{
	::_Connection.sendQuery("util.pingrouter", this.PingHandler("Router"), []);
};
this.InputCommands.ping2 <- function ( args )
{
	if (this.Util.hasPermission("dev"))
	{
		this.Screens.toggle("PingScreen");
	}
};
this.InputCommands.kick <- function ( args )
{
	::_Connection.sendQuery("util.kick", this.KickHandler(), args);
};
this.InputCommands.creatureVis <- function ( args )
{
	if (!this.Util.hasPermission("reassemble"))
	{
		return;
	}

	local vis = args.len() > 0 ? args[0] != "off" : true;
	local visFunc = function ( mo ) : ( vis )
	{
		mo.setVisible(vis);
	};

	foreach( id, c in this._sceneObjectManager.mCreatures )
	{
		this.Util.visitMovables(c.getNode(), visFunc);
	}
};
this.InputCommands.disconnect <- function ( args )
{
	::_Connection.close();
};
this.InputCommands.emote <- function ( args )
{
	local def = ::_avatar.getDef();

	if (args.len() > 0 && args[0] == "?")
	{
		local str = ::TXT("emote must be formated as follows: emote <emote>\n" + "Avalable Animations:\n");

		foreach( i, x in this.Util.tableKeys(::BipedAnimationDef.Animations, true) )
		{
			if (i > 0)
			{
				str += ", ";
			}

			str += x;
		}

		this.IGIS.info(str);
		return;
	}

	if (args.len() > 0)
	{
		this._contentLoader.load([
			"Biped-Anim-Emote"
		], this.ContentLoader.PRIORITY_REQUIRED, "Emote-AnimDeps", {
			function onPackageComplete( pkgName ) : ( args )
			{
				::_avatar.mAnimationHandler.onFF(args[0]);
				::_Connection.sendComm(args[0], "emote");
			}

			function onPackageError( pkg, error )
			{
				this.log.debug("Error loading package " + pkg + " - " + error);
				this.onPackageComplete(pkg);
			}

		});
		return true;
	}

	this.IGIS.error(::TXT("emote needs to be formated emote <emote name>"));
	return false;
};
this.InputCommands.playSound <- function ( args )
{
	if (!this.Util.hasPermission("playSound"))
	{
		return;
	}

	if (args.len() == 0)
	{
		this.IGIS.error("Usage: /playSound <soundFile>");
	}
	else
	{
		this._avatar.playSound(args[0]);
	}
};
this.InputCommands.compositor <- function ( args )
{
	if (args.len() == 0 || !this.Util.hasPermission("compositor"))
	{
		return;
	}

	::_root.removeAllTargetCompositors();

	if (args[0].tolower() != "none")
	{
		::_root.addTargetCompositor(args[0]);
		::_root.setTargetCompositorEnabled(args[0], true);
	}
};
this.InputCommands.showCollision <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local f = this._scene.getVisibilityMask();
	local arg = args.len() > 0 ? args[0].tolower() : "";

	if (arg == "true" || arg == "on")
	{
		f = f | this.VisibilityFlags.COLLISION;
	}
	else if (arg == "false" || arg == "off")
	{
		f = f | ~this.VisibilityFlags.COLLISION;
	}
	else if (f & this.VisibilityFlags.COLLISION)
	{
		f = f & ~this.VisibilityFlags.COLLISION;
	}
	else
	{
		f = f | this.VisibilityFlags.COLLISION;
	}

	this._scene.setVisibilityMask(f);
};
this.InputCommands.unstick <- function ( args )
{
	::_Connection.sendQuery("unstick", this);
};
this.InputCommands.setFOV <- function ( args )
{
	if (this.Util.hasPermission("debug"))
	{
		this.Interpolate(this._camera, "setFOVy", this._camera.getFOVy(), args[0].tofloat() * this.Math.PI / 180);
	}
};
this.InputCommands.setPositionDebugObjects <- function ( args )
{
	if (this.Util.hasPermission("debug"))
	{
		this.gPositionDebugObjects = args[0].tointeger() != 0;
	}
};
this.InputCommands.setAmbientLight <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local a = 0.80000001;

	if (args.len() > 0)
	{
		a = args[0].tofloat();
	}

	this._scene.setAmbientLight(this.Color(a, a, a));
};
this.InputCommands.lag <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local LoopAmount = 100000;

	if (args.len() > 0)
	{
		LoopAmount = args[0].tointeger();
	}

	for( local i = 0; i < LoopAmount; i++ )
	{
		this.log.debug("lag");
	}
};
this.InputCommands.setEnvironment <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		this._Environment.setOverride(args[0]);
	}
	else
	{
		this._Environment.setOverride(null);
	}
};
this.InputCommands.setTimeOfDay <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		this._Environment.setTimeOfDay(args[0]);
	}
	else
	{
		this._Environment.setTimeOfDay("Day");
	}

	this._Environment.update(0);
};
this.InputCommands.scriptProfile <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		if (args[0] == "on")
		{
			this.System.setScriptProfilerEnabled(true);
		}
		else if (args[0] == "off")
		{
			this.System.setScriptProfilerEnabled(false);
		}
		else if (args[0] == "dump")
		{
			this.log.info("Script Profile:\n" + this.System.getScriptProfile());
		}
	}
	else
	{
		this.IGIS.info("Usage: /scriptProfile (on|off|dump)");
	}
};
this.InputCommands.test <- function ( args )
{
	::test(args.len() > 0 ? args[0] : "");
};
this.InputCommands.gm <- function ( args )
{
	this.Screens.toggle("GMScreen");
};
this.InputCommands.petition <- function ( args )
{
	this.Screens.toggle("PetitionScreen");
};
this.InputCommands.earthsage <- function ( args )
{
	this.Screens.toggle("PetitionScreen");
};
::_UIVisible <- true;
this.InputCommands.toggleUI <- function ( args )
{
	::_UIVisible = !::_UIVisible;
	this.Screen.toggleOverlayForceInvisible(this.GUI.CONFIRMATION_OVERLAY);
	this.Screen.toggleOverlayForceInvisible(this.GUI.POPUP_OVERLAY);
	this.Screen.toggleOverlayForceInvisible("GUI/ChatOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/ChatWindowOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/Overlay");
	this.Screen.toggleOverlayForceInvisible("GUI/Overlay2");
	this.Screen.toggleOverlayForceInvisible("GUI/FullScreenComponentOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/IGISOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/TooltipOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/SelectionBox");
	this.Screen.toggleOverlayForceInvisible("GUI/QuickBarOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/EditBorderOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/CursorOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/DragOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/DebugScreen");
	this.Screen.toggleOverlayForceInvisible("GUI/ChatBubbleOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/MainUIOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/TargetOverlay");
	this.Screen.toggleOverlayForceInvisible("GUI/MiniMap");
	this.Screen.toggleOverlayForceInvisible("GUI/QuestTracker");
	this.Screen.toggleOverlayForceInvisible("GUI/TutorialOverlay");
	::_igisManager.setFloatieVisible(::_UIVisible);
	this.QuestIndicator.indicatorsVisibility(::_UIVisible);
};
this.InputCommands.sceneryBrowser <- function ( args )
{
	if (!this.Util.hasPermission("build"))
	{
		return;
	}

	local frame = this.Screens.SceneryObjectBrowser();
	frame.setVisible(true);
};
this.InputCommands.setFullscreen <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	::Screen.setFullscreen(args[0].tolower() == "true" ? true : false);
};
this.InputCommands.toggleWeapons <- function ( args )
{
	local animated = true;

	if (::_avatar.hasStatusEffect(this.StatusEffects.AUTO_ATTACK) || ::_avatar.hasStatusEffect(this.StatusEffects.AUTO_ATTACK_RANGED))
	{
		animated = false;
	}

	local slot;

	switch(::_avatar.getVisibleWeapon())
	{
	case this.VisibleWeaponSet.NONE:
		if (::_avatar.hasWeaponSet(this.VisibleWeaponSet.MELEE))
		{
			slot = this.VisibleWeaponSet.MELEE;
		}
		else if (::_avatar.hasWeaponSet(this.VisibleWeaponSet.RANGED))
		{
			slot = this.VisibleWeaponSet.RANGED;
		}
		else
		{
			return;
		}

		break;

	case this.VisibleWeaponSet.MELEE:
		if (::_avatar.hasWeaponSet(this.VisibleWeaponSet.RANGED))
		{
			slot = this.VisibleWeaponSet.RANGED;
		}
		else
		{
			slot = this.VisibleWeaponSet.NONE;
		}

		break;

	case this.VisibleWeaponSet.RANGED:
	default:
		slot = this.VisibleWeaponSet.NONE;
	}

	if (slot == this.VisibleWeaponSet.NONE)
	{
		::Audio.playSound("Sound-Ability-Sheath.ogg");
	}
	else
	{
		::Audio.playSound("Sound-Ability-Unsheath.ogg");
	}

	::_avatar.setVisibleWeapon(slot, animated);
};
this.InputCommands.togglePolygonMode <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local cam = this._scene.getCamera("Default");

	if (args.len() > 0 && args[0] == "?")
	{
		return;
	}

	local mode = args.len() > 0 ? args[0].tolower() : "";

	switch(mode)
	{
	case "solid":
		cam.setPolygonMode(this.Camera.PM_SOLID);
		break;

	case "wireframe":
		cam.setPolygonMode(this.Camera.PM_WIREFRAME);
		break;

	case "points":
		cam.setPolygonMode(this.Camera.PM_POINTS);
		break;

	default:
		if (cam.getPolygonMode() == this.Camera.PM_SOLID)
		{
			cam.setPolygonMode(this.Camera.PM_WIREFRAME);
		}
		else
		{
			cam.setPolygonMode(this.Camera.PM_SOLID);
		}
	}
};
this.require("UI/MiniMapScreen");
this.InputCommands.minimap <- function ( args )
{
	local screen = this.Screens.get("MiniMapScreen", false);

	if (screen == null)
	{
		return;
	}

	if (args.len() == 0)
	{
		screen.toggleMode();
		return;
	}

	switch(args[0])
	{
	case "small":
		screen.setMode(this.MINIMAP_SMALL);
		break;

	case "large":
		screen.setMode(this.MINIMAP_LARGE);
		break;

	case "off":
		screen.setMode(this.MINIMAP_OFF);
		break;
	}
};
this.InputCommands.exportVegetation <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local text = "<?xml version=\"1.0\" ?>\n<tables>\n";

	foreach( k, v in ::Vegetation )
	{
		text += "\t<vegetation name=\"" + k + "\" " + ("-meta-" in v ? "cellSize = \"" + v["-meta-"].cellSize.tostring() + "\" " : "") + ">\n";

		foreach( n, e in v )
		{
			if (n != "-meta-")
			{
				text += "\t\t<entry name=\"" + n + "\" ";

				if ("minSize" in e)
				{
					text += "minSize=\"" + e.minSize + "\" ";
				}

				if ("maxSize" in e)
				{
					text += "maxSize=\"" + e.maxSize + "\" ";
				}

				if ("weight" in e)
				{
					text += "weight=\"" + e.weight + "\" ";
				}

				text += "/>\n";
			}
		}

		text += "\t</vegetation>\n";
	}

	text += "</tables>";
	this.System.writeToFile("../../../Server/resources/Vegetation.xml", text);
};
this.InputCommands.searchRequests <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.log.debug("----------------------------------------------");

	foreach( r in this._contentLoader.mRequests )
	{
		if (args.len() == 0 || r.mState == args[0])
		{
			this.log.debug("STATE: " + r.mState + ", PACKAGE: " + r.mMedia);
		}
	}
};
this.InputCommands.togglePreview <- function ( args )
{
	if (this.Util.hasPermission("preview") == false)
	{
		return;
	}

	if (!::_avatar)
	{
		return;
	}

	if (this.gToolMode == "play")
	{
		::_buildTool.setPreviewMode(true);
		this._tools.setActiveTool(::_buildTool);
		this.gToolMode = "build";
	}
	else
	{
		this._tools.setActiveTool(::_playTool);
		this.gToolMode = "play";
	}

	this.IGIS.info("Now in preview mode.");
};
this.InputCommands.toggleBuilding <- function ( args )
{
	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	if (!this.Util.hasPermission("build"))
	{
		return;
	}

	if (this.gToolMode == "play")
	{
		::_buildTool.setPreviewMode(false);
		this._tools.setActiveTool(::_buildTool);
		this.gToolMode = "build";
	}
	else
	{
		this._tools.setActiveTool(::_playTool);
		this.gToolMode = "play";
	}

	this.IGIS.info("Now in " + this.gToolMode + " mode.");
};
this.InputCommands.toggleQuickbar <- function ( args )
{
	if (::_quickBarManager.getVisible())
	{
		::_quickBarManager.hide();
	}
	else
	{
		::_quickBarManager.show();
	}
};
this.InputCommands.tss <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.Screens.toggle("TeamSlayerScreen");
};
this.InputCommands.ctfs <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.Screens.toggle("CTFScreen");
};
this.InputCommands.inv <- function ( args )
{
	this.Screens.toggle("Inventory");
};
this.InputCommands.macro <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.Screens.toggle("MacroCreator");
};
this.InputCommands.eq <- function ( args )
{
	this.Screens.toggle("Equipment");
};
this.InputCommands.itemAppearance <- function ( args )
{
	if (this.Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	this.Screens.toggle("ItemAppearanceTweak");
};
this.InputCommands.scriptTest <- function ( args )
{
	if (!this.Util.hasPermission("scriptTest"))
	{
		return;
	}

	this.Screens.toggle("ScriptTest");
};
this.InputCommands.map <- function ( args )
{
	this.Screens.toggle("MapWindow");
};
this.InputCommands.qj <- function ( args )
{
	this.Screens.toggle("QuestJournal");
};
this.InputCommands.dumpResourceUsage <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local verbose = false;

	if (args.len() > 0)
	{
		verbose = args[0] == "verbose";
	}

	::_root.dumpResourceUsage(verbose);
};
this.InputCommands.resize <- function ( args )
{
	this.USE_OLD_SCREEN = !this.USE_OLD_SCREEN;
};
this.InputCommands.stats <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local obj = ::_avatar.getTargetObject();
	local stats = ::_avatar.getTargetObject().getStats();

	if (stats)
	{
		foreach( k, v in stats )
		{
			this.log.debug(::Stat[k].name + ": " + v);
		}
	}
	else
	{
		this.log.debug("No stats!");
	}
};
this.InputCommands.myColors <- function ( args )
{
	if (!this.Util.hasPermission("build"))
	{
		return;
	}

	::Screens.toggle("CustomColorsScreen", true);
};
this.InputCommands.hideChat <- function ( args )
{
	::Screens.hide("ChatWindow");
};
this.InputCommands.zoneTweak <- function ( args )
{
	if (this.Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local frame = this.Screens.ZoneTweakScreen();
	frame.setPosition(200, 100);
	frame.setOverlay(this.GUI.OVERLAY);
	frame.setVisible(true);
};
class this.TweakCreature extends this.DefaultQueryHandler
{
	mCreating = false;
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.mCreating = vargv[0];
		}
	}

	function onQueryComplete( qa, results )
	{
		if (results.len() == 0)
		{
			local callback = {
				name = qa.args[0],
				function onActionSelected( mb, alt )
				{
					if (alt == "Yes")
					{
						::_Connection.sendQuery("creature.def.edit", this.TweakCreature(true), [
							"NEW",
							"name",
							this.name
						]);
					}
				}

			};
			this.GUI.MessageBox.showYesNo("The creature type \"" + qa.args[0] + "\" does not exist. " + "Do you want to create it?", callback);
			return;
		}

		local row = results[0];
		local id = row[0].tointeger();
		local a = this.GetAssembler("Creature", id);

		if (this.mCreating)
		{
			a.setStat(this.Stat.DISPLAY_NAME, qa.args[2]);
		}

		local ct = this.Screens.show("CreatureTweakScreen");
		ct.setCurrentType(id);
	}

}

this.InputCommands.pages <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.log.debug("Total pages: " + this._sceneObjectManager.mPages.len());

	foreach( p in this._sceneObjectManager.mPages )
	{
		local s = p.getState();

		switch(s)
		{
		case this.PageState.PENDINGREQUEST:
			s = "Pending Request";
			break;

		case this.PageState.REQUESTED:
			s = "Requested";
			break;

		case this.PageState.LOADING:
			s = "Loading";
			break;

		case this.PageState.READY:
			s = "Ready";
			break;

		case this.PageState.ERRORED:
			s = "Errored";
			break;
		}

		this.log.debug("Page " + p.getX() + ", " + p.getZ() + ": " + s);
	}
};
this.InputCommands.bug <- function ( args )
{
	this.Screens.show("BugReport");
};
this.InputCommands.terrain <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.log.debug("Total terrain pages: " + this._sceneObjectManager.mLoadedTerrain.len());

	foreach( k, v in this._sceneObjectManager.mLoadedTerrain )
	{
		this.log.debug("Terrain Page " + v.x + ", " + v.z + ": " + v.mode);
	}
};
this.InputCommands.sd <- function ( args )
{
	local tmp = [
		"WARP_TARGET"
	];
	tmp.extend(args);
	::_Connection.sendQuery("go", this.NullQueryHandler(), tmp);
};
this.InputCommands.minimapCreatureCategory <- function ( args )
{
	if (args.len() < 1)
	{
		foreach( creatureCategory in this.LegendItemCreatures )
		{
			::LegendItemSelected[creatureCategory] = true;
		}
	}
	else
	{
		local filteredCreatureCategory = args[0];

		foreach( creatureCategory in this.LegendItemCreatures )
		{
			if (filteredCreatureCategory == creatureCategory)
			{
				::LegendItemSelected[creatureCategory] = true;
			}
			else
			{
				::LegendItemSelected[creatureCategory] = false;
			}
		}
	}

	local legendSelectedTable = this.deepClone(::LegendItemSelected);
	::Pref.set("map.LegendItems", legendSelectedTable);
	::Util.updateMiniMapStickers();
};
this.InputCommands.minimapShopkeepers <- function ( args )
{
	if (args.len() < 1)
	{
		return;
	}

	if (args[0].tolower() == "1")
	{
		::LegendItemSelected[this.LegendItemTypes.SHOP] = true;
	}
	else
	{
		::LegendItemSelected[this.LegendItemTypes.SHOP] = false;
	}

	local legendSelectedTable = this.deepClone(::LegendItemSelected);
	::Pref.set("map.LegendItems", legendSelectedTable);
	::Util.updateMiniMapStickers();
};
this.InputCommands.setAppearance <- function ( args )
{
	local appearance = args.len() > 0 ? this.Util.trim(args[0]) : "";

	if (appearance != "")
	{
		this._avatar.getAssembler().setStat(this.Stat.APPEARANCE, appearance);
	}
};
this.InputCommands.links <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	::_scene.setLinksVisible(!::_scene.getLinksVisible());
};
this.InputCommands.abilityScreen <- function ( args )
{
	local as = this.Screens.get("AbilityFrame", false);

	if (as && as.isVisible())
	{
		as.setVisible(false);
	}
	else
	{
		as = this.Screens.show("AbilityFrame");
	}
};
this.InputCommands.partyScreen <- function ( args )
{
	local ps = this.Screens.get("PartyScreen", false);

	if (ps && ps.isVisible())
	{
		ps.setVisible(false);
	}
	else
	{
		ps = this.Screens.show("PartyScreen");
	}
};
this.InputCommands.iconBrowser <- function ( args )
{
	if (this.Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local ib = this.Screens.get("IconBrowserScreen", false);

	if (ib && ib.isVisible())
	{
		ib.setVisible(false);
	}
	else
	{
		ib = this.Screens.show("IconBrowserScreen");
	}
};
this.InputCommands.creatureDebug <- function ( args )
{
	if (this.Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	if (args.len() > 0)
	{
		local id = 0;

		try
		{
			id = args[0].tointeger();
		}
		catch( err )
		{
			id = 0;
		}

		if (id != 0)
		{
		}

		local creatureDebug = this.Screens.show("CreatureDebugScreen");
		creatureDebug.setCurrentType(id);
	}
	else
	{
		local creatureDebug = this.Screens.get("CreatureDebugScreen", false);

		if (creatureDebug && creatureDebug.isVisible())
		{
			creatureDebug.setVisible(false);
		}
		else
		{
			creatureDebug = this.Screens.show("CreatureDebugScreen");

			if (this._avatar.getTargetObject())
			{
				creatureDebug.setCurrentType(this._avatar.getTargetObject().getType());
			}
			else
			{
				creatureDebug.setCurrentType(this._avatar.getType());
			}
		}
	}
};
this.InputCommands.creatureTweak <- function ( args )
{
	if (this.Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	if (args.len() > 0)
	{
		local id = 0;

		try
		{
			id = args[0].tointeger();
		}
		catch( err )
		{
			id = 0;
		}

		if (id == 0)
		{
			this._Connection.sendQuery("creature.def.list", this.TweakCreature(), [
				"",
				args[0]
			]);
		}
		else
		{
			local ct = this.Screens.show("CreatureTweakScreen");
			ct.setCurrentType(id);
		}
	}
	else
	{
		local ct = this.Screens.get("CreatureTweakScreen", false);

		if (ct && ct.isVisible())
		{
			ct.setVisible(false);
		}
		else
		{
			ct = this.Screens.show("CreatureTweakScreen");

			if (this._avatar.getTargetObject())
			{
				ct.setCurrentType(this._avatar.getTargetObject().getType());
			}
			else
			{
				ct.setCurrentType(this._avatar.getType());
			}
		}
	}
};
this.InputCommands.creatureBrowse <- function ( args )
{
	if (this.Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	this.Screens.show("CreatureBrowserScreen");
};
this.InputCommands.shaky <- function ( args )
{
	if (args.len() != 3)
	{
		this.IGIS.error("usage: /shaky amount time range");
		return;
	}

	::_playTool.addShaky(::_avatar.getPosition(), args[0].tofloat(), args[1].tofloat(), args[2].tofloat());
};
this.InputCommands.markers <- function ( args )
{
	if (!this.Util.hasPermission("tweakScreens"))
	{
		return;
	}

	this.Screens.show("MarkerTweakScreen");
};
this.InputCommands.inventory <- function ( args )
{
	this.Screens.toggle("Inventory");
};
this.InputCommands.shards <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	::Screens.toggle("ShardSelectScreen");
};
this.InputCommands.friends <- function ( args )
{
	::Screens.toggle("SocialWindow");
};
this.InputCommands.undo <- function ( args )
{
	if (!this.Util.hasPermission("build"))
	{
		return;
	}

	if (this._opHistory.canUndo())
	{
		this.IGIS.info("Undoing: " + this._opHistory.getUndoPresentationName());

		try
		{
			this._opHistory.undo();
		}
		catch( err )
		{
			this.IGIS.error("Error during undo: " + err);
		}
	}
	else
	{
		this.IGIS.info("Nothing to redo (or last operation not undoable).");
	}
};
this.InputCommands.redo <- function ( args )
{
	if (!this.Util.hasPermission("build"))
	{
		return;
	}

	if (this._opHistory.canRedo())
	{
		this.IGIS.info("Redoing: " + this._opHistory.getRedoPresentationName());

		try
		{
			this._opHistory.redo();
		}
		catch( err )
		{
			this.IGIS.error("Error during redo: " + err);
		}
	}
	else
	{
		this.IGIS.info("Nothing to redo.");
	}
};
this.InputCommands.debug <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		local p = "debug." + args[0];

		if (this.Pref.isDeclared(p))
		{
			if (this.Pref.get(p) == this.Pref.getDefault(p))
			{
				this.Pref.set(p, !this.Pref.getDefault(p));
			}
			else
			{
				this.Pref.set(p, null);
			}
		}
	}
	else
	{
		this.Screens.toggle("DebugScreen");
	}
};
this.InputCommands.dps <- function ( args )
{
	this.Screens.toggle("DPSMeter");
};
this.InputCommands.updateAbilities <- function ( args )
{
	if (!this.Util.hasPermission("dev"))
	{
		return;
	}

	::_AbilityManager.handleUpdatingAbilities(::AbilityIndex, false);
};
this.InputCommands.updateClientAbilities <- function ( args )
{
	if (!this.Util.hasPermission("dev"))
	{
		return;
	}

	::_AbilityHelper.updateClientAbilities();
};
this.InputCommands.importExcel <- function ( args )
{
	if (this.Util.hasPermission("importAbilities") == false)
	{
		return;
	}

	this.Screens.show("AbEditScreen").onImport(null);
};
this.InputCommands.te <- function ( args )
{
	this.InputCommands.testEffect(args);
};
this.InputCommands.testEffect <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() == 0)
	{
		this.IGIS.error("Usage: /testEffect Script1[,Script2,...]");
		return;
	}

	if (!(args[0] in ::EffectDef))
	{
		::IGIS.error("Invalid effect name: " + args[0]);
		return;
	}

	this.System.reloadScript("EffectDef");
	this._contentLoader.load([
		"Biped-Anim-Combat",
		"Biped-Anim-Emote"
	], this.ContentLoader.PRIORITY_REQUIRED, "Effect-AnimDeps", {
		function onPackageComplete( pkgName ) : ( args )
		{
			try
			{
				::_avatar.cue(args[0]);
			}
			catch( err )
			{
				this.log.error("Error cuing effect: " + err);
			}
		}

		function onPackageError( pkg, error )
		{
			this.log.debug("Error loading package " + pkg + " - " + error);
			this.onPackageComplete(pkg);
		}

	});
};
this.InputCommands.reassemble <- function ( args )
{
	if (!this.Util.hasPermission("reassemble"))
	{
		return;
	}

	foreach( k, v in this._sceneObjectManager.mScenery )
	{
		v.reassemble();
	}

	foreach( k, v in this._sceneObjectManager.mCreatures )
	{
		v.reassemble();
	}
};
this.InputCommands.copyPos <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local node = ::_loadNode;

	if (node == null && ::_avatar != null)
	{
		node = ::_avatar.getNode();
	}

	if (node)
	{
		local pos = node.getPosition();
		local str = "\"(" + pos.x + "," + pos.y + "," + pos.z + "," + ::_sceneObjectManager.getCurrentZoneDefID() + ")\"";
		::System.setClipboard(str);
		::IGIS.info("Position copied to cliboard");
	}
	else
	{
		::IGIS.error("Cannot copy position to clipboard");
	}
};
this.InputCommands.hide <- function ( args )
{
	if (this._buildTool && this._buildTool.isInBuildMode())
	{
		if (this._buildTool.isShowingTerrainOnly())
		{
			this._buildTool.setShowTerrainOnly(false);
			local previousFlags = this._buildTool.getPreviousVisibilityFlags();
			this._scene.setVisibilityMask(previousFlags);
		}
		else
		{
			this._buildTool.setShowTerrainOnly(true);
			this._buildTool.setPreviousVisibilityFlags(this._scene.getVisibilityMask());
			this._scene.setVisibilityMask(this.VisibilityFlags.SCENERY & ~this.VisibilityFlags.WATER);
		}

		this.print("SHOWING TERRAIN ONLY");
	}
};
this.InputCommands.hideNonMimimapProps <- function ( args )
{
	if (this._buildTool && this._buildTool.isInBuildMode())
	{
		if (this._buildTool.isShowingTerrainOnly())
		{
			this._buildTool.setShowTerrainOnly(false);
			local previousFlags = this._buildTool.getPreviousVisibilityFlags();
			this._scene.setVisibilityMask(previousFlags);
		}
		else
		{
			this._buildTool.setShowTerrainOnly(true);
			this._buildTool.setPreviousVisibilityFlags(this._scene.getVisibilityMask());
			this._scene.setVisibilityMask(this.VisibilityFlags.PROPS | this.VisibilityFlags.SCENERY | this.VisibilityFlags.CREATURE | this.VisibilityFlags.ATTACHMENT | this.VisibilityFlags.WATER | this.VisibilityFlags.ANYTHING | this.VisibilityFlags.HELPER_GEOMETRY);
		}

		this.print("SHOWING TERRAIN ONLY");
	}
};
this.InputCommands.resetIndicators <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	::QuestIndicator.updateCreatureIndicators();
};
this.InputCommands.resetQuests <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	::_questManager.reset();
	::QuestIndicator.updateCreatureIndicators();
	local questJournal = ::Screens.get("QuestJournal", true);
	questJournal.updateCharacterSavedQuestMarkers();
	::_useableCreatureManager.refreshCache();
};
this.InputCommands.rq <- function ( args )
{
	this.InputCommands.resetQuests(args);
};
this.InputCommands.configVideo <- function ( args )
{
	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	::Screens.toggle("VideoOptionsScreen");
};
this.InputCommands.browserScript <- function ( args )
{
	if (("runBrowserScript" in ::System) && args.len() > 0)
	{
		this.System.runBrowserScript(args[0]);
	}
};
this.InputCommands.reloadTextures <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		foreach( pattern in args )
		{
			this.System.forceTextureRefresh(pattern);
		}
	}
	else
	{
		this.System.forceTextureRefresh();
	}
};
this.InputCommands.shop <- function ( args )
{
	local target = ::_avatar.getTargetObject();

	if (target)
	{
		this.Screens.get("ItemShop", true).setMerchantId(target.getID());
		this.Screens.show("ItemShop", true);
	}
	else
	{
		this.log.debug("You must select an NPC first.");
	}
};
this.InputCommands["do"] <- function ( args )
{
	if (args.len() == 0)
	{
		return;
	}

	local flags = 0;

	if (args.len() > 1 && args[1] == "party")
	{
		flags = this.AbilityFlags.PARTY_CAST;
	}

	local ab = this._AbilityManager.getAbilityByName(args[0]);

	if (ab != null && ab.getIsValid())
	{
		if (ab.getActions().find("GTAE") == null)
		{
			ab.sendActivationRequest(true, flags);
		}
		else if (this._groundTargetTool)
		{
			local actions = ab.getActions();
			local indexOfGTAE = actions.find("GTAE");
			local indexOfOpenParam = actions.find("(", indexOfGTAE);
			local indexOfCloseParam = actions.find(")", indexOfOpenParam);
			local size = actions.slice(indexOfOpenParam + 1, indexOfCloseParam).tointeger();
			this._groundTargetTool.setSize(size * 2, size * 2);
			this._groundTargetTool.setAbility(ab);
			this._tools.push(this._groundTargetTool);
		}
	}
};
this.InputCommands.stop <- function ( args )
{
	local ab = this._AbilityManager.getAbilityByName(args[0]);

	if (ab.getIsValid())
	{
		ab.sendActivationRequest(false);
	}
};
this.InputCommands.switchATS <- function ( args )
{
	if (!this.Util.hasPermission("build"))
	{
		return;
	}

	if (args.len() > 0)
	{
		local ats = args[0];
		local sel = ::_buildTool.getSelection().objects();
		local op = this.CompoundOperation();
		local count = 0;

		foreach( so in sel )
		{
			if ("ATS" in so.mVars)
			{
				local a = so.getVarsTypeAsAsset();
				a.mVars.ATS = ats;
				op.add(this.BeanSetPropertyOp(so, "asset", a));
				count++;
			}
		}

		if (count > 0)
		{
			op.setPresentationName("SwitchATS to " + ats + " (x " + op.len() + ")");
			this._opHistory.execute(op);
		}
	}
};
this.InputCommands.reloadScript <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		foreach( script in args )
		{
			this.System.reloadScript(script);
		}
	}
};
this.InputCommands.reloadEnvironment <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	this.System.reloadScript("Environments");
	::_Environment.setForceNextUpdate(true);
	::_Environment.update();
};
this.InputCommands.rps <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		foreach( script in args )
		{
			this.System.reloadPSystem(script);
		}
	}
};
this.InputCommands.TimeofDay <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		local tod = args[0];

		if (tod == "off")
		{
			::_Environment.turnOffTimeOfDayOveride();
		}
		else
		{
			::_Environment.setOverrideTimeOfDay(tod);
		}
	}
};
this.InputCommands.downloadSize <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local result = "Name\tSize (Kilobytes)\tSize (Megabytes)\r\n\r\n";
	local total = 0;
	local archives = ::_contentLoader.getLoadedArchives();

	foreach( k, v in archives )
	{
		if (k in ::MediaIndex)
		{
			result += k + "\t" + ::MediaIndex[k][0] / 1024 + "\t" + this.Util.limitSignificantDigits(::MediaIndex[k][0].tofloat() / 1024 / 1024, 4) + "\r\n";
			total += ::MediaIndex[k][0];
		}
	}

	result += "\r\n\r\nTotal kilobytes\t" + total / 1024;
	result += "\r\nTotal megabytes\t" + total / 1024 / 1024;
	this.System.setClipboard(result);
	this.IGIS.info("Download sizes copied to clipboard.");
};
function sortLoadTimes( p1, p2 )
{
	if (p1 == p2)
	{
		return 0;
	}

	if (p1[1].total > p2[1].total)
	{
		return -1;
	}

	if (p1[1].total < p2[1].total)
	{
		return 1;
	}

	return 0;
}

this.InputCommands.loadTimes <- function ( args )
{
	if (!this.Util.hasPermission("debug"))
	{
		return;
	}

	local result = "Name\tTotal Time (Seconds)\tFetch Time (Seconds)\tLoad Time (Seconds)\r\n\r\n";
	local total = 0;
	local archives = ::_contentLoader.getLoadTimes();
	local sortedList = [];

	foreach( k, v in archives )
	{
		sortedList.append([
			k,
			v
		]);
	}

	this.Util.bubbleSort(sortedList, this.sortLoadTimes);

	foreach( e in sortedList )
	{
		result += e[0] + "\t" + e[1].total / 1000.0 + "\t" + e[1].fetch / 1000.0 + "\t" + e[1].load / 1000.0 + "\r\n";
		total += e[1].total;
	}

	result += "\r\nTotal seconds\t" + total / 1000.0;
	this.System.setClipboard(result);
	this.IGIS.info("Load times copied to clipboard.");
};
this.InputCommands.follow <- function ( args )
{
	if (::_avatar && ::_avatar.getTargetObject())
	{
		::_avatar.getController().startFollowing(::_avatar.getTargetObject(), false);
	}
};
function EvalCommand( text )
{
	if (::InputCommands.CheckCommand(text))
	{
		return;
	}

	if (this._Connection)
	{
		this._Connection.sendComm(text);
	}
}

this.InputCommandHelpers.getTokenRegExp <- function ( pString, pRegExp )
{
	local tokens = [];
	local result = pRegExp.capture(pString);

	if (result == null)
	{
		return null;
	}

	foreach( i, x in result )
	{
		if (i != 0)
		{
			tokens.append(pString.slice(x.begin, x.end));
		}
	}

	return tokens;
};
this.InputCommandHelpers.getTokenRegExpList <- function ( pString, pRegExpList )
{
	local tokens = [];

	foreach( i, x in pRegExpList )
	{
		tokens = this.InputCommandHelpers.getTokenRegExp(pString, x);

		if (tokens)
		{
			return tokens;
		}
	}
};
this.InputCommandHelpers.parseCommand <- function ( command )
{
	if (!command || command.len() <= 0)
	{
		throw this.Exception("Invalid command");
	}

	local tokens = ::InputCommandHelpers.parseLine(command.slice(1));

	if (tokens.len() == 0)
	{
		return null;
	}

	command = tokens[0];
	tokens.remove(0);
	return {
		cmd = command,
		args = tokens
	};
};
this.InputCommandHelpers.parseLine <- function ( line, ... )
{
	local parseStrings = true;

	if (vargc > 0)
	{
		parseStrings = vargv[0];
	}

	line = this.lstrip(this.rstrip(line));
	local tokens = [];
	local rx_string = this.regexp("^\"([^\"]*)\"");
	local rx_white = this.regexp("^\\s+");
	local rx_nonstring = this.regexp("^([^ \\t\\n\\r\\f\"]+)");
	local pos = 0;
	local len = line.len();

	while (pos < len)
	{
		local res;
		res = rx_white.search(line, pos);

		if (res != null)
		{
			pos = res.end;
			continue;
		}

		res = rx_string.search(line, pos);

		if (res != null)
		{
			tokens.append(line.slice(res.begin + 1, res.end - 1));
			pos = res.end + 1;
			continue;
		}

		res = rx_nonstring.search(line, pos);

		if (res != null)
		{
			tokens.append(line.slice(res.begin, res.end));
			pos = res.end;
			continue;
		}

		tokens.append("\"");
		pos++;
	}

	return tokens;
};
