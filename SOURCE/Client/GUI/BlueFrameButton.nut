this.require("GUI/Button");
class this.GUI.BlueFrameButton extends this.GUI.Button
{
	constructor( myComponent, ... )
	{
		this.GUI.Container.constructor();
		this.mInitAppearance = "Button";
		this.setInsets(3, 5, 3, 5);
		this.setLayoutManager(this.GUI.BorderLayout());
		this.mIcon = null;

		if (!(myComponent instanceof this.GUI.Component))
		{
			this.mLabel = this.GUI.Label(myComponent);
		}
		else
		{
			this.mLabel = this.GUI.Label("");
		}

		this.mLabel.setTextAlignment(0.5, 0.5);

		if (!(myComponent instanceof this.GUI.Component))
		{
			this.add(this.mLabel, this.GUI.BorderLayout.CENTER);
		}
		else
		{
			this.add(myComponent, this.GUI.BorderLayout.CENTER);
		}

		this.setDisabledFontColor("CCCCCC");
		this.setSize(this.getPreferredSize());
		this.setSelection(true);
		this.mInitFontColor = this.Color(this.GUI.DefaultButtonFontColor);
		this.mReleaseMessage = "onActionPerformed";
		this.mOpenMenuMessage = "onOpenMenu";
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			if (typeof vargv[0] == "table" || typeof vargv[0] == "instance")
			{
				this.addActionListener(vargv[0]);
			}
		}

		if (vargc > 1 && typeof vargv[1] == "string")
		{
			this.mReleaseMessage = vargv[1];
		}

		if (vargc > 2 && typeof vargv[2] == "string")
		{
			this.mOpenMenuMessage = vargv[2];
		}

		this.setAppearance("BlueFrameButton");
		this.setFixedSize(220, 49);
		this.setSelection(false);
	}

	static mClassName = "BlueFrameButton";
}

