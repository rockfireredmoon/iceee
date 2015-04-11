this.require("GUI/Component");
this.require("GUI/Label");
class this.GUI.ContainedLabel extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		local text = "";

		if (vargc > 0 && typeof vargv[0] == "string")
		{
			text = vargv[0];
		}

		this.mLabel = this.GUI.Label(text);
		this.add(this.mLabel);
	}

	function setText( string )
	{
		this.mLabel.setText(string);
	}

	function getText()
	{
		return this.mLabel.getText();
	}

	function setFont( font )
	{
		this.mLabel.setFont(font);
	}

	function setFontColor( fontColor )
	{
		this.mLabel.setFontColor(fontColor);
	}

	function setTextAlignment( horizontal, vertical )
	{
		this.mLabel.setTextAlignment(horizontal, vertical);
	}

	function getTextAlignment()
	{
		return this.mLabel.getTextAlignment();
	}

	function getPreferredSize()
	{
		return this.mLabel.getPreferredSize();
	}

	function getWidth()
	{
		return this.mLabel.getWidth();
	}

	function getHeight()
	{
		return this.mLabel.getHeight();
	}

	mLabel = null;
}

