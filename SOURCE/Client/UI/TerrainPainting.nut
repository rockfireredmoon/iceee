this.require("GUI/GUI");
this.TerrainPaintHandler <- {};
class this.TerrainPaintHandler.Splat 
{
	mSplatTexture = 0;
	constructor( splatTexture )
	{
		this.mSplatTexture = splatTexture;
	}

	function onActivated()
	{
	}

	function onDeactivated()
	{
	}

	function onTerrainPaintStart( tool )
	{
	}

	function onTerrainPaintEnd( tool )
	{
	}

	function onTerrainPaint( tool, evt )
	{
		local pos = tool.mCursorNode.getWorldPosition();
		this._root.terrainSplatPaint(pos.x, pos.z, tool.mBrushRadius, tool.mBrushWeight, tool.mBrushFalloff, this.mSplatTexture);
		::_buildTool.markDirty("Coverage", pos, tool.mBrushRadius);
		tool.setStatus("Splatting " + this.mSplatTexture + "...");
	}

}

class this.TerrainPaintHandler.RaiseAndLower 
{
	function onTerrainPaintStart( tool )
	{
	}

	function onTerrainPaintEnd( tool )
	{
	}

	function onActivated()
	{
	}

	function onDeactivated()
	{
	}

	function onTerrainPaint( tool, evt )
	{
		local pos = tool.mCursorNode.getWorldPosition();
		local amount = this.gTerrainRaiseAmount;

		if (evt.isShiftDown())
		{
			tool.setStatus("Lowering...");
			amount = -amount;
		}
		else
		{
			tool.setStatus("Raising...");
		}

		this._root.terrainHeightPaint(pos.x, pos.z, tool.mBrushRadius, tool.mBrushWeight, tool.mBrushFalloff, 0, amount);
		::_buildTool.markDirty("Height", pos, tool.mBrushRadius);
	}

}

class this.TerrainPaintHandler.Flatten 
{
	mFlattenHeight = 0.0;
	function onActivated()
	{
	}

	function onDeactivated()
	{
	}

	function onTerrainPaintStart( tool )
	{
		this.mFlattenHeight = tool.mCursorNode.getWorldPosition().y;
	}

	function onTerrainPaintEnd( tool )
	{
	}

	function onTerrainPaint( tool, evt )
	{
		local pos = tool.mCursorNode.getWorldPosition();
		this._root.terrainHeightPaint(pos.x, pos.z, tool.mBrushRadius, tool.mBrushWeight, tool.mBrushFalloff, 1, this.mFlattenHeight);
		::_buildTool.markDirty("Height", pos, tool.mBrushRadius);
		tool.setStatus("Flattening to " + this.mFlattenHeight + "...");
	}

}

class this.TerrainPaintHandler.Smooth 
{
	mSmoothRadius = 3;
	function onActivated()
	{
	}

	function onDeactivated()
	{
	}

	function onTerrainPaintStart( tool )
	{
	}

	function onTerrainPaintEnd( tool )
	{
	}

	function onTerrainPaint( tool, evt )
	{
		local pos = tool.mCursorNode.getWorldPosition();
		this._root.terrainHeightPaint(pos.x, pos.z, tool.mBrushRadius, tool.mBrushWeight, tool.mBrushFalloff, 2, this.mSmoothRadius);
		::_buildTool.markDirty("Height", pos, tool.mBrushRadius);
		tool.setStatus("Smoothing...");
	}

}

class this.TerrainPaintHandler.Noise 
{
	function onActivated()
	{
	}

	function onDeactivated()
	{
	}

	function onTerrainPaintStart( tool )
	{
	}

	function onTerrainPaintEnd( tool )
	{
	}

	function onTerrainPaint( tool, evt )
	{
		local pos = tool.mCursorNode.getWorldPosition();
		local amount = 5;
		this._root.terrainHeightPaint(pos.x, pos.z, tool.mBrushRadius, tool.mBrushWeight, tool.mBrushFalloff, 3, amount);
		::_buildTool.markDirty("Height", pos, tool.mBrushRadius);
		tool.setStatus("Roughening...");
	}

}

