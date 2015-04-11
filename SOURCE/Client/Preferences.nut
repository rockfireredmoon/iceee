this.require("UI/MapDef");
this.Pref <- {
	ACCOUNT_LOCAL = 0,
	CHARACTER_LOCAL = 1,
	SYSTEM_LOCAL = 2,
	ACCOUNT = 3,
	CHARACTER = 4,
	Account = null,
	Character = null
};
this.PreferenceDef <- {};
class this.PreferenceRequestHandler 
{
	mPrefs = "";
	constructor( prefs )
	{
		this.mPrefs = prefs;
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "pref.get")
		{
			this.gPreferenceCharacterUpdate = true;
		}

		local n;

		for( n = 0; n < results.len(); n++ )
		{
			local res = results[n];
			::Pref.set(this.mPrefs[n], res[0] == "" ? null : this.unserialize(res[0]), false);
		}
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error("" + qa.query + " failed: " + error);
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

}

class this.PreferenceSetHandler 
{
	constructor( ... )
	{
	}

	function onQueryComplete( qa, results )
	{
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error("" + qa.query + " failed: " + error);
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

}

this.Pref.get <- function ( setting )
{
	if ((setting in ::PreferenceDef) == false)
	{
		throw this.Exception("Invalid setting name passed to Pref::get: " + setting);
	}

	local entry = ::PreferenceDef[setting];
	local res = entry.currentValue != null ? entry.currentValue : entry.defaultValue;

	if (typeof res == "table" || typeof res == "array")
	{
		return clone res;
	}

	return res;
};
this.Pref.getDefault <- function ( setting )
{
	if ((setting in ::PreferenceDef) == false)
	{
		throw this.Exception("Invalid setting name passed to Pref::get: " + setting);
	}

	return ::PreferenceDef[setting].defaultValue;
};
this.Pref.set <- function ( setting, value, ... )
{
	if ((setting in ::PreferenceDef) == false)
	{
		throw this.Exception("Invalid setting name passed to Pref::set: " + setting);
	}

	local entry = ::PreferenceDef[setting];

	if (value != null)
	{
		value = this.Util.convertToType(value, typeof entry.defaultValue);
	}

	if (value != null && value == entry.currentValue)
	{
		return;
	}

	entry.currentValue = value;

	if (vargc < 2 || vargv[1] == true)
	{
		try
		{
			local cbname = "PreferenceUpdate_" + this.Util.replace(setting, ".", "_");

			if (cbname in ::Pref)
			{
				::Pref[cbname](value != null ? value : entry.defaultValue);
			}
		}
		catch( e )
		{
			this.log.error("Error while setting preference \'" + setting + "\': " + e);
		}
	}

	if (vargc == 0 || vargv[0] == true)
	{
		::Pref.savePref(setting);
	}
};
this.Pref.isDeclared <- function ( setting )
{
	return setting in ::PreferenceDef;
};
this.Pref.declare <- function ( setting, defaultValue, type )
{
	::PreferenceDef[setting] <- {
		defaultValue = defaultValue,
		currentValue = null,
		type = type
	};
};
function _loadDefaults( type )
{
	foreach( k, v in ::PreferenceDef )
	{
		if (v.type == type)
		{
			this.Pref.set(k, v.defaultValue, false);
		}
	}
}

function _loadPreferences( name, type )
{
	local prefs;

	try
	{
		prefs = this.unserialize(this._cache.getCookie(name));

		if (typeof prefs != "array")
		{
			throw this.Exception("Error reading preferences cookie");
		}
	}
	catch( e )
	{
		this.log.error("Could not load preferences, using defaults...");
		this._loadDefaults(type);
		return;
	}

	foreach( v in prefs )
	{
		this.Pref.set(v[0], v[1]);
	}
}

this.Pref.setAccount <- function ( account )
{
	this.Pref.Account = account;

	if (account != null)
	{
		this._loadPreferences("Preferences_AccountLocal_" + account, this.Pref.ACCOUNT_LOCAL);
	}
	else
	{
		this._loadDefaults(this.Pref.ACCOUNT_LOCAL);
	}

	this.Pref.setCharacter(null);
};
this.Pref.setCharacter <- function ( characterIndex )
{
	this.Pref.Character = characterIndex;

	if (this.Pref.Account == null && characterIndex != null)
	{
		throw this.Exception("Pref::setCharacter called without an account set");
	}

	if (this.Pref.Account != null && characterIndex != null)
	{
		this._loadPreferences("Preferences_CharacterLocal_" + this.Pref.Account + "_" + characterIndex, this.Pref.CHARACTER_LOCAL);
	}
	else
	{
		this._loadDefaults(this.Pref.CHARACTER_LOCAL);
	}
};
function _serializePreferences( type )
{
	local save = [];

	foreach( k, e in ::PreferenceDef )
	{
		if (e.type == type)
		{
			local value = this.Pref.get(k);
			save.append([
				k,
				value
			]);
		}
	}

	return this.serialize(save);
}

function _saveRemotePreferences()
{
	local save = [];

	foreach( k, e in ::PreferenceDef )
	{
		if (e.type == this.Pref.CHARACTER || e.type == this.Pref.ACCOUNT)
		{
			local value = this.Pref.get(k);
			::_Connection.sendQuery(e.type == this.Pref.CHARACTER ? "pref.set" : "pref.setA", this.PreferenceSetHandler(), [
				k,
				this.serialize(value)
			]);
		}
	}
}

this.Pref.findPreferences <- function ( group )
{
	local tmp = [];

	foreach( k, v in ::PreferenceDef )
	{
		if (k.find(group + ".") == 0)
		{
			tmp.append({
				name = k,
				value = this.Pref.get(k),
				type = typeof v.defaultValue
			});
		}
	}

	return tmp;
};
this.Pref.setDefaults <- function ()
{
	foreach( k, v in ::PreferenceDef )
	{
		this.Pref.set(k, v.defaultValue, false);
	}
};
this.Pref.savePref <- function ( name )
{
	try
	{
		if (name in ::PreferenceDef)
		{
			local def = ::PreferenceDef[name];

			if (def.type == this.Pref.CHARACTER || def.type == this.Pref.ACCOUNT)
			{
				local value = ::Pref.get(name);
				::_Connection.sendQuery(def.type == this.Pref.CHARACTER ? "pref.set" : "pref.setA", this.PreferenceSetHandler(), [
					name,
					this.serialize(value)
				]);
			}
			else
			{
				this._saveLocalPreferences();
			}
		}
	}
	catch( err )
	{
		this.log.debug("Error setting preference " + name + ": " + err);
	}
};
this.Pref._saveLocalPreferences <- function ()
{
	this._cache.setCookie("Preferences_SystemLocal", this._serializePreferences(this.Pref.SYSTEM_LOCAL));

	if (this.Pref.Account != null)
	{
		this._cache.setCookie("Preferences_AccountLocal_" + this.Pref.Account, this._serializePreferences(this.Pref.ACCOUNT_LOCAL));
	}

	if (this.Pref.Account != null && this.Pref.Character != null)
	{
		this._cache.setCookie("Preferences_CharacterLocal_" + this.Pref.Account + "_" + this.Pref.Character, this._serializePreferences(this.Pref.CHARACTER_LOCAL));
	}
};
this.Pref.save <- function ()
{
	this._saveLocalPreferences();
};
this.Pref.download <- function ( type )
{
	local names = [];
	local indexMap = {};
	local index = 0;

	foreach( k, v in ::PreferenceDef )
	{
		if (v.type == type)
		{
			indexMap[index++] <- k;
			names.append(k);
		}
	}

	if (names.len() > 0)
	{
		::_Connection.sendQuery(type == this.Pref.CHARACTER ? "pref.get" : "pref.getA", this.PreferenceRequestHandler(indexMap), names);
	}
};
this.Pref.PreferenceUpdate_video_Bloom <- function ( value )
{
	::_root.setTargetCompositorEnabled("Bloom", value);
};
this.Pref.PreferenceUpdate_video_Splatting <- function ( value )
{
	::_scene.setTerrainTechniqueOverride(value == true ? "" : "Base");
};
this.Pref.PreferenceUpdate_video_UICache <- function ( value )
{
	this.Util.updateUICache(value);
};
this.Pref.PreferenceUpdate_chatwindow_color <- function ( value )
{
	::_ChatWindow.unserializeColor(value);
};
this.Pref.PreferenceUpdate_chatwindow_chattabs <- function ( value )
{
	::_ChatWindow.unserializeChatTabs(value);
};
this.Pref.PreferenceUpdate_chatwindow_windowSize <- function ( value )
{
	::_ChatWindow.handleWindowResized(value);
};
this.Pref.PreferenceUpdate_screens_Positions <- function ( value )
{
	::Screens.loadSavePosition(value);
};
this.Pref.PreferenceUpdate_video_TerrainDistance <- function ( value )
{
	value = this.Math.clamp(value, 500, 3000);
	this.gCamera.farClippingDistance = value;
	this._scene.getCamera("Default").setFarClipDistance(value.tofloat());

	if (::_Environment)
	{
		::_Environment.setForceFogUpdate(true);
		::_Environment._blend();
	}
};
this.Pref.PreferenceUpdate_video_CharacterShadows <- function ( value )
{
	this.gShadows = value;

	if (value)
	{
		this._scene.setShadowTechnique(this.Scene.SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
		this._scene.setShadowDirLightTextureOffset(0.5);
		this._scene.setShadowFarDistance(this.gShadowDistance);
		this._scene.setShadowTextureSize(1024);
	}
	else
	{
		this._scene.setShadowTechnique(this.Scene.SHADOWTYPE_NONE);
		this._scene.setShadowFarDistance(0);
	}
};
this.Pref.PreferenceUpdate_igis_OverheadNames <- function ( value )
{
	if (value == "a")
	{
		foreach( creature in ::_sceneObjectManager.getCreatures() )
		{
			creature.setShowName(true);
		}
	}
	else if (value == "s")
	{
		foreach( creature in ::_sceneObjectManager.getCreatures() )
		{
			creature.setShowName("selected");
		}
	}
	else if (value == "n")
	{
		foreach( creature in ::_sceneObjectManager.getCreatures() )
		{
			creature.setShowName(false);
		}
	}
};
this.Pref.PreferenceUpdate_video_ClutterDistance <- function ( value )
{
	::_scene.setClutterDistance(value);
};
this.Pref.PreferenceUpdate_other_MouseSensitivity <- function ( value )
{
	this.gCamera.sensitivity = value;
};
this.Pref.PreferenceUpdate_control_Keybindings <- function ( bindings )
{
	if (::_playTool)
	{
		::_playTool.setCustomKeybindings(bindings);
	}
};
this.Pref.PreferenceUpdate_video_FSAA <- function ( value )
{
	::System.setMultisample(value);
};
this.Pref.PreferenceUpdate_video_ClutterVisible <- function ( value )
{
	::_scene.setClutterVisible(value);
};
this.Pref.PreferenceUpdate_video_ClutterDensity <- function ( value )
{
	::_scene.setClutterDensity(value);
};
this.Pref.PreferenceUpdate_video_Settings <- function ( value )
{
};
this.Pref.PreferenceUpdate_audio_Music <- function ( value )
{
	::Audio.setMusicMuted(value == false, this.Audio.DEFAULT_CHANNEL);
	::_root.setAudioChannelMuted("Music", value == false);
};
this.Pref.PreferenceUpdate_audio_Mute <- function ( value )
{
	::Audio.setMuted(value);
};
this.Pref.PreferenceUpdate_audio_Sounds <- function ( value )
{
	::Audio.setMusicMuted(value, this.Audio.NOISE_CHANNEL);
	::Audio.setAmbientMuted(value);
};
this.Pref.PreferenceUpdate_audio_MusicLevel <- function ( value )
{
	::_root.setAudioChannelVolume("Music", value);
};
this.Pref.PreferenceUpdate_audio_CombatSFXLevel <- function ( value )
{
	::_root.setAudioChannelVolume("Combat", value);
};
this.Pref.PreferenceUpdate_audio_AmbientSFXLevel <- function ( value )
{
	::_root.setAudioChannelVolume("Ambient", value);
};
this.Pref.PreferenceUpdate_map_LegendItems <- function ( value )
{
	local mapWindow = this.Screens.get("MapWindow", true);
	mapWindow.setSelectedLegendItems(value);
};
this.Pref.PreferenceUpdate_map_ZoomLevel <- function ( value )
{
	local mapWindow = this.Screens.get("MapWindow", true);
	mapWindow.setWindowZoomLevel(value);
};
this.Pref.PreferenceUpdate_map_MapType <- function ( value )
{
	local mapWindow = this.Screens.get("MapWindow", true);
	mapWindow.setMapType(value);
};
this.Pref.PreferenceUpdate_minimap_ZoomScale <- function ( value )
{
	local miniMapWindow = this.Screens.get("MiniMapScreen", false);

	if (miniMapWindow)
	{
		miniMapWindow.setZoomScale(value);
	}
};
this.Pref.PreferenceUpdate_quest_QuestMarkerType <- function ( value )
{
	local questJournal = this.Screens.get("QuestJournal", true);
	questJournal.setSelectedQuestMarkerType(value);
};
this.Pref.PreferenceUpdate_quest_CurrentSelectedQuest <- function ( value )
{
	local questJournal = this.Screens.get("QuestJournal", true);
	questJournal.setSelectedQuest(value);
};
this.Pref.PreferenceUpdate_chat_ProfanityFilter <- function ( value )
{
	::UI.ChatManager.setProfanityFilter(value);
};
this.Pref.PreferenceUpdate_chat_BoldText <- function ( value )
{
	local chatWindow = this.Screens.get("ChatWindow", false);

	if (chatWindow)
	{
		chatWindow.updateBoldness(value);
	}
};
this.Pref.PreferenceUpdate_tutorial_active <- function ( value )
{
	::_tutorialManager.setTutorialsActive(value);
};
this.Pref.PreferenceUpdate_tutorial_seen <- function ( value )
{
	if (value != "")
	{
		::_tutorialManager.unserialize(value);
	}
};
this.Pref.PreferenceUpdate_tutorial_diaplayTutorial <- function ( value )
{
	if (value != "")
	{
		::_tutorialManager.updateDisplayedTutorials(value);
	}
};
this.Pref.PreferenceUpdate_gameplay_mousemovement <- function ( value )
{
};
this.Pref.PreferenceUpdate_gameplay_eqcomparisons <- function ( value )
{
};
this.Pref.resetRemote <- function ()
{
	foreach( k, v in ::PreferenceDef )
	{
		if (v.type == this.Pref.CHARACTER || v.type == this.Pref.ACCOUNT)
		{
			::Pref.set(k, null, false);
		}
	}

	this.gPreferenceCharacterUpdate = false;
};
this.Pref.declare("igis.AutoPopup", true, this.Pref.CHARACTER_LOCAL);
this.Pref.declare("igis.OverheadNames", "a", this.Pref.ACCOUNT_LOCAL);
this.Pref.declare("login.Credentials", "", this.Pref.SYSTEM_LOCAL);
this.Pref.declare("login.LastCharacter", 0, this.Pref.ACCOUNT_LOCAL);
this.Pref.declare("build.AdvancedMode", false, this.Pref.ACCOUNT_LOCAL);
this.Pref.declare("comm.AutoIM", true, this.Pref.ACCOUNT_LOCAL);
this.Pref.declare("comm.Tidy", true, this.Pref.ACCOUNT_LOCAL);
this.Pref.declare("mail.ConfirmAttachmentSend", true, this.Pref.CHARACTER_LOCAL);
this.Pref.declare("video.Splatting", true, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.Bloom", true, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.TerrainDistance", 2500, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.CharacterShadows", false, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.ClutterDistance", 350.0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.ClutterDensity", 1.0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.ClutterVisible", true, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.FSAA", 0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.Settings", "None", this.Pref.SYSTEM_LOCAL);
this.Pref.declare("video.UICache", true, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("other.MouseSensitivity", 0.15000001, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("control.Keybindings", [], this.Pref.ACCOUNT);
this.Pref.declare("other.BindPopup", true, this.Pref.ACCOUNT);
this.Pref.declare("quickbar.0", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.1", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.2", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.3", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.4", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.5", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.6", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.7", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.8", "", this.Pref.CHARACTER);
this.Pref.declare("quickbar.9", "", this.Pref.CHARACTER);
this.Pref.declare("map.LegendItems", {
	[this.LegendItemTypes.DEFAULT] = false,
	[this.LegendItemTypes.YOU] = true,
	[this.LegendItemTypes.TOWN_GATE] = false,
	[this.LegendItemTypes.QUEST] = true,
	[this.LegendItemTypes.QUEST_GIVER] = false,
	[this.LegendItemTypes.HENGE] = false,
	[this.LegendItemTypes.SANCTUARY] = false,
	[this.LegendItemTypes.SHOP] = true,
	[this.LegendItemTypes.VAULT] = false,
	[this.LegendItemTypes.CITY] = false,
	[this.LegendItemTypes.PARTY] = true,
	[this.LegendItemTypes.ANIMAL] = true,
	[this.LegendItemTypes.DEMON] = true,
	[this.LegendItemTypes.DIVINE] = true,
	[this.LegendItemTypes.DRAGONKIN] = true,
	[this.LegendItemTypes.ELEMENTAL] = true,
	[this.LegendItemTypes.MAGICAL] = true,
	[this.LegendItemTypes.MORTAL] = true,
	[this.LegendItemTypes.UNLIVING] = true
}, this.Pref.CHARACTER);
this.Pref.declare("map.ZoomLevel", "World", this.Pref.CHARACTER);
this.Pref.declare("map.MapType", "World", this.Pref.CHARACTER);
this.Pref.declare("minimap.ZoomScale", 4096.0, this.Pref.CHARACTER);
this.Pref.declare("audio.Volume", 1.0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("audio.Sounds", false, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("audio.Music", true, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("audio.Mute", false, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("audio.CombatSFXLevel", 1.0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("audio.AmbientSFXLevel", 1.0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("audio.MusicLevel", 1.0, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("chat.ignoreList", {}, this.Pref.ACCOUNT);
this.Pref.declare("chat.ProfanityFilter", true, this.Pref.ACCOUNT);
this.Pref.declare("chat.BoldText", false, this.Pref.CHARACTER);
this.Pref.declare("tutorial.active", true, this.Pref.ACCOUNT);
this.Pref.declare("tutorial.seen", "", this.Pref.ACCOUNT);
this.Pref.declare("tutorial.diaplayTutorial", "", this.Pref.ACCOUNT_LOCAL);
this.Pref.declare("gameplay.mousemovement", false, this.Pref.ACCOUNT);
this.Pref.declare("gameplay.eqcomparisons", true, this.Pref.ACCOUNT);
this.Pref.declare("debug.LoadScreen", false, this.Pref.SYSTEM_LOCAL);
this.Pref.declare("quest.QuestMarkerType", {
	[0] = {
		questId = -1,
		isSelected = false
	},
	[1] = {
		questId = -1,
		isSelected = false
	},
	[2] = {
		questId = -1,
		isSelected = false
	},
	[3] = {
		questId = -1,
		isSelected = false
	}
}, this.Pref.CHARACTER);
this.Pref.declare("quest.CurrentSelectedQuest", -1, this.Pref.CHARACTER);
this.Pref.declare("chatwindow.color", {
	r = 0.0,
	g = 0.0,
	b = 0.0,
	a = 0.0
}, this.Pref.CHARACTER);
this.Pref.declare("chatwindow.chattabs", [
	{
		name = "General",
		filters = {
			Say = true,
			Emote = true,
			Tell = true,
			Clan = true,
			["Clan Officer"] = true,
			Party = true,
			Region = true,
			Trade = true,
			System = true,
			["Private Channel"] = true
		}
	},
	{
		name = "Combat",
		filters = {
			["My Combat Incoming"] = true,
			["My Combat Outgoing"] = true,
			["Other Player Combat Incoming"] = true,
			["Other Player Combat Outgoing"] = true
		}
	}
], this.Pref.CHARACTER);
this.Pref.declare("chatwindow.windowSize", "Small", this.Pref.CHARACTER);
this.Pref.declare("screens.Positions", "", this.Pref.ACCOUNT_LOCAL);
