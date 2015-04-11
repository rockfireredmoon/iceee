this.require("Assemblers/Assembler");
this.nextRibbonTrailID <- 0;
class this.Assembler.Attachment extends this.Assembler.Factory
{
	mItemName = null;
	mPoint = null;
	mEntity = null;
	mTexture = null;
	MIPS = 1;
	constructor( type )
	{
		this.Assembler.Factory.constructor("Item", type);
	}

	function destroy()
	{
		this.mTexture = null;
	}

	function _resetAssemblyData( io )
	{
		io.mAssemblyData = {
			ribbon = null,
			entities = {}
		};
	}

	function _assemble( io )
	{
		local so = io.getAttachedTo();

		if (so == null)
		{
			return false;
		}

		if (so.mAssembler == null)
		{
			return false;
		}

		local pointName = io.getAttachmentPointName();

		if (pointName in so.mAssembler.mHiddenAttachPoints)
		{
			return false;
		}

		this.mItemName = io.getMeshName();

		if (!this.mItemName)
		{
			this.log.debug("Assembler not ready yet: " + this.getAssemblerName());
			return false;
		}

		this.log.debug("Assembling " + io.getName() + " using " + this.getAssemblerName());
		this._resetAssemblyData(io);
		this._assemblePointData(so, io);
		this._assembleTexture(io);
		this._assembleItem(so, io);
		io._setAssembled(true);

		if (io.getEffectsHandler())
		{
			io.getEffectsHandler().onAssembled();
		}

		return true;
	}

	function _invalidateTextures()
	{
		this.mTexture = null;
	}

	static function _fileBasename( filename )
	{
		local pos = filename.find(".");

		if (pos == null)
		{
			return filename;
		}

		return filename.slice(0, pos);
	}

	static function _deriveTintMapName( filename )
	{
		return this._fileBasename(filename) + "-Tint.png";
	}

	function _resolveTextureName( tex )
	{
		local texName;

		if (typeof tex == "string")
		{
			texName = this.mTexture == null ? this.mItemName + ".png" : tex + ".png";
		}
		else if (tex instanceof this.ProceduralTexture)
		{
			texName = tex.getName();
		}
		else
		{
			throw this.Exception("Unsupported texture type/name: " + tex);
		}

		return texName;
	}

	function _assembleTexture( io )
	{
		local baseTex;
		this.mTexture = null;

		if ("colors" in this.mPoint)
		{
			if (this.mPoint.colors != null && baseTex == null)
			{
				baseTex = this.mItemName;
			}

			if (baseTex != null)
			{
				try
				{
					this.mTexture = ::_root.createProceduralTexture(io.getName() + "/PTexture", baseTex + ".png");

					if (this.mPoint.colors)
					{
						local colors = this.ColorPalette.arrayToTable(this.mPoint.colors);
						this.mTexture.colorizeRegions(this._deriveTintMapName(baseTex), colors);
					}
				}
				catch( err )
				{
					this.log.debug(err);
				}
			}
		}
		else
		{
			this.mTexture = baseTex;
		}

		io.mAssemblyData.texture <- this.mTexture;
	}

	function _createEntity( io, name, mesh, diffuseTexture )
	{
		if (name in io.mAssemblyData.entities)
		{
			throw this.Exception("Recreating entity " + name + " for Item " + io.getName());
		}

		local entityName = io.getAttachedTo().getNodeName() + "/" + name;
		local e = this._scene.createEntity(entityName, mesh);
		e.setVisibilityFlags(this.VisibilityFlags.ATTACHMENT | this.VisibilityFlags.ANY | this.VisibilityFlags.LIGHT_GROUP_ANY);

		if (diffuseTexture)
		{
			local texName = this._resolveTextureName(diffuseTexture);

			if (io.getIlluminated())
			{
				e.applyTextureAliases({
					Diffuse = texName,
					Illuminated = this.mItemName + "-Illum.png"
				});
			}
			else
			{
				e.applyTextureAliases({
					Diffuse = texName
				});
			}
		}

		if (io.getAnimated())
		{
			e.setAnimationUnitCount(1);
			e.getAnimationUnit(0).setIdleState("Idle");
			e.getAnimationUnit(0).setEnabled(true);
		}

		io.mAssemblyData.entities[name] <- e;
		return e;
	}

	function _assemblePointData( so, io )
	{
		this.mPoint = {};
		local pointName = io.getAttachmentPointName();
		local attachable = io.getMeshName();

		if (attachable)
		{
			if (attachable in ::AttachableDef)
			{
				local adef = ::AttachableDef[attachable];

				if ("attachPoints" in ::AttachableDef[attachable])
				{
					local found = false;

					foreach( i, x in adef.attachPoints )
					{
						if (x == pointName)
						{
							found = true;
						}
					}

					if (found)
					{
						if ("attachAliases" in adef)
						{
							if (pointName in adef.attachAliases)
							{
								pointName = adef.attachAliases[pointName];
							}
						}
					}
					else
					{
						this.log.error(pointName + " not a valide attachment point of item " + attachable);
					}
				}
				else
				{
					this.log.error(attachable + " does not have it\'s attachment points set");
				}
			}
			else
			{
				this.log.error(attachable + " not found in AttachableDef");
			}
		}

		if (so.mAssembler.mAttachmentPointSet)
		{
			if (pointName in so.mAssembler.mAttachmentPointSet)
			{
				this.Util.overrideSlots(this.mPoint, so.mAssembler.mAttachmentPointSet[pointName]);
			}
		}

		if (attachable)
		{
			this.mPoint.entity <- attachable;
			this.mPoint.texture <- attachable;

			if (attachable in ::AttachableDef)
			{
				local adef = ::AttachableDef[attachable];

				if ("mesh" in adef)
				{
					this.mPoint.entity <- adef.mesh;
				}

				if ("entity" in adef)
				{
					this.mPoint.entity <- adef.entity;
				}

				if ("texture" in adef)
				{
					this.mPoint.texture <- adef.texture;
				}

				if (("particle" in adef) && adef.particle == true)
				{
					this.mPoint.particle <- true;
				}

				if ("colors" in adef)
				{
					this.mPoint.colors <- this.deepClone(adef.colors);
				}

				local ioColors = io.getColors();

				if (this.type(ioColors) == "table")
				{
					ioColors = this.ColorPalette.tableToArray(ioColors);
				}

				if (ioColors.len() != 0)
				{
					foreach( i, color in ioColors )
					{
						if (("colors" in this.mPoint) && i < this.mPoint.colors.len())
						{
							this.mPoint.colors[i] = ioColors[i];
						}
					}
				}
			}
		}
	}

	function _assembleItem( so, io )
	{
		local entity = so.mAssembler.getBaseEntity(so);

		if (!("bone" in this.mPoint))
		{
			this.log.warn("Unable to resolve attachment (" + io.getAttachmentPointName() + ") for " + so.mAssembler);
			return;
		}

		if ("entity" in this.mPoint)
		{
			local name = io.getName() + "/Entity";

			try
			{
				if ("particle" in this.mPoint)
				{
					this.mEntity = this._scene.createParticleSystem(name, this.mPoint.entity);
				}
				else
				{
					this.mEntity = this._createEntity(io, name, this.mPoint.entity + ".mesh", this.mTexture);
				}

				entity.attachObjectToBone(this.mPoint.bone, this.mEntity);
				so.onAttachmentPointChanged(this.mEntity);
			}
			catch( error )
			{
				this.log.debug(error);

				if (name in io.mAssemblyData.entities)
				{
					delete io.mAssemblyData.entities[name];
				}

				if (this.mEntity)
				{
					this.mEntity.destroy();
					this.mEntity = null;
				}

				return;
			}

			if ("position" in this.mPoint)
			{
				this.mEntity.getParentNode().translate(this.mPoint.position);
			}

			if ("orientation" in this.mPoint)
			{
				this.mEntity.getParentNode().setOrientation(this.mPoint.orientation);
			}
		}

		local ribbonSettings = io.getRibbonSettings();

		if (ribbonSettings != null)
		{
			local ribbon = ::_scene.createRibbon();

			try
			{
				ribbon.setMaterial("material" in ribbonSettings ? ribbonSettings.material : "LightRibbonTrail");
				ribbon.setInitialColor("initialColor" in ribbonSettings ? this.Color(ribbonSettings.initialColor) : this.Color(0.30000001, 0.30000001, 0.30000001, 1.0));
				ribbon.setColorChange("colorChange" in ribbonSettings ? this.Color(ribbonSettings.colorChange[0], ribbonSettings.colorChange[1], ribbonSettings.colorChange[2], ribbonSettings.colorChange[3]) : this.Color(2.5, 2.5, 2.5, 2.0));
				ribbon.setMaxSegments("maxSegments" in ribbonSettings ? ribbonSettings.maxSegments.tointeger() : 32);
				ribbon.setInitialWidth("width" in ribbonSettings ? ribbonSettings.width.tofloat() : this.mEntity.getBoundingBox().getSize().y);
				ribbon.setOffset("offset" in ribbonSettings ? ribbonSettings.offset.tofloat() : 0.0);
				ribbon.setActive(false);
				this.mEntity.getParentSceneNode().attachObject(ribbon);
				so.onAttachmentPointChanged(ribbon);
				ribbon.setTrackedMovableObject(this.mEntity);
				io.mAssemblyData.ribbon <- ribbon;
			}
			catch( err )
			{
				this.log.debug("Error assembling ribbon: " + err);
				ribbon.destroy();
			}
		}

		local name = io.getEffectName();

		if (name != null)
		{
			local pname = io.getName() + "/PARTICLE_EFFECT";

			try
			{
				local particleSystem = this._scene.createParticleSystem(pname, name);
				this.log.debug("attachObject(particleSystem)");
				entity.attachObjectToBone(this.mPoint.bone, particleSystem);
				io.mAssemblyData.entities[pname] <- particleSystem;

				if ("position" in this.mPoint)
				{
					particleSystem.getParentNode().translate(this.mPoint.position);
				}

				if ("orientation" in this.mPoint)
				{
					particleSystem.getParentNode().setOrientation(this.mPoint.orientation);
				}
			}
			catch( error )
			{
				this.particleSystem.destroy();
				return;
			}
		}

		if (this.mEntity && "scale" in this.mPoint)
		{
			this.mEntity.getParentNode().setScale(this.mPoint.scale);
		}
	}

	function disassemble( io )
	{
		if (io.getEffectsHandler())
		{
			io.getEffectsHandler().onDisassembled();
		}

		if (io == null)
		{
			throw this.Exception("Assembler.Attachment: A null item object was passed to dissasemble!");
		}

		if ("entities" in io.mAssemblyData)
		{
			foreach( i, x in io.mAssemblyData.entities )
			{
				if (x != null && ((x instanceof this.Entity) || (x instanceof this.ParticleSystem)))
				{
					x.destroy();
				}
			}
		}

		if ("ribbon" in io.mAssemblyData)
		{
			if (io.mAssemblyData.ribbon)
			{
				io.mAssemblyData.ribbon.destroy();
				io.mAssemblyData.ribbon = null;
			}
		}

		this._resetAssemblyData(io);
		io._setAssembled(false);
	}

	function getParticleSystem( io )
	{
		local name = io.getEffectName();

		if (name)
		{
			return io.mAssemblyData.entities["Attachment/" + io.getID() + "/PARTICLE_EFFECT"];
		}
		else
		{
			return null;
		}
	}

	function getBaseEntity( io )
	{
		local name = "Attachment/" + io.getID() + "/Entity";
		local entities = io.mAssemblyData.entities;

		if (name in entities)
		{
			return entities[name];
		}

		return null;
	}

}

