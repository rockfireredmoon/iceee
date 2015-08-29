require("UI/Screens");

local wasVisible = Screens.close("SpawnMasterScreen");

/**
	A simple frame that allows GMs to quickly spawn pre-defined mobs for
	events.
*/

class SpawnMasterRow extends GUI.Container {

	spawnId = null;
	qty = null;
	spawnType = null;
	data = null;
	attack = null;
	flags = null;
	ability = null;

	constructor() {
		GUI.Container.constructor(GUI.BoxLayout());
		
		spawnId  = GUI.InputArea();
		spawnId.setSize(60, 20);
		spawnId.setPreferredSize(60, 20);
		spawnId.setMaxCharacters(8);
		spawnId.setTooltip("Creature Def ID# of the creature to spawn");			
		add(spawnId);	
		
		qty  = GUI.InputArea();
		qty.setSize(40, 20);
		qty.setText("1");
		qty.setPreferredSize(40, 20);
		qty.setMaxCharacters(2);
		qty.setTooltip("How many creatures to spawn");
		add(qty);
		
		spawnType = GUI.DropDownList();
		spawnType.addChoice("RANDOM");
		spawnType.addChoice("AT_PROP");
		spawnType.setSize(60, 20);
		spawnType.setPreferredSize(60, 20);
		spawnType.setTooltip("Where to spawn the creature");
		add(spawnType);
				
		data  = GUI.InputArea();
		data.setSize(60, 20);
		data.setPreferredSize(60, 20);
		data.setText("100");
		data.setMaxCharacters(64);
		data.setTooltip("For RANDOM, the radius from the selection (or the spawner). For PROP, the prop ID");
		add(data);
		
		flags = GUI.DropDownList();
		flags.addChoice("FRIEND");
		flags.addChoice("NEUTRAL");
		flags.addChoice("ENEMY");
		flags.addChoice("FATTACK");
		flags.setSize(60, 20);
		flags.setPreferredSize(60, 20);
		flags.setTooltip("Flags for the spawn");
		add(flags);
		
		attack = GUI.DropDownList();
		attack.addChoice("SELECTED");
		attack.addChoice("RANDPLR");
		attack.addChoice("RANDNPC");
		attack.addChoice("NONE");
		attack.setTooltip("Whether the spawn should attack the selected spawn/player, or a random creature in the instance");
		attack.setSize(60, 20);
		attack.setPreferredSize(60, 20);
		add(attack);
				
		ability  = GUI.InputArea();
		ability.setSize(60, 20);
		ability.setPreferredSize(60, 20);
		ability.setText("32766");
		ability.setMaxCharacters(6);
		ability.setTooltip("Opening ability ID when attacking on spawn (32766 is default 'melee' ability)");
		add(ability);
	
		local spawnButton = GUI.Button("Spawn");
		spawnButton.addActionListener(this);
		spawnButton.setReleaseMessage("_onSpawnPressed");
		add(spawnButton);
	}	
	
	function _onSpawnPressed(evt) {
		local f = 0;
		if(flags.getCurrentIndex() == 0) {
			f = 1;
		}
		if(flags.getCurrentIndex() == 1) {
			f = 4;
		}
		if(flags.getCurrentIndex() == 2) {
			f = 32;
		}
		if(flags.getCurrentIndex() == 3) {
			f = 16;
		}
	
		::_Connection.sendQuery("gm.spawn", this, [
			spawnId.getText().tointeger(),
			qty.getText().tointeger(),
			spawnType.getCurrentIndex(),
			data.getText(),
			f,
			attack.getCurrentIndex(),			
			ability.getText().tointeger()
		]);
	}
}


class Screens.SpawnMasterScreen extends GUI.Frame {

	static mClassName = "Screens.SpawnMasterScreen";
	static mCookieName = "SpawnMasterScreen";
	mRows = 10;
	
	//mColorPickers = null;
	
	constructor() {
	
		GUI.Frame.constructor("Spawn Master");
		
		local table = GUI.Container(GUI.BoxLayoutV());
		table.setInsets(3);

		for( local r = 0; r < mRows; r++ )
			table.add(SpawnMasterRow());

		setContentPane(table);
		setSize(getPreferredSize());
		_loadState();
	}
	
	function _onSpawnPressed(button) {
	}

	function _onPickerChanged( picker )	{
		_saveState();
	}

	function _reshapeNotify() {
		GUI.Frame._reshapeNotify();
		_saveState();
	}
	
	/**
		Restore the state of this window from a serialized cookie. This is
		called when the component is first created. More interesting is
		_saveState() which will serialize the interesting state everytime
		something changes.
	*/
	function _loadState() {
		/*
		local state = unserialize(_cache.getCookie(mCookieName));

		if ("colors" in state) {
			local i;
			local n = state.colors.len();
			local max = mRows * mCols;

			if (n > max)
				n = max;

			for( local i = 0; i < n; i++ )
				mColorPickers[i].setColor(Color(state.colors[i]), false);
		}

		if (("pos" in state) && typeof state.pos == "table") {
			setPosition(state.pos);
			keepOnScreen();
		}
		*/
	}

	function _saveState() {
		/*
		local state = {};
		state.pos <- getPosition();
		state.colors <- [];
		local n = mRows * mCols;

		for( local i = 0; i < n; i++ )
			state.colors.append(mColorPickers[i].getCurrent());

		try {
			_cache.setCookie(mCookieName, serialize(state));
		}
		catch(e) {
			log.warn("Failed to set preference. " + e);
		}
		*/
	}
}


if (wasVisible)
	Screens.toggle("SpawnMasterScreen");