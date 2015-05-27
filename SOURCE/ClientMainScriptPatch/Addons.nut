/*

MOD FILE FOR PLANET FOREVER

*/

/* NOTE  When hex editing the States/CharacterSelectionState.cnut file for increased character slots,
  Modify these addresses:
00001DD0: 04 05
00001E18: 04 05

Squirrel instructions
Inst#  Offset      8 bytes of args
0369:00001DC8:0000007E 01 04 00 00  :      LOAD  [4] = MAX_CHARACTERS
0379:00001E18:00000004 02 05 00 00  :   LOADINT _loc5 = 4
*/


require("Assemblers/Assembler");
require("Assemblers/CreatureAssembler");
require("Tools/PlayTool");
require("Util");
require("InputCommands");
require("Constants");
require("Globals");
require("ServerConstants");  //For Stat.HEALTH type hack.
require("Connection");

//Hack to make Health use integer values instead of shorts.
Stat[Stat.HEALTH]["type"] <- "int";

//gServerListenDistance <- 1200.0;  //This is the destroy distance, don't want to alter this.
//gCreatureVisibleRange <- 900.0;

//Hack to assist detection of idle connections during the load phase.
//If a server's simulator connection has not received a message recently,
//it will send a heartbeat message to ping the client.  The acknowledgement
//phase will send back a dummy response so the server can recognize at least
//one recent incoming packet.
ProtocolDef[0][90] <- "_handleHeartbeatMessage";
ProtocolDef[0][100] <- "_handleModMessage";
ProtocolDef[0]["acknowledgeHeartbeat"] <- 20;
ProtocolDef[1][100] <- "_handleModMessage";
ProtocolDef[1]["acknowledgeHeartbeat"] <- 20;

//Hack for diagnostic pings.
ProtocolDef[1]["debugServerPing"] <- 19;
ProtocolDef[1][119] <- "_handleDebugServerPing";

WeaponTypeClassRestrictions <-
{
[WeaponType.NONE] 	= {none = false, knight = false, rogue = false, mage = false, druid = false},
[WeaponType.SMALL]	= {none = false, knight = false, rogue = true,  mage = true,  druid = false},
[WeaponType.ONE_HAND]	= {none = false, knight = true,	 rogue = true,  mage = false, druid = true},
[WeaponType.TWO_HAND]	= {none = false, knight = true,	 rogue = false, mage = false, druid = false},
[WeaponType.POLE]	= {none = false, knight = false, rogue = false, mage = true, druid = true},   //mage was false
[WeaponType.WAND]	= {none = false, knight = false, rogue = false, mage = true,  druid = true},  //druid was false
[WeaponType.BOW]	= {none = false, knight = true,	 rogue = true, mage = false, druid = true},   //rogue was false
[WeaponType.THROWN]	= {none = false, knight = true,  rogue = true,  mage = true, druid = false},  //knight and mage false
[WeaponType.ARCANE_TOTEM]={none = false, knight = true,	 rogue = true,  mage = true,  druid = true},
}

function PlayTool::_increaseAvatarSpeed()
{
	if(Util.hasSpeedPermission())
		_avatar.getController().onIncreaseAvatarSpeed();
}

function PlayTool::_decreaseAvatarSpeed()
{
	if(Util.hasSpeedPermission())
		_avatar.getController().onDecreaseAvatarSpeed();
}


function Util::hasPermission(permission)
{
	return true;
}

function Util::hasSpeedPermission()
{
	return ::_accountPermissionGroup == "admin";
}

function Util::hasBuildPermission()
{
	if(Util.hasSpeedPermission())
		return true;

	local zonePart = Util.split(::_avatar.mZoneID, "-");
	if(zonePart.len() < 2)
		return false;
	if(zonePart[1].tointeger() >= 5000)
		return true;

	return false;
}


function InputCommands::togglePreview_mod(args)
{
	if(Util.hasBuildPermission() == false)
		IGIS.error("You may not use that here.");
	else
		_togglePreview(args);
}

function InputCommands::toggleBuilding_mod(args)
{

	if(Util.hasBuildPermission() == false)
		IGIS.error("You may not use that here.");
	else
		_toggleBuilding(args);
}

::InputCommands["_togglePreview"] <- ::InputCommands["togglePreview"];
::InputCommands["togglePreview"] <- ::InputCommands["togglePreview_mod"];

::InputCommands["_toggleBuilding"] <- ::InputCommands["toggleBuilding"];
::InputCommands["toggleBuilding"] <- ::InputCommands["toggleBuilding_mod"];


class Screens.ModPanel extends GUI.Frame
{
	static mClassName = "Screens.ModPanel";

	mButtonGroveTools = null;
	mButtonPropSearch = null;
	mButtonPropGenerator = null;
	mButtonEasyATS = null;

	mButtonModSettings = null;
	mButtonIGF = null;
	mButtonItemPreview = null;
	mButtonInstanceScript = null;
	mButtonEmoteBrowser = null;
	mButtonPetBrowser = null;

	constructor()
	{
		GUI.Frame.constructor("Mod Panel");

		local cmain = GUI.Container(GUI.BoxLayoutV());

		mButtonGroveTools = _createButton("Grove Tools");
		mButtonGroveTools.setTooltip(_createTooltip("/GT", "Ctrl+F5"));

		mButtonPropSearch = _createButton("Prop Search");
		mButtonPropSearch.setTooltip(_createTooltip("/PS", "Ctrl+F6"));

		mButtonPropGenerator = _createButton("Prop Generator");
		mButtonPropGenerator.setTooltip(_createTooltip("/PG", "Ctrl+F7"));

		mButtonEasyATS = _createButton("Easy ATS");
		mButtonEasyATS.setTooltip(_createTooltip("/EATS", "Ctrl+F8"));

		mButtonModSettings = _createButton("Mod Settings");
		mButtonModSettings.setTooltip(_createTooltip("/chatMod"));

		mButtonIGF = _createButton("In-Game Forum");
		mButtonIGF.setTooltip(_createTooltip("/IGF", "Ctrl+F9"));

		mButtonItemPreview = _createButton("Item Preview");
		mButtonItemPreview.setTooltip(_createTooltip("Ctrl+F10"));

		mButtonInstanceScript = _createButton("Instance Script");
		mButtonInstanceScript.setTooltip(_createTooltip("/iscript"));

		mButtonEmoteBrowser = _createButton("Emote Browser");
		mButtonEmoteBrowser.setTooltip(_createTooltip("/pose"));

		mButtonPetBrowser = _createButton("Pet Browser");
		mButtonPetBrowser.setTooltip(_createTooltip("/pet"));

		cmain.add(GUI.Spacer(0, 10));
		cmain.add(GUI.Label("Hover to see shortcut command."));
		cmain.add(GUI.Spacer(0, 10));
		cmain.add(mButtonGroveTools);
		cmain.add(mButtonPropSearch);
		cmain.add(mButtonPropGenerator);
		cmain.add(mButtonEasyATS);
		cmain.add(mButtonInstanceScript);
		cmain.add(GUI.Spacer(0, 15));
		cmain.add(mButtonModSettings);
		cmain.add(GUI.Spacer(0, 15));
		cmain.add(mButtonIGF);
		cmain.add(mButtonItemPreview);
		cmain.add(GUI.Spacer(0, 15));
		cmain.add(mButtonEmoteBrowser);
		cmain.add(mButtonPetBrowser);

		setContentPane(cmain);
		setSize(200, 430);
		centerOnScreen();
	}
	function _highlightWrapper(text)
	{
		return "<font color=\"00FFFF\"><b>" + text + "</b></font>";
	}
	function _createTooltip(shortcut, ...)
	{
		local text = "<font size=\"24\">";
		text += "Shortcut: " + _highlightWrapper(shortcut);
		for(local i = 0; i < vargc; i++)
		{
			text += " OR " + _highlightWrapper(vargv[i]);
		}

		text += "</font>"
		return text;
	}
	function _createButton(name)
	{
		local button = GUI.NarrowButton(name);
		button.setFixedSize(160, 32);
		button.addActionListener(this);
		button.setReleaseMessage("onButtonPressed");
		return button;
	}
	function onButtonPressed(button)
	{
		if(button == mButtonGroveTools)
			Screens.show("GroveTools");
		else if(button == mButtonPropSearch)
			Screens.show("PropSearch");
		else if(button == mButtonPropGenerator)
			Screens.show("PropGenerator");
		else if(button == mButtonEasyATS)
			Screens.show("EasyATS");
		else if(button == mButtonModSettings)
			Screens.show("ModSettings");
		else if(button == mButtonIGF)
			Screens.show("IGForum");
		else if(button == mButtonItemPreview)
			Screens.show("PreviewItem");
		else if(button == mButtonEmoteBrowser)
			Screens.show("EmoteBrowser");
		else if(button == mButtonPetBrowser)
			Screens.show("PetScreen");
	}
}

function InputCommands::mod(args)
{
	Screens.show("ModPanel");
}

function InputCommands::tb(args)
{
	toggleBuilding(args);
}

function InputCommands::sb(args)
{
	Screens.show("SceneryObjectBrowser");
}

function InputCommands::sc(args)
{
	showCollision(args);
}

function InputCommands::cb(args)
{
	creatureBrowse(args);
}



// Hack to set the proper domain domain name when connecting to an arbitrary
// port (other than HTTP port 80).
// Ex: http://example.com:81/Release/Current/EarthEternal.car
// Originally it doesn't support arbitrary port numbers.
// Derived from code used in the original attemptToConnect() function.
// Also allows a router port override hack.
function ModGetCustomRouterPort()
{
	if("router" in ::_args)
	{
		return ::_args["router"].tointeger();
	}

	try
	{
		local t = unserialize( _cache.getCookie("Router") );
		if(t)
		{
			if("mod.router" in t)
				return t["mod.router"].tointeger();
		}
	}
	catch(e)
	{
	}
	
	return 4242;   //Corresponds to hardcoded default.
}

function Connection::attemptToConnectHack()
{
	local domain = _cache.getBaseURL();
	local custom = false;

	//Base URL will look something like this
	//http://localhost/Release/Current

	if( domain.find("://") != null )
	{
		domain = Util.split(domain, "://")[1];
		
		if( domain.find("@") != null )
		{
			domain = Util.split(domain, "@")[1];
		}
		
		if( domain.find("/") != null )
		{
			domain = Util.split(domain, "/")[0];
		}
		
		if(domain.find(":") != null)
		{
			custom = true;
			domain = Util.split(domain, ":")[0];
		}

		local routerPort = ModGetCustomRouterPort();

		if(custom == true)
		{
			mCurrentHost = domain + ":" + routerPort;
			
			log.info( "Connecting to " + mCurrentHost );
			log.info( "[MOD] Custom domain port detected.");
			if(routerPort != 4242)
				log.info( "[MOD] Custom router port detected.");
				
			if( Util.isDevMode() )
				Screen.setTitle("Earth Eternal (" + mCurrentHost + ")");
			socket.connect(domain, routerPort, 0);
			return;
		}
	}
	
	//Hack didn't work, call original
	attemptToConnectOriginal();
}

::Connection["attemptToConnectOriginal"] <- ::Connection["attemptToConnect"];
::Connection["attemptToConnect"] <- ::Connection["attemptToConnectHack"];






function Connection::_handleProtocolChangedMsg_hack(data)
{
	// This is just a convenient place to intercept a loading point in the game
	// to load our custom preferences.
	_ModPackage.mFirstLoadScreen = false;
	::_ModPackage.LoadPref();
	_handleProtocolChangedMsg_old(data);
}

::Connection["_handleProtocolChangedMsg_old"] <- ::Connection["_handleProtocolChangedMsg"];
::Connection["_handleProtocolChangedMsg"] <- ::Connection["_handleProtocolChangedMsg_hack"];


function GetVector(args)
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


function InputCommands::TailSize(args)
{
	local nscale = GetVector(args);
	local asm = ::_avatar.getAssembler();
	foreach(i, d in asm.mDetails)
	{
		if(d.point == "tail")
			d["scale"] <- nscale;
	}
	::_avatar.reassemble();
}

function InputCommands::EarSize(args)
{
	local nscale = GetVector(args);
	local asm = ::_avatar.getAssembler();
	foreach(i, d in asm.mDetails)
	{
		if(d.point == "left_ear" || d.point == "right_ear")
			d["scale"] <- nscale;
	}
	::_avatar.reassemble();
}

function InputCommands::SaveSize(args)
{
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
		if(d.point == "tail")
		{
			if("scale" in d)
				thisPref["tail"] <- [d.scale.x, d.scale.y, d.scale.z];
		}
		else if(d.point == "left_ear")
		{
			if("scale" in d)
				thisPref["ear"] <- [d.scale.x, d.scale.y, d.scale.z];
		}
	}
	local cdefid = ::_avatar.getType();
	fullPref[cdefid] <- thisPref;

	_ModPackage.SetPref("body_customize", fullPref);

	IGIS.info("Saved.");
}

function InputCommands::RemoveSize(args)
{
	local fullPref = _ModPackage.GetPref("body_customize");
	if(!fullPref)
		fullPref = {};
	local cdefid = ::_avatar.getType();
	local found = false;
	foreach(i, d in fullPref)
	{
		if(i == cdefid)
		{
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



function Connection::_handleModMessage(data)
{
	local event = data.getByte();
	switch(event)
	{
	case 1:
		if(::_ModPackage.mSettingSupercritDisabled == false)
			::_playTool.addShaky(::_avatar.getPosition(), 100, 0.5, 100);
		break;
	case 2:
		local actorID = data.getInteger();
	 	local emoteName = data.getStringUTF();
	 	local emoteSpeed = data.getFloat();
	 	local loop = (data.getByte() != 0);
	 	local c = ::_sceneObjectManager.hasCreature(actorID);
        if( c )
        {
        	local animHandler = c.getAnimationHandler( );
    		if ( animHandler )
       			animHandler.onFF( emoteName, emoteSpeed, loop );
        }
        break;
	case 3:
		local actorID = data.getInteger();
		local event = data.getByte();

	 	local c = ::_sceneObjectManager.hasCreature(actorID);
        if( c )
        {
        	local animHandler = c.getAnimationHandler( );
    		if ( animHandler )
    		{
    			if(event == 1)
					animHandler.fullStop();
				else if(event == 2)
				{
					animHandler.mEntity.getAnimationUnit(0).setTimeScaleFactor(0.000001);
					animHandler.mPaused = true;
				}
    		}
        }
        break;
	case 4:
		local assetPkg = data.getStringUTF();
		local soundFile = data.getStringUTF();
		::Audio.playArchiveSound(assetPkg, soundFile);
		break;
	case 20:
		local time = data.getInteger();
		_DiagnosticPings.Start(time);
		break;
	case 21:
		_DiagnosticPings.Stop();
		break;
	case 22:
		_DiagnosticPings.QueryStatistics();
		break;
	case 23:  //The server is asking for a generic response.  Very simplistic way of checking for client info.
		local reqStr = data.getStringUTF();
		local response = [];
		response.append(reqStr);
		if(::_avatar)
		{
			response.append(::_avatar.getStat(Stat.DISPLAY_NAME, true));
			response.append(::_avatar.getStat(Stat.LEVEL, true));
		}
		::_Connection.sendQuery("mod.genericResponse", NullQueryHandler(), response);
		break;
	case 30:   //Display a message box to the user.
		local msg = data.getStringUTF();
		GUI.MessageBox.show(msg);
		break;
	case 40:  //Force stop swimming.  This helps prevent the glitch of teleporting out of a high elevation body of water, to swim over lower terrain.
		if(::_avatar)
		{
			::_avatar.mSwimming = false;
			::_avatar.mController.onStopSwimming();
		}
		break;
	}
}


// Hack to the heartbeat function to return a response message to the server.
// The server needs a better way to detect when the client has dropped.
function Connection::_handleHeartbeatMessage(data)
{
	local timeElapsed = data.getInteger();
	::_gameTime.updateGameTime(timeElapsed);

	_beginSend("acknowledgeHeartbeat");
	_send();
}

function Connection::_handleCommunicationMsg_mod(data)
{
	if(::_ModPackage.mSettingChatSoundEnabled == true)
	{
		local speakerID = data.getInteger();
	 	local speakerName = data.getStringUTF();

		local creature = ::_sceneObjectManager.getCreatureByID(speakerID);
		if(speakerName == null)
		{
			if(creature != null)
				speakerName = creature.getName();
		}
		if(speakerID != ::_avatar.getID())
			if(Util.isInIgnoreList(speakerName) == false)
				::_avatar.playSound(::_ModPackage.mSettingChatSoundFile);

		data.rewind();
	}
	_handleCommunicationMsg_old(data);
}


// DIAGNOSTIC PINGS
function Connection::_handleDebugServerPing(data)
{
	//If we get this message, the client must respond to a ping initiated by the server.
	local MessageID = data.getInteger();
 	local InitialSendTime = data.getInteger();

	_beginSend("debugServerPing");
	mOutBuf.putInteger(MessageID);
	mOutBuf.putInteger(InitialSendTime);
	_send();
}

class DiagnosticPings
{
	//Controls operation of the pinging system
	mOperational = false;
	mFireDelay = 1.0;
	mWarningTime = 0;

	//Tracking info
	mPingID = 0;
	mSuccessCount = 0;
	mFailCount = 0;
	mTimeoutCount = 0;

	mLowestTime = 0;
	mHighestTime = 0;
	mTotalTime = 0;
	mReceivedCount = 0;

	function Start(iTimeMS)
	{
		if(mOperational == true)
			return;

		mOperational = true;
		mFireDelay = iTimeMS / 1000.0;
		Trigger();
	}
	function Stop()
	{
		mOperational = false;
	}
	function Trigger()
	{
		if(mOperational == false)
			return;

		if(::_Connection.isConnected() == false)
		{
			mOperational = false;
			return;
		}
		if(::_Connection.isPlaying() == false)
		{
			mOperational = false;
			return;
		}
		::_eventScheduler.fireIn(mFireDelay, this, "Trigger");

		//The server is already set up to respond to regular pings used elsewhere.
		::_Connection.sendQuery( "util.pingsim", this, [++mPingID, System.currentTimeMillis()]);
	}
	function onQueryComplete(qa, results)
	{
		mSuccessCount++;
		local timeDiff = System.currentTimeMillis() - qa.args[1].tointeger();
		if((mLowestTime == 0) || (timeDiff < mLowestTime))
			mLowestTime = timeDiff;
		else if((mHighestTime == 0) || (timeDiff > mHighestTime))
			mHighestTime = timeDiff;

		if((mWarningTime != 0) && (timeDiff > mWarningTime))
			IGIS.info("Client detected a ping of " + timeDiff + " ms.");

		mTotalTime += timeDiff;
		mReceivedCount++;
	}
	function onQueryError(qa, error)
	{
		mFailCount++;
		mReceivedCount++;
	}
	function onQueryTimeout(qa)
	{
		mTimeoutCount++;
	}
	function QueryStatistics()
	{
		::_Connection.sendQuery( "mod.ping.statistics", NullQueryHandler(), [mPingID, mSuccessCount, mFailCount, mTimeoutCount, mLowestTime, mHighestTime, mTotalTime, mReceivedCount]);
	}
	function WriteStatistic(sString, sLabel, iValue)
	{
		if(sString.len() != 0)
			sString += "  ";
		sString += sLabel + ":" + iValue;
		return sString;
	}
	function PrintStatistics()
	{
		local str = "";
		local average = 0;
		if(mReceivedCount != 0)
			average = mTotalTime / mReceivedCount;
		str = WriteStatistic(str, "Pings", mPingID);
		str = WriteStatistic(str, "Lowest", mLowestTime);
		str = WriteStatistic(str, "Highest", mHighestTime);
		str = WriteStatistic(str, "Avg", average);
		if(mFailCount != 0)
			str = WriteStatistic(str, "Failed", mFailCount);
		if(mTimeoutCount != 0)
			str = WriteStatistic(str, "Timed Out", mTimeoutCount);
		IGIS.info(str);
	}
	function SetWarning(iTimeMS)
	{
		if(iTimeMS >= 0)
		{
			mWarningTime = iTimeMS;
			if(mWarningTime == 0)
				IGIS.info("Client ping time notification is now off.");
			else
				IGIS.info("Client ping time notification set to " + mWarningTime + " ms.");
		}
		else
		{
			mPingID = 0;
			mSuccessCount = 0;
			mFailCount = 0;
			mTimeoutCount = 0;

			mLowestTime = 0;
			mHighestTime = 0;
			mTotalTime = 0;
			mReceivedCount = 0;
			IGIS.info("Client ping statistics cleared.");
		}
	}
}

_DiagnosticPings <- DiagnosticPings();

function InputCommands::cping(args)
{
	if(args.len() == 0)
		_DiagnosticPings.PrintStatistics();
	else
		_DiagnosticPings.SetWarning(args[0].tointeger());
}

// END DIAGNOSTIC PINGS


::Connection["_handleCommunicationMsg_old"] <- ::Connection["_handleCommunicationMsg"];
::Connection["_handleCommunicationMsg"] <- ::Connection["_handleCommunicationMsg_mod"];




class URLManager
{
	mLoaded = false;
	mURLs = {};
	mQueryCalled = false;
	mPendingTag = "";

	constructor()
	{
	}
	function LaunchURL(tagName)
	{
		if(mLoaded == false)
		{
			LoadURLs();
			mPendingTag = tagName;
			return;
		}

		if(tagName in mURLs)
		{
			System.openURL(mURLs[tagName]);
		}
		else
		{
			if(mLoaded == true)
			{
				GUI.MessageBox.show("The server administrator has not provided a URL for:<br>" + tagName);
			}
		}
	}

	function onProtocolChanged(newProto)
	{
		GUI.MessageBox.show("TEST");
		if(newProto == "Play" )
		{
			IGIS.info("PROTOCOL:" + newProto);
			if(mLoaded == false)
				LoadURLs();
		}
	}
	function LoadURLs()
	{
		if(mURLs == null)
			mURLs = {};

		FetchURLs();
	}
	function FetchURLs()
	{
		if(mQueryCalled == false)
		{
			::_Connection.sendQuery("mod.getURL", this, []);
			mQueryCalled = true;
		}
	}

	function onQueryComplete(qa, results)
	{
		if(mURLs == null)
			mURLs = {};
		foreach(row in results)
		{
			if(row.len() >= 2)
			{
				mURLs[row[0]] <- row[1];
			}
		}
		mLoaded = true;
		if(mPendingTag != "")
		{
			LaunchURL(mPendingTag);
			mPendingTag = "";
		}
	}
	function onQueryError(qa, error)
	{
	}
}

_URLManager <- URLManager();

function SortItemArray(a, b)
{
	if(a.intId > b.intId) return 1;
	else if(a.intId < b.intId) return -1;
	return 0;
}

function InputCommands::ItemLinks(args)
{
	local notFound = false;
	if(args.len() > 0)
	{
		if(!(args[0] in ::InventoryMapping))
			notFound = true;
	}
	if(args.len() == 0 || notFound == true)
	{
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
	foreach(itemId in container.mContents)
	{
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
	foreach(i, d in arr)
	{
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
	if(count > 0)
	{
		::System.setClipboard(ostr);
		::IGIS.info( "Copied " + count + " item entries to clipboard.");
	}
	else
	{
		IGIS.info("No items in container.");
	}
}

function InputCommands::SaveChat(args)
{
	local tabname = "none";
	if(args.len() >= 1)
		tabname = args[0].tostring().tolower();

	local ostr = "";
	local tabs = 0;
	local messages = 0;
	ostr = "<html>\r\n<head><title>Chat Log</title></head>\r\n";
	ostr += "<body style=\"color:#FFFFFF; background-color:#000000\">\r\n";
	foreach( i, x in ::_ChatWindow.mTabContents )
	{
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

//
// EM - Trying to decompile Second Object fully ...
//

class this.SceneObject extends this.MessageBroadcaster
{
	mID = -1;
	mAlwaysVisible = null;
	mObjectClass = null;
	mZoneID = null;
	mSceneryLayer = "";
	mCreatureDef = null;
	mType = "";
	mVars = null;
	mDef = null;
	mOpacity = -1.0;
	mFlags = 0;
	mTimeSinceLastUpdate = 0;
	mNode = null;
	mFadeEnabled = true;
	mAssemblingNode = null;
	mDeathAppearanceChangeEvent = null;
	mAnimationState = null;
	mAssembler = null;
	mPickedAssembler = null;
	mVelocityUpdatePending = false;
	mVelocityUpdateSchedule = null;
	mAssemblyData = null;
	mAssembling = false;
	mAssembled = false;
	mAssemblyQueueTime = null;
	mAnimationHandler = null;
	mController = null;
	mEffectsHandler = null;
	mMorphEffectsHandler = null;
	mSlopeSlideInertia = null;
	mDead = false;
	mNeedToRunDeathAnim = false;
	mHasLoot = false;
	mInCombat = false;
	mAttemptingCombat = false;
	mLastServerUpdate = null;
	mCurrentlyJumping = false;
	mSpeed = 0.0;
	mHeading = 0.0;
	mRotation = 0.0;
	mVerticalSpeed = 0.0;
	mObjectBelowAvatar = false;
	mDownwardAcceleration = 250.0;
	mDistanceToFloor = 0.0;
	mShadowVisible = false;
	mDestroying = false;
	mResetTabTargetting = false;
	mShadowDecal = null;
	mShadowProjector = null;
	mNameInRange = false;
	mNamePlatePosition = null;
	mNamePlateScale = null;
	UPDATE_TIMEOUT = 30000;
	mTimeoutEnabled = true;
	mNormalSize = 1.0;
	mTimer = null;
	mCastingEndTime = 0.0;
	mCastingWarmupTime = 0.0;
	mUsingAbilityID = 0;
	mSelectionProjector = null;
	mSelectionNode = null;
	mStats = null;
	mIsSunVisible = true;
	mShowName = null;
	mNameBoard = null;
	mShowHeadLight = false;
	mHeadLight = null;
	mStatusEffects = null;
	mStatusModifiers = null;
	mUniqueBuffs = null;
	mUniqueStatusEffects = null;
	mUniqueStatusModifiers = null;
	mCarryingFlag = false;
	mFlag = null;
	mStartNormal = null;
	mEndNormal = null;
	mInterpolateFramesLeft = -1;
	FRAMES_TO_INTERPOLATE = 15;
	FORCE_UPDATE = 100;
	mInteractParticle = null;
	mInteractParticleNode = null;
	UP_DOWN_SLOPE_ANGLE = 0.34999999;
	mFloorAlignMode = this.FloorAlignMode.NONE;
	mCurrentlyOriented = false;
	mLastNormal = this.Vector3();
	mTargetObject = null;
	mAttachments = null;
	mAttachmentOverride = null;
	mWeapons = null;
	mPreviousWeaponSet = this.VisibleWeaponSet.INVALID;
	mVisibleWeaponSet = this.VisibleWeaponSet.INVALID;
	mMeleeAutoAttackActive = false;
	mRangedAutoAttackActive = false;
	mLODLevel = -1;
	mPercentToNextLOD = 1.0;
	mProperties = {};
	mGarbagificationTime = null;
	mGeometryPage = null;
	mSoundEmitters = null;
	LOCKED = 1;
	PRIMARY = 2;
	mTypeString = "";
	mCurrentFadeLevel = 0.0;
	mDesiredFadeLevel = 1.0;
	mFadeTarget = 1.0;
	mCorking = false;
	mCorkedStatusEffects = null;
	mCorkedStatusModifiers = null;
	mCorkedFloaties = null;
	mCorkedChatMessage = null;
	mCorkTimeout = 0;
	mGone = false;
	mAbilityEffect = null;
	mSwimming = false;
	mWaterElevation = 0.0;
	mQuestIndicator = null;
	mWidth = null;
	mHeight = null;
	mLastYPos = 0;
	mClickBox = null;
	mBaseStats = null;
	mFloorAlignOrientation = null;
	mForceShowEquipment = false;
	mIsScenery = false;
	mPreviousAsset = null;
	mPreviousScale = null;
	
	constructor( pID, objectClass )
	{
		::MessageBroadcaster.constructor();
		this.mIsScenery = objectClass == "Scenery";
		this.mObjectClass = objectClass;
		this.mProperties = {};
		this.mStatusModifiers = [];
		this.mStatusEffects = {};
		this.mUniqueBuffs = [];
		this.mUniqueStatusEffects = [];
		this.mUniqueStatusModifiers = [];
		this.mAttachments = {};
		this.mAttachmentOverride = {};
		this.mWeapons = {};
		this.mCorkedFloaties = [];
		this.mCorkedChatMessage = [];

		if (pID == null && objectClass == "Dummy")
		{
			this.mID = ::_DummyCount;
			::_DummyCount++;
		}
		else
		{
			this.mID = pID;
		}

		this.mDef = {};
		this.mNode = ::_scene.createSceneNode(this.mObjectClass + "/" + this.mID);
		::_scene.getRootSceneNode().addChild(this.mNode);

		if (this.mObjectClass == "Creature")
		{
			if (typeof this.mID == "integer")
			{
				this._Connection.sendInspectCreature(this.mID);
			}

			this.setShowingShadow(true);
			this.prepareAssemblingNode();
			this.mTimer = ::Timer();
			this.mBaseStats = {};
			this.mBaseStats[this.Stat.STRENGTH] <- 0;
			this.mBaseStats[this.Stat.DEXTERITY] <- 0;
			this.mBaseStats[this.Stat.CONSTITUTION] <- 0;
			this.mBaseStats[this.Stat.PSYCHE] <- 0;
			this.mBaseStats[this.Stat.SPIRIT] <- 0;
		}

		this.mMorphEffectsHandler = this.EffectsHandler(this);
		this.mEffectsHandler = this.EffectsHandler(this);

		if (objectClass == "Dummy")
		{
			this._setAssembled(true);
		}

		::_scene.updateLinks();
	}

	function setZoneID( zoneID )
	{
		this.mZoneID = zoneID;
	}

	function resetTimeSinceLastUpdate()
	{
		this.mTimeSinceLastUpdate = 0;
	}

	function getZoneID()
	{
		return this.mZoneID;
	}

	function getZoneDefId()
	{
		if (this.mZoneID != null && this.mZoneID != "")
		{
			local splitZoneString = this.Util.split(this.mZoneID, "-");

			if (splitZoneString.len() == 3)
			{
				return splitZoneString[1].tointeger();
			}
		}

		return -1;
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "creature.isusable")
		{
			local result = results[0][0];

			if (result == "Q")
			{
				this.addInteractParticle();
			}
			else if (result == "D")
			{
				this.removeInteractParticle();
			}
			else
			{
				this.removeInteractParticle();
			}

			::_useableCreatureManager.addCreatureDef(qa.args[0], result);

			if (::_playTool == null)
			{
				return;
			}

			local cursorpos = this.Screen.getCursorPos();
			local so = this._sceneObjectManager.pickSceneObject(cursorpos.x, cursorpos.y, this.QueryFlags.ANY);

			if (so != null && so.getCreatureDef() != null && so.getCreatureDef().getID() == qa.args[0].tointeger())
			{
				::_playTool.updateMouseCursor(so);
			}
			else
			{
				::_playTool.updateMouseCursor(null);
			}
		}
	}

	function setFadeEnabled( which )
	{
		this.mFadeEnabled = which;
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "scenery.edit")
		{
			::_ChatWindow.addMessage("err/", error, "General");
		}
		else if (qa.query == "creature.use")
		{
			::IGIS.error(error);
		}
	}

	function onSetAvatar()
	{
		this.mVelocityUpdateSchedule = ::_eventScheduler.fireIn(0.80000001, this, "sendPendingVelocityUpdate");
	}

	function queryUsable()
	{
		if (this.hasStatusEffect(this.StatusEffects.IS_USABLE))
		{
			::_Connection.sendQuery("creature.isusable", this, this.getID());
		}
	}

	function setProperties( values )
	{
		this.mProperties = values;
	}

	function getAssemblyQueueTime()
	{
		return this.mAssemblyQueueTime;
	}

	function showErrorDetails()
	{
		local error = this.getAssemblerError();
		this.GUI.MessageBox.show(error ? error.tostring() : "No Error");
	}

	function addProperty( name, value )
	{
		this.mProperties[name] = value;
		local args = [];
		args.append(this.mID);
		args.append(name);
		args.append("" + value);
		this._Connection.sendQuery("scenery.edit", {}, args);
	}

	function getProperties()
	{
		return this.mProperties;
	}

	function isPropCreature()
	{
		if (this.isCreature() && this.mAssembler && ("mSceneryAssembler" in this.mAssembler) && this.mAssembler.getSceneryAssembler())
		{
			return true;
		}

		return false;
	}

	function hasStatusEffect( id )
	{
		return id in this.mStatusEffects;
	}

	function setForceShowWeapon( visible )
	{
		this.mForceShowEquipment = visible;
	}

	function isForceShowWeapon()
	{
		return this.mForceShowEquipment;
	}

	function setStatusModifiers( mods, effects )
	{
		local reztest_WasDead = this.StatusEffects.DEAD in this.mStatusEffects;
		local reztest_IsDead = this.StatusEffects.DEAD in effects;
		local meleeAutoAttackOn = this.StatusEffects.AUTO_ATTACK in this.mStatusEffects;
		local rangedAutoAttackOn = this.StatusEffects.AUTO_ATTACK_RANGED in this.mStatusEffects;

		if (this.mCorking)
		{
			if (::_avatar == this && this.mDead != reztest_IsDead)
			{
				if (reztest_WasDead)
				{
					this.log.debug("REZ: corked: dead status changed: Rezed!");
				}
				else
				{
					this.log.debug("REZ: corked: dead status changed: Died!");
				}
			}

			this.mCorkedStatusEffects = effects;
			this.mCorkedStatusModifiers = mods;
			return;
		}

		if (::_avatar == this && this.mDead != reztest_IsDead)
		{
			if (reztest_WasDead)
			{
				this.log.debug("REZ: dead status changed: Rezed!");
			}
			else
			{
				this.log.debug("REZ: dead status changed: Died!");
			}
		}

		local previousEffects = this.mStatusEffects;
		local previousModifiers = this.mStatusModifiers;
		this.mStatusEffects = effects;
		this.mStatusModifiers = mods;

		if (this == ::_avatar)
		{
			if (meleeAutoAttackOn)
			{
				if (!(this.StatusEffects.AUTO_ATTACK in this.mStatusEffects))
				{
					this.stopAutoAttack(false);
				}
			}

			if (rangedAutoAttackOn)
			{
				if (!(this.StatusEffects.AUTO_ATTACK_RANGED in this.mStatusEffects))
				{
					this.stopAutoAttack(true);
				}
			}
		}

		local buffDebuffScreen;

		if (::_avatar == this)
		{
			buffDebuffScreen = this.Screens.get("PlayerBuffDebuff", false);
		}
		else
		{
			local partyMemberComponent = ::partyManager.getMemberComponent(this.mID);

			if (partyMemberComponent)
			{
				buffDebuffScreen = partyMemberComponent;
			}
		}

		local targetBuffDebuffScreen;

		if (::_avatar != null && ::_avatar.getTargetObject() == this)
		{
			targetBuffDebuffScreen = this.Screens.get("TargetWindow", false);
		}

		this.mUniqueBuffs = [];
		this.mUniqueStatusModifiers = [];

		foreach( mod in this.mStatusModifiers )
		{
			local modExists = false;

			foreach( existingBuff in this.mUniqueBuffs )
			{
				if (mod.getAbilityID() == existingBuff.getAbilityID())
				{
					existingBuff.addStatusModifier(mod);
					modExists = true;
					break;
				}
			}

			if (!modExists)
			{
				local abilityInfo = ::_AbilityManager.getAbilityById(mod.getAbilityID());
				local newBuff = this.BuffDebuff(this, mod, abilityInfo.mForegroundImage + "|" + abilityInfo.mBackgroundImage);
				this.mUniqueBuffs.append(newBuff);
			}
		}

		this.mUniqueStatusEffects = [];

		foreach( k, v in this.mStatusEffects )
		{
			if (v)
			{
				local newStatusEffect = this.StatusEffect(k);
				this.mUniqueStatusEffects.append(newStatusEffect);
			}
		}

		if (this.hasStatusEffect(this.StatusEffects.UNATTACKABLE))
		{
			if (!(this.StatusEffects.UNATTACKABLE in previousEffects))
			{
				this.updateNameBoardColor();
			}
		}
		else if (this.StatusEffects.UNATTACKABLE in previousEffects)
		{
			this.updateNameBoardColor();
		}

		if (this.hasStatusEffect(this.StatusEffects.INVINCIBLE))
		{
			if (!(this.StatusEffects.INVINCIBLE in previousEffects))
			{
				this.updateNameBoardColor();
			}
		}
		else if (this.StatusEffects.INVINCIBLE in previousEffects)
		{
			this.updateNameBoardColor();
		}

		if (this.hasStatusEffect(this.StatusEffects.DAZE) && !(this.StatusEffects.DAZE in previousEffects))
		{
			this.setAbilityEffect(this.cue("Daze"));
		}
		else if (!this.hasStatusEffect(this.StatusEffects.DAZE) && this.StatusEffects.DAZE in previousEffects)
		{
			this.interruptAbility(-1);
		}

		if (this.hasStatusEffect(this.StatusEffects.STUN) && !(this.StatusEffects.STUN in previousEffects))
		{
			this.setAbilityEffect(this.cue("Stun"));
		}
		else if (!this.hasStatusEffect(this.StatusEffects.STUN) && this.StatusEffects.STUN in previousEffects)
		{
			this.interruptAbility(-1);
		}

		this.mUniqueStatusModifiers = this.mUniqueBuffs;

		foreach( statusEffect in this.mUniqueStatusEffects )
		{
			this.mUniqueStatusModifiers.append(statusEffect);
		}

		if (buffDebuffScreen)
		{
			buffDebuffScreen.updateMods(this.mUniqueStatusModifiers);
		}

		if (targetBuffDebuffScreen)
		{
			targetBuffDebuffScreen.updateMods(this);
		}

		foreach( effectToAdd in this.mUniqueStatusEffects )
		{
			if (::_avatar == this)
			{
				::_tutorialManager.onStatusEffectSet(effectToAdd.getEffectID());
			}
		}

		if (!this.mDead)
		{
			if (this.hasStatusEffect(this.StatusEffects.DEAD))
			{
				this.mDead = true;
				this.Util.updateMiniMapStickers();
				local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

				if (selfTargetWindow)
				{
					selfTargetWindow.fillOut();
				}

				local portalRequestScreen = this.Screens.get("PortalRequest", false);

				if (portalRequestScreen)
				{
					portalRequestScreen.setVisible(false);
				}

				if (::_avatar && ::_avatar.getTargetObject() == this)
				{
					local targetWindow = this.Screens.get("TargetWindow", false);

					if (targetWindow)
					{
						targetWindow.fillOut(this);
					}
				}

				this.onDeath();

				if (this == ::_avatar)
				{
					::Screens.show("RezScreen");
					::_avatar.setTargetObject(null);
					this.Screens.get("RezScreen", false).start();
				}
			}

			if (this.mAssembled)
			{
				this.checkAndSetCTFFlag();
			}

			this.queryUsable();
		}
		else if (!this.hasStatusEffect(this.StatusEffects.DEAD))
		{
			this.mDead = false;
			this.onRes();
		}

		if (this == ::_avatar)
		{
			this.updateInvisibilityScreen();
		}

		local inCombat = this.hasStatusEffect(this.StatusEffects.IN_COMBAT);
		local autoAttack = this.hasStatusEffect(this.StatusEffects.AUTO_ATTACK);
		local autoAttackRanged = this.hasStatusEffect(this.StatusEffects.AUTO_ATTACK_RANGED);
		local combatStand = this.hasStatusEffect(this.StatusEffects.IN_COMBAT_STAND);

		if (this == ::_avatar)
		{
			if (this.mInCombat && !inCombat)
			{
				local selfTargetWindow = this.Screens.get("SelfTargetWindow", true);

				if (selfTargetWindow)
				{
					selfTargetWindow.startCombatEndBlinkdown();
				}
			}
			else if (!this.mInCombat && inCombat)
			{
				local portalRequestScreen = this.Screens.get("PortalRequest", false);

				if (portalRequestScreen)
				{
					portalRequestScreen.setVisible(false);
				}

				local selfTargetWindow = this.Screens.get("SelfTargetWindow", true);

				if (selfTargetWindow)
				{
					selfTargetWindow.endCombatBlinkdown();
				}
			}
		}

		this.mInCombat = inCombat;

		if (this.mAnimationHandler)
		{
			if (combatStand)
			{
				this.mAnimationHandler.setCombat(true);
				this.mAnimationHandler.setIdleState("$IDLE_COMBAT$");
			}
			else
			{
				this.mAnimationHandler.setCombat(false);
				this.mAnimationHandler.setIdleState("Idle");
			}
		}

		inCombat = inCombat || autoAttack || autoAttackRanged;

		if (inCombat)
		{
			this.mAttemptingCombat = true;

			if (::_avatar == this)
			{
				if (autoAttackRanged && this.hasWeapon(this.ItemEquipSlot.WEAPON_RANGED))
				{
					this.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
				}
				else if (this.hasWeapon(this.ItemEquipSlot.WEAPON_MAIN_HAND))
				{
					if (!::_avatar.hasWeaponSet(this.VisibleWeaponSet.RANGED))
					{
						this.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
					}
				}
			}
		}
	}

	function getUniqueBuffs()
	{
		return this.mUniqueBuffs;
	}

	function getUniqueEffects()
	{
		return this.mUniqueStatusEffects;
	}

	function getUniqueStatusModifiers()
	{
		return this.mUniqueStatusModifiers;
	}

	function updateInvisibilityScreen()
	{
		local invisScreen = this.Screens.get("InvisibilityScreen", true);

		if (invisScreen)
		{
			if ((this.hasStatusEffect(this.StatusEffects.INVISIBLE) || this.hasStatusEffect(this.StatusEffects.WALK_IN_SHADOWS)) && !this.hasStatusEffect(this.StatusEffects.GM_INVISIBLE))
			{
				if (!invisScreen.isVisible())
				{
					invisScreen.reset();
					invisScreen.setVisible(true);
				}
			}
			else
			{
				invisScreen.setVisible(false);
			}
		}
	}

	function getStatusModifiers()
	{
		return this.mStatusModifiers;
	}

	function getObjectClass()
	{
		return this.mObjectClass;
	}

	function isScenery()
	{
		return this.mIsScenery;
	}

	function isCreature()
	{
		return !this.mIsScenery;
	}

	function isPlayer()
	{
		return this.mCreatureDef && this.mCreatureDef.isPlayer();
	}

	function _tostring()
	{
		if (this == this._avatar)
		{
			return this.getNodeName() + " (Avatar)";
		}

		local str = this.getNodeName();

		if (this.mAssembler)
		{
			str += " (" + this.mAssembler.getAssemblerDesc() + ")";
		}

		return str;
	}

	function getAssembler()
	{
		return this.mAssembler;
	}

	function getAssemblerError()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getArchiveError();
		}

		return null;
	}

	function isAssembled()
	{
		return this.mAssembled;
	}

	function forceUpdate()
	{
		this.mFrameCounter = this.FORCE_UPDATE;
		this.mForceUpdate = true;
	}

	function prepareAssemblingNode()
	{
		if (this.mAssemblingNode != null)
		{
			return;
		}

		this.mAssemblingNode = this._scene.createSceneNode("Assembling/" + this.mID);
		local assemblingNode = this.getAssemblingNode();

		if (assemblingNode)
		{
			local boxNode = this._scene.createEntity(this.mAssemblingNode.getName() + "/InvisBox", "Manipulator-ClickBox.mesh");
			boxNode.setVisibilityFlags(this.VisibilityFlags.ANY);
			boxNode.setQueryFlags(this.QueryFlags.ANY | this.QueryFlags.MANIPULATOR);
			boxNode.setOpacity(0.0);
			local loadEntity = this._scene.createEntity(this.mAssemblingNode.getName() + "/Creature_Load", "Manipulator-Creature_Load.mesh");
			loadEntity.setAnimationUnitCount(1);
			loadEntity.getAnimationUnit(0).setIdleState("Idle");
			loadEntity.getAnimationUnit(0).setEnabled(true);
			assemblingNode.setAutoTracking(true, this._camera.getParentSceneNode());
			assemblingNode.setFixedYawAxis(true);
			loadEntity.setVisibilityFlags(this.VisibilityFlags.ANY);
			this.log.debug("attachObject(boxNode)");
			assemblingNode.attachObject(boxNode);
			this.log.debug("attachObject(loadEntity)");
			assemblingNode.attachObject(loadEntity);
			::_scene.getRootSceneNode().addChild(assemblingNode);
		}
	}

	function _setAssembled( value )
	{
		if (value)
		{
			this.markAsGarbage(false);
			this.mAssembling = false;
			this.forceUpdate();

			if (this.hasStatusEffect(this.StatusEffects.CARRYING_RED_FLAG))
			{
				local flagLoadedCallback = {
					sceneObject = this,
					function onPackageComplete( pkg )
					{
						this.sceneObject.checkAndSetCTFFlag();
					}

				};
				::_contentLoader.load("Armor-Base1A-Helmet", this.ContentLoader.PRIORITY_NORMAL, "Red Flag", flagLoadedCallback);
			}
			else if (this.hasStatusEffect(this.StatusEffects.CARRYING_BLUE_FLAG))
			{
				local flagLoadedCallback = {
					sceneObject = this,
					function onPackageComplete( pkg )
					{
						this.sceneObject.checkAndSetCTFFlag();
					}

				};
				::_contentLoader.load("Armor-Base1A-Helmet", this.ContentLoader.PRIORITY_NORMAL, "Blue Flag", flagLoadedCallback);
			}
		}
		else
		{
			if (this.isScenery() && this.mAssembler)
			{
				this._sceneObjectManager._cancelCSMBuild(this.getName() + "/" + this.mAssembler.getName());
			}

			this.mCurrentFadeLevel = 0.0;
		}

		if (value && this.mAssemblingNode)
		{
			local wasShowingSelection = this.isShowingSelection();

			if (wasShowingSelection)
			{
				this.mSelectionNode.detachObject(this.mSelectionProjector);
				this.mAssemblingNode.removeChild(this.mSelectionNode);
				this.mSelectionProjector.destroy();
				this.mSelectionNode.destroy();
				this.mSelectionProjector = null;
				this.mSelectionNode = null;
			}

			this.mAssemblingNode.destroy();
			this.mAssemblingNode = null;

			if (wasShowingSelection)
			{
				this.setShowingSelection(wasShowingSelection);
			}
		}

		this.mAssembled = value;
		this._sceneObjectManager._updateAssemblyStatus(this, this.mAssembled);

		if (this.mAssembled && this.isCreature())
		{
			if (this.mCreatureDef && (this.getMeta("quest_giver") || this.getMeta("quest_ender") || this.hasStatusEffect(this.StatusEffects.IS_USABLE)) && this.mQuestIndicator == null)
			{
				if (this.mQuestIndicator == null)
				{
					this.mQuestIndicator = this.QuestIndicator(this, this.mID, this.mNode, this.getNamePlatePosition());
				}

				this.mQuestIndicator.setCreatureId(this.mID);
				this.mQuestIndicator.requestQuestIndicator();
			}

			local currWeapon = this.getStat(this.Stat.VIS_WEAPON);

			if (!currWeapon)
			{
				currWeapon = this.mVisibleWeaponSet;
			}

			this.setVisibleWeapon(this.VisibleWeaponSet.INVALID, false, false);

			if (currWeapon != this.VisibleWeaponSet.INVALID)
			{
				this.setVisibleWeapon(currWeapon, false, false);
			}
			else
			{
				this.setVisibleWeapon(this.VisibleWeaponSet.NONE, false, false);
			}

			local pt = this.Util.safePointOnFloor(this.getPosition());
			this.setDistanceToFloor(this.getPosition().y - pt.y);

			if (this.mDistanceToFloor < 0.0)
			{
				this.mDistanceToFloor = 0.0;
			}
		}

		this._positionName();

		if (value)
		{
			if (this.mEffectsHandler != null)
			{
				this.mEffectsHandler.onAssembled();
			}

			if (this.isDead() == true)
			{
			}
			else
			{
				local controller = this.getController();

				if (controller)
				{
					if (("mServerPosition" in controller) && controller.mServerPosition != null)
					{
						controller.mServerPosition.y = this.getPosition().y;
					}

					if (("mLastServerPos" in controller) && controller.mLastServerPos != null)
					{
						controller.mLastServerPos.y = this.getPosition().y;
					}
				}

				this.mLastServerUpdate = null;
				this.fireUpdate();
			}

			this.updateInteractParticle();
			this.queryUsable();
		}
		else
		{
			if (this.mQuestIndicator)
			{
				this.mQuestIndicator.destroy();
				this.mQuestIndicator = null;
			}

			::_useableCreatureManager.removeFromCache(this.getID());
			this.removeInteractParticle();
		}
	}

	function _getDefaultNamePlatePosition()
	{
		local position = this.getBoundingBox().getMaximum();
		position.y *= this.getScale().y;
		position.y += 3.0;
		local name = !this.mAssembler ? null : this.mAssembler.getPropName();

		if (name && name in this.ForcedQuestMarkerYPosition)
		{
			position.y = this.ForcedQuestMarkerYPosition[name];
		}

		return position;
	}

	function getNamePlatePosition()
	{
		this._calculateNamePlatePosition();
		return this.mNamePlatePosition;
	}

	function _calculateNamePlatePosition()
	{
		this.mNamePlatePosition = null;
		local entity = this.getEntity();

		if (entity)
		{
			local namePointAttachment = this.mAssembler.getAttachPointDef("nameplate");

			if (namePointAttachment)
			{
				local boneNameplate = namePointAttachment.bone;

				if (entity.hasBone(boneNameplate))
				{
					this.mNamePlatePosition = entity.getBoneDerivedPosition(boneNameplate);

					if ("position" in namePointAttachment)
					{
						this.mNamePlatePosition += namePointAttachment.position;
					}

					if ("scale" in namePointAttachment)
					{
						this.mNamePlateScale = namePointAttachment.scale.y;
					}
				}
			}
			else if (entity.hasBone("Bone-Head"))
			{
				this.mNamePlatePosition = entity.getBoneDerivedPosition("Bone-Head");
			}

			if (this.mNamePlatePosition)
			{
				this.mNamePlatePosition.y *= this.getScale().y;
				this.mNamePlatePosition.y += 3.0;
			}
		}

		if (!this.mNamePlatePosition)
		{
			this.mNamePlatePosition = this._getDefaultNamePlatePosition();
		}
	}

	function removeInteractParticle()
	{
		if (this.mInteractParticle)
		{
			this.mInteractParticle.destroy();
			this.mInteractParticle = null;
			this.mInteractParticleNode.destroy();
			this.mInteractParticleNode = null;
		}
	}

	function updateInteractParticle()
	{
		local MIN_SCALE = 1.0;
		local MAX_SCALE = 2.5;

		if (this.mInteractParticleNode != null)
		{
			local boundingRadius = this.getBoundingRadius();
			local size = boundingRadius / 3.0;

			if (size < MIN_SCALE)
			{
				size = MIN_SCALE;
			}
			else if (size > MAX_SCALE)
			{
				size = MAX_SCALE;
			}

			this.mInteractParticleNode.setScale(this.Vector3(size, size, size));
			this.mInteractParticle.setVisible(this.mAssembled);
		}
	}

	function addInteractParticle()
	{
		if (this.mInteractParticle == null && this.mNode != null)
		{
			local uniqueName = this.mNode.getName() + "/Interact_Particle";
			this.mInteractParticle = ::_scene.createParticleSystem(uniqueName, "Par-Ground_Sparkle");
			this.mInteractParticle.setVisibilityFlags(this.VisibilityFlags.ANY);
			this.mInteractParticleNode = this.mNode.createChildSceneNode();
			this.mInteractParticleNode.attachObject(this.mInteractParticle);
			this.updateInteractParticle();
		}
	}

	function getQuestIndicator()
	{
		return this.mQuestIndicator;
	}

	function disassemble()
	{
		if (this.mAssembler && this.mAssembled)
		{
			if (this.mAbilityEffect)
			{
				this.mAbilityEffect.destroy();
				this.mAbilityEffect = null;
			}

			this.broadcastMessage("onDisassemble");
			this.mAssembler.disassemble(this);
			this.Assert.isEqual(this.mAssembled, false);
		}
	}

	function reassemble( ... )
	{
		local assembleNow = vargc > 0 ? vargv[0] : false;

		if (!this.mAssembler && !this.mPickedAssembler)
		{
			this.setAssembler(null);
		}

		if (this.mAssembler)
		{
			if (assembleNow)
			{
				return this.mAssembler.reassemble(this);
			}
			else
			{
				if (this == null)
				{
					this.log.debug("REASSEMBLE - NULL THIS!");
				}

				this.mAssembler.disassemble(this);
				this._sceneObjectManager.queueAssembly(this);
			}
		}

		return false;
	}

	function shouldRender()
	{
		if (!this.mNode)
		{
			return false;
		}

		if (this == ::_avatar)
		{
			return true;
		}

		return this.isAlwaysVisible() || this.mLODLevel < 5;
	}

	function isPageReady()
	{
		local cpage = this.getTerrainPageCoords();
		return !this._sceneObjectManager.isPagePending(this._sceneObjectManager.mCurrentZoneDefId, cpage.x, cpage.z);
	}

	function setAbilityEffect( eff )
	{
		this.mAbilityEffect = eff;
	}

	function interruptAbility( abId )
	{
		if (this.mAbilityEffect)
		{
			this.mAbilityEffect.dispatch("onAbilityCancel");
		}

		this.mAbilityEffect = null;
	}

	function setbackAbility()
	{
		if (this.mAbilityEffect)
		{
			this.mAbilityEffect.dispatch("onAbilitySetback");
		}
	}

	function warmupComplete()
	{
		if (this.mAbilityEffect)
		{
			this.mAbilityEffect.dispatch("onAbilityWarmupComplete");
		}
	}

	function handleSpecialCaseUsage( so )
	{
		local callback = {
			creature = so,
			function onActionSelected( mb, alt )
			{
				if (alt == "Ok")
				{
					if (!::_avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT))
					{
						if (::_useableCreatureManager.isUseable(this.creature.getID()))
						{
							::_Connection.sendQuery("creature.use", this, this.creature.getID());
						}
					}
				}
			}

		};

		if (!::_useableCreatureManager.isUseable(so.getID()))
		{
			return false;
		}

		switch(so.getCreatureDef().getID())
		{
		case 2100:
			if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.Util.getRangeOffset(this._avatar, so) + this.MAX_USE_DISTANCE)
			{
				local popupBox = this.GUI.MessageBox.showOkCancel("You are about to leave Corsica. Once you enter the Southend Passage you will not be able to return to Corsica." + " Make sure you have finished any remaining quests before you continue!", callback);
				return true;
			}

			break;

		case 2198:
			if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.Util.getRangeOffset(this._avatar, so) + this.MAX_USE_DISTANCE)
			{
				local popupBox = this.GUI.MessageBox.showOkCancel("You are about to leave the new player area. Once you enter the portal you will be teleported to Anglorum." + " You will not be able to return to this area again. Make sure you have finished any remaining quests before you continue!", callback);
				return true;
			}

			break;
		}

		return false;
	}

	function isUsableDistance( targetSo )
	{
		if (!targetSo)
		{
			return false;
		}

		local sqDistance = this.getPosition().squaredDistance(targetSo.getPosition());
		local usableDistance = this.Util.getRangeOffset(this, targetSo);
		usableDistance *= usableDistance;
		usableDistance += this.MAX_USE_DISTANCE_SQ;
		return sqDistance <= usableDistance;
	}

	function useCreature( so )
	{
		local pvpable = so.hasStatusEffect(this.StatusEffects.PVPABLE);
		local creatureUsed = true;
		::_avatar.setResetTabTarget(true);

		if (!(so == ::_avatar))
		{
			::_avatar.setTargetObject(so);
		}

		local useageType = "";

		if (this.handleSpecialCaseUsage(so))
		{
			useageType = "Special";
		}
		else if (!so.isDead() && this.Key.isDown(this.Key.VK_SHIFT))
		{
			this.sendConMessage(::_avatar.getStat(this.Stat.LEVEL), so.getStat(this.Stat.LEVEL));
			useageType = "Consider";
		}
		else if (so.isDead() && so.hasLoot())
		{
			if (this.isDead())
			{
				this.IGIS.error("You cannot loot while dead.");
				return;
			}

			if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.MAX_USE_DISTANCE)
			{
				useageType = "Loot";
				local lootScreen = this.Screens.get("LootScreen", true);

				if (lootScreen)
				{
					if (lootScreen.checkLootingPermissions(so))
					{
						if (this.Key.isDown(this.Key.VK_SHIFT))
						{
							lootScreen.setAutoLoot(true);
						}
						else
						{
							lootScreen.setAutoLoot(false);
						}

						lootScreen.setTitle(so.getName());
						lootScreen.populateLoot(so.getID());
						lootScreen.setVisible(false, true);
					}
				}
			}
			else
			{
				::_avatar.getController().startFollowing(so, true);
			}
		}
		else if (so.getMeta("persona") && !pvpable)
		{
			::_playTool.setupCreatureMenu(so);
			::_playTool.mMenu.showMenu();
			::_playTool.mRotating = false;
			useageType = "PlayerMenu";
		}
		else if (!so.isDead() && !so.hasStatusEffect(this.StatusEffects.UNATTACKABLE))
		{
			local forceAAUpdate = false;
			local distance = this.Math.DetermineDistanceBetweenTwoPoints(this._avatar.getPosition(), so.getPosition());
			local meleeAA = this._AbilityManager.getAbilityByName("melee");
			local range = meleeAA.getRange();
			local mouseMoveEnabled = true;
			local rangeOffset = this.Util.getRangeOffset(this._avatar, so);

			if (distance < range + rangeOffset)
			{
				if (::_avatar.isRangedAutoAttackActive())
				{
					::_avatar.stopAutoAttack(true);
					forceAAUpdate = true;
				}

				::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
				::_avatar.startAutoAttack(false, forceAAUpdate);
			}
			else
			{
				::_avatar.getController().startFollowing(so, true);
				creatureUsed = false;
			}

			useageType = "Attack";
		}
		else if (this.isUsableDistance(so))
		{
			if (this.isDead())
			{
				this.IGIS.error("You cannot talk to creatures while dead.");
				return;
			}

			if (so.getMeta("copper_shopkeeper"))
			{
				local shopScreen = this.Screens.get("ItemShop", true);
				shopScreen.setMerchantId(so.getID());
				shopScreen.setShopScreenCategory(this.CurrencyCategory.COPPER);
				this.Screens.show("ItemShop", true);
				useageType = "CopperShop";
			}
			else if (so.getMeta("credit_shopkeeper"))
			{
				local shopScreen = this.Screens.get("ItemShop", true);
				shopScreen.setMerchantId(so.getID());
				shopScreen.setShopScreenCategory(this.CurrencyCategory.CREDITS);
				this.Screens.show("ItemShop", true);
				useageType = "CreditShop";
			}
			else if (so.getMeta("essence_vendor"))
			{
				local essenceScreen = this.Screens.get("EssenceShop", true);
				essenceScreen.setMerchantId(so.getID());
				essenceScreen.setVisible(true);
				useageType = "EssenceShop";
			}
			else if (so.getMeta("vault"))
			{
				local vaultScreen = this.Screens.get("Vault", true);

				if (vaultScreen)
				{
					vaultScreen.setVaultId(so.getID());
					vaultScreen.setVisible(true);
				}

				useageType = "Vault";
			}
			else if (so.getMeta("clan_registrar"))
			{
				local socialWindow = ::Screens.get("SocialWindow", true);

				if (socialWindow && socialWindow.isClanLeader())
				{
					::_playTool.beginClanTransfer();
				}
				else
				{
					local callback = {
						playTool = ::_playTool,
						function onActionSelected( mb, alt )
						{
							if (alt == "Yes")
							{
								::_playTool.beginClanCreation();
							}
						}

					};
					local copperAmt = 0;

					if (::_avatar)
					{
						copperAmt = ::_avatar.getStat(this.Stat.COPPER);
					}

					local amountNeeded = this.gCopperPerGold * 10;

					if (copperAmt < amountNeeded)
					{
						this.GUI.MessageBox.showYesNo("You do not have enough gold to create a clan.  Are you sure you want to continue?", callback);
					}
					else
					{
						::_playTool.beginClanCreation();
					}
				}

				useageType = "Clan";
			}
			else if (so.getMeta("crafter"))
			{
				local craftScreen = this.Screens.get("CraftingWindow", true);
				craftScreen.clearCraftingWindow();
				craftScreen.setCrafterId(so.getID());
				this.Screens.show("CraftingWindow", true);
				useageType = "Crafter";
			}
			else if (so.getMeta("credit_shop") != null)
			{
				local creditShop = this.Screens.get("CreditShop", true);

				if (creditShop)
				{
					local metaData = so.getMeta("credit_shop");
					creditShop.setVisible(true);
					creditShop.selectPanel(metaData);
				}

				useageType = "CreditPurchaseShop";
			}
			else if (so.hasStatusEffect(this.StatusEffects.TRANSFORMER))
			{
				local mscreen = this.Screens.get("MorphItemScreen", true);
				mscreen.reset();
				mscreen.setMorpherId(so.getID());
				this.Screens.show("MorphItemScreen");
				useageType = "Transformer";
			}
			else if (so.getQuestIndicator() && so.getQuestIndicator().hasValidQuest())
			{
				::_questManager.requestQuestOffer(so.getQuestIndicator().getCreatureId());
				useageType = "RequestQuest";
			}
			else if (so.getQuestIndicator() && so.getQuestIndicator().hasCompletedNotTurnInQuest())
			{
				::_questManager.requestCompleteNotTurnInQuest(so.getQuestIndicator().getCreatureId());
				useageType = "RequestComplete";
			}
			else if (::_useableCreatureManager.isUseable(so.getID()))
			{
				if (!::_avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT))
				{
					::_Connection.sendQuery("creature.use", this, so.getID());
					useageType = "UseCreature";
				}
				else
				{
					this.IGIS.info("You cannot interact with this when you\'re in combat.");
				}
			}
		}
		else if (!so.isDead())
		{
			if (::_avatar.getController().canInteractWith(so))
			{
				::_avatar.getController().startFollowing(so, true);
			}
			else
			{
				if (this.Pref.get("gameplay.mousemovement") == true)
				{
					local distance = this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition());

					if (distance < 500)
					{
						::_avatar.getController().startFollowing(so, true);
					}
				}

				creatureUsed = false;
			}
		}
		else
		{
			creatureUsed = false;
		}

		if (creatureUsed)
		{
			::_tutorialManager.onCreatureUsed(so.getID(), useageType);
		}

		return creatureUsed;
	}

	function gone()
	{
		if (this.mAssembled == false || this.mOpacity <= 0.0)
		{
			this.mGone = true;
			this.destroy();
			return;
		}

		this.mDesiredFadeLevel = 0.0;
		this.mFadeTarget = 0.0;
		this.mGone = true;
	}

	function destroy()
	{
		this.mDestroying = true;
		this._sceneObjectManager.removeUpdateListener(this);

		if (this.mVelocityUpdateSchedule)
		{
			::_eventScheduler.cancel(this.mVelocityUpdateSchedule);
			this.mVelocityUpdateSchedule = null;
		}

		if (::_buildTool)
		{
			this._buildTool.getSelection().remove(this);
		}

		this.setShowingShadow(false);
		this.setShowName(false);
		local name = this.getName();

		if (::_avatar && ::_avatar.getTargetObject() == this)
		{
			this.setResetTabTarget(true);
			this.setTargetObject(null);
		}

		if (::_Environment.isMarker(this))
		{
			::_Environment.removeMarker(this);
		}

		if (this.mEffectsHandler)
		{
			this.mEffectsHandler = this.mEffectsHandler.destroy();
		}

		if (this.mController)
		{
			this.mController = this.mController.destroy();
		}

		if (this.mHeadLight)
		{
			this.setHeadLight(false);
		}

		if (this.mAttachments.len() > 0)
		{
			local i;
			local slot;

			foreach( i, slot in this.Util.tableKeys(this.mAttachments) )
			{
				local io = this.mAttachments[slot];
				this.removeAttachment(io);
				io.destroy();
			}
		}

		this.mAttachments = {};

		foreach( att in this.mWeapons )
		{
			att.destroy();
		}

		foreach( att in this.mAttachmentOverride )
		{
			att.destroy();
		}

		this.mWeapons = {};
		this.mAttachmentOverride = {};
		this.disassemble();

		if (this.mAssembler)
		{
			this.mAssembler.removeManagedInstance(this);
			this.mAssembler = null;
		}

		this._sceneObjectManager._remove(this);
		this.mFlags = 0;
		this.stopSounds();

		if (::_avatar == this)
		{
			::_scene.setSoundListener(null);
			::_avatar = null;
		}

		if (this.mAssemblingNode)
		{
			this.mAssemblingNode.destroy();
			this.mAssemblingNode = null;
		}

		if (this.mQuestIndicator)
		{
			this.mQuestIndicator.destroy();
			this.mQuestIndicator = null;
		}

		if (this.mNode)
		{
			this.mNode.destroy();
			this.mNode = null;
		}
	}

	function onOtherDestroy( so )
	{
		if (so == this.mTargetObject)
		{
			this.setResetTabTarget(true);
			this.setTargetObject(null);
		}
	}

	function getVarsTypeAsAsset()
	{
		local a = this.AssetReference(this.mType);

		if (this.mVars && this.mVars.len() > 0)
		{
			a.setVars(this.mVars);
		}

		return a;
	}

	function setVarsTypeFromAsset( assetRef )
	{
		local archive = this.GetAssetArchive(assetRef.getAsset());

		if (archive == null)
		{
			throw this.Exception("Cannot determine archive for: " + assetRef.getAsset() + " (forget to rebuild the catalog?)");
		}

		this.setType(assetRef.getAsset(), assetRef.getVars());

		if (this.isAssembled())
		{
			this.reassemble();
		}
		else
		{
			this._eventScheduler.fireIn(1.0, this, "reassemble");
		}
	}

	function getType()
	{
		return this.mType;
	}

	function setTypeFromString( pType )
	{
		local tmp = this.AssetReference(pType);
		return this.setType(tmp.getAsset(), tmp.getVars());
	}

	function getTypeString()
	{
		return this.mTypeString;
	}

	function setType( pType, ... )
	{
		if (pType == null)
		{
			throw this.Exception("Invalid type: " + pType);
		}

		local oldVarStr = this.mVars == null ? null : this.System.encodeVars(this.mVars);
		local newVarStr;

		if (vargc > 0)
		{
			local vars = vargv[0];

			if (typeof vars == "string")
			{
				vars = this.System.decodeVars(vars);
			}

			this.mVars = vars;
			newVarStr = this.mVars == null ? null : this.System.encodeVars(this.mVars);
		}
		else
		{
			newVarStr = oldVarStr;
		}

		if (pType == this.mType && newVarStr == oldVarStr)
		{
			return false;
		}

		this.mTypeString = pType + (newVarStr != null ? "?" + newVarStr : "");
		this.mType = pType;

		if (typeof this.mID == "integer" && this.isCreature())
		{
			this.mCreatureDef = ::_creatureDefManager.getCreatureDef(this.mType);
		}
		else
		{
			this.mCreatureDef = null;
		}

		if (typeof this.mID == "integer" && ::_Environment.isMarker(this))
		{
			::_Environment.addMarker(this);
		}

		if (this.mPickedAssembler == null)
		{
			this.setAssembler(null);
		}

		if (this.isCreature())
		{
			this.setController(::_avatar == this ? "Avatar2" : "Creature");
		}
		else
		{
			this.setController(null);
		}

		return true;
	}

	function getDef()
	{
		return this.mDef;
	}

	function isInteractive()
	{
		return this.hasStatusEffect(this.StatusEffects.IS_USABLE);
	}

	function setAssembler( assembler )
	{
		this.mPickedAssembler = assembler;
		local wasAssembled = this.mAssembled;

		if (this.mAssembler)
		{
			this.disassemble();
			this.mAssembler.removeManagedInstance(this);
			this.mAssembler = null;
		}

		this.mAnimationHandler = null;
		this.mAnimationState = null;
		this.mDef = {};

		if (assembler == null)
		{
			this.mAssembler = this.GetAssembler(this.mObjectClass, this.mType);
		}
		else
		{
			this.mAssembler = assembler;
		}

		if (this.mAssembler)
		{
			this.mAssembler.addManagedInstance(this);
		}

		if (wasAssembled)
		{
			this.reassemble();
		}

		if (this.isCreature())
		{
			this._sceneObjectManager.addUpdateListener(this);
		}

		this.broadcastMessage("onAssembled");
	}

	function getAttachPointDef( pointName )
	{
		if (!this.mAssembler)
		{
			return null;
		}

		return this.mAssembler.getAttachPointDef(pointName);
	}

	function getCreatureDef()
	{
		return this.mCreatureDef;
	}

	function getDefaultAssembler()
	{
		return this.GetAssembler(this.mObjectClass, this.mType);
	}

	function _weaponSlot( slot )
	{
		switch(slot)
		{
		case this.ItemEquipSlot.WEAPON_MAIN_HAND:
		case this.ItemEquipSlot.WEAPON_OFF_HAND:
		case this.ItemEquipSlot.WEAPON_RANGED:
			return true;
		}

		return false;
	}

	function addAttachment( io )
	{
		this.mAttachments[io.getID()] <- io;
		io.setAttachedTo(this);
		this.broadcastMessage("onAttachmentAdded", io);

		if (this.mAnimationHandler && io.mAssemblyData && ("right_hand" == io.mAttachmentPointName || "left_hand" == io.mAttachmentPointName))
		{
			this.updateGrip();
		}
	}

	function addAttachmentOverride( io, slot )
	{
		local attachmentPoint = "right_hand";

		if (slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
		{
			attachmentPoint = "left_hand";
		}

		foreach( attachment in this.mAttachments )
		{
			if (attachment.getAttachmentPointName() == attachmentPoint)
			{
				this.removeAttachment(attachment);

				if (!(slot in this.mWeapons) && !(slot in this.mAttachmentOverride))
				{
					this.mWeapons[slot] <- attachment;
				}
			}
		}

		this.mAttachmentOverride[slot] <- io;
		this.addAttachment(io);
	}

	function removeAttachmentOverride( slot )
	{
		if (!(slot in this.mAttachmentOverride))
		{
			return;
		}

		local attachmentPoint = "right_hand";

		if (slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
		{
			attachmentPoint = "left_hand";
		}

		if (this.mAttachmentOverride[slot].getID() in this.mAttachments)
		{
			delete this.mAttachments[this.mAttachmentOverride[slot].getID()];
		}

		this.mAttachmentOverride[slot].destroy();
		delete this.mAttachmentOverride[slot];

		if (slot in this.mWeapons)
		{
			local new_weapon = this.Item.Attachable(null, this.mWeapons[slot].mMeshName, attachmentPoint, this.mWeapons[slot].mColors, this.mWeapons[slot].mEffectName);
			this.addAttachment(new_weapon);
			new_weapon.assemble();
		}
	}

	function removeAttachment( io )
	{
		if (io.getID() in this.mAttachments)
		{
			this.broadcastMessage("onAttachmentRemoved", io);
			delete this.mAttachments[io.getID()];
			io.destroy();

			if (!this.mDestroying && this.mAnimationHandler && ("right_hand" == io.mAttachmentPointName || "left_hand" == io.mAttachmentPointName))
			{
				this.updateGrip();
			}
		}
	}

	function hideWeapons()
	{
		foreach( att in this.mAttachments )
		{
			foreach( w in this.mWeapons )
			{
				if (att.getID() == w.getID())
				{
					this.removeAttachment(att);
				}
			}
		}

		this.updateSheathedWeapons();
	}

	function inWeaponSet( set, slot )
	{
		switch(set)
		{
		case this.VisibleWeaponSet.MELEE:
			return slot == this.ItemEquipSlot.WEAPON_MAIN_HAND || slot == this.ItemEquipSlot.WEAPON_OFF_HAND;

		case this.VisibleWeaponSet.RANGED:
			return slot == this.ItemEquipSlot.WEAPON_RANGED;
		}

		return false;
	}

	function showWeapons()
	{
		foreach( s, w in this.mWeapons )
		{
			if (this.inWeaponSet(this.mVisibleWeaponSet, s))
			{
				local att_point = w.mAttachmentPointName;

				if (w.getWeaponType() == "Bow")
				{
					att_point = "left_hand";
				}

				local new_weapon = this.Item.Attachable(null, w.mMeshName, att_point, w.mColors, w.mEffectName);
				this.addAttachment(new_weapon);
				new_weapon.assemble();
				local oldWeaponAttachemnt = this.mWeapons[s];

				if (oldWeaponAttachemnt)
				{
					this.removeAttachment(oldWeaponAttachemnt);
				}

				this.mWeapons[s] <- new_weapon;
			}
		}

		if (this.mVisibleWeaponSet == this.VisibleWeaponSet.MELEE)
		{
			this.removedSheathedWeapons();
		}

		this.updateGrip();
	}

	function updateSheathedWeapons()
	{
		if (this.mForceShowEquipment || this.mAssembler == null || !("mRequestedItems" in this.mAssembler))
		{
			return;
		}

		local unserializedAppearance = this.mAssembler.mRequestedItems;

		if (unserializedAppearance == null)
		{
			return;
		}

		local visibleWeapons = this.getStat(this.Stat.VIS_WEAPON);

		if (visibleWeapons != null && visibleWeapons)
		{
			return;
		}

		if (this.mWeapons.len() == 0)
		{
			return;
		}

		this.removedSheathedWeapons();

		if ((this.ItemEquipSlot.WEAPON_MAIN_HAND in this.mWeapons) && this.ItemEquipSlot.WEAPON_MAIN_HAND in unserializedAppearance)
		{
			local mainHand = unserializedAppearance[this.ItemEquipSlot.WEAPON_MAIN_HAND];
			local mainHandCallback = {
				so = this,
				function doWork( itemDef )
				{
					local unserializedAppearance = this.so.mAssembler.mRequestedItems;

					if (unserializedAppearance && unserializedAppearance[this.ItemEquipSlot.WEAPON_MAIN_HAND] == itemDef.mID)
					{
						this.so.sheathWeapon(itemDef, this.ItemEquipSlot.WEAPON_MAIN_HAND);
					}
				}

			};
			::_ItemDataManager.getItemDef(mainHand, mainHandCallback);
		}

		if ((this.ItemEquipSlot.WEAPON_OFF_HAND in this.mWeapons) && this.ItemEquipSlot.WEAPON_OFF_HAND in unserializedAppearance)
		{
			local offHand = unserializedAppearance[this.ItemEquipSlot.WEAPON_OFF_HAND];
			local offHandCallback = {
				so = this,
				function doWork( itemDef )
				{
					local unserializedAppearance = this.so.mAssembler.mRequestedItems;

					if (unserializedAppearance && unserializedAppearance[this.ItemEquipSlot.WEAPON_OFF_HAND] == itemDef.mID)
					{
						this.so.sheathWeapon(itemDef, this.ItemEquipSlot.WEAPON_OFF_HAND);
					}
				}

			};
			::_ItemDataManager.getItemDef(offHand, offHandCallback);
		}
	}

	function sheathWeapon( def, slot )
	{
		local attachmentPoint;
		local rightHandedWeapon = false;

		if (slot == this.ItemEquipSlot.WEAPON_MAIN_HAND)
		{
			rightHandedWeapon = true;
		}

		local weaponType = def.getWeaponType();
		local armorType = def.getArmorType();

		if (weaponType != this.WeaponType.NONE)
		{
			attachmentPoint = this.getShealthedAttachmentPointForWeaponType(weaponType, rightHandedWeapon);
		}
		else if (armorType == this.ArmorType.SHIELD)
		{
			attachmentPoint = "back_6";
		}

		local new_weapon = this.Item.Attachable(null, this.mWeapons[slot].mMeshName, attachmentPoint, this.mWeapons[slot].mColors, this.mWeapons[slot].mEffectName);
		this.addAttachment(new_weapon);
		new_weapon.assemble();
	}

	function getShealthedAttachmentPointForWeaponType( weaponType, rightHandedWeapon )
	{
		local attachmentPoint = "";

		if (rightHandedWeapon)
		{
			attachmentPoint = "left";
		}
		else
		{
			attachmentPoint = "right";
		}

		if (weaponType != this.WeaponType.NONE)
		{
			switch(weaponType)
			{
			case this.WeaponType.SMALL:
				return attachmentPoint += "_hip";
				break;

			case this.WeaponType.ONE_HAND:
				return attachmentPoint += "_hip";
				break;

			case this.WeaponType.TWO_HAND:
				attachmentPoint = "back_sheathe";
				return attachmentPoint;
				break;

			case this.WeaponType.POLE:
				attachmentPoint = "back_sheathe";
				return attachmentPoint;
				break;
			}
		}
	}

	function removedSheathedWeapons()
	{
		foreach( att in this.mAttachments )
		{
			if (!att.mAttachmentPointName)
			{
				continue;
			}

			if (att.mAttachmentPointName == "right_hip" || att.mAttachmentPointName == "left_hip" || att.mAttachmentPointName == "back_sheathe" || att.mAttachmentPointName == "back_6")
			{
				this.removeAttachment(att);
			}
		}
	}

	function setVisibleWeapon( set, animated, ... )
	{
		if (("switchingWeapons" in this.mAnimationHandler) && this.mAnimationHandler.switchingWeapons())
		{
			return false;
		}

		if (this.mVisibleWeaponSet == set)
		{
			return false;
		}

		this.mPreviousWeaponSet = this.mVisibleWeaponSet;
		this.mVisibleWeaponSet = set;
		local notify = true;
		local wait = false;
		local waiter;

		switch(vargc)
		{
		case 1:
			notify = vargv[0];
			break;

		case 2:
			wait = vargv[0];
			waiter = vargv[1];
			break;

		case 3:
			notify = vargv[0];
			wait = vargv[1];
			waiter = vargv[2];
			break;
		}

		if (animated && this.mAnimationHandler)
		{
			this.mAnimationHandler.switchWeapons(this.mPreviousWeaponSet, this.mVisibleWeaponSet, waiter);
		}
		else
		{
			this.hideWeapons();
			this.showWeapons();
		}

		if (notify && ::_Connection.isPlaying())
		{
			::_Connection.sendQuery("visWeapon", this, [
				set
			]);
		}

		return true;
	}

	function onVisibleWeaponUpdate( visible )
	{
		if (visible)
		{
			return;
		}

		if (this.mAssembler == null || !("mRequestedItems" in this.mAssembler) || this.mAssembler.mRequestedItems == null)
		{
			return;
		}

		this.updateSheathedWeapons();
	}

	function onEQAppearanceUpdate( value )
	{
		local visibleWeapon = this.getStat(this.Stat.VIS_WEAPON);

		if (visibleWeapon == null || visibleWeapon)
		{
			return;
		}

		this.updateSheathedWeapons();
	}

	function getVisibleWeapon()
	{
		return this.mVisibleWeaponSet;
	}

	function setWeapon( slot, att )
	{
		if (!this._weaponSlot(slot))
		{
			return;
		}

		this.mWeapons[slot] <- att;
	}

	function removeWeapons()
	{
		this.mWeapons = {};
	}

	function hasWeapon( slot )
	{
		return slot in this.mWeapons;
	}

	function hasWeaponSet( set )
	{
		foreach( s, w in this.mWeapons )
		{
			if (this.inWeaponSet(set, s))
			{
				return true;
			}
		}

		return false;
	}

	function updateGrip()
	{
		if ("setGrip" in this.mAnimationHandler)
		{
			if (this.hasItemInHand())
			{
				this.mAnimationHandler.setGrip(true);
			}
			else
			{
				this.mAnimationHandler.setGrip(false);
			}
		}
	}

	function hideHandAttachments()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				if (attachment.getEntity())
				{
					attachment.getEntity().setVisible(false);
				}

				local particleSystem = attachment.getParticleSystem();

				if (particleSystem)
				{
					particleSystem.setVisible(false);
				}
			}
		}
	}

	function unHideHandAttachments()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity)
				{
					entity.setVisible(true);
				}

				local particleSystem = attachment.getParticleSystem();

				if (particleSystem)
				{
					particleSystem.setVisible(true);
				}
			}
		}
	}

	function hasItemInHand()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && entity.isVisible())
				{
					return true;
				}
			}
		}

		return false;
	}

	function getHandAttachments( ... )
	{
		local visibleOnly = vargc > 0 ? vargv[0] : true;
		local results = [];

		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && (!visibleOnly || entity.isVisible()))
				{
					results.append(attachment);
				}
			}
		}

		return results;
	}

	function getLeftHandItem()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && entity.isVisible())
				{
					return attachment;
				}
			}
		}

		return null;
	}

	function getRightHandItem()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && entity.isVisible())
				{
					return attachment;
				}
			}
		}

		return null;
	}

	function removeAllAttachments()
	{
		local da = [];

		foreach( io in this.mAttachments )
		{
			this.broadcastMessage("onAttachmentRemoved", io);
			io.destroy();
		}

		this.mAttachments.clear();
	}

	function removeNoneItemAttachments()
	{
		local da = [];

		foreach( key, io in this.mAttachments )
		{
			if (io.getObjectClass() == "Attachment")
			{
				da.append(io.getID());
				io.disassemble();
				io.destroy();
			}
		}

		foreach( x in da )
		{
			delete this.mAttachments[x];
		}
	}

	function getAttachments()
	{
		return this.mAttachments;
	}

	function setSceneryName( name )
	{
		this.mProperties.NAME = name;
	}

	function getSceneryName()
	{
		if ("NAME" in this.mProperties)
		{
			return this.mProperties.NAME;
		}

		return "Unnamed (" + this.getName() + ")";
	}

	function getName()
	{
		local name = this.getStat(this.Stat.DISPLAY_NAME);
		return name == null ? this.getNodeName() : name;
	}

	function onDeath()
	{
		if (this.isCreature())
		{
			if (this.mAnimationHandler)
			{
				this.mAnimationHandler.onDeath();
			}
			else
			{
				this.mNeedToRunDeathAnim = true;
			}
		}
	}

	function onRes()
	{
		if (this.isCreature())
		{
			if (this.mAnimationHandler)
			{
				this.mAnimationHandler.onRes();
			}
		}
	}

	function getNodeName()
	{
		return this.mNode.getName();
	}

	function getNode()
	{
		return this.mNode;
	}

	function getAssemblingNode()
	{
		return this.mAssemblingNode;
	}

	function getID()
	{
		return this.mID;
	}

	function getLOD()
	{
		return this.mLODLevel;
	}

	function markAsGarbage( ... )
	{
		if (vargc > 0 && !vargv[0])
		{
			if (this.mGarbagificationTime != null)
			{
				this.mGarbagificationTime = null;
				this._sceneObjectManager._queueDestroy(this, false);
			}
		}
		else if (this.mGarbagificationTime == null)
		{
			this.mGarbagificationTime = this.System.currentTimeMillis();
			this._sceneObjectManager._queueDestroy(this, true);
		}
	}

	function setAnimationHandler( pAnimationHandler )
	{
		if (this.mAnimationHandler == pAnimationHandler)
		{
			return;
		}

		if (this.mAnimationHandler)
		{
			this.mAnimationState = this.mAnimationHandler.getAnimationState();
			this.mAnimationHandler.destroy();
		}

		this.mAnimationHandler = pAnimationHandler;

		if (this.mAnimationHandler && this.mAnimationState)
		{
			this.mAnimationHandler.setAnimationState(this.mAnimationState);
			this.mAnimationState = null;
		}

		if (this.mNeedToRunDeathAnim)
		{
			if (this.mAnimationHandler)
			{
				this.mAnimationHandler.onDeath();
			}

			this.mNeedToRunDeathAnim = false;
		}

		if (this.mAnimationHandler && this.mSpeed > 0.0)
		{
			this.mAnimationHandler.onMove(this.mSpeed);
		}
	}

	function getAnimationHandler()
	{
		return this.mAnimationHandler;
	}

	function setController( pControllerName )
	{
		local data;

		if (this.mController)
		{
			data = this.mController.getData();
			this.mController.destroy();
		}

		if (pControllerName in this.Controller)
		{
			this.mController = this.Controller[pControllerName](this);
		}
		else
		{
			this.mController = null;
		}

		if (data && this.mController)
		{
			this.mController.setData(data);
		}
	}

	function getController()
	{
		return this.mController;
	}

	function setEffectsHandler( effectsHandler )
	{
		this.mEffectsHandler = effectsHandler;
	}

	function getEffectsHandler()
	{
		return this.mEffectsHandler;
	}

	function serverVelosityUpdate( ... )
	{
		if (this == ::_avatar)
		{
			local force = vargc > 0 ? vargv[0] : false;

			if (force)
			{
				this._Connection.sendVelocityUpdate(this.getPosition(), this.mHeading, this.mRotation, this.mSpeed);
			}
			else
			{
				this.mVelocityUpdatePending = true;
			}
		}
	}

	function sendPendingVelocityUpdate()
	{
		if (this.mVelocityUpdatePending == true)
		{
			this._Connection.sendVelocityUpdate(this.getPosition(), this.mHeading, this.mRotation, this.mSpeed);
			this.mVelocityUpdatePending = false;
		}

		if (::_avatar == this)
		{
			this.mVelocityUpdateSchedule = ::_eventScheduler.fireIn(0.25, this, "sendPendingVelocityUpdate");
		}
		else
		{
			this.mVelocityUpdateSchedule = null;
		}
	}

	function setVerticalSpeed( speed )
	{
		this.mVerticalSpeed = speed;
	}

	function collideAndMoveSweep( pos, dir, mask )
	{
		local stepHeight = 1.9;
		local box = this.Vector3(2.5, 7.0, 2.5);
		return this.Util.collideAndSlide(box, stepHeight, pos, dir, mask);
	}

	function _setAssembling( which )
	{
		this.mAssembling = which;
	}

	function getAssembling()
	{
		return this.mAssembling;
	}

	function _notifyUpdateReceived()
	{
	}

	function isCharacterOrientableOnTerrain( heading, normal )
	{
		if (this.mFloorAlignMode == this.FloorAlignMode.ALWAYS || (heading.dot(normal) > this.UP_DOWN_SLOPE_ANGLE || heading.dot(normal) < -this.UP_DOWN_SLOPE_ANGLE) && this.mFloorAlignMode == this.FloorAlignMode.WHILE_ASCENDING_DESCENDING)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	function _scaleNameBoard()
	{
		if (this.mNameBoard == null)
		{
			return;
		}

		local distance = this._getDistanceFromCamera();
		local h = this.Math.lerp(this.gNameNearHeight, this.gNameFarHeight, distance / this.gNameFarClip);

		if (this.mNamePlateScale)
		{
			this.mNameBoard.setLineHeight(h * this.mNamePlateScale);
		}
		else
		{
			this.mNameBoard.setLineHeight(h);
		}
	}

	function isAlwaysVisible()
	{
		if (this.mAlwaysVisible != null)
		{
			return this.mAlwaysVisible;
		}

		local assembler = this.getAssembler();

		if (assembler)
		{
			local config = assembler.getConfig();

			if (config != null)
			{
				local tmp = config.find("[\"c\"]=\"");

				if (tmp != null)
				{
					local end = config.find("\"", tmp + 7);
					local type = config.slice(tmp + 7, end);

					if (type && type in ::AlwaysVisible)
					{
						this.mAlwaysVisible = ::AlwaysVisible[type];
						return this.mAlwaysVisible;
					}
				}
			}
		}

		return false;
	}

	function updateFade()
	{
		if (this.mGone)
		{
			return;
		}

		if (this.isAlwaysVisible())
		{
			this.mCurrentFadeLevel = 1.0;
			this.mFadeTarget = 1.0;
			this.setOpacity(1.0);
			return;
		}

		if (this.mFadeEnabled == false)
		{
			this.mCurrentFadeLevel = this.mDesiredFadeLevel;
			this.mFadeTarget = this.mDesiredFadeLevel;
			this.setOpacity(this.mDesiredFadeLevel);
			return;
		}

		local distance = this._getDistanceFromCamera();
		local fadeStart = this.gCreatureVisibleRange - this.gLodBlockSize;
		local fadeEnd = this.gCreatureVisibleRange;
		local fadeSpan = fadeEnd - fadeStart;
		local fadeLevel = 0.0;

		if (distance >= fadeEnd)
		{
			fadeLevel = 0.0;
		}
		else if (distance >= fadeStart)
		{
			fadeLevel = 1.0 - (distance - fadeStart) / fadeSpan;
			this.Math.clamp(fadeLevel, 0.0, 1.0);
		}
		else
		{
			fadeLevel = 1.0;
		}

		if (this.isInvisible())
		{
			fadeLevel *= 0.30000001;
		}

		this.mFadeTarget = fadeLevel;
	}

	function isInvisible()
	{
		return this.hasStatusEffect(this.StatusEffects.INVISIBLE) || this.hasStatusEffect(this.StatusEffects.WALK_IN_SHADOWS) || this.hasStatusEffect(this.StatusEffects.GM_INVISIBLE);
	}

	function _getDistanceFromCamera()
	{
		local pos = this.getPosition();
		local cpos = this._camera.getParentSceneNode().getPosition();
		return pos.distance(cpos);
	}

	function _interpolateFade()
	{
		local delta = this._deltat / 1000.0 * 2.0;
		local opacity = this.mOpacity;

		if (this.mFadeTarget < opacity)
		{
			opacity = this.Math.max(this.mFadeTarget, opacity - delta);
		}
		else if (this.mFadeTarget > opacity)
		{
			opacity = this.Math.min(this.mFadeTarget, opacity + delta);
		}

		this.setOpacity(opacity);
	}

	function _updateLOD()
	{
		local distance = this._getDistanceFromCamera();
		this.mNameInRange = distance <= this.gNameFarClip;

		if (!this.mForceUpdate)
		{
			this.mLODCounter++;

			if (this.mLODCounter < this.LOD_FRAME_COHERENCE)
			{
				return;
			}
		}

		this.mLODCounter = 0;
		local newLod = (distance / this.gLodBlockSize).tointeger();

		if (newLod > 5)
		{
			newLod = 5;
		}

		local lodStart = newLod * this.gLodBlockSize;
		local lodEnd = (newLod + 1) * this.gLodBlockSize;
		local mPercentToNextLOD = (lodEnd - distance) / this.gLodBlockSize;

		if (newLod != this.mLODLevel)
		{
			local oldLod = this.mLODLevel;
			this.mLODLevel = newLod;
			this._sceneObjectManager._updateLodBucket(this, oldLod, newLod);

			if (newLod <= 3)
			{
				if (!this.mAssembled && this.mType != "")
				{
					this.reassemble();
				}
			}
			else if (newLod >= 5 && !this.isAlwaysVisible())
			{
				if (this.mAssembled)
				{
					this.disassemble();
				}
			}
		}
	}

	LOD_FRAME_COHERENCE = 32;
	FRAME_COHERENCE = 32;
	ORIENT_COHERENCE = 15;
	mFirstUpdate = false;
	mOrientCounter = 0;
	mFrameCounter = 0;
	mForceUpdate = true;
	mLODCounter = 0;
	mVisible = false;
	function _updateVisibility()
	{
		if (this == ::_avatar)
		{
			this.mVisible = true;
			return;
		}

		local pos = this.getPosition();

		if (pos != null)
		{
			local forward = ::_camera.getParentNode()._getDerivedOrientation().rotate(this.Vector3(0.0, 0.0, -1.0));
			forward.normalize();
			local dir = pos - ::_camera.getParentNode().getWorldPosition();
			dir.normalize();
			local vis = forward.dot(dir) > 0.60000002;

			if (vis != this.mVisible)
			{
				this.forceUpdate();
			}

			this.mVisible = vis;
		}
	}

	function _basicUpdate()
	{
		local speed = this.gDefaultCreatureSpeed * (this.mSpeed / 100.0);

		if (speed > 0.0099999998)
		{
			local dir = ::Math.ConvertRadToVector(this.mHeading);
			dir = dir * speed * ::_deltat / 1000.0;
			local newPos = this.getPosition() + dir;

			if (this.mVisible && this.mLODLevel < 4)
			{
				local floor = this.Util.getFloorHeightAt(newPos, 20.0, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true, this.getNode());

				if (floor)
				{
					newPos.y = floor.pos.y;
				}
			}

			this.setPosition(newPos);
		}
	}

	function _updateFloorAlignment()
	{
		if (this.mFloorAlignMode != this.FloorAlignMode.WHILE_ASCENDING_DESCENDING)
		{
			return;
		}

		local pos = this.getPosition();
		local terrain = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.KEEP_ON_FLOOR, true, this.getNode());

		if (terrain == null)
		{
			return;
		}

		local heading = ::Math.ConvertRadToVector(this.mHeading);

		if (this.mInterpolateFramesLeft <= 0 && !this.isCharacterOrientableOnTerrain(heading, terrain.normal))
		{
			this.mStartNormal = this.mLastNormal;
			this.mEndNormal = terrain.normal;
			this.mLastNormal = this.mEndNormal;
			this.mInterpolateFramesLeft = this.FRAMES_TO_INTERPOLATE;
			this.mCurrentlyOriented = false;
		}
	}

	function _interpolateFloorAlignment()
	{
		if (this.mInterpolateFramesLeft >= 0)
		{
			local yAxis = this.Math.slerpVectors(this.mStartNormal, this.mEndNormal, 1.0 - this.mInterpolateFramesLeft.tofloat() / this.FRAMES_TO_INTERPOLATE.tofloat());
			local xAxis = this.Vector3(1.0, 0.0, 0.0);
			local zAxis = xAxis.cross(yAxis);
			zAxis.normalize();
			xAxis = yAxis.cross(zAxis);
			xAxis.normalize();
			local coordSpace = this.Quaternion(xAxis, yAxis, zAxis);
			local entity = this.getEntity();

			if (entity)
			{
				this.mFloorAlignOrientation = coordSpace;
				entity.getParentNode().setOrientation(coordSpace);
				entity.getParentNode().rotate(this.Vector3(0, 1, 0), this.mRotation);
			}

			this.mInterpolateFramesLeft--;
		}
	}

	function setTimeoutEnabled( which )
	{
		this.mTimeoutEnabled = which;
	}

	function _checkTimeout()
	{
		if (this != ::_avatar && this.mTimeoutEnabled && !this.mGone)
		{
			this.mTimeSinceLastUpdate += this._deltat;

			if (this.mTimeSinceLastUpdate > this.UPDATE_TIMEOUT)
			{
				this.log.debug("Removing creature " + this.getName() + " because it has expired!");
				this.gone();
			}
		}
	}

	function onEnterFrame()
	{
		this._checkTimeout();

		if (!this.mAssembled && !this.mAssemblingNode)
		{
			return;
		}

		this._updateVisibility();
		this._updateLOD();
		this._positionName();

		if ((this.mIsScenery || !this.shouldRender()) && !this.mForceUpdate)
		{
			return;
		}

		this._interpolateFade();
		this._interpolateFloorAlignment();

		if (this.mVisible)
		{
			this._scaleNameBoard();
		}

		if (this.mCorking && this.mCorkTimeout > 0)
		{
			this.mCorkTimeout -= this._deltat;

			if (this.mCorkTimeout <= 0)
			{
				this.uncork();
			}
		}

		if (this.mController)
		{
			if (::_avatar == this)
			{
				this.mController.setEnabled(!this.hasStatusEffect(this.StatusEffects.ROOT) && !this.hasStatusEffect(this.StatusEffects.DEAD));
			}

			this.mController.onEnterFrame();
		}

		if (this.mAnimationHandler)
		{
			this.mAnimationHandler.onEnterFrame();
		}

		if (!this.isPlayer() && !this.mForceUpdate && this.mDistanceToFloor < 0.25)
		{
			this.mFrameCounter++;

			if (this.mFrameCounter < this.FRAME_COHERENCE)
			{
				this._basicUpdate();
				return;
			}

			this.mFrameCounter = 0;
		}

		this._updateFloorAlignment();
		this._checkSounds();
		this.updateFade();

		if (this.mGone && this.mOpacity <= 0.0)
		{
			this.destroy();
			return;
		}

		local pos = this.getPosition();
		local oldPos = this.Vector3(pos.x, pos.y, pos.z);

		if (!this.mForceUpdate && this.mSpeed < 0.001 && this.mDistanceToFloor < 0.001 && this.mSlopeSlideInertia == null)
		{
			return;
		}

		local dir = ::Math.ConvertRadToVector(this.mHeading);
		local baseSpeed = this.gDefaultCreatureSpeed;
		local speed = baseSpeed * (this.mSpeed / 100.0);
		dir = dir * speed * ::_deltat / 1000.0;
		local sizeY = this.BASE_AVATAR_HEIGHT * this.getScale().y;

		if (sizeY <= 0.0)
		{
			sizeY = 1.0;
		}

		local checkSwimming = !this.mSwimming;

		if (this.mSwimming)
		{
			local sterrain = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.KEEP_ON_FLOOR, true, this.getNode());

			if (sterrain && (this.mWaterElevation - sterrain.pos.y) / sizeY < 0.30000001)
			{
				checkSwimming = false;
				this.mSwimming = false;
				this.mController.onStopSwimming();
			}
			else
			{
				local avatarPosition = this.getPosition();
				local startingPoint = this.Vector3(avatarPosition.x, this.mWaterElevation, avatarPosition.z);
				local box = this.Vector3(2.0, 4.0, 2.0);
				local groundTestDir = this.Vector3(0.0, -1.5, 0.0);
				local groundTest = this._scene.sweepBox(box, startingPoint, startingPoint + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);

				if (sterrain == null || groundTest.distance < 1)
				{
					checkSwimming = false;
					this.mSwimming = false;
					this.mController.onStopSwimming();
				}
				else
				{
					pos.y = this.mWaterElevation;
					pos = this.collideAndMoveSweep(pos, dir, this.QueryFlags.BLOCKING).pos;
					this.setPosition(pos);

					if (this.mAnimationHandler)
					{
						this.mAnimationHandler.forceAnimUpdate();
					}

					return;
				}
			}
		}

		pos = this.collideAndMoveSweep(pos, dir, this.QueryFlags.BLOCKING).pos;
		local deltaY = this.mVerticalSpeed * ::_deltat / 1000.0;

		if (this.mDistanceToFloor > 0.001)
		{
			local res = this.collideAndMoveSweep(pos, this.Vector3(0.0, deltaY, 0.0), this.QueryFlags.BLOCKING);
			pos = res.pos;

			if (this.mVerticalSpeed > 0.0 && res.hit == true)
			{
				this.mVerticalSpeed = 0.0;
			}

			this.mVerticalSpeed -= this.mDownwardAcceleration * ::_deltat / 1000.0;
		}

		local terrain = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.KEEP_ON_FLOOR, true, this.getNode());
		this._updateFloorAlignment();
		local StepValue = 6.0;
		local avatarPosition = this.getPosition();
		local startingPoint = this.Vector3(avatarPosition.x, avatarPosition.y + StepValue, avatarPosition.z);
		local box = this.Vector3(2.0, 4.0, 2.0);
		local movementTestDir = this.Vector3(0.0, deltaY, 0.0);
		local MovementTest = this._scene.sweepBox(box, startingPoint, startingPoint + movementTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);
		local finalMovementPos = startingPoint + movementTestDir * MovementTest.distance;
		local groundTestDir = this.Vector3(0.0, -5000.0, 0.0);
		local groundTest = this._scene.sweepBox(box, startingPoint, startingPoint + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);
		local finalGroundPos = startingPoint + groundTestDir * groundTest.distance;
		finalGroundPos.y -= StepValue;
		finalMovementPos.y -= StepValue;

		if (groundTest.distance < 1.0)
		{
			if (this.mDistanceToFloor > 0.001)
			{
				local groundTest2 = this._scene.sweepBox(this.Vector3(3.0, 4.0, 3.0), startingPoint, startingPoint + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);
				local normalAngle = groundTest2.normal.dot(this.Vector3(0, 1, 0));

				if (normalAngle < 0.2)
				{
					finalGroundPos += groundTest.normal * 3.5;
				}
				else
				{
					finalGroundPos += groundTest.normal * 1;
				}
			}
			else
			{
			}
		}

		local floor = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true, this.getNode());

		if (terrain == null)
		{
			terrain = floor;
		}

		if (floor != null && terrain != null)
		{
			if (!this.mCurrentlyJumping && this.abs(pos.y - finalGroundPos.y) < 6.5 && (this.abs(dir.x) > 0.5 || this.abs(dir.z) > 0.5))
			{
				this.setDistanceToFloor(0.0, floor.normal);
				this.mVerticalSpeed = 0.0;
				pos.y = finalGroundPos.y;
			}

			if (groundTest.distance < 1 && finalGroundPos.y > terrain.pos.y)
			{
				if (this.isPropCreature() && groundTest.distance <= 0)
				{
				}
				else if (this.mObjectBelowAvatar == false && this.mVerticalSpeed <= 0.1 && this.mVerticalSpeed >= -0.1)
				{
					this.setDistanceToFloor(0.0, floor.normal);
					this.mVerticalSpeed = 0.0;
					this.mCurrentlyJumping = false;
					pos.y = finalGroundPos.y;
				}
				else if (finalMovementPos.y - finalGroundPos.y <= 1.0)
				{
					pos.y = finalGroundPos.y;
					this.mVerticalSpeed = 0.0;
					this.setDistanceToFloor(0.0, terrain.normal);
					this.mCurrentlyJumping = false;
				}
				else
				{
					this.setDistanceToFloor(finalMovementPos.y - finalGroundPos.y - 1, floor.normal);
				}

				this.mObjectBelowAvatar = true;
			}
			else
			{
				if (::_avatar == this)
				{
				}

				this.mObjectBelowAvatar = false;

				if (pos.y <= terrain.pos.y)
				{
					this.mVerticalSpeed = 0.0;
					pos = terrain.pos;
					this.setDistanceToFloor(0.0, terrain.normal);
					this.mCurrentlyJumping = false;
				}
				else
				{
					this.setDistanceToFloor(-terrain.t, terrain.normal);
				}
			}

			if (pos.y < floor.pos.y)
			{
				pos.y = floor.pos.y;
				this.mObjectBelowAvatar = false;
			}

			local slideableTerrainHeight = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.FLOOR, true, this.getNode());

			if (::_avatar == this)
			{
				if (slideableTerrainHeight != null && this.abs(floor.pos.y - slideableTerrainHeight.pos.y) < 0.050000001 && slideableTerrainHeight.normal.dot(this.Vector3(0, 1, 0)) < this.gMaxSlope && this.mDistanceToFloor <= 0.001)
				{
					local norm = slideableTerrainHeight.normal;
					norm.y = 0;
					norm.normalize();

					if (this.mSlopeSlideInertia != null)
					{
						this.mSlopeSlideInertia.x += norm.x * baseSpeed * 4 * ::_deltat / 1000.0;
						this.mSlopeSlideInertia.z += norm.z * baseSpeed * 4 * ::_deltat / 1000.0;
					}
					else
					{
						this.mSlopeSlideInertia = {
							x = norm.x * baseSpeed * 4 * ::_deltat / 1000.0,
							z = norm.z * baseSpeed * 4 * ::_deltat / 1000.0
						};
					}
				}
				else if (this.mSlopeSlideInertia != null)
				{
					this.mSlopeSlideInertia.x *= 0.60000002;
					this.mSlopeSlideInertia.z *= 0.60000002;

					if (this.fabs(this.mSlopeSlideInertia.x) < 0.050000001 && this.fabs(this.mSlopeSlideInertia.z) < 0.050000001)
					{
						this.mSlopeSlideInertia = null;
						this.serverVelosityUpdate();
					}
				}
			}

			if (this.mSlopeSlideInertia != null && this.mDistanceToFloor <= 0.001)
			{
				if (::_avatar == this)
				{
				}

				this.serverVelosityUpdate();
				local slidePosition = this.Vector3(pos.x, pos.y, pos.z);
				slidePosition.x += this.mSlopeSlideInertia.x * ::_deltat / 1000.0;
				slidePosition.z += this.mSlopeSlideInertia.z * ::_deltat / 1000.0;
				slidePosition.y += StepValue;
				local startingPos = this.Vector3(pos.x, pos.y, pos.z);
				local collision = this._scene.sweepBox(box, pos, slidePosition, this.QueryFlags.BLOCKING, false);

				if (collision.distance < 1.0)
				{
					pos = oldPos;
					pos.y += 5.0;
					local slideVector = slidePosition - pos;
					slidePosition = this.collideAndMoveSweep(pos, slideVector, this.QueryFlags.BLOCKING | this.QueryFlags.FLOOR).pos;
				}

				pos.x = slidePosition.x;
				pos.z = slidePosition.z;
				pos.y = slidePosition.y;
				local terrainHeight = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.FLOOR, false, this.getNode());
				local height = this._scene.sweepBox(box, pos, pos + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);

				if (height.distance < 1.0)
				{
					local finalGroundPos = pos + groundTestDir * height.distance;
					pos.y = finalGroundPos.y;
				}

				pos.y -= StepValue;

				if (terrainHeight != null)
				{
					if (pos.y < terrainHeight || height.distance >= 1.0)
					{
						pos.y = terrainHeight;
					}
				}

				checkSwimming = true;
			}
		}

		if (checkSwimming)
		{
			local swimming = false;
			this.mWaterElevation = this.Util.getWaterHeightAt(pos);

			if (this.mWaterElevation > pos.y)
			{
				if (sizeY > 0.0)
				{
					local uwp = (this.mWaterElevation - pos.y) / sizeY;

					if (uwp > 0.5)
					{
						this.mSwimming = true;
						this.mController.onStartSwimming(this.mWaterElevation);
						pos.y = this.mWaterElevation - sizeY * 0.60000002;
					}
				}
			}
		}

		local collision = ::_scene.sweepSphere(0.5, avatarPosition, pos);

		if (this.mSlopeSlideInertia != null && collision.distance < 1.0)
		{
			collision.normal.y = 0;
			pos = avatarPosition + collision.normal;
		}

		if (::_avatar == this)
		{
		}

		this.setPosition(pos);
		::_LightingManager.updateVisibility(this);
		this.mForceUpdate = false;
	}

	function fireUpdate()
	{
		if (this.mController)
		{
			this.mController.onUpdate();
		}

		this.broadcastMessage("onUpdate");
	}

	function addFloatie( text, ... )
	{
		local floatieType = this.IGIS.FLOATIE_DEFAULT;

		if (vargc > 0)
		{
			floatieType = vargv[0];
		}

		if (this.mCorking)
		{
			this.mCorkedFloaties.append({
				message = text,
				type = floatieType
			});
			return;
		}

		::IGIS.floatie(text, floatieType, this);
	}

	function addCombatMessage( msgChannel, combatMessage )
	{
		if (this.mCorking)
		{
			this.mCorkedChatMessage.append({
				message = combatMessage,
				channel = msgChannel
			});
			return;
		}

		::_ChatManager.addMessage(msgChannel, combatMessage);
	}

	function onJump()
	{
		if (this.mController)
		{
			this.mController.onJump();
			this.setJumping(true);
		}
	}

	function setJumping( value )
	{
		this.mCurrentlyJumping = true;
	}

	function startAutoAttack( ranged, ... )
	{
		local force = false;

		if (vargc > 0)
		{
			force = vargv[0];
		}

		local target = this.getTargetObject();

		if (target == null)
		{
			this.IGIS.error("Nothing to attack.");
			return;
		}

		if (target.isDead())
		{
			this.IGIS.error("Your target is dead.");
			return;
		}

		if (target.hasStatusEffect(this.StatusEffects.UNATTACKABLE) || target.hasStatusEffect(this.StatusEffects.INVINCIBLE))
		{
			this.IGIS.error("You cannot attack that target.");
			return;
		}

		if (target.isPlayer() && !target.hasStatusEffect(this.StatusEffects.PVPABLE))
		{
			this.IGIS.error("You cannot attack that target.");
			return;
		}

		if (force || !this._avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT_STAND))
		{
			local ab;
			local quickbar1 = ::_quickBarManager.getQuickBar(0);
			local abilityActiveAnimation = this.GUI.Container();
			abilityActiveAnimation.setSize(32, 32);

			if (ranged)
			{
				this.mRangedAutoAttackActive = true;
				ab = this._AbilityManager.getAbilityByName("ranged_melee");
				local rangedAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(1);

				if (rangedAbilityButton)
				{
					rangedAbilityButton.addExtraComponent(abilityActiveAnimation);
					abilityActiveAnimation.setMaterial("AbilityActive");
					quickbar1.getActionContainer().updateContainer();
				}
			}
			else
			{
				this.mMeleeAutoAttackActive = true;
				ab = this._AbilityManager.getAbilityByName("melee");
				local meleeAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(0);

				if (meleeAbilityButton)
				{
					meleeAbilityButton.addExtraComponent(abilityActiveAnimation);
					abilityActiveAnimation.setMaterial("AbilityActive");
					quickbar1.getActionContainer().updateContainer();
				}
			}

			if (ab)
			{
				ab.sendActivationRequest();
			}
		}
	}

	function stopAutoAttack( ranged )
	{
		local quickbar1 = ::_quickBarManager.getQuickBar(0);

		if (ranged)
		{
			local rangedAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(1);

			if (rangedAbilityButton)
			{
				rangedAbilityButton.removeExtraComponent();
				this.mRangedAutoAttackActive = false;
			}
		}
		else
		{
			local meleeAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(0);

			if (meleeAbilityButton)
			{
				meleeAbilityButton.removeExtraComponent();
				this.mMeleeAutoAttackActive = false;
			}
		}
	}

	function onServerPosition( pX, pY, pZ )
	{
		if (!this.mController)
		{
			local pos = this.Util.safePointOnFloor(this.Vector3(pX, pY, pZ), this.getNode());
			this.setPosition(pos);
		}
		else
		{
			this.mController.onServerPosition(pX, pY, pZ);
		}
	}

	function onServerVelocity( pServerHeading, pServerRotation, pServerSpeed )
	{
		if (!this.mLastServerUpdate)
		{
			local rotation = pServerRotation;
			this.mRotation = pServerRotation;
			this.mHeading = pServerHeading;
			this.setOrientation(rotation);
		}

		if (this.mController)
		{
			this.mController.onServerVelocity(pServerHeading, pServerRotation, pServerSpeed);
		}
	}

	function getDistanceToFloor()
	{
		return this.mDistanceToFloor;
	}

	function setDistanceToFloor( value, ... )
	{
		if (this.mDistanceToFloor == value)
		{
			return;
		}

		this.mDistanceToFloor = value;

		if (this.mController)
		{
			this.mController.setFalling(this.mDistanceToFloor > 0.0099999998);
		}
	}

	function getPosition()
	{
		return this.mNode != null ? this.mNode.getPosition() : null;
	}

	function getTerrainPageName()
	{
		return this.Util.getTerrainPageName(this.mNode.getPosition());
	}

	function getTerrainPageCoords()
	{
		return this.Util.getTerrainPageIndex(this.mNode.getPosition());
	}

	function getSceneryPageCoords()
	{
		return {
			x = (this.mNode.getPosition().x / ::_sceneObjectManager.mCurrentZonePageSize).tointeger(),
			z = (this.mNode.getPosition().z / ::_sceneObjectManager.mCurrentZonePageSize).tointeger()
		};
	}

	function setPosition( pos )
	{
		if (pos == null)
		{
			return;
		}

		local tx = pos.x;
		local ty = pos.y;
		local tz = pos.z;

		if (this.mNode == null)
		{
			return;
		}

		local oldPos = this.mNode.getPosition();

		if (!this.Util.fuzzyCmpVector3(pos, oldPos))
		{
			this.mNode.setPosition(pos);

			if (this.mAssemblingNode)
			{
				pos.y += 8.0;
				this.mAssemblingNode.setPosition(pos);
			}

			if (this.mAssembler)
			{
				this.mAssembler.notifyTransformed(this);
			}

			this._LightingManager.queueVisibilityUpdate(this);
		}
	}

	function setOrientation( value )
	{
		if (typeof value == "float" || typeof value == "integer")
		{
			value = this.Quaternion(value, this.Vector3().UNIT_Y);
		}

		this.Assert.isInstanceOf(value, this.Quaternion);
		local rot = this.mNode.getOrientation();

		if (rot && this.Util.fuzzyCmpQuaternion(rot, value))
		{
			return;
		}

		this.mNode.setOrientation(value);

		if (this.mAssembler)
		{
			this.mAssembler.notifyTransformed(this);
		}
	}

	function getOrientation()
	{
		return this.mNode.getOrientation();
	}

	function setShowingShadow( value )
	{
		if ((value ? true : false) == this.isShowingShadow())
		{
			return;
		}

		this.mShadowVisible = value;

		if (value)
		{
		}
		else if (this.mShadowDecal)
		{
			this.mShadowDecal.destroy();
			this.mShadowDecal = null;
		}
		else if (this.mShadowProjector)
		{
			this.mShadowProjector.getParentSceneNode().destroy();
			this.mShadowProjector = null;
		}
	}

	function isShowingShadow()
	{
		return this.mShadowVisible;
	}

	function setShowingSelection( value )
	{
		if ((value ? true : false) == this.isShowingSelection())
		{
			return;
		}

		if (value)
		{
			local radius = this.getBoundingRadius();

			if (radius <= 1.5)
			{
				radius = 1.5;
			}

			this.mSelectionProjector = this._scene.createTextureProjector(this.getNodeName() + "/SelectionDecal", "SelectionRing.png");
			this.mSelectionProjector.setNearClipDistance(0.1);
			this.mSelectionProjector.setFarClipDistance(160);
			this.mSelectionProjector.setOrthoWindow(radius, radius);
			this.mSelectionProjector.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
			this.mSelectionProjector.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
			this.mSelectionProjector.setAlphaBlended(true);
			local parentNode = this.mAssemblingNode ? this.mAssemblingNode : this.mNode;
			this.mSelectionNode = parentNode.createChildSceneNode();
			this.mSelectionNode.setPosition(this.Vector3(0, 150, 0));
			this.mSelectionNode.attachObject(this.mSelectionProjector);
			this.mSelectionNode.lookAt(parentNode.getWorldPosition());
		}
		else if (this.mSelectionProjector)
		{
			local parentNode = this.mAssemblingNode ? this.mAssemblingNode : this.mNode;
			this.mSelectionNode.detachObject(this.mSelectionProjector);
			parentNode.removeChild(this.mSelectionNode);
			this.mSelectionProjector.destroy();
			this.mSelectionNode.destroy();
			this.mSelectionProjector = null;
			this.mSelectionNode = null;
		}
	}

	function isShowingSelection()
	{
		return this.mSelectionProjector != null;
	}

	function setScale( value )
	{
		if (value == null)
		{
			value = this.mNormalSize;
		}

		if (typeof value == "float" || typeof value == "integer")
		{
			value = this.Vector3(value, value, value);
		}

		local s = this.mNode.getScale();

		if (s && this.Util.fuzzyCmpVector3(s, value))
		{
			return;
		}

		this.mNode.setScale(value);

		if (this.mAssembler)
		{
			this.mAssembler.notifyTransformed(this);
		}

		this.updateInteractParticle();
	}

	function getScale()
	{
		return this.mNode.getScale();
	}

	function getNormalSize()
	{
		return this.mNormalSize;
	}

	function setNormalSize( size )
	{
		this.mNormalSize = size;
	}

	function getVars()
	{
		return this.mVars;
	}

	function setVars( vars )
	{
		this.setType(this.mType, vars);
	}

	function getFlags()
	{
		return this.mFlags;
	}

	function setFlags( flags )
	{
		local wasPrimary = this.isPrimary();
		this.mFlags = flags;
	}

	function isLocked()
	{
		return (this.mFlags & this.LOCKED) != 0;
	}

	function isDead()
	{
		return this.mDead;
	}

	function isGMFrozen()
	{
		return this.hasStatusEffect(this.StatusEffects.GM_FROZEN);
	}

	function hasLoot()
	{
		return this.mHasLoot;
	}

	function setLocked( value )
	{
		if (value)
		{
			this.setFlags(this.mFlags | this.LOCKED);
		}
		else
		{
			this.setFlags(this.mFlags & ~this.LOCKED);
		}
	}

	function isPrimary()
	{
		return this.mFlags & this.PRIMARY;
	}

	function setPrimary( value )
	{
		if (value)
		{
			this.setFlags(this.mFlags | this.PRIMARY);
		}
		else
		{
			this.setFlags(this.mFlags & ~this.PRIMARY);
		}
	}

	function setSceneryLayer( name )
	{
		this.mSceneryLayer = name;
	}

	function getSceneryLayer()
	{
		return this.mSceneryLayer;
	}

	function setOpacity( value )
	{
		if (value > 0.99900001)
		{
			value = 1.0;
		}

		if (this.mOpacity == value)
		{
			return;
		}

		this.mOpacity = value;

		if (this.mNode == null)
		{
			return;
		}

		local body = this.getAssembler().getBaseEntity(this);

		if (body)
		{
			body.setOpacity(value);
			local entities = body.getAttachedObjects();

			foreach( i, x in entities )
			{
				if (x != null && (x instanceof this.Entity))
				{
					x.setOpacity(value);
				}
			}
		}
	}

	function onAttachmentPointChanged( entity )
	{
		if (entity != null && (entity instanceof this.Entity))
		{
			entity.setOpacity(this.mOpacity);
		}
	}

	function getOpacity()
	{
		local assembler = this.getAssembler();

		if (assembler)
		{
			local body = assembler.getBaseEntity(this);

			if (body)
			{
				return body.getOpacity();
			}
		}

		return 1.0;
	}

	function getEntity()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getBaseEntity(this);
		}

		return null;
	}

	function getBoundingRadius()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getBoundingRadius(this);
		}

		return 0.0;
	}

	function getBoundingBox()
	{
		local e = this.getEntity();

		if (e)
		{
			return e.getBoundingBox();
		}

		return this.AxisAlignedBox();
	}

	function onCreatureDefUpdate( statId, value )
	{
		this.broadcastMessage("onStatUpdated", this, statId, value);
	}

	function setStat( statId, value )
	{
		if (this.mStats == null)
		{
			this.mStats = {};
		}

		local oldValue = statId in this.mStats ? this.mStats[statId] : null;

		if (this.Util.tableSetOrRemove(this.mStats, statId, value))
		{
			this.onStatUpdated(statId, value, oldValue);
			this.broadcastMessage("onStatUpdated", this, statId, value);
		}
	}

	function onStatUpdated( statId, value, oldValue )
	{
		if (::_avatar == this)
		{
			::_tutorialManager.onStatUpdated(statId, value, oldValue);
		}

		if (statId == this.Stat.APPEARANCE_OVERRIDE || statId == this.Stat.LOOT_SEEABLE_PLAYER_IDS)
		{
			local appearance = this.getStat(this.Stat.APPEARANCE_OVERRIDE);
			local lootString = this.getStat(this.Stat.LOOT_SEEABLE_PLAYER_IDS);

			if (appearance == "")
			{
				this.setAssembler(null);
				return;
			}

			if (this != ::_avatar && appearance && lootString)
			{
				this.handleLootbagChange(lootString, appearance);
			}
		}
		else if (statId == this.Stat.REZ_PENDING)
		{
			if (value > 0 && this.mDead)
			{
				local rez_screen = this.Screens.get("RezScreen", false);
			}
		}
		else if (statId == this.Stat.LEVEL)
		{
			if (::_avatar == this)
			{
				::QuestIndicator.updateCreatureIndicators();
			}

			if (oldValue != null && value != oldValue)
			{
				if (::_avatar == this)
				{
					::IGIS.info("You have reached Level " + value + "!");
				}

				this.cue("LevelDing");
			}
		}
		else if (statId == this.Stat.CURRENT_ABILITY_POINTS)
		{
			if (oldValue != null && value != oldValue)
			{
				if (::_avatar == this)
				{
					local difference = value - oldValue;

					if (difference > 0)
					{
						::IGIS.info("You gain " + difference + " ability points.");
					}
				}
			}
		}
		else if (statId == this.Stat.VIS_WEAPON)
		{
			if (::_avatar != this)
			{
				this.setVisibleWeapon(value, true, false);
			}
		}
		else if (statId == this.Stat.MOD_MOVEMENT)
		{
			if (this == ::_avatar)
			{
				::_avatar.getController().setAvatarSpeed(100 + value);
			}
		}
		else if (statId == this.Stat.BASE_STATS)
		{
			if (value != "")
			{
				local splitStats = this.Util.split(value, ",");
				this.mBaseStats[this.Stat.STRENGTH] = splitStats[0].tointeger();
				this.mBaseStats[this.Stat.DEXTERITY] = splitStats[1].tointeger();
				this.mBaseStats[this.Stat.CONSTITUTION] = splitStats[2].tointeger();
				this.mBaseStats[this.Stat.PSYCHE] = splitStats[3].tointeger();
				this.mBaseStats[this.Stat.SPIRIT] = splitStats[4].tointeger();
			}
		}
		else if (statId == this.Stat.SELECTIVE_EQ_OVERRIDE)
		{
			if (this.mDead)
			{
				return;
			}

			local eqOverride = this.unserialize(value);

			if (oldValue && oldValue != "" && oldValue != value)
			{
				local oldEQOverride = this.unserialize(oldValue);

				foreach( eqSlot, itemDef in oldEQOverride )
				{
					if (!(eqSlot in eqOverride))
					{
						this.removeAttachmentOverride(eqSlot);
					}
				}
			}

			if (value != "")
			{
				foreach( eqSlot, itemDefId in eqOverride )
				{
					local callback = {
						so = this,
						slot = eqSlot,
						function doWork( itemDef )
						{
							if (this.so == null)
							{
								return;
							}

							local type;
							local colors;
							local appearance = itemDef.getAppearance();

							if (appearance && appearance != "")
							{
								if (typeof appearance == "string")
								{
									appearance = this.unserialize(appearance);
								}

								appearance = appearance[0];

								if ("a" in appearance)
								{
									appearance = appearance.a;

									if ("type" in appearance)
									{
										type = appearance.type;
									}

									if ("colors" in appearance)
									{
										colors = appearance.colors;
									}

									local placeAttacmentCallback = {
										sceneObject = this.so,
										equipmentSlot = this.slot,
										def = itemDef,
										attType = type,
										attColors = colors,
										function onPackageComplete( pkg )
										{
											this.sceneObject.placeAttachmentOverride(this.attType, this.attColors, this.def, this.equipmentSlot);
										}

									};

									if (!(type in ::AttachableDef))
									{
										this.Util.waitForAssets(type, placeAttacmentCallback);
										return;
									}

									this.so.placeAttachmentOverride(type, colors, itemDef, this.slot);
								}
							}
						}

					};
					::_ItemDataManager.getItemDef(itemDefId, callback);
				}
			}
		}
		else if (statId == this.Stat.AGGRO_PLAYERS)
		{
			this.updateNameBoardColor();
		}
		else if (statId == this.Stat.SUB_NAME)
		{
			if (this.mNameBoard)
			{
				if (value != "")
				{
					this.mNameBoard.setText(this.getName() + "\n<" + value + ">");
				}
				else
				{
					this.mNameBoard.setText(this.getName());
				}
			}
		}
	}

	function updateNameBoardColor()
	{
		local aggroPlayers = this.getStat(this.Stat.AGGRO_PLAYERS);

		if (aggroPlayers != null && this.mNameBoard)
		{
			if (aggroPlayers == 1)
			{
				this.mNameBoard.setColorTop(this.Color(1.0, 0.0, 0.0, 1.0));
				this.mNameBoard.setColorBottom(this.Color(1.0, 0.0, 0.0, 1.0));
			}
			else if (this.hasStatusEffect(this.StatusEffects.UNATTACKABLE) || this.hasStatusEffect(this.StatusEffects.INVINCIBLE))
			{
				this.mNameBoard.setColorTop(this.Color(0.0, 1.0, 0.0, 1.0));
				this.mNameBoard.setColorBottom(this.Color(0.0, 1.0, 0.0, 1.0));
			}
			else
			{
				this.mNameBoard.setColorTop(this.Color(1.0, 1.0, 0.0, 1.0));
				this.mNameBoard.setColorBottom(this.Color(1.0, 1.0, 0.0, 1.0));
			}
		}
	}

	function placeAttachmentOverride( type, colors, itemDef, slot )
	{
		if (type != null && colors != null)
		{
			local selectiveOverride = this.getStat(this.Stat.SELECTIVE_EQ_OVERRIDE);
			local stringToFind = slot + "]=" + itemDef.getID();

			if (selectiveOverride.find(stringToFind))
			{
				local attachmentPoint = "right_hand";

				if (slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
				{
					attachmentPoint = "left_hand";
				}

				local new_weapon = this.Item.Attachable(null, type, attachmentPoint, colors, null);
				this.addAttachmentOverride(new_weapon, slot);
				new_weapon.assemble();
			}
		}
	}

	function getBaseStatValue( stat )
	{
		if (stat in this.mBaseStats)
		{
			return this.mBaseStats[stat];
		}
		else
		{
			return 0;
		}
	}

	function handleLootbagChange( lootableSeeablePlayerIds, newAppearances )
	{
		local avatarID = ::_avatar.getID();

		if (this.hasStatusEffect(this.StatusEffects.DEAD))
		{
			local appearances = this.Util.split(newAppearances, "|");
			local def = ::_avatar.mCreatureDef;
			local newAppearance = newAppearances;

			if (def)
			{
				local lootableIds = this.Util.split(lootableSeeablePlayerIds, ",");
				local visible = false;

				foreach( i in lootableIds )
				{
					if (i != "" && i.tointeger() == def.mID)
					{
						visible = true;
						break;
					}
				}

				if (visible)
				{
					newAppearance = appearances[0];
					this.mHasLoot = true;
					::_tutorialManager.lootDropped();
				}
				else if (appearances.len() > 1)
				{
					newAppearance = appearances[1];
					this.mHasLoot = false;
				}

				this.morphFromStat(newAppearance);
			}
		}
	}

	function morphFromStat( stat )
	{
		try
		{
			local data = ::unserialize(stat);
			local assembler = this.getAssembler();

			if (data != null && assembler.getObjectType() == data.a)
			{
				  // [016]  OP_POPTRAP        1      0    0    0
				return;
			}

			if (this.mDeathAppearanceChangeEvent)
			{
				::_eventScheduler.cancel(this.mDeathAppearanceChangeEvent);
				this.mDeathAppearanceChangeEvent = null;
			}

			this.log.debug("Morphing to: " + stat);

			if (data == null)
			{
				this.setAssembler(null);
				  // [039]  OP_POPTRAP        1      0    0    0
				return;
			}

			local delay = "delay" in data ? data.delay : 0;
			delay = delay.tointeger();

			if (this.isAssembled())
			{
				if (delay > 0)
				{
					this.mDeathAppearanceChangeEvent = ::_eventScheduler.fireIn(delay.tofloat() / 1000.0, this, "performMorph", data);
					this.log.debug("Scheduled morph: " + stat);
					  // [076]  OP_POPTRAP        1      0    0    0
					return;
				}
			}

			if (this.mDeathAppearanceChangeEvent == null)
			{
				this.performMorph(data);
			}
		}
		catch( err )
		{
			this.log.debug(err);
		}
	}

	function performMorph( data )
	{
		local type = "type" in data ? data.type : "CreatureDef";

		if (type == "CreatureDef")
		{
			type = "Creature";
		}

		if (this.isAssembled() && "effect" in data)
		{
			if (this.mMorphEffectsHandler)
			{
				this.mMorphEffectsHandler.addEffectNarrative(data.effect);
			}
		}

		local assembler = this.GetAssembler(type, data.a);
		this.setAssembler(assembler);
		local size = "size" in data ? data.size : null;
		this.setScale(size);
		this.mDeathAppearanceChangeEvent = null;
	}

	function getStat( statId, ... )
	{
		local value;

		if (this.mStats == null)
		{
			value = null;
		}
		else
		{
			value = this.Util.tableSafeGet(this.mStats, statId);
		}

		if ((value == null || value == "{}") && this.mCreatureDef && (vargc == 0 || vargv[0]))
		{
			value = this.mCreatureDef.getStat(statId);
		}

		return value;
	}

	function getMeta( key )
	{
		return this.mCreatureDef ? this.mCreatureDef.getMeta(key) : null;
	}

	function setShowName( value )
	{
		this.mShowName = value;
	}

	function setHeadLight( bool )
	{
		if (bool && !this.mHeadLight)
		{
			local lightNode = this.mNode.createChildSceneNode();
			this.mShowHeadLight = bool;

			if (this._scene.hasLight("avatarHeadLight"))
			{
				this._scene.getLight("avatarHeadLight").destroy();
				this.log.error("avatarHeadLight being set but it already exists!");
			}

			this.mHeadLight = this._scene.createLight("avatarHeadLight");
			this.mHeadLight.setLightType(this.Light.POINT);
			this.mHeadLight.setAttenuation(1000.0, 0.2, 0.0, 0.00050000002);
			this.mHeadLight.setDiffuseColor(this.Color("ffffff"));
			lightNode.setPosition(0.0, 25.0, 0.0);
			lightNode.attachObject(this.mHeadLight);
		}
		else if (!bool && this.mHeadLight)
		{
			this.mShowHeadLight = bool;
			this.mHeadLight.destroy();
		}
	}

	function setSpeed( value )
	{
		this.mSpeed = value;
	}

	function getSpeed()
	{
		return this.mSpeed;
	}

	function addTimeCastingTime( amtToAdd )
	{
		if (this.isCreature())
		{
			this.mCastingEndTime += amtToAdd;
		}
	}

	function cancelCasting()
	{
		this.mCastingEndTime = 0;
	}

	function getCastingTimeRemaining()
	{
		if (this.isCreature() && this.mTimer)
		{
			return this.mCastingEndTime - this.mTimer.getMilliseconds();
		}

		return -1;
	}

	function getCurrentCastingTime()
	{
		if (this.isCreature() && this.mTimer)
		{
			return this.mTimer.getMilliseconds();
		}

		return -1;
	}

	function getCastingEndTime()
	{
		if (this.isCreature())
		{
			return this.mCastingEndTime;
		}

		return -1;
	}

	function getUsingAbilityID()
	{
		if (this.isCreature())
		{
			return this.mUsingAbilityID;
		}

		return null;
	}

	function isCasting()
	{
		if (this.isCreature())
		{
			return this.mTimer.getMilliseconds() < this.mCastingEndTime;
		}

		return false;
	}

	function isRangedAutoAttackActive()
	{
		return this.mRangedAutoAttackActive;
	}

	function isMeleeAutoAttackActive()
	{
		return this.mMeleeAutoAttackActive;
	}

	function startCasting( id )
	{
		if (this.isCreature() && this.mTimer)
		{
			local ab = this._AbilityManager.getAbilityById(id);
			this.mCastingWarmupTime = ab.getWarmupDuration();
			local totalCastMod = 0.0;
			local modCastingSpeed = this.getStat(this.Stat.MOD_CASTING_SPEED);
			local magicAttackSpeed = this.getStat(this.Stat.MAGIC_ATTACK_SPEED);

			if (modCastingSpeed)
			{
				totalCastMod += modCastingSpeed;
			}

			if (modCastingSpeed)
			{
				totalCastMod += magicAttackSpeed * 0.001;
			}

			this.mCastingWarmupTime -= this.mCastingWarmupTime * totalCastMod;
			this.mCastingEndTime = this.mCastingWarmupTime + this.mTimer.getMilliseconds();
			this.mUsingAbilityID = id;
		}
	}

	function setHeading( value )
	{
		this.mHeading = value;

		if (this.mController)
		{
			this.mController.onHeadingChanged();
		}
	}

	function updateHeading( value )
	{
		this.mHeading = value;
	}

	function getHeading()
	{
		return this.mHeading;
	}

	function setRotation( value )
	{
		this.setOrientation(value);
		this.mRotation = value;
	}

	function getRotation()
	{
		return this.mRotation;
	}

	function getVerticalSpeed()
	{
		return this.mVerticalSpeed;
	}

	function getTargetObject()
	{
		return this.mTargetObject;
	}

	function getStats()
	{
		return this.mStats;
	}

	function getResetTabTarget()
	{
		return this.mResetTabTargetting;
	}

	function setResetTabTarget( value )
	{
		this.mResetTabTargetting = value;
	}

	function setTargetObject( so )
	{
		if (so)
		{
			::_tutorialManager.onTargetSelected(so);
		}

		if (this == ::_avatar && this.hasStatusEffect(this.StatusEffects.DEAD))
		{
			so = null;
		}

		if (this.mTargetObject && this.mTargetObject.hasStatusEffect(this.StatusEffects.HENGE))
		{
			local projectorNode = this._scene.getSceneNode(this.mTargetObject.getNodeName() + "/FeedbackNode");

			if (projectorNode)
			{
				this._scene.getRootSceneNode().removeChild(projectorNode);
				projectorNode.destroy();
			}
		}

		if (this.mTargetObject)
		{
			this.mTargetObject.setShowingSelection(false);
		}

		if (so)
		{
			local targetWindow = ::Screens.show("TargetWindow");
			targetWindow.fillOut(so);
			targetWindow.updateMods(so);

			if (so.hasStatusEffect(this.StatusEffects.HENGE))
			{
				local op = this._scene.createTextureProjector(so.getNodeName() + "/FeedbackOuterProjector", "Area_Circle_Green.png");
				op.setNearClipDistance(0.1);
				op.setFarClipDistance(500);
				op.setAlphaBlended(true);
				op.setOrthoWindow(300, 300);
				op.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
				op.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
				local opn = this._scene.getRootSceneNode().createChildSceneNode(so.getNodeName() + "/FeedbackNode");
				opn.attachObject(op);
				local pos = so.getNode().getWorldPosition();
				pos.y += 200;
				opn.setPosition(pos);
				opn.lookAt(pos + this.Vector3(0, -1, 0));
			}
		}
		else if (::_avatar && ::_avatar.getTargetObject() == so || null == so)
		{
			::Screens.hide("TargetWindow");
		}

		local lastTargetObject = this.mTargetObject;
		this.mTargetObject = so;

		if (lastTargetObject && lastTargetObject != this.mTargetObject)
		{
			lastTargetObject._positionName();
		}

		if (this.mTargetObject)
		{
			this.mTargetObject.setShowingSelection(true);
		}

		::_avatar.broadcastMessage("onTargetObjectChanged", this, this.mTargetObject);
	}

	function isNameShown()
	{
		if (this.mShowName == false)
		{
			return false;
		}

		if (this.mAssembler)
		{
			return this.mAssembler.getShowNameType();
		}

		return false;
	}

	function cork()
	{
		if (::_avatar == this)
		{
			this.log.debug("REZ: Corking");

			if (this.mCorkedStatusEffects)
			{
				this.log.debug("REZ: CorkedStatusEffects alive!!");
			}
		}

		this.mCorking = true;
		this.mCorkTimeout = this.gCorkTimeout;
	}

	function uncork()
	{
		if (::_avatar == this)
		{
			this.log.debug("REZ: Uncorking");
		}

		if (!this.mCorking)
		{
			this.log.debug("Uncorking malfunction");
			this.mCorkTimeout = 0;
			this.mCorkedStatusEffects = null;
			this.mCorkedStatusModifiers = null;
			this.mCorkedFloaties = [];
			this.mCorkedChatMessage = [];
			return;
		}

		local reztest_IsDead = this.StatusEffects.DEAD in this.mCorkedStatusEffects;

		if (::_avatar == this && this.mDead != reztest_IsDead)
		{
			if (this.mDead)
			{
				this.log.debug("REZ: uncorked: dead status changed: Rezed!");
			}
			else
			{
				this.log.debug("REZ: uncorked: dead status changed: Died!");
			}
		}

		this.mCorking = false;
		this.mCorkTimeout = 0;
		local CorkedStatusModifiers = this.mCorkedStatusModifiers;
		local CorkedStatusEffects = this.mCorkedStatusEffects;
		local CorkedFloaties = this.mCorkedFloaties;
		local CorkedCombatMessages = this.mCorkedChatMessage;
		this.mCorkedStatusModifiers = null;
		this.mCorkedStatusEffects = null;
		this.mCorkedFloaties = [];
		this.mCorkedChatMessage = [];

		if (CorkedStatusEffects && CorkedStatusModifiers)
		{
			local wasDead = this.mDead;
			this.setStatusModifiers(CorkedStatusModifiers, CorkedStatusEffects);

			if (!wasDead && this.mDead)
			{
				local appearanceOverride = this.getStat(this.Stat.APPEARANCE_OVERRIDE);
				local lootSeeablePlayerString = this.getStat(this.Stat.LOOT_SEEABLE_PLAYER_IDS);

				if (appearanceOverride && appearanceOverride != "" && lootSeeablePlayerString)
				{
					this.handleLootbagChange(lootSeeablePlayerString, appearanceOverride);
				}
			}
		}

		foreach( cf in CorkedFloaties )
		{
			this.addFloatie(cf.message, cf.type);
		}

		foreach( combatMessage in CorkedCombatMessages )
		{
			this.addCombatMessage(combatMessage.channel, combatMessage.message);
		}
	}

	function isInRange()
	{
		return this.mNameInRange;
	}

	function _positionName()
	{
		if (this.isAssembled() && this.isNameShown() && ::_avatar != this && this.mVisible && this.mNameInRange)
		{
			local namePlateOffset = 0;
			local showNameboard = true;

			if (this.getStat(this.Stat.HIDE_NAMEBOARD) == 1)
			{
				showNameboard = false;

				if (this.mNameBoard)
				{
					this.mNameBoard.destroy();
					this.mNameBoard = null;
				}
			}

			if (this.mNameBoard == null && showNameboard)
			{
				local scale = this.mNamePlateScale == null ? 1.0 : this.mNamePlateScale;
				this.mNameBoard = this._scene.createTextBoard(this.getNodeName() + "/NameBoard", "MaiandraOutline_16", 2.0 * scale, this.getName());
				this.mNode.attachObject(this.mNameBoard);
				this.mNameBoard.setYOffset(this.getNamePlatePosition().y);
				this.mNameBoard.setVisibilityFlags(this.VisibilityFlags.ANY | this.VisibilityFlags.FEEDBACK);
				this.updateNameBoardColor();
				local subName = this.getStat(this.Stat.SUB_NAME);

				if (subName != null && subName != "")
				{
					this.mNameBoard.setText(this.getName() + "\n<" + subName + ">");
				}
				else
				{
					this.mNameBoard.setText(this.getName());
				}
			}
			else if (!showNameboard)
			{
				if (this.mNameBoard)
				{
					this.mNameBoard.destroy();
					this.mNameBoard = null;
				}
			}

			if (this.mNameBoard)
			{
				this.mNameBoard.setVisibilityFlags(::_UIVisible == true ? this.VisibilityFlags.ANY | this.VisibilityFlags.FEEDBACK : 0);
			}
		}
		else if (this.mNameBoard)
		{
			this.mNameBoard.destroy();
			this.mNameBoard = null;
		}
	}

	function cue( visual, ... )
	{
		if (!this.mAssembled)
		{
			return;
		}

		if (this.gLogEffects)
		{
			this.log.debug("Visual Cue for " + this + ": " + visual);
		}

		local result;

		if (this.mEffectsHandler)
		{
			if (vargc > 2)
			{
				result = this.mEffectsHandler.addEffectNarrative(visual, vargv[0], vargv[1], vargv[2]);
			}
			else if (vargc > 1)
			{
				result = this.mEffectsHandler.addEffectNarrative(visual, vargv[0], vargv[1]);
			}
			else if (vargc > 0)
			{
				result = this.mEffectsHandler.addEffectNarrative(visual, vargv[0]);
			}
			else
			{
				result = this.mEffectsHandler.addEffectNarrative(visual);
			}
		}

		return result;
	}

	function playSound( sound, ... )
	{
		if (this.mSoundEmitters == null)
		{
			this.mSoundEmitters = [];

			if (this.mObjectClass == "Scenery")
			{
				this._sceneObjectManager.addUpdateListener(this);
			}
		}

		local emitter = this._audioManager.createSoundEmitter(sound);
		emitter.setAmbient(true);
		this.mSoundEmitters.append(emitter);
		this.mNode.attachObject(emitter);
		emitter.play();
		return emitter;
	}

	function stopSounds()
	{
		if (this.mSoundEmitters == null)
		{
			return;
		}

		foreach( e in this.mSoundEmitters )
		{
			e.stop();
			e.destroy();
		}

		this.mSoundEmitters = null;

		if (this.mObjectClass == "Scenery")
		{
			this._sceneObjectManager.removeUpdateListener(this);
		}
	}

	function getFloorAlignMode()
	{
		return this.mFloorAlignMode;
	}

	function setFloorAlignMode( alignMode )
	{
		this.mFloorAlignMode = alignMode;
	}

	function checkAndSetCTFFlag()
	{
		local carryingRedFlag = this.hasStatusEffect(this.StatusEffects.CARRYING_RED_FLAG);
		local carryingBlueFlag = this.hasStatusEffect(this.StatusEffects.CARRYING_BLUE_FLAG);

		if (carryingRedFlag || carryingBlueFlag)
		{
			this.mCarryingFlag = true;

			if (carryingRedFlag)
			{
				this.mFlag = this.Item.Attachable(null, "Armor-Base1A-Helmet", "back_sheathe");
				this.addAttachment(this.mFlag);
				this.mFlag.assemble();
			}
			else
			{
				this.mFlag = this.Item.Attachable(null, "Armor-Base1A-Helmet", "back_sheathe");
				this.addAttachment(this.mFlag);
				this.mFlag.assemble();
			}
		}
		else if (this.mCarryingFlag == true)
		{
			this.mCarryingFlag = false;
			this.removeAttachment(this.mFlag);
			this.mFlag = null;
		}
	}

	function _checkSounds()
	{
		if (this.mSoundEmitters == null)
		{
			return;
		}

		local newEmitters = [];

		foreach( e in this.mSoundEmitters )
		{
			if (!e.isPlaying())
			{
				e.destroy();
			}
			else
			{
				newEmitters.append(e);
			}
		}

		if (newEmitters.len() == 0)
		{
			this.mSoundEmitters = null;

			if (this.mObjectClass == "Scenery")
			{
				this._sceneObjectManager.removeUpdateListener(this);
			}
		}
		else
		{
			this.mSoundEmitters = newEmitters;
		}
	}

}

// Particles attached to props
this.SceneObject.mParticleAttachments <- {};
this.SceneObject.detachParticleSystem <- function(tag)
{
	if(tag in this.mParticleAttachments)
	{
		local particles = this.mParticleAttachments[tag];
		particles[0].destroy();
		particles[1].destroy();
		delete this.mParticleAttachments[tag];
	}
}
this.SceneObject.attachParticleSystem <- function(name, tag, size)
{
		// TODO make more unique
		local uniqueName = this.mNode.getName() + "/" + name;
		local particle = ::_scene.createParticleSystem(uniqueName, name);
		particle.setVisibilityFlags(this.VisibilityFlags.ANY);
		local particleNode = this.mNode.createChildSceneNode();
		particleNode.attachObject(particle);
		particleNode.setScale(this.Vector3(size, size, size));
		particle.setVisible(this.mAssembled);
		this.mParticleAttachments[tag] <- [particle, particleNode];
		return uniqueName;
}


	
/*
 * Override the 'setViaContentDef2' assembly to support 'ts' and 'es' for tail size and ear size
 * respectively
 */
 
this.Assembler.Creature.original_setViaContentDef2 <- this.Assembler.Creature.setViaContentDef2;
this.Assembler.Creature.setViaContentDef2 <- function(c) {
	local r = this.original_setViaContentDef2(c);
	if("ts" in c && mDetails != null) {
		foreach(i, d in mDetails)
			if(d.point == "tail")
				d["scale"] <- Vector3(c.ts.tofloat(),c.ts.tofloat(),c.ts.tofloat());
	}
	if("es" in c && mDetails != null) { 
		foreach(i, d in mDetails)
			if(d.point == "left_ear" || d.point == "right_ear")
				d["scale"] <- Vector3(c.es.tofloat(),c.es.tofloat(),c.es.tofloat());
	}
	return r;
}
	
/*
 * Override the 'applyEquipment' assembly to support 'ea', which is a list of extra
 * attachments (used for example in CTF script)
 */
this.Assembler.Creature.original_applyEquipment <- this.Assembler.Creature.applyEquipment;
this.Assembler.Creature.applyEquipment <- function(table) {
	local tableCopy = this.original_applyEquipment(table);
	if ("ea" in table)
	{
		local attachment;
		foreach(attachment in table.ea) {
			if("node" in attachment) {
				local entry = {
					node = attachment.node,
					type = attachment.type,
				};
				if ("colors" in attachment)
					entry.colors <- attachment.colors;

				if ("effect" in attachment)
					entry.effect <- attachment.effect;
				tableCopy.a.append(entry);
			}
		}
	}
	return tableCopy;
}

/*
 * Item market editor
 */
 

function InputCommands::MarketEdit(args)
{
	local frame = this.Screens.ItemMarketEditScreen();
	frame.setVisible(true);
}
 
