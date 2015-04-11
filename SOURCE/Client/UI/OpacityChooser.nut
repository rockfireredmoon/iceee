this.require("GUI/Slider");
class this.GUI.OpacityChooser extends this.GUI.Container
{
	mMainWindow = null;
	mSlider = null;
	mLabel = null;
	mBroadcaster = null;
	constructor()
	{
		this.GUI.Container.constructor();
		this.mMainWindow = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mMainWindow.setSize(150, 100);
		this.mMainWindow.setInsets(0, 0, 0, 8);
		this.mSlider = this.GUI.Slider(this.gHORIZONTAL_SLIDER, 0, 100, 100);
		this.mSlider.addChangeListener(this);
		this.mSlider.setPosition(0, 20);
		this.mLabel = this.GUI.Label();
		this.mMainWindow.add(this.mSlider);
		this.mMainWindow.add(this.mLabel);
		this.add(this.mMainWindow);
		this.mBroadcaster = this.MessageBroadcaster();
	}

	function addActionListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function onSliderUpdated( slider )
	{
		if (slider == this.mSlider)
		{
			local newValue = this.mSlider.getValue();
			this.mBroadcaster.broadcastMessage("onOpacityUpdate", newValue);
			this.mLabel.setText(newValue.tointeger() + "%");
		}
	}

	function setOpacityValue( opacity )
	{
		this.mSlider.setValue(opacity);
		this.mLabel.setText(opacity.tointeger() + "%");
	}

}

