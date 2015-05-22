this.require("GUI/Button");
class this.GUI.AbilityButton extends this.GUI.Button
{
	constructor( pLabel, ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor(pLabel, vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor(pLabel);
		}

		local font = ::GUI.Font("MaiandraOutline", 24);
		this.setAppearance("AbilityButton");
		this.setFixedSize(220, 49);
		this.setFont(font);
		this.insets.right += 8;
		this.insets.bottom += 8;
		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = true;
		this.setSelection(true);
	}

	static mClassName = "AbilityButton";
}

