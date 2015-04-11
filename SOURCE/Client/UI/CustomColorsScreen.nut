this.require("UI/Screens");
local wasVisible = this.Screens.close("CustomColorsScreen");
class this.Screens.CustomColorsScreen extends this.GUI.Frame
{
	static mClassName = "Screens.CustomColorsScreen";
	static mCookieName = "CustomColorsScreen";
	mRows = 3;
	mCols = 5;
	mColorPickers = null;
	constructor()
	{
		this.GUI.Frame.constructor("Custom Colors");
		local container = this.GUI.Container(this.GUI.GridLayout(this.mRows, this.mCols));
		container.setInsets(3);
		this.mColorPickers = [];

		for( local r = 0; r < this.mRows; r++ )
		{
			for( local c = 0; c < this.mCols; c++ )
			{
				local name = "Custom_" + r + "_" + c;
				local p = this.GUI.ColorPicker(name, name, this.ColorRestrictionType.DEVELOPMENT);
				p.setChangeMessage("_onPickerChanged");
				p.addActionListener(this);
				this.mColorPickers.append(p);
				container.add(p);
			}
		}

		this.setContentPane(container);
		this.setSize(this.getPreferredSize());
		this._loadState();
	}

	function _onPickerChanged( picker )
	{
		this._saveState();
	}

	function _reshapeNotify()
	{
		this.GUI.Frame._reshapeNotify();
		this._saveState();
	}

	function _loadState()
	{
		local state = this.unserialize(this._cache.getCookie(this.mCookieName));

		if ("colors" in state)
		{
			local i;
			local n = state.colors.len();
			local max = this.mRows * this.mCols;

			if (n > max)
			{
				n = max;
			}

			for( local i = 0; i < n; i++ )
			{
				this.mColorPickers[i].setColor(this.Color(state.colors[i]), false);
			}
		}

		if (("pos" in state) && typeof state.pos == "table")
		{
			this.setPosition(state.pos);
			this.keepOnScreen();
		}
	}

	function _saveState()
	{
		local state = {};
		state.pos <- this.getPosition();
		state.colors <- [];
		local n = this.mRows * this.mCols;

		for( local i = 0; i < n; i++ )
		{
			state.colors.append(this.mColorPickers[i].getCurrent());
		}

		this._cache.setCookie(this.mCookieName, this.serialize(state));
	}

}


if (wasVisible)
{
	this.Screens.toggle("CustomColorsScreen");
}
