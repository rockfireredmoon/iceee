require("GUI/Button");

class GUI.BigButton extends GUI.Button
{
	constructor( pLabel, ... )
	{
		if (vargc > 0)
			GUI.Button.constructor(pLabel, vargv[0]);
		else
			GUI.Button.constructor(pLabel);

		local font = ::GUI.Font("MaiandraOutline", 24);
		setAppearance("BigButton");
		setFixedSize(220, 49);
		setFont(font);
		
		mUseMouseOverEffect = true;
		mUseOffsetEffect = true;
		setSelection(true);
	}

	static mClassName = "BigButton";
}

class GUI.LtGreenBigButton extends GUI.BigButton
{
	constructor( pLabel, ... )
	{
		if (vargc > 0)
			GUI.BigButton.constructor(pLabel, vargv[0]);
		else
			GUI.BigButton.constructor(pLabel);

		local font = ::GUI.Font("Maiandra", 32);
		setFont(font);
		setFontColor(this.Colors.white);
		setAppearance("LtGreenBigButton");
		setSelection(true);
	}

	static mClassName = "LtGreenBigButton";
}

