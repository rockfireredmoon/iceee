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

