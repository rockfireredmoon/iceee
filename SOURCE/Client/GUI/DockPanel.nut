this.require("GUI/Panel");
this.require("GUI/DnDEvent");
this.require("GUI/FlowLayout");
class this.GUI.DockPanel extends this.GUI.Panel
{
	static mClassName = "DockPanel";
	constructor()
	{
		this.GUI.Panel.constructor(this.GUI.FlowLayout());
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Panel._removeNotify();
	}

	function onDragEnter( evt )
	{
	}

	function onDragExit( evt )
	{
	}

	function _getDroppedComponent( evt )
	{
		local t = evt.getTransferable();

		if (typeof t != "instance" || !(t instanceof this.GUI.Component))
		{
			return null;
		}

		return t;
	}

	function onDragOver( evt )
	{
		if (!this._getDroppedComponent(evt))
		{
			return;
		}

		evt.acceptDrop(this.GUI.DnDEvent.ACTION_MOVE);
	}

	function onDrop( evt )
	{
		local c = this._getDroppedComponent(evt);
		this.add(c);
	}

}

