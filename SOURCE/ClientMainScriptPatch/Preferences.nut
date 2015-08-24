/**
	A small API for consistent accessing of user preferences.
	
	Preferences are stored in several different locations and apply to
	accounts and characters depending on their "scope":
	<ul>
	<li><b>SystemLocal</b> &mdash; Stored on user's PC, applies to
		all characters on all accounts.</li>
	<li><b>AccountLocal</b> &mdash; Stored on user's PC, applies to
		all characters of a single account.</li> 
	<li><b>CharacterLocal</b> &mdash; Stored on user's PC, applies to
		a single character of a single account.</li>
	<li><b>Account</b> &mdash; Stored on server, applies to
		all characters of a single account.</li> 
	<li><b>Character</b> &mdash; Stored on server, applies to
		a single character of a single account.</li> 
	</ul>
	
	@see <a href="https://wiki.sparkplaymedia.com/display/EE/Preferences">Preferences</a> 
*/

require("UI/MapDef");

Pref <- {

	/**
		Account-level preferences that are stored locally
	*/
	ACCOUNT_LOCAL = 0,
	
	/**
		Character-level preferences that are stored locally
	*/
	CHARACTER_LOCAL = 1,
	
	/**
		System-level preferences that are stored locally
	*/
	SYSTEM_LOCAL = 2,
	
	/**
		Account-level preferences that are stored on the server
	*/
	ACCOUNT = 3,
	
	/**
		Character-level preferences that are stored on the server
	*/
	CHARACTER = 4,
	
	Account = null,
	Character = null
};


/**
	A runtime table of preference definitions.
	@see Pref#declare
*/
PreferenceDef <- {};


/**
	Handles preference request replies from the server
*/
class PreferenceRequestHandler {
	mPrefs = "";
	constructor(prefs) {
		mPrefs = prefs;
	}

	function onQueryComplete(qa, results)	{
		if (qa.query == "pref.get")
			gPreferenceCharacterUpdate = true;

		for(local n = 0; n < results.len(); n++ ) {
			local res = results[n];
			::Pref.set(mPrefs[n], res[0] == "" ? null : unserialize(res[0]), false);
		}
	}

	function onQueryError(qa, error) {
		IGIS.error("" + qa.query + " failed: " + error);
	}

	function onQueryTimeout(qa) {
		::_Connection.sendQuery(qa.query, this, qa.args);
	}
}


/**
	Handles a request to set a preference on the server
*/
class PreferenceSetHandler {
	constructor(...) { }

	function onQueryComplete( qa, results ) {
	}

	function onQueryError(qa, error) {
		IGIS.error("" + qa.query + " failed: " + error);
	}

	function onQueryTimeout(qa) {
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

}

/**
	Retrieve the current value of a named configuration setting. If the
	value has not been explicitly set, the builtin default value should
	be returned.
	
	@param setting
		The name of a setting to look up. E.g. "video.Bloom"
	
	@returns The configuration setting's current (or default) value (type varies).
	
	@see #getDefault
	@throws If the setting name is not recognized.
*/
function Pref::get(setting) {
	// Make sure this is a valid setting name
	if ((setting in ::PreferenceDef) == false)
		throw Exception("Invalid setting name passed to Pref::get: " + setting);

	// Get the entry for the preference
	local entry = ::PreferenceDef[setting];

	// Return current value if set, or default value otherwise
	local res = entry.currentValue != null ? entry.currentValue : entry.defaultValue;

	if (typeof res == "table" || typeof res == "array")
		return clone res;

	return res;
}

/**
	Retrieve the builtin default value of a named configuration setting.
	
	@param setting
		The name of a setting to look up. E.g. "video.Bloom"
	
	@returns The configuration setting's built-in defaul) value (type varies).
	
	@throws If the setting name is not recognized.
*/
function Pref::getDefault(setting) {
	// Make sure this is a valid setting name
	if ((setting in ::PreferenceDef) == false)
		throw Exception("Invalid setting name passed to Pref::get: " + setting);

	// Return default value of entry
	return ::PreferenceDef[setting].defaultValue;
}

/**
	Set the configuration setting to a new value. The value is in a string
	representation and it should automatically be converted to the necessary
	type (if this is even needed). If the new value is identical to the
	current one (or default value if it hasn't been explicitly set),
	this may safely return without doing anything. However, if the setting
	changes, it should be immediately saved to whatever backing store
	is associated with that setting (set during preference declarations).
	
	@param setting
		The name of a setting to look up. E.g. "video.Bloom"
	@param value
		The string representation of the value for this setting. E.g.
		"true" or "1". If this is <tt>null</tt>, then a customized
		value should be cleared and the setting reverted to its 
		default value.
	@param save ( optional )
		A boolean that specifies whether or not to save the preferences
		to disk after changing them. Default is true.
	@param callback ( optional )
		Performs a callback to notify the system that the preference has changed.
		Default is true.
	
	@returns True if the setting actually changed, or false if not.
	@throws If the setting name is not recognized.
	@throws If the value provided is invalid for the given setting.
*/
function Pref::set(setting, value, ...) {
	// Make sure this is a valid setting name
	if ((setting in ::PreferenceDef) == false)
		throw Exception("Invalid setting name passed to Pref::set: " + setting);

	// Get the preference entry
	local entry = ::PreferenceDef[setting];

	if (value != null)
		value = Util.convertToType(value, typeof entry.defaultValue);

	// If the values are the same, do nothing
	if (value != null && value == entry.currentValue)
		return;

	// Update value on entry
	entry.currentValue = value;

	if (vargc < 2 || vargv[1] == true) {
		try {
			local cbname = "PreferenceUpdate_" + Util.replace(setting, ".", "_");

			if (cbname in ::Pref) {
				::Pref[cbname](value != null ? value : entry.defaultValue);
			}
		}
		catch(e) {
			log.error("Error while setting preference \'" + setting + "\': " + e);
		}
	}
	
	// Save settings to disk/network
	if (vargc == 0 || vargv[0] == true) {
		::Pref.savePref(setting);
	}
}

/**
	Is a preference setting with the given name declared?
	
	@param setting
		The name of the setting to check. E.g. "video.Bloom"
*/
function Pref::isDeclared(setting) {
	return setting in ::PreferenceDef;
}

/**
	Declare a new preference type to the preference system. This is done
	during initialization and sets up the scoping (type) and the default
	value associated with the setting.
*/
function Pref::declare(setting, defaultValue, type ) {
	::PreferenceDef[setting] <- {
		defaultValue = defaultValue,
		currentValue = null,
		type = type
	};
}

/**
	Loads the default preferences for a certain preference type
*/
function _loadDefaults(type) {
	foreach(k, v in ::PreferenceDef)
		if (v.type == type)
			Pref.set(k, v.defaultValue, false);
}


/**
	Loads the preferences from a cookie name and sets them
*/
function _loadPreferences( name, type ) {
	local prefs;

	try	{
		// Grab the preferences from the cache
		prefs = unserialize(_cache.getCookie(name));
		if (typeof prefs != "array")
			throw Exception("Error reading preferences cookie");
	}
	catch(e) {
		// The preferences could not be loaded, so just use the defaults
		log.error("Could not load preferences, using defaults...");
		
		// Set the default preferences
		
		_loadDefaults(type);
		
		// All done!
		return;
	}

		
	// Set the preferences that were stored in the cookie
	foreach(v in prefs)
		Pref.set(v[0], v[1]);
}

/**
	Set the account currently being used. This affects what set of settings
	are used for Account and AccountLocal type preferences. The new settings
	should be loaded and take effect immediately. If there are no custom
	settings for this account, defaults are used.
	
	@param account The name of the account the user has selected (to login).
*/
function Pref::setAccount(account) {
	Pref.Account = account;
	
	// Load and set the preferences, or use defaults if we no longer are using a specific account

	if (account != null)
		_loadPreferences("Preferences_AccountLocal_" + account, Pref.ACCOUNT_LOCAL);
	else
		_loadDefaults(Pref.ACCOUNT_LOCAL);
		
	// Reset character preferences

	Pref.setCharacter(null);
}

/**
	Set the character being used by the current account. This affects what
	Character and CharacterLocal preference settings are used. Like setAccount(),
	this should immediately load and affect any settings as necessary.
	
	@param character The character index (within the account) of the 
		character that the user has selected to login with.
*/
function Pref::setCharacter(characterIndex) {
	Pref.Character = characterIndex;
	
	// Make sure the account has been set first

	if (Pref.Account == null && characterIndex != null)
		throw Exception("Pref::setCharacter called without an account set");
	
	// Load and set the preferences, or use defaults if we no longer are using a specific character

	if (Pref.Account != null && characterIndex != null)
		_loadPreferences("Preferences_CharacterLocal_" + Pref.Account + "_" + characterIndex, Pref.CHARACTER_LOCAL);
	else
		_loadDefaults(Pref.CHARACTER_LOCAL);
}

/**
	Serializes all preferences of a specified type to a list of {name, value} pairs
	for saving to a cookie.
*/
function _serializePreferences( type ) {
	local save = [];

	foreach( k, e in ::PreferenceDef ) {
		if (e.type == type) {
			local value = Pref.get(k);
			save.append([
				k,
				value
			]);
		}
	}

	return serialize(save);
}


/**
	Saves all preferences to the server
*/
function _saveRemotePreferences() {
	local save = [];

	foreach( k, e in ::PreferenceDef ) {
		if (e.type == Pref.CHARACTER || e.type == Pref.ACCOUNT) {
			local value = Pref.get(k);
			::_Connection.sendQuery(e.type == Pref.CHARACTER ? "pref.set" : "pref.setA", PreferenceSetHandler(), [
				k,
				serialize(value)
			]);
		}
	}
}

/**
	Returns a list of info for preferences that belong to a specific group
*/
function Pref::findPreferences( group ) {
	local tmp = [];

	foreach( k, v in ::PreferenceDef ) 
		if (k.find(group + ".") == 0)
			tmp.append({
				name = k,
				value = Pref.get(k),
				type = typeof v.defaultValue
			});

	return tmp;
}

/**
	Loads the defaults of all preferences
*/
function Pref::setDefaults() {
	foreach( k, v in ::PreferenceDef ) 
		Pref.set(k, v.defaultValue, false);
}

/**
	Saves a preference immediately
*/
function Pref::savePref( name ) {
	try	{
		if (name in ::PreferenceDef) {
			local def = ::PreferenceDef[name];
	
			// If the preference is remote, then save it to the server

			if (def.type == Pref.CHARACTER || def.type == Pref.ACCOUNT) {
				local value = ::Pref.get(name);
				::_Connection.sendQuery(def.type == Pref.CHARACTER ? "pref.set" : "pref.setA", PreferenceSetHandler(), [
					name,
					serialize(value)
				]);
			}
			else
				_saveLocalPreferences();
		}
	}
	catch(err) {
		log.debug("Error setting preference " + name + ": " + err);
	}
}

/**
	Saves all local preferences to the cookies
*/
function Pref::_saveLocalPreferences( ) {
	_cache.setCookie("Preferences_SystemLocal", _serializePreferences(Pref.SYSTEM_LOCAL));

	if (Pref.Account != null)
		_cache.setCookie("Preferences_AccountLocal_" + Pref.Account, _serializePreferences(Pref.ACCOUNT_LOCAL));

	if (Pref.Account != null && Pref.Character != null)
		_cache.setCookie("Preferences_CharacterLocal_" + Pref.Account + "_" + Pref.Character, _serializePreferences(Pref.CHARACTER_LOCAL));
}

/**
	Saves the preferences to cookies/server
*/
function Pref::save( ) {
	_saveLocalPreferences();
}

/**
	Requests server-side preferences to be downloaded
*/
function Pref::download( type ) {
	local names = [];
	local indexMap = {};
	local index = 0;

	foreach( k, v in ::PreferenceDef ) {
		if (v.type == type)	{
			indexMap[index++] <- k;
			names.append(k);
		}
	}

	if (names.len() > 0)
		::_Connection.sendQuery(type == Pref.CHARACTER ? "pref.get" : "pref.getA", PreferenceRequestHandler(indexMap), names);
}


/**
	Called when the bloom preference has been updated
*/
Pref.PreferenceUpdate_video_Bloom <- function ( value ) {
	::_root.setTargetCompositorEnabled("Bloom", value);
}

Pref.PreferenceUpdate_video_Splatting <- function ( value ) {
	::_scene.setTerrainTechniqueOverride(value == true ? "" : "Base");
}

Pref.PreferenceUpdate_video_UICache <- function ( value ) {
	Util.updateUICache(value);
}

Pref.PreferenceUpdate_chatwindow_color <- function ( value ) {
	if(::_ChatWindow)
		::_ChatWindow.unserializeColor(value);
}

Pref.PreferenceUpdate_chatwindow_chattabs <- function ( value ) {
	if(::_ChatWindow)
		::_ChatWindow.unserializeChatTabs(value);
}

Pref.PreferenceUpdate_chatwindow_windowSize <- function ( value ) {
	if(::_ChatWindow)
		::_ChatWindow.handleWindowResized(value);
}

Pref.PreferenceUpdate_screens_Positions <- function (value) {
	::Screens.loadSavePosition(value);
}

/**
	Called when the terrain distance preference has been updated
*/
Pref.PreferenceUpdate_video_TerrainDistance <- function (value) {
	value = Math.clamp(value, 500, 3000);
	gCamera.farClippingDistance = value;
	_scene.getCamera("Default").setFarClipDistance(value.tofloat());

	if (::_Environment) {
		::_Environment.setForceFogUpdate(true);
		::_Environment._blend();
	}
}


/**
	Called when the character shadow preference has been updated
*/
Pref.PreferenceUpdate_video_CharacterShadows <- function (value) {
	gShadows = value;

	if (value) {
		_scene.setShadowTechnique(Scene.SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
		_scene.setShadowDirLightTextureOffset(0.5);
		_scene.setShadowFarDistance(gShadowDistance);
		_scene.setShadowTextureSize(1024);
	}
	else {
		_scene.setShadowTechnique(Scene.SHADOWTYPE_NONE);
		_scene.setShadowFarDistance(0);
	}
}

/**
	Called when the names over the head of characters preference has been updated
*/
Pref.PreferenceUpdate_igis_OverheadNames <- function ( value ) {
	if (value == "a") {
		foreach(creature in ::_sceneObjectManager.getCreatures())
			creature.setShowName(true);
	}
	else if (value == "s") {
		foreach(creature in ::_sceneObjectManager.getCreatures())
			creature.setShowName("selected");
	}
	else if (value == "n")
	{
		foreach(creature in ::_sceneObjectManager.getCreatures())
			creature.setShowName(false);
	}
}


/**
	Called when the clutter distance has been updated
*/
Pref.PreferenceUpdate_video_ClutterDistance <- function (value) {
	::_scene.setClutterDistance(value);
}


/**
	Called when the mouse sensitivity has been updated
*/
Pref.PreferenceUpdate_other_MouseSensitivity <- function (value) {
	gCamera.sensitivity = value;
}

Pref.PreferenceUpdate_control_Keybindings <- function (bindings) {
	if (::_playTool)
		::_playTool.setCustomKeybindings(bindings);
}

/**
	Called when the multisample amount has been updated
*/
Pref.PreferenceUpdate_video_FSAA <- function (value) {
	::System.setMultisample(value);
}

/**
	Called when the clutter distance has been updated
*/
Pref.PreferenceUpdate_video_ClutterVisible <- function (value) {
	::_scene.setClutterVisible(value);
}

/**
	Called when the clutter density has been updated
*/
Pref.PreferenceUpdate_video_ClutterDensity <- function (value) {
	::_scene.setClutterDensity(value);
}

Pref.PreferenceUpdate_video_Settings <- function (value) {
}

/**
	Called when the music preference has been updated
*/
Pref.PreferenceUpdate_audio_Music <- function (value) {
	::Audio.setMusicMuted(value == false, Audio.DEFAULT_CHANNEL);
	::_root.setAudioChannelMuted("Music", value == false);
}

/**
	Called when the mute preference has been updated
*/
Pref.PreferenceUpdate_audio_Mute <- function (value) {
	::Audio.setMuted(value);
}

/**
	Called when the sound preference has been updated
*/
Pref.PreferenceUpdate_audio_Sounds <- function (value) {
	::Audio.setMusicMuted(value, Audio.NOISE_CHANNEL);
	::Audio.setAmbientMuted(value);
}

/**
	Called when the music channel level changes
*/
Pref.PreferenceUpdate_audio_MusicLevel <- function (value) {
	::_root.setAudioChannelVolume("Music", value);
}

/**
	Called when the combat SFX level changes
*/
Pref.PreferenceUpdate_audio_CombatSFXLevel <- function (value) {
	::_root.setAudioChannelVolume("Combat", value);
}

/**
	Called when the ambient SFX level changes
*/
Pref.PreferenceUpdate_audio_AmbientSFXLevel <- function (value) {
	::_root.setAudioChannelVolume("Ambient", value);
}

/**
	Called when selected legend item is pressed
*/
Pref.PreferenceUpdate_map_LegendItems <- function (value) {
	local mapWindow = Screens.get("MapWindow", true);
	mapWindow.setSelectedLegendItems(value);
}

/**
	Called when the map zoom level is pressed
*/
Pref.PreferenceUpdate_map_ZoomLevel <- function (value) {
	local mapWindow = Screens.get("MapWindow", true);
	mapWindow.setWindowZoomLevel(value);
}

/**
	Called when the map zoom level is pressed
*/
Pref.PreferenceUpdate_map_MapType <- function (value) {
	local mapWindow = Screens.get("MapWindow", true);
	mapWindow.setMapType(value);
}

Pref.PreferenceUpdate_minimap_ZoomScale <- function (value) {
	local miniMapWindow = Screens.get("MiniMapScreen", false);

	if (miniMapWindow)
		miniMapWindow.setZoomScale(value);
}

/**
	Called when a quest is used as a marker for tracking and for the map
*/
Pref.PreferenceUpdate_quest_QuestMarkerType <- function (value) {
	local questJournal = Screens.get("QuestJournal", true);
	questJournal.setSelectedQuestMarkerType(value);
}

/**
	Called when a quest is used as a marker for tracking and for the map
*/
Pref.PreferenceUpdate_quest_CurrentSelectedQuest <- function (value) {
	local questJournal = Screens.get("QuestJournal", true);
	questJournal.setSelectedQuest(value);
}

/**
	Called when profanity filter check box is toggled
*/
Pref.PreferenceUpdate_chat_ProfanityFilter <- function (value) {
	::UI.ChatManager.setProfanityFilter(value);
}

Pref.PreferenceUpdate_chat_BoldText <- function (value) {
	local chatWindow = Screens.get("ChatWindow", false);
	if (chatWindow)
		chatWindow.updateBoldness(value);
};

Pref.PreferenceUpdate_tutorial_active <- function (value) {
	if(::_tutorialManager)
		::_tutorialManager.setTutorialsActive(value);
}

Pref.PreferenceUpdate_tutorial_seen <- function (value) {
	if (value != "")
		::_tutorialManager.unserialize(value);
}

Pref.PreferenceUpdate_tutorial_diaplayTutorial <- function (value) {
	if (value != "")
		::_tutorialManager.updateDisplayedTutorials(value);
}

Pref.PreferenceUpdate_gameplay_mousemovement <- function (value) {
}

Pref.PreferenceUpdate_gameplay_eqcomparisons <- function (value) {
}

/**
	Resets all remote preferences
*/
function Pref::resetRemote( ) {
	foreach(k, v in ::PreferenceDef)
	{
		if (v.type == Pref.CHARACTER || v.type == Pref.ACCOUNT)
			::Pref.set(k, null, false);
	}

	gPreferenceCharacterUpdate = false;
}

// Declare all of the preferences

Pref.declare("igis.AutoPopup", true, Pref.CHARACTER_LOCAL);
Pref.declare("igis.OverheadNames", "a", Pref.ACCOUNT_LOCAL);

Pref.declare("login.Credentials", "", Pref.SYSTEM_LOCAL);
Pref.declare("login.LastCharacter", 0, Pref.ACCOUNT_LOCAL);

Pref.declare("build.AdvancedMode", false, Pref.ACCOUNT_LOCAL);

Pref.declare("comm.AutoIM", true, Pref.ACCOUNT_LOCAL);
Pref.declare("comm.Tidy", true, Pref.ACCOUNT_LOCAL);

Pref.declare("mail.ConfirmAttachmentSend", true, Pref.CHARACTER_LOCAL);

Pref.declare("video.Splatting", true, Pref.SYSTEM_LOCAL);
Pref.declare("video.Bloom", true, Pref.SYSTEM_LOCAL);
Pref.declare("video.TerrainDistance", 2500, Pref.SYSTEM_LOCAL);
Pref.declare("video.CharacterShadows", false, Pref.SYSTEM_LOCAL);
Pref.declare("video.ClutterDistance", 350.0, Pref.SYSTEM_LOCAL);
Pref.declare("video.ClutterDensity", 1.0, Pref.SYSTEM_LOCAL);
Pref.declare("video.ClutterVisible", true, Pref.SYSTEM_LOCAL);
Pref.declare("video.FSAA", 0, Pref.SYSTEM_LOCAL);
Pref.declare("video.Settings", "None", Pref.SYSTEM_LOCAL);
Pref.declare("video.UICache", true, Pref.SYSTEM_LOCAL);
Pref.declare("other.MouseSensitivity", 0.15000001, Pref.SYSTEM_LOCAL);

Pref.declare("control.Keybindings", [], Pref.ACCOUNT);

Pref.declare("other.BindPopup", true, Pref.ACCOUNT);

Pref.declare("quickbar.0", "", Pref.CHARACTER);
Pref.declare("quickbar.1", "", Pref.CHARACTER);
Pref.declare("quickbar.2", "", Pref.CHARACTER);
Pref.declare("quickbar.3", "", Pref.CHARACTER);
Pref.declare("quickbar.4", "", Pref.CHARACTER);
Pref.declare("quickbar.5", "", Pref.CHARACTER);
Pref.declare("quickbar.6", "", Pref.CHARACTER);
Pref.declare("quickbar.7", "", Pref.CHARACTER);
Pref.declare("quickbar.8", "", Pref.CHARACTER);
Pref.declare("quickbar.9", "", Pref.CHARACTER);

Pref.declare("map.LegendItems", {
	[LegendItemTypes.DEFAULT] = false,
	[LegendItemTypes.YOU] = true,
	[LegendItemTypes.TOWN_GATE] = false,
	[LegendItemTypes.QUEST] = true,
	[LegendItemTypes.QUEST_GIVER] = false,
	[LegendItemTypes.HENGE] = false,
	[LegendItemTypes.SANCTUARY] = false,
	[LegendItemTypes.SHOP] = true,
	[LegendItemTypes.VAULT] = false,
	[LegendItemTypes.CITY] = false,
	[LegendItemTypes.PARTY] = true,
	[LegendItemTypes.ANIMAL] = true,
	[LegendItemTypes.DEMON] = true,
	[LegendItemTypes.DIVINE] = true,
	[LegendItemTypes.DRAGONKIN] = true,
	[LegendItemTypes.ELEMENTAL] = true,
	[LegendItemTypes.MAGICAL] = true,
	[LegendItemTypes.MORTAL] = true,
	[LegendItemTypes.UNLIVING] = true
}, Pref.CHARACTER);

Pref.declare("map.ZoomLevel", "World", Pref.CHARACTER);
Pref.declare("map.MapType", "World", Pref.CHARACTER);

Pref.declare("minimap.ZoomScale", 4096.0, Pref.CHARACTER);

Pref.declare("audio.Volume", 1.0, Pref.SYSTEM_LOCAL);
Pref.declare("audio.Sounds", false, Pref.SYSTEM_LOCAL);
Pref.declare("audio.Music", true, Pref.SYSTEM_LOCAL);
Pref.declare("audio.Mute", false, Pref.SYSTEM_LOCAL);
Pref.declare("audio.CombatSFXLevel", 1.0, Pref.SYSTEM_LOCAL);
Pref.declare("audio.AmbientSFXLevel", 1.0, Pref.SYSTEM_LOCAL);
Pref.declare("audio.MusicLevel", 1.0, Pref.SYSTEM_LOCAL);

Pref.declare("chat.ignoreList", {}, Pref.ACCOUNT);
Pref.declare("chat.ProfanityFilter", true, Pref.ACCOUNT);
Pref.declare("chat.BoldText", false, Pref.CHARACTER);
Pref.declare("chatwindow.color", {
								r = 0.0,
								g = 0.0,
								b = 0.0,
								a = 0.0
							}, Pref.CHARACTER);
Pref.declare("chatwindow.chattabs", [
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
], Pref.CHARACTER);
Pref.declare("chatwindow.windowSize", "Small", Pref.CHARACTER);

Pref.declare("tutorial.active", true, Pref.ACCOUNT);
Pref.declare("tutorial.seen", "", Pref.ACCOUNT);
Pref.declare("tutorial.diaplayTutorial", "", Pref.ACCOUNT_LOCAL);

Pref.declare("gameplay.mousemovement", false, Pref.ACCOUNT);
Pref.declare("gameplay.eqcomparisons", true, Pref.ACCOUNT);

Pref.declare("debug.LoadScreen", false, Pref.SYSTEM_LOCAL);

Pref.declare("quest.QuestMarkerType", {
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
}, Pref.CHARACTER);
Pref.declare("quest.CurrentSelectedQuest", -1, Pref.CHARACTER);

Pref.declare("screens.Positions", "", Pref.ACCOUNT_LOCAL);
