this.require("GUI/Panel");
class this.GUI.TabPanel extends this.GUI.Panel
{
	constructor( ... )
	{
		this.GUI.Panel.constructor();
		this.setInsets(0, 0, 0, 0);
		this.setLayoutManager(this.GUI.CardLayout());
	}

	function onTabSwitch( n )
	{
		this.getLayoutManager().show(n, this);
	}

	function addContents( t, contents )
	{
		this.add(contents, t);
	}

	function getTab( t )
	{
		local c;

		foreach( c in this.components )
		{
			if (this.getLayoutManager().getConstraint(this, c) == t)
			{
				return c;
			}
		}

		return null;
	}

	static mClassName = "TabPanel";
}

