require("UI/Screens");

MINIMAP_SMALL <- 0;
MINIMAP_LARGE <- 1;
MINIMAP_OFF <- 2;

class Screens.MiniMapScreen extends GUI.Component
{
	static mClassName = "MiniMapScreen";
	
	mMode = MINIMAP_SMALL;
	mMMZoom = 2500.0;
	mMMap = null;
	mToolTipComp = null;
	mToolTipPosition = null;
	mToolTipLastPosition = null;
	mTooltipTimer = null;
	mZoomInButton = null;
	mZoomOutButton = null;
	mPVPModeButton = null;
	mPVEModeButton = null;
	mCheckDistanceEvent = null;
	mGroveButton = null;
	mPVPRadioGroup = null;
	
	static TOOLTIP_SIZE = 10;
	static LOCATION_HEIGHT = 19;
	
	constructor() {
		GUI.Component.constructor(GUI.FlowLayout());
		
		local font = GUI.Font("Maiandra", 24, true);
		
		mToolTipPosition = {
			x = 0,
			y = 0
		};
		mToolTipLastPosition = {
			x = 0,
			y = 0
		};
		
		mMMap = GUI.MiniMap();
		mMMap.addActionListener(this);
		mMMap.setPreferredSize(175, 175);
		mMMap.setSize(175, 175);
		mMMap.setPosition(0, 0);
		add(mMMap);
		
		local zhack = GUI.ZOffset(1, null);
		zhack.setPosition(0, 0);
		zhack.setSize(175, 175);
		add(zhack);
		
		local bigMapContainer = GUI.Container(null);
		bigMapContainer.setAppearance("MiniMap/MapButtonHolder");
		bigMapContainer.setSize(35, 35);
		
		zhack.setPosition(0, 10);
		zhack.add(bigMapContainer);
		
		local bigMapButton = GUI.ImageButton();
		bigMapButton.setAppearance("MiniMap/MapButton");
		bigMapButton.setGlowImageName("MiniMap/MapButton/Glow");
		bigMapButton.setPressMessage("onMapToggle");
		bigMapButton.addActionListener(this);
		bigMapButton.setSize(35, 35);
		bigMapButton.setTooltip("World Map");
		bigMapContainer.add(bigMapButton);
		
		mZoomInButton = GUI.ImageButton();
		mZoomInButton.setAppearance("MiniMapPlus");
		mZoomInButton.setGlowEnabled(false);
		mZoomInButton.setPressMessage("onZoomIn");
		mZoomInButton.addActionListener(this);
		mZoomInButton.setPosition(133, 13);
		mZoomInButton.setSize(22, 22);
		mZoomInButton.setTooltip("Zoom In");
		zhack.add(mZoomInButton);
		
		mZoomOutButton = GUI.ImageButton();
		mZoomOutButton.setAppearance("MiniMapMinus");
		mZoomOutButton.setGlowEnabled(false);
		mZoomOutButton.setPressMessage("onZoomOut");
		mZoomOutButton.addActionListener(this);
		mZoomOutButton.setPosition(147, 13 + 20);
		mZoomOutButton.setSize(22, 22);
		mZoomOutButton.setTooltip("Zoom Out");
		zhack.add(mZoomOutButton);
		
		mPVPRadioGroup = this.GUI.RadioGroup();
		
		mPVPModeButton = GUI.ImageButton();
		mPVPModeButton.setRadioGroup(mPVPRadioGroup);
		mPVPModeButton.setAppearance("MiniMapPVP");
		mPVPModeButton.setGlowImageName("MiniMapPVPSelected");
		mPVPModeButton.setPressMessage("onPVPPressed");
		mPVPModeButton.addActionListener(this);
		mPVPModeButton.setPosition(133, 118);
		mPVPModeButton.setSize(22, 22);
		mPVPModeButton.setTooltip("Select PVP mode, you may attack players who are also in PVP mode");
		zhack.add(mPVPModeButton);
		
		mPVEModeButton = GUI.ImageButton();
		mPVEModeButton.setRadioGroup(mPVPRadioGroup);
		mPVEModeButton.setAppearance("MiniMapPVE");
		mPVEModeButton.setGlowImageName("MiniMapPVESelected");
		mPVEModeButton.setPressMessage("onPVEPressed");
		mPVEModeButton.addActionListener(this);
		mPVEModeButton.setPosition(147, 96);
		mPVEModeButton.setSize(22, 22);
		mPVEModeButton.setTooltip("Select PVE mode, you will not be attacked by players");
		zhack.add(mPVEModeButton);
		
		mGroveButton = GUI.ImageButton();
		mGroveButton.setAppearance("MiniMapGrove");
		mGroveButton.setGlowEnabled(false);
		mGroveButton.setPressMessage("onGrove");
		mGroveButton.addActionListener(this);
		mGroveButton.setPosition(0, 96);
		mGroveButton.setSize(22, 22);
		mGroveButton.setTooltip("Go to your grove. Must be at a sanctuary");
		zhack.add(mGroveButton);
		
		mToolTipComp = GUI.Container(null);
		mToolTipComp.setSize(TOOLTIP_SIZE, TOOLTIP_SIZE);
		mToolTipComp.setPreferredSize(TOOLTIP_SIZE, TOOLTIP_SIZE);
		mToolTipComp.setPosition(0, 0);
		zhack.add(mToolTipComp);
		
		local tbut = GUI.Button("size");
		tbut.setReleaseMessage("onSize");
		tbut.addActionListener(this);
		tbut = GUI.Button("close");
		tbut.setReleaseMessage("onClose");
		tbut.addActionListener(this);
		
		setSticky("right", "top");
		setPosition(-getWidth() - 4, 4 + LOCATION_HEIGHT);
		::Screen.setOverlayVisible("GUI/MiniMap", true);
		setOverlay("GUI/MiniMap");
		setMode(MINIMAP_SMALL);
		
		::_Connection.addListener(this);
		
		if(::_avatar) {
			setPVP(::_avatar.hasStatusEffect(StatusEffects.PVPABLE));
		}
	}
	
	function onStatusUpdate( creature )	{
		if(!::_avatar || ::_avatar.getID() != creature.getID()) 
			return;
	
		setPVP(creature.hasStatusEffect(StatusEffects.PVPABLE));
	}
	
	function isPVP() {
		return mPVPRadioGroup.getSelected() == mPVPModeButton;
	}
	
	function setPVP(pvp) {
		mPVPRadioGroup.setSelected(pvp ? mPVPModeButton : mPVEModeButton);
	}

	function toggleMode() {
		if (mMode == MINIMAP_OFF)
			mMode = MINIMAP_SMALL;
		else
			mMode = MINIMAP_OFF;

		setMode(mMode);
	}

	function setMode( mode ) {
		switch(mode) {
		case MINIMAP_SMALL:
			setPreferredSize(175, 175);
			setSize(175, 175);
			setVisible(true);
			break;

		case MINIMAP_LARGE:
			setPreferredSize(256, 256);
			setSize(256, 256);
			setVisible(true);
			break;

		case MINIMAP_OFF:
			setVisible(false);
			break;
		}

		log.debug(getPreferredSize().width + ", " + getPreferredSize().height);
		setPosition(-getWidth() - 4, 4 + LOCATION_HEIGHT);
		local miniMapLocation = Screens.get("MiniMapLocation", false);

		if (miniMapLocation) {
			miniMapLocation.updatePosition(getWidth());
		}

		mMode = mode;
	}
	
	
	function sanctuaryDistanceCheck() {
		if (!isVisible())
			return;

		/*
		local vault = ::_sceneObjectManager.getCreatureByID(mCurrentVaultId);

		if (vault) {
			if (Math.manhattanDistanceXZ(::_avatar.getPosition(), vault.getPosition()) > Util.getRangeOffset(_avatar, vault) + MAX_USE_DISTANCE)
			{
				IGIS.error("You are too far away from the vault to continue using it.");
				setVisible(false);
				mCurrentVaultId = -1;
			}
			else {
				mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "vaultDistanceCheck");
			}
		}
		else {
			IGIS.error("This vault no longer exists.");
			setVisible(false);
			mCurrentVaultId = -1;
		}
		*/
	}

	function setVisible( visible ) {
		local miniMapLocation = Screens.get("MiniMapLocation", true);
		miniMapLocation.setVisible(visible);

		if (visible) {
			Screens.show("MiniMapLocation");

			if (!mCheckDistanceEvent)
				mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "sanctuaryDistanceCheck");

			mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "sanctuaryDistanceCheck");
		}
		else {
			if (mCheckDistanceEvent) {
				::_eventScheduler.cancel(mCheckDistanceEvent);
				mCheckDistanceEvent = null;
			}
		}

		GUI.Component.setVisible(visible);
	}

	function onMapToggle( button ) {
		Screens.toggle("MapWindow");
	}

	function onGrove( pButton ) {
		::_Connection.sendQuery("grove", this, []);
	}

	function onPVPPressed( pButton ) {
		::_Connection.sendQuery("mode", this, [
			"pvp"
		]);
	}
	
	function onPVEPressed( pButton ) {
		::_Connection.sendQuery("mode", this, [
			"pve"
		]);
	}

	function onZoomIn( pButton ) {
		local newScale = mMMap.mScale / 2;
		mMMap.setScale(newScale);
		::Pref.set("minimap.ZoomScale", newScale, true);

		if (newScale <= GUI.MiniMap.MIN_SCALE)
		{
			mZoomInButton.setEnabled(false);
		}

		mZoomOutButton.setEnabled(true);
	}

	function onZoomOut( pButton ) {
		local newScale = mMMap.mScale * 2;
		mMMap.setScale(newScale);
		::Pref.set("minimap.ZoomScale", newScale, true);

		if (newScale >= GUI.MiniMap.MAX_SCALE)
		{
			mZoomOutButton.setEnabled(false);
		}

		mZoomInButton.setEnabled(true);
	}

	function setZoomScale( scaleSize ) {
		mMMap.setScale(scaleSize);
		mZoomOutButton.setEnabled(true);
		mZoomInButton.setEnabled(true);

		if (newScale <= GUI.MiniMap.MIN_SCALE)
			mZoomInButton.setEnabled(false);

		if (newScale >= GUI.MiniMap.MAX_SCALE)
			mZoomOutButton.setEnabled(false);
	}

	function onClose( pButton )	{
		setMode(MINIMAP_OFF);
	}

	function onSize( pButton ) {
		setMode(mMode == MINIMAP_SMALL ? MINIMAP_LARGE : MINIMAP_SMALL);
	}

	function onBuildMode( button, state ) {
	}

	function onMinimapClick( pMinimap, x, z ) {
		if (Util.isDevMode())
		{
			print("*** clicked " + x + ", " + z);
			::_Connection.sendQuery("go", this, [
				"" + x,
				"-1.0",
				"" + z
			]);
			print("*** welcome to warp zone!");
		}
	}

	function onRowSelectionChanged( sender, index, bool ) {
		local selection = sender.getRow(index);

		if (!selection)
			return;

		if (!_avatar)
			return;

		if (!mInitialAvatarType) {
			local iniType = _avatar.getType();

			if (!iniType)
				return;
			else
				mInitialAvatarType = iniType;
		}

		local pType = selection[0];
		log.debug("Selection: " + selection[0]);

		if (pType == "-None-")
			_swapAvatar(pType);
		else
			_loadCreatrureArchives(selection[0], true);
	}

	function onRunAnimation( evt ) {
		command("/emote " + mAnimationDropDown.getCurrent());
	}

	function command( pString )	{
		::EvalCommand(pString);
	}

	function _addNotify() {
		GUI.Component._addNotify();

		if (!mWidget)
			mWidget.addListener(this);
	}

	function _removeNotify() {
		if (mWidget != null)
			mWidget.removeListener(this);

		GUI.Component._removeNotify();
		::_Connection.removeListener(this);
	}

	function onMouseMoved( evt ) {
		mToolTipPosition = {
			x = evt.x,
			y = evt.y
		};
	}

	function onMouseEnter( evt ) {
		mTooltipTimer = ::Timer();
		_exitFrameRelay.addListener(this);
	}

	function onMouseExit( evt )	{
		mTooltipTimer = null;
		_exitFrameRelay.removeListener(this);
	}

	function onExitFrame() {
		if (mTooltipTimer && mTooltipTimer.getMilliseconds() > 490 && (mToolTipLastPosition.x != mToolTipPosition.x || mToolTipLastPosition.y != mToolTipPosition.y)) {
			local VIEW_SIZE = 256.0;
			local MAP_SIZE = 175.0;
			local viewScale = VIEW_SIZE / MAP_SIZE;
			local x = mToolTipPosition.x.tofloat() * viewScale;
			local y = mToolTipPosition.y.tofloat() * viewScale;
			local result = _root.getMarkerToolTip(x, y);

			if (result.name.find("Creature/") != null) {
				local creatureId = ::Util.replace(result.name, "Creature/", "");
				creatureId = creatureId.tointeger();

				if (::_sceneObjectManager.hasCreature(creatureId))
				{
					local creature = ::_sceneObjectManager.getCreatureByID(creatureId);
					mToolTipComp.setTooltip(creature.getName());
				}
			}
			else
				mToolTipComp.setTooltip(result.name);

			if (result.name != "")
				mToolTipComp.setPosition(mToolTipPosition.x - TOOLTIP_SIZE / 2, mToolTipPosition.y - TOOLTIP_SIZE / 2 - 10);

			mToolTipLastPosition = deepClone(mToolTipPosition);
			mTooltipTimer = null;
			mTooltipTimer = ::Timer();
			mTooltipTimer.reset();
		}
	}

	function onMousePressed( evt ) {
		if (evt.clickCount != 1)
			return;

		if (evt.button == MouseEvent.RBUTTON)
			evt.consume();

		if (evt.button == MouseEvent.LBUTTON) {
			local viewScale = 256.0 / 175.0;
			local x = evt.x.tofloat() * viewScale;
			local y = evt.y.tofloat() * viewScale;
			local clickpos = _root.getMiniMapWorldCoordinate(x, y);
			onMinimapClick(this, clickpos.x, clickpos.z);
			evt.consume();
		}
	}

}

