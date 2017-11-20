require("Interpolator");
require("Util");
require("UI/MapDef");


/////////////////////////////////////////////////////////////////////////////
//
//  !!!
//
//  Make sure you update update the reference sheet when adding or changing
//  commands!
//     https://wiki.sparkplaymedia.com/display/EE/Controls+Reference
//  !!!
//
/////////////////////////////////////////////////////////////////////////////

InputCommands <- {};
InputCommandHelpers <- {};
LastCommand <- "s";
EchoTrigger <- "";

/**
	The following table is a list of regular expresions that can be used to
	identifiy and tokenize strings.  Any new expressions should be added to
	this list.
*/
InputRegExp <- {
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

/**
	This is the main function the processes slash commands.  The function checks to
	see if there is a client has a function with the same name as the slash
	command. If so it runs it, if not it returns false.
*/
function InputCommands::CheckCommand(pString) {
	local str = Util.trim(pString);

	if (str == "" || str == "/")
		return true;

	if (str[0] == 47) {
		// HACK: Argh, stupid hack for demo. We can't use normal
		// parsing because it uses special formatting.
		// TODO: remove this
		
		if (Util.hasPermission("setappearance") && Util.startsWith(str.tolower(), "/setappearance")) {
			InputCommands.setAppearance([
				str.slice(14)
			]);
			return true;
		}

		if (Util.startsWith(str.tolower(), "/syschat ")) {
			LastCommand = "*SysChat";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(9), LastCommand);
			return true;
		}

		if (Util.startsWith(str.tolower(), "/say ")) {
			LastCommand = "s";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(5), LastCommand);
			return true;
		}

		if (Util.startsWith(str.tolower(), "/s ")) {
			LastCommand = "s";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(3), LastCommand);
			return true;
		}

		if (Util.startsWith(str.tolower(), "/tell ") || Util.startsWith(str.tolower(), "/t ")) {
			local result;

			if (Util.startsWith(str.tolower(), "/tell"))
				result = Util.splitQuoteSafe(str.slice(6), " ");
			else
				result = Util.splitQuoteSafe(str.slice(3), " ");

			if (result.len() < 2)
				return false;

			local s = result[1];

			for( local i = 2; i < result.len(); ++i )
				s = s + " " + result[i];

			LastCommand = "t/" + result[0];
			_Connection.sendComm(s, LastCommand);

			if (Util.startsWith(result[0], "\""))
				result[0] = result[0].slice(1, result[0].len());

			if (result[0].slice(result[0].len() - 1, result[0].len()) == "\"")
				result[0] = result[0].slice(0, result[0].len() - 1);

			EchoTrigger = "yt/" + result[0];

			if (!::_avatar.hasStatusEffect(StatusEffects.GM_SILENCED))
				::_ChatManager.addMessage("yt/" + result[0], s);

			return true;
		}

		if (Util.startsWith(str.tolower(), "/p ")) {
			LastCommand = "party";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(3), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/party ")) {
			LastCommand = "party";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(7), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/clan ")) {
			LastCommand = "clan";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(6), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/emote "))	{
			LastCommand = "emote";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(7), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/em ") || Util.startsWith(str.tolower(), "/me")) {
			LastCommand = "emote";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(4), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/ch ")) {
			local result = Util.splitQuoteSafe(str.slice(4), " ");

			if (result.len() < 2)
				return false;

			local s = result[1];

			for( local i = 2; i < result.len(); ++i )
				s = s + " " + result[i];

			LastCommand = "ch/" + result[0];
			EchoTrigger = "";
			_Connection.sendComm(s, LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/gm ")) {
			LastCommand = "gm/earthsages";
			EchoTrigger = "";
			_Connection.sendComm(str.slice(4), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/trade "))	{
			LastCommand = "tc/" + ::_Connection.getCurrentRegionChannel();
			EchoTrigger = "";
			_Connection.sendComm(str.slice(7), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/region ")) {
			LastCommand = "rc/" + ::_Connection.getCurrentRegionChannel();
			EchoTrigger = "";
			_Connection.sendComm(str.slice(8), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/chat ")) {
			LastCommand = "rc/" + ::_Connection.getCurrentRegionChannel();
			EchoTrigger = "";
			_Connection.sendComm(str.slice(6), LastCommand);
			return true;
		}
		else if (Util.startsWith(str.tolower(), "/announce ")) {
			_Connection.sendQuery("util.sysannounce", NullQueryHandler(), [
				str.slice(10)
			]);
			return true;
		}

		local parsed = ::InputCommandHelpers.parseCommand(str);

		if (!parsed)
			return false;

		local cmd = parsed.cmd.tolower();

		// Find the command name case-insensitively.
		foreach( i, x in InputCommands ) {
			if (i.tolower() == cmd)	{
				InputCommands[i](parsed.args);
				return true;
			}
		}

		if (strcasecmp(cmd, "util.reprofession") == 0)
			_Connection.sendQuery(parsed.cmd, ReprofessionHandler(), parsed.args[0]);
		else if (strcasecmp(cmd, "util.swear.list") == 0)
			_Connection.sendQuery(parsed.cmd, SwearWordListHandler());
		else
			::_Connection.sendAction(parsed.cmd, parsed.args);

		return true;
	}

	_Connection.sendComm(str, LastCommand);

	if (EchoTrigger != "")
		::_ChatManager.addMessage(EchoTrigger, str);

	return true;
}

class PubSubQueryHandler 
{
	function onQueryComplete( qa, results ) {
		if (qa == null || qa.query == null)
			return;

		if (qa.query == "ps.join" && results[0][0] == "OK")
			IGIS.info("Joined channel " + qa.args[0] + " successfully");
		else if (qa.query == "ps.leave" && results[0][0] == "OK")
			IGIS.info("Left channel " + qa.args[0] + " successfully");
	}

	function onQueryError( qa, results ) {
		if (results)
			return;

		IGIS.error(results);
	}

}

QueryHandler <- PubSubQueryHandler();

function InputCommands::join(args) {
	if (args.len() == 2)
		_Connection.sendQuery("ps.join", QueryHandler, [
			args[0],
			md5(args[1])
		]);
	else if (args.len() == 1)
		_Connection.sendQuery("ps.join", QueryHandler, [
			args[0]
		]);
	else
		IGIS.error("Usage: /join <topic> [password]");
}

function InputCommands::leave(args) {
	if (args.len() == 1)
	{
		_Connection.sendQuery("ps.leave", QueryHandler, [
			args[0]
		]);
	}
	else
	{
		IGIS.error("Usage: /leave <topic>");
	}
}

function InputCommands::auditlog(args) {
	if (!Util.hasPermission("auditlog"))
	{
		return;
	}

	System.openURL("https://secure.sparkplaymedia.com/ee/auditlog/logview.php");
}

class VersionHandler extends DefaultQueryHandler {
	function onQueryComplete( qa, results )	{
		local text = "--[ Version Info ]--\n";

		foreach( row in results )
			text += row[0] + ": " + row[1] + "\n";

		IGIS.info(text);
		log.info(text);
	}
}

class KickHandler extends DefaultQueryHandler {
	function onQueryComplete( qa, results )	{
	}

	function onQueryError( qa, reason )	{
		IGIS.error(reason);
	}
}

class SimHandler extends DefaultQueryHandler {
	function onQueryComplete( qa, results )	{
		local text = "Simulator name: " + results[0][0];
		IGIS.info(text);
		log.info(text);
	}

}

class ReprofessionHandler extends DefaultQueryHandler {
	mUpdateAbilityWindow = false;
	constructor() {
		::_Connection.addListener(this);
	}

	function onQueryComplete( qa, results )	{
		mUpdateAbilityWindow = true;
		local str = resultAsString(qa, results);
		IGIS.info(str);
		log.info(str);
	}

	function onProfessionUpdate( value ) {
		if (mUpdateAbilityWindow) {
			::_AbilityManager.handleAbRespec();
			mUpdateAbilityWindow = false;
		}
	}

}

class PingHandler extends DefaultQueryHandler {
	type = "";
	startTime = 0;
	constructor( txt ) {
		type = txt;
		startTime = ::System.currentTimeMillis();
	}

	function onQueryComplete( qa, results )	{
		local text = type + " ping time: " + (::System.currentTimeMillis() - startTime) + " milliseconds.";
		IGIS.info(text);
		log.info(text);
	}

}

class SwearWordListHandler extends DefaultQueryHandler
{
	mSwearList = null;
	constructor() {
		mSwearList = {};
	}

	function onQueryComplete( qa, results )	{
		foreach( result in results )
		{
			local type = result[0];

			if (type in mSwearList)
				mSwearList[type].append(result[1]);
			else
			{
				local wordList = [];
				wordList.append(result[1]);
				mSwearList[type] <- wordList;
			}
		}

		saveAbilityData(mSwearList);
	}

	function getSwearWordPath()	{
		local basePath = _cache.getBaseURL();

		if (basePath.slice(0, 8) != "file:///")
			throw Exception("Swear word list path unavailble for base URL: " + basePath);

		basePath = basePath.slice(8);
		basePath += "/../../Media/Catalogs/";
		return basePath;
	}

	function saveAbilityData( results )	{
		local name = "SwearWordList";
		local filename = getSwearWordPath() + name + ".nut";
		local out = "SwearWordIndex <- " + serialize(results) + ";";
		::System.writeToFile(filename, out);
		::_ChatManager.loadSwearWordList(results);
	}

}

function InputCommands::version(args) {
	if (Util.hasPermission("version"))
		::_Connection.sendQuery("util.version", VersionHandler(), []);
}

function InputCommands::sim(args) {
	if (Util.hasPermission("version"))
		::_Connection.sendQuery("util.simaddress", SimHandler(), []);
}

function InputCommands::ping(args) {
	::_Connection.sendQuery("util.pingrouter", PingHandler("Router"), []);
	::_Connection.sendQuery("util.pingsim", PingHandler("Simulator"), []);
}

function InputCommands::pingSim(args) {
	::_Connection.sendQuery("util.pingsim", PingHandler("Simulator"), []);
}

function InputCommands::pingRouter(args) {
	::_Connection.sendQuery("util.pingrouter", PingHandler("Router"), []);
}

function InputCommands::ping2(args) {
	if (Util.hasPermission("dev"))
		Screens.toggle("PingScreen");
}

function InputCommands::kick(args) {
	::_Connection.sendQuery("util.kick", KickHandler(), args);
}

/**
	Quick helper for me to toggle creatures on or off for testing performance
	stuff.
*/
function InputCommands::creatureVis(args) {
	if (!Util.hasPermission("reassemble"))
	{
		return;
	}

	local vis = args.len() > 0 ? args[0] != "off" : true;
	local visFunc = function ( mo ) : ( vis )
	{
		mo.setVisible(vis);
	};

	foreach( id, c in _sceneObjectManager.mCreatures )
		Util.visitMovables(c.getNode(), visFunc);
}


/**
	This command will disconnect from the server
*/
function InputCommands::disconnect(args) {
	::_Connection.close();
}

/**
	This command will send a requrest to the avatar to play a animation.
*/
function InputCommands::emote(args) {
	local def = ::_avatar.getDef();

	if (args.len() > 0 && args[0] == "?") {
		local str = ::TXT("emote must be formated as follows: emote <emote>\n" + "Avalable Animations:\n");

		foreach( i, x in Util.tableKeys(::BipedAnimationDef.Animations, true) )	{
			if (i > 0)
				str += ", ";

			str += x;
		}

		IGIS.info(str);
		return;
	}

	if (args.len() > 0)	{
	
		// Wrap with content loader request because I've made them
		// load on demand to speed up initial startup...
		
		_contentLoader.load([
			"Biped-Anim-Emote"
		], ContentLoader.PRIORITY_REQUIRED, "Emote-AnimDeps", {
			function onPackageComplete( pkgName ) : ( args ) {
				::_avatar.mAnimationHandler.onFF(args[0]);
				::_Connection.sendComm(args[0], "emote");
			}

			function onPackageError( pkg, error ) {
				log.debug("Error loading package " + pkg + " - " + error);
				onPackageComplete(pkg);
			}
		});
		return true;
	}

	IGIS.error(::TXT("emote needs to be formated emote <emote name>"));
	return false;
}

function InputCommands::playSound(args) {
	if (!Util.hasPermission("playSound"))
		return;

	if (args.len() == 0)
		IGIS.error("Usage: /playSound <soundFile>");
	else
		_avatar.playSound(args[0]);
}

function InputCommands::compositor(args) {
	if (args.len() == 0 || !Util.hasPermission("compositor"))
		return;

	::_root.removeAllTargetCompositors();

	if (args[0].tolower() != "none")
	{
		::_root.addTargetCompositor(args[0]);
		::_root.setTargetCompositorEnabled(args[0], true);
	}
}

function InputCommands::showCollision(args) {
	if (!Util.hasPermission("debug"))
		return;

	local f = _scene.getVisibilityMask();
	local arg = args.len() > 0 ? args[0].tolower() : "";

	if (arg == "true" || arg == "on")
		f = f | VisibilityFlags.COLLISION;
	else if (arg == "false" || arg == "off")
		f = f | ~VisibilityFlags.COLLISION;
	else if (f & VisibilityFlags.COLLISION)
		f = f & ~VisibilityFlags.COLLISION;
	else
		f = f | VisibilityFlags.COLLISION;

	_scene.setVisibilityMask(f);
}

function InputCommands::unstick(args) {
	::_Connection.sendQuery("unstick", this);
}

function InputCommands::setFOV(args) {
	if (Util.hasPermission("debug"))
		Interpolate(_camera, "setFOVy", _camera.getFOVy(), args[0].tofloat() * Math.PI / 180);
}

function InputCommands::setPositionDebugObjects(args) {
	if (Util.hasPermission("debug"))
		gPositionDebugObjects = args[0].tointeger() != 0;
}

function InputCommands::setAmbientLight(args) {
	if (!Util.hasPermission("debug"))
		return;

	local a = 0.80000001;

	if (args.len() > 0)
		a = args[0].tofloat();

	_scene.setAmbientLight(Color(a, a, a));
}

function InputCommands::lag(args) {
	if (!Util.hasPermission("debug"))
		return;

	local LoopAmount = 100000;

	if (args.len() > 0)
		LoopAmount = args[0].tointeger();

	for( local i = 0; i < LoopAmount; i++ )
		log.debug("lag");
}

function InputCommands::setEnvironment(args) {
	if (!Util.hasPermission("debug"))
		return;

	if (args.len() > 0)
		_Environment.setOverride(args[0]);
	else
		_Environment.setOverride(null);
}

function InputCommands::setTimeOfDay(args) {
	if (!Util.hasPermission("debug"))
		return;

	if (args.len() > 0)
		_Environment.setTimeOfDay(args[0]);
	else
		_Environment.setTimeOfDay("Day");

	_Environment.update(0);
}

function InputCommands::scriptProfile(args) {
	if (!Util.hasPermission("debug"))
		return;

	if (args.len() > 0)
	{
		if (args[0] == "on")
			System.setScriptProfilerEnabled(true);
		else if (args[0] == "off")
			System.setScriptProfilerEnabled(false);
		else if (args[0] == "dump")
			log.info("Script Profile:\n" + System.getScriptProfile());
	}
	else
		IGIS.info("Usage: /scriptProfile (on|off|dump)");
}

function InputCommands::test(args) {
	::test(args.len() > 0 ? args[0] : "");
}

function InputCommands::gm(args) {
	Screens.toggle("GMScreen");
}

function InputCommands::petition(args) {
	Screens.toggle("PetitionScreen");
}

function InputCommands::earthsage(args) {
	Screens.toggle("PetitionScreen");
}

// This is used by the build puff ball particle effect
// to toggle it on/off in the build tool. :|
::_UIVisible <- true;

function InputCommands::toggleUI( args ) {
	::_UIVisible = !::_UIVisible;
	
	Screen.toggleOverlayForceInvisible(GUI.CONFIRMATION_OVERLAY);
	Screen.toggleOverlayForceInvisible(GUI.POPUP_OVERLAY);
	Screen.toggleOverlayForceInvisible("GUI/ChatOverlay");
	Screen.toggleOverlayForceInvisible("GUI/ChatWindowOverlay");
	Screen.toggleOverlayForceInvisible("GUI/Overlay");
	Screen.toggleOverlayForceInvisible("GUI/Overlay2");
	Screen.toggleOverlayForceInvisible("GUI/FullScreenComponentOverlay");
	Screen.toggleOverlayForceInvisible("GUI/IGISOverlay");
	Screen.toggleOverlayForceInvisible("GUI/TooltipOverlay");
	Screen.toggleOverlayForceInvisible("GUI/SelectionBox");
	Screen.toggleOverlayForceInvisible("GUI/QuickBarOverlay");
	Screen.toggleOverlayForceInvisible("GUI/EditBorderOverlay");
	Screen.toggleOverlayForceInvisible("GUI/CursorOverlay");
	Screen.toggleOverlayForceInvisible("GUI/DragOverlay");
	Screen.toggleOverlayForceInvisible("GUI/DebugScreen");
	Screen.toggleOverlayForceInvisible("GUI/ChatBubbleOverlay");
	Screen.toggleOverlayForceInvisible("GUI/MainUIOverlay");
	Screen.toggleOverlayForceInvisible("GUI/TargetOverlay");
	Screen.toggleOverlayForceInvisible("GUI/MiniMap");
	Screen.toggleOverlayForceInvisible("GUI/QuestTracker");
	Screen.toggleOverlayForceInvisible("GUI/TutorialOverlay");
	
	::_igisManager.setFloatieVisible(::_UIVisible);
	
	QuestIndicator.indicatorsVisibility(::_UIVisible);
}

function InputCommands::sceneryBrowser(args) {
	if (!Util.hasPermission("build"))
		return;

	local frame = Screens.SceneryObjectBrowser();
	frame.setVisible(true);
}

function InputCommands::setFullscreen( args ) {
	if (!Util.hasPermission("debug"))
		return;

	::Screen.setFullscreen(args[0].tolower() == "true" ? true : false);
}


/**
        Toggles range auto-attack on/off
*/

/**
        cicles between weapons
*/
function InputCommands::toggleWeapons(args) {
	local animated = true;

	if (::_avatar.hasStatusEffect(StatusEffects.AUTO_ATTACK) || ::_avatar.hasStatusEffect(StatusEffects.AUTO_ATTACK_RANGED))
		animated = false;

	local slot;

	switch(::_avatar.getVisibleWeapon())
	{
	case VisibleWeaponSet.NONE:
		if (::_avatar.hasWeaponSet(VisibleWeaponSet.MELEE))
			slot = VisibleWeaponSet.MELEE;
		else if (::_avatar.hasWeaponSet(VisibleWeaponSet.RANGED))
			slot = VisibleWeaponSet.RANGED;
		else
			return;

		break;

	case VisibleWeaponSet.MELEE:
		if (::_avatar.hasWeaponSet(VisibleWeaponSet.RANGED))
			slot = VisibleWeaponSet.RANGED;
		else
			slot = VisibleWeaponSet.NONE;

		break;

	case VisibleWeaponSet.RANGED:
	default:
		slot = VisibleWeaponSet.NONE;
	}

	if (slot == VisibleWeaponSet.NONE)
		::Audio.playSound("Sound-Ability-Sheath.ogg");
	else
		::Audio.playSound("Sound-Ability-Unsheath.ogg");

	::_avatar.setVisibleWeapon(slot, animated);
}

/**
        Set camera polygon mode to points, wireframe, or solid.  With
        no arguments, turns to wireframe if currently solid or to
        wireframe otherwise (i.e. toggling between wireframe and solid
        without cycling through points as well)

        polygonMode [solid|wireframe|points]

        Example: polygonMode

        Example: polygonMode points

        Use "polygonMode ?" to get ingame help.
*/
function InputCommands::togglePolygonMode(args) {
	if (!Util.hasPermission("debug"))
		return;

	local cam = _scene.getCamera("Default");

	if (args.len() > 0 && args[0] == "?")
		return;

	local mode = args.len() > 0 ? args[0].tolower() : "";

	switch(mode)
	{
	case "solid":
		cam.setPolygonMode(Camera.PM_SOLID);
		break;

	case "wireframe":
		cam.setPolygonMode(Camera.PM_WIREFRAME);
		break;

	case "points":
		cam.setPolygonMode(Camera.PM_POINTS);
		break;

	default:
		if (cam.getPolygonMode() == Camera.PM_SOLID)
			cam.setPolygonMode(Camera.PM_WIREFRAME);
		else
			cam.setPolygonMode(Camera.PM_SOLID);
	}
}

require("UI/MiniMapScreen");

/**
	Toggle minimap mode
*/
function InputCommands::minimap(args) {
	local screen = Screens.get("MiniMapScreen", false);

	if (screen == null)
		return;

	if (args.len() == 0)
	{
		screen.toggleMode();
		return;
	}

	switch(args[0])
	{
	case "small":
		screen.setMode(MINIMAP_SMALL);
		break;

	case "large":
		screen.setMode(MINIMAP_LARGE);
		break;

	case "off":
		screen.setMode(MINIMAP_OFF);
		break;
	}
}

function InputCommands::saveTerrain(args) {
	::_buildTool.saveTerrain();
}

function InputCommands::exportVegetation(args) {
	if (!Util.hasPermission("debug"))
		return;

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
					text += "minSize=\"" + e.minSize + "\" ";

				if ("maxSize" in e)
					text += "maxSize=\"" + e.maxSize + "\" ";

				if ("weight" in e)
					text += "weight=\"" + e.weight + "\" ";

				text += "/>\n";
			}
		}

		text += "\t</vegetation>\n";
	}

	text += "</tables>";
	System.writeToFile("../../../Server/resources/Vegetation.xml", text);
}

/**
	Take a screenshot and save it in the predefined path.
*/
/*
function InputCommands::screenshot(args) {
	// The default prefix of "Screenshot" is fine
    ::Screen.saveScreenshot();
}
*/

function InputCommands::searchRequests(args) {
	if (!Util.hasPermission("debug"))
		return;

	log.debug("----------------------------------------------");

	foreach( r in _contentLoader.mRequests )
		if (args.len() == 0 || r.mState == args[0])
			log.debug("STATE: " + r.mState + ", PACKAGE: " + r.mMedia);
}

/**
	Try and enter world building mode.
*/
function InputCommands::toggleBuilding(args) {

	if (Util.hasPermission("preview") == false)
		return;
		
	if(!Util.hasBuildPermission()) {
		IGIS.error("You have no build permission.");
		return;
	}

	if (!::_avatar)
		return;

	if (gToolMode == "play") {
		::_buildTool.setPreviewMode(true);
		_tools.setActiveTool(::_buildTool);
		gToolMode = "build";
	}
	else {
		_tools.setActiveTool(::_playTool);
		gToolMode = "play";
	}

	IGIS.info("Now in preview mode.");
}

/**
	Toggle the quickbar
*/
function InputCommands::toggleBuilding(args) {
	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
		return;

	if(!Util.hasBuildPermission()) {
		IGIS.error("You have no build permission.");
		return;
	}
	
	if (!Util.hasPermission("build")) {
		IGIS.error("You may not use that here.");
		return;
	}
	
	if (gToolMode == "play") {
		::_buildTool.setPreviewMode(false);
		_tools.setActiveTool(::_buildTool);
		gToolMode = "build";
	}
	else {
		_tools.setActiveTool(::_playTool);
		gToolMode = "play";
	}

	IGIS.info("Now in " + gToolMode + " mode.");
}


/**
	Toggle the quickbar
*/
function InputCommands::toggleQuickbar(args) {
	if (::_quickBarManager.getVisible())
		::_quickBarManager.hide();
	else
		::_quickBarManager.show();
}

function InputCommands::tss(args) {
	if (!Util.hasPermission("debug"))
		return;

	Screens.toggle("TeamSlayerScreen");
}

function InputCommands::ctfs(args) {
	if (!Util.hasPermission("debug"))
		return;

	Screens.toggle("CTFScreen");
}

function InputCommands::inv(args) {
	Screens.toggle("Inventory");
}

function InputCommands::macro(args) {
	if (!Util.hasPermission("debug"))
		return;

	Screens.toggle("MacroCreator");
}

function InputCommands::eq(args) {
	Screens.toggle("Equipment");
}

function InputCommands::itemAppearance(args) {
	if (Util.hasPermission("tweakScreens") == false)
		return;

	Screens.toggle("ItemAppearanceTweak");
}

function InputCommands::scriptTest(args) {
	if (!Util.hasPermission("scriptTest"))
		return;

	Screens.toggle("ScriptTest");
}

function InputCommands::map(args) {
	Screens.toggle("MapWindow");
}

function InputCommands::qj(args) {
	Screens.toggle("QuestJournal");
}

function InputCommands::dumpResourceUsage(args) {
	if (!Util.hasPermission("debug"))
		return;

	local verbose = false;

	if (args.len() > 0)
		verbose = args[0] == "verbose";

	::_root.dumpResourceUsage(verbose);
}

function InputCommands::resize(args) {
	USE_OLD_SCREEN = !USE_OLD_SCREEN;
}

function InputCommands::stats(args) {
	if (!Util.hasPermission("debug"))
		return;

	local obj = ::_avatar.getTargetObject();
	local stats = ::_avatar.getTargetObject().getStats();

	if (stats)
		foreach( k, v in stats )
			log.debug(::Stat[k].name + ": " + v);
	else
		log.debug("No stats!");
}

function InputCommands::spawnMaster(args) {
	::Screens.toggle("SpawnMasterScreen", true);
}

function InputCommands::myColors(args) {
	if (!Util.hasPermission("build"))
		return;

	::Screens.toggle("CustomColorsScreen", true);
}

function InputCommands::TailSize(args) {
	local nscale = _getVector(args);
	local asm = ::_avatar.getAssembler();
	foreach(i, d in asm.mDetails) {
		if(d.point == "tail")
			d["scale"] <- nscale;
	}
	::_avatar.reassemble();
}

function InputCommands::EarSize(args) {
	local nscale = _getVector(args);
	local asm = ::_avatar.getAssembler();
	foreach(i, d in asm.mDetails)
	{
		if(d.point == "left_ear" || d.point == "right_ear")
			d["scale"] <- nscale;
	}
	::_avatar.reassemble();
}

function InputCommands::SaveSize(args) {
	//Format:
	/*
	["sizepref"] = {
		[charid]={tail=[x,y,z], ear=[x,y,z]},
		[charid]={...},
		...
	};
	*/

	local fullPref = _ModPackage.GetPref("body_customize");
	if(!fullPref)
		fullPref = {};

	local thisPref = {};

	local asm = ::_avatar.getAssembler();
	foreach(i, d in asm.mDetails)
	{
		if(d.point == "tail") {
			if("scale" in d)
				thisPref["tail"] <- [d.scale.x, d.scale.y, d.scale.z];
		}
		else if(d.point == "left_ear") {
			if("scale" in d)
				thisPref["ear"] <- [d.scale.x, d.scale.y, d.scale.z];
		}
	}
	local cdefid = ::_avatar.getType();
	fullPref[cdefid] <- thisPref;

	_ModPackage.SetPref("body_customize", fullPref);

	IGIS.info("Saved.");
}

function InputCommands::RemoveSize(args) {
	local fullPref = _ModPackage.GetPref("body_customize");
	if(!fullPref)
		fullPref = {};
	local cdefid = ::_avatar.getType();
	local found = false;
	foreach(i, d in fullPref) {
		if(i == cdefid) {
			delete fullPref[i];
			//fullPref.remove(i);
			found = true;
			break;
		}
	}

	_ModPackage.SetPref("body_customize", fullPref);

	if(found == true)
		IGIS.info("Removed this character's saved body customization. It will be normal when you next restart the client.");
	else
		IGIS.info("This character does not have a saved body customization.");
}

function InputCommands::cping(args) {
	if(args.len() == 0)
		_DiagnosticPings.PrintStatistics();
	else
		_DiagnosticPings.SetWarning(args[0].tointeger());
}

function InputCommands::ItemLinks(args) {
	local notFound = false;
	if(args.len() > 0) {
		if(!(args[0] in ::InventoryMapping))
			notFound = true;
	}
	if(args.len() == 0 || notFound == true)	{
		local ostr = "";
		foreach(i, d in ::InventoryMapping)
		{
			if(ostr.len() > 0)
				ostr += ",";
			ostr += i;
		}
		IGIS.info("No container specified. Use [" + ostr + "]");
		return;
	}

	local container = ::_ItemDataManager.getContents(args[0]);
	if(!container.hasAllItems() )
		return;

	local extraData = false;
	if(args.len() > 1)
		extraData = true;

	local arr = [];

	local resultStr = "";
	local count = 0;
	foreach(itemId in container.mContents) {
		// The iterator, itemId, is a hex string of the inventory container/slot data
		// for that item.  Because string sorts don't work properly, convert to integer.
		// But keep it associated to its hex ID so we can use it to look up the item.

		local tableEntry = {intId = ::atoi(itemId, 16), hexId = itemId};
		arr.append(tableEntry);
	}
	arr.sort(SortItemArray);
	if(args[0] == "buyback")
		arr.reverse();

	local ostr = "";
	local count = 0;
	foreach(i, d in arr) {
		// The ItemData() object returned by getItem() contains the information of a
		// particular inventory slot, including look ID (refashioned) bind status,
		// stack count, etc.
		// The ItemDefData() object returned by getItemDef() contains the server
		// properties of a specific item.  Name, flavor text, armor, damage,
		// strength, etc.

		local slotId = d.hexId;
		local itemData = ::_ItemDataManager.getItem(slotId);
		local itemDef = ::_ItemDataManager.getItemDef(itemData.mItemDefId);

		// Fill out the [item] tag information from ID, Display Name, and Look ID.

		local itemStr = "[item]" + itemDef.mID + ":" + itemDef.mDisplayName;
		if(itemData.mItemLookDefId != 0)
			itemStr += ":" + itemData.mItemLookDefId;
		itemStr += "[/item]";

		if(extraData == true)
		{
			itemStr += " Lv:" + itemDef.mLevel;
			local qlev = itemDef.mQualityLevel;
			switch(itemDef.mQualityLevel)
			{
			case 3: itemStr += " Rare"; break;
			case 4: itemStr += " Epic"; break;
			case 5: itemStr += " Legendary"; break;
			case 6: itemStr += " Artifact"; break;
			}
		}
		ostr += itemStr + "\r\n";
		count++;
	}
	if(count > 0) {
		::System.setClipboard(ostr);
		::IGIS.info( "Copied " + count + " item entries to clipboard.");
	}
	else
		IGIS.info("No items in container.");
}

function InputCommands::SaveChat(args) {
	local tabname = "none";
	if(args.len() >= 1)
		tabname = args[0].tostring().tolower();

	local ostr = "";
	local tabs = 0;
	local messages = 0;
	ostr = "<html>\r\n<head><title>Chat Log</title></head>\r\n";
	ostr += "<body style=\"color:#FFFFFF; background-color:#000000\">\r\n";
	foreach( i, x in ::_ChatWindow.mTabContents ) {
		if(tabname == "none" || x.mName.tolower() == tabname)
		{
		tabs++;

		ostr += "<h1>" + x.mName + "</h1>\r\n";
		foreach(li, ld in x.mLog)
		{
			local color = _ChatManager.getColor(ld.channel);
			ostr += "<div style=\"color:#" + color + "\">";

			ostr += ld.message + "</div>\r\n";
			messages++;
		}
		ostr += "<br><br><br>\r\n";

		} //End tab check
	}
	ostr += "</body>\r\n</html>\r\n";
	::System.setClipboard( ostr );
	::IGIS.info( "Chat log saved to clipboard (" + messages + " messages in " + tabs + " tabs).");
}

function InputCommands::MarketEdit(args) {
	local frame = this.Screens.ItemMarketEditScreen();
	frame.setVisible(true);
}

function InputCommands::DumpBipedAnims(args) {
	print("{\n");
	foreach(k, v in ::BipedAnimationDef) {		
		print("[\"" + k + "\"] = " + serialize(v) + "\n");
	}
	print("}\n");
}
 
function InputCommands::mod(args) {
	Screens.show("ModPanel");
}

function InputCommands::tb(args) {
	toggleBuilding(args);
}

function InputCommands::sb(args) {
	Screens.show("SceneryObjectBrowser");
}

function InputCommands::ah(args) {
	Screens.show("AuctionHouse");
}

function InputCommands::sc(args) {
	showCollision(args);
}

function InputCommands::cb(args) {
	creatureBrowse(args);
}



//
//
//
//
// TODO everything below this point needs to be reformatted
//

InputCommands.hideChat <- function ( args )
{
	::Screens.hide("ChatWindow");
};
InputCommands.zoneTweak <- function ( args )
{
	if (Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local frame = Screens.ZoneTweakScreen();
	frame.setPosition(200, 100);
	frame.setOverlay(GUI.OVERLAY);
	frame.setVisible(true);
};
class TweakCreature extends DefaultQueryHandler
{
	mCreating = false;
	constructor( ... )
	{
		if (vargc > 0)
		{
			mCreating = vargv[0];
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
						::_Connection.sendQuery("creature.def.edit", TweakCreature(true), [
							"NEW",
							"name",
							name
						]);
					}
				}

			};
			GUI.MessageBox.showYesNo("The creature type \"" + qa.args[0] + "\" does not exist. " + "Do you want to create it?", callback);
			return;
		}

		local row = results[0];
		local id = row[0].tointeger();
		local a = GetAssembler("Creature", id);

		if (mCreating)
		{
			a.setStat(Stat.DISPLAY_NAME, qa.args[2]);
		}

		local ct = Screens.show("CreatureTweakScreen");
		ct.setCurrentType(id);
	}

}

InputCommands.pages <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	log.debug("Total pages: " + _sceneObjectManager.mPages.len());

	foreach( p in _sceneObjectManager.mPages )
	{
		local s = p.getState();

		switch(s)
		{
		case PageState.PENDINGREQUEST:
			s = "Pending Request";
			break;

		case PageState.REQUESTED:
			s = "Requested";
			break;

		case PageState.LOADING:
			s = "Loading";
			break;

		case PageState.READY:
			s = "Ready";
			break;

		case PageState.ERRORED:
			s = "Errored";
			break;
		}

		log.debug("Page " + p.getX() + ", " + p.getZ() + ": " + s);
	}
};
InputCommands.bug <- function ( args )
{
	Screens.show("BugReport");
};
InputCommands.terrain <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	log.debug("Total terrain pages: " + _sceneObjectManager.mLoadedTerrain.len());

	foreach( k, v in _sceneObjectManager.mLoadedTerrain )
	{
		log.debug("Terrain Page " + v.x + ", " + v.z + ": " + v.mode);
	}
};
InputCommands.sd <- function ( args )
{
	local tmp = [
		"WARP_TARGET"
	];
	tmp.extend(args);
	::_Connection.sendQuery("go", NullQueryHandler(), tmp);
};
InputCommands.minimapCreatureCategory <- function ( args )
{
	if (args.len() < 1)
	{
		foreach( creatureCategory in LegendItemCreatures )
		{
			::LegendItemSelected[creatureCategory] = true;
		}
	}
	else
	{
		local filteredCreatureCategory = args[0];

		foreach( creatureCategory in LegendItemCreatures )
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

	local legendSelectedTable = deepClone(::LegendItemSelected);
	::Pref.set("map.LegendItems", legendSelectedTable);
	::Util.updateMiniMapStickers();
};
InputCommands.minimapShopkeepers <- function ( args )
{
	if (args.len() < 1)
	{
		return;
	}

	if (args[0].tolower() == "1")
	{
		::LegendItemSelected[LegendItemTypes.SHOP] = true;
	}
	else
	{
		::LegendItemSelected[LegendItemTypes.SHOP] = false;
	}

	local legendSelectedTable = deepClone(::LegendItemSelected);
	::Pref.set("map.LegendItems", legendSelectedTable);
	::Util.updateMiniMapStickers();
};
InputCommands.setAppearance <- function ( args )
{
	local appearance = args.len() > 0 ? Util.trim(args[0]) : "";

	if (appearance != "")
	{
		_avatar.getAssembler().setStat(Stat.APPEARANCE, appearance);
	}
};
InputCommands.links <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	::_scene.setLinksVisible(!::_scene.getLinksVisible());
};
InputCommands.abilityScreen <- function ( args )
{
	local as = Screens.get("AbilityFrame", false);

	if (as && as.isVisible())
	{
		as.setVisible(false);
	}
	else
	{
		as = Screens.show("AbilityFrame");
	}
};
InputCommands.partyScreen <- function ( args )
{
	local ps = Screens.get("PartyScreen", false);

	if (ps && ps.isVisible())
	{
		ps.setVisible(false);
	}
	else
	{
		ps = Screens.show("PartyScreen");
	}
};
InputCommands.iconBrowser <- function ( args )
{
	if (Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local ib = Screens.get("IconBrowserScreen", false);

	if (ib && ib.isVisible())
	{
		ib.setVisible(false);
	}
	else
	{
		ib = Screens.show("IconBrowserScreen");
	}
};
InputCommands.creatureDebug <- function ( args )
{
	if (Util.hasPermission("tweakScreens") == false)
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

		local creatureDebug = Screens.show("CreatureDebugScreen");
		creatureDebug.setCurrentType(id);
	}
	else
	{
		local creatureDebug = Screens.get("CreatureDebugScreen", false);

		if (creatureDebug && creatureDebug.isVisible())
		{
			creatureDebug.setVisible(false);
		}
		else
		{
			creatureDebug = Screens.show("CreatureDebugScreen");

			if (_avatar.getTargetObject())
			{
				creatureDebug.setCurrentType(_avatar.getTargetObject().getType());
			}
			else
			{
				creatureDebug.setCurrentType(_avatar.getType());
			}
		}
	}
};
InputCommands.creatureTweak <- function ( args )
{
	if (Util.hasPermission("tweakScreens") == false)
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
			_Connection.sendQuery("creature.def.list", TweakCreature(), [
				"",
				args[0]
			]);
		}
		else
		{
			local ct = Screens.show("CreatureTweakScreen");
			ct.setCurrentType(id);
		}
	}
	else
	{
		local ct = Screens.get("CreatureTweakScreen", false);

		if (ct && ct.isVisible())
		{
			ct.setVisible(false);
		}
		else
		{
			ct = Screens.show("CreatureTweakScreen");

			if (_avatar.getTargetObject())
			{
				ct.setCurrentType(_avatar.getTargetObject().getType());
			}
			else
			{
				ct.setCurrentType(_avatar.getType());
			}
		}
	}
};
InputCommands.creatureBrowse <- function ( args )
{
	if (Util.hasPermission("tweakScreens") == false)
	{
		return;
	}

	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	Screens.show("CreatureBrowserScreen");
};
InputCommands.shaky <- function ( args )
{
	if (args.len() != 3)
	{
		IGIS.error("usage: /shaky amount time range");
		return;
	}

	::_playTool.addShaky(::_avatar.getPosition(), args[0].tofloat(), args[1].tofloat(), args[2].tofloat());
};
InputCommands.markers <- function ( args )
{
	if (!Util.hasPermission("tweakScreens"))
	{
		return;
	}

	Screens.show("MarkerTweakScreen");
};
InputCommands.inventory <- function ( args )
{
	Screens.toggle("Inventory");
};
InputCommands.shards <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	::Screens.toggle("ShardSelectScreen");
};
InputCommands.friends <- function ( args )
{
	::Screens.toggle("SocialWindow");
};
InputCommands.undo <- function ( args )
{
	if (!Util.hasPermission("build"))
	{
		return;
	}

	if (_opHistory.canUndo())
	{
		IGIS.info("Undoing: " + _opHistory.getUndoPresentationName());

		try
		{
			_opHistory.undo();
		}
		catch( err )
		{
			IGIS.error("Error during undo: " + err);
		}
	}
	else
	{
		IGIS.info("Nothing to redo (or last operation not undoable).");
	}
};
InputCommands.redo <- function ( args )
{
	if (!Util.hasPermission("build"))
	{
		return;
	}

	if (_opHistory.canRedo())
	{
		IGIS.info("Redoing: " + _opHistory.getRedoPresentationName());

		try
		{
			_opHistory.redo();
		}
		catch( err )
		{
			IGIS.error("Error during redo: " + err);
		}
	}
	else
	{
		IGIS.info("Nothing to redo.");
	}
};
InputCommands.debug <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		local p = "debug." + args[0];

		if (Pref.isDeclared(p))
		{
			if (Pref.get(p) == Pref.getDefault(p))
			{
				Pref.set(p, !Pref.getDefault(p));
			}
			else
			{
				Pref.set(p, null);
			}
		}
	}
	else
	{
		Screens.toggle("DebugScreen");
	}
};
InputCommands.dps <- function ( args )
{
	Screens.toggle("DPSMeter");
};
InputCommands.updateAbilities <- function ( args )
{
	if (!Util.hasPermission("dev"))
	{
		return;
	}

	::_AbilityManager.handleUpdatingAbilities(::AbilityIndex, false);
};
InputCommands.updateClientAbilities <- function ( args )
{
	if (!Util.hasPermission("dev"))
	{
		return;
	}

	::_AbilityHelper.updateClientAbilities();
};
InputCommands.importExcel <- function ( args )
{
	if (Util.hasPermission("importAbilities") == false)
	{
		return;
	}

	Screens.show("AbEditScreen").onImport(null);
};
InputCommands.te <- function ( args )
{
	InputCommands.testEffect(args);
};
InputCommands.testEffect <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() == 0)
	{
		IGIS.error("Usage: /testEffect Script1[,Script2,...]");
		return;
	}

	if (!(args[0] in ::EffectDef))
	{
		::IGIS.error("Invalid effect name: " + args[0]);
		return;
	}

	System.reloadScript("EffectDef");
	_contentLoader.load([
		"Biped-Anim-Combat",
		"Biped-Anim-Emote"
	], ContentLoader.PRIORITY_REQUIRED, "Effect-AnimDeps", {
		function onPackageComplete( pkgName ) : ( args )
		{
			try
			{
				::_avatar.cue(args[0]);
			}
			catch( err )
			{
				log.error("Error cuing effect: " + err);
			}
		}

		function onPackageError( pkg, error )
		{
			log.debug("Error loading package " + pkg + " - " + error);
			onPackageComplete(pkg);
		}

	});
};
InputCommands.reassemble <- function ( args )
{
	if (!Util.hasPermission("reassemble"))
	{
		return;
	}

	foreach( k, v in _sceneObjectManager.mScenery )
	{
		v.reassemble();
	}

	foreach( k, v in _sceneObjectManager.mCreatures )
	{
		v.reassemble();
	}
};
InputCommands.copyPos <- function ( args )
{
	if (!Util.hasPermission("debug"))
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
		::IGIS.info("Position copied to clipboard");
	}
	else
	{
		::IGIS.error("Cannot copy position to clipboard");
	}
};
InputCommands.hide <- function ( args )
{
	if (_buildTool && _buildTool.isInBuildMode())
	{
		if (_buildTool.isShowingTerrainOnly())
		{
			_buildTool.setShowTerrainOnly(false);
			local previousFlags = _buildTool.getPreviousVisibilityFlags();
			_scene.setVisibilityMask(previousFlags);
		}
		else
		{
			_buildTool.setShowTerrainOnly(true);
			_buildTool.setPreviousVisibilityFlags(_scene.getVisibilityMask());
			_scene.setVisibilityMask(VisibilityFlags.SCENERY & ~VisibilityFlags.WATER);
		}

		print("SHOWING TERRAIN ONLY");
	}
};
InputCommands.hideNonMimimapProps <- function ( args )
{
	if (_buildTool && _buildTool.isInBuildMode())
	{
		if (_buildTool.isShowingTerrainOnly())
		{
			_buildTool.setShowTerrainOnly(false);
			local previousFlags = _buildTool.getPreviousVisibilityFlags();
			_scene.setVisibilityMask(previousFlags);
		}
		else
		{
			_buildTool.setShowTerrainOnly(true);
			_buildTool.setPreviousVisibilityFlags(_scene.getVisibilityMask());
			_scene.setVisibilityMask(VisibilityFlags.PROPS | VisibilityFlags.SCENERY | VisibilityFlags.CREATURE | VisibilityFlags.ATTACHMENT | VisibilityFlags.WATER | VisibilityFlags.ANYTHING | VisibilityFlags.HELPER_GEOMETRY);
		}

		print("SHOWING TERRAIN ONLY");
	}
};
InputCommands.resetIndicators <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	::QuestIndicator.updateCreatureIndicators();
};
InputCommands.resetQuests <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	::_questManager.reset();
	::QuestIndicator.updateCreatureIndicators();
	local questJournal = ::Screens.get("QuestJournal", true);
	questJournal.updateCharacterSavedQuestMarkers();
	::_useableCreatureManager.refreshCache();
};
InputCommands.rq <- function ( args )
{
	InputCommands.resetQuests(args);
};
InputCommands.configVideo <- function ( args )
{
	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName != "GameState")
	{
		return;
	}

	::Screens.toggle("VideoOptionsScreen");
};
InputCommands.browserScript <- function ( args )
{
	if (("runBrowserScript" in ::System) && args.len() > 0)
	{
		System.runBrowserScript(args[0]);
	}
};
InputCommands.reloadTextures <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		foreach( pattern in args )
		{
			System.forceTextureRefresh(pattern);
		}
	}
	else
	{
		System.forceTextureRefresh();
	}
};
InputCommands.shop <- function ( args )
{
	local target = ::_avatar.getTargetObject();

	if (target)
	{
		Screens.get("ItemShop", true).setMerchantId(target.getID());
		Screens.show("ItemShop", true);
	}
	else
	{
		log.debug("You must select an NPC first.");
	}
};
InputCommands["do"] <- function ( args )
{
	if (args.len() == 0)
	{
		return;
	}

	local flags = 0;

	if (args.len() > 1 && args[1] == "party")
	{
		flags = AbilityFlags.PARTY_CAST;
	}

	local ab = _AbilityManager.getAbilityByName(args[0]);

	if (ab != null && ab.getIsValid())
	{
		if (ab.getActions().find("GTAE") == null)
		{
			ab.sendActivationRequest(true, flags);
		}
		else if (_groundTargetTool)
		{
			local actions = ab.getActions();
			local indexOfGTAE = actions.find("GTAE");
			local indexOfOpenParam = actions.find("(", indexOfGTAE);
			local indexOfCloseParam = actions.find(")", indexOfOpenParam);
			local size = actions.slice(indexOfOpenParam + 1, indexOfCloseParam).tointeger();
			_groundTargetTool.setSize(size * 2, size * 2);
			_groundTargetTool.setAbility(ab);
			_tools.push(_groundTargetTool);
		}
	}
};
InputCommands.stop <- function ( args )
{
	local ab = _AbilityManager.getAbilityByName(args[0]);

	if (ab.getIsValid())
	{
		ab.sendActivationRequest(false);
	}
};
InputCommands.switchATS <- function ( args )
{
	if (!Util.hasPermission("build"))
	{
		return;
	}

	if (args.len() > 0)
	{
		local ats = args[0];
		local sel = ::_buildTool.getSelection().objects();
		local op = CompoundOperation();
		local count = 0;

		foreach( so in sel )
		{
			if ("ATS" in so.mVars)
			{
				local a = so.getVarsTypeAsAsset();
				a.mVars.ATS = ats;
				op.add(BeanSetPropertyOp(so, "asset", a));
				count++;
			}
		}

		if (count > 0)
		{
			op.setPresentationName("SwitchATS to " + ats + " (x " + op.len() + ")");
			_opHistory.execute(op);
		}
	}
};
InputCommands.reloadScript <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		foreach( script in args )
		{
			System.reloadScript(script);
		}
	}
};
InputCommands.reloadEnvironment <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	System.reloadScript("Environments");
	::_Environment.setForceNextUpdate(true);
	::_Environment.update();
};
InputCommands.rps <- function ( args )
{
	if (!Util.hasPermission("debug"))
	{
		return;
	}

	if (args.len() > 0)
	{
		foreach( script in args )
		{
			System.reloadPSystem(script);
		}
	}
};
InputCommands.TimeofDay <- function ( args )
{
	if (!Util.hasPermission("debug"))
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
InputCommands.downloadSize <- function ( args )
{
	if (!Util.hasPermission("debug"))
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
			result += k + "\t" + ::MediaIndex[k][0] / 1024 + "\t" + Util.limitSignificantDigits(::MediaIndex[k][0].tofloat() / 1024 / 1024, 4) + "\r\n";
			total += ::MediaIndex[k][0];
		}
	}

	result += "\r\n\r\nTotal kilobytes\t" + total / 1024;
	result += "\r\nTotal megabytes\t" + total / 1024 / 1024;
	System.setClipboard(result);
	IGIS.info("Download sizes copied to clipboard.");
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

InputCommands.loadTimes <- function ( args )
{
	if (!Util.hasPermission("debug"))
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

	Util.bubbleSort(sortedList, sortLoadTimes);

	foreach( e in sortedList )
	{
		result += e[0] + "\t" + e[1].total / 1000.0 + "\t" + e[1].fetch / 1000.0 + "\t" + e[1].load / 1000.0 + "\r\n";
		total += e[1].total;
	}

	result += "\r\nTotal seconds\t" + total / 1000.0;
	System.setClipboard(result);
	IGIS.info("Load times copied to clipboard.");
};
InputCommands.follow <- function ( args )
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

	if (_Connection)
	{
		_Connection.sendComm(text);
	}
}

InputCommandHelpers.getTokenRegExp <- function ( pString, pRegExp )
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
InputCommandHelpers.getTokenRegExpList <- function ( pString, pRegExpList )
{
	local tokens = [];

	foreach( i, x in pRegExpList )
	{
		tokens = InputCommandHelpers.getTokenRegExp(pString, x);

		if (tokens)
		{
			return tokens;
		}
	}
};
InputCommandHelpers.parseCommand <- function ( command )
{
	if (!command || command.len() <= 0)
	{
		throw Exception("Invalid command");
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
InputCommandHelpers.parseLine <- function ( line, ... )
{
	local parseStrings = true;

	if (vargc > 0)
	{
		parseStrings = vargv[0];
	}

	line = lstrip(rstrip(line));
	local tokens = [];
	local rx_string = regexp("^\"([^\"]*)\"");
	local rx_white = regexp("^\\s+");
	local rx_nonstring = regexp("^([^ \\t\\n\\r\\f\"]+)");
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


function InputCommands::_getVector(args)
{
	if(typeof args != "array")
		args = [1.0];

	local sizex = 1.0;
	local sizey = 1.0;
	local sizez = 1.0;
	if(args.len() == 1)
	{
		local n = args[0].tofloat();
		sizex = n;
		sizey = n;
		sizez = n;
	}
	else if(args.len() >= 3)
	{
		sizex = args[0].tofloat();
		sizey = args[1].tofloat();
		sizez = args[2].tofloat();
	}
	return Vector3(sizex, sizey, sizez)
}
