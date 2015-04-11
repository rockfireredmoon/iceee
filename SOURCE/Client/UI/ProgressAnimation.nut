this.require("GUI/Component");
class this.GUI.ProgressAnimation extends this.GUI.Component
{
	constructor()
	{
		this.GUI.Component.constructor(null);
		this.setAppearance("Container");
		this.setMaterial("ProgressAnimation");
		this.setPreferredSize(32, 32);
		this.setSize(32, 32);
		this.setVisible(true);
	}

}

