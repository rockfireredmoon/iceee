class this.UndoableOperation 
{
	mAlive = true;
	mHasBeenDone = false;
	mPresentationName = null;
	constructor( name )
	{
		this.mPresentationName = name;
		this.mAlive = true;
	}

	function canUndo()
	{
		return this.mAlive && this.mHasBeenDone && "_unexecute" in this;
	}

	function undo()
	{
		if (!this.canUndo())
		{
			throw this.Exception("Cannot undo " + this);
		}

		this._unexecute();
		this.mHasBeenDone = false;
	}

	function canRedo()
	{
		return this.mAlive && !this.mHasBeenDone;
	}

	function redo()
	{
		if (!this.canRedo())
		{
			throw this.Exception("Cannot redo " + this);
		}

		this._execute();
		this.mHasBeenDone = true;
	}

	function _execute()
	{
	}

	function _cannotPlaceUndo()
	{
	}

	function getPresentationName()
	{
		return "" + this.mPresentationName;
	}

	function destroy()
	{
		this.mAlive = false;
	}

	function _tostring()
	{
		return this.getPresentationName();
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "scenery.edit" || qa.query == "scenery.delete")
		{
			this._cannotPlaceUndo();
			::_ChatWindow.addMessage("err/", error, "General");
		}
	}

}

class this.CompoundOperation extends this.UndoableOperation
{
	mInProgress = true;
	mOperations = null;
	constructor()
	{
		this.UndoableOperation.constructor(null);
		this.mOperations = [];
		this.mInProgress = true;
	}

	function len()
	{
		return this.mOperations.len();
	}

	function add( operation )
	{
		if (!this.mInProgress)
		{
			throw this.Exception("cannot add operation to CompoundOperation while not in progress");
		}

		this.mOperations.append(operation);
		return true;
	}

	function canUndo()
	{
		return !this.mInProgress && this.UndoableOperation.canUndo();
	}

	function canRedo()
	{
		return !this.mInProgress && this.UndoableOperation.canRedo();
	}

	function end()
	{
		this.mInProgress = false;
	}

	function getPresentationName()
	{
		if (this.mPresentationName != null)
		{
			return this.mPresentationName;
		}

		if (this.mOperations.len() > 0)
		{
			return this.mOperations[this.mOperations.len() - 1].getPresentationName();
		}

		return "Compound Operation";
	}

	function setPresentationName( presentationName )
	{
		this.mPresentationName = presentationName;
	}

	function isInProgress()
	{
		return this.mInProgress;
	}

	function isSignificant()
	{
		return this.mOperations.len() > 0;
	}

	function _execute()
	{
		if (this.mInProgress)
		{
			throw this.Exception("Cannot execute compound operation while in progress");
		}

		local i;
		local n = this.mOperations.len();

		for( i = 0; i < n; i++ )
		{
			this.mOperations[i].redo();
		}
	}

	function _unexecute()
	{
		local i;

		for( i = this.mOperations.len() - 1; i >= 0; i-- )
		{
			this.mOperations[i].undo();
		}
	}

}

class this.OperationHistory extends this.CompoundOperation
{
	mIndex = 0;
	mLimit = 100;
	constructor( ... )
	{
		this.CompoundOperation.constructor();
		this.mLimit = vargc > 0 ? vargv[0] : 100;
	}

	function reset()
	{
		local op;

		foreach( op in this.mOperations )
		{
			op.destroy();
		}

		this.mOperations = [];
		this.mIndex = 0;
	}

	function execute( operation )
	{
		if (operation instanceof this.CompoundOperation)
		{
			operation.end();
		}

		if (this.mOperations.len() >= this.mLimit)
		{
			local op = this.mOperations.remove(0);
			op.destroy();
			this.mIndex--;
		}

		while (this.mIndex < this.mOperations.len())
		{
			local op = this.mOperations.remove(this.mIndex);
			op.destroy();
		}

		this.CompoundOperation.add(operation);
		operation.redo();
		this.mIndex++;
	}

	function add( operation )
	{
		this.execute(operation);
	}

	function undo()
	{
		if (!this.canUndo())
		{
			return;
		}

		this.mOperations[this.mIndex - 1].undo();
		this.mIndex--;
	}

	function canUndo()
	{
		return this.mIndex > 0 && this.mOperations[this.mIndex - 1].canUndo();
	}

	function redo()
	{
		if (!this.canRedo())
		{
			return;
		}

		this.log.debug("Redoing: " + this.mOperations[this.mIndex]);
		this.mOperations[this.mIndex].redo();
		this.mIndex++;
	}

	function canRedo()
	{
		return this.mOperations.len() > 0 && this.mIndex < this.mOperations.len() && this.mOperations[this.mIndex].canRedo();
	}

	function getUndoPresentationName()
	{
		if (this.canUndo())
		{
			return this.mOperations[this.mIndex - 1].getPresentationName();
		}

		return "Cannot Undo";
	}

	function getRedoPresentationName()
	{
		if (this.canRedo())
		{
			return this.mOperations[this.mIndex].getPresentationName();
		}

		return "Cannot Redo";
	}

}

this._opHistory <- this.OperationHistory();
class this.SceneryCreateOp extends this.UndoableOperation
{
	mSceneryId = null;
	mAsset = null;
	mXform = null;
	constructor( asset, xform )
	{
		this.UndoableOperation.constructor("Create Scenery");
		this.Assert.isInstanceOf(asset, this.AssetReference);
		this.Assert.isTable(xform);
		this.mSceneryId = null;
		this.mAsset = asset;
		this.mXform = clone xform;
	}

	function _execute()
	{
		local args = [];
		args.append("NEW");
		args.append("asset");
		args.append(this.mAsset);
		args.append("p");
		args.append("" + this.mXform.position);
		args.append("q");
		args.append("" + this.mXform.orientation);
		args.append("s");
		args.append("" + this.mXform.scale);
		this._Connection.sendQuery("scenery.edit", this, args);
	}

	function canUndo()
	{
		return this.mSceneryId != null && this.UndoableOperation.canUndo();
	}

	function _unexecute()
	{
		local id = this.mSceneryId;

		if (id == null)
		{
			return;
		}

		this._buildTool.selectionRemove("Scenery/" + this.mSceneryId);
		this.mSceneryId = null;
		this._Connection.sendQuery("scenery.delete", {}, [
			id
		]);
	}

	function onQueryComplete( qa, rows )
	{
		this.mSceneryId = rows[0][0];
		local so = this._sceneObjectManager.getSceneryByID(this.mSceneryId);
		::_buildTool.selectionAdd(so);
	}

}

class this.SceneryUpdateOp extends this.UndoableOperation
{
	mSceneryId = null;
	mNew = null;
	mOld = null;
	constructor( sceneObject, prevXform )
	{
		this.Assert.isInstanceOf(sceneObject, this.SceneObject);
		this.Assert.isTable(prevXform);
		this.UndoableOperation.constructor("Update Scenery");
		this.mSceneryId = sceneObject.getID();
		this.mNew = this.Util.getNodeXform(sceneObject.getNode());
		this.mOld = clone prevXform;
	}

	function _execute()
	{
		local args = [];
		args.append(this.mSceneryId);
		args.append("p");
		args.append("" + this.mNew.position);
		args.append("q");
		args.append("" + this.mNew.orientation);
		args.append("s");
		args.append("" + this.mNew.scale);
		this._Connection.sendQuery("scenery.edit", this, args);
	}

	function _unexecute()
	{
		local args = [];
		args.append(this.mSceneryId);
		args.append("p");
		args.append("" + this.mOld.position);
		args.append("q");
		args.append("" + this.mOld.orientation);
		args.append("s");
		args.append("" + this.mOld.scale);
		this._Connection.sendQuery("scenery.edit", this, args);
	}

	function _cannotPlaceUndo()
	{
		if (this.mSceneryId == null)
		{
			return;
		}

		local so = ::_sceneObjectManager.getSceneryByID(this.mSceneryId);
		local node = so.getNode();
		node.setPosition(this.mOld.position);
		node.setOrientation(this.mOld.orientation);
		node.setScale(this.mOld.scale);
	}

}

class this.SceneryDeleteOp extends this.UndoableOperation
{
	mOldAsset = null;
	mSceneryId = null;
	mOldXform = null;
	constructor( sceneObject )
	{
		this.UndoableOperation.constructor("Delete Scenery");
		this.mSceneryId = sceneObject.getID();
		this.mOldAsset = sceneObject.getVarsTypeAsAsset();
		this.mOldXform = this.Util.getNodeXform(sceneObject.getNode());
	}

	function _execute()
	{
		if (this.mSceneryId == null)
		{
			throw this.Exception("Cannot delete scenery");
		}

		local id = this.mSceneryId;
		this._buildTool.selectionRemove("Scenery/" + this.mSceneryId);
		this.mSceneryId = null;
		this._Connection.sendQuery("scenery.delete", {}, [
			id
		]);
	}

	function canRedo()
	{
		return this.mSceneryId != null && this.UndoableOperation.canRedo();
	}

	function _unexecute()
	{
		local args = [];
		args.append("NEW");
		args.append("asset");
		args.append(this.mOldAsset);
		args.append("p");
		args.append("" + this.mOldXform.position);
		args.append("q");
		args.append("" + this.mOldXform.orientation);
		args.append("s");
		args.append("" + this.mOldXform.scale);
		this._Connection.sendQuery("scenery.edit", this, args);
	}

	function onQueryComplete( qa, rows )
	{
		this.mSceneryId = rows[0][0];
		local so = this._sceneObjectManager.getSceneryByID(this.mSceneryId);
		::_buildTool.selectionAdd(so);
	}

}

class this.BeanSetPropertyOp extends this.UndoableOperation
{
	mBean = null;
	mProperty = null;
	mOldValue = null;
	mNewValue = null;
	constructor( bean, property, newvalue )
	{
		if (typeof property == "string")
		{
			property = this.Bean.getPropertyDescriptor(bean, property);
		}

		this.UndoableOperation.constructor("Set " + property.getDisplayName());
		this.mBean = bean;
		this.mProperty = property;
		this.mNewValue = newvalue;
		this.mOldValue = property.getValue(bean);
	}

	function _execute()
	{
		this.log.debug("Setting " + this.mBean + " " + this.mProperty.getDisplayName() + " = " + this.mNewValue);
		this.mProperty.setValue(this.mBean, this.mNewValue);
	}

	function _unexecute()
	{
		this.log.debug("Setting (via undo) " + this.mBean + " " + this.mProperty.getDisplayName() + " = " + this.mOldValue);
		this.mProperty.setValue(this.mBean, this.mOldValue);
	}

}

class this.CreatureCreateOp extends this.UndoableOperation
{
	mCreatureId = null;
	mCreatureType = null;
	mXform = null;
	constructor( creatureType, xform )
	{
		this.UndoableOperation.constructor("Create Creature");
		this.Assert.isTable(xform);
		this.mCreatureId = null;
		this.mCreatureType = creatureType;
		this.mXform = clone xform;
	}

	function _execute()
	{
		local args = [];
		args.append("SPAWN");
		args.append(this.mCreatureType);
		args.append("" + this.mXform.position);
		this._Connection.sendQuery("creature.create", this, args);
	}

	function canUndo()
	{
		return this.mCreatureId != null && this.UndoableOperation.canUndo();
	}

	function _unexecute()
	{
		local id = this.mCreatureId;

		if (id == null)
		{
			return;
		}

		this._buildTool.selectionRemove("Creature/" + this.mCreatureId);
		this.mCreatureId = null;
		this._Connection.sendQuery("creature.delete", {}, [
			id,
			"PERMANENT"
		]);
	}

	function onQueryComplete( qa, rows )
	{
		this.mCreatureId = rows[0][0];
		local so = this._sceneObjectManager.getCreatureByID(this.mCreatureId);
		::_buildTool.selectionAdd(so);
	}

}

class this.CreatureMoveOp extends this.UndoableOperation
{
	mCreatureId = null;
	mNew = null;
	mOld = null;
	constructor( sceneObject, prevXform )
	{
		this.Assert.isInstanceOf(sceneObject, this.SceneObject);
		this.Assert.isTable(prevXform);
		this.UndoableOperation.constructor("Move Creature");
		this.mCreatureId = sceneObject.getID();
		this.mNew = this.Util.getNodeXform(sceneObject.getNode());
		this.mOld = clone prevXform;
	}

	function _quatToHeading( quat )
	{
		if (this.fabs(this.Vector3().UNIT_Y.y - quat.getAxis().y) <= 0.001)
		{
			return this.Math.rad2byterot(quat.getAngle());
		}
		else
		{
			return this.Math.rad2byterot(-quat.getAngle());
		}
	}

	function _execute()
	{
		local args = [];
		args.append(this.mCreatureId);
		args.append("position");
		args.append("" + this.mNew.position);
		args.append("spawn.position");
		args.append("" + this.mNew.position);
		local h = "" + this._quatToHeading(this.mNew.orientation);
		args.append("heading");
		args.append(h);
		args.append("rotation");
		args.append(h);
		args.append("spawn.heading");
		args.append(h);
		this._Connection.sendQuery("creature.edit", {}, args);
	}

	function _unexecute()
	{
		local args = [];
		args.append(this.mCreatureId);
		args.append("position");
		args.append("" + this.mOld.position);
		args.append("spawn.position");
		args.append("" + this.mOld.position);
		local h = "" + this._quatToHeading(this.mOld.orientation);
		args.append("heading");
		args.append(h);
		args.append("rotation");
		args.append(h);
		args.append("spawn.heading");
		args.append(h);
		this._Connection.sendQuery("creature.edit", {}, args);
	}

}

class this.CreatureDeleteOp extends this.UndoableOperation
{
	mOldType = null;
	mCreatureId = null;
	mOldXform = null;
	constructor( sceneObject )
	{
		this.UndoableOperation.constructor("Delete Creature");
		this.mCreatureId = sceneObject.getID();
		this.mOldType = sceneObject.getType();
		this.mOldXform = this.Util.getNodeXform(sceneObject.getNode());
	}

	function _execute()
	{
		if (this.mCreatureId == null)
		{
			throw this.Exception("Cannot delete creature");
		}

		local id = this.mCreatureId;
		this._buildTool.selectionRemove("Creature/" + this.mCreatureId);
		this.mCreatureId = null;
		this._Connection.sendQuery("creature.delete", {}, [
			id,
			"PERMANENT"
		]);
	}

	function canRedo()
	{
		return this.mCreatureId != null && this.UndoableOperation.canRedo();
	}

	function _unexecute()
	{
		local args = [];
		args.append("SPAWN");
		args.append(this.mOldType);
		args.append("" + this.mOldXform.position);
		this._Connection.sendQuery("creature.create", this, args);
	}

	function onQueryComplete( qa, rows )
	{
		this.mCreatureId = rows[0][0];
		local so = this._sceneObjectManager.getCreatureByID(this.mCreatureId);
		::_buildTool.selectionAdd(so);
	}

}

class this.CreatureDefStatEditOp extends this.UndoableOperation
{
	mCreatureDefId = null;
	mStatId = null;
	mNew = null;
	mOld = null;
	constructor( creatureDefId, statId, newValue )
	{
		if (!(statId in ::Stat))
		{
			throw this.Exception("Invalid stat ID: " + statId);
		}

		this.UndoableOperation.constructor("Edit Creature " + ::Stat[statId].prettyName);
		this.mCreatureDefId = creatureDefId;
		this.mStatId = statId;
		this.mNew = newValue;
		this.mOld = ::_creatureDefManager.getCreatureDef(creatureDefId).getStat(statId);
	}

	function _execute()
	{
		local args = [];
		args.append(this.mCreatureDefId);
		args.append("" + ::Stat[this.mStatId].name);
		args.append("" + this.mNew);
		this._Connection.sendQuery("creature.def.edit", {}, args);
	}

	function _unexecute()
	{
		local args = [];
		args.append(this.mCreatureDefId);
		args.append("" + ::Stat[this.mStatId].name);
		args.append("" + this.mOld);
		this._Connection.sendQuery("creature.def.edit", {}, args);
	}

}

class this.SpawnPointCreateOp extends this.UndoableOperation
{
	mSceneryId = null;
	mClonedId = null;
	mXform = null;
	constructor( clonedId, xform )
	{
		this.UndoableOperation.constructor("Create SpawnPoint");
		this.Assert.isTable(xform);
		this.mSceneryId = null;
		this.mXform = clone xform;
		this.mClonedId = clonedId;
	}

	function _execute()
	{
		local args = [];
		args.append("CLONE");
		args.append("" + this.mClonedId);
		args.append("" + this.mXform.position);
		this._Connection.sendQuery("spawn.create", this, args);
	}

	function canUndo()
	{
		return this.mSceneryId != null && this.UndoableOperation.canUndo();
	}

	function _unexecute()
	{
		local id = this.mSceneryId;

		if (id == null)
		{
			return;
		}

		this._buildTool.selectionRemove("Scenery/" + this.mSceneryId);
		this.mSceneryId = null;
		this._Connection.sendQuery("scenery.delete", {}, [
			id
		]);
	}

	function onQueryComplete( qa, rows )
	{
	}

}

class this.PathNodeCreateOp extends this.UndoableOperation
{
	mSceneryId = null;
	mClonedId = null;
	mXform = null;
	constructor( clonedId, xform )
	{
		this.UndoableOperation.constructor("Create PathNode");
		this.Assert.isTable(xform);
		this.mSceneryId = null;
		this.mXform = clone xform;
		this.mClonedId = clonedId;
	}

	function _execute()
	{
		local args = [];
		local args = [];
		args.append("NEW");
		args.append("asset");
		args.append("Manipulator-PathNode");
		args.append("p");
		args.append("" + this.mXform.position);
		args.append("q");
		args.append("" + this.mXform.orientation);
		args.append("s");
		args.append("" + this.mXform.scale);
		this._Connection.sendQuery("scenery.edit", this, args);
	}

	function canUndo()
	{
		return this.mSceneryId != null && this.UndoableOperation.canUndo();
	}

	function _unexecute()
	{
		local id = this.mSceneryId;

		if (id == null)
		{
			return;
		}

		this._buildTool.selectionRemove("Scenery/" + this.mSceneryId);
		this.mSceneryId = null;
		this._Connection.sendQuery("scenery.delete", {}, [
			id
		]);
	}

	function onQueryComplete( qa, rows )
	{
		this.mSceneryId = rows[0][0];
		local so = this._sceneObjectManager.getSceneryByID(this.mSceneryId);
		local cl = this._sceneObjectManager.getSceneryByID(this.mClonedId);
		::_buildTool.selectionAdd(so);
		::_buildTool.selectionAdd(cl);
		this._buildTool._linkPatrolObjects();
		::_buildTool.selectionRemove(cl);
	}

}

