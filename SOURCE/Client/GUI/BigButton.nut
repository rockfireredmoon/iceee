this.require("GUI/Button");
class this.GUI.BigButton extends this.GUI.Button
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
		this.setAppearance("BigButton");
		this.setFixedSize(220, 49);
		this.setFont(font);
		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = true;
		this.setSelection(true);
	}

	static mClassName = "BigButton";
}

class this.GUI.LtGreenBigButton extends this.GUI.BigButton
{
	constructor( pLabel, ... )
	{
		if (vargc > 0)
		{
			this.GUI.BigButton.constructor(pLabel, vargv[0]);
		}
		else
		{
			this.GUI.BigButton.constructor(pLabel);
		}

		local font = ::GUI.Font("Maiandra", 32);
		this.setFont(font);
		this.setFontColor(this.Colors.white);
		this.setAppearance("LtGreenBigButton");
		this.setSelection(true);
	}

	static mClassName = "LtGreenBigButton";
}

