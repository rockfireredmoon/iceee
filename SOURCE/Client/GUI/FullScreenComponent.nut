this.require("GUI/Container");
class this.GUI.FullScreenComponent extends this.GUI.Container
{
	constructor( layoutManager )
	{
		this.GUI.Component.constructor(layoutManager);
		this.setPosition(0, 0);
		this.onScreenResize();
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this._root.addListener(this);
	}

	function _removeNotify()
	{
		this._root.removeListener(this);
		this.GUI.Component._removeNotify();
	}

	function onScreenResize()
	{
		this.setPosition(0, 0);
		this.setSize(::Screen.getWidth(), ::Screen.getHeight());
	}

}

