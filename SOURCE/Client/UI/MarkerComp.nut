class this.GUI.MarkerComp extends this.GUI.Component
{
	mPressMessage = null;
	mToolTipLabel = null;
	mCoordLabel = null;
	static LOCAL_ICON_SIZE = 14.0;
	constructor( markerObj, mapImage )
	{
		this.GUI.Component.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		local iconSize = this.LOCAL_ICON_SIZE;
		this.setSize(iconSize, iconSize);
		this.setPreferredSize(iconSize, iconSize);
		local scaledPosition = mapImage.scaleXZPosition(markerObj.position);

		if (scaledPosition)
		{
			this.setPosition(scaledPosition.x - iconSize / 2.0, scaledPosition.z - iconSize / 2.0);
		}

		this.setAppearance(::LegendItems[markerObj.iconType].iconName);
		this.setData(markerObj);
		local tooltipComp = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mToolTipLabel = this.GUI.Label(markerObj.name);
		tooltipComp.add(this.mToolTipLabel);
		this.mCoordLabel = this.GUI.Label("x:" + this.Util.trim(markerObj.position.x.tostring()) + " z: " + markerObj.position.z);
		tooltipComp.add(this.mCoordLabel);

		if (!this.Util.isDevMode())
		{
			this.mCoordLabel.setVisible(false);
		}

		this.setTooltip(tooltipComp);
	}

	function setTooltipText( title, coordText )
	{
		this.mToolTipLabel.setText(title);

		if (this.Util.isDevMode())
		{
			this.mCoordLabel.setText(coordText);
			this.mCoordLabel.setVisible(true);
		}
		else
		{
			this.mCoordLabel.setVisible(false);
		}
	}

	function addActionListener( vListener )
	{
		this.mMessageBroadcaster.addListener(vListener);
	}

	function setPressMessage( vMessage )
	{
		this.mPressMessage = vMessage;
	}

	function _fireActionPerformed( vMessage )
	{
		if (vMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(vMessage, this);
		}
	}

	function onMousePressed( vEvent )
	{
		if (vEvent.button == this.MouseEvent.LBUTTON && this.Key.isDown(this.Key.VK_CONTROL))
		{
			if (this.mPressMessage)
			{
				this._fireActionPerformed(this.mPressMessage);
			}

			vEvent.consume();
		}
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();

		if (this.mWidget != null)
		{
			this.mWidget.addListener(this);
		}
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

}

class this.GUI.InvisibleMarkerComp extends this.GUI.MarkerComp
{
	static LOCAL_ICON_SIZE = 14.0;
	constructor( invisMarker )
	{
		this.GUI.Container.constructor(null);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		local iconSize = this.LOCAL_ICON_SIZE;
		this.setSize(iconSize, iconSize);
		this.setPreferredSize(iconSize, iconSize);
		this.setPosition(invisMarker.x - iconSize / 2.0, invisMarker.y - iconSize / 2.0);
		this.setPassThru(true);
		this.generateTooltipWithPassThru(true);
		local tooltipComp = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mToolTipLabel = this.GUI.Label(invisMarker.name);
		tooltipComp.add(this.mToolTipLabel);
		this.mCoordLabel = this.GUI.Label("x:" + this.Util.trim(invisMarker.x.tostring()) + " y: " + invisMarker.y.tostring());
		tooltipComp.add(this.mCoordLabel);

		if (!this.Util.isDevMode())
		{
			this.mCoordLabel.setVisible(false);
		}

		this.setTooltip(tooltipComp);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();

		if (this.mWidget != null)
		{
			this.mWidget.addListener(this);
		}
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function onMousePressed( vEvent )
	{
		return;
	}

}

