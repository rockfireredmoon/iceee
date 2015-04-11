this.Item <- {};
this.Item._NextID <- 1;
class this.Item.Basic extends this.MessageBroadcaster
{
	constructor( pID )
	{
		::MessageBroadcaster.constructor();
		this.mID = pID;
		this.mDef = {};
	}

	function getObjectClass()
	{
		return this.mObjectClass;
	}

	function _tostring()
	{
		return this.getNodeName();
	}

	function getID()
	{
		return this.mID;
	}

	function getName()
	{
		return this.getNodeName();
	}

	function getNodeName()
	{
		return this.mObjectClass + "/" + this.getID();
	}

	function getDef()
	{
		return this.mDef;
	}

	function onUpdate()
	{
		this.broadcastMessage("onUpdate");
	}

	mID = -1;
	mObjectClass = "Item.Basic";
	mLastServerUpdate = null;
	mDef = {};
}

class this.Item.Attachable extends this.Item.Basic
{
	constructor( pID, ... )
	{
		this.mColors = {};

		if (pID == null)
		{
			this.Item._NextID += 1;
			pID = this.Item._NextID;
			this.mObjectClass = "Attachment";
		}
		else
		{
			this.mObjectClass = "Item";
		}

		this.Item.Basic.constructor(pID);

		if (vargc > 0)
		{
			if (this.type(vargv[0]) == "table")
			{
				this.setMeshName(vargv[0].type);
				this.setAttachmentPointName(vargv[0].node);

				if ("colors" in vargv[0])
				{
					this.setColors(vargv[0].colors);
				}
			}
			else
			{
				if (vargc > 0)
				{
					this.setMeshName(vargv[0]);
				}

				if (vargc > 1)
				{
					this.setAttachmentPointName(vargv[1]);
				}

				if (vargc > 2)
				{
					this.setColors(vargv[2]);
				}

				if (vargc > 3)
				{
					this.setEffectName(vargv[3]);
				}
			}
		}

		this.setEffectsHandler(this.EffectsHandler(this));
	}

	function isAssembled()
	{
		return this.mAssembled;
	}

	function _setAssembled( value )
	{
		if (value == this.mAssembled)
		{
			return;
		}

		this.mAssembled = value;
	}

	function assemble()
	{
		if (this.mAssembler)
		{
			this.mAssembler.assemble(this);
			this.broadcastMessage("onAssembled");
		}
	}

	function disassemble()
	{
		if (this.mAssembler)
		{
			this.broadcastMessage("onDisassemble");
			this.mAssembler.disassemble(this);
		}
	}

	function reassemble( ... )
	{
		if (this.mAssembled)
		{
			this.disassemble();
		}

		this.assemble();
	}

	function destroy()
	{
		if (this.mEffectsHandler)
		{
			this.mEffectsHandler.destroy();
			this.mEffectsHandler = null;
		}

		if (this.mAssembler)
		{
			this.disassemble();
			this.mAssembler.removeManagedInstance(this);
		}
	}

	function setEffectsHandler( effectsHandler )
	{
		this.mEffectsHandler = effectsHandler;
	}

	function getEffectsHandler()
	{
		return this.mEffectsHandler;
	}

	function setAttachmentPointName( attachmentPointName )
	{
		this.mAttachmentPointName = attachmentPointName;

		if (this.mAssembled)
		{
			this.reassemble();
		}
	}

	function getAttachmentPointName()
	{
		return this.mAttachmentPointName;
	}

	function setColors( hexColorTable )
	{
		this.mColors = hexColorTable;

		if (this.mAssembled)
		{
			this.reassemble();
		}
	}

	function getColors()
	{
		return this.deepClone(this.mColors);
	}

	function setEffectName( vEffectName )
	{
		this.mEffectName = vEffectName;

		if (this.mAssembled)
		{
			this.reassemble();
		}
	}

	function getEffectName()
	{
		return this.mEffectName;
	}

	function getRibbonSettings()
	{
		if (this.mMeshName in ::AttachableDef)
		{
			local def = ::AttachableDef[this.mMeshName];

			if ("ribbon" in def)
			{
				return def.ribbon;
			}
		}

		return null;
	}

	function setMeshName( meshName )
	{
		this.mMeshName = meshName;
		local wasAssembled = this.mAssembled;

		if (this.mAssembler)
		{
			if (this.mAssembled)
			{
				this.disassemble();
			}

			this.mAssembler.removeManagedInstance(this);
		}

		this.mAssembler = this.GetAssembler("Attachment", meshName);
		this.mAssembler.addManagedInstance(this);

		if (wasAssembled)
		{
			this.reassemble();
		}
	}

	function getMeshName()
	{
		return this.mMeshName;
	}

	function getAttachPointDef( name )
	{
		return null;
	}

	function setAttachedTo( so, ... )
	{
		this.mAttachedTo = so;

		if (vargc > 0)
		{
			this.setAttachmentPointName(vargv[0]);
		}

		if (this.mAssembled)
		{
			this.reassemble();
		}
	}

	function getAttachedTo()
	{
		return this.mAttachedTo;
	}

	function getIlluminated()
	{
		if (this.mMeshName in ::AttachableDef)
		{
			local def = ::AttachableDef[this.mMeshName];

			if ("illuminated" in def)
			{
				return def.illuminated;
			}
		}

		return false;
	}

	function getAnimated()
	{
		if (this.mMeshName in ::AttachableDef)
		{
			local def = ::AttachableDef[this.mMeshName];

			if ("animated" in def)
			{
				return def.animated;
			}
		}

		return false;
	}

	function getParticleSystem()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getParticleSystem(this);
		}

		return null;
	}

	function getEntity()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getBaseEntity(this);
		}

		return null;
	}

	function getWeaponType()
	{
		if (this.mWeaponType == null)
		{
			if (this.mMeshName.find("Shield", 0) != null)
			{
				return "Shield";
			}

			if (this.mMeshName.find("-2h", 0) != null || this.mMeshName.find("-Two_Handed", 0) != null)
			{
				return "2h";
			}

			if (this.mMeshName.find("-Staff", 0) != null || this.mMeshName.find("-Spear", 0) != null)
			{
				return "Staff";
			}

			if (this.mMeshName.find("-Bow", 0) != null)
			{
				return "Bow";
			}

			if (this.mMeshName.find("-Wand", 0) != null)
			{
				return "Wand";
			}

			if (this.mMeshName.find("-Dagger", 0) != null || this.mMeshName.find("-Katar", 0) != null || this.mMeshName.find("-Claw", 0) != null)
			{
				return "Small";
			}

			return "1h";
		}

		return this.mWeaponType;
	}

	function getTextureName()
	{
		if (("texture" in this.mAssemblyData) && "getName" in this.mAssemblyData.texture)
		{
			return this.mAssemblyData.texture.getName();
		}

		return null;
	}

	mAssembler = null;
	mAssemblyData = null;
	mAttachmentPointName = null;
	mAssembled = false;
	mMeshName = null;
	mWeaponType = null;
	mAttachedTo = null;
	mColors = {};
	mEffectName = null;
	mEffectsHandler = null;
}

