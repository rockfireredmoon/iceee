this.require("GUI/StretchproofPanel");
class this.GUI.MiniMapTile extends this.GUI.StretchproofPanel
{
	constructor( texname, texx, texy, position, scale )
	{
		this.GUI.StretchproofPanel.constructor();
		this.mName = texname;
		this.mPosition = this.Vector3(position.x, position.y, position.z);
		this.mScale = scale;
		this.mCamera = this._scene.createCamera("MinimapCam/" + this.mName);
		this.mCamera.setQueryFlags(this.QueryFlags.CAMERA);
		this.mCamera.setNearClipDistance(1.0);
		this.mCamera.setFarClipDistance(100000.0);
		local pm = [
			[
				2.0,
				0.0,
				0.0,
				-0.0
			],
			[
				0.0,
				2.0,
				0.0,
				-0.0
			],
			[
				0.0,
				0.0,
				-1.65687e-006,
				-1.00002
			],
			[
				0.0,
				0.0,
				0.0,
				1.0
			]
		];
		pm[0][0] /= this.mScale;
		pm[1][1] /= this.mScale;
		this.mCamera.setCustomProjectionMatrix(true, pm[0][0], pm[0][1], pm[0][2], pm[0][3], pm[1][0], pm[1][1], pm[1][2], pm[1][3], pm[2][0], pm[2][1], pm[2][2], pm[2][3], pm[3][0], pm[3][1], pm[3][2], pm[3][3]);
		this.mNode = this._scene.getRootSceneNode().createChildSceneNode();
		this.mNode.setPosition(this.mPosition);
		this.mNode.attachObject(this.mCamera);
		this.mNode.lookAt(this.mPosition.x, this.mPosition.y - 1000.0, this.mPosition.z);
		this.mTexture = this._root.createProceduralTexture(texname, texx, texy, this.Color("000000"));
		this.mTexture.renderScene(this.mCamera);
		this.mTexture.update();
	}

	function redrawtile()
	{
		local oldfogmode = ::_scene.getFogMode();
		local oldfogcolor = ::_scene.getFogColor();
		local oldfogdensity = ::_scene.getFogDensity();
		local oldfogstart = ::_scene.getFogStart();
		local oldfogend = ::_scene.getFogEnd();
		::_scene.setFog(::Scene.FOG_NONE, ::Color(0.0, 0.0, 0.0), 0.0, 0.0, 1.0);
		this.mTexture.update();
		::_scene.setFog(oldfogmode, oldfogcolor, oldfogdensity, oldfogstart, oldfogend);
	}

	function setCameraPosition( x, z )
	{
		this.mPosition = this.Vector3(x, 50000.0, z);
		this.mNode.setPosition(this.mPosition);
	}

	function setCameraScale( pScale )
	{
		this.mScale = pScale;
		local pm = [
			[
				2.0,
				0.0,
				0.0,
				-0.0
			],
			[
				0.0,
				2.0,
				0.0,
				-0.0
			],
			[
				0.0,
				0.0,
				-1.65687e-006,
				-1.00002
			],
			[
				0.0,
				0.0,
				0.0,
				1.0
			]
		];
		pm[0][0] /= this.mScale;
		pm[1][1] /= this.mScale;
		this.mCamera.setCustomProjectionMatrix(true, pm[0][0], pm[0][1], pm[0][2], pm[0][3], pm[1][0], pm[1][1], pm[1][2], pm[1][3], pm[2][0], pm[2][1], pm[2][2], pm[2][3], pm[3][0], pm[3][1], pm[3][2], pm[3][3]);
	}

	mName = null;
	mPosition = null;
	mScale = 1.0;
	mCamera = null;
	mNode = null;
	mTexture = null;
}

