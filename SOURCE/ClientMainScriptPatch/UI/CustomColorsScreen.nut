require("UI/Screens");
local wasVisible = Screens.close("CustomColorsScreen");

/**
	A simple frame that holds an array of color swatches, serializing
	them between sessions. This helps artists keep a custom palette of
	often used colors and makes coordinating various appearances easier.
*/
class Screens.CustomColorsScreen extends GUI.Frame
{
	static mClassName = "Screens.CustomColorsScreen";
	static mCookieName = "CustomColorsScreen";
	mRows = 3;
	mCols = 5;
	mColorPickers = null;
	constructor()
	{
		GUI.Frame.constructor("Custom Colors");
		local container = GUI.Container(GUI.GridLayout(mRows, mCols));
		container.setInsets(3);
		mColorPickers = [];

		for( local r = 0; r < mRows; r++ )
		{
			for( local c = 0; c < mCols; c++ )
			{
				local name = "Custom_" + r + "_" + c;
				local p = GUI.ColorPicker(name, name, ColorRestrictionType.DEVELOPMENT);
				p.setChangeMessage("_onPickerChanged");
				p.addActionListener(this);
				mColorPickers.append(p);
				container.add(p);
			}
		}

		setContentPane(container);
		setSize(getPreferredSize());
		_loadState();
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
	}

	function _saveState() {
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
	}
}


if (wasVisible)
	Screens.toggle("CustomColorsScreen");