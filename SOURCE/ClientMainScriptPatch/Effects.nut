this.require("Math");
class this.EffectBase 
{
	static OFF = 0;
	static STOPPING = 1;
	static RUNNING = 2;
	static FROZEN = 3;
	static mEffectName = "Base";
	mState = 0;
	mParent = null;
	mName = null;
	mTimedEvents = null;
	mObject = null;
	mComplete = false;
	mPriority = null;
	mStartTime = null;
	mMaxTime = 30000;
	
	function loopForever() 
	{
		this.mMaxTime = -1;
	}
	
	function getState()
	{
		return this.mState;
	}

	function setState( state )
	{
		if (state == this.mState)
		{
			return;
		}

		switch(state)
		{
		case this.RUNNING:
			this.start();
			break;

		case this.STOPPING:
			this.stop();
			break;

		case this.FROZEN:
			this.freeze();
			break;

		case this.OFF:
			this.destroy();
			break;
		}
	}

	function isRunning()
	{
		return this.mState == this.RUNNING;
	}

	function isStopped()
	{
		return this.mState == this.OFF;
	}

	function isComplete()
	{
		return this.mState == this.OFF && this.mComplete;
	}

	function getName()
	{
		return this.mName;
	}

	function getPriority()
	{
		local x = this;

		while (x != null)
		{
			if (x.mPriority != null)
			{
				return x.mPriority;
			}

			x = x.mParent;
		}

		return null;
	}

	function getObject()
	{
		local x = this;

		while (x != null)
		{
			if (x.mObject)
			{
				return x.mObject;
			}

			x = x.mParent;
		}

		return null;
	}

	function isObjectAssembled()
	{
		local o = this.getObject();
		return o ? o.isAssembled() : false;
	}

	function getHandler()
	{
		local o = this.getObject();
		return o ? o.getEffectsHandler() : null;
	}

	function dispatch( event )
	{
		if (event in this)
		{
			this[event]();
		}
	}

	function start()
	{
		this.mStartTime = ::_gameTime.getGameTimeMiliseconds();

		if (this.mState == this.FROZEN)
		{
			this.thaw();
		}
		else if (this.mState == this.OFF)
		{
			if (this.gLogEffects)
			{
				this.log.debug("FX Starting: " + this);
			}

			this.mState = this.RUNNING;
			//try {
				this._effectStart();
			//}
			//catch(e) {
				//print("ICE! Failed to start effect. " + e);
				//this.mState = this.OFF;
			//}
		}
	}

	function finish()
	{
		this.mComplete = true;
		this.stop();
	}

	function stop()
	{
		if (this.mState == this.FROZEN)
		{
			if (this.gLogEffects)
			{
				this.log.debug("FX Destroying frozen effect: " + this);
			}

			this.destroy();
			return 0;
		}

		if (this.mState != this.RUNNING)
		{
			return 0;
		}

		this.mState = this.STOPPING;

		if (this.mParent)
		{
			this.mParent._notifyStopping(this);
		}

		if (this.gLogEffects)
		{
			this.log.debug("FX Stopping: " + this);
		}

		this._effectStop();
	}

	function cancel()
	{
		if ("onCancel" in this)
		{
			this.onCancel();
		}

		this.stop();
	}

	function destroy()
	{
		if (this.mState == this.OFF)
		{
			return;
		}

		if (this.mTimedEvents)
		{
			foreach( evt in this.mTimedEvents )
			{
				::_eventScheduler.cancel(evt);
			}

			this.mTimedEvents = null;
		}

		this.mState = this.OFF;
		this._effectDestroy();

		if (this.mParent)
		{
			this.mParent._notifyStopped(this);
		}
	}

	function freeze()
	{
		if (this.mState != this.RUNNING)
		{
			return;
		}

		this.mState = this.FROZEN;

		if (this.gLogEffects)
		{
			this.log.debug("FX Freezing: " + this);
		}
	}

	function thaw()
	{
		if (this.mState != this.FROZEN)
		{
			return;
		}

		this.mState = this.RUNNING;

		if (this.gLogEffects)
		{
			this.log.debug("FX Thawing: " + this);
		}

		this._effectThaw();
	}
	
	function getMaxTime()
	{
		if(this.mParent) {
			return this.mParent.getMaxTime();
		}
		return this.mMaxTime;
	}

	function onEnterFrame()
	{
		local mt = this.getMaxTime();
		if (mt != -1 && ( ::_gameTime.getGameTimeMiliseconds() - this.mStartTime > mt ) )
		{
			this.destroy();
		}
		else if (this.mState == this.RUNNING || this.mState == this.STOPPING)
		{
			this._effectFrame();
		}
	}

	function fireIn( delay, eventName, ... )
	{
		if (!this.mTimedEvents)
		{
			this.mTimedEvents = [];
		}

		local evt;

		if (vargc > 0)
		{
			evt = ::_eventScheduler.fireIn(delay, this, eventName, vargv[0]);
		}
		else
		{
			evt = ::_eventScheduler.fireIn(delay, this, eventName);
		}

		this.mTimedEvents.append(evt);
		return evt;
	}

	function repeatIn( delay, period, eventName, ... )
	{
		if (!this.mTimedEvents)
		{
			this.mTimedEvents = [];
		}

		local evt;

		if (vargc > 0)
		{
			evt = ::_eventScheduler.repeatIn(delay, period, this, eventName, vargv[0]);
		}
		else
		{
			evt = ::_eventScheduler.repeatIn(delay, period, this, eventName);
		}

		this.mTimedEvents.append(evt);
		return evt;
	}

	function _effectStart()
	{
	}

	function _effectStop()
	{
		this.destroy();
	}

	function _effectDestroy()
	{
	}

	function _effectFreeze()
	{
	}

	function _effectThaw()
	{
	}

	function _effectFrame()
	{
	}

	function _notifyStopping( effect )
	{
	}

	function _notifyStopped( effect )
	{
	}

	function _tostring()
	{
		local str = "";
		local x = this;

		while (x != null)
		{
			local name = x.mEffectName;

			if (x.mName)
			{
				name += "[" + x.mName + "]";
			}

			if (str != "")
			{
				str = name + "/" + str;
			}
			else
			{
				str = name;
			}

			x = x.mParent;
		}

		return str;
	}

}

class this.EffectGroup extends this.EffectBase
{
	static mEffectName = "Group";
	mEffects = null;
	mGroups = null;
	mDetached = false;
	function add( effect, ... )
	{
		local nameIndex = 0;

		if (typeof effect == "string")
		{
			if (!(effect in ::Effect))
			{
				throw this.Exception("Unknown effect type: " + effect);
			}

			local args = {};

			if (vargc > 0)
			{
				args = vargv[0];

				if (typeof args != "table")
				{
					throw this.Exception("Expecting args table as second argument");
				}

				nameIndex = 1;
			}

			effect = ::Effect[effect]();
			effect.mParent = this;
			effect._parseArgs(args);
		}
		else
		{
			effect.mParent = this;
		}

		if (this.mEffects == null)
		{
			this.mEffects = [];
		}

		this.mEffects.append(effect);
		effect.setState(this.mState);

		if (vargc > nameIndex)
		{
			if (this.mGroups == null)
			{
				this.mGroups = {};
			}

			local name = vargv[nameIndex];

			if (name in this.mGroups)
			{
				throw this.Exception("Redefinition of named effect: " + name);
			}

			effect.mName = name;
			this.mGroups[name] <- effect;
		}

		if (effect.mName == null)
		{
			effect.mName = (this.mEffects.len() - 1).tostring();
		}

		if (this.gLogEffects)
		{
			this.log.debug("FX Added: " + effect);
		}

		return effect;
	}

	function createGroup( name, ... )
	{
		local e = this.EffectGroup();

		if (vargc > 0)
		{
			local o = vargv[0];

			if (o && !(o instanceof this.SceneObject))
			{
				throw this.Exception("Invalid object in createGroup(): " + o);
			}

			e.mObject = o;
		}
		else
		{
			e.detach(this.getObject().getNode());
		}

		e.mName = name;
		this.add(e, name);
		return e;
	}

	function createSequence( name, ... )
	{
		local e = this.EffectSequence();

		if (vargc > 0)
		{
			local o = vargv[0];

			if (!(o instanceof this.SceneObject))
			{
				throw this.Exception("Invalid object in createSequence(): " + o);
			}

			e.mObject = o;
		}
		else
		{
			e.detach(this.getObject().getNode());
		}

		e.mName = name;
		this.add(e, name);
		return e;
	}

	function get( name )
	{
		if (this.mGroups == null || !(name in this.mGroups))
		{
			throw this.Exception("Named effect not found: " + name);
		}

		return this.mGroups[name];
	}

	function detach( ... )
	{
		if (this.mDetached)
		{
			return;
		}

		this.mObject = this.SceneObject(null, "Dummy");
		this.mDetached = true;

		if (vargc > 0 && (vargv[0] instanceof this.SceneNode))
		{
			local node = this.mObject.getNode();

			if (node.getParent())
			{
				node.getParent().removeChild(node);
			}

			vargv[0].addChild(node);
		}
	}

	function _detachGroup( group, location )
	{
		local node = group.getObject().getNode();
		node.getParent().removeChild(node);
		::_scene.getRootSceneNode().addChild(node);
		node.setPosition(location);
	}

	function getEffectCount()
	{
		return this.mEffects == null ? 0 : this.mEffects.len();
	}

	function dispatch( event )
	{
		this.EffectBase.dispatch(event);

		if (this.mEffects != null)
		{
			foreach( e in this.mEffects )
			{
				e.dispatch(event);
			}
		}
	}

	function onCancel()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				if ("onCancel" in e)
				{
					e.onCancel();
				}
			}
		}
	}

	function _effectStart()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				e.start();
			}
		}
	}

	function _effectStop()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				e.stop();
			}
		}
	}

	function _effectDestroy()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				e.destroy();
			}
		}

		if (this.mDetached)
		{
			this.mObject.markAsGarbage();
			this.mObject = null;
			this.mDetached = false;
		}
	}

	function _effectFreeze()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				e.freeze();
			}
		}
	}

	function _effectThaw()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				e.thaw();
			}
		}
	}

	function _effectFrame()
	{
		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				e.onEnterFrame();
			}
		}
	}

	function _notifyStopped( effect )
	{
		local count = 0;
		local total = 0;

		if (this.mEffects)
		{
			foreach( e in this.mEffects )
			{
				if (e.isStopped())
				{
					count++;
				}

				total++;
			}
		}

		if (this.gLogEffects)
		{
			this.log.debug("FX _notifyStopped (" + count + " of " + total + "): " + effect);
		}

		if (count < total)
		{
			return;
		}

		this.destroy();
	}

}

class this.EffectSequence extends this.EffectGroup
{
	static mEffectName = "Sequence";
	mIndex = -1;
	function startNext()
	{
		this.mIndex++;

		if (this.mIndex < this.getEffectCount())
		{
			if (this.gLogEffects)
			{
				this.log.debug("FX Sequence advancing to " + this.mIndex + ": " + this);
			}

			this.mEffects[this.mIndex].start();
		}
		else
		{
			if (this.gLogEffects)
			{
				this.log.debug("FX Sequence complete: " + this);
			}

			this.finish();
		}
	}

	function _notifyStopping( effect )
	{
		if (this.gLogEffects)
		{
			this.log.debug("FX _notifyStopping: " + effect);
		}

		if (effect.mComplete)
		{
			this.startNext();
		}
		else
		{
			if (this.gLogEffects)
			{
				this.log.debug("FX Abandoning sequence prematurely: " + this);
			}

			this.stop();
		}
	}

	function _effectStart()
	{
		this.mIndex = -1;
		this.startNext();
	}

}

class this.EffectScript extends this.EffectGroup
{
	static mEffectName = "Script";
	function _parseArgs( args )
	{
	}

	function startNext()
	{
		local n = this.getNarrative();

		if (n)
		{
			n.startNext();
		}
	}

	function getNarrative()
	{
		local x = this.mParent;

		while (x != null && !(x instanceof this.EffectNarrative))
		{
			x = x.mParent;
		}

		return x;
	}

	function getSource()
	{
		local n = this.getNarrative();
		return n ? n.mSource : null;
	}

	function getTarget()
	{
		local n = this.getNarrative();

		if (!n)
		{
			return null;
		}

		if (!n.mTargets || n.mTargets.len() == 0)
		{
			return null;
		}

		return n.mTargets[0];
	}

	function getTargets()
	{
		local n = this.getNarrative();
		return n ? n.mTargets : null;
	}

	function getSecondaryTarget()
	{
		local n = this.getNarrative();

		if (!n)
		{
			return null;
		}

		if (!n.mSecondaryTargets || n.mSecondaryTargets.len() == 0)
		{
			return null;
		}

		return n.mSecondaryTargets[0];
	}

	function getSecondaryTargets()
	{
		local n = this.getNarrative();
		return n ? n.mSecondaryTargets : null;
	}

	function getPositionalTarget()
	{
		local n = this.getNarrative();
		return n ? n.mPosTarget : null;
	}

	static function _getParentScript( effect )
	{
		if (effect == null)
		{
			return null;
		}

		for( effect = effect.mParent; effect != null; effect = effect.mParent )
		{
			if (effect instanceof this.EffectScript)
			{
				return effect;
			}
		}

		return effect;
	}

	function _effectStart()
	{
		if ("onStart" in this)
		{
			this.onStart();
		}

		this.EffectGroup._effectStart();
	}

	function _effectStop()
	{
		if ("onStop" in this)
		{
			this.onStop();
		}

		this.EffectGroup._effectStop();
	}

	function _effectDestroy()
	{
		if ("onDestroy" in this)
		{
			this.onDestroy();
		}

		this.EffectGroup._effectDestroy();
	}

}

class this.EffectNarrative extends this.EffectSequence
{
	static mEffectName = "Narrative";
	mSource = null;
	mTargets = null;
	mSecondaryTargets = null;
	mPosTarget = null;
	constructor( object, scriptNames )
	{
		if (typeof scriptNames == "string")
		{
			scriptNames = this.Util.split(scriptNames, ",");
		}
		else if (typeof scriptNames != "array")
		{
			throw this.Exception("Invalid narrative: " + scriptNames);
		}

		this.mObject = object;
		this.mSource = this.mObject;

		foreach( x in scriptNames )
		{
			if (typeof x != "string")
			{
				throw this.Exception("Invalid effect script reference: " + x);
			}

			x = this.Util.trim(x);

			if (!(x in ::EffectDef))
			{
				local known = this.Util.join(this.Util.tableKeys(this.EffectDef, true), ",");
				continue;
			}

			this.add(::EffectDef[x](this));
		}
	}

	function getSourceObject()
	{
		return this.mSource;
	}

	function getTargetObject()
	{
		if (this.mTargets && this.mTargets.len() > 0)
		{
			return this.mTargets[0];
		}

		return null;
	}

	function setTargets( targets )
	{
		this.mTargets = targets;
	}

	function setSecondaryTargets( targets )
	{
		this.mSecondaryTargets = targets;
	}

}

class this.EffectsHandler 
{
	static mEffectName = "Handler";
	mRegisteredAlterations = null;
	mActiveAlterations = null;
	mAlterationsValid = false;
	mEffects = null;
	mObject = null;
	mAttachmentAware = null;
	constructor( object )
	{
		if (!object)
		{
			throw "EffectsHandler was passed a null value for the object it is attached to.";
		}

		this.mRegisteredAlterations = [];
		this.mActiveAlterations = [];
		this.mEffects = [];
		this.mAttachmentAware = [];
		this.mObject = object;
		object.addListener(this);
	}

	function findRegisteredAlteration( type, effect )
	{
		foreach( i, x in this.mRegisteredAlterations )
		{
			if (x.effect == effect && x.type == type)
			{
				return i;
			}
		}

		return null;
	}

	function registerAlteration( type, effect, priority )
	{
		local ai = this.findRegisteredAlteration(type, effect);

		if (ai)
		{
			if (this.mRegisteredAlterations[ai].priority != priority)
			{
				this.mRegisteredAlterations[ai].priority = priority;
				this.mAlterationsValid = false;
			}
		}
		else
		{
			this.mRegisteredAlterations.append({
				type = type,
				effect = effect,
				priority = priority
			});
			this.mAlterationsValid = false;
		}

		if (this.mAlterationsValid)
		{
			this._updateActiveAlterations();
		}
	}

	function removeAlteration( type, effect )
	{
		local typeAndEffectMatch = function ( x ) : ( type, effect )
		{
			return x.type == type && x.effect == effect;
		};

		if (this.Util.removeIf(this.mRegisteredAlterations, typeAndEffectMatch))
		{
			this.mAlterationsValid = false;
		}
	}

	function removeAllAlterations()
	{
		this.mRegisteredAlterations.clear();
		this.mAlterationsValid = false;
	}

	function _updateActiveAlterations()
	{
		this.mActiveAlterations = [];

		foreach( reg in this.mRegisteredAlterations )
		{
			local matchFound = false;

			foreach( index, active in this.mActiveAlterations )
			{
				if (active.type == reg.type)
				{
					if (active.priority <= reg.priority)
					{
						if (this.gLogEffects)
						{
							this.log.debug("Effect was found with a higher priority");
						}

						this.mActiveAlterations[index] = this.deepClone(reg);
						matchFound = true;
					}
				}
			}

			if (!matchFound)
			{
				this.mActiveAlterations.append(this.deepClone(reg));
			}
		}

		this.mAlterationsValid = true;
	}

	function getActiveAlteration( type )
	{
		foreach( i, x in this.mActiveAlterations )
		{
			if (x.type == type)
			{
				return x.effect;
			}
		}

		return null;
	}

	function _cloneEffectToAttachment( effect, io )
	{
		local running = effect.getState() == this.RUNNING || effect.getState() == this.STOPPING;

		if (running)
		{
			effect.freeze();
		}

		local copy = this.deepClone(effect);

		if (running)
		{
			effect.thaw();
		}

		io.getEffectsHandler.addEffect(copy);
	}

	function addAttachmentAware( effect )
	{
		this.mAttachmentAware.append(effect);
		this.addEffectToAttachments(effect);
	}

	function removeAttachmentAware( effect )
	{
		throw this.Exception("not implemented yet");
	}

	function addEffectToAttachments( effect )
	{
		if (!("mAttachments" in this.mObject))
		{
			return;
		}

		foreach( io in this.mObject.mAttachments )
		{
			this._cloneEffectToAttachment(effect, io);
		}
	}

	function addEffectNarrative( narrative, ... )
	{
		local e = this.EffectNarrative(this.mObject, narrative);
		local primary_set = false;

		for( local p = 0; p < vargc; p++ )
		{
			if (typeof vargv[p] == "array")
			{
				if (!primary_set)
				{
					primary_set = true;
					e.setTargets(vargv[p]);
				}
				else
				{
					e.setSecondaryTargets(vargv[p]);
				}
			}
			else if (vargv[p] instanceof this.Vector3)
			{
				e.mPosTarget = vargv[p];
			}
			else
			{
				e.mPriority = vargv[p];
			}
		}

		return this.addEffect(e);
	}

	function addEffect( effect, ... )
	{
		if (!(effect instanceof this.EffectBase))
		{
			throw this.Exception("Invalid effect: " + effect);
		}

		if (vargc > 0)
		{
			effect.mPriority = vargv[0];
		}

		if (this.mEffects.len() == 0)
		{
			::_enterFrameRelay.addListener(this);
		}

		this.mEffects.append(effect);
		effect.start();
		return effect;
	}

	function onEvent( sender, event )
	{
	}

	function isObjectAssembled()
	{
		return this.mObject.isAssembled();
	}

	function onAssembled()
	{
		foreach( e in this.mEffects )
		{
			e.thaw();
		}

		foreach( a in this.mRegisteredAlterations )
		{
			a.effect.thaw();
		}
	}

	function onDisassembled()
	{
		foreach( e in this.mEffects )
		{
			e.freeze();
		}

		foreach( a in this.mRegisteredAlterations )
		{
			a.effect.freeze();
		}
	}

	function onAttachmentAdded( io )
	{
		foreach( e in this.mAttachmentAware )
		{
			this._cloneEffectToAttachment(e, io);
		}
	}

	function onAttachmentRemoved( io )
	{
	}

	function onEnterFrame()
	{
		local destroyMe = false;
		local err;

		try
		{
			if (!this.mAlterationsValid)
			{
				this._updateActiveAlterations();
			}

			local newE = [];
			local removeE = [];

			foreach( e in this.mEffects )
			{
				e.onEnterFrame();

				if (!e.isStopped())
				{
					newE.append(e);
				}
				else
				{
					removeE.append(e);
				}
			}

			foreach( effect in removeE )
			{
				effect.destroy();
			}

			this.mEffects = newE;

			if (this.mEffects.len() == 0)
			{
				::_enterFrameRelay.removeListener(this);
			}
		}
		catch( err )
		{
			destroyMe = true;
			this.log.error("Error during effects enter frame (destroying effect: " + this + "): " + err);
		}

		if (destroyMe)
		{
			try
			{
				this.destroy();
			}
			catch( err )
			{
				this.log.error("Error destroying effects during enter frame cleanup: " + err);
			}
		}
	}

	function destroy()
	{
		this._enterFrameRelay.removeListener(this);
		this.mObject.removeListener(this);
		local alts = this.mRegisteredAlterations;
		this.mRegisteredAlterations = [];
		this.mActiveAlterations.clear();

		foreach( a in alts )
		{
			a.effect.destroy();
		}

		if (this.mEffects.len() > 0)
		{
			foreach( e in this.mEffects )
			{
				e.destroy();
			}

			this.mEffects.clear();
		}

		this.mAttachmentAware.clear();
		return null;
	}

}

this.Effect <- {};
this.Effect.nextID <- 0;
this.Effect.genID <- function ( prefix )
{
	return prefix + "_" + ++this.Effect.nextID;
};
class this.Effect.Dummy extends this.EffectBase
{
	static mEffectName = "Dummy";
	function _parseArgs( args )
	{
	}

}

class this.Effect.ColorPulse extends this.EffectBase
{
	static mEffectName = "ColorPulse";
	mTimer = null;
	mColorType = "Diffuse";
	mColor1 = "000000";
	mColor2 = "ff0000";
	mRate = 2.0;
	mRevertToOriginal = true;
	mRevertLenght = 1000;
	mRevertStart = 0;
	mRevertColor = null;
	mCurrentColor = null;
	function _parseArgs( args )
	{
		if ("type" in args)
		{
			this.mColorType = args.type;
		}

		if ("color1" in args)
		{
			this.mColor1 = args.color1;
		}

		if ("color2" in args)
		{
			this.mColor2 = args.color2;
		}

		if ("rate" in args)
		{
			this.mRate = args.rate;
		}

		if ("revert" in args)
		{
			this.mRevertToOriginal = args.revert;
		}

		if ("revert_lenght" in args)
		{
			this.mRevertLenght = args.revert_lenght;
		}
	}

	function setColorOne( hexString )
	{
		this.mColor1 = hexString;
	}

	function setColorTwo( hexString )
	{
		this.mColor2 = hexString;
	}

	function setRate( rate )
	{
		this.mRate = rate;
	}

	function _effectStart()
	{
		if (!this.mTimer)
		{
			this.mTimer = this.Timer();
		}
		else
		{
			this.mTimer.reset();
		}

		this.getHandler().registerAlteration(this.mColorType, this, this.mPriority);
		this.getHandler()._updateActiveAlterations();
		this._effectFrame();
	}

	static function lerp( c1, c2, t )
	{
		local r = c1.r * t + c2.r * (1.0 - t);
		local g = c1.g * t + c2.g * (1.0 - t);
		local b = c1.b * t + c2.b * (1.0 - t);
		return this.Color(r, g, b, 1.0);
	}

	function isActiveAlteration()
	{
		return this.isObjectAssembled() && this.getHandler().getActiveAlteration(this.mColorType) == this;
	}

	function _effectFrame()
	{
		if (!this.isActiveAlteration())
		{
			return;
		}

		if (this.mState == this.STOPPING)
		{
			local elapsed = this.mTimer.getMilliseconds() - this.mRevertStart;

			if (elapsed > this.mRevertLenght)
			{
				this.destroy();
				return;
			}

			local alpha = elapsed.tofloat() / this.mRevertLenght;
			local c = this.lerp(this.Color(0, 0, 0, 0), this.mRevertColor, alpha);
			c.a = 1.0 - alpha;
			this._applyColor(c);
			return;
		}

		local color1 = this.Color(this.mColor1);
		local color2 = this.Color(this.mColor2);
		local t = this.mTimer.getMilliseconds();
		local r = (this.mRate * 1000).tointeger();
		local alpha = (t % r).tofloat() / r;

		if (alpha < 0.5)
		{
			this._applyColor(this.lerp(color1, color2, alpha));
		}
		else
		{
			this._applyColor(this.lerp(color2, color1, alpha));
		}
	}

	function _effectStop()
	{
		if (this.mRevertToOriginal)
		{
			this.mRevertStart = this.mTimer.getMilliseconds();
			this.mRevertColor = this.mCurrentColor;
		}
		else
		{
			this.destroy();
		}
	}

	function _effectDestroy()
	{
		this.mTimer = null;

		if (this.isObjectAssembled())
		{
			this._applyColor(this.Color(0, 0, 0, 0));
		}

		this.getHandler().removeAlteration(this.mColorType, this);
	}

	function _applyColor( color )
	{
		this.mCurrentColor = color;
		local obj = this.getObject();

		if ("entities" in obj.mAssemblyData)
		{
			foreach( i, e in obj.mAssemblyData.entities )
			{
				this._alter(e, color);
			}
		}

		if ("mAttachments" in obj)
		{
			foreach( io in obj.mAttachments )
			{
				if ("entities" in io.mAssemblyData)
				{
					foreach( i, e in io.mAssemblyData.entities )
					{
						this._alter(e, color);
					}
				}
			}
		}
	}

	function _alter( entity, color )
	{
		switch(this.mColorType)
		{
		case "Diffuse":
			entity.setDiffuse(color);
			return;

		case "Ambient":
			entity.setAmbient(color);
			return;
		}
	}

}

class this.Effect.DiffusePulse extends this.Effect.ColorPulse
{
	static mEffectName = "DiffusePulse";
	mColorType = "Diffuse";
}

class this.Effect.AmbientPulse extends this.Effect.ColorPulse
{
	static mEffectName = "AmbientPulse";
	mColorType = "Ambient";
}

class this.Effect.ParticleSystem extends this.EffectBase
{
	static mEffectName = "ParticleSystem";
	mParticleSystems = null;
	mEmitters = null;
	mParticleSystemName = null;
	mEmitterPointName = "node";
	mParticleScale = 1.0;
	mParticleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.TTL | this.PSystemFlags.VELOCITY;
	constructor()
	{
		this.mParticleSystems = [];
		this.mEmitters = [];
	}

	function _parseArgs( args )
	{
		if ("particleSystem" in args)
		{
			this.mParticleSystemName = args.particleSystem;
		}

		if ("emitterPoint" in args)
		{
			this.mEmitterPointName = args.emitterPoint;
		}

		if ("particleScale" in args)
		{
			this.mParticleScale = args.particleScale;
		}

		if ("particleScaleProps" in args)
		{
			this.mParticleScaleProps = args.particleScaleProps;
		}
	}

	function _addEmitterPoint( ep, obj )
	{
		if (typeof ep == "string")
		{
			if (ep == "node")
			{
				this.mEmitters.append({
					bone = null
				});
			}
			else
			{
				local ap = obj.getAttachPointDef(ep);

				if (ap)
				{
					this.mEmitters.append(ap);
				}
				else
				{
					this.log.warn("Invalid attachment point (" + ep + ")");
				}
			}
		}
		else if (ep instanceof this.Vector3)
		{
			this.mEmitters.append({
				bone = null,
				position = this.x
			});
		}
		else
		{
			this.mEmitters.append({
				bone = null
			});
		}
	}

	function _resolveEmitterPointByTypeOrClass( objectType, obj )
	{
		if (objectType in this.EmitterAliases)
		{
			local emitterAliases = this.EmitterAliases[objectType];

			if (this.mEmitterPointName in emitterAliases)
			{
				foreach( x in emitterAliases[this.mEmitterPointName] )
				{
					this._addEmitterPoint(x, obj);
				}
			}
			else
			{
				this._addEmitterPoint(this.mEmitterPointName, obj);
			}
		}
	}

	function _resolveEmitterPoints( object )
	{
		local objectClass = object.getObjectClass();
		local objectType = object.getType();
		this.mEmitters = [];
		this._resolveEmitterPointByTypeOrClass(objectClass, object);
		this._resolveEmitterPointByTypeOrClass(objectType, object);
	}

	function _effectStart()
	{
		if (!this.isObjectAssembled())
		{
			return;
		}

		local obj = this.getObject();
		this._resolveEmitterPoints(obj);
		local objScale = 1.0;

		if ("getNode" in obj)
		{
			objScale = obj.getNode().getScale().x;
		}

		foreach( i, x in this.mEmitters )
		{
			local node;
			local psName = obj.getNodeName() + "/" + x.bone + "/" + this.Effect.genID(this.mParticleSystemName);
			local particleSystem = this._scene.createParticleSystem(psName, this.mParticleSystemName);
			particleSystem.setVisibilityFlags(this.VisibilityFlags.ANY);
			local scale = this.mParticleScale;

			if ("scale" in x)
			{
				scale *= x.scale.x;
			}

			scale *= objScale;

			if (scale != 1.0)
			{
				particleSystem.scaleProps(this.mParticleScaleProps, scale);
			}

			if (x.bone == null || x.bone == "node")
			{
				x.bone = "node";
				node = obj.getNode().createChildSceneNode(psName + "/Node");
				node.attachObject(particleSystem);
			}
			else
			{
				try
				{
					local entity = obj.getEntity();
					entity.attachObjectToBone(x.bone, particleSystem);
					node = particleSystem.getParentNode();
				}
				catch( err )
				{
				}
			}

			this.mParticleSystems.append({
				bone = x.bone,
				particleSystem = particleSystem,
				node = node
			});
			this.Util.setNodeXform(node, x);
		}
	}

	function _effectStop()
	{
		foreach( ps in this.mParticleSystems )
		{
			ps.particleSystem.removeAllEmitters();
		}
	}

	function _effectFrame()
	{
		if (this.mState == this.STOPPING)
		{
			local totalParticles = 0;

			foreach( ps in this.mParticleSystems )
			{
				totalParticles += ps.particleSystem.getNumParticles();
			}

			if (totalParticles == 0)
			{
				this.destroy();
			}
		}
	}

	function _effectDestroy()
	{
		foreach( ps in this.mParticleSystems )
		{
			ps.particleSystem.destroy();

			if (ps.bone == "node")
			{
				ps.node.destroy();
			}
		}

		this.mParticleSystems.clear();
	}

}

this._tmp2 <- 0;
class this.Effect.FFAnimation extends this.EffectBase
{
	static mEffectName = "FFAnimation";
	mAnimationName = "";
	mAnimationEvents = null;
	mOldTimePosition = -0.1;
	mLength = 0.0;
	mSpeed = 0;
	mLoop = false;
	function _parseArgs( args )
	{
		this.mSpeed = null;

		if ("animation" in args)
		{
			this.mAnimationName = args.animation;
		}

		if ("events" in args)
		{
			this.mAnimationEvents = args.events;
		}

		if ("speed" in args)
		{
			this.mSpeed = args.speed;
		}

		if ("loop" in args)
		{
			this.mLoop = args.loop;
		}

		if (this.mSpeed <= 0)
		{
			this.mSpeed = 1.0;
		}

		this.mOldTimePosition = -0.1;
		this.mLength = 0.0;
	}

	function _effectStart()
	{
		local ah = this.getObject().getAnimationHandler();

		if (!ah)
		{
			return;
		}

		local prevName = this.mAnimationName;
		this.mAnimationName = ah.translateAnim(this.mAnimationName);
		ah.onFF(this.mAnimationName, this.mSpeed, this.mLoop);

		if (ah.getCurrentAnim() == this.mAnimationName || prevName == "$MELEE$" || prevName == "$HIT$")
		{
			this.mState = this.RUNNING;
			local speed = ah.getCurrentAnimTimeScale();
			this.mLength = ah.getCurrentAnimLength() / speed;
			local script = this.EffectScript._getParentScript(this);

			if (script && this.mAnimationEvents)
			{
				foreach( at, evt in this.mAnimationEvents )
				{
					local t = this.mLength * at;

					if (this.mLoop)
					{
						script.repeatIn(t, t, evt);
					}
					else
					{
						script.fireIn(t, evt);
					}
				}
			}
		}
		else
		{
			this.stop();
		}
	}

	function _effectFrame()
	{
		local ah = this.getObject().getAnimationHandler();

		if (!ah)
		{
			this.stop();
			return;
		}

		local name = ah.getCurrentAnim();

		if (ah.getCurrentAnim() != this.mAnimationName)
		{
			this.stop();
			return;
		}

		local tp = ah.getCurrentAnimTimePosition();

		if (tp < this.mOldTimePosition && !this.mLoop)
		{
			this.finish();
			return;
		}

		this.mOldTimePosition = tp;
	}

	function onCancel()
	{
		local ah = this.getObject().getAnimationHandler();

		if (ah)
		{
			if (ah.getCurrentAnim() == this.mAnimationName)
			{
				ah.stopAnim(this.mAnimationName);
			}
		}
	}

	function _effectDestroy()
	{
		local ah = this.getObject().getAnimationHandler();

		if (ah)
		{
			this.stop();
		}
	}

}

class this.Effect.CueEffect extends this.EffectBase
{
	static mEffectName = "CueEffect";
	mNarrativeName = null;
	mTarget = null;
	function _parseArgs( args )
	{
		if ("narrative" in args)
		{
			this.mNarrativeName = args.narrative;
		}

		if ("target" in args)
		{
			this.mTarget = args.target;
		}
	}

	function _effectStart()
	{
		if (this.mNarrativeName && this.mTarget)
		{
			this.mTarget.cue(this.mNarrativeName);
		}
	}

	function _effectFrame()
	{
	}

	function _effectDestroy()
	{
	}

}

class this.Effect.Mesh extends this.EffectBase
{
	static mEffectName = "Mesh";
	mMeshName = "";
	mTexture = null;
	mEntity = null;
	mBone = null;
	mPoint = null;
	mOpacity = 0.0;
	mFadeInTime = 0.30000001;
	mFadeOutTime = 0.0;
	mTimer = null;
	mRandom = this.Random();
	function _parseArgs( args )
	{
		if ("mesh" in args)
		{
			this.mMeshName = args.mesh;
		}

		if ("bone" in args)
		{
			this.mBone = args.bone;
		}

		if ("point" in args)
		{
			this.mPoint = args.point;
		}

		if ("fadeInTime" in args)
		{
			this.mFadeInTime = args.fadeInTime;
		}

		if ("fadeOutTime" in args)
		{
			this.mFadeOutTime = args.fadeOutTime;
		}

		if ("texture" in args)
		{
			this.mTexture = args.texture;
		}

		if (!this.mBone)
		{
			this.mBone = "node";
		}
	}

	function _effectStart()
	{
		local obj = this.getObject();
		local uniqueName = "MeshEffect/" + this.mRandom.nextFloat();
		this.mEntity = ::_scene.createEntity(uniqueName, this.mMeshName);
		this.mEntity.setVisibilityFlags(this.VisibilityFlags.ATTACHMENT | this.VisibilityFlags.ANY | this.VisibilityFlags.LIGHT_GROUP_ANY);

		if (this.mTexture)
		{
			this.mEntity.applyTextureAliases({
				Diffuse = this.mTexture
			});
		}

		local attached = false;
		local bone = this.mBone;
		local xform;

		if (this.mPoint)
		{
			local ap = obj.getAttachPointDef(this.mPoint);

			if (ap != null && "bone" in ap)
			{
				bone = ap.bone;
				xform = ap;
			}
		}

		if (bone == "node")
		{
			obj.getNode().attachObject(this.mEntity);
			xform = null;
		}
		else
		{
			try
			{
				obj.getEntity().attachObjectToBone(bone, this.mEntity);
			}
			catch( err )
			{
				this.log.error("Effect." + this.mEffectName + ".start - " + bone + " was not found on entity " + obj.getName());
				bone = "node";
				xform = null;
				obj.getNode().attachObject(this.mEntity);
			}
		}

		if (xform)
		{
			this.Util.setNodeXform(this.mEntity.getParentNode(), xform);
		}

		if (this.mFadeInTime > 0)
		{
			this.mEntity.setOpacity(this.mOpacity);
			this.mTimer = this.Timer();
		}
	}

	function _effectFrame()
	{
		if (!this.mTimer)
		{
			return;
		}

		local t = this.mTimer.getMilliseconds() / 1000.0;

		if (this.mState == this.STOPPING)
		{
			if (t >= this.mFadeOutTime)
			{
				this.destroy();
				return;
			}

			this.mOpacity = 1.0 - t / this.mFadeOutTime;
			this.mEntity.setOpacity(this.mOpacity);
		}
		else
		{
			if (t < this.mFadeInTime)
			{
				this.mOpacity = t / this.mFadeInTime;
			}
			else
			{
				this.mOpacity = 1.0;
				this.mTimer = null;
			}

			this.mEntity.setOpacity(this.mOpacity);
		}
	}

	function _effectStop()
	{
		if (this.mFadeOutTime > 0)
		{
			if (this.mTimer)
			{
				this.mTimer.reset();
			}
			else
			{
				this.mTimer = this.Timer();
			}
		}
		else
		{
			this.destroy();
		}
	}

	function _effectDestroy()
	{
		if (this.mEntity)
		{
			this.mEntity.destroy();
		}

		this.mTimer = null;
	}

}

class this.Effect.MoveToTarget extends this.EffectBase
{
	static mEffectName = "MoveToTarget";
	mSource = null;
	mSourceBone = null;
	mSourcePoint = null;
	mTarget = null;
	mTargetBone = null;
	mTargetPoint = null;
	mOrient = false;
	mVelocity = 0.0;
	mAcceleration = 5.0;
	mTopSpeed = 100.0;
	mEvent = "onContact";
	mTimer = null;
	function _parseArgs( args )
	{
		if ("source" in args)
		{
			this.mSource = args.source;
		}

		if ("sourceBone" in args)
		{
			this.mSourceBone = args.sourceBone;
		}

		if ("sourcePoint" in args)
		{
			this.mSourcePoint = args.sourcePoint;
		}

		if ("target" in args)
		{
			this.mTarget = args.target;
		}

		if ("targetBone" in args)
		{
			this.mTargetBone = args.targetBone;
		}

		if ("targetPoint" in args)
		{
			this.mTargetPoint = args.targetPoint;
		}

		if ("event" in args)
		{
			this.mEvent = args.event;
		}

		if ("intVelocity" in args)
		{
			this.mVelocity = args.intVelocity;
		}

		if ("topSpeed" in args)
		{
			this.mTopSpeed = args.topSpeed;
		}

		if ("orient" in args)
		{
			this.mOrient = args.orient;
		}

		if ("accel" in args)
		{
			this.mAcceleration = args.accel;
		}
	}

	function _resolvePos( so, point, bone )
	{
		if (point)
		{
			local ap = so.getAttachPointDef(point);

			if (ap == null || !("bone" in ap))
			{
				return this.Util.getBoneWorldPosition(so, bone);
			}

			return this.Util.getBoneWorldPosition(so, ap.bone, ap);
		}

		return this.Util.getBoneWorldPosition(so, bone);
	}

	function _sourcePos()
	{
		return this._resolvePos(this.mSource, this.mSourcePoint, this.mSourceBone);
	}

	function _targetPos()
	{
		return this._resolvePos(this.mTarget, this.mTargetPoint, this.mTargetBone);
	}

	function _effectStart()
	{
		if (!this.mSource || !this.mTarget)
		{
			this.stop();
			return;
		}

		local pos = this._sourcePos();
		local node = this.getObject().getNode();
		local parentNode = node.getParent();

		if (parentNode)
		{
			parentNode.removeChild(node);
		}

		::_scene.getRootSceneNode().addChild(node);
		this.getObject().setPosition(pos);

		if (this.mOrient == true)
		{
			node.lookAt(this._targetPos());
		}
	}

	function _effectFrame()
	{
		local obj = this.getObject();
		local pos = obj.getPosition();
		local tar = this._targetPos();

		if (tar == null)
		{
			return;
		}

		local dir = tar - pos;
		local distance = dir.length();
		local acc = this._deltat / 1000.0 * this.mAcceleration;
		local speed = this.mVelocity + acc;

		if (speed > this.mTopSpeed)
		{
			speed = this.mTopSpeed;
		}

		this.mVelocity = speed;

		if (distance <= 5.0 || speed >= distance - 5.0)
		{
			obj.setPosition(tar);
			local script = this.EffectScript._getParentScript(this);

			if (script)
			{
				script.fireIn(0, this.mEvent);
			}

			this.finish();
			return;
		}

		dir /= distance;
		dir *= speed;
		obj.setPosition(pos + dir);
		local node = this.getObject().getNode();

		if (this.mOrient == true)
		{
			node.lookAt(this._targetPos());
		}
	}

	function _effectDestroy()
	{
	}

}

class this.Effect.ArcToTarget extends this.EffectBase
{
	static mEffectName = "ArcToTarget";
	mSource = null;
	mSourceBone = null;
	mSourcePoint = null;
	mTarget = null;
	mTargetBone = null;
	mTargetPoint = null;
	mOrient = false;
	mInitialOrient = null;
	mTimer = null;
	mArcForwardAngle = 0.0;
	mArcSideAngle = 90.0;
	mArcTime = 0.0;
	mArcEnd = 0.5;
	mVelocity = 0.0;
	mAcceleration = 5.0;
	mTopSpeed = 100.0;
	mEvent = "onContact";
	mAdjustedTime = 1.0;
	mTimer = null;
	function _parseArgs( args )
	{
		if ("source" in args)
		{
			this.mSource = args.source;
		}

		if ("sourceBone" in args)
		{
			this.mSourceBone = args.sourceBone;
		}

		if ("sourcePoint" in args)
		{
			this.mSourcePoint = args.sourcePoint;
		}

		if ("target" in args)
		{
			this.mTarget = args.target;
		}

		if ("targetBone" in args)
		{
			this.mTargetBone = args.targetBone;
		}

		if ("targetPoint" in args)
		{
			this.mTargetPoint = args.targetPoint;
		}

		if ("event" in args)
		{
			this.mEvent = args.event;
		}

		if ("intVelocity" in args)
		{
			this.mVelocity = args.intVelocity;
		}

		if ("topSpeed" in args)
		{
			this.mTopSpeed = args.topSpeed;
		}

		if ("orient" in args)
		{
			this.mOrient = args.orient;
		}

		if ("accel" in args)
		{
			this.mAcceleration = args.accel;
		}

		if ("arcEnd" in args)
		{
			this.mArcEnd = args.arcEnd;
		}

		if ("arcForwardAngle" in args)
		{
			this.mArcForwardAngle = args.arcForwardAngle;
		}

		if ("arcSideAngle" in args)
		{
			this.mArcSideAngle = args.arcSideAngle;
		}
	}

	function _resolveArcPoint( pRad )
	{
		local vector = this.Vector3(0.0, 0.0, 0.0);
		vector.z = 0;
		vector.x = this.sin(pRad);
		vector.y = this.cos(pRad);
		vector.normalize();
		return vector;
	}

	function _resolvePos( so, point, bone )
	{
		if (point)
		{
			local ap = so.getAttachPointDef(point);

			if (ap == null || !("bone" in ap))
			{
				return this.Util.getBoneWorldPosition(so, bone);
			}

			return this.Util.getBoneWorldPosition(so, ap.bone, ap);
		}

		return this.Util.getBoneWorldPosition(so, bone);
	}

	function _sourcePos()
	{
		return this._resolvePos(this.mSource, this.mSourcePoint, this.mSourceBone);
	}

	function _targetPos()
	{
		return this._resolvePos(this.mTarget, this.mTargetPoint, this.mTargetBone);
	}

	function _effectStart()
	{
		if (!this.mSource || !this.mTarget)
		{
			this.stop();
			return;
		}

		local pos = this._sourcePos();
		local node = this.getObject().getNode();
		local parentNode = node.getParent();

		if (parentNode)
		{
			parentNode.removeChild(node);
		}

		::_scene.getRootSceneNode().addChild(node);
		this.getObject().setPosition(pos);
		local forwardAngle = (this.mArcForwardAngle + 90.0) / 360.0;
		local sideAngle = this.mArcSideAngle / 360.0;
		this.mInitialOrient = this.Quaternion(this.Math.ConvertPercentageToRad(sideAngle), this.Vector3(0.0, 1.0, 0.0));
		this.mInitialOrient = this.Quaternion(this.Math.ConvertPercentageToRad(forwardAngle), this.Vector3(1.0, 0.0, 0.0)) * this.mInitialOrient;
		this.mInitialOrient.normalize();
		local startOrient = this.mSource.getNode().getWorldOrientation();
		local newOrient = startOrient * this.mInitialOrient;
		newOrient.normalize();
		this.mInitialOrient = newOrient;
		this.mTimer = this.Timer();
		local tar = this._targetPos();
		local dir = tar - pos;
		local distance = dir.length();
		local time = 0.1;
		local velocity = this.mVelocity;

		while (distance > 0)
		{
			velocity += this.mAcceleration * 0.1;
			distance -= velocity;
			time += 0.1;
		}

		this.mArcTime = time * this.mArcEnd;
	}

	function _effectFrame()
	{
		local obj = this.getObject();
		local pos = obj.getPosition();
		local tar = this._targetPos();
		local dir = tar - pos;
		local distance = dir.length();
		local acc = this._deltat / 1000.0 * this.mAcceleration;
		local speed = this.mVelocity + acc;

		if (speed > this.mTopSpeed)
		{
			speed = this.mTopSpeed;
		}

		this.mVelocity = speed;

		if (distance <= 5.0 || speed >= distance - 5.0)
		{
			obj.setPosition(tar);
			local script = this.EffectScript._getParentScript(this);

			if (script)
			{
				script.fireIn(0, this.mEvent);
			}

			this.finish();
			return;
		}

		local node = this.getObject().getNode();
		node.lookAt(this._targetPos());
		local tarOrient = node.getWorldOrientation();
		tarOrient.normalize();
		local slerp = this.mTimer.getMilliseconds() / 1000.0;

		if (this.mArcTime == 0.0)
		{
			slerp = 1.0;
		}
		else
		{
			slerp = slerp / this.mArcTime;
		}

		if (slerp > 1.0)
		{
			slerp = 1.0;
		}

		if (this.gLogEffects)
		{
			this.log.debug("Slerp: " + slerp);
		}

		local newOrient = this.mInitialOrient.slerp(slerp, tarOrient);
		node.setOrientation(newOrient);
		local axis = newOrient.zAxis() * -1;
		axis.normalize();
		axis = axis * speed;
		obj.setPosition(pos + axis);
	}

	function _effectDestroy()
	{
	}

}

class this.Effect.Ribbon extends this.EffectBase
{
	static mEffectName = "Ribbon";
	mRibbon = null;
	mAttachment = null;
	mAttMovable = null;
	mNameRandomizer = this.Random();
	function _parseArgs( args )
	{
		local entity;
		local height = 20.0;
		local so = this.getObject();

		if ("attachment_source" in args)
		{
			so = args.source;
		}

		if ("attachment" in args)
		{
			local ap = so.getAttachPointDef(args.attachment);

			if ("bone" in ap)
			{
				this.mAttMovable = ::_scene.createSoundEmitter("RibbonEffect_Helper/" + this.mNameRandomizer.nextInt().tostring());
				so.getAssembler().getBaseEntity(so).attachObjectToBone(ap.bone, this.mAttMovable);
				entity = this.mAttMovable;
			}
		}
		else
		{
			local objects = this.getObject().getNode().getAttachedObjects();

			if (objects.len() > 0)
			{
				entity = objects[0];
				height = entity.getBoundingBox().getSize().y;
			}
		}

		if (!entity)
		{
			return;
		}

		this.mRibbon = ::_scene.createRibbon();
		this.mRibbon.setMaterial("material" in args ? args.material : "LightRibbonTrail");
		this.mRibbon.setInitialColor("initialColor" in args ? this.Color(args.initialColor) : this.Color(0.30000001, 0.30000001, 0.30000001, 1.0));
		this.mRibbon.setColorChange("colorChange" in args ? this.Color(args.colorChange[0], args.colorChange[1], args.colorChange[2], args.colorChange[3]) : this.Color(2.5, 2.5, 2.5, 2.0));
		this.mRibbon.setMaxSegments("maxSegments" in args ? args.maxSegments.tointeger() : 32);
		this.mRibbon.setInitialWidth("width" in args ? args.width : height);
		this.mRibbon.setOffset("offset" in args ? args.offset.tofloat() : height / 2.0);
		this.mRibbon.setTrackedMovableObject(entity);
	}

	function _effectFrame()
	{
		if (this.mState == this.STOPPING)
		{
			local segments = this.mRibbon.getActiveSegments();

			if (segments <= 0)
			{
				this.mRibbon.destroy();
				this.mRibbon = null;
				this.destroy();
			}
		}
	}

	function _effectStop()
	{
		if (this.mRibbon)
		{
			this.mRibbon.setActive(false);
		}
		else
		{
			this.EffectBase._effectStop();
		}
	}

	function _effectDestroy()
	{
		this._effectStop();

		if (this.mAttMovable)
		{
			this.mAttMovable.destroy();
		}
	}

}

class this.Effect.WeaponRibbon extends this.EffectBase
{
	static mEffectName = "WeaponRibbon";
	mRibbonTrails = null;
	function _parseArgs( args )
	{
		this.mRibbonTrails = [];
		local attachments = this.getObject().getHandAttachments();

		foreach( k, v in attachments )
		{
			if (v.mAssemblyData != null && v.mAssemblyData.ribbon != null)
			{
				if (v.getWeaponType() != "Shield")
				{
					this.mRibbonTrails.append(v.mAssemblyData.ribbon);
					v.mAssemblyData.ribbon.setActive(true);
					v.mAssemblyData.ribbon.setInitialColor("initialColor" in args ? this.Color(args.initialColor) : this.Color(0.30000001, 0.30000001, 0.30000001, 1.0));
					v.mAssemblyData.ribbon.setColorChange("colorChange" in args ? this.Color(args.colorChange[0], args.colorChange[1], args.colorChange[2], args.colorChange[3]) : this.Color(2.5, 2.5, 2.5, 2.0));
				}
			}
		}
	}

	function _effectFrame()
	{
		if (this.mState == this.STOPPING)
		{
			this.destroy();
			return;
		}
	}

	function _effectStop()
	{
		foreach( r in this.mRibbonTrails )
		{
			r.setActive(false);
		}

		this.mRibbonTrails = [];
	}

	function _effectDestroy()
	{
		this._effectStop();
	}

}

class this.Effect.Spin extends this.EffectBase
{
	static mEffectName = "Spin";
	mAxis = null;
	mAccel = 0;
	mSpeed = 0.5;
	mMinSpeed = 0;
	mMaxSpeed = 3;
	mStopTime = 0;
	mExtraStopTime = 0;
	mAccum = 0;
	function _parseArgs( args )
	{
		if ("axis" in args)
		{
			this.mAxis = args.axis;

			if (typeof this.mAxis == "string")
			{
				if (this.mAxis.tolower() == "x")
				{
					this.mAxis = this.Vector3(1, 0, 0);
				}
				else if (this.mAxis.tolower() == "y")
				{
					this.mAxis = this.Vector3(0, 1, 0);
				}
				else if (this.mAxis.tolower() == "z")
				{
					this.mAxis = this.Vector3(0, 0, 1);
				}
				else
				{
					throw this.Exception("Unknown axis: " + this.mAxis);
				}
			}
		}

		if ("accel" in args)
		{
			this.mAccel = args.accel;
		}

		if ("speed" in args)
		{
			this.mSpeed = args.speed;
		}

		if ("minSpeed" in args)
		{
			this.mMinSpeed = args.minSpeed;
		}

		if ("maxSpeed" in args)
		{
			this.mMaxSpeed = args.maxSpeed;
		}

		if ("extraStopTime" in args)
		{
			this.mExtraStopTime = args.extraStopTime * 1000;
		}

		if (!this.mAxis)
		{
			this.mAxis = this.Vector3(0, 1, 0);
		}
	}

	function _effectStop()
	{
		if (this.mExtraStopTime <= 0)
		{
			this.destroy();
		}
		else
		{
			this.mStopTime = this._time;
		}
	}

	function _effectFrame()
	{
		local obj = this.getObject();

		if (this.mState == this.STOPPING)
		{
			if (this.mStopTime + this.mExtraStopTime <= this._time)
			{
				this.destroy();
				return;
			}
		}

		local t = this._deltat / 1000.0;

		if (this.mAccel != 0)
		{
			this.mSpeed += t * t * this.mAccel;
			this.mSpeed = this.Math.clamp(this.mSpeed, this.mMinSpeed, this.mMaxSpeed);
		}

		if (obj instanceof this.SceneObject)
		{
			if (obj.getNode() != null)
			{
				this.mAccum += this.mSpeed * t * this.Math.PI * 2.0;
				obj.getNode().rotate(this.mAxis, this.mSpeed * t * this.Math.PI * 2.0);
			}
			else
			{
				this.log.error("Effect.Spin: Trying to rotate an node that doesn\'t exist.");
			}
		}
		else
		{
			this.log.error("Effect.Spin: Trying to rotate an object that is not a instance of SceneObject");
		}
	}

}

class this.Effect.ScaleTo extends this.EffectBase
{
	static mEffectName = "Rotate";
	mAxis = null;
	mStartSize = null;
	mEndSize = 2.0;
	mDuration = 1.0;
	mInterpMode = "linear";
	mCurrentT = null;
	mMaintain = false;
	function _parseArgs( args )
	{
		this.mStartSize = this.getObject().getNode().getScale().x;

		if ("size" in args)
		{
			this.mEndSize = args.size;
		}

		if ("startSize" in args)
		{
			this.mStartSize = args.startSize;
		}

		if ("duration" in args)
		{
			this.mDuration = args.duration;
		}

		if ("maintain" in args)
		{
			this.mMaintain = args.maintain;
		}
	}

	function _effectStart()
	{
		if (this.mDuration <= 0)
		{
			if (!this.mMaintain)
			{
				this.getObject().setScale(null);
				this.finish();
			}
			else
			{
				this.getObject().setScale(this.mEndSize);
			}
		}
		else
		{
			this.mCurrentT = 0;
		}
	}

	function _effectFrame()
	{
		local obj = this.getObject();

		if (this.mCurrentT == null)
		{
			this.mCurrentT = this._deltat;
		}
		else
		{
			this.mCurrentT += this._deltat;
		}

		local t = this.mDuration > 0.0 ? this.mCurrentT.tofloat() / 1000.0 / this.mDuration : 1.0;

		if (t >= 1.0)
		{
			obj.setScale(this.mEndSize);

			if (!this.mMaintain)
			{
				this.finish();
			}
		}
		else
		{
			local size = t * this.mEndSize + (1.0 - t) * this.mStartSize;
			obj.setScale(size);
		}
	}

	function _effectDestroy()
	{
		if (!this.mMaintain)
		{
			this.getObject().setScale(null);
		}
		else
		{
			this.getObject().setScale(this.mEndSize);
		}
	}

}

class this.Effect.Sound extends this.EffectBase
{
	static mEffectName = "Sound";
	mEmitter = null;
	mSound = null;
	mGain = 1.0;
	mPriority = 0;
	mLoop = false;
	mAmbient = true;
	function _parseArgs( args )
	{
		if ("sound" in args)
		{
			this.mSound = args.sound;
		}

		if ("gain" in args)
		{
			this.mGain = args.gain;
		}

		if ("priority" in args)
		{
			this.mPriority = args.priority;
		}

		if ("loop" in args)
		{
			this.mLoop = args.loop;
		}

		if ("ambient" in args)
		{
			this.mAmbient = args.ambient;
		}
	}

	function _effectStart()
	{
		if (this.mSound == null)
		{
			this.finish();
			return;
		}

		this.mEmitter = ::_scene.createSoundEmitter(this.Effect.genID("Effect.Sound"), this.mSound);
		this.mEmitter.setVisibilityFlags(this.VisibilityFlags.ANY);
		this.mEmitter.setAmbient(true);
		this.getObject().getNode().attachObject(this.mEmitter);
		this.mEmitter.setGain(this.mGain);

		if (this.mLoop)
		{
			this.mEmitter.setLooping(true);
		}

		this.mEmitter.play();
	}

	function _effectDestroy()
	{
		if (this.mEmitter)
		{
			this.mEmitter.destroy();
			this.mEmitter = null;
		}
	}

	function _effectStop()
	{
		if (this.mLoop || !this.mEmitter || !this.mEmitter.isPlaying())
		{
			this.destroy();
		}
	}

	function _effectFrame()
	{
		if (this.mState == this.STOPPING)
		{
			if (this.mLoop || !this.mEmitter || !this.mEmitter.isPlaying())
			{
				this.destroy();
			}
		}
		else if (this.mEmitter && !this.mEmitter.isPlaying())
		{
			this.finish();
		}
	}

}

class this.Effect.LookAtTarget extends this.EffectBase
{
	static mEffectName = "LookAtTarget";
	mTarget = null;
	mTargetBone = null;
	mTargetPoint = null;
	mSource = null;
	mSourcePoint = null;
	mSourceBone = null;
	mVelocity = 3.0;
	mAcceleration = 0.0;
	mTopSpeed = 20;
	mYawOnly = false;
	mEvent = "onContact";
	mUtilNode = null;
	mTimer = null;
	function _parseArgs( args )
	{
		if ("source" in args)
		{
			this.mSource = args.source;
		}

		if ("sourcePoint" in args)
		{
			this.mSourcePoint = args.sourcePoint;
		}

		if ("sourceBone" in args)
		{
			this.mSourceBone = args.sourceBone;
		}

		if ("target" in args)
		{
			this.mTarget = args.target;
		}

		if ("targetBone" in args)
		{
			this.mTargetBone = args.targetBone;
		}

		if ("targetPoint" in args)
		{
			this.mTargetPoint = args.targetPoint;
		}

		if ("event" in args)
		{
			this.mEvent = args.event;
		}

		if ("topSpeed" in args)
		{
			this.mTopSpeed = args.topSpeed;
		}

		if ("accel" in args)
		{
			this.mAcceleration = args.accel;
		}

		if ("velocity" in args)
		{
			this.mVelocity = args.velocity;
		}

		if ("yawOnly" in args)
		{
			this.mYawOnly = args.yawOnly;
		}
	}

	function _resolvePos( so, point, bone )
	{
		if (point)
		{
			local ap = so.getAttachPointDef(point);

			if (ap == null || !("bone" in ap))
			{
				return this.Util.getBoneWorldPosition(so, bone);
			}

			return this.Util.getBoneWorldPosition(so, ap.bone, ap);
		}

		if (bone)
		{
			return this.Util.getBoneWorldPosition(so, bone);
		}

		return so.getNode().getWorldPosition();
	}

	function _sourcePos()
	{
		return this._resolvePos(this.mSource, this.mSourcePoint, this.mSourceBone);
	}

	function _targetPos()
	{
		return this._resolvePos(this.mTarget, this.mTargetPoint, this.mTargetBone);
	}

	function _effectStart()
	{
		if (!this.mSource)
		{
			this.mSource = this.getObject();
		}

		if (!this.mSource || !this.mTarget)
		{
			this.stop();
			return;
		}

		local pos = this._sourcePos();
		local node = this.mSource.getNode();
		local parentNode = node.getParent();

		if (parentNode)
		{
			parentNode.removeChild(node);
		}

		::_scene.getRootSceneNode().addChild(node);
		this.getObject().setPosition(pos);
		this.mUtilNode = this._scene.createSceneNode();
		this.mTimer = this.Timer();
	}

	function _effectFrame()
	{
		local obj = this.mSource;
		local tar = this._targetPos();
		local src = this._sourcePos();
		this.mUtilNode.setPosition(src);
		this.mUtilNode.lookAt(tar);
		local curr_orient = obj.getOrientation();
		local dest_orient = this.mUtilNode.getOrientation();
		local angle = this.Math.abs(dest_orient.getYaw() - curr_orient.getYaw());

		if (!this.mYawOnly)
		{
			angle += this.Math.abs(dest_orient.getRoll() - curr_orient.getRoll());
			angle += this.Math.abs(dest_orient.getPitch() - curr_orient.getPitch());
		}

		if (angle <= 0.0099999998)
		{
			local script = this.EffectScript._getParentScript(this);

			if (script)
			{
				script.fireIn(0, this.mEvent);
			}

			this.finish();
			return;
		}

		local acc = this.mTimer.getMilliseconds() / 1000.0 * this.mAcceleration;
		local speed = this.mVelocity + acc * acc;

		if (speed > this.mTopSpeed)
		{
			speed = this.mTopSpeed;
		}

		speed = this.Math.deg2rad(speed);
		local s = this.Math.clamp(speed / angle, 0.0, 1.0);
		local orient = curr_orient.slerp(s, dest_orient);

		if (this.mYawOnly)
		{
			orient = this.Quaternion(orient.getYaw(), this.Vector3(0, 1, 0));
		}

		obj.setOrientation(orient);
	}

	function _effectDestroy()
	{
		this.mTimer = null;
		::_scene.getRootSceneNode().removeChild(this.mUtilNode);
		this.mUtilNode.destroy();
	}

}

class this.Effect.Projector extends this.EffectBase
{
	static mEffectName = "Projector";
	mProjector = null;
	mProjectorNode = null;
	mTextureName = "";
	mDuration = 0;
	mFadeIn = 0;
	mFadeOut = 0;
	mTarget = null;
	mTargetBone = null;
	mOffset = null;
	mTargetDir = null;
	mNearClipDistance = 0.1;
	mFarClipDistance = 50.0;
	mOrthographic = true;
	mOrthoWidth = 3.0;
	mOrthoHeight = 3.0;
	mFOVy = 1.0;
	mAdditive = false;
	mAlphaBlended = false;
	mQueryFlags = this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR;
	mTimer = null;
	mStopTime = 0;
	mNameRandomizer = this.Random();
	mDebug = false;
	function _parseArgs( args )
	{
		if ("duration" in args)
		{
			this.mDuration = args.duration;
		}

		if ("fadeIn" in args)
		{
			this.mFadeIn = args.fadeIn;
		}

		if ("fadeOut" in args)
		{
			this.mFadeOut = args.fadeOut;
		}

		if ("target" in args)
		{
			this.mTarget = args.target;
		}

		if ("targetBone" in args)
		{
			this.mTargetBone = args.targetBone;
		}

		if ("offset" in args)
		{
			this.mOffset = args.offset;
		}

		if ("targetDir" in args)
		{
			this.mTargetDir = args.targetDir;
		}

		if ("textureName" in args)
		{
			this.mTextureName = args.textureName;
		}

		if ("near" in args)
		{
			this.mNearClipDistance = args.near;
		}

		if ("far" in args)
		{
			this.mFarClipDistance = args.far;
		}

		if ("ortho" in args)
		{
			this.mOrthographic = args.ortho;
		}

		if ("orthoWidth" in args)
		{
			this.mOrthoWidth = args.orthoWidth;
		}

		if ("orthoHeight" in args)
		{
			this.mOrthoHeight = args.orthoHeight;
		}

		if ("fov" in args)
		{
			this.mFOVy = args.fov;
		}

		if ("additive" in args)
		{
			this.mAdditive = args.additive;
		}

		if ("alphaBlended" in args)
		{
			this.mAlphaBlended = args.alphaBlended;
		}

		if ("queryFlags" in args)
		{
			this.mQueryFlags = args.queryFlags;
		}

		if ("debug" in args)
		{
			this.mDebug = args.debug;
		}
	}

	function _effectStart()
	{
		if (!this.mTarget)
		{
			this.stop();
			return;
		}

		this.mTimer = this.Timer();
		this.mProjector = this._scene.createTextureProjector(this.mNameRandomizer.nextInt().tostring() + "/ProjectorEffect", this.mTextureName);
		this.mProjector.setNearClipDistance(this.mNearClipDistance);
		this.mProjector.setFarClipDistance(this.mFarClipDistance);

		if (this.mAdditive)
		{
			this.mProjector.setAdditive(true);
		}
		else if (this.mAlphaBlended)
		{
			this.mProjector.setAlphaBlended(true);
		}

		if (this.mOrthographic)
		{
			this.mProjector.setOrthoWindow(this.mOrthoWidth, this.mOrthoHeight);
		}
		else
		{
			this.mProjector.setFOVy(this.mFOVy);
		}

		if (this.mDebug)
		{
			this.mProjector.setDrawingFrustum(true);
		}

		this.mProjector.setProjectionQueryMask(this.mQueryFlags);
		this.mProjector.setVisibilityFlags(this.VisibilityFlags.ANYTHING);

		if (this.mTargetBone)
		{
			this.mTarget.attachObjectToBone(this.mTargetBone, this.mProjector);
		}
		else
		{
			this.mProjectorNode = this._scene.getRootSceneNode().createChildSceneNode();
			this.mProjectorNode.attachObject(this.mProjector);
			local pos = this.mTarget;

			if ("getNode" in this.mTarget)
			{
				pos = this.mTarget.getNode().getWorldPosition();
			}

			local dir = this.Vector3(0, -1, 0);

			if (this.mOffset)
			{
				pos = pos + this.mOffset;
			}

			if (this.mTargetDir)
			{
				dir = this.mTargetDir;
			}

			this.mProjectorNode.setPosition(pos);
			this.mProjectorNode.lookAt(pos + dir);
			pos = this.mProjectorNode.getWorldPosition();
			local pos2 = this.getObject().getNode().getWorldPosition();
			this._scene.getRootSceneNode().removeChild(this.mProjectorNode);
			this.getObject().getNode().addChild(this.mProjectorNode);
			this.mProjectorNode.setPosition(pos - pos2);
		}
	}

	function _effectStop()
	{
		local elapsed = this.mTimer.getMilliseconds();

		if (elapsed >= this.mDuration || this.mFadeOut <= 0)
		{
			this.EffectBase._effectStop();
		}
		else if (this.mFadeOut > this.mDuration - elapsed)
		{
			this.mStopTime = -1;
		}
		else
		{
			this.mStopTime = elapsed;
		}
	}

	function _effectFrame()
	{
		local elapsed = this.mTimer.getMilliseconds();

		if (this.mState == this.STOPPING && this.mStopTime > 0)
		{
			elapsed = this.mDuration - this.mFadeOut + (elapsed - this.mStopTime);
		}

		if (this.mDuration > 0 && elapsed >= this.mDuration)
		{
			if (this.mState == this.STOPPING)
			{
				this.destroy();
			}
			else
			{
				this.finish();
			}

			return;
		}

		if (this.mTargetBone)
		{
			return;
		}

		local strenght = 1.0;

		if (this.mFadeIn > elapsed)
		{
			strenght = elapsed / this.mFadeIn.tofloat();
		}
		else if (this.mFadeOut > this.mDuration - elapsed)
		{
			strenght = (this.mDuration - elapsed) / this.mFadeOut.tofloat();
		}

		this.mProjector.setStrength(strenght);
	}

	function _effectDestroy()
	{
		this.mTimer = null;

		if (this.mTargetBone)
		{
			this.mTarget.detachObjectFromBone(this.mTargetBone, this.mProjector);
		}
		else
		{
			this._scene.getRootSceneNode().removeChild(this.mProjectorNode);
		}

		this.mProjector.destroy();
		this.mProjectorNode.destroy();
	}

}

class this.Effect.LinearTranslation extends this.EffectBase
{
	static mEffectName = "LinearTranslation";
	mSource = null;
	mDirection = null;
	mDuration = 3.0;
	mAccTime = 0.0;
	mDeaccTime = 1.0;
	mTopSpeed = 4.0;
	mEvent = "onContact";
	mTimer = null;
	function _parseArgs( args )
	{
		if ("source" in args)
		{
			this.mSource = args.source;
		}

		if ("event" in args)
		{
			this.mEvent = args.event;
		}

		if ("topSpeed" in args)
		{
			this.mTopSpeed = args.topSpeed;
		}

		if ("direction" in args)
		{
			this.mDirection = args.direction;
		}

		if ("duration" in args)
		{
			this.mDuration = args.duration;
		}

		if ("accTime" in args)
		{
			this.mAccTime = args.accTime;
		}

		if ("deaccTime" in args)
		{
			this.mDeaccTime = args.deaccTime;
		}
	}

	function _effectStart()
	{
		if (!this.mSource)
		{
			this.stop();
			return;
		}

		local pos = this.mSource.getNode().getWorldPosition();
		local node = this.getObject().getNode();
		local parentNode = node.getParent();

		if (parentNode)
		{
			parentNode.removeChild(node);
		}

		::_scene.getRootSceneNode().addChild(node);
		this.mSource.setPosition(pos);
		this.mTimer = this.Timer();
	}

	function _effectFrame()
	{
		local obj = this.mSource;

		if (!obj.getNode())
		{
			this.finish();
			return;
		}

		local pos = obj.getPosition();
		local secs = this.mTimer.getMilliseconds() / 1000.0;
		local timePerc = secs / this.mDuration;

		if (timePerc >= 1.0)
		{
			local script = this.EffectScript._getParentScript(this);

			if (script)
			{
				script.fireIn(0, this.mEvent);
			}

			this.finish();
			return;
		}

		local speed = this.mTopSpeed;

		if (timePerc < this.mAccTime)
		{
			speed = this.mTopSpeed * (timePerc / this.mAccTime);
		}
		else if (timePerc > this.mDeaccTime)
		{
			speed = this.mTopSpeed * ((1 - timePerc) / (1 - this.mDeaccTime));
		}

		if (speed > this.mTopSpeed)
		{
			speed = this.mTopSpeed;
		}

		pos += this.mDirection * speed;
		obj.setPosition(pos);
	}

	function _effectDestroy()
	{
		this.mTimer = null;
	}

}

