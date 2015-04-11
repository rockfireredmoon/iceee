this.require("GUI/Component");
this.require("GUI/Label");
this.require("GUI/Image");
class this.GUI.CheckBoxLabelImage extends this.GUI.Component
{
	mCheckBox = null;
	mLabel = null;
	mIcon = null;
	static mClassName = "CheckBoxLabelImage";
	static GAP = 10;
	static mCheckBoxLabelSize = {
		width = 100,
		height = 24
	};
	constructor( labelName, imageName, ... )
	{
		if (vargc > 0)
		{
			this.mCheckBoxLabelSize.height = vargv[0];
		}

		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.setInsets(0, 0, 0, 5);
		this.setSize(this.mCheckBoxLabelSize.width, this.mCheckBoxLabelSize.height);
		this.setPreferredSize(this.mCheckBoxLabelSize.width, this.mCheckBoxLabelSize.height);
		this.getLayoutManager().setGap(5);
		local checkBoxSize = 16;
		this.mCheckBox = this.GUI.CheckBox();
		this.mCheckBox.setAppearance("CheckBoxSmall");
		this.mCheckBox.setFixedSize(checkBoxSize, checkBoxSize);
		local labelContainer = this.GUI.Container(this.GUI.BoxLayout());
		labelContainer.setPreferredSize(100, this.mCheckBoxLabelSize.height);
		labelContainer.setSize(100, this.mCheckBoxLabelSize.height);
		this.mLabel = ::GUI.Label(labelName);
		this.mLabel.setSize(100, this.mCheckBoxLabelSize.height);
		this.mLabel.setPreferredSize(100, this.mCheckBoxLabelSize.height);
		this.mLabel.setTextAlignment(0.0, 0.5);
		labelContainer.add(this.mLabel);
		local iconSize = 14;
		this.mIcon = this.GUI.Component(null);
		this.mIcon.setAppearance(imageName);
		this.mIcon.setSize(iconSize, iconSize);
		this.mIcon.setPreferredSize(iconSize, iconSize);
		this.add(this.mIcon);
		this.add(labelContainer);
		this.add(this.mCheckBox);
	}

	function getCheckBox()
	{
		return this.mCheckBox;
	}

	function setCheckBoxData( data )
	{
		this.mCheckBox.setData(data);
	}

	function setReleaseMessage( callbackFunc, callbackObj )
	{
		this.mCheckBox.setReleaseMessage(callbackFunc);
		this.mCheckBox.addActionListener(callbackObj);
	}

}

