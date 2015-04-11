this.require("GUI/Container");
class this.GUI.LabelContainer extends this.GUI.Container
{
	mClassName = "LabelContainer";
	mLabel = null;
	constructor( label, component )
	{
		this.GUI.Container.constructor(this.GUI.BorderLayout());
		this.setInsets(3);
		this.mLabel = this.GUI.Label(label);
		this.mLabel.insets.right += 4;
		this.add(this.mLabel, this.GUI.BorderLayout.WEST);
		this.add(component, this.GUI.BorderLayout.CENTER);
	}

}

