this.require("GUI/GUI");
this.require("UI/MapDef");
::_minimap <- null;
class this.GUI.MiniMap extends this.GUI.Component
{
	static MIN_SCALE = 128.0;
	static MAX_SCALE = 8192.0;
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.mStickers = [];
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setLayoutManager(this.GUI.FlowLayout());
		this.mAppearance = "MiniMap";
		this._renderSystemEventRelay.addListener(this);
		this._scene.addListener(this);
		::_minimap = this;
		this._root.setMiniMapVisibilityMask(this.VisibilityFlags.PROPS | this.VisibilityFlags.SCENERY | this.VisibilityFlags.LIGHT_GROUP_ANY | this.VisibilityFlags.WATER);
		this._root.setMiniMapStickerTexture("DefaultSkin/sticker-legend.png");
		this._root.setMiniMapBorderTexture("DefaultSkin/MainUI-MiniMapBorder.png");
		this._root.addStickerOnBorderVisible(::LegendItemTypes.QUEST_TRACKER1, ::LegendItemTypes.QUEST_TRACKER2, ::LegendItemTypes.QUEST_TRACKER3, ::LegendItemTypes.QUEST_TRACKER4, ::LegendItemTypes.PARTY);
		local StickerDescTexCoord = {};
		local NUM_STICKER_COLUMNS = 8.0;
		local NUM_STICKER_ROWS = 8.0;
		local startU = 0;
		local startV = 0;
		local cellWidth = 1.0 / NUM_STICKER_COLUMNS;
		local cellHeight = 1.0 / NUM_STICKER_ROWS;
		local count = 0;
		local row = 0;
		local column = 0;

		foreach( legendType in this.LegendItemOrder )
		{
			StickerDescTexCoord[legendType] <- {
				texCoord00u = startU.tofloat() + column.tofloat() * cellWidth.tofloat(),
				texCoord00v = startV.tofloat() + row.tofloat() * cellHeight.tofloat(),
				texCoord01u = startU.tofloat() + column.tofloat() * cellWidth.tofloat(),
				texCoord01v = startV.tofloat() + (row.tofloat() + 1.0) * cellHeight.tofloat(),
				texCoord10u = startU.tofloat() + (column.tofloat() + 1.0) * cellWidth.tofloat(),
				texCoord10v = startV.tofloat() + row.tofloat() * cellHeight.tofloat(),
				texCoord11u = startU.tofloat() + (column.tofloat() + 1.0) * cellWidth.tofloat(),
				texCoord11v = startV.tofloat() + (row.tofloat() + 1.0) * cellHeight.tofloat()
			};
			count = count + 1;
			row = count / NUM_STICKER_COLUMNS;
			row = row.tointeger();
			column = count % NUM_STICKER_COLUMNS;
		}

		foreach( type, stickerDesc in StickerDescTexCoord )
		{
			this._root.addMinimapStickerDesc(type, 1.0, 1.0, 1.0, 0.80000001, stickerDesc.texCoord00u, stickerDesc.texCoord00v, stickerDesc.texCoord01u, stickerDesc.texCoord01v, stickerDesc.texCoord10u, stickerDesc.texCoord10v, stickerDesc.texCoord11u, stickerDesc.texCoord11v);
		}

		this.setScale(4096.0);
	}

	function onDeviceRestored()
	{
		this._root.updateMiniMapBackground();
	}

	function onTerrainPageLoaded( pageX, pageZ, aabb )
	{
		this._root.updateMiniMapBackground();
	}

	function _floor( x )
	{
		local toint = x.tointeger();

		if (toint <= x)
		{
			return toint;
		}
		else
		{
			return toint - 1;
		}
	}

	function onExitFrame()
	{
		if (::_avatar == null)
		{
			return;
		}

		local apos = ::_avatar.getNode().getPosition();
		this._root.setMiniMapViewCenter(apos.x, apos.z);
	}

	function setScale( s )
	{
		s = this.Math.clamp(s, this.MIN_SCALE, this.MAX_SCALE);

		if (this.mScale == s)
		{
			return;
		}

		this.mScale = s;
		this._root.setMiniMapZoom(this.mScale);
		this._root.updateMiniMapBackground();
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
		::_exitFrameRelay.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this._enterFrameRelay.removeListener(this);
		this.GUI.Component._removeNotify();
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			local clickpos = this._root.getMiniMapWorldCoordinate(evt.x, evt.y);
			this.print("clicked " + evt.x + ", " + evt.y);
			this.print("in world coords that is " + clickpos.x + ", " + clickpos.z);
			this.mMessageBroadcaster.broadcastMessage("onMinimapClick", this, clickpos.x, clickpos.z);
			evt.consume();
		}
	}

	function destroy()
	{
		::_renderSystemEventRelay.removeListener(this);
		::_scene.removeListener(this);
		this.GUI.Component.destroy();
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();
	}

	mMessageBroadcaster = null;
	mTextureCenterX = 0;
	mTextureCenterZ = 0;
	mViewPosX = 0;
	mViewPosY = 0;
	mScale = 128.0;
	mDestScale = 128.0;
	mZO = null;
	mBuildTex = null;
	mBuildCam = null;
	mBuildNode = null;
	mBuildPanel = null;
	mDirty = true;
	mStickers = null;
	mMapName = "BuildMap";
	static mClassName = "MiniMap";
}

