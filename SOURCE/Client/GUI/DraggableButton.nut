this.require("GUI/Button");
class this.GUI.DraggableButton extends this.GUI.Button
{
	static mClassName = "DraggableButton";
	constructor( label )
	{
		this.GUI.Button.constructor(label);
	}

	function onDragRequested( evt )
	{
		evt.acceptDrag(this, this.GUI.DnDEvent.ACTION_MOVE);
	}

}

