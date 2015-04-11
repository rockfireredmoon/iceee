this.require("GUI/Component");
class this.GUI.Spacer extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.mAppearance = null;
		this.mSpacerWidth = vargc > 0 ? vargv[0] : 5;
		this.mSpacerHeight = vargc > 1 ? vargv[1] : 5;
	}

	function getPreferredSize()
	{
		return {
			width = this.mSpacerWidth,
			height = this.mSpacerHeight
		};
	}

	mSpacerWidth = 0;
	mSpacerHeight = 0;
	static mClassName = "Spacer";
}

