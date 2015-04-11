this.require("GUI/GUI");
class this.GUI.Font 
{
	constructor( _face, _height, ... )
	{
		this.face = _face;
		this.height = _height;

		if (vargc > 0)
		{
			this.bold = vargv[0];
		}

		if (vargc > 1)
		{
			this.italic = vargv[1];
		}

		if (this.height <= 16)
		{
			this.baseHeight = 8;
		}
		else if (this.height <= 32)
		{
			this.baseHeight = 16;
		}
		else if (this.height <= 64)
		{
			this.baseHeight = 32;
		}
		else
		{
			this.baseHeight = 32;
		}

		this.ascent = (this.height * this.heightScaler).tointeger();
		this.descent = this.height - this.ascent;

		if (this.descent == 0)
		{
			this.ascent -= 1;
			this.descent += 1;
		}
	}

	function derive( _height, ... )
	{
		return this.GUI.Font(this.face, _height, vargc > 0 ? vargv[0] : this.bold, vargc > 1 ? vargv[1] : this.italic);
	}

	function getTextMetrics( text )
	{
		local m = this.Screen.getTextMetrics(text, this.getFullFace(), this.height);
		m.ascent <- this.ascent;
		m.descent <- this.descent;
		return m;
	}

	function getPositionOffset( height )
	{
		local offset = height * ((1 - this.heightScaler) * -1);
		return offset.tointeger();
	}

	function getFullFace()
	{
		local faceName = this.face + "_" + this.baseHeight;

		if (this.bold)
		{
			faceName += "_bold";
		}

		if (this.italic)
		{
			faceName += "_italic";
		}

		return faceName;
	}

	function getFace()
	{
		return this.face;
	}

	function getHeight()
	{
		return this.height;
	}

	function getAscent()
	{
		return this.ascent;
	}

	function getDescent()
	{
		return this.descent;
	}

	function isBold()
	{
		return this.bold;
	}

	function isItalic()
	{
		return this.italic;
	}

	function _tostring()
	{
		return "Font[\"" + this.getFullFace() + "\", " + this.height + "]";
	}

	heightScaler = 0.89999998;
	face = null;
	baseHeight = 8;
	height = 16;
	ascent = 14;
	descent = 2;
	bold = false;
	italic = false;
}

this.GUI.DefaultFont = this.GUI.Font("Maiandra", 16);
