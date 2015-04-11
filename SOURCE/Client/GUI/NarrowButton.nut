this.require("GUI/Button");
class this.GUI.NarrowButton extends this.GUI.Button
{
	constructor( pLabel, ... )
	{
		if (vargc > 1)
		{
			this.GUI.Button.constructor(pLabel, vargv[0], vargv[1]);
		}
		else if (vargc > 0)
		{
			this.GUI.Button.constructor(pLabel, vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor(pLabel);
		}

		local font = ::GUI.Font("Maiandra", 22);
		this.setAppearance("NarrowButton");
		this.setFontColor(this.Colors.white);
		this.setFixedSize(100, 32);
		this.setFont(font);
		this.insets.bottom += 8;
		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = true;
		this.setSelection(true);
	}

	static mClassName = "NarrowButton";
}

class this.GUI.BlueNarrowButton extends this.GUI.Button
{
	constructor( pLabel, ... )
	{
		if (vargc > 1)
		{
			this.GUI.NarrowButton.constructor(pLabel, vargv[0], vargv[1]);
		}
		else if (vargc > 0)
		{
			this.GUI.NarrowButton.constructor(pLabel, vargv[0]);
		}
		else
		{
			this.GUI.NarrowButton.constructor(pLabel);
		}

		this.setAppearance("BlueNarrowButton");
		this.setSelection(true);
	}

	static mClassName = "BlueNarrowButton";
}

class this.GUI.RedNarrowButton extends this.GUI.Button
{
	constructor( pLabel, ... )
	{
		if (vargc > 1)
		{
			this.GUI.NarrowButton.constructor(pLabel, vargv[0], vargv[1]);
		}
		else if (vargc > 0)
		{
			this.GUI.NarrowButton.constructor(pLabel, vargv[0]);
		}
		else
		{
			this.GUI.NarrowButton.constructor(pLabel);
		}

		this.setAppearance("RedNarrowButton");
		this.setSelection(true);
	}

	static mClassName = "RedNarrowButton";
}

