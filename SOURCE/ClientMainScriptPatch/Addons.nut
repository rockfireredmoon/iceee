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
		
	case 41:  //Force start swimming. 
		if(::_avatar)
		{
			::_avatar.mSwimming = true;
			::_avatar.mController.onStartSwimming(0);
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
