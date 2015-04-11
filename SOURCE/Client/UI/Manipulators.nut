this.require("Constants");
class this.ManipulatorManager 
{
	mManipulators = [];
	mActiveManipulator = null;
	mHoverManipulator = null;
	constructor()
	{
		::_root.addListener(this);
		this._enterFrameRelay.addListener(this);
	}

	function onDestroy()
	{
		this._enterFrameRelay.removeListener(this);
		::_root.removeListener(this);

		if (this.mManipulators.len() > 0)
		{
			this.log.error(this.mManipulators.len().tostring() + " manipulator(s) remain in ManipulatorManager at destruction.  All manipulators should be destroyed beforehand.");
		}
	}

	function addManipulator( vManipulator )
	{
		this.mManipulators.append(vManipulator);
	}

	function removeManipulator( vManipulator )
	{
		local i = 0;

		while (i < this.mManipulators.len())
		{
			if (this.mManipulators[i] == vManipulator)
			{
				this.mManipulators[i].onDestroy();
				this.mManipulators[i] = null;
				this.mManipulators.remove(i);
				break;
			}
		}

		if (vManipulator == this.mActiveManipulator)
		{
			this.mActiveManipulator = null;
		}

		if (vManipulator == this.mHoverManipulator)
		{
			this.mHoverManipulator = null;
		}
	}

	function removeAllManipulators()
	{
		while (this.mManipulators.len() > 0)
		{
			this.mManipulators[0].onDestroy();
			this.mManipulators[0] = null;
			this.mManipulators.remove(0);
		}

		this.mActiveManipulator = null;
		this.mHoverManipulator = null;
	}

	function findManipulator( x, y )
	{
		local ray = this.Screen.getViewportRay(x, y);
		local hits = this._scene.rayQuery(ray.origin, ray.dir, this.QueryFlags.MANIPULATOR, true, false);
		local nearest_obj;
		local nearest_dist = 100000;

		foreach( h in hits )
		{
			if (h.object != 0 && h.t < nearest_dist)
			{
				nearest_obj = h.object;
				nearest_dist = h.t;
			}
		}

		if (nearest_obj != null)
		{
			foreach( m in this.mManipulators )
			{
				local entities = m.getEntities();

				foreach( e in entities )
				{
					if (e == nearest_obj)
					{
						return m;
					}
				}
			}
		}

		return null;
	}

	function _canManipulate()
	{
		if (!("_tools" in this.getroottable()))
		{
			return false;
		}

		local tool = ::_tools.findFunction("getOrbiting");

		if (tool && tool.getOrbiting())
		{
			return false;
		}

		return true;
	}

	function onMousePressed( vEvent )
	{
		if (!this._canManipulate())
		{
			return;
		}

		local cur = this.findManipulator(vEvent.x, vEvent.y);

		if (cur)
		{
			cur.onMousePressed(vEvent);

			if (vEvent.isConsumed())
			{
				this.mActiveManipulator = cur;
			}
		}
	}

	function onMouseReleased( vEvent )
	{
		if (!this._canManipulate())
		{
			return;
		}

		if (this.mActiveManipulator)
		{
			this.mActiveManipulator.onMouseReleased(vEvent);
			this.mActiveManipulator = null;
		}

		local cur = this.findManipulator(vEvent.x, vEvent.y);

		if (this.mHoverManipulator && cur != this.mHoverManipulator)
		{
			this.mHoverManipulator.onMouseExited(vEvent);
			this.mHoverManipulator = null;
		}
	}

	function onMouseMoved( vEvent )
	{
		if (!this._canManipulate())
		{
			return;
		}

		local cur = this.findManipulator(vEvent.x, vEvent.y);

		if (this.mActiveManipulator == null && this.mHoverManipulator && cur != this.mHoverManipulator)
		{
			this.mHoverManipulator.onMouseExited(vEvent);
			this.mHoverManipulator = null;
		}

		if (this.mActiveManipulator == null && cur && cur != this.mHoverManipulator)
		{
			this.mHoverManipulator = cur;
			this.mHoverManipulator.onMouseEntered(vEvent);
		}

		if (this.mActiveManipulator)
		{
			this.mActiveManipulator.onMouseMoved(vEvent);
		}
	}

	function onMouseScrollWheel( vEvent )
	{
		local cur = this.findManipulator(vEvent.x, vEvent.y);

		if (cur)
		{
			cur.onMouseScrollWheel(vEvent);
		}
	}

	function onEnterFrame()
	{
		local delta = ::_deltat / 1000.0;
		local alphachanged = false;

		foreach( m in this.mManipulators )
		{
			m.onEnterFrame();
		}
	}

}

class this.Manipulator 
{
	mHandles = null;
	constructor()
	{
	}

	function onDestroy()
	{
	}

	function getEntities()
	{
		return [];
	}

	function onMousePressed( vEvent )
	{
	}

	function onMouseReleased( vEvent )
	{
	}

	function onMouseMoved( vEvent )
	{
	}

	function onMouseScrollWheel( vEvent )
	{
	}

	function onMouseEntered( vEvent )
	{
	}

	function onMouseExited( vEvent )
	{
	}

	function onEnterFrame()
	{
	}

	function getHandles()
	{
		return this.mHandles;
	}

}

class this.ObjectManipulator extends this.Manipulator
{
	mTargetObject = null;
	mTargetObjectBackup = null;
	mTargetObjectChanged = false;
	mDefaultEmissiveColor = this.Color(0.5, 0.5, 0.5, 1);
	mSelectedEmissiveColor = this.Color(0.2, 0.2, 1, 1);
	mDefaultDiffuseColor = this.Color(1, 1, 1, 1);
	mSelectedDiffuseColor = this.Color(0, 0, 1, 1);
	mUniversalSize = 0.0099999998;
	mManipulatorNode = null;
	constructor( vTargetObject )
	{
		this.Manipulator.constructor();
		this.setTargetObject(vTargetObject);
	}

	function onDestroy()
	{
	}

	function setTargetObject( vTargetObject )
	{
		this.mTargetObject = vTargetObject;
		this._updateTargetObjectBackup();
	}

	function getTargetObject()
	{
		return this.mTargetObject;
	}

	function _updateTargetObject()
	{
		if (this.mTargetObjectChanged)
		{
			local op = this.CompoundOperation();

			if (this.mTargetObject.isScenery())
			{
				op.add(this.SceneryUpdateOp(this.mTargetObject, this.mTargetObjectBackup));
			}
			else if (this.mTargetObject.isCreature())
			{
				op.add(this.CreatureMoveOp(this.mTargetObject, this.mTargetObjectBackup));
			}

			op.setPresentationName("Update " + op.len() + " objects");
			this._opHistory.execute(op);
			this._updateTargetObjectBackup();
			this.mTargetObjectChanged = false;
		}
	}

	function _updateTargetObjectBackup()
	{
		this.mTargetObjectBackup = this.Util.getNodeXform(this.mTargetObject.getNode());
	}

	function _updateManipulatorNode()
	{
		if (this.mTargetObject == null)
		{
			return;
		}

		local node = this.mTargetObject.getNode();

		if (node == null)
		{
			return;
		}

		local worldpos = node.getWorldPosition();
		this.mManipulatorNode.setPosition(worldpos);
		local camera = ::_scene.getCamera("Default");
		local camerapos = camera.getParentSceneNode().getWorldPosition();
		local dist = (camerapos - worldpos).length();
		local s = dist * this.mUniversalSize;
		this.mManipulatorNode.setScale(this.Vector3(s, s, s));
	}

}

class this.Plane 
{
	mNormal = null;
	mD = 0;
	constructor( vNormal, vPosition )
	{
		this.mNormal = vNormal;
		this.mD = -this.mNormal.dot(vPosition);
	}

	function projectRay( vRay )
	{
		local t = -(vRay.origin.dot(this.mNormal) + this.mD) / vRay.dir.dot(this.mNormal);
		return vRay.origin + this.Vector3(vRay.dir.x * t, vRay.dir.y * t, vRay.dir.z * t);
	}

}

class this.ObjectTranslateAxisManipulator extends this.ObjectManipulator
{
	mDragging = false;
	mDragOffset = null;
	mOccilation = 0;
	mScale = 0.5;
	mPlane = null;
	mSpread = 9;
	mAxis = "x";
	mAnimated = false;
	mNode1 = null;
	mNode2 = null;
	mEntity1 = null;
	mEntity2 = null;
	mCloning = false;
	constructor( vTargetObject, vAxis )
	{
		this.mAxis = vAxis;
		this.ObjectManipulator.constructor(vTargetObject);
	}

	function onDestroy()
	{
		this._destroyEntity();
	}

	function _destroyEntity()
	{
		if (this.mManipulatorNode != null)
		{
			this.mManipulatorNode.destroy();
			this.mManipulatorNode = null;
		}
	}

	function _createEntity()
	{
		this.mManipulatorNode = this._scene.getRootSceneNode().createChildSceneNode();
		this._updateManipulatorNode();
		this.mNode1 = this.mManipulatorNode.createChildSceneNode();

		switch(this.mAxis)
		{
		case "x":
			this.mNode1.translate(this.Vector3(this.mSpread, 0, 0));
			this.mNode1.rotate(this.Vector3(0, 0, 1), this.PI * 1.5);
			break;

		case "y":
			this.mNode1.translate(this.Vector3(0, this.mSpread, 0));
			this.mNode1.rotate(this.Vector3(1, 0, 0), 0);
			break;

		case "z":
			this.mNode1.translate(this.Vector3(0, 0, this.mSpread));
			this.mNode1.rotate(this.Vector3(1, 0, 0), this.PI * 0.5);
			break;
		}

		this.mNode1.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
		this.mEntity1 = this._scene.createEntity("Translate_Arrow_" + this.mAxis + "1", "Manipulator-Movement_Arrow.mesh");
		this.mEntity1.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
		this.mEntity1.setQueryFlags(this.QueryFlags.MANIPULATOR);
		this.mNode1.attachObject(this.mEntity1);
		this.mNode2 = this.mManipulatorNode.createChildSceneNode();

		switch(this.mAxis)
		{
		case "x":
			this.mNode2.translate(this.Vector3(-this.mSpread, 0, 0));
			this.mNode2.rotate(this.Vector3(0, 0, 1), this.PI * 0.5);
			break;

		case "y":
			this.mNode2.translate(this.Vector3(0, -this.mSpread, 0));
			this.mNode2.rotate(this.Vector3(1, 0, 0), this.PI);
			break;

		case "z":
			this.mNode2.translate(this.Vector3(0, 0, -this.mSpread));
			this.mNode2.rotate(this.Vector3(1, 0, 0), this.PI * 1.5);
			break;
		}

		this.mNode2.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
		this.mEntity2 = this._scene.createEntity("Translate_Arrow_" + this.mAxis + "2", "Manipulator-Movement_Arrow.mesh");
		this.mEntity2.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
		this.mEntity2.setQueryFlags(this.QueryFlags.MANIPULATOR);
		this.mNode2.attachObject(this.mEntity2);
		this.mEntity1.setDiffuse(this.mDefaultDiffuseColor);
		this.mEntity2.setDiffuse(this.mDefaultDiffuseColor);
		this.mEntity1.setEmissive(this.mDefaultEmissiveColor);
		this.mEntity2.setEmissive(this.mDefaultEmissiveColor);
	}

	function getEntities()
	{
		return [
			this.mEntity1,
			this.mEntity2
		];
	}

	function setTargetObject( vTargetObject )
	{
		this.ObjectManipulator.setTargetObject(vTargetObject);

		if (vTargetObject != null && this.mEntity1 == null)
		{
			this._createEntity();
		}
	}

	function setAnimated( val )
	{
		this.mAnimated = val;

		if (this.mAnimated == false)
		{
			switch(this.mAxis)
			{
			case "x":
				this.mNode2.setPosition(this.Vector3(-this.mSpread, 0, 0));
				this.mNode1.setPosition(this.Vector3(this.mSpread, 0, 0));
				break;

			case "y":
				this.mNode1.setPosition(this.Vector3(0, this.mSpread, 0));
				this.mNode2.setPosition(this.Vector3(0, -this.mSpread, 0));
				break;

			case "z":
				this.mNode1.setPosition(this.Vector3(0, 0, this.mSpread));
				this.mNode2.setPosition(this.Vector3(0, 0, -this.mSpread));
				break;
			}

			this.mNode1.setScale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mNode2.setScale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mEntity1.setEmissive(this.mDefaultEmissiveColor);
			this.mEntity2.setEmissive(this.mDefaultEmissiveColor);
			this.mEntity1.setDiffuse(this.mDefaultDiffuseColor);
			this.mEntity2.setDiffuse(this.mDefaultDiffuseColor);
		}
	}

	function onMousePressed( vEvent )
	{
		if (vEvent.button == 1)
		{
			this.mDragging = true;
			local camera = ::_scene.getCamera("Default");
			local camerapos = camera.getParentSceneNode().getWorldPosition();
			local objnormal = this.mTargetObject.getPosition() - camerapos;
			objnormal.normalize();
			this.log.debug("Camera normal is: " + objnormal);
			local normal1;
			local normal2;

			switch(this.mAxis)
			{
			case "x":
				normal1 = this.Vector3(0, 1, 0);
				normal2 = this.Vector3(0, 0, 1);
				break;

			case "y":
				normal1 = this.Vector3(1, 0, 0);
				normal2 = this.Vector3(0, 0, 1);
				break;

			case "z":
				normal1 = this.Vector3(1, 0, 0);
				normal2 = this.Vector3(0, 1, 0);
				break;
			}

			local plane;
			local dot1 = objnormal.dot(normal1);
			local dot2 = objnormal.dot(normal2);
			this.log.debug("Dot1 = " + this.abs(dot1));
			this.log.debug("Dot2 = " + this.abs(dot2));

			if (this.fabs(dot1) > this.fabs(dot2))
			{
				this.log.debug("Dot1 > Dot2");
				this.mPlane = this.Plane(normal1, this.mTargetObject.getPosition());
				this.log.debug("Using plane aligned to: " + normal1);
			}
			else
			{
				this.log.debug("Dot1 < Dot2");
				this.mPlane = this.Plane(normal2, this.mTargetObject.getPosition());
				this.log.debug("Using plane aligned to: " + normal2);
			}

			local ray = this.Screen.getViewportRay(vEvent.x, vEvent.y);
			local final = this.mPlane.projectRay(ray);
			local pos = this.mTargetObject.getPosition();

			if (vEvent.isControlDown())
			{
				this.mCloning = this._buildTool.getSceneryTool()._beginMove(true);
			}

			this.mDragOffset = this.Vector3(0, 0, 0);

			switch(this.mAxis)
			{
			case "x":
				this.mDragOffset.x = pos.x - final.x;
				break;

			case "y":
				this.mDragOffset.y = pos.y - final.y;
				break;

			case "z":
				this.mDragOffset.z = pos.z - final.z;
				break;
			}

			vEvent.consume();
			::Screen.setMouseCapture(::_manipulators);
		}
	}

	function onMouseReleased( vEvent )
	{
		if (this.mDragging)
		{
			if (this.mCloning)
			{
				this._buildTool.getSceneryTool()._end();
			}

			this._updateTargetObject();
			::Screen.setMouseCapture(null);
			this.mDragging = false;
			this.mCloning = false;
			vEvent.consume();
		}
	}

	function onMouseMoved( vEvent )
	{
		if (this.mDragging)
		{
			if (this.mCloning)
			{
				this._buildTool.getSceneryTool()._modified();
			}

			local ray = this.Screen.getViewportRay(vEvent.x, vEvent.y);
			local final = this.mPlane.projectRay(ray);
			local increment = this.gBuildTranslateSnap;
			local pos = this.mTargetObject.getPosition();
			local snapToGrid = ::_buildTool.getSceneryTool().getSnapToGrid();

			switch(this.mAxis)
			{
			case "x":
				if (snapToGrid)
				{
					local total = this.mDragOffset.x + final.x - pos.x;

					if (total >= 5 || total <= -5)
					{
						pos.x += total;
					}
					else
					{
						vEvent.consume();
						return;
					}

					pos.x = (pos.x / increment).tointeger() * increment;
				}
				else
				{
					pos.x = final.x;
				}

				break;

			case "y":
				if (snapToGrid)
				{
					local total = this.mDragOffset.y + final.y - pos.y;

					if (total >= 5 || total <= -5)
					{
						pos.y += total;
					}
					else
					{
						vEvent.consume();
						return;
					}

					pos.y = (pos.y / increment).tointeger() * increment;
				}
				else
				{
					pos.y = final.y;
				}

				break;

			case "z":
				if (snapToGrid)
				{
					local total = this.mDragOffset.z + final.z - pos.z;

					if (total >= 5 || total <= -5)
					{
						pos.z += total;
					}
					else
					{
						vEvent.consume();
						return;
					}

					pos.z = (pos.z / increment).tointeger() * increment;
				}
				else
				{
					pos.z = final.z;
				}

				break;
			}

			if (!snapToGrid)
			{
				pos = pos + this.mDragOffset;
			}

			this.mTargetObject.getNode().setPosition(pos);
			::_buildTool._refreshSpawnersPos();
			this.mTargetObjectChanged = true;
			vEvent.consume();
		}
	}

	function onMouseEntered( vEvent )
	{
		this.setAnimated(true);
	}

	function onMouseExited( vEvent )
	{
		this.setAnimated(false);
	}

	function onEnterFrame()
	{
		local delta = ::_deltat / 1000.0;
		this.mOccilation += delta;

		if (this.mAnimated)
		{
			switch(this.mAxis)
			{
			case "x":
				this.mNode1.setPosition(this.Vector3(this.mSpread + this.sin(this.mOccilation * 4), 0, 0));
				this.mNode2.setPosition(this.Vector3(-this.mSpread + this.sin(this.mOccilation * 4 + 0.5), 0, 0));
				break;

			case "y":
				this.mNode1.setPosition(this.Vector3(0, this.mSpread + this.sin(this.mOccilation * 4), 0));
				this.mNode2.setPosition(this.Vector3(0, -this.mSpread + this.sin(this.mOccilation * 4 + 0.5), 0));
				break;

			case "z":
				this.mNode1.setPosition(this.Vector3(0, 0, this.mSpread + this.sin(this.mOccilation * 4)));
				this.mNode2.setPosition(this.Vector3(0, 0, -this.mSpread + this.sin(this.mOccilation * 4 + 0.5)));
				break;
			}

			this.mNode1.setScale(this.Vector3(this.mScale * 1.5, this.mScale * 1.5, this.mScale * 1.5));
			this.mNode2.setScale(this.Vector3(this.mScale * 1.5, this.mScale * 1.5, this.mScale * 1.5));
			this.mEntity1.setEmissive(this.mSelectedEmissiveColor);
			this.mEntity2.setEmissive(this.mSelectedEmissiveColor);
			this.mEntity1.setDiffuse(this.mSelectedDiffuseColor);
			this.mEntity2.setDiffuse(this.mSelectedDiffuseColor);
		}

		if (this.mTargetObject && this.mTargetObject.getNode() == null)
		{
			this.mTargetObject = null;
			return;
		}

		this._updateManipulatorNode();
	}

}

class this.ObjectRotateManipulator extends this.ObjectManipulator
{
	mDragging = false;
	mAxis = "y";
	mFullRing = true;
	mDragOffset = null;
	mStartOrientation = null;
	mCursorDelta = null;
	mOccilation = 0;
	mScale = 0.5;
	mAnimated = false;
	mRotateNode1 = null;
	mRotateNode2 = null;
	mRotateNode3 = null;
	mRotateNode4 = null;
	mRotateEntity1 = null;
	mRotateEntity2 = null;
	mRotateEntity3 = null;
	mRotateEntity4 = null;
	constructor( vTargetObject, ... )
	{
		if (vargc > 0)
		{
			this.mAxis = vargv[0];
		}

		if (vargc > 1)
		{
			this.mFullRing = vargv[1];
		}

		this.ObjectManipulator.constructor(vTargetObject);
	}

	function onDestroy()
	{
		this._destroyEntity();
	}

	function _destroyEntity()
	{
		if (this.mManipulatorNode != null)
		{
			this.mManipulatorNode.destroy();
			this.mManipulatorNode = null;
		}
	}

	function _createEntity()
	{
		this.mManipulatorNode = this._scene.getRootSceneNode().createChildSceneNode();
		this._updateManipulatorNode();
		this.mManipulatorNode.setOrientation(this.Quaternion(0, 0, 0, 1));

		switch(this.mAxis)
		{
		case "x":
			this.mManipulatorNode.rotate(this.Vector3(0, 0, 1), this.PI * 0.5);
			break;

		case "y":
			break;

		case "z":
			this.mManipulatorNode.rotate(this.Vector3(1, 0, 0), -this.PI * 0.5);
			break;
		}

		this.mRotateNode1 = this.mManipulatorNode.createChildSceneNode();
		this.mRotateNode1.rotate(this.Vector3(0, 1, 0), this.PI * (1.0 / 4.0));
		this.mRotateNode1.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
		this.mRotateEntity1 = this._scene.createEntity("Rotate_Arrow_" + this.mAxis + "1", "Manipulator-Rotation_Arrow.mesh");
		this.mRotateEntity1.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
		this.mRotateEntity1.setQueryFlags(this.QueryFlags.MANIPULATOR);
		this.mRotateEntity1.setEmissive(this.mDefaultEmissiveColor);
		this.mRotateEntity1.setDiffuse(this.mDefaultDiffuseColor);
		this.mRotateNode1.attachObject(this.mRotateEntity1);

		if (this.mFullRing)
		{
			this.mRotateNode2 = this.mManipulatorNode.createChildSceneNode();
			this.mRotateNode2.rotate(this.Vector3(0, 1, 0), this.PI * (3.0 / 4.0));
			this.mRotateNode2.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mRotateEntity2 = this._scene.createEntity("Rotate_Arrow_" + this.mAxis + "2", "Manipulator-Rotation_Arrow.mesh");
			this.mRotateEntity2.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
			this.mRotateEntity2.setEmissive(this.mDefaultEmissiveColor);
			this.mRotateEntity2.setDiffuse(this.mDefaultDiffuseColor);
			this.mRotateEntity2.setQueryFlags(this.QueryFlags.MANIPULATOR);
			this.mRotateNode2.attachObject(this.mRotateEntity2);
			this.mRotateNode3 = this.mManipulatorNode.createChildSceneNode();
			this.mRotateNode3.rotate(this.Vector3(0, 1, 0), this.PI * (5.0 / 4.0));
			this.mRotateNode3.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mRotateEntity3 = this._scene.createEntity("Rotate_Arrow_" + this.mAxis + "3", "Manipulator-Rotation_Arrow.mesh");
			this.mRotateEntity3.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
			this.mRotateEntity3.setEmissive(this.mDefaultEmissiveColor);
			this.mRotateEntity3.setDiffuse(this.mDefaultDiffuseColor);
			this.mRotateEntity3.setQueryFlags(this.QueryFlags.MANIPULATOR);
			this.mRotateNode3.attachObject(this.mRotateEntity3);
			this.mRotateNode4 = this.mManipulatorNode.createChildSceneNode();
			this.mRotateNode4.rotate(this.Vector3(0, 1, 0), this.PI * (7.0 / 4.0));
			this.mRotateNode4.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mRotateEntity4 = this._scene.createEntity("Rotate_Arrow_" + this.mAxis + "4", "Manipulator-Rotation_Arrow.mesh");
			this.mRotateEntity4.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
			this.mRotateEntity4.setEmissive(this.mDefaultEmissiveColor);
			this.mRotateEntity4.setDiffuse(this.mDefaultDiffuseColor);
			this.mRotateEntity4.setQueryFlags(this.QueryFlags.MANIPULATOR);
			this.mRotateNode4.attachObject(this.mRotateEntity4);
		}
	}

	function getEntities()
	{
		if (this.mFullRing)
		{
			return [
				this.mRotateEntity1,
				this.mRotateEntity2,
				this.mRotateEntity3,
				this.mRotateEntity4
			];
		}
		else
		{
			return [
				this.mRotateEntity1
			];
		}
	}

	function setTargetObject( vTargetObject )
	{
		this.ObjectManipulator.setTargetObject(vTargetObject);

		if (vTargetObject != null)
		{
			this._createEntity();
		}
	}

	function setAnimated( val )
	{
		this.mAnimated = val;

		if (this.mAnimated == false)
		{
			this.mManipulatorNode.setOrientation(this.Quaternion(0, 0, 0, 1));

			switch(this.mAxis)
			{
			case "x":
				this.mManipulatorNode.rotate(this.Vector3(0, 0, 1), this.PI * 0.5);
				break;

			case "z":
				this.mManipulatorNode.rotate(this.Vector3(1, 0, 0), -this.PI * 0.5);
				break;
			}

			this.mRotateEntity1.setEmissive(this.mDefaultEmissiveColor);
			this.mRotateEntity1.setDiffuse(this.mDefaultDiffuseColor);

			if (this.mFullRing)
			{
				this.mRotateEntity2.setEmissive(this.mDefaultEmissiveColor);
				this.mRotateEntity3.setEmissive(this.mDefaultEmissiveColor);
				this.mRotateEntity4.setEmissive(this.mDefaultEmissiveColor);
				this.mRotateEntity2.setDiffuse(this.mDefaultDiffuseColor);
				this.mRotateEntity3.setDiffuse(this.mDefaultDiffuseColor);
				this.mRotateEntity4.setDiffuse(this.mDefaultDiffuseColor);
			}
		}
	}

	function onMousePressed( vEvent )
	{
		if (vEvent.button == 1)
		{
			this.Screen.setCursorVisible(false);
			::_cursor.setLocked(true);
			this.mDragOffset = {
				x = vEvent.x,
				y = vEvent.y
			};
			this.mDragging = true;
			this.mStartOrientation = this.mTargetObject.getNode().getOrientation();
			this.mCursorDelta = 0;
			::Screen.setMouseCapture(this);
			vEvent.consume();
		}
	}

	function onMouseReleased( vEvent )
	{
		if (this.mDragging)
		{
			this.Screen.setCursorVisible(true);
			::_cursor.setLocked(false);
			this._updateTargetObject();
			this.mDragging = false;
			::Screen.setMouseCapture(null);
			vEvent.consume();
		}
	}

	function onMouseMoved( vEvent )
	{
		if (this.mDragging)
		{
			local increment = this.gBuildRotateSnap;
			local angleInc = 0.0099999998;
			local snapToGrid = ::_buildTool.getSceneryTool().getSnapToGrid();
			this.mTargetObject.getNode().setOrientation(this.mStartOrientation);

			switch(this.mAxis)
			{
			case "x":
				this.mCursorDelta += this.mDragOffset.y - vEvent.y;
				local angle = this.mCursorDelta * angleInc;

				if (snapToGrid)
				{
					angle = (angle / increment).tointeger() * increment;
				}

				this.mTargetObject.getNode().rotate(this.Vector3().UNIT_X, angle);
				break;

			case "y":
				this.mCursorDelta += vEvent.x - this.mDragOffset.x;
				local angle = this.mCursorDelta * angleInc;

				if (snapToGrid)
				{
					angle = (angle / increment).tointeger() * increment;
				}

				this.mTargetObject.getNode().rotate(this.Vector3().UNIT_Y, angle);
				break;

			case "z":
				this.mCursorDelta += this.mDragOffset.y - vEvent.y;
				local angle = this.mCursorDelta * angleInc;

				if (snapToGrid)
				{
					angle = (angle / increment).tointeger() * increment;
				}

				this.mTargetObject.getNode().rotate(this.Vector3().UNIT_Z, angle);
				break;
			}

			this.mTargetObjectChanged = true;
			vEvent.consume();
		}
	}

	function onMouseEntered( vEvent )
	{
		this.setAnimated(true);
	}

	function onMouseExited( vEvent )
	{
		this.setAnimated(false);
	}

	function onEnterFrame()
	{
		local delta = ::_deltat / 1000.0;
		this.mOccilation += delta;

		if (this.mAnimated)
		{
			this.mManipulatorNode.setOrientation(this.Quaternion(0, 0, 0, 1));

			switch(this.mAxis)
			{
			case "x":
				this.mManipulatorNode.rotate(this.Vector3(0, 0, 1), this.PI * 0.5);
				break;

			case "z":
				this.mManipulatorNode.rotate(this.Vector3(1, 0, 0), -this.PI * 0.5);
				break;
			}

			this.mManipulatorNode.rotate(this.Vector3(0, 1, 0), this.sin(this.mOccilation * 4) * (this.PI * 0.050000001));
			this.mRotateEntity1.setEmissive(this.mSelectedEmissiveColor);
			this.mRotateEntity1.setDiffuse(this.mSelectedDiffuseColor);

			if (this.mFullRing)
			{
				this.mRotateEntity2.setEmissive(this.mSelectedEmissiveColor);
				this.mRotateEntity3.setEmissive(this.mSelectedEmissiveColor);
				this.mRotateEntity4.setEmissive(this.mSelectedEmissiveColor);
				this.mRotateEntity2.setDiffuse(this.mSelectedDiffuseColor);
				this.mRotateEntity3.setDiffuse(this.mSelectedDiffuseColor);
				this.mRotateEntity4.setDiffuse(this.mSelectedDiffuseColor);
			}
		}

		this._updateManipulatorNode();
	}

}

class this.ObjectScaleManipulator extends this.ObjectManipulator
{
	mDragging = false;
	mDragOffset = null;
	mScale = 0.5;
	mOccilation = 0;
	mSpread = 10;
	mAnimated = false;
	mScaleNode1 = null;
	mScaleEntity1 = null;
	mScaleNode2 = null;
	mScaleEntity2 = null;
	constructor( vTargetObject )
	{
		this.ObjectManipulator.constructor(vTargetObject);
	}

	function onDestroy()
	{
		this._destroyEntity();
	}

	function _destroyEntity()
	{
		if (this.mManipulatorNode != null)
		{
			this.mManipulatorNode.destroy();
			this.mManipulatorNode = null;
		}
	}

	function _createEntity()
	{
		this.mManipulatorNode = this._scene.getRootSceneNode().createChildSceneNode();
		this._updateManipulatorNode();
		this.mScaleNode1 = this.mManipulatorNode.createChildSceneNode();
		this.mScaleNode1.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
		this.mScaleNode1.setPosition(this.Vector3(-this.mSpread, 0, -this.mSpread));
		this.mScaleEntity1 = this._scene.createEntity("Scale_Sphere1", "Manipulator-Scale_Sphere.mesh");
		this.mScaleEntity1.setVisibilityFlags(this.VisibilityFlags.FEEDBACK);
		this.mScaleEntity1.setQueryFlags(this.QueryFlags.MANIPULATOR);
		this.mScaleNode1.attachObject(this.mScaleEntity1);
		this.mScaleNode2 = this.mManipulatorNode.createChildSceneNode();
		this.mScaleNode2.scale(this.Vector3(this.mScale, this.mScale, this.mScale));
		this.mScaleNode2.setPosition(this.Vector3(this.mSpread, 0, this.mSpread));
		this.mScaleEntity2 = this._scene.createEntity("Scale_Sphere2", "Manipulator-Scale_Sphere.mesh");
		this.mScaleEntity2.setQueryFlags(this.QueryFlags.MANIPULATOR);
		this.mScaleNode2.attachObject(this.mScaleEntity2);
	}

	function getEntities()
	{
		return [
			this.mScaleEntity1,
			this.mScaleEntity2
		];
	}

	function setTargetObject( vTargetObject )
	{
		this.ObjectManipulator.setTargetObject(vTargetObject);

		if (vTargetObject != null)
		{
			this._createEntity();
		}
	}

	function setAnimated( val )
	{
		this.mAnimated = val;

		if (this.mAnimated == false)
		{
			this.mScaleNode1.setPosition(this.Vector3(-this.mSpread, 0, -this.mSpread));
			this.mScaleNode1.setScale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mScaleEntity1.setEmissive(this.mDefaultEmissiveColor);
			this.mScaleEntity1.setDiffuse(this.mDefaultDiffuseColor);
			this.mScaleNode2.setPosition(this.Vector3(this.mSpread, 0, this.mSpread));
			this.mScaleNode2.setScale(this.Vector3(this.mScale, this.mScale, this.mScale));
			this.mScaleEntity2.setEmissive(this.mDefaultEmissiveColor);
			this.mScaleEntity2.setDiffuse(this.mDefaultDiffuseColor);
		}
	}

	function onMousePressed( vEvent )
	{
		if (vEvent.button == 1)
		{
			this.Screen.setCursorVisible(false);
			::_cursor.setLocked(true);
			this.mDragOffset = {
				x = vEvent.x,
				y = vEvent.y
			};
			this.mDragging = true;
			vEvent.consume();
		}
	}

	function onMouseReleased( vEvent )
	{
		if (this.mDragging)
		{
			this.Screen.setCursorVisible(true);
			::_cursor.setLocked(false);
			this._updateTargetObject();
			this.mDragging = false;
			vEvent.consume();
		}
	}

	function onMouseMoved( vEvent )
	{
		if (this.mDragging)
		{
			local diff = -(vEvent.y - this.mDragOffset.y) * 0.025;
			this.mDragOffset.y = vEvent.y;
			local s = this.mTargetObject.getScale().x;
			s += diff;
			s = s > 0.25 ? s : 0.25;
			s = s < 10 ? s : 10;
			this.mTargetObject.setScale(s);
			this.mTargetObjectChanged = true;
			vEvent.consume();
		}
	}

	function onMouseEntered( vEvent )
	{
		this.setAnimated(true);
	}

	function onMouseExited( vEvent )
	{
		this.setAnimated(false);
	}

	function onEnterFrame()
	{
		local delta = ::_deltat / 1000.0;
		this.mOccilation += delta;

		if (this.mAnimated)
		{
			local p = this.mSpread + this.sin(this.mOccilation * 4);
			this.mScaleNode1.setPosition(this.Vector3(p, 0, p));
			this.mScaleNode2.setPosition(this.Vector3(-p, 0, -p));
			this.mScaleNode1.setScale(this.Vector3(this.mScale * 1.5, this.mScale * 1.5, this.mScale * 1.5));
			this.mScaleNode2.setScale(this.Vector3(this.mScale * 1.5, this.mScale * 1.5, this.mScale * 1.5));
			this.mScaleEntity1.setEmissive(this.mSelectedEmissiveColor);
			this.mScaleEntity2.setEmissive(this.mSelectedEmissiveColor);
			this.mScaleEntity1.setDiffuse(this.mSelectedDiffuseColor);
			this.mScaleEntity2.setDiffuse(this.mSelectedDiffuseColor);
		}

		this._updateManipulatorNode();
	}

}

::_manipulators <- this.ManipulatorManager();
