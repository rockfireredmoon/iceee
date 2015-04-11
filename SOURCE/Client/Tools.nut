this.require("SceneObject");
this.require("UI/Manipulators");
this.require("Constants");
this.require("UseableCreatureManager");
class this.ToolManager 
{
	constructor()
	{
		::_root.addListener(this);
	}

	function reset()
	{
		foreach( t in this.mStack )
		{
			t.deactivate();
		}

		this.mStack = [];
	}

	function onTerrainSaved()
	{
		foreach( t in this.mStack )
		{
			t.onTerrainSaved();
		}
	}

	function getActiveTool()
	{
		return this.mStack.len() > 0 ? this.mStack[0] : null;
	}

	function setActiveTool( tool )
	{
		while (this.mStack.len() > 0)
		{
			this.pop();
		}

		this.push(tool);
	}

	function pop()
	{
		if (this.mStack.len() == 0)
		{
			return null;
		}

		local t = this.mStack[0];
		this.mStack.remove(0);
		t.mIsActivated = false;
		t.deactivate();

		if (this.mStack.len() > 0)
		{
			this.mStack[0].onResume();
		}

		return t;
	}

	function push( tool )
	{
		if (this.mStack.len() > 0)
		{
			this.mStack[0].onSuspend();
		}

		this.mStack.insert(0, tool);
		tool.mIsActivated = true;
		tool.activate();
	}

	function _checkCommand( command )
	{
		if (!command || command.len() == 0)
		{
			return false;
		}

		foreach( tool in this.mStack )
		{
			local method;
			local args;

			if (command[0] == 47)
			{
				local parsed = this.InputCommandHelpers.parseCommand(command);
				method = parsed.cmd;
				args = parsed.args;
			}
			else
			{
				method = command;
				args = [];
			}

			if (method in tool)
			{
				try
				{
					if (args.len() > 0)
					{
						throw this.Exception("Squirrel ACALL is broken, can\'t pass args to handler");
					}

					tool[method].call(tool);
				}
				catch( err )
				{
					this.log.error("Error with " + tool + "." + command + "(): " + err);
				}

				return true;
			}
		}

		if (command in ::GameEvents)
		{
			::GameEvents[command]();
			return true;
		}

		if (command[0] == 47)
		{
			return this.InputCommands.CheckCommand(command);
		}

		this.log.warn("Command not handled by " + this + ": " + command);
		return false;
	}

	function findFunction( command )
	{
		foreach( tool in this.mStack )
		{
			if (command in tool)
			{
				return tool;
			}
		}

		return null;
	}

	function onKeyPressed( evt )
	{
		local bindKey = this.KeyHelper.getKeyText(evt, true);

		foreach( tool in this.mStack )
		{
			tool.onKeyPressed(evt);

			if (evt.isConsumed())
			{
				return;
			}

			if (evt.keyCode == this.Key.VK_ESCAPE)
			{
				if (bindKey in tool.mKeyBindings)
				{
					this._checkCommand(tool.mKeyBindings[bindKey]);
					return;
				}

				tool.onCancel(0);
				return;
			}

			if (bindKey in tool.mKeyBindings)
			{
				this._checkCommand(tool.mKeyBindings[bindKey]);
				return;
			}
		}

		if (bindKey in ::KeyBindingDef)
		{
			this._checkCommand(::KeyBindingDef[bindKey]);
		}
	}

	function onKeyReleased( evt )
	{
		local bindKey = this.KeyHelper.getKeyText(evt, false);

		foreach( tool in this.mStack )
		{
			tool.onKeyReleased(evt);

			if (evt.isConsumed())
			{
				return;
			}

			if (bindKey in tool.mKeyBindings)
			{
				this._checkCommand(tool.mKeyBindings[bindKey]);
				return;
			}
		}

		if (bindKey in ::KeyBindingDef)
		{
			this._checkCommand(::KeyBindingDef[bindKey]);
		}
	}

	function onMousePressed( evt )
	{
		foreach( tool in this.mStack )
		{
			tool.onMousePressed(evt);

			if (evt.isConsumed())
			{
				return;
			}
		}
	}

	function onMouseReleased( evt )
	{
		foreach( tool in this.mStack )
		{
			tool.onMouseReleased(evt);

			if (evt.isConsumed())
			{
				return;
			}
		}
	}

	function onMouseMoved( evt )
	{
		foreach( tool in this.mStack )
		{
			tool.onMouseMoved(evt);

			if (evt.isConsumed())
			{
				return;
			}
		}
	}

	function onMouseWheel( evt )
	{
		foreach( tool in this.mStack )
		{
			tool.onMouseWheel(evt);

			if (evt.isConsumed())
			{
				return;
			}
		}
	}

	function onEnterFrame()
	{
		foreach( tool in this.mStack )
		{
			tool.onEnterFrame();
		}
	}

	function onExitFrame()
	{
		foreach( tool in this.mStack )
		{
			tool.onExitFrame();
		}
	}

	function _tostring()
	{
		local str = "[";

		foreach( tool in this.mStack )
		{
			if (str.len() > 1)
			{
				str += " < ";
			}

			str += tool;
		}

		return str + "]";
	}

	mStack = [];
}

class this.Tool 
{
	constructor( toolName )
	{
		this.mKeyBindings = {};
		this.mToolName = toolName;
	}

	function activate()
	{
	}

	function onTerrainSaved()
	{
	}

	function deactivate()
	{
	}

	function onSuspend()
	{
	}

	function onResume()
	{
	}

	function onSetCursor()
	{
	}

	function getCursor()
	{
		return null;
	}

	function onCancel( reason )
	{
	}

	function isActivated()
	{
		return this.mIsActivated;
	}

	function _popMe()
	{
		this.Assert.isEqual(this.mIsActivated, true);

		for( local t = this._tools.pop(); t != null && t != this;  )
		{
			t = this._tools.pop();
		}
	}

	function _setSubTool( tool )
	{
		this.Assert.isEqual(this.mIsActivated, true);
		local t = this._tools.getActiveTool();

		while (t != null && t != this)
		{
			this._tools.pop();
			t = this._tools.getActiveTool();
		}

		if (tool != null)
		{
			this._tools.push(tool);
		}
	}

	function _tostring()
	{
		return this.mToolName;
	}

	static function pickTerrainPoint( x, y, ... )
	{
		local flags = vargc > 0 ? vargv[0] : this.QueryFlags.ANY;
		local ray = this.Screen.getViewportRay(x, y);
		local hits = this._scene.rayQuery(ray.origin, ray.dir, flags, true, false);
		local h;

		foreach( h in hits )
		{
			if (!h.object)
			{
				return h.pos;
			}
		}

		if (hits.len() > 0)
		{
			return hits[0].pos;
		}

		return null;
	}

	static function pickCreature( x, y, ... )
	{
		local flags = vargc > 0 ? vargv[0] : this.QueryFlags.ANY;
		local ray = this.Screen.getViewportRay(x, y);
		local hits = this._scene.rayQuery(ray.origin, ray.dir, flags, true, false);
		local h;

		foreach( h in hits )
		{
			if (!h.object)
			{
				return h.pos;
			}
		}

		return null;
	}

	static function pickFloorPoint( x, y )
	{
		local ray = this.Screen.getViewportRay(x, y);
		local hits = this._scene.rayQuery(ray.origin, ray.dir, this.QueryFlags.FLOOR, true, true);

		if (hits.len() > 0)
		{
			return hits[0].pos;
		}

		local normal = this.Vector3(0, 1, 0);
		local denom = normal.dot(ray.dir);

		if (this.fabs(denom) <= 9.9999997e-005)
		{
			return null;
		}

		local nom = normal.dot(ray.origin);
		local t = -(nom / denom);

		if (t < 0)
		{
			return null;
		}

		return ray.origin + ray.dir * t;
	}

	function onKeyPressed( evt )
	{
	}

	function onKeyReleased( evt )
	{
	}

	function onMousePressed( evt )
	{
	}

	function onMouseReleased( evt )
	{
	}

	function onMouseMoved( evt )
	{
	}

	function onMouseWheel( evt )
	{
	}

	function onEnterFrame()
	{
	}

	function onExitFrame()
	{
	}

	mKeyBindings = null;
	mToolName = "Tool";
	mIsActivated = false;
}

class this.AbstractCameraTool extends this.Tool
{
	mCurrentDistance = 0.0;
	constructor( toolName, popOnRelease )
	{
		this.Tool.constructor(toolName);
		this.mPopOnRelease = popOnRelease;
	}

	function activate()
	{
		this._update(this.mCurrentDistance);
	}

	function _getCamera()
	{
		return this._scene.getCamera("Default");
	}

	function _getCameraNode()
	{
		return this._getCamera().getParentSceneNode();
	}

	function _update( currentDistance )
	{
	}

	mPopOnRelease = false;
}

class this.CameraOrbitTool extends this.AbstractCameraTool
{
	constructor( orbitNode, popOnRelease )
	{
		this.AbstractCameraTool.constructor("CameraOrbitTool", popOnRelease);
		this.mKeyBindings[this.KB(this.Key.VK_PAGEUP)] <- "zoomIn";
		this.mKeyBindings[this.KB(this.Key.VK_PAGEDOWN)] <- "zoomOut";
		this.mCameraDistance = ::gCamera.initalZoom;
		this.mXq = this.Quaternion(0.932163, -0.362039, 0.0, 0.0);
		this.mYq = this.Quaternion(-0.0074999998, 0.0, 0.99997199, 0.0);
		this.setOrbitNode(orbitNode);
	}

	function setOrbitNode( node )
	{
		local camNode = this._getCameraNode();

		if (node != null)
		{
			this.mOrbitNode = node;
			camNode.getParent().removeChild(camNode);
			camNode.setInheritScale(false);
			node.addChild(camNode);
		}
		else
		{
			if (this.mOrbitNode != null && this.mOrbitNode.getName() == camNode.getParent().getName())
			{
				local worldPos = camNode.getWorldPosition();
				camNode.getParent().removeChild(camNode);
				camNode.setInheritScale(false);
				this._scene.getRootSceneNode().addChild(camNode);
				camNode.setPosition(worldPos);
			}

			this.mOrbitNode = null;
		}
	}

	function _update( distance )
	{
		this.checkLimits();
		local qview = this.mYq * this.mXq;
		qview.normalize();
		local camNode = this._getCameraNode();
		camNode.setOrientation(qview);
		local qview = camNode.getOrientation();
		local tempZAxis;
		local heightCheck = 0.0;
		local distance = distance;
		tempZAxis = qview.zAxis();
		tempZAxis = tempZAxis * distance;
		tempZAxis.y = tempZAxis.y + ::gCamera.height;
		camNode.setPosition(tempZAxis);

		if (!this.mOrbitNode)
		{
			this.log.warn("CameraOrbitTool requires OrbitNode to be set.");
		}
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.mOrbitButton && !this.mIsOrbiting)
		{
			this.mXOld = evt.x;
			this.mYOld = evt.y;
			this.mIsOrbiting = true;
			this.mOrbitPerformed = false;
			this.mOrientingAvatar = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.mOrbitButton && this.mIsOrbiting)
		{
			if (::_cursor && this.mOrbitPerformed)
			{
				::_cursor.setState(this.GUI.Cursor.DEFAULT);
				::_cursor.setLocked(false);
			}

			this.mIsOrbiting = false;
			this.mOrbitPerformed = false;
			this.mOrientingAvatar = false;

			if (this.mPopOnRelease)
			{
				this._popMe();
			}
		}
	}

	function _isOrbitButtonDown( evt )
	{
		switch(this.mOrbitButton)
		{
		case 1:
			return evt.isLButtonDown();

		case 2:
			return evt.isMButtonDown();

		case 3:
			return evt.isRButtonDown();
		}

		return false;
	}

	function onMouseMoved( evt )
	{
		if (!this.mIsOrbiting)
		{
			return;
		}

		if (evt.x == this.mXOld && evt.y == this.mYOld)
		{
			return;
		}

		if (::_cursor)
		{
			::_cursor.setState(this.GUI.Cursor.ROTATE);
			::_cursor.setLocked(true, this.mXOld, this.mYOld);
		}

		this.mOrbitPerformed = true;
		local mSensitivity = 100.0;
		local phi = -(evt.x.tofloat() - this.mXOld.tofloat()) / mSensitivity;
		local theta = -(evt.y.tofloat() - this.mYOld.tofloat()) / mSensitivity / 2.0;
		local qhorz = this.Quaternion(phi, this.Vector3(0.0, 1.0, 0.0));
		local qvert = this.Quaternion(theta, this.Vector3(1.0, 0.0, 0.0));
		this.mXq = this.mXq * qvert;
		this.mXq.normalize();
		this.mYq = this.mYq * qhorz;
		this.mYq.normalize();
		this.onExitFrame();
		evt.consume();
	}

	function onMouseWheel( evt )
	{
		if (evt.units_v != 0)
		{
			local sensitivity = this.gCamera.mouseWheelSensitivity;

			if (evt.isControlDown())
			{
				sensitivity *= 50;
			}

			this.zoom(evt.units_v * sensitivity * -1);
		}
	}

	function _rayCheckPosition( avatarPos, targetPos )
	{
		local cameraDir = targetPos - avatarPos;
		cameraDir.normalize();
		local hits = ::_scene.rayQuery(avatarPos, cameraDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true, true, 1, ::_avatar.getNode());
		local so;
		local distance;

		foreach( h in hits )
		{
			if (h.t > this.mCameraMaxDistance)
			{
				break;
			}

			distance = h.t;
			return distance;
		}

		return distance;
	}

	function _checkObstruction()
	{
		if (::_avatar == null)
		{
			return false;
		}

		this.mObsticalDistance = null;
		local node = this._getCameraNode();
		local ray;
		local cameraPos = node.getWorldPosition();
		local rayArray = [];
		rayArray.append(cameraPos);
		ray = ::Screen.getViewportRay(::Screen.getWidth() / 2, 0);
		rayArray.append(ray.origin);
		ray = this.Screen.getViewportRay(::Screen.getWidth(), ::Screen.getHeight() / 2);
		rayArray.append(ray.origin);
		ray = this.Screen.getViewportRay(::Screen.getWidth() / 2, ::Screen.getHeight());
		rayArray.append(ray.origin);
		ray = this.Screen.getViewportRay(0, ::Screen.getHeight() / 2);
		rayArray.append(ray.origin);
		local avatarPos = ::_avatar.getNode().getWorldPosition();
		avatarPos.y += this.gCamera.height;
		local distance = this.mCameraMaxDistance;

		foreach( i, x in rayArray )
		{
			local shift = x - cameraPos;
			local sourcePos = avatarPos + shift / 2;
			local tempDist = this._rayCheckPosition(sourcePos, x);

			if (tempDist < distance && tempDist != null)
			{
				distance = tempDist;
			}
		}

		if (distance)
		{
			this.mObsticalDistance = distance;

			if (this.mObsticalDistance < 0.0)
			{
				this.mObsticalDistance = 0.0;
			}
		}
	}

	function onExitFrame()
	{
		this.mObsticalDistance = null;

		if (this.mIsCheckObstructions)
		{
			this._checkObstruction();
		}

		local targetDistance = this.mCameraDistance;

		if (this.mCameraDistance > this.mObsticalDistance && this.mObsticalDistance)
		{
			targetDistance = this.mObsticalDistance;
			this.mCurrentDistance = targetDistance;
		}
		else
		{
			this.mCurrentDistance = this.Math.GravitateValue(this.mCurrentDistance, targetDistance, this._deltat / 1000.0, 6.0);
		}

		this._update(this.mCurrentDistance);

		if (this.mCurrentDistance < 80.0)
		{
			if (this._avatar != null)
			{
				if (!this._avatar.getAssembler())
				{
					return;
				}

				local opacity;

				if (this.mCurrentDistance < this.gCamera.transparencyDistance)
				{
					opacity = this.Math.lerp(0.75, 0.0, (this.gCamera.transparencyDistance - this.mCurrentDistance) / this.gCamera.transparencyDistance);
				}
				else
				{
					opacity = 1.0;
				}

				::_avatar.setOpacity(opacity);
			}
		}
	}

	function zoom( pChange )
	{
		this.mCameraDistance += pChange;

		if (this.mCameraDistance < this.gCamera.minZoom)
		{
			this.mCameraDistance = this.gCamera.minZoom;
		}
		else if (this.mCameraDistance > this.mCameraMaxDistance)
		{
			this.mCameraDistance = this.mCameraMaxDistance;
		}
	}

	function zoomIn()
	{
		this.zoom(this.gCamera.stepZoom);
	}

	function zoomOut()
	{
		this.zoom(-this.gCamera.stepZoom);
	}

	function checkLimits()
	{
		if (this.mXq.x < -0.687437)
		{
			this.mXq = this.Quaternion(0.72624397, -0.687437, 0, 0);
		}

		if (this.mXq.x > 0.39589801)
		{
			this.mXq = this.Quaternion(0.91829401, 0.39589801, 0.0, 0.0);
		}
	}

	mOrbitButton = 2;
	mTwoMousePressed = false;
	mIsOrbiting = false;
	mOrbitPerformed = false;
	mOrbitNode = null;
	mXOld = 0;
	mYOld = 0;
	mXq = null;
	mYq = null;
	mObsticalDistance = null;
	mCameraDistance = 10.0;
	mCurrentDistance = 0.0;
	mCameraMaxDistance = 250;
	mIsCheckObstructions = true;
	mOrientingAvatar = false;
}

class this.CameraOrbitTool2 extends this.AbstractCameraTool
{
	constructor( orbitNode, popOnRelease )
	{
		this.AbstractCameraTool.constructor("CameraOrbitTool", popOnRelease);
		::_sceneObjectManager.addListener(this);
		this.mKeyBindings[this.KB(this.Key.VK_PAGEUP)] <- "_onZoomIn";
		this.mKeyBindings[this.KB(this.Key.VK_PAGEDOWN)] <- "_onZoomOut";
		this.setDistance(this.gCamera.initalZoom);
		this.setYaw(this.gCamera.initialYaw);
		this.setOrbitNode(orbitNode);
	}

	function setOrbitNode( node )
	{
		this.mOrbitNode = node;
	}

	function _tostring()
	{
		return "CameraOrbitTool2";
	}

	function activate()
	{
	}

	function deactivate()
	{
	}

	function onAvatarSet()
	{
	}

	function onUpdate()
	{
	}

	function onResume()
	{
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.mRotateButton)
		{
			this.mDestYaw = this.mYaw;
			this.mRotating = false;
			this.mRotatePressed = true;
		}
		else if (evt.button == this.mMoveButton)
		{
			this.mMoving = true;
		}
		else if (evt.button == this.mOrientButton)
		{
			this.mOrientPressed = true;
			this.mOrienting = false;
		}

		if (this.mOrientPressed && this.mRotatePressed)
		{
			if (::_cursor)
			{
				::_cursor.setState(this.GUI.Cursor.DEFAULT);
				::_cursor.setLocked(false);
			}

			this.mTwoMousePressed = true;
			this._forwardStart();
		}

		this.mMouseOriginX = evt.x;
		this.mMouseOriginY = evt.y;
	}

	function autoRun()
	{
	}

	function onMouseReleased( evt )
	{
		if (this.mMoving || this.mRotating || this.mOrienting)
		{
			if (::_cursor)
			{
				::_cursor.setState(this.GUI.Cursor.DEFAULT);
				::_cursor.setLocked(false);
			}
		}

		if (evt.button == this.mMoveButton && this.mMoving)
		{
			this.mMoving = false;
		}

		if (evt.button == this.mRotateButton)
		{
			this.mRotating = false;
			this.mRotatePressed = false;
		}

		if (evt.button == this.mOrientButton)
		{
			this.mOrientPressed = false;
			this.mOrienting = false;
		}

		if ((!this.mOrientPressed || !this.mRotatePressed) && this.mTwoMousePressed)
		{
			this.mTwoMousePressed = false;
			this._forwardStop();
		}

		this.mDragged = false;
	}

	function onMouseMoved( evt )
	{
		if (this.mOrientPressed)
		{
			this.mOrienting = true;
		}

		if (this.mRotatePressed)
		{
			this.mRotating = true;
		}

		if ((this.mRotating || this.mOrienting) && this.mMoving == false)
		{
			local dx = evt.x - this.mMouseOriginX;
			local dy = evt.y - this.mMouseOriginY;

			if (dx == 0 && dy == 0)
			{
				return;
			}

			local amountX = dx.tofloat() * this.gCamera.sensitivity;
			local amountY = dy.tofloat() * this.gCamera.sensitivity * (this.mMoving ? 0.25 : 1.0);
			this.mPitch = this.Math.clamp(this.mPitch - amountY, -75.0, 75.0);
			this.mDestYaw = this.Math.FloatModulos(this.mDestYaw - amountX, 360.0);
			this.mYaw = this.mDestYaw;

			if (::_cursor)
			{
				::_cursor.setState(this.GUI.Cursor.ROTATE);
				::_cursor.setLocked(true, this.mMouseOriginX, this.mMouseOriginY);
			}

			this.mDragged = true;
		}
	}

	function onMouseWheel( evt )
	{
		if (evt.units_v != 0)
		{
			local sensitivity = this.gCamera.mouseWheelSensitivity;

			if (this.mAdvancedMode == true && evt.isControlDown() == true)
			{
				sensitivity *= 5;
			}

			this.mDestDistance = this.Math.clamp(this.mDestDistance + evt.units_v * (sensitivity * 25) * -1, this.gCamera.minZoom, this.mAdvancedMode ? this.gCamera.maxZoom * 80 : this.gCamera.maxZoom);
		}
	}

	function _onZoomIn()
	{
		this.mDestDistance = this.Math.clamp(this.mDestDistance + 3.5 * (this.gCamera.sensitivity * 25) * -1, this.gCamera.minZoom, this.gCamera.maxZoom);
	}

	function _onZoomOut()
	{
		this.mDestDistance = this.Math.clamp(this.mDestDistance + -3.5 * (this.gCamera.sensitivity * 25) * -1, this.gCamera.minZoom, this.gCamera.maxZoom);
	}

	function _rayCheckPosition( avatarPos, dir, dist )
	{
		if (this.mCameraRayTestEnabled)
		{
			local hits = ::_scene.rayQuery(avatarPos, dir, this.mQueryFlags, true, false, 1, this.mOrbitNode);

			foreach( h in hits )
			{
				if (h.t >= dist)
				{
					return [
						dist,
						this.Vector3()
					];
				}

				return [
					h.t,
					h.normal
				];
			}
		}

		return [
			dist,
			this.Vector3()
		];
	}

	function getRotation()
	{
		return 0.0;
	}

	function getInMotion()
	{
		return false;
	}

	function onEnterFrame()
	{
		if (this.mOrbitNode != null)
		{
			local delta = this._deltat / 1000.0;
			local avatarPos = this.mOrbitNode.getPosition();
			local camNode = this._getCameraNode();

			if (this.mRotating == false && this.mMoving == false && this.getInMotion())
			{
				this.mDestYaw = this.Math.FloatModulos(this.Math.rad2deg(this.getRotation()) + 180.0, 360.0);
			}

			this.mYaw = this.Math.GravitateAngle(this.mYaw, this.mDestYaw, delta, 6.0);

			if (this.mOrienting || this.mRotating)
			{
				this.mDestYaw = this.mYaw;
			}

			local qx = this.Quaternion(this.Math.deg2rad(this.mPitch), this.Vector3(1.0, 0.0, 0.0));
			local qy = this.Quaternion(this.Math.deg2rad(this.mYaw), this.Vector3(0.0, 1.0, 0.0));
			local qyaw = qy * qx;
			local qpitch = this.Quaternion(this.Math.deg2rad(this.mPitch), this.Vector3(1.0, 0.0, 0.0));
			local dir = this.Vector3(0.0, 0.0, -1.0);
			dir = qyaw.rotate(dir);
			dir.normalize();
			local dneg = this.Vector3(0.0, 0.0, 1.0);
			dneg = qyaw.rotate(dneg);
			dneg.normalize();
			local scale = ::_avatar.getScale().y;
			local offset = this.Vector3(0.0, this.gCamera.height * scale, 0.0);
			local dist = this.Math.GravitateValue(this.mDistance, this.mDestDistance, delta, 6.0);
			local distScaled = dist * scale;
			local result = this._rayCheckPosition(avatarPos + offset, dneg, distScaled);
			this.mDistance = result[0] / scale;
			local camPos = avatarPos + dneg * this.mDistance * scale + offset + this.Vector3(0.0, ::_CameraObject.getNearClipDistance(), 0.0);
			camNode.setOrientation(qyaw);
			camNode.setPosition(camPos);
			this.onCameraUpdated();
		}
	}

	function onCameraUpdated()
	{
	}

	function setAdvancedMode( value )
	{
		this.mAdvancedMode = value;
	}

	function getAdvancedMode()
	{
		return this.mAdvancedMode;
	}

	function toggleAdvancedMode()
	{
		if (this.mAdvancedMode)
		{
			this.setAdvancedMode(false);
		}
		else
		{
			this.setAdvancedMode(true);
		}
	}

	function toggleSpawners()
	{
		this.mSpawnerToggle = !this.mSpawnerToggle;

		if (!this.mSpawnerToggle)
		{
			foreach( ss in this.mSelectedSpawners )
			{
				this._removeSpawner(ss);
			}

			this.mSelectedSpawners = [];
		}
		else
		{
			local scenery = ::_sceneObjectManager.getScenery();

			foreach( s in scenery )
			{
				if (!("getType" in s) || s.getType() != "Manipulator-SpawnPoint")
				{
					continue;
				}

				if (this._findSpawner(s))
				{
					continue;
				}

				this._addSpawner(s, s.getID());
			}
		}
	}

	function setDistance( value )
	{
		this.mDestDistance = value;
		this.mDistance = value;
	}

	function getDistance()
	{
		return this.mDistance;
	}

	function setDestDistance( value )
	{
		this.mDestDistance = value;
	}

	function setYaw( value )
	{
		this.mYaw = value;
		this.mDestYaw = value;
	}

	function getYaw()
	{
		return this.mYaw;
	}

	function setPitch( value )
	{
		this.mPitch = value;
	}

	function getPitch()
	{
		return this.mPitch;
	}

	function setQueryFlags( flags )
	{
		this.mQueryFlags = flags;
	}

	function getOrbiting()
	{
		return this.mOrienting || this.mRotating;
	}

	mOldX = 0;
	mOldY = 0;
	mMouseOriginX = 0;
	mMouseOriginY = 0;
	mOrbitNode = null;
	mQueryFlags = this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING;
	mAdvancedMode = true;
	mRotateButton = 2;
	mMoveButton = 1;
	mOrientButton = 3;
	mOrientPressed = false;
	mTwoMousePressed = false;
	mDragged = false;
	mOrienting = false;
	mRotatePressed = false;
	mRotating = false;
	mMoving = false;
	mDestDistance = 30.0;
	mDistance = 30.0;
	mCameraRayTestEnabled = true;
	mPitch = 0.0;
	mMenu = null;
	mYaw = 0.0;
	mDestYaw = 0.0;
}

class this.AddFriendHandler 
{
	mID = null;
	constructor( id )
	{
		this.mID = id;
	}

	function onQueryComplete( qa, results )
	{
	}

	function onQueryError( qa, err )
	{
		::IGIS.error(err);
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery("friends.add", this, [
			this.mID
		]);
	}

}

class this.SceneryLinkHandler 
{
	function onQueryComplete( qa, results )
	{
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error("" + qa.query + " failed: " + error);
	}

	function onQueryTimeout( qa )
	{
		this.IGIS.error("" + qa.query + " timed out.");
	}

}

class this.BuildTool extends this.CameraOrbitTool2
{
	constructor()
	{
		this.CameraOrbitTool2.constructor(null, false);
		this.mToolName = "BuildTool";
		this.mSceneryTool = this.SceneryTool();
		this.mPaintTool = this.TerrainPaintTool();
		this.mSelection = this.SceneObjectSelection();
		::_sceneObjectManager.addListener(this);
		this.mKeyBindings[this.KB(this.Key.VK_TAB)] <- "enableFarCam";
		this.mKeyBindings[this._KB(this.Key.VK_TAB)] <- "disableFarCam";
		this.mKeyBindings[this.KB(this.Key.VK_Z)] <- "toggleFarCam";
		this.mKeyBindings[this.c_KB(this.Key.VK_F)] <- "toggleFlyMode";
		this.mKeyBindings[this.c_KB(this.Key.VK_S)] <- "saveTerrain";
		this.mKeyBindings[this.KB(this.Key.VK_W)] <- "_forwardStart";
		this.mKeyBindings[this._KB(this.Key.VK_W)] <- "_forwardStop";
		this.mKeyBindings[this.KB(this.Key.VK_A)] <- "_leftStart";
		this.mKeyBindings[this.KB(this.Key.VK_D)] <- "_rightStart";
		this.mKeyBindings[this._KB(this.Key.VK_A)] <- "_leftStop";
		this.mKeyBindings[this._KB(this.Key.VK_D)] <- "_rightStop";
		this.mKeyBindings[this.KB(this.Key.VK_S)] <- "_backwardStart";
		this.mKeyBindings[this._KB(this.Key.VK_S)] <- "_backwardStop";
		this.mKeyBindings[this.KB(this.Key.VK_Q)] <- "_strafeLeftStart";
		this.mKeyBindings[this._KB(this.Key.VK_Q)] <- "_strafeLeftStop";
		this.mKeyBindings[this.KB(this.Key.VK_E)] <- "_strafeRightStart";
		this.mKeyBindings[this._KB(this.Key.VK_E)] <- "_strafeRightStop";
		this.mKeyBindings[this.KB(this.Key.VK_V)] <- "_toggleLinkVisibility";
		this.mKeyBindings[this.KB(this.Key.VK_B)] <- "_toggleTerrainBorders";
		this.mKeyBindings[this.KB(this.Key.VK_T)] <- "_toggleTerrainRegions";
		this.mKeyBindings[this.KB(this.Key.VK_P)] <- "_linkPatrolObjects";
		this.mKeyBindings[this.KB(this.Key.VK_L)] <- "_linkObjects";
		this.mKeyBindings[this.KB(this.Key.VK_U)] <- "_unlinkObjects";
		this.mKeyBindings[this.KB(this.Key.VK_ADD)] <- "increasePanSpeed";
		this.mKeyBindings[this.KB(this.Key.VK_SUBTRACT)] <- "decreasePanSpeed";
		this.mKeyBindings[this.KB(this.Key.VK_Z)] <- "jumpToAvatar";
		this.mKeyBindings[this.KB(this.Key.VK_C)] <- "moveAvatarToBuildNode";
		this.mKeyBindings[this.KB(this.Key.VK_DELETE)] <- "deleteObjects";
		this.mKeyBindings[this.c_KB(this.Key.VK_Z)] <- "/undo";
		this.mKeyBindings[this.c_KB(this.Key.VK_Y)] <- "/redo";
		this.mKeyBindings[this.KB(this.Key.VK_X)] <- "toggleAdvancedMode";
		this.mKeyBindings[this.c_KB(this.Key.VK_L)] <- "_toggleLocks";
		this.mKeyBindings[this.c_KB(this.Key.VK_P)] <- "/copyPos";
		this.mKeyBindings[this.c_KB(this.Key.VK_H)] <- "/hide";
		this.mKeyBindings[this.c_KB(this.Key.VK_M)] <- "/hideNonMimimapProps";
		this.mKeyBindings[this.KB(this.Key.VK_N)] <- "toggleSpawners";
		this.setQueryFlags(0);
		this.mSelectedSpawners = [];
		this.mMenu = this.GUI.PopupMenu();
		this.mMenu.addActionListener(this);
		this.mMenu.addMenuOption("ViewAuditLog", "View Audit Log");
	}

	function isInBuildMode()
	{
		if (this.mBuildScreen)
		{
			return true;
		}

		return false;
	}

	function markPageDirty( what, x, z )
	{
		if (!this.mDirtyParts)
		{
			this.mDirtyParts = {};
		}

		local pg = what + "_x" + x + "y" + z;
		this.mDirtyParts[pg] <- pg;
	}

	function getDirtyParts()
	{
		return this.mDirtyParts;
	}

	function markDirty( what, pos, radius )
	{
		local r = this.Vector3(radius, 0, radius);
		local p0 = this.Util.getTerrainPageIndex(pos - r);
		local p1 = this.Util.getTerrainPageIndex(pos + r);
		local x0 = p0.x;
		local z0 = p0.z;
		local x1 = p1.x;
		local z1 = p1.z;

		if (x0 < 0)
		{
			x0 = 0;
		}

		if (z0 < 0)
		{
			z0 = 0;
		}

		for( local z = z0; z <= z1; z++ )
		{
			for( local x = x0; x <= x1; x++ )
			{
				this.markPageDirty(what, x, z);
			}
		}
	}

	function saveTerrain()
	{
		this.log.info("Save Terrain Initiated.");

		if (!this.mDirtyParts)
		{
			this.log.info("No Dirty Terrain Parts Found.");
			return;
		}

		if (!("dev" in ::_args) || ::_args.dev != "2")
		{
			this.GUI.MessageBox.show("Terrain saving not supported in this mode.");
			return;
		}

		local tbase = this.Util.getTerrainPathInfo();
		local base = tbase[0];
		local path = tbase[1];
		this.log.info("Saving Dirty Terrain Parts");

		foreach( k, part in this.mDirtyParts )
		{
			if (this.Util.startsWith(part, "Coverage_"))
			{
				local rx = this.regexp("Coverage_x([0-9]+)y([0-9]+)");
				local res = rx.capture(part);
				local x = part.slice(res[1].begin, res[1].end).tointeger();
				local z = part.slice(res[2].begin, res[2].end).tointeger();
				local txname = base + "_" + part + ".png";
				this.log.debug("Baking splats for image " + txname);
				this._root.terrainBakeSplats(x, z, txname, path, base, 256);
				this.log.info("Saving Dirty Coverage File");
				this.System.writeTextureToFile(txname, path + txname);
				this.log.info("Saved " + path + txname);
			}
			else if (this.Util.startsWith(part, "Height_"))
			{
				this.log.info("Saving Dirty Heights File.");
				local fname = base + "_" + part + ".png";
				local rx = this.regexp("Height_x([0-9]+)y([0-9]+)");
				local res = rx.capture(part);
				local x = part.slice(res[1].begin, res[1].end).tointeger();
				local z = part.slice(res[2].begin, res[2].end).tointeger();

				try
				{
					this.System.writeTerrainHeightToFile(x, z, path + fname);
				}
				catch( err )
				{
					this.GUI.MessageBox.show("Failed to save terrain properly - " + err);
				}

				this.log.debug("Saved " + path + fname);
			}
			else
			{
				this.log.error("Don\'t know how to save terrain part: " + k);
			}
		}

		this.mDirtyParts = null;
		::_tools.onTerrainSaved();
	}

	function _toggleTerrainBorders()
	{
		::_scene.setTerrainBordersVisible(!::_scene.getTerrainBordersVisible());
		::_root.updateMiniMapBackground();
	}

	function _toggleTerrainRegions()
	{
		::_scene.setTerrainRegionsVisible(!::_scene.getTerrainRegionsVisible());
		::_root.updateMiniMapBackground();
	}

	function _toggleLocks()
	{
		local objects = this.getSelectedObjects();

		foreach( o in objects )
		{
			if (o.isScenery())
			{
				o.setLocked(!o.isLocked());
				::_Connection.sendQuery("scenery.edit", null, [
					o.getID(),
					"flags",
					o.getFlags()
				]);
			}
		}
	}

	function _linkObjects()
	{
		local objects = this.getSelectedObjects();

		if (objects.len() != 2)
		{
			this.IGIS.error("Link: Can only link 2 objects at a time");
			return;
		}

		if (objects[0].isScenery() == false || objects[1].isScenery() == false)
		{
			this.IGIS.error("Link: Only scenery can be linked");
			return;
		}

		::_scene.addLink(objects[0].getNode().getName(), objects[1].getNode().getName(), this.Color(1.0, 0.0, 1.0, 1.0));
		::_Connection.sendQuery("scenery.link.add", this.SceneryLinkHandler(), [
			objects[0].getID(),
			objects[1].getID(),
			0
		]);
	}

	function _linkPatrolObjects()
	{
		local objects = this.getSelectedObjects();

		if (objects.len() != 2)
		{
			this.IGIS.error("Link: Can only link 2 objects at a time");
			return;
		}

		if (objects[0].isScenery() == false || objects[1].isScenery() == false)
		{
			this.IGIS.error("Link: Only scenery can be linked");
			return;
		}

		::_scene.addLink(objects[0].getNode().getName(), objects[1].getNode().getName(), this.Color(0.0, 1.0, 1.0, 1.0));
		::_Connection.sendQuery("scenery.link.add", this.SceneryLinkHandler(), [
			objects[0].getID(),
			objects[1].getID(),
			1
		]);
	}

	function _unlinkObjects()
	{
		local objects = this.getSelectedObjects();

		if (objects.len() != 2)
		{
			this.IGIS.error("Unlink: Can only unlink 2 objects at a time");
			return;
		}

		if (objects[0].isScenery() == false || objects[1].isScenery() == false)
		{
			this.IGIS.error("Unlink: Only scenery can be unlinked");
			return;
		}

		::_scene.removeLink(objects[0].getNode().getName(), objects[1].getNode().getName());
		::_Connection.sendQuery("scenery.link.del", this.SceneryLinkHandler(), [
			objects[0].getID(),
			objects[1].getID()
		]);
	}

	function _toggleLinkVisibility()
	{
		this.mLinksVisible = !this.mLinksVisible;
		this._scene.setLinksVisible(this.mLinksVisible);
	}

	function getPaintTool()
	{
		return this.mPaintTool;
	}

	function _tostring()
	{
		return "BuildTool";
	}

	function _forwardStart()
	{
		this.mForward = true;
	}

	function _forwardStop()
	{
		this.mForward = false;
	}

	function _backwardStart()
	{
		this.mBackward = true;
	}

	function _backwardStop()
	{
		this.mBackward = false;
	}

	function _strafeLeftStart()
	{
		this.mLeft = true;
	}

	function _strafeLeftStop()
	{
		this.mLeft = false;
	}

	function _strafeRightStart()
	{
		this.mRight = true;
	}

	function _strafeRightStop()
	{
		this.mRight = false;
	}

	function getSelectionBox()
	{
		return this.mSelectionBox;
	}

	function updateSelectionBox( x, y )
	{
		if (this.mSelectionBox == null)
		{
			this.mSelectionBox = this.GUI.Panel();
			this.mSelectionBox.setPassThru(true);
			this.mSelectionBox.setVisible(false);
			this.mSelectionBox.setAppearance("WhiteBorder");
			this.mSelectionBox.setOverlay("GUI/SelectionBox");
			this.mSelectionBoxPoint = {
				x = x,
				y = y
			};
		}
		else
		{
			local posx = this.mSelectionBoxPoint.x;
			local posy = this.mSelectionBoxPoint.y;
			local posr = x;
			local posb = y;

			if (posr < posx)
			{
				local tmp = posx;
				posx = posr;
				posr = tmp;
			}

			if (posb < posy)
			{
				local tmp = posy;
				posy = posb;
				posb = tmp;
			}

			local w = posr - posx;
			local h = posb - posy;
			this.mSelectionBox.setPosition(posx, posy);
			this.mSelectionBox.setVisible(true);
			this.mSelectionBox.setSize(w, h);
		}
	}

	function grabRotation()
	{
		local curPos = this.mBuildNode.getWorldPosition();
		local oldY = curPos.y;
		local camPos = this._getCameraNode().getWorldPosition();
		local dir = curPos - camPos;
		dir.normalize();
		local yangle = this.Math.polarAngle(dir.x, dir.z);
		this.mRotation = yangle;
	}

	function _leftStart()
	{
		if (this.mOrienting == false && this.mMoving == false && this.mTurn == 0.0)
		{
			this.grabRotation();
			this.mTurn = 1.0;
		}
	}

	function _rightStart()
	{
		if (this.mOrienting == false && this.mMoving == false && this.mTurn == 0.0)
		{
			this.grabRotation();
			this.mTurn = -1.0;
		}
	}

	function _leftStop()
	{
		this.mTurn = 0.0;
	}

	function _rightStop()
	{
		this.mTurn = 0.0;
	}

	function getInMotion()
	{
		return this.mTurn != 0.0;
	}

	function setPreviewMode( which )
	{
		this.mPreviewMode = which;
	}

	function activate()
	{
		this._screenResizeRelay.addListener(this);
		this.mBorder = ::GUI.Panel(null);
		this.mBorder.setAppearance("RedBorder");
		this.mBorder.setOverlay("GUI/EditBorderOverlay");
		this.mBorder.setVisible(true);
		this.onScreenResize();
		this.mBuildNode = this._scene.getRootSceneNode().createChildSceneNode("BuildTool/Node");
		this.mBuildCompass = this._scene.createDecal("BuildTool/Compass", "BuildCompass", 15.0, 15.0);
		this.mBuildCompass.setVisibilityFlags(this.VisibilityFlags.HELPER_GEOMETRY);
		this.mBuildNode.attachObject(this.mBuildCompass);
		this.onCameraUpdated();

		if (this._avatar)
		{
			this.mBuildNode.setPosition(this._avatar.getNode().getWorldPosition());
		}
		else
		{
			this.mBuildNode.setPosition(this._getCameraNode().getWorldPosition());
		}

		this.setOrbitNode(this.mBuildNode);
		this.grabRotation();
		this.setAdvancedMode(true);
		this._setSubTool(this.mSceneryTool);
		::_loadNode = this.mBuildNode;
		this._scene.setLinksVisible(this.mLinksVisible);
		::_ChatWindow.setVisible(true);
		this.mBuildPuff = this._scene.createParticleSystem("Build_Particle_Puff", "Par-Point");
		this.mBuildPuff.setVisibilityFlags(this.VisibilityFlags.HELPER_GEOMETRY);
		this.mBuildNode.attachObject(this.mBuildPuff);

		if (::_playTool != null)
		{
			this.setYaw(::_playTool.getYaw());
			this.setPitch(::_playTool.getPitch());
			this.setDistance(::_playTool.getDistance());
		}

		::_scene.setRayQueryLength(4000.0);
	}

	function onScreenResize()
	{
		this.mBorder.setSize(::Screen.getWidth(), ::Screen.getHeight());
	}

	function deactivate()
	{
		this._manipulators.removeAllManipulators();
		this._screenResizeRelay.removeListener(this);
		::_loadNode = null;
		::_CameraObject.setLodBias(1.0);

		if (this.mShowTerrainOnly)
		{
			this._scene.setVisibilityMask(this.mPreviousVisibilityFlags);
		}

		this.mShowTerrainOnly = false;
		this.mPreviousVisibilityFlags = null;
		this.mBorder.destroy();
		this.mBorder = null;
		this.setOrbitNode(null);
		this.mBuildPuff.destroy();
		this.mBuildPuff = null;
		this.mBuildCompass.destroy();
		this.mBuildCompass = null;
		this.mBuildNode.destroy();
		this.mBuildNode = null;
		this.Screens.hide("BuildScreen");
		this.mBuildScreen = null;
		this._scene.setVisibilityMask(this._scene.getVisibilityMask() & ~this.VisibilityFlags.HELPER_GEOMETRY);
		this._scene.setTerrainBordersVisible(false);
		this._scene.setLinksVisible(false);
		this._root.updateMiniMapBackground();
		::_scene.setRayQueryLength(500.0);
	}

	function getBuildNode()
	{
		return this.mBuildNode;
	}

	function onAvatarSet()
	{
		if (this.mIsActivated)
		{
			this.jumpToAvatar();
		}
	}

	function jumpToAvatar()
	{
		this.mYOffset = 10;

		if (this.mBuildNode && this._avatar != null)
		{
			this.mBuildNode.setPosition(this._avatar.getNode().getWorldPosition());
		}
	}

	function moveAvatarToBuildNode()
	{
		if (this._avatar != null)
		{
			local pos = this.mBuildNode.getPosition();
			this._Connection.sendGo(pos.x, pos.y + 50, pos.z);
		}
	}

	function setStatusText( text )
	{
		if (this.mAdvancedMode && this.mBuildScreen)
		{
			this.mBuildScreen.setStatusText(text);
		}
	}

	function onCameraUpdated()
	{
		if (this.mBuildCompass)
		{
			local scale = (this._getCameraNode().getPosition() - this.mBuildNode.getPosition()).length();
			scale /= 100.0;
			this.mBuildCompass.setSize(15.0 * scale, 15.0 * scale);
		}
	}

	function isFarCamEnabled()
	{
		return this.mCameraNearDistance != null;
	}

	function enableFarCam()
	{
		if (this.isFarCamEnabled())
		{
			return;
		}

		this.mCameraNearDistance = this.mCameraDistance;
		this.mCameraDistance = this.mCameraFarDistance;
	}

	function disableFarCam()
	{
		if (!this.isFarCamEnabled())
		{
			return;
		}

		this.mCameraFarDistance = this.mCameraDistance;
		this.mCameraDistance = this.mCameraNearDistance;
		this.mCameraNearDistance = null;
	}

	function toggleFarCam()
	{
		if (this.isFarCamEnabled())
		{
			this.disableFarCam();
		}
		else
		{
			this.enableFarCam();
		}
	}

	function insertScenery( asset )
	{
		this.mSceneryTool._beginInsert(asset);
		this.setStatusText("Insert new scenery.");
	}

	function getSceneryTool()
	{
		return this.mSceneryTool;
	}

	function selectSceneryTool( ... )
	{
		this.mSpawnersOnly = vargc > 0 ? vargv[0] : false;

		if (this.mSceneryTool.isActivated())
		{
			this.mSceneryTool.onCancel(1);
		}
		else
		{
			this._setSubTool(this.mSceneryTool);
		}

		return this.mSceneryTool;
	}

	function selectPaintTool()
	{
		if (this.mPaintTool.isActivated())
		{
			this.mPaintTool.onCancel(1);
		}
		else
		{
			this._setSubTool(this.mPaintTool);
		}

		return this.mPaintTool;
	}

	function getSelection()
	{
		return this.mSelection;
	}

	function selectionAdd( so )
	{
		if (so == null || this.mPreviewMode == true)
		{
			return;
		}

		if (this.mSpawnersOnly == true)
		{
			if (so.isScenery() && so.getType().find("SpawnPoint") == null)
			{
				return;
			}
		}

		this.mSelection.add(so);

		if (this.mAdvancedMode && this.mBuildScreen)
		{
			if (this.mSelection.len() == 1)
			{
				this.mBuildScreen.updateSelectedObject(so);
			}
			else
			{
				this.mBuildScreen.updateSelectedObject(null);
			}
		}

		this._manipulators.removeAllManipulators();

		if (this.mSelection.len() == 1)
		{
			this._manipulators.addManipulator(this.ObjectTranslateAxisManipulator(so, "x"));
			this._manipulators.addManipulator(this.ObjectTranslateAxisManipulator(so, "y"));
			this._manipulators.addManipulator(this.ObjectTranslateAxisManipulator(so, "z"));
			this._manipulators.addManipulator(this.ObjectRotateManipulator(so, "y"));

			if (this.mAdvancedMode)
			{
				this._manipulators.addManipulator(this.ObjectRotateManipulator(so, "x", false));
				this._manipulators.addManipulator(this.ObjectRotateManipulator(so, "z", false));
			}
		}

		this._refreshSpawners();
	}

	function selectionRemove( so )
	{
		if (so == null)
		{
			return;
		}

		so = this.mSelection.remove(so);

		if (this.mAdvancedMode)
		{
			if (so == this.mBuildScreen.getSelectedObject())
			{
				this.mBuildScreen.updateSelectedObject(null);
			}
		}

		this._manipulators.removeAllManipulators();
		this._refreshSpawners();
	}

	function selectionClear()
	{
		this.mSelection.clear();

		if (this.mBuildScreen)
		{
			this.mBuildScreen.updateSelectedObject(null);
		}

		this._manipulators.removeAllManipulators();
		this._refreshSpawners();
	}

	function getSelectedObjects( ... )
	{
		if (vargc > 0)
		{
			return this.mSelection.objects(vargv[0]);
		}
		else
		{
			return this.mSelection.objects();
		}
	}

	function deleteObjects()
	{
		local count = 0;
		local op = this.CompoundOperation();
		local so;

		foreach( so in this.mSelection.objects() )
		{
			if (so.isScenery())
			{
				if (so.getType() == "Manipulator-SpawnPoint")
				{
					local i = 0;

					foreach( ss in this.mSelectedSpawners )
					{
						if (ss.ID == so.getID())
						{
							this.mSelectedSpawners.remove(i);
							this._removeSpawner(ss);
							break;
						}

						i++;
					}
				}

				op.add(this.SceneryDeleteOp(so));
			}
			else if (so.isCreature())
			{
				op.add(this.CreatureDeleteOp(so));
			}
			else
			{
				this.log.info("TODO: Delete Op: " + so);
			}
		}

		if (op.len() > 1)
		{
			op.setPresentationName("Delete " + op.len() + " objects");
		}

		this._opHistory.execute(op);
		this._manipulators.removeAllManipulators();
	}

	function toggleFlyMode()
	{
		this.mFlyMode = !this.mFlyMode;
	}

	function increasePanSpeed()
	{
		this.mPanSpeed += 10;
	}

	function decreasePanSpeed()
	{
		if (this.mPanSpeed > 10)
		{
			this.mPanSpeed -= 10;
		}
	}

	function onEnterFrame()
	{
		local panx = 0;
		local pany = 0;
		local doupdate = false;

		if (::_UIVisible == true)
		{
			this._scene.setVisibilityMask(this._scene.getVisibilityMask() | this.VisibilityFlags.HELPER_GEOMETRY);
		}
		else
		{
			this._scene.setVisibilityMask(this._scene.getVisibilityMask() & ~this.VisibilityFlags.HELPER_GEOMETRY);
		}

		if (this._root.hasKeyboardFocus())
		{
			if (this.Key.isDown(this.Key.VK_R))
			{
				this.mYOffset += this._deltat.tofloat() / 50 * this.mPanSpeed;
				doupdate = true;
			}

			if (this.Key.isDown(this.Key.VK_F))
			{
				this.mYOffset -= this._deltat.tofloat() / 50 * this.mPanSpeed;
				doupdate = true;
			}

			if (this.mLeft)
			{
				panx -= 1;
			}

			if (this.mRight)
			{
				panx += 1;
			}

			if (this.mForward)
			{
				pany += 1;
			}

			if (this.mBackward)
			{
				pany -= 1;
			}

			if (panx || pany || doupdate)
			{
				this.pan(panx * this._deltat / 5, pany * this._deltat / 5);
			}
		}

		this.CameraOrbitTool2.onEnterFrame();
	}

	function pan( dx, dy )
	{
		local curPos = this.mBuildNode.getWorldPosition();
		local oldY = curPos.y;
		local camPos = this._getCameraNode().getWorldPosition();
		local dir = curPos - camPos;
		local sensitivity = dir.length() / 1500 * this.mPanSpeed + 1;
		dir.normalize();
		local panx = dx.tofloat() * sensitivity;
		local pany = dy.tofloat() * sensitivity;
		local ax = dir.cross(this.Vector3().UNIT_Y);
		local ay = this.Vector3().UNIT_Y.cross(ax);
		local displacement = ax * panx + ay * pany;
		local stepHeight = 1.5;
		local finalPos = this.getAdvancedMode() == false ? this.Util.collideAndSlide(this.Vector3(2.0, 2.0, 2.0), stepHeight, curPos, displacement, this.QueryFlags.BLOCKING).pos : curPos + displacement;
		local y = this.Util.getFloorHeightAt(finalPos, 10.0, this.QueryFlags.FLOOR);

		if (y != null)
		{
			finalPos.y = y + this.mYOffset;
		}
		else
		{
			finalPos.y = this.mYOffset;
		}

		this.mBuildNode.setPosition(finalPos);
	}

	function getBuildNodePosition()
	{
		return this.mBuildNode.getPosition();
	}

	function setShowTerrainOnly( terrainOnly )
	{
		return this.mShowTerrainOnly = terrainOnly;
	}

	function isShowingTerrainOnly()
	{
		return this.mShowTerrainOnly;
	}

	function setPreviousVisibilityFlags( flags )
	{
		this.mPreviousVisibilityFlags = flags;
	}

	function getPreviousVisibilityFlags()
	{
		return this.mPreviousVisibilityFlags;
	}

	function onMousePressed( evt )
	{
		this.GUI._Manager.addMouseCapturer(this);

		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == 1)
		{
			local so = this._sceneObjectManager.pickSceneObject(evt.x, evt.y, this.QueryFlags.ANY, evt.isControlDown());

			if (evt.isShiftDown() && evt.isAltDown())
			{
				this.updateSelectionBox(evt.x, evt.y);
			}

			if (!this.mSelection.contains(so))
			{
				if (!evt.isShiftDown())
				{
					this.selectionClear();
				}

				this.selectionAdd(so);
				evt.consume();
				return;
			}
		}

		this.CameraOrbitTool2.onMousePressed(evt);
		evt.consume();
	}

	function onMouseMoved( evt )
	{
		if (this.mSelectionBox)
		{
			this.updateSelectionBox(evt.x, evt.y);
			evt.consume();
		}
		else
		{
			this.CameraOrbitTool2.onMouseMoved(evt);
		}
	}

	function onMouseReleased( evt )
	{
		this.GUI._Manager.removeMouseCapturer(this);

		if (this.mSelectionBox != null)
		{
			local x = this.mSelectionBox.getPosition().x;
			local y = this.mSelectionBox.getPosition().y;
			local w = this.mSelectionBox.getWidth();
			local h = this.mSelectionBox.getHeight();

			foreach( k, v in ::_sceneObjectManager.getScenery() )
			{
				if (::_scene.getVerticesInRectangle(x, y, w, h, v.getNode()))
				{
					local tmp = evt.isControlDown();
					local tmp2 = v.isLocked();

					if (evt.isControlDown() || v.isLocked() == false)
					{
						this.selectionAdd(v);
					}
				}
			}

			foreach( k, v in ::_sceneObjectManager.getCreatures() )
			{
				if (::_scene.getVerticesInRectangle(x, y, w, h, v.getNode()))
				{
					local tmp = evt.isControlDown();
					local tmp2 = v.isLocked();

					if (evt.isControlDown() || v.isLocked() == false)
					{
						this.selectionAdd(v);
					}
				}
			}

			this.mSelectionBoxPoint = null;
			this.mSelectionBox.destroy();
			this.mSelectionBox = null;
		}

		if (evt.button == this.MouseEvent.RBUTTON && this.mDragged == false)
		{
			local so = this._sceneObjectManager.pickSceneObject(evt.x, evt.y, this.QueryFlags.ANY, evt.isControlDown());

			if (so != null)
			{
				this.mMenu.setData(so.getID());
				this.mMenu.showMenu();
				this.mRotating = false;
			}
		}

		this.CameraOrbitTool2.onMouseReleased(evt);
		evt.consume();
	}

	function onMenuItemPressed( menu, menuID )
	{
		if (menuID == "ViewAuditLog")
		{
			local id = menu.getData();

			if (::_sceneObjectManager.hasObject("Creature", id))
			{
				local creature = ::_sceneObjectManager.getCreatureByID(id);

				if (creature == null)
				{
					  // [019]  OP_JMP            0     26    0    0
				}

				local creatureDef = creature.getCreatureDef();

				if (creatureDef == null)
				{
					  // [025]  OP_JMP            0     20    0    0
				}

				this.System.openURL("https://secure.sparkplaymedia.com/ee/auditlog/logview.php?type=CreatureDef&id=" + creatureDef.getID());
			}

			if (::_sceneObjectManager.hasObject("Scenery", id))
			{
				this.System.openURL("https://secure.sparkplaymedia.com/ee/auditlog/logview.php?type=Scenery&id=" + id);
			}
		}
		else
		{
		}
	}

	function getRotation()
	{
		local factor = ::_avatar != null ? ::_avatar.getController().getTurnSpeed() : 1.0;
		this.mRotation += this.mTurn * (this._deltat / 1000.0) * factor;
		return this.mRotation;
	}

	function setAdvancedMode( value )
	{
		this.CameraOrbitTool2.setAdvancedMode(value);

		if (this.mAdvancedMode)
		{
			if (this.mPreviewMode == false)
			{
				this.mBuildScreen = this.Screens.show("BuildScreen");
			}

			::_CameraObject.setLodBias(16.0);
		}
		else
		{
			::_CameraObject.setLodBias(1.0);
			this.selectSceneryTool();
			this.Screens.hide("BuildScreen");
			this.mBuildScreen = null;
		}

		this.selectionClear();
	}

	function updateSpawnerFeedback( s )
	{
		foreach( ss in this.mSelectedSpawners )
		{
			if (ss.ID == s.getID())
			{
				local props = s.getProperties();
				local inner = 2 * props.innerRadius;
				local outer = 2 * props.outerRadius;
				ss.inner_projector.setOrthoWindow(inner, inner);
				ss.outer_projector.setOrthoWindow(outer, outer);
				return;
			}
		}
	}

	function _findSpawner( s )
	{
		foreach( ss in this.mSelectedSpawners )
		{
			if (ss.ID == s.getID())
			{
				return true;
			}
		}

		return false;
	}

	function _findSpawnerIdInSelection( id )
	{
		local selection = this.mSelection.objects();

		if (selection)
		{
			foreach( s in selection )
			{
				if (id == s.getID())
				{
					return true;
				}
			}
		}

		return false;
	}

	function _addSpawner( s, ownerId )
	{
		if (this._findSpawner(s))
		{
			return;
		}

		local props = s.getProperties();
		local inner = 2 * this.Util.tableSafeGet(props, "innerRadius", 0);
		local outer = 2 * this.Util.tableSafeGet(props, "outerRadius", 0);
		local root_node = ::_scene.getRootSceneNode();
		local ip = this._scene.createTextureProjector(s.getNodeName() + "/FeedbackInnerProjector", "Area_Circle_Green.png");
		ip.setNearClipDistance(0.1);
		ip.setFarClipDistance(500);
		ip.setAlphaBlended(true);
		ip.setOrthoWindow(inner, inner);
		ip.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
		ip.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
		local op = this._scene.createTextureProjector(s.getNodeName() + "/FeedbackOuterProjector", "Area_Circle_Green.png");
		op.setNearClipDistance(0.1);
		op.setFarClipDistance(500);
		op.setAlphaBlended(true);
		op.setOrthoWindow(outer, outer);
		op.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
		op.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
		local ipn = root_node.createChildSceneNode();
		ipn.attachObject(ip);
		local opn = root_node.createChildSceneNode();
		opn.attachObject(op);
		local pos = s.getNode().getWorldPosition();
		pos.y += 200;
		ipn.setPosition(pos);
		ipn.lookAt(pos + this.Vector3(0, -1, 0));
		opn.setPosition(pos);
		opn.lookAt(pos + this.Vector3(0, -1, 0));
		this.mSelectedSpawners.append({
			ID = s.getID(),
			ownerID = ownerId,
			inner_projector = ip,
			inner_projectorNode = ipn,
			outer_projector = op,
			outer_projectorNode = opn
		});
	}

	function _removeSpawner( ss )
	{
		this._scene.getRootSceneNode().removeChild(ss.inner_projectorNode);
		ss.inner_projectorNode.detachObject(ss.inner_projector);
		ss.inner_projector.destroy();
		ss.inner_projectorNode.destroy();
		this._scene.getRootSceneNode().removeChild(ss.outer_projectorNode);
		ss.outer_projectorNode.detachObject(ss.outer_projector);
		ss.outer_projector.destroy();
		ss.outer_projectorNode.destroy();
	}

	function _refreshSpawnersPos()
	{
		foreach( s in this.mSelectedSpawners )
		{
			local ss = this._sceneObjectManager.getSceneryByID(s.ID);

			if (ss)
			{
				local pos = ss.getNode().getWorldPosition();
				pos.y += 200;
				s.inner_projectorNode.setPosition(pos);
				s.outer_projectorNode.setPosition(pos);
			}
		}
	}

	function _refreshSpawners()
	{
		if (this.mSpawnerToggle)
		{
			return;
		}

		local selection = this.mSelection.objects();

		if (selection)
		{
			foreach( s in selection )
			{
				if (!("getType" in s) || s.getType() != "Manipulator-SpawnPoint")
				{
					continue;
				}

				if (this._findSpawner(s))
				{
					continue;
				}

				this._addSpawner(s, s.getID());
				::_Connection.sendQuery("spawn.emitters", this, [
					s.getID()
				]);
			}
		}

		for( local i = this.mSelectedSpawners.len() - 1; i >= 0; i-- )
		{
			local ss = this.mSelectedSpawners[i];

			if (!this._findSpawnerIdInSelection(ss.ownerID))
			{
				this._removeSpawner(ss);
				this.mSelectedSpawners.remove(i);
			}
		}
	}

	function _convertToXFormObjs( objectArray )
	{
		local templateObjects = [];

		foreach( item in objectArray )
		{
			local newObj = {
				asset = item.asset
			};

			if ("position" in item)
			{
				if ("x" in item.position)
				{
					newObj.position <- this.Vector3(item.position.x, item.position.y, item.position.z);
				}
				else
				{
					newObj.position <- this.Vector3(item.position[0], item.position[1], item.position[2]);
				}
			}

			if ("orientation" in item)
			{
				if ("x" in item.orientation)
				{
					newObj.orientation <- this.Quaternion(item.orientation.w, item.orientation.x, item.orientation.y, item.orientation.z);
				}
				else
				{
					newObj.orientation <- this.Quaternion(item.orientation[3], item.orientation[0], item.orientation[1], item.orientation[2]);
				}
			}
			else
			{
				newObj.orientation <- this.Quaternion(1, 0, 0, 0);
			}

			if ("scale" in item)
			{
				if ("x" in item.scale)
				{
					newObj.scale <- this.Vector3(item.scale.x, item.scale.y, item.scale.z);
				}
				else
				{
					newObj.scale <- this.Vector3(item.scale[0], item.scale[1], item.scale[2]);
				}
			}
			else
			{
				newObj.scale <- this.Vector3(1, 1, 1);
			}

			templateObjects.append(newObj);
		}

		if (templateObjects.len() == 0)
		{
			return null;
		}

		return templateObjects;
	}

	function templateSaveQuery( name, objectArray )
	{
		local s = this.serialize(objectArray);

		if (s.len() < 4096)
		{
			this._Connection.sendQuery("build.template.save", this, name, s);
		}
		else
		{
			this.IGIS.error("Too many objects in a template");
		}
	}

	function templateGetQuery( name )
	{
		this._Connection.sendQuery("build.template.get", this, name);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "spawn.emitters")
		{
			local ownerId = qa.args[0];

			if (!this._findSpawnerIdInSelection(ownerId))
			{
				return;
			}

			foreach( si in results )
			{
				local ss = this._sceneObjectManager.getSceneryByID(si[0]);

				if (ss && ss.getID() != ownerId)
				{
					this._addSpawner(ss, ownerId);
				}
			}
		}
		else if (qa.query == "build.template.get")
		{
			foreach( item in results )
			{
				local resultString = item[0];

				if (resultString != "NOT FOUND")
				{
					local objectArray = this.unserialize(resultString);
					objectArray = this._convertToXFormObjs(objectArray);
					this.mSceneryTool._beginTemplateInsert(objectArray);
				}
			}
		}
		else if (qa.query == "build.template.save")
		{
			this.mBuildScreen.triggerBuildTemplateList();
		}
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "scenery.edit")
		{
			::_ChatWindow.addMessage("err/", error, "General");
		}
	}

	mBorder = null;
	mCameraNearDistance = null;
	mCameraFarDistance = 3000;
	mBuildNode = null;
	mBuildCompass = null;
	mBuildPuff = null;
	mBuildScreen = null;
	mLinksVisible = true;
	mFlyMode = false;
	mPanSpeed = 2.5;
	mShowTerrainOnly = false;
	mPreviousVisibilityFlags = null;
	mSpawnersOnly = false;
	mDirtyParts = null;
	mForward = false;
	mLeft = false;
	mRight = false;
	mBackward = false;
	mYOffset = 5.0;
	mRotation = 0.0;
	mTurn = 0.0;
	mSelection = null;
	mSelectionBox = null;
	mSelectionBoxPoint = null;
	mSceneryTool = null;
	mPaintTool = null;
	mIsCheckObstructions = false;
	mSelectedSpawners = null;
	mSpawnerToggle = false;
	mPreviewMode = false;
}

class this.SceneryTool extends this.Tool
{
	constructor()
	{
		this.Tool.constructor("SceneryTool");
		this.mKeyBindings[this.KB(this.Key.VK_DOWN)] <- "decreaseFloorOffset";
		this.mKeyBindings[this.c_KB(this.Key.VK_DOWN)] <- "decreaseFloorOffset";
		this.mKeyBindings[this.KB(this.Key.VK_UP)] <- "increaseFloorOffset";
		this.mKeyBindings[this.c_KB(this.Key.VK_UP)] <- "increaseFloorOffset";
		this.mKeyBindings[this.KB(this.Key.VK_RIGHT)] <- "resetFloorOffset";
	}

	function activate()
	{
	}

	function deactivate()
	{
		this._end();
	}

	function snapToGrid( value )
	{
		this.mSnapToGrid = value;
	}

	function snapToFloor( value )
	{
		this.mSnapToFloor = value;
	}

	function getSnapToGrid()
	{
		return this.mSnapToGrid;
	}

	function getSnapToFloor()
	{
		return this.mSnapToFloor;
	}

	function onCancel( reason )
	{
		if (this.mUpdateSet)
		{
			local item;

			foreach( item in this.mUpdateSet )
			{
				this.Util.setNodeXform(item.node, item.initialXform);
			}
		}

		this._end();
		this._buildTool.setStatusText(this.mDefaultStatus);
	}

	function onAssetComplete( assetRef )
	{
		this.mStartPos = this.pickFloorPoint(this.Screen.getWidth() / 2, this.Screen.getHeight() / 2);

		if (this.mStartPos == null)
		{
			this.mStartPos = this._buildTool.getBuildNodePosition();
		}

		this.mDragOffset = this.Vector3();
		local node = this._scene.getRootSceneNode().createChildSceneNode();
		node.setPosition(this.mStartPos);
		this.AssembleAsset(assetRef, node, true);
		this.Util.setNodeOpacity(node, 0.40000001);
		this.mPreviewSet = [
			{
				soObjectClass = "Scenery",
				initialXform = {
					position = this.mStartPos,
					orientation = this.Quaternion(),
					scale = this.Vector3(1, 1, 1)
				},
				floorOffset = 0.0,
				asset = assetRef,
				node = node
			}
		];
		this.mSet = this.mPreviewSet;
		this._buildTool.setStatusText("Inserting... (Alt+Drag = rotate)");
	}

	function onSuspend()
	{
		if (this.mSet)
		{
			local item;

			foreach( item in this.mSet )
			{
				item.node.getParent().removeChild(item.node);
			}
		}
	}

	function onResume()
	{
		if (this.mSet)
		{
			local item;

			foreach( item in this.mSet )
			{
				this._scene.getRootSceneNode().addChild(item.node);
			}
		}
	}

	function _updatePreview( value )
	{
		if (value)
		{
			if (this.mUpdateSet && this.mPreviewSet == null)
			{
				local item;
				this.mPreviewSet = [];

				foreach( item in this.mUpdateSet )
				{
					local preview = {
						initialXform = item.initialXform,
						floorOffset = item.floorOffset,
						soObjectClass = item.so.getObjectClass(),
						asset = item.asset,
						so = item.so
					};

					if (item.so.isScenery())
					{
						preview.previewSO <- this.SceneObject("Preview_Of_" + item.so.getID(), "Scenery");
						preview.previewSO.setTypeFromString(item.asset);
						preview.previewSO.reassemble();
						preview.node <- preview.previewSO.getNode();
					}
					else if (item.so.isCreature())
					{
						preview.previewSO <- this.SceneObject("Preview_Of_" + item.so.getID(), "Creature");
						preview.previewSO.setType(item.so.getType());
						preview.node <- preview.previewSO.getNode();
					}
					else
					{
						this.log.warn("Don\'t know how to create preview of " + item.so);
						preview.node <- this._scene.getRootSceneNode().createChildSceneNode();
					}

					this.Util.setNodeOpacity(preview.node, 0.40000001);
					this.Util.setNodeXform(preview.node, this.Util.getNodeXform(item.node));
					this.mPreviewSet.append(preview);
				}
			}

			this.mUpdateSet = null;
		}
		else if (this.mPreviewSet)
		{
			local item;

			foreach( item in this.mPreviewSet )
			{
				if ("previewSO" in item)
				{
					item.previewSO.destroy();
				}
				else
				{
					item.node.destroy();
				}
			}

			this.mPreviewSet = null;
		}
	}

	function onMousePressed( evt )
	{
		if (evt.isConsumed())
		{
			return;
		}

		if (evt.button == 1)
		{
			this.mCursorStartX = evt.x;
			this.mCursorStartY = evt.y;
			::Screen.setMouseCapture(this);
		}
		else if (evt.button == 3 && this.mStartPos)
		{
			this.resetFloorOffset();
		}
	}

	function onMouseReleased( evt )
	{
		if (this.mRotationNode)
		{
			this.mRotationNode.destroy();
			this.mRotationNode = null;
		}

		if (evt.isConsumed())
		{
			return;
		}

		::Screen.setMouseCapture(null);

		if (evt.button != 1)
		{
			return;
		}

		this.mDragOffset = null;

		if (this.mStartPos)
		{
			this._end();
		}
	}

	function onMouseMoved( evt )
	{
		if (evt.isConsumed())
		{
			return;
		}

		if (::_buildTool.getSelectionBox() != null)
		{
			return;
		}

		local pos = this.pickFloorPoint(evt.x, evt.y);

		if (pos == null)
		{
			return;
		}

		if (this.mSet == null)
		{
			this._buildTool.setStatusText(this.mDefaultStatus);

			if (!evt.isLButtonDown() || this.abs(this.mCursorStartX - evt.x) < 2 && this.abs(this.mCursorStartY - evt.y) < 2)
			{
				return;
			}

			if (!this._selectedObjects())
			{
				return;
			}

			this._beginMove(evt.isControlDown() && !evt.isShiftDown());

			if (this.mSet)
			{
				local count = 0;
				this.mStartPos = null;

				foreach( item in this.mSet )
				{
					if (this.mStartPos == null)
					{
						this.mStartPos = item.initialXform.position;
					}
					else
					{
						this.mStartPos += item.initialXform.position;
					}

					count += 1;
				}

				if (count > 1)
				{
					this.mStartPos.x /= count;
					this.mStartPos.y /= count;
					this.mStartPos.z /= count;
				}

				if (this.mSnapToGrid)
				{
					this.mStartPos.x = (this.mStartPos.x / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
					this.mStartPos.y = (this.mStartPos.y / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
					this.mStartPos.z = (this.mStartPos.z / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
				}

				this.mDragOffset = this.mStartPos - pos;
			}
		}

		if (!this.mSet)
		{
			return;
		}

		local dir = pos - this.mStartPos;

		if (evt.isAltDown() && this._buildTool.getAdvancedMode())
		{
			local angle = (this.mCursorStartX - evt.x) * 0.0099999998;

			if (evt.isShiftDown())
			{
				local increment = this.gBuildRotateSnap;
				angle = (angle / increment).tointeger() * increment;
			}

			if (!this.mRotationNode && this.mSet.len() > 1)
			{
				this.mRotationNode = this._scene.createSceneNode();
				this.mRotationNode.setPosition(this.mStartPos);
				this._scene.getRootSceneNode().addChild(this.mRotationNode);

				foreach( item in this.mSet )
				{
					local iNode = this._scene.createSceneNode();
					iNode.setPosition(item.initialXform.position - this.mStartPos);
					this.mRotationNode.addChild(iNode);
				}
			}

			if (this.mRotationNode)
			{
				this.mRotationNode.setOrientation(this.Quaternion(1, 0, 0, 0));
				this.mRotationNode.rotate(this.Vector3().UNIT_Y, -angle, 0);
				local count = 0;

				foreach( n in this.mRotationNode.getChildren() )
				{
					local item = this.mSet[count++];
					item.node.setPosition(n.getWorldPosition());
					item.node.setOrientation(item.initialXform.orientation);
					item.node.rotate(this.Vector3().UNIT_Y, -angle, 0);
				}
			}
			else
			{
				local item = this.mSet[0];
				item.node.setOrientation(item.initialXform.orientation);
				item.node.rotate(this.Vector3().UNIT_Y, -angle, 0);
			}

			this._buildTool.setStatusText("Rotating... (Shift = constrain to 15 degrees)");
		}
		else if ((evt.isShiftDown() && !evt.isControlDown()) && this._buildTool.getAdvancedMode())
		{
			local item;

			foreach( item in this.mSet )
			{
				local newscale = item.initialXform.scale.x + (this.mCursorStartY - evt.y) * 0.025;
				newscale = newscale > 0.25 ? newscale : 0.25;
				newscale = newscale < 10 ? newscale : 10;
				item.node.setScale(this.Vector3(newscale, newscale, newscale));
			}

			this._buildTool.setStatusText("Scaling...");
		}
		else
		{
			local item;

			foreach( item in this.mSet )
			{
				local newpos = item.initialXform.position + dir + this.mDragOffset;
				local oldPos = item.node.getPosition();

				if (this.mSnapToFloor)
				{
					local floor = this.Util.pointOnFloor(newpos, item.node);
					newpos.y = floor.y + item.floorOffset;
				}
				else if (this.mSet.len() > 1)
				{
					newpos.y = oldPos.y;
				}
				else
				{
					newpos.y = this.mStartPos.y;
				}

				if (this.mSnapToGrid)
				{
					local increment = this.gBuildTranslateSnap;
					newpos.x = (newpos.x / increment).tointeger() * increment;
					newpos.z = (newpos.z / increment).tointeger() * increment;

					if (!this.mSnapToFloor)
					{
						newpos.y = (newpos.y / increment).tointeger() * increment;
					}
				}

				item.node.setPosition(newpos);
			}

			this._buildTool._refreshSpawnersPos();
		}

		this.mIsModified = true;
	}

	function _selectedObjects()
	{
		return this._buildTool.getSelectedObjects();
	}

	function _tryLocalEdit()
	{
		local localEdit = false;

		if (!this.mStartPos)
		{
			if (!this._selectedObjects())
			{
				return null;
			}

			localEdit = this._beginMove(false);
		}

		return localEdit;
	}

	function _getFloorOffset( node )
	{
		local pos = node.getPosition();
		local floor = this.Util.pointOnFloor(pos, node);
		return pos.y - floor.y;
	}

	function _adjustFloorOffset( amount, relative )
	{
		local localEdit = this._tryLocalEdit();

		if (localEdit == null)
		{
			return;
		}

		if (relative && this.Key.isDown(this.Key.VK_CONTROL))
		{
			amount /= 5.0;
		}

		if (this.mSet == null)
		{
			return;
		}

		local item;

		foreach( item in this.mSet )
		{
			local floor = this.Util.pointOnFloor(item.node.getPosition(), item.node);

			if (relative)
			{
				item.floorOffset += amount;
				local pos = item.node.getPosition();
				pos.y += amount;
				item.node.setPosition(pos);
			}
			else
			{
				item.floorOffset = amount;
				local pos = floor;
				pos.y += amount;
				item.node.setPosition(pos);
			}
		}

		this.mIsModified = true;

		if (localEdit)
		{
			this._end();
		}
	}

	function _modified()
	{
		this.mIsModified = true;
	}

	function increaseFloorOffset()
	{
		this._adjustFloorOffset(1, true);
	}

	function decreaseFloorOffset()
	{
		this._adjustFloorOffset(-1, true);
	}

	function resetFloorOffset()
	{
		this._adjustFloorOffset(0, false);
	}

	function _beginMove( forClone )
	{
		local objects = this._selectedObjects();

		if (!objects || objects.len() == 0)
		{
			return false;
		}

		this.mUpdateSet = [];
		local so;

		foreach( so in objects )
		{
			if (!forClone && so.isLocked())
			{
				continue;
			}

			this.mUpdateSet.append({
				so = so,
				node = so.getNode(),
				floorOffset = this._getFloorOffset(so.getNode()),
				initialXform = this.Util.getNodeXform(so.getNode()),
				asset = so.getTypeString(),
				previewSO = so
			});
		}

		if (this.mUpdateSet.len() == 0)
		{
			this._buildTool.setStatusText("Cannot move or rotate locked object(s).");
			this.mUpdateSet = null;
			return false;
		}

		local what;

		if (forClone)
		{
			this._updatePreview(true);
			what = "Cloning";
			this.mSet = this.mPreviewSet;
		}
		else
		{
			what = "Moving";
			this.mSet = this.mUpdateSet;
		}

		this._buildTool.setStatusText(what + "... (Alt+Drag = rotate)");
		return true;
	}

	function _beginInsert( assetName )
	{
		this.onCancel(0);
		this.mStartPos = this.pickFloorPoint(this.Screen.getWidth() / 2, this.Screen.getHeight() / 2);

		if (this.mStartPos == null)
		{
			this.mStartPos = this._buildTool.getBuildNodePosition();
		}

		this.mDragOffset = this.Vector3();

		if (::_buildTool.getSceneryTool().getSnapToGrid())
		{
			this.mStartPos.x = (this.mStartPos.x / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
			this.mStartPos.y = (this.mStartPos.y / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
			this.mStartPos.z = (this.mStartPos.z / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
		}

		local so = this.SceneObject("", "Scenery");
		so.setTypeFromString(assetName);
		so.setPosition(this.mStartPos);
		so.reassemble();
		this.mPreviewSet = [
			{
				soObjectClass = "Scenery",
				initialXform = {
					position = this.mStartPos,
					orientation = this.Quaternion(),
					scale = this.Vector3(1, 1, 1)
				},
				floorOffset = 0.0,
				asset = assetName,
				node = so.getNode(),
				previewSO = so
			}
		];
		this.mSet = this.mPreviewSet;
		this.Util.setNodeOpacity(so.getNode(), 0.40000001);
		this._buildTool.setStatusText("Inserting... (Alt+Drag = rotate)");
	}

	function _beginTemplateInsert( objectArray )
	{
		this.onCancel(0);
		this.mStartPos = this.pickFloorPoint(this.Screen.getWidth() / 2, this.Screen.getHeight() / 2);

		if (this.mStartPos == null)
		{
			this.mStartPos = this._buildTool.getBuildNodePosition();
		}

		this.mStartPos = this._buildTool.getBuildNodePosition();

		if (::_buildTool.getSceneryTool().getSnapToGrid())
		{
			this.mStartPos.x = (this.mStartPos.x / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
			this.mStartPos.y = (this.mStartPos.y / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
			this.mStartPos.z = (this.mStartPos.z / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
		}

		this.mDragOffset = this.Vector3();
		local previewObjects = [];
		local idIncrement = 0;
		local first_item;

		foreach( item in objectArray )
		{
			local so = this.SceneObject("Build_Template_" + idIncrement, "Scenery");
			idIncrement = idIncrement + 1;
			so.setTypeFromString(item.asset);
			so.setPosition(this.mStartPos + item.position);
			so.setOrientation(item.orientation);
			so.setScale(item.scale);
			so.reassemble();
			local floorOffset = 0;

			if (!first_item)
			{
				first_item = item;
			}
			else
			{
				floorOffset = item.position.y - first_item.position.y;
			}

			local previewItem = {
				soObjectClass = "Scenery",
				initialXform = {
					position = this.mStartPos + item.position,
					orientation = item.orientation,
					scale = item.scale
				},
				floorOffset = floorOffset,
				asset = item.asset,
				node = so.getNode(),
				previewSO = so
			};
			this.Util.setNodeOpacity(so.getNode(), 0.40000001);
			previewObjects.append(previewItem);
		}

		this.mPreviewSet = previewObjects;
		this.mSet = this.mPreviewSet;
		this._buildTool.setStatusText("Inserting... (Alt+Drag = rotate)");
	}

	function _end()
	{
		if (this.mIsModified)
		{
			local args = [];

			foreach( so in this._selectedObjects() )
			{
				so.reassemble();
			}

			if (this.mPreviewSet != null)
			{
				local op = this.CompoundOperation();

				foreach( item in this.mPreviewSet )
				{
					if (item.soObjectClass == "Scenery")
					{
						if (item.asset == "Manipulator-SpawnPoint")
						{
							op.add(this.SpawnPointCreateOp(item.so.getID(), this.Util.getNodeXform(item.node)));
						}
						else if (item.asset == "Manipulator-PathNode" && "so" in item)
						{
							op.add(this.PathNodeCreateOp(item.so.getID(), this.Util.getNodeXform(item.node)));
						}
						else
						{
							op.add(this.SceneryCreateOp(this.AssetReference(item.asset), this.Util.getNodeXform(item.node)));
						}
					}
					else if (item.soObjectClass == "Creature")
					{
						op.add(this.CreatureCreateOp(item.previewSO.getType(), this.Util.getNodeXform(item.node)));
					}
					else
					{
						this.log.info("TODO: Create Op: " + item.soObjectClass);
					}
				}

				if (op.len() > 1)
				{
					op.setPresentationName("Create " + op.len() + " objects");
				}

				::_buildTool.selectionClear();
				this._opHistory.execute(op);
			}
			else
			{
				local op = this.CompoundOperation();
				local item;

				foreach( item in this.mUpdateSet )
				{
					if (this.mSnapToGrid)
					{
						local node = item.so.getNode();
						local p = node.getPosition();
						p.x = (p.x / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
						p.z = (p.z / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;

						if (!this.mSnapToFloor)
						{
							p.y = (p.y / this.gBuildTranslateSnap).tointeger() * this.gBuildTranslateSnap;
						}

						node.setPosition(p);
					}

					if (item.so.isScenery())
					{
						op.add(this.SceneryUpdateOp(item.so, item.initialXform));
					}
					else if (item.so.isCreature())
					{
						op.add(this.CreatureMoveOp(item.so, item.initialXform));
					}
					else
					{
						this.log.info("TODO: Update Op: " + item.so);
					}
				}

				if (op.len() > 1)
				{
					op.setPresentationName("Update " + op.len() + " objects");
				}

				this._opHistory.execute(op);
			}
		}

		this.mIsModified = false;
		this.mUpdateSet = null;

		if (this.mPreviewSet)
		{
			this._updatePreview(false);
		}

		this.mSet = null;
		this.mStartPos = null;
	}

	mIsModified = false;
	mStartPos = null;
	mDragOffset = null;
	mSet = null;
	mUpdateSet = null;
	mPreviewSet = null;
	mCursorStartX = 0;
	mCursorStartY = 0;
	static mDefaultStatus = "Drag=move|Alt+Drag=rotate|Shift+Drag=scale|Ctrl+Drag=clone|Ctrl+Shift=sticky move";
	mSnapToGrid = false;
	mSnapToFloor = true;
	mRotationNode = null;
}

this.require("UI/TerrainPainting");
class this.TerrainPaintTool extends this.Tool
{
	mPalette = null;
	mCursorNode = null;
	mBrushRadius = 50.0;
	mBrushWeight = 1.0;
	mBrushFalloff = 1.0;
	mDecal = null;
	mLastPos = null;
	mDefaultStatus = "Click to paint.";
	mHandler = null;
	constructor()
	{
		this.Tool.constructor("TerrainPaintTool");
		this.mKeyBindings[this.KB(this.Key.VK_LBRACKET)] <- "decreaseBrushWeight";
		this.mKeyBindings[this.KB(this.Key.VK_RBRACKET)] <- "increaseBrushWeight";
		this.mKeyBindings[this.KB(this.Key.VK_H)] <- "terrainShadow";
	}

	function activate()
	{
		this.mCursorNode = this._scene.getRootSceneNode().createChildSceneNode();
		this.mCursorNode.setPosition(this._buildTool.getBuildNodePosition());
		this.setBrushVisible(true);
		this.setStatus(this.mDefaultStatus);
	}

	function deactivate()
	{
		this._scene.setTerrainGridVisible(false);
		this._scene.setTerrainLODEnabled(true);
		this.mCursorNode.destroy();
		this.mCursorNode = null;
		this.mDecal = null;
	}

	function setStatus( ... )
	{
		this._root.setMaterialFragmentProgramConstant("Terrain/Brush", "Opacity", this.mBrushWeight);
		this._root.setMaterialFragmentProgramConstant("Terrain/Brush", "Falloff", this.mBrushFalloff);
		local status = "Weight: " + this.mBrushWeight;
		status += ", Falloff: " + this.mBrushFalloff;

		if (::_buildTool.getDirtyParts() != null)
		{
			status += " (UNSAVED)";
		}

		local bs = this.Screens.get("BuildScreen", true);
		local text = vargc == 0 ? null : vargv[0];
		bs.setStatusText(text, status);
	}

	function onTerrainSaved()
	{
		this.setStatus();
	}

	function getPaintHandler()
	{
		return this.mHandler;
	}

	function setPaintHandler( handler )
	{
		if (this.mHandler)
		{
			this.mHandler.onDeactivated();
		}

		this.mHandler = handler;

		if (this.mHandler)
		{
			this.mHandler.onActivated();
		}
	}

	function setBrushRadius( radius )
	{
		this.mBrushRadius = this.Math.clamp(radius, 1.0, 500.0);

		if (this.mDecal)
		{
			this.mDecal.setSize(this.mBrushRadius * 2, this.mBrushRadius * 2);
		}

		this.setStatus();
	}

	function setBrushWeight( weight )
	{
		this.mBrushWeight = this.Math.clamp(weight, 0.1, 1.0);
		this.setStatus();
	}

	function decreaseBrushWeight()
	{
		this.setBrushWeight(this.mBrushWeight - 0.1);
	}

	function increaseBrushWeight()
	{
		this.setBrushWeight(this.mBrushWeight + 0.1);
	}

	function setBrushFalloff( falloff )
	{
		this.mBrushFalloff = this.Math.clamp(falloff, 1.0, 10.0);
		this.setStatus();
	}

	function decreaseBrushFalloff()
	{
		this.setBrushFalloff(this.mBrushFalloff - 1);
	}

	function increaseBrushFalloff()
	{
		this.setBrushFalloff(this.mBrushFalloff + 1);
	}

	function setBrushVisible( value )
	{
		if (value && !this.mDecal)
		{
			this.mDecal = this._scene.createDecal(this.mCursorNode + "/BrushDecal", "Terrain/Brush", this.mBrushRadius * 2, this.mBrushRadius * 2);
			this.mDecal.setRenderQueueGroup(35);
			this.mDecal.setVisibilityFlags(this.VisibilityFlags.FEEDBACK | this.VisibilityFlags.ANY | this.VisibilityFlags.SCENERY);
			this.mCursorNode.attachObject(this.mDecal);
		}
		else if (!value && this.mDecal)
		{
			this.mDecal.destroy();
			this.mDecal = null;
		}
	}

	function isBrushVisible()
	{
		return this.mDecal != null;
	}

	function hasUnsavedTerrainData()
	{
		return ::_buildTool.getDirtyParts() ? true : false;
	}

	function terrainShadow()
	{
		local apos = ::_buildTool.getBuildNodePosition();
		local tpos = this.Util.getTerrainPageIndex(apos);
		local or = this._Environment.getSunDirection();
		this._root.terrainShadow(or, tpos.x, tpos.z, this.QueryFlags.LIGHT_OCCLUDER);
		local offsets_x = [
			0,
			1,
			0,
			1,
			-1,
			1,
			0,
			-1,
			-1
		];
		local offsets_z = [
			0,
			1,
			1,
			0,
			1,
			-1,
			-1,
			0,
			-1
		];

		for( local s = 0; s < 9; s++ )
		{
			local x = tpos.x + offsets_x[s];
			local z = tpos.z + offsets_z[s];

			if (x >= 0 && z >= 0)
			{
				::_buildTool.markPageDirty("Coverage", x, z);
			}
		}

		this.setStatus();
	}

	function onMouseWheel( evt )
	{
		if (evt.units_v == 0)
		{
			return;
		}

		if (evt.isAltDown() && evt.isShiftDown())
		{
			this.setBrushFalloff(this.mBrushFalloff + evt.units_v);
			evt.consume();
		}
		else if (evt.isAltDown())
		{
			this.setBrushWeight(this.mBrushWeight + evt.units_v * 0.1);
			evt.consume();
		}
		else if (evt.isShiftDown())
		{
			this.setBrushRadius(this.mBrushRadius + evt.units_v * 10);
			evt.consume();
		}
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.isLButtonDown())
		{
			evt.consume();
		}

		local pos = this.pickTerrainPoint(evt.x, evt.y);

		if (pos == null)
		{
			return;
		}

		if (evt.isLButtonDown())
		{
			::_buildTool.mCameraRayTestEnabled = false;
			this.mLastPos = pos;
			this.onTerrainPaintStart();
			this.onTerrainPaint(evt);
		}
	}

	function onMouseReleased( evt )
	{
		if (this.mLastPos != null)
		{
			this.onTerrainPaintEnd();
			this.mLastPos = null;
		}

		::_buildTool.mCameraRayTestEnabled = true;
		::_buildTool.pan(0, 0);
	}

	function onMouseMoved( evt )
	{
		local pos = this.pickTerrainPoint(evt.x, evt.y);

		if (pos == null)
		{
			return;
		}

		if (evt.isLButtonDown())
		{
			this.mLastPos = this.mCursorNode.getPosition();
		}

		this.mCursorNode.setPosition(pos);

		if (evt.isLButtonDown())
		{
			this.onTerrainPaint(evt);
		}
	}

	function onTerrainPaintStart()
	{
		this.setStatus("Painting...");

		if (this.mHandler)
		{
			this.mHandler.onTerrainPaintStart(this);
		}
	}

	function onTerrainPaintEnd()
	{
		if (this.mHandler)
		{
			this.mHandler.onTerrainPaintEnd(this);
		}

		this.setStatus(this.mDefaultStatus);
		::_scene.resetClutter();
	}

	function onTerrainPaint( evt )
	{
		if (this.mHandler)
		{
			this.mHandler.onTerrainPaint(this, evt);
		}
	}

}

class this.GroundTargetTool extends this.Tool
{
	mAbility = null;
	mActivate = true;
	mFlags = 0;
	mProjector = null;
	mProjectorNode = null;
	mAlreadyInUse = false;
	constructor()
	{
		this.Tool.constructor("GroundTarget");
		this.mProjector = this._scene.createTextureProjector("GroundTarget/Projector", "SelectionRing.png");
		this.mProjector.setNearClipDistance(0.1);
		this.mProjector.setFarClipDistance(160);
		this.mProjector.setOrthoWindow(20, 20);
		this.mProjector.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
		this.mProjector.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
		this.mProjector.setAlphaBlended(true);
		local root_node = ::_scene.getRootSceneNode();
		this.mProjectorNode = root_node.createChildSceneNode();
		this.mProjectorNode.attachObject(this.mProjector);
	}

	function destroy()
	{
		this._scene.getRootSceneNode().removeChild(this.mProjectorNode);
		this.mProjectorNode.detachObject(this.mProjector);
		this.mProjector.destroy();
		this.mProjectorNode.destroy();
	}

	function setSize( xSize, ySize )
	{
		this.mProjector.setOrthoWindow(xSize, ySize);
	}

	function setAbility( ab )
	{
		this.mAbility = ab;
		this._enable();
	}

	function onCancel( reason )
	{
		this.mAbility.sendActivationTargeted(false);
		this._disable();
	}

	function onMousePressed( evt )
	{
		if (evt.button == 1)
		{
			evt.consume();
		}
	}

	function onMouseMoved( evt )
	{
		local floor = this.pickTerrainPoint(evt.x, evt.y, this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);

		if (floor != null)
		{
			this.mProjector.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
			floor.y += 10;
			this.mProjectorNode.setPosition(floor);
			floor.y = 0;
			this.mProjectorNode.lookAt(floor);
		}
		else
		{
			this.mProjector.setProjectionQueryMask(0);
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == 1)
		{
			local floor = this.pickTerrainPoint(evt.x, evt.y, this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);

			if (floor != null)
			{
				this.mAbility.sendActivationTargeted(this.mActivate, this.mFlags, floor);
				this._disable();
			}

			evt.consume();
		}
	}

	function inUse()
	{
		return this.mAlreadyInUse;
	}

	function _enable()
	{
		this.mAlreadyInUse = true;
		::_cursor.setState(this.GUI.Cursor.SELECTGROUND);
	}

	function _disable()
	{
		this.mAlreadyInUse = false;
		this.mProjector.setProjectionQueryMask(0);
		this._popMe();
		::_cursor.setState(this.GUI.Cursor.DEFAULT);
	}

}

::_playTool <- null;
::_buildTool <- null;
::_groundTargetTool <- null;
::_tools <- this.ToolManager();
::gToolMode <- null;
