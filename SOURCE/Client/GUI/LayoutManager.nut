this.require("GUI/GUI");
class this.GUI.LayoutManager 
{
	function layoutContainer( container )
	{
	}

	function preferredLayoutSize( container )
	{
		return {
			width = 0,
			height = 0
		};
	}

	function minimumLayoutSize( container )
	{
		return {
			width = 0,
			height = 0
		};
	}

	function getConstraint( container, component )
	{
		local c = container.mLayoutConstraints;

		if (c && component.mName in c)
		{
			return c[component.mName];
		}

		return null;
	}

	function _tostring()
	{
		return this.mClassName;
	}

	static mClassName = "LayoutManager";
}

