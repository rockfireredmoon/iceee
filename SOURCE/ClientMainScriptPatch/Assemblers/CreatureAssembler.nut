this.require("Assemblers/Assembler");

// TODO: Remove this once the assets have been converted over...
::EntityDef <- {};

this._minimapFilterCreatureCategory <- "";
this._minimapFilterShopkeepers <- true;
class this.Assembler.Creature extends this.Assembler.Factory
{
	/**
		The stages of the assemble process
	*/
	ASSEMBLE_PREPARE = 0;
	ASSEMBLE_BODY = 1;
	ASSEMBLE_HEAD = 2;
	ASSEMBLE_DETAILS = 3;
	ASSEMBLE_ATTACHMENTS = 4;
	ASSEMBLE_FINALIZE = 5;
	ASSEMBLE_DONE = 6;
	
	/**
		The default creature appearance
	*/
	static DEFAULT_APPEARANCE = "n4:{[\"c\"]=\"Horde-Shroomie\",[\"sk\"]={[\"cap_bottom\"]=\"000BBF\",[\"body\"]=\"FF1685\",[\"eyes\"]=\"000000\",[\"cap\"]=\"FF1685\"}}";
	
	/**
		The amount of time in milliseconds to wait before moving to the next assembly step
	*/
	ASSEMBLE_FREQUENCY = 50;
	
	/**
		The amount of time to wait while before processing the next 'substep' of an assembly step
		(Like processing a single layer of clothing)
	*/
	ASSEMBLE_SUBFREQUENCY = 25;

	/**
		this is here until there is an easier way to lookup the race definition
		of a biped based character.
	*/
	mRaceGenderDefName = null;

	/**
		This is the base texture to use on the body mesh.
		Things like recolorization and clothing are layered
		on top of this to form the final texture.
	*/
	mBodyBaseTexture = null;

	/**
		If there is a separate head part to this creature
		then this holds the base head texture name.
	*/
	mBody = null;

	/**
		If there is a separate head part, this holds the
		name of the mesh to use for the head (sans .mesh).
	*/
	mHeadBaseTexture = null;
	mHead = null;
	
	/**
		The name of a click mesh to use
	*/
	mClickMesh = null;
	mClickMeshOffset = null;
	mClickMeshScale = null;

	/**
		If there is a separate tail texture for this creature
		(this assumes mTail is also set), this holds that
		texture, otherwise the head texture is used.
	*/
	mTailBaseTexture = null;

	/**
		If there is a tail mesh, this holds the name of the
		mesh (sans .mesh) of the tail. It will be attached
		to the Bone-TailEnd bone and uses the same texture
		as the head by default.
	*/
	mTail = null;
	mSkinColors = null;
	mClothing = null;
	
	/**
		Waits for the creature content def to load
	*/
	mContentDefWaiter = null;
	
	/**
		Waits for the clothing defs to load
	*/
	mClothingDefWaiter = null;
	
	/**
		Waits for attachment and detail defs to load
	*/
	mAttachmentDefWaiter = null;

	/**
		A general scale factor. This is applied to the scene node
		for every creature created (used to scale the body mesh for
		things like race, etc).
	*/
	mSize = 1.0;

	/**
		If using a separate forearm mesh (e.g. PCs) then this is the
		name of the mesh (minus the .mesh) to use for the left forearm.
	*/
	mLeftForearm = null;
	
	/**
		The name of the texture to use for the forearm mesh. This
		defaults to "body" to use the standard body mesh, and is
		ignored if the corresponding mesh is not set (which it's
		not by default).
	*/
	mLeftForearmTexture = "body";

	/**
		If using a separate forearm mesh (e.g. PCs) then this is the
		name of the mesh (minus the .mesh) to use for the left forearm.
	*/
	mRightForearm = null;

	/**
		The name of the texture to use for the forearm mesh. This
		defaults to "body" to use the standard body mesh, and is
		ignored if the corresponding mesh is not set (which it's
		not by default).
	*/
	mRightForearmTexture = "body";

	/**
		Attachments are arbitrary mesh-to-bone links. It creates
		a new entity and hooks into a bone (optionally with a
		certain offset or orientation tweak). See addAttachment()
		for what these entries look like.
	*/
	mAttachments = null;
	
	/**
		Attachment points map a bone/position/scale/orientation
		to a named location. For instance, "head" could be
		{ bone="Bone-Head", scale=Vector3(1,1,.8) }. The only
		required element is the bone name.
		<p>
		This is not the same as a Skeleton, though it is highly
		correlated with it. It maps the differences between
		various skeletons and their constituent body meshes.
		The female body mesh, for instance, is slightly slimmer
		but uses the same skeleton, so we need to capture the
		different "surface" parameters such that the attachments
		still look okay.
		<p>
		An assembler will generally just pick one of the predefined
		attachment point sets (one for each skeleton/body type).
	*/
	mAttachmentPointSet = null;
	
	/**
		Details are attachments that are generally not going to be
		removed (such as ears or horns). See addAttachment() for the
		structure of the entires in this list.
	*/
	mDetails = null;
	
	/**
		This is the procedural texture of the body that's built
		up to hold all of the colorizations and color options for
		creatures of this type. (It's NOT set via accessors, its
		the "final" state of the texture.)
	*/
	mBodyTexture = null;
	
	/**
		This is the procedural texture of the head that's built
		up to hold all of the colorizations and color options for
		creatures of this type. (It's NOT set via accessors, its
		the "final" state of the texture.)
	*/
	mHeadTexture = null;
	
	/** Sometimes we store multiple models inside of 1 mesh file.  Each part
	    will need a different diffuse texture set on its part.*/
	mMultipartMesh = false;
	
	/**
		If this was configured via configure() this stores the last
		value passed to that function.
	*/
	mConfig = null;
	
	/**
		Stores the equipment def list for the creature
	*/
	mEquipmentDef = null;
	
	/**
		Whether or not the chest clothing overrides the legs
	*/
	mChestOverride = false;
	
	/**
		Stores a list of the attachment points that are not to be assembled
		because of items attached to the body.
	*/
	mHiddenAttachPoints = null;
	
	/**
		A list of items that have been requested from the server for assembly
	*/
	mRequestedItems = null;
	
	/**
		Number of mip maps to generate for blended textures.
	*/
	MIPS = 1;
	
	/**
		Whether or not the clothing needs to be rendered for the object
	*/
	mNeedClothes = false;
	
	/**
		Whether or not the configuration has been completed
	*/
	mConfigured = false;
	
	/**
		The index of the next clothing layer to process
	*/
	mClothingIndex = 0;
	
	/**
		Whether or not the skin has been assembled
	*/
	mSkinAssembled = false;
	
	/**
		Whether or not the clothing has been assembled
	*/
	mClothingAssembled = false;
	
	/**
		If an error occurred while loading content for a creature,
		this will be set to the error message.
	*/
	mContentLoadError = null;
	
	/**
		Whether or not all required packages have been loaded and we're prepared to assemble
	*/
	mPrepared = null;
	
	/**
		Some creatures are suppose to look like props.  In that case, they use the scenery assembler to assemble
		instead of the normal creature assembler
	*/
	mSceneryAssembler = null;
	
	constructor( type )
	{
		this.Assembler.Factory.constructor("Creature", type);
		::_ItemDataManager.addListener(this);

		try
		{
			::_Connection.sendInspectCreatureDef(type.tointeger());
		}
		catch( err )
		{
			// Bails on a non-integer type, which are basically
			// the temporary preview types the demo panel uses...
		}
	}

	function getBoundingRadius( so )
	{
		if (this.mSceneryAssembler)
			return this.mSceneryAssembler.getBoundingRadius(so);

		return this.Assembler.Factory.getBoundingRadius(so);
	}

	function onItemDefUpdated( itemDefId, itemDef )
	{
		if (this.mRequestedItems)
		{
			foreach( k, v in this.mRequestedItems )
			{
				if (v == itemDefId)
				{
					this.mConfigured = false;
					this.reassembleInstances();
				}
			}
		}
	}

	function getSceneryAssembler()
	{
		return this.mSceneryAssembler;
	}

	function getShowNameType()
	{
		if (this.mSceneryAssembler)
		{
			return false;
		}

		return this.mShowNameType;
	}

	function reset()
	{
		this.mBodyBaseTexture = null;
		this.mClickMesh = null;
		this.mClickMeshOffset = null;
		this.mClickMeshScale = null;
		this.mBody = null;
		this.mHeadBaseTexture = null;
		this.mHead = null;
		this.mTailBaseTexture = "head";
		this.mTail = null;
		this.mSkinColors = null;
		this.mClothing = null;
		this.mSize = 1.0;
		this.mLeftForearm = null;
		this.mLeftForearmTexture = "body";
		this.mRightForearm = null;
		this.mRightForearmTexture = "body";
		this.mAttachments = null;
		this.mAttachmentPointSet = null;
		this.mDetails = null;
		this.mBodyTexture = null;
		this.mHeadTexture = null;
		this.mRequiredArchives = [];
		this.mConfigured = false;
		this.mSkinAssembled = false;
		this.mClothingAssembled = false;
		this.mContentLoadError = null;
		this.mClothingIndex = 0;
		this.mPrepared = null;
		this.mRequiredResources = null;
		
		// Resets required archives and error flags
		this.Assembler.Factory.reset();
	}

	function destroy()
	{
		try
		{
			local id = this.mObjectType.tointeger();
			::_creatureDefManager.removeCreatureDef(id);
		}
		catch( err )
		{
		}

		::_ItemDataManager.removeListener(this);
		this.reset();
	}

	function getSize()
	{
		return this.mSize;
	}

	function getConfig()
	{
		return this.mConfig;
	}

	function getReady()
	{
		return this.mConfig != null;
	}

	function setBody( mesh, ... )
	{
		if (mesh != this.mBody)
		{
			this.mBody = mesh;

			if (vargc > 0)
				this.mBodyBaseTexture = vargv[0];
			else
				this.mBodyBaseTexture = null;

			this._invalidateTextures();
		}
	}

	function getBody()
	{
		return this.mBody;
	}

	function setHead( mesh, ... )
	{
		if (mesh != this.mHead)
		{
			this.mHead = mesh;

			if (vargc > 0)
				this.mHeadBaseTexture = vargv[0];
			else
				this.mHeadBaseTexture = null;

			this._invalidateTextures();
		}
	}

	function setHeadBaseTextureName( texture )
	{
		this.mHeadBaseTexture = texture;
	}

	function setTail( mesh, ... )
	{
		return;
		this.mTail = mesh;

		if (vargc > 0)
			this.mTailBaseTexture = vargv[0];
		else
			this.mTailBaseTexture = null;

		this._invalidateTextures();
	}

	function setSize( size )
	{
		this.mSize = size;
	}

	function setSkinColors( colors )
	{
		this.Assert.isTable(colors);
		this.mSkinColors = colors;
		this._invalidateTextures();
	}

	/**
		Set the valid attachment points defined for this creature.
		See the documentation for mAttachmentPointSet for details
		regarding the structure of this.
		<p>
		This is essentially the "Body Type" or standard set of
		attachments.
	*/
	function setAttachmentPointDefaults( attachmentPointDefaults )
	{
		this.mAttachmentPointSet = attachmentPointDefaults;
	}

	/**
		Configure a PC style character (using algorithm C1). This one uses
		a lot of data from the ContentDef table to further define the look
		of a creature.

		@param race
				The "race" for this creature (determines body type
				and attachment point configurations). It must be one of
				the keys in ::Races. (Example: "a")

		@param gender
				Either 'm' (male) or 'f' (female).

		@param scale
				The relative size of the creature (between .5 and 2.5).
				If this is null, it will default to 1.0.

		@param variantChoices
				This is a table containing detail "variants". i.e. for
				each "section" in a ContentDef's Detail table, this will
				point to one of the keys in that table. If a key is not
				in this table for a given section, "default" will be tried,
				if no default is found, that section will be skipped.

		@param skinColors
				A table with skin recolorization settings. If null, this
				will be omitted.

		@param clothing
				A table with clothing slot settings. If null, no clothing
				will be set.


	*/
	function setViaContentDef1( race, gender, scale, variantChoices, skinColors, clothing )
	{
		if (gender != "m" && gender != "f")
			throw this.Exception("Invalid gender: " + gender);

		if (!(race in ::Races))
			throw this.Exception("Invalid race: " + race);

		gender = gender == "f" ? "Female" : "Male";
		race = ::Races[race];
		local r_g = race + "_" + gender;
		this.mRaceGenderDefName = "Biped-" + r_g;

		if (!this._checkContentDefLoaded(this.mRaceGenderDefName))
			return;

		if (scale < 0.1)
			scale = 0.1;
		else if (scale > 10.0)
			scale = 10.0;

		this.mSize = scale;
		
		this.setBody("Biped-" + gender, "Biped-" + r_g + "-Body");
		
		this._checkHelmet();
		
		this.setHead("Biped-" + r_g + "-Head", "Biped-" + r_g + "-Head");
		this.setTail("Biped-" + r_g + "-Tail");
		
		this.mAttachmentPointSet = null;
		local key = "Biped." + gender + "." + race;

		if (key in ::AttachmentPointSet)
		{
			this.log.warn("Use of AttachmentPointSet[" + key + "] is deprecated. Add it to ContentDef");
			this.setAttachmentPointDefaults(::AttachmentPointSet[key]);
		}


		// All PCs have forearms. Currently we just derive from
		// the gender, but we could just as easily have custom
		// arms -- beefy/skinny/etc, or for some huge gauntleted
		// hands with fins and spikes and whatnot.
		// TODO: In fact, we can just move this into a "detail section"
		// and allow selection between them that way.
		this.addDetail({
			name = "left_forearm",
			mesh = "Biped-" + gender + "-Forearm_Left.mesh",
			bone = "Bone-LeftForearm",
			texture = "body"
		});
		this.addDetail({
			name = "right_forearm",
			mesh = "Biped-" + gender + "-Forearm_Right.mesh",
			bone = "Bone-RightForearm",
			texture = "body"
		});

		// Any extra details according to the predefined Race/Gender
		// setup? Things like ears, etc.
		key = "Biped-" + r_g;

		if (key in ::ContentDef)
		{
			local def = ::ContentDef[key];

			if ("SizeMultiplier" in def)
				this.mSize *= def.SizeMultiplier;

			if ("AttachmentPoints" in def)
				this.setAttachmentPointDefaults(def.AttachmentPoints);

			if ("Details" in def)
			{
				local section;
				local variants;

				foreach( section, sectionVariants in def.Details )
				{
					local choice = "default";

					// If there is an explicit choice for this section,
					// use that instead.
					if (variantChoices != null && section in variantChoices)
						choice = variantChoices[section];

					if (choice in sectionVariants)
					{
						local detail = sectionVariants[choice];
						
						// Now, we've got one, but each value here can be one of
						// three things:
						//   1. null -- Don't do anything (an explicit "none")
						//   2. A single table -- This is the detail spec
						//   3. An array -- Multiple detail specs, attach each.

						if (typeof detail == "array")
						{
							// An array of details (for instance, some ears)
							local d;

							foreach( d in detail )
								this.addDetail(d);
						}
						else if (typeof detail == "table")
						{
							// A single detail (say, some horns)
							this.addDetail(detail);
						}
						else if (detail != null)
						{
							// This is an error in the ContentDef spec.
							this.log.warn("Invalid detail variant: " + detail);
						}
					}
				}
			}
		}
		else
		{
			// We should have one of these for each race/gender combo as well.
			this.log.warn("ContentDef[" + key + "] is not defined");
		}

		if (skinColors != null)
			this.setSkinColors(skinColors);

		if (clothing != null)
		{
			//This is a hack to avoid issues with the old string format
			foreach( slot, x in clothing )
			{
				//The slot contains type, colors
				x = this.System.decodeVars(x);

				if ("colors" in x)
				{
					x.colors = this.System.decodeVars(x.colors);
				}

				clothing[slot] = x;
			}

			this.setClothing(clothing);
		}
	}

	function colorTexture( base, colors )
	{
		// Like the body texture, only create this if necessary
		local tex = ::_root.createProceduralTexture(this.getAssemblerName() + "/" + base + "/PTexture", base, this.MIPS);

		if (colors)
			tex.colorizeRegions(this._deriveTintMapName(base), this.ColorPalette.arrayToTable(colors));

		return tex;
	}

	function getChestOverride()
	{
		return this.mChestOverride;
	}

	function getArmorRobed()
	{
		if (this.mChestOverride)
			return true;

		if ("leggings" in this.mClothing)
		{
			local type = this.mClothing.leggings.type;

			if (type in ::ClothingDef)
			{
				local def = ::ClothingDef[type];

				if (("bodyType" in def) && def.bodyType == "Robed")
				{
					return true;
				}
			}
		}

		return false;
	}

	function setViaContentDef2( c )
	{
		// Sanity checking here verifies that things work properly later.
		if (("g" in c) && c.g != "m" && c.g != "f")
			throw this.Exception("Invalid gender: " + this.gender);

		if (!(c.r in ::Races))
			throw this.Exception("Invalid race: " + c.r);

		c.g = c.g == "f" ? "Female" : "Male";
		c.r = ::Races[c.r];
		local r_g = c.r + "_" + c.g;

		// Check the race/gender definitions are loaded
		this.mRaceGenderDefName = "Biped-" + r_g;
		if (!this._checkContentDefLoaded(this.mRaceGenderDefName))
			return false;

		// Set the clothing from the config
		if (("c" in c) && c.c != null)
			this.setClothing(c.c);

		// We need to make sure that the clothing is available before continuing
		if (!this._checkClothingLoaded())
			return false;

		// Sanity check on the scale.
		if ("sz" in c)
		{
			c.sz = c.sz.tofloat();

			if (c.sz < 0.1)
				c.sz = 0.1;
			else if (c.sz > 10.0)
				c.sz = 10.0;

			this.mSize = c.sz;
		}
		else
		{
			this.mSize = 1.0;
		}

		local bodyType = "";

		if ("b" in c)
		{
			if (c.b in ::BodyTypes)
			{
				if (c.b != "n")
				{
					bodyType = "-" + ::BodyTypes[c.b];
				}
			}
		}

		local headName = "Biped-" + r_g + "-Head";
		local headTexture = "Biped-" + r_g + "-Head";
		this.setBody("Biped-" + c.g + bodyType, "Biped-" + r_g + "-Body");
		this.setTail("Biped-" + r_g + "-Tail");

		// Set the attachments to use
		if (("a" in c) && c.a != null)
			this.setAttachments(c.a);

		if (!this._checkAttachmentsLoaded())
			return false;

		this.mAttachmentPointSet = null;
		local key = "Biped." + c.g + "." + c.r;

		if (key in ::AttachmentPointSet)
		{
			this.log.warn("Use of AttachmentPointSet[" + key + "] is deprecated. Add it to ContentDef");
			this.setAttachmentPointDefaults(::AttachmentPointSet[key]);
		}

		// Any extra details according to the predefined Race/Gender
		// setup? Things like ears, etc.
		local cd = ::ContentDef;
		key = "Biped-" + r_g;

		if (key in ::ContentDef)
		{
			local def = ::ContentDef[key];

			if ("SizeMultiplier" in def)
				this.mSize *= def.SizeMultiplier;

			if ("AttachmentPoints" in def)
				this.setAttachmentPointDefaults(def.AttachmentPoints);

			if ("Details" in def)
			{
				local section;
				local variants;

				foreach( section, sectionVariants in def.Details )
				{
					local choice = "default";


					// If there is an explicit choice for this section,
					// use that instead.
					if (("d" in c) && c.d != null && section in c.d)
						choice = c.d[section];

					// Now, see if we have one for that section.
					if (choice in sectionVariants)
					{
						local detail = sectionVariants[choice];
						
						// Now, we've got one, but each value here can be one of
						// three things:
						//   1. null -- Don't do anything (an explicit "none")
						//   2. A single table -- This is the detail spec
						//   3. An array -- Multiple detail specs, attach each.
						
						if (typeof detail == "array")
						{
							// An array of details (for instance, some ears)
							local d;
							foreach( d in detail )
							{
								this.addDetail(d);
							}
						}
						else if (typeof detail == "table")
						{
							// A single detail (say, some horns)
							this.addDetail(detail);
						}
						else if (detail != null)
						{
							// This is an error in the ContentDef spec.
							this.log.warn("Invalid detail variant: " + detail);
						}
					}
				}
			}

			if (("HelmetHead" in def) && this._checkHelmet())
			{
				headName = "Biped-" + r_g + "-Head-Helmet";
			}

			if ("Heads" in def)
			{
				if (("h" in c) && c.h != null && c.h < def.Heads.len())
				{
					local headEntry = def.Heads[c.h];

					if (this._checkHelmet() && (("helmetHead" in headEntry) && headEntry.helmetHead == true || ("HelmetHead" in def) && def.HelmetHead == true))
						headName = headEntry.mesh + "-Helmet";
					else
						headName = headEntry.mesh;

					if ("texture" in headEntry)
						headTexture = headEntry.texture;
				}
			}
		}
		else
		{
			// We should have one of these for each race/gender combo as well.
			this.log.warn("ContentDef[" + key + "] is not defined");
		}

		this.setHead(headName, headTexture);

		if (("sk" in c) && c.sk != null)
			this.setSkinColors(c.sk);

		// Check for chest/legs override

		this.mChestOverride = false;

		if (this.mClothing != null && ("chest" in this.mClothing) && this.mClothing.chest.type in ::ClothingDef)
		{
			local def = ::ClothingDef[this.mClothing.chest.type];
			if ("overridesLegs" in def)
				this.mChestOverride = def.overridesLegs;
		}
		
		// Set clothing body type

		local forearmType = "";

		if (("c" in c) && c.c != null)
		{
			this._checkClothingBodyType(c);
			forearmType = this._checkClothingForearm(c);
		}

		// Setup default texture and meshes for forarms
		
		local forarmMesh_Left = "Biped-" + c.g + bodyType + forearmType + "-Forearm_Left";
		local forarmMesh_Right = "Biped-" + c.g + bodyType + forearmType + "-Forearm_Right";
		local forarmTexture_Left = "body";
		local forarmTexture_Right = "body";

		// Use forarm mesh overrides specified in the clothing def if they exist

		if (("gloves" in this.mClothing) && this.mClothing.gloves.type in ::ClothingDef)
		{
			local def = ::ClothingDef[this.mClothing.gloves.type];

			if ("gauntlet_l" in def)
			{
				local texName = def.gauntlet_l + ".png";
				forarmTexture_Left = ("c" in c) && c.c != null && "gloves" in c.c ? this.colorTexture(texName, c.c.gloves.colors) : texName;
				forarmMesh_Left = def.gauntlet_l;
			}

			if ("gauntlet_r" in def)
			{
				local texName = def.gauntlet_r + ".png";
				forarmTexture_Right = ("c" in c) && c.c != null && "gloves" in c.c ? this.colorTexture(texName, c.c.gloves.colors) : texName;
				forarmMesh_Right = def.gauntlet_r;
			}
		}

		// All PCs have forearms. Currently we just derive from
		// the gender, but we could just as easily have custom
		// arms -- beefy/skinny/etc, or for some huge gauntleted
		// hands with fins and spikes and whatnot.
		// TODO: In fact, we can just move this into a "detail section"
		// and allow selection between them that way.

		this.addDetail({
			point = "left_forearm",
			mesh = forarmMesh_Left,
			texture = forarmTexture_Left
		});
		this.addDetail({
			point = "right_forearm",
			mesh = forarmMesh_Right,
			texture = forarmTexture_Right
		});
		

		// Choose the definition to use for the boots.
		// The chest may override boot definitions.
		local defName = this.mChestOverride == true ? "chest" : "boots";
		local def;

		if ((defName in this.mClothing) && this.mClothing[defName].type in ::ClothingDef)
			def = ::ClothingDef[this.mClothing[defName].type];
		else
			def = null;

		if (def)
		{
			// We don't want to add greaves for robed clothing
			if (("bodyType" in def) == false || def.bodyType != "Robed")
			{
				if ("greave_l" in def)
				{
					local texName = def.greave_l + ".png";
					local textureName = ("c" in c) && c.c != null ? this.colorTexture(texName, c.c.boots.colors) : texName;
					local meshName = def.greave_l;
					this.addDetail({
						point = "left_calf",
						mesh = meshName,
						texture = textureName
					});
				}

				if ("greave_r" in def)
				{
					local texName = def.greave_r + ".png";
					local textureName = ("c" in c) && c.c != null ? this.colorTexture(texName, c.c.boots.colors) : texName;
					local meshName = def.greave_r;
					this.addDetail({
						point = "right_calf",
						mesh = meshName,
						texture = textureName
					});
				}
			}
		}

		this._gatherHiddenAttachPoint();
		return true;
	}

	function setViaModelDef( type, skinColors, size, config )
	{
		local a = this.GetAssetArchive(type);

		if (a != null && !this._checkContentDefLoaded(a))
		{
			return false;
		}

		local tmp = ::ModelDef;

		if (!(type in ::ModelDef))
		{
			throw this.Exception("Configure error: unknown entity/npc type: " + type);
		}

		if (("Skin" in ::ModelDef[type]) && typeof ::ModelDef[type].Skin == "array")
		{
			throw this.Exception("Configure error: Asset is using old EntityDef format: " + type);
		}

		local def = ::ModelDef[type];
		local body = a;

		if ("Body" in def)
		{
			body = def.Body;
		}

		if ("ClickMesh" in def)
		{
			this.mClickMesh = def.ClickMesh;

			if ("ClickMeshScale" in def)
			{
				this.mClickMeshScale = this.Vector3(def.ClickMeshScale[0], def.ClickMeshScale[1], def.ClickMeshScale[2]);
			}

			if ("ClickMeshOffset" in def)
			{
				this.mClickMeshOffset = this.Vector3(def.ClickMeshOffset[0], def.ClickMeshOffset[1], def.ClickMeshOffset[2]);
			}
		}

		if ("Texture" in def)
		{
			if (typeof def.Texture == "array" && def.Texture.len() > 1)
			{
				this.setBody(body, def.Texture[0]);
				this.setHeadBaseTextureName(def.Texture[1]);
				this.mMultipartMesh = true;
			}
			else
			{
				this.setBody(body, def.Texture);
			}
		}
		else
		{
			this.setBody(body);
		}

		if (("a" in config) && config.a != null)
		{
			this.setAttachments(config.a);
		}

		if (!this._checkAttachmentsLoaded())
		{
			return false;
		}

		if ("AttachmentPointSet" in def)
		{
			this.setAttachmentPointDefaults(::AttachmentPointSet[def.AttachmentPointSet]);
		}

		if (size != null)
		{
			this.setSize(size);
		}
		else
		{
			this.setSize("Size" in def ? def.Size : 1.0);
		}

		if (skinColors)
		{
			this.setSkinColors(skinColors);
		}
		else if ("Skin" in def)
		{
			local skinTable = {};

			foreach( key, value in def.Skin )
			{
				skinTable[key] <- value.def;
			}

			this.setSkinColors(skinTable);
		}

		return true;
	}

	function _gatherHiddenAttachPoint()
	{
		this.mHiddenAttachPoints = {};

		if (this.mAttachments == null)
		{
			return;
		}

		if (this.mAttachmentPointSet == null)
		{
			return;
		}

		foreach( i, x in this.mAttachments )
		{
			if (x.node in this.mAttachmentPointSet)
			{
				local ap = this.mAttachmentPointSet[x.node];

				if ("hidden" in ap)
				{
					if (x.type in ::AttachableDef)
					{
						local def = ::AttachableDef[x.type];

						if (this.Util.indexOf(def.attachPoints, x.node) != null)
						{
							foreach( j, y in ap.hidden )
							{
								local found = false;

								foreach( q, z in this.mHiddenAttachPoints )
								{
									if (q == y)
									{
										found = true;
									}
								}

								if (!found)
								{
									this.mHiddenAttachPoints[y] <- true;
								}
							}
						}
					}
				}
			}
		}
	}

	function _getDefaultItemName( itemDef, slot )
	{
		local myDefaultClothes = {
			[this.ItemEquipSlot.ARMOR_HANDS] = "Armor-Light-Epic1",
			[this.ItemEquipSlot.ARMOR_HEAD] = "Armor-Cloth-Silly1-Helmet",
			[this.ItemEquipSlot.ARMOR_WAIST] = "Armor-Cloth-Low1",
			[this.ItemEquipSlot.ARMOR_SHOULDER] = "Armor-Chain1-Left_Pauldron",
			[this.ItemEquipSlot.WEAPON_MAIN_HAND] = "Item-1hSword-Basic1",
			[this.ItemEquipSlot.WEAPON_RANGED] = "Item-1hSword-Basic2",
			[this.ItemEquipSlot.WEAPON_OFF_HAND] = "Item-Bow-Basic1"
		};
		local myDefaultClothesEquipType = {
			[this.ItemEquipType.WEAPON_1H] = "Item-1hSword-Basic1",
			[this.ItemEquipType.WEAPON_1H_UNIQUE] = "Item-1hSword-Epic2",
			[this.ItemEquipType.WEAPON_1H_MAIN] = "Item-1hSword-High1",
			[this.ItemEquipType.WEAPON_1H_OFF] = "Item-1hSword-Medium1",
			[this.ItemEquipType.WEAPON_2H] = "Item-2hAxe-Basic1",
			[this.ItemEquipType.WEAPON_RANGED] = "Item-Bow-Basic1",
			[this.ItemEquipType.ARMOR_SHIELD] = "Item-Shield-Basic1",
			[this.ItemEquipType.ARMOR_SHOULDER] = "Armor-Warrior-Athenian",
			[this.ItemEquipType.COSEMETIC_SHOULDER] = "Armor-Base1B",
			[this.ItemEquipType.COSEMETIC_HIP] = "Item-Wand-Basic1"
		};
		local defaultWeaponType = {
			[this.WeaponType.SMALL] = "Item-Katar-Basic1",
			[this.WeaponType.ONE_HAND] = "Item-1hMace-Basic1",
			[this.WeaponType.TWO_HAND] = "Item-2hMace-Medium1",
			[this.WeaponType.POLE] = "Item-Spear-Basic1",
			[this.WeaponType.WAND] = "Item-Wand-Basic6",
			[this.WeaponType.BOW] = "Item-Bow-Basic3",
			[this.WeaponType.THROWN] = "Item-Dagger-Basic1",
			[this.WeaponType.ARCANE_TOTEM] = "Item-Talisman-Basic1"
		};
		local equipPosition = itemDef.getEquipType();
		local weaponType = itemDef.getWeaponType();
		local defaultCloth = "Armor-Cloth-Epic1";

		if (slot in myDefaultClothes)
		{
			defaultCloth = myDefaultClothes[slot];
		}

		if (equipPosition in myDefaultClothesEquipType)
		{
			defaultCloth = myDefaultClothesEquipType[equipPosition];
		}

		if (weaponType in defaultWeaponType)
		{
			defaultCloth = defaultWeaponType[weaponType];
		}

		return defaultCloth;
	}

	function applyEquipment( table )
	{

		print("ICE! applyEquipment\n");
		if (this.mEquipmentDef == "" || this.mRequestedItems == null)
		{
			return table;
		}

		local tableCopy = clone table;
		local attachments = [];
		local clothing = {};

		foreach( k, v in this.mRequestedItems )
		{
			local itemDef = ::_ItemDataManager.getItemDef(v);

			if (itemDef.isValid() == false)
			{
				return null;
			}

			if (itemDef.mAppearance == null || typeof itemDef.mAppearance == "array" && itemDef.mAppearance.len() == 2 && itemDef.mAppearance[0] == null && itemDef.mAppearance[1] == null)
			{
				local defaultClothes = this._getDefaultItemName(itemDef, k);
				local myAppear;

				if (k in ::ItemEquipSlotToClothing)
				{
					myAppear = {
						c = {
							type = defaultClothes,
							colors = [
								::Colors.fuschia,
								::Colors.fuschia,
								::Colors.fuschia,
								::Colors.fuschia,
								::Colors.fuschia,
								::Colors.fuschia
							]
						}
					};
					itemDef.mAppearance = myAppear;
				}

				if ((k in ::ItemEquipSlotToAttachmentPoint) && myAppear == null)
				{
					myAppear = {
						a = {
							type = defaultClothes,
							colors = [
								::Colors.fuschia,
								::Colors.fuschia
							]
						}
					};
					itemDef.mAppearance = myAppear;
				}

				if (myAppear == null)
				{
					continue;
				}
			}

			local appearanceList = itemDef.getAppearance();

			if (typeof appearanceList != "array")
			{
				appearanceList = [
					appearanceList
				];
			}

			local x = 0;
			for ( x = 0; x < appearanceList.len( ); x++ )
			{
				local appearance = appearanceList[x];
				local byPassAppearance = false;

				if (k in ::ItemEquipSlotToAttachmentPoint)
				{
					if (("a" in appearance) && !("point_override" in appearance.a))
					{
						local points = ::ItemEquipSlotToAttachmentPoint[k];
						local attachment = appearance.a;

						if (x < points.len())
						{
							local node = points[x];

							if (node)
							{
								local entry = {
									node = points[x],
									type = attachment.type,
									slot = k
								};

								if ("colors" in attachment)
								{
									entry.colors <- attachment.colors;
								}

								if ("effect" in attachment)
								{
									entry.effect <- attachment.effect;
								}

								attachments.append(entry);
								byPassAppearance = true;
							}
						}
					}
				}

				if (k in ::ItemEquipSlotToClothing)
				{
					if ("c" in appearance)
					{
						local clothingSlot = ::ItemEquipSlotToClothing[k];
						clothing[clothingSlot] <- {
							type = appearance.c.type
						};

						if ("colors" in appearance.c)
						{
							clothing[clothingSlot].colors <- appearance.c.colors;
						}
					}
				}

				if (("a" in appearance) && !byPassAppearance)
				{
					local attachment = appearance.a;

					if ("point_override" in attachment)
					{
						local entry = {
							node = attachment.point_override,
							type = attachment.type
						};

						if ("colors" in attachment)
						{
							entry.colors <- attachment.colors;
						}

						if ("effect" in attachment)
						{
							entry.effect <- attachment.effect;
						}

						attachments.append(entry);
					}
				} 

				x++;
			}
		}
		
		
		// ICEEE - Allow appearances to contain attachments to that get added as well
		if ("a" in table)
		{
			print("ICE! adding a bunch of additional attachments\n");
			foreach(attachment in table.a) {
				print("ICE! attachment: " + attachment + "\n");
				if("node" in attachment) {
					print("ICE! attachment node: " + attachment.node + "\n");
					local entry = {
						node = attachment.node,
						type = attachment.type,
					};
					print("ICE! attachment type: " + attachment.type + "\n");
					if ("colors" in attachment)
						entry.colors <- attachment.colors;

					if ("effect" in attachment)
						entry.effect <- attachment.effect;

					attachments.append(entry);
				}
			}
		}

		print("ICE! applyEq: existing attach: " + attachments.len() + "\n");
		tableCopy.a <- attachments;
		tableCopy.c <- clothing;
		return tableCopy;
	}

	function doConfigure()
	{
		if (this.mConfigured == true)
		{
			return true;
		}

		if (this.mConfig == null || this.mEquipmentDef == null)
		{
			return false;
		}

		local pos = this.mConfig.find(":");

		if (pos == null)
		{
			throw this.Exception("Invalid serialized configuration (no alg): " + this.mConfig);
		}

		local alg = this.mConfig.slice(0, pos);
		local data = this.mConfig.slice(pos + 1);
		this.reset();
		this.mSceneryAssembler = null;

		switch(alg)
		{
		case "c1":
			local opts = this.System.decodeVars(data);
			this.setViaContentDef1(opts.r, opts.g, "sz" in opts ? opts.sz.tofloat() : 1.0, "d" in opts ? this.System.decodeVars(opts.d) : null, "sk" in opts ? this.System.decodeVars(opts.sk) : null, "c" in opts ? this.System.decodeVars(opts.c) : null);
			break;

		case "c2":
			local sdata = this.unserialize(data);
			print("ICE! SDATA: " + data);
			local opts = this.applyEquipment(sdata);

			if (opts == null || this.setViaContentDef2(opts) == false)
			{
				return false;
			}

			break;

		case "n1":
			local opts = this.System.decodeVars(data);

			if ("t" in opts)
			{
				this.setBody(opts.m, opts.t);
			}
			else
			{
				this.setBody(opts.m);
			}

			if ("sz" in opts)
			{
				this.setSize(opts.sz.tofloat());
			}

			if ("sk" in opts)
			{
				this.setSkinColors(this.System.decodeVars(opts.sk));
			}

			break;

		case "n2":
		case "n3":
			local opts = alg == "n4" ? this.System.decodeVars(data) : this.unserialize(data);
			this.log.warn(this.getAssemblerName() + " has deprecated n2 / n3 config format: " + data);
			return false;
			break;

		case "n4":
			local opts = this.unserialize(data);

			if (this.setViaModelDef(opts.c, "sk" in opts ? opts.sk : null, "sz" in opts ? opts.sz.tofloat() : null, opts) == false)
			{
				return false;
			}

			break;

		case "p1":
			local opts = this.unserialize(data);

			if (this.setViaPropDef(opts) == false)
			{
				return false;
			}

			break;

		default:
			throw this.Exception("Unknown creature config algorithm: " + alg);
		}

		this.mConfigured = true;
		return true;
	}

	function setViaPropDef( data )
	{
		this.mSceneryAssembler = this.GetAssembler("Scenery", data.a);
		this.mSize = "sz" in data ? data.sz.tofloat() : 1.0;
		return true;
	}

	function resetDefWaiters()
	{
		this.mClothingDefWaiter = null;
		this.mAttachmentDefWaiter = null;
		this.mContentDefWaiter = null;
	}

	function configure( config )
	{
		if (config == "")
		{
			config = this.DEFAULT_APPEARANCE;
		}

		this.mConfig = config;
		this.resetDefWaiters();
		this.reset();
		this.reassembleInstances();
		return true;
	}

	function setEquipmentAppearance( value )
	{
		foreach( obj in this.mInstances )
		{
			obj.removeWeapons();
		}

		this.mEquipmentDef = value;

		if (this.mEquipmentDef == "" || this.mEquipmentDef == "{}")
		{
			this.mEquipmentDef = "";
			this.mRequestedItems = null;
			this.reset();
			this.reassembleInstances();
			return;
		}

		this.mRequestedItems = this.unserialize(value);

		foreach( k, v in this.mRequestedItems )
		{
			::_ItemDataManager.getItemDef(v);
		}

		this.resetDefWaiters();
		this.reset();
		this.reassembleInstances();
	}

	function getAttachPointDef( name )
	{
		if (name in this.mHiddenAttachPoints)
		{
			return null;
		}

		if (!this.mAttachmentPointSet)
		{
			return null;
		}

		if (name in this.mAttachmentPointSet)
		{
			return this.mAttachmentPointSet[name];
		}

		return null;
	}

	function setAttachments( attachments )
	{
		this.mAttachments = attachments;
	}

	
	function _assembleAttachments(so)
	{
		//so.removeNoneItemAttachments();

		if( (mAttachments == null) || (so.mAssemblyData.attachmentIndex >= mAttachments.len( )) )
			return true;

		// Pick either a particle system override or the default effect

		local iot = mAttachments[so.mAssemblyData.attachmentIndex++];
		local io = Item.Attachable( ("id" in iot) ? iot.id : null,
							 		iot.type,
							  		iot.node,
							  		("colors" in iot) ? iot.colors : {},
							  		("effect" in iot) ? iot.effect : null );

		if (("slot" in iot))
			so.setWeapon(iot.slot, io);
	
		so.addAttachment(io);
		io.assemble( );

		return false;
	}

	function addDetail( attachment )
	{
		if (attachment == null || typeof attachment != "table" && typeof attachment != "string")
		{
			this.log.error("Invalid attachment specified");
			return;
		}

		if (this.mDetails == null)
		{
			this.mDetails = [];
		}

		this.mDetails.append(attachment);
	}

	function setClothing( ... )
	{
		if (vargc > 1)
		{
			local slot = vargv[0];
			local type = vargv[1];
			local colorTable;

			if (vargc > 2)
			{
				colorTable = vargv[2];
			}

			if (type == null)
			{
				if (this.mClothing)
				{
					delete this.mClothing[slot];

					if (this.mClothing.len() == 0)
					{
						this.mClothing = null;
					}
				}
			}
			else
			{
				if (this.mClothing == null)
				{
					this.mClothing = {};
				}

				this.mClothing[slot] <- {};
				this.mClothing[slot].type <- type;

				if (colorTable.len() > 0)
				{
					this.mClothing[slot].colors <- colorTable;
				}
			}
		}
		else if (vargc == 1)
		{
			if (typeof vargv[0] == "null")
			{
				this.mClothing = null;
			}
			else if (typeof vargv[0] == "table")
			{
				this.mClothing = vargv[0];
			}
		}

		this._invalidateTextures();
	}

	function setStat( statId, value )
	{
		this.Assembler.Factory.setStat(statId, value);
		local ct = this.Screens.get("CreatureTweakScreen", false);

		if (ct && ct.isVisible() && ct.getCurrentType() == this.mObjectType)
		{
			ct.statUpdate(statId, value);
		}
	}

	function _resetAssemblyData( so )
	{
		local be;

		if ("entities" in so.mAssemblyData)
		{
			foreach( i, x in so.mAssemblyData.entities )
			{
				if (x != null)
				{
					if (i == "Body")
					{
						be = x;
					}
					else
					{
						x.destroy();
					}
				}
			}
		}

		if (("clickEntity" in so.mAssemblyData) && so.mAssemblyData.clickEntity != null)
		{
			so.mAssemblyData.clickEntity.destroy();
			so.mAssemblyData.clickNode.destroy();
		}

		if (this.mSceneryAssembler)
		{
			this.mSceneryAssembler.disassemble(so);
		}

		if (be != null)
		{
			be.destroy();
		}

		so.mAssemblyData = {
			assemblyStage = 0,
			detailIndex = 0,
			attachmentIndex = 0,
			entities = {},
			clickEntity = null,
			clickNode = null
		};
	}

	function _checkClothingForearm( c )
	{
		local type = "";

		if ("arms" in c.c)
		{
			if (c.c.arms.type in this.ClothingDef)
			{
				local cdef = this.ClothingDef[c.c.arms.type];

				if ("bodyType" in cdef)
				{
					type = "-" + cdef.bodyType;
				}
			}
		}

		return type;
	}

	function _checkClothingBodyType( c )
	{
		local name = this.mChestOverride ? "chest" : "leggings";

		if (name in c.c)
		{
			if (c.c[name].type in this.ClothingDef)
			{
				local cdef = ::ClothingDef[c.c[name].type];

				if ("bodyType" in cdef)
				{
					this.setBody(this.mBody + "-" + cdef.bodyType, this.mBodyBaseTexture);
				}
			}
		}
	}

	function _prepare()
	{
		local now = this.System.currentTimeMillis();

		if (this.mPrepared != null)
		{
			return true;
		}

		if (!this._checkEquipmentLoaded())
		{
			return false;
		}

		if (!this._checkContentDefLoaded(this.mRaceGenderDefName))
		{
			return false;
		}

		if (!this.mBody)
		{
			this.log.debug(this.getAssemblerName() + " is not ready yet because it does not have a body: " + this.mConfig);
			return false;
		}

		if (this.mRequiredResources == null)
		{
			this._populateRequiredResources();
		}

		if (!this._checkRequiredResourcesPrepared())
		{
			return false;
		}

		this.mPrepared = now;
		return true;
	}

	function _addRequiredColorizedResource( tex )
	{
		this._addRequiredResource(tex);
		local tintmap = this._deriveTintMapName(tex);
		this._addRequiredResource(tintmap);
		this._addRequiredResource(tintmap + ".colormap");
	}

	function _populateRequiredResources()
	{
		this.mRequiredResources = null;
		local biped = this.Util.startsWith(this.mBody, "Biped-");
		this._addRequiredResource(this.mBody + ".mesh");

		if (biped)
		{
			this._addRequiredResource("MOB_Standard");
			this._addRequiredResource("Biped-Male2_mesh.skeleton");
			local baseBodyTex = this.mBodyBaseTexture;

			if (this.mSkinColors != null && baseBodyTex == null)
			{
				baseBodyTex = this.mBody;
			}

			this._addRequiredColorizedResource(baseBodyTex + ".png");
		}

		if (this.mClickMesh)
		{
			this._addRequiredResource(this.mClickMesh + ".mesh");
		}

		if (this.mHead)
		{
			this._addRequiredResource(this.mHead + ".mesh");
			local baseTex = this.mHeadBaseTexture;

			if (this.mSkinColors != null && baseTex == null)
			{
				baseTex = this.mHead;
			}

			this._addRequiredColorizedResource(baseTex + ".png");
		}

		if (this.mLeftForearm)
		{
			this._addRequiredResource(this.mLeftForearm + ".mesh");

			if (this.mLeftForearmTexture != "body")
			{
				this._addRequiredColorizedResource(this.mLeftForearmTexture);
			}
		}

		if (this.mRightForearm)
		{
			this._addRequiredResource(this.mRightForearm + ".mesh");

			if (this.mRightForearmTexture != "body")
			{
				this._addRequiredColorizedResource(this.mRightForearmTexture);
			}
		}

		if (this.mTail)
		{
			this._addRequiredResource(this.mTail + ".mesh");
			this._addRequiredResource(this.mTail + "_mesh.skeleton");
		}

		if (biped && this.mClothing)
		{
			foreach( k, v in this.mClothing )
			{
				if (v.type)
				{
					local type = v.type;
					this._addRequiredResource(type + "-Core.png");
					this._addRequiredResource(type + "-Core-Tint.png");
					this._addRequiredResource(type + "-Core-Tint.png.colormap");
					this._addRequiredResource(type + "-Core-Clothing_Map.png");
					this._addRequiredResource(type + "-Core-Clothing_Map.png.colormap");
					this._addRequiredResource(type + "-Accessories.png");
					this._addRequiredResource(type + "-Accessories-Tint.png");
					this._addRequiredResource(type + "-Accessories-Tint.png.colormap");
					this._addRequiredResource(type + "-Accessories-Clothing_Map.png");
					this._addRequiredResource(type + "-Accessories-Clothing_Map.png.colormap");
					this._addRequiredResource(type + "-Extremities.png");
					this._addRequiredResource(type + "-Extremities-Tint.png");
					this._addRequiredResource(type + "-Extremities-Tint.png.colormap");
					this._addRequiredResource(type + "-Extremities-Clothing_Map.png");
					this._addRequiredResource(type + "-Extremities-Clothing_Map.png.colormap");
				}
			}
		}
	}

	function _checkEquipmentLoaded()
	{
		if (this.mEquipmentDef != "")
		{
			foreach( k, v in this.mRequestedItems )
			{
				if (::_ItemDataManager.isItemDefFetched(v) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	function _checkContentDefLoaded( defName )
	{
		if (defName == null)
		{
			return true;
		}

		if (this.mContentDefWaiter != null)
		{
			if (this.mContentDefWaiter.isReady())
			{
				return true;
			}

			return false;
		}

		this.mContentDefWaiter = this.Util.waitForAssets(defName, null);
		return false;
	}

	function _checkClothingLoaded()
	{
		if (this.mClothingDefWaiter != null)
		{
			if (this.mClothingDefWaiter.isReady())
			{
				return true;
			}

			return false;
		}

		if (this.mClothing == null)
		{
			return true;
		}

		local defs = [];

		foreach( k, v in this.mClothing )
		{
			if (v.type)
			{
				local archive = this.GetAssetArchive(v.type);

				if (archive != null)
				{
					defs.append(archive);
				}
			}
		}

		this.mClothingDefWaiter = this.Util.waitForAssets(defs, null);
		return false;
	}

	function _checkAttachmentsLoaded()
	{
		if (this.mAttachmentDefWaiter != null)
		{
			if (this.mAttachmentDefWaiter.isReady())
			{
				return true;
			}

			return false;
		}

		local defs = [];

		if (this.mAttachments)
		{
			foreach( k, v in this.mAttachments )
			{
				if (v.type)
				{
					local archive = this.GetAssetArchive(v.type);

					if (archive != null)
					{
						defs.append(archive);
					}
				}
			}
		}

		if (this.mDetails)
		{
			foreach( slot, d in this.mDetails )
			{
				if (d.mesh)
				{
					local archive = this.GetAssetArchive(d.mesh);

					if (archive != null)
					{
						defs.append(archive);
					}
				}
			}
		}

		this.mAttachmentDefWaiter = this.Util.waitForAssets(defs, null);
		return false;
	}

	function _assemble( so )
	{
		if (!this._sceneObjectManager.getCreatureReady(so))
		{
			return this.ASSEMBLE_FREQUENCY;
		}

		if (this.mContentLoadError != null)
		{
			throw this.Exception("Error while loading content: " + this.mContentLoadError);
		}

		so._setAssembling(true);

		if (this.doConfigure() == false)
		{
			return this.ASSEMBLE_FREQUENCY;
		}

		if (this.mSceneryAssembler)
		{
			local result = this.mSceneryAssembler._assemble(so);

			if (result == true)
			{
				so.setNormalSize(this.mSize);
				so.setScale(null);
			}

			return result;
		}

		if (so.mAssemblyData == null)
		{
			this._resetAssemblyData(so);
		}

		if (this.mConfig == null || this.mEquipmentDef == null)
		{
			this.log.debug("Not assembling " + so + " because it does not have a body or equipment appearance.");
			return this.ASSEMBLE_FREQUENCY;
		}

		if (this._prepare() == false)
		{
			return this.ASSEMBLE_FREQUENCY;
		}

		if (this._assembleSkin() == false)
		{
			return this.ASSEMBLE_FREQUENCY;
		}

		if (this._assembleClothing() == false)
		{
			return this.ASSEMBLE_FREQUENCY;
		}

		switch(so.mAssemblyData.assemblyStage)
		{
		case this.ASSEMBLE_PREPARE:
			this.log.debug("Preparing creature: " + this.getAssemblerName());
			this._resetAssemblyData(so);
			local node = so.getNode();

			if (node.getParent() != null)
			{
				node.getParent().removeChild(node);
			}

			so.mAssemblyData.assemblyStage = this.ASSEMBLE_BODY;
			return this.ASSEMBLE_FREQUENCY;

		case this.ASSEMBLE_BODY:
			this._assembleBody(so);
			so.mAssemblyData.assemblyStage = this.ASSEMBLE_HEAD;
			return this.ASSEMBLE_FREQUENCY;

		case this.ASSEMBLE_HEAD:
			this._assembleHead(so);
			so.mAssemblyData.detailIndex = 0;
			so.mAssemblyData.assemblyStage = this.ASSEMBLE_DETAILS;
			return this.ASSEMBLE_FREQUENCY;

		case this.ASSEMBLE_DETAILS:
			if (this._assembleDetails(so, this.mDetails))
			{
				so.removeNoneItemAttachments();
				so.mAssemblyData.attachmentIndex = 0;
				so.mAssemblyData.assemblyStage = this.ASSEMBLE_ATTACHMENTS;
				return this.ASSEMBLE_FREQUENCY;
			}

			return this.ASSEMBLE_SUBFREQUENCY;

		case this.ASSEMBLE_ATTACHMENTS:
			if (this._assembleAttachments(so))
			{
				so.mAssemblyData.assemblyStage = this.ASSEMBLE_FINALIZE;

				if (so.isForceShowWeapon())
				{
					if (so.hasWeapon(this.ItemEquipSlot.WEAPON_RANGED))
					{
						so.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
					}
					else if (so.hasWeapon(this.ItemEquipSlot.WEAPON_MAIN_HAND))
					{
						so.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
					}
				}

				return this.ASSEMBLE_FREQUENCY;
			}

			return this.ASSEMBLE_SUBFREQUENCY;

		case this.ASSEMBLE_FINALIZE:
			so.setNormalSize(this.mSize);
			local animationHandler;
			local mdef;

			if (this.mBody in ::ModelDef)
			{
				mdef = ::ModelDef[this.mBody];
			}

			if (this.mAttachmentPointSet == null)
			{
				if ("AttachmentPoints" in mdef)
				{
					this.setAttachmentPointDefaults(mdef.AttachmentPoints);
				}
				else
				{
					this.log.warn("Attachment point set not found for (" + this.mBody + ")");
				}
			}

			if ("AnimationHandler" in mdef)
			{
				animationHandler = ::AnimationHandler[mdef.AnimationHandler](so);

				if (mdef.AnimationHandler == "Biped" && "Posture" in mdef)
				{
					animationHandler.setPosture(mdef.Posture);
				}
			}
			else
			{
				animationHandler = ::AnimationHandler.Biped(so);

				if (this.mRaceGenderDefName != null && (this.mRaceGenderDefName in ::ContentDef) && "Posture" in ::ContentDef[this.mRaceGenderDefName])
				{
					animationHandler.setPosture(::ContentDef[this.mRaceGenderDefName].Posture);
				}
			}

			animationHandler.onStop();
			so.setAnimationHandler(animationHandler);

			if ("AlignToFloorMode" in mdef)
			{
				local mode = this.FloorAlignMode.NONE;

				switch(mdef.AlignToFloorMode)
				{
				case "NONE":
					mode = this.FloorAlignMode.NONE;
					break;

				case "WHILE_ASCENDING_DESCENDING":
					mode = this.FloorAlignMode.WHILE_ASCENDING_DESCENDING;
					break;

				case "ALWAYS":
					mode = this.FloorAlignMode.ALWAYS;
					break;

				default:
					mode = this.FloorAlignMode.NONE;
					break;
				}

				so.setFloorAlignMode(mode);
			}
			else
			{
				so.setFloorAlignMode(this.FloorAlignMode.NONE);
			}

			local ct = this.Screens.get("CreatureTweakScreen", false);

			if (ct && ct.isVisible())
			{
				ct._restoreAnimationState(so);
			}

			so._setAssembled(true);
			::_scene.getRootSceneNode().addChild(so.getNode());
			so.updateGrip();
			so.updateSheathedWeapons();
			so.mAssemblyData.assemblyStage = this.ASSEMBLE_PREPARE;
			return true;
		}

		return false;
	}

	function onError( obj, error )
	{
		if (!obj.getNode().getParent())
		{
			::_scene.getRootSceneNode().addChild(obj.getNode());
		}
	}

	function _getAnimationHandler( so )
	{
	}

	function _invalidateTextures()
	{
		this.mBodyTexture = null;
		this.mHeadTexture = null;
		this.mSkinAssembled = false;
		this.mClothingAssembled = false;
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
			if (tex == "head")
			{
				texName = this.mHeadTexture == null ? this.mHead + ".png" : this.mHeadTexture.getName();
			}
			else if (tex == "body")
			{
				texName = this.mBodyTexture == null ? this.mBody + ".png" : this.mBodyTexture.getName();
			}
			else
			{
				texName = tex;
			}
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

	function _createEntity( so, name, mesh, diffuseTexture )
	{
		if (name in so.mAssemblyData.entities)
		{
			throw this.Exception("Recreating entity " + name + " for " + so);
		}

		local entityName = so.getNodeName() + "/" + name;
		local e;

		try
		{
			e = this._scene.createEntity(entityName, mesh);
		}
		catch( err )
		{
			e = this._scene.getEntity(entityName);
		}

		e.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY | this.VisibilityFlags.LIGHT_GROUP_ANY);

		if (diffuseTexture)
		{
			local texName = this._resolveTextureName(diffuseTexture);
			e.applyTextureAliases({
				Diffuse = texName
			}, 0);

			if (this.mMultipartMesh)
			{
				local headtexName;

				if (diffuseTexture == "body")
				{
					headtexName = this._resolveTextureName("head");
				}
				else if (diffuseTexture == "head")
				{
					headtexName = this._resolveTextureName("body");
				}

				e.applyTextureAliases({
					Diffuse = headtexName
				}, 1);
			}
		}

		so.mAssemblyData.entities[name] <- e;
		return e;
	}

	function _assembleSkin()
	{
		if (this.mSkinAssembled == true)
		{
			return true;
		}

		local baseBodyTex = this.mBodyBaseTexture;

		if (this.mSkinColors != null && baseBodyTex == null)
		{
			baseBodyTex = this.mBody;
		}

		if (this.mBodyTexture == null && baseBodyTex != null)
		{
			this.mBodyTexture = ::_root.createProceduralTexture(this.getAssemblerName() + "/Body/PTexture", baseBodyTex + ".png", this.MIPS);
			this.mNeedClothes = true;

			if (this.mSkinColors)
			{
				this.mBodyTexture.colorizeRegions(this._deriveTintMapName(baseBodyTex), this.mSkinColors);
			}
		}

		if (this.mHeadTexture == null && this.mHeadBaseTexture != null)
		{
			this.mHeadTexture = ::_root.createProceduralTexture(this.getAssemblerName() + "/Head/PTexture", this.mHeadBaseTexture + ".png", this.MIPS);
			this.mNeedClothes = true;

			if (this.mSkinColors)
			{
				this.mHeadTexture.colorizeRegions(this._deriveTintMapName(this.mHeadBaseTexture), this.mSkinColors);
			}
		}

		this.mSkinAssembled = true;
	}

	function _processClothingLayer( layer )
	{
		if (!(layer in this.mClothing) || !("type" in this.mClothing[layer]) || this.mClothing[layer].type == "")
		{
			return;
		}

		local type;

		if (layer == "leggings" && this.mChestOverride)
		{
			type = this.mClothing.chest.type;
		}
		else
		{
			type = this.mClothing[layer].type;
		}

		local colorArray;

		if ("colors" in this.mClothing[layer])
		{
			colorArray = this.mClothing[layer].colors;
		}

		if (typeof colorArray == "table")
		{
			colorArray = this.ColorPalette.tableToArray(colorArray);
		}

		if (!(type in ::ClothingDef))
		{
			this.log.warn("Clothing (" + type + ") not found in ClothingDef for " + this.getAssemblerName());
			return;
		}

		local cdef = ::ClothingDef[type];

		if ("type" in cdef)
		{
			type = cdef.type;
		}

		local bodyTex = this.mBodyTexture;
		this.log.debug("  adding " + layer + " (" + type + ") to " + bodyTex.getName());

		if (!("regions" in cdef))
		{
			throw type + " nut file does has not had regions setup";
		}

		if (!(layer in cdef.regions))
		{
			return;
		}

		local mapType = "";
		mapType = cdef.regions[layer];

		if ("colors" in cdef)
		{
			local texture;
			local textureKey = mapType + "-" + layer;
			local colors = [];

			foreach( i, color in cdef.colors )
			{
				if (colorArray && i < colorArray.len() && colorArray[i] != null)
				{
					colors.append(colorArray[i]);
				}
				else if (color != null)
				{
					colors.append(color);
				}
			}

			colors = this.ColorPalette.arrayToTable(colors);
			texture = ::_root.createProceduralTexture(type + "/" + mapType + "/" + layer + "/PTexture", type + "-" + mapType + ".png");
			texture.colorizeRegions(type + "-" + mapType + "-Tint.png", colors);
			local regionTable = {
				[layer] = texture.getName()
			};
			bodyTex.blendRegions(type + "-" + mapType + "-Clothing_Map.png", regionTable);
		}
		else
		{
			bodyTex.blendRegions(type + "-" + mapType + "-Clothing_Map.png", {
				[layer] = type + "-" + mapType + ".png"
			});
		}
	}

	function _assembleClothing()
	{
		if (this.mClothingAssembled == true)
		{
			return true;
		}

		local regionMapping = {
			arms = "arms",
			gloves = "gloves",
			robed_arms = "arms",
			boots = "boots",
			leggings = "leggings",
			chest = "chest",
			overlapping_leggings = "leggings",
			overlapping_chest = "chest",
			collar = "collar",
			belt = "belt"
		};
		local normalOrder = [
			"arms",
			"gloves",
			"robed_arms",
			"leggings",
			"chest",
			"overlapping_leggings",
			"overlapping_chest",
			"boots",
			"collar",
			"belt"
		];
		local robedOrder = [
			"arms",
			"gloves",
			"robed_arms",
			"boots",
			"leggings",
			"chest",
			"overlapping_leggings",
			"overlapping_chest",
			"collar",
			"belt"
		];
		local order = this.getArmorRobed() ? robedOrder : normalOrder;

		while (this.mClothingIndex < order.len())
		{
			local l = order[this.mClothingIndex++];
			local region = regionMapping[l];

			if ((region in this.mClothing) == false)
			{
				continue;
			}

			local setting = this.mClothing[region].type;

			if ((setting in ::ClothingDef) == false)
			{
				continue;
			}

			local def = ::ClothingDef[setting];
			local priority = region;

			if (("region_priority" in def) && region in def.region_priority)
			{
				priority = def.region_priority[region];
			}

			if (l == priority)
			{
				this._processClothingLayer(region);
			}
		}

		this.mClothingAssembled = true;
		return true;
	}

	function _updateMinimapSceneNodeSticker( so, node )
	{
		if (::_avatar == null)
		{
			return;
		}

		if (::_avatar.getID() == so.getID() && this.LegendItemSelected[this.LegendItemTypes.YOU])
		{
			this._root.setMinimapSceneNodeSticker(node, this.LegendItemTypes.YOU);
		}
		else if ((so.getMeta("copper_shopkeeper") || so.getMeta("credit_shopkeeper") || so.getMeta("credit_shop") != null) && this.LegendItemSelected[this.LegendItemTypes.SHOP])
		{
			this._root.setMinimapSceneNodeSticker(node, this.LegendItemTypes.SHOP);
		}
		else if (so.getMeta("quest_giver") && ::LegendItemSelected[this.LegendItemTypes.QUEST_GIVER])
		{
			return;
		}
		else if ((so.getStat(this.Stat.CREATURE_CATEGORY) in this.LegendItemSelected) && this.LegendItemSelected[so.getStat(this.Stat.CREATURE_CATEGORY)])
		{
			local creatureCategory = so.getStat(this.Stat.CREATURE_CATEGORY);
			this._root.setMinimapSceneNodeSticker(node, creatureCategory);
		}
		else
		{
			this._root.setMinimapSceneNodeSticker(node, "");
		}
	}

	function _assembleBody( so )
	{
		local node = so.getNode();

		if (this.mClickMesh)
		{
			local name = node.getName() + "/ClickBox";
			local clickBox = this._scene.createEntity(name, this.mClickMesh + ".mesh");
			clickBox.setMaterialName("Interact");
			local clickNode = node.createChildSceneNode(name);

			if (this.mClickMeshOffset)
			{
				clickNode.setPosition(this.mClickMeshOffset);
			}

			if (this.mClickMeshScale)
			{
				clickNode.setScale(this.mClickMeshScale);
			}

			clickNode.attachObject(clickBox);
			so.mAssemblyData.clickEntity <- clickBox;
			so.mAssemblyData.clickNode <- clickNode;
		}

		local entity = this._createEntity(so, "Body", this.mBody + ".mesh", "body");
		so.setScale(::Vector3(this.mSize, this.mSize, this.mSize));
		node.attachObject(entity);
		so.onAttachmentPointChanged(entity);
		so.mAssemblyData.bodyEntity <- entity;
		this._updateMinimapSceneNodeSticker(so, node);
		local ct = this.Screens.get("CreatureTweakScreen", false);

		if (ct && ct.isVisible())
		{
			ct._updateAnimationList();
		}
	}

	function _assembleForearms( so )
	{
		local entity = so.mAssemblyData.bodyEntity;

		if (this.mLeftForearm)
		{
			local e = this._createEntity(so, "LeftForearm", this.mLeftForearm + ".mesh", this.mLeftForearmTexture);
			entity.attachObjectToBone("Bone-LeftForearm", e);
			so.onAttachmentPointChanged(e);
		}

		if (this.mRightForearm)
		{
			local e = this._createEntity(so, "RightForearm", this.mRightForearm + ".mesh", this.mRightForearmTexture);
			entity.attachObjectToBone("Bone-RightForearm", e);
			so.onAttachmentPointChanged(e);
		}
	}

	function _assembleHead( so )
	{
		if (!this.mHead)
		{
			return;
		}

		local entity = so.mAssemblyData.bodyEntity;

		if (this._isFullHelmetAttached())
		{
			return;
		}

		local e = this._createEntity(so, "Head", this.mHead + ".mesh", "head");
		entity.attachObjectToBone("Bone-Head", e);
		so.onAttachmentPointChanged(e);
	}

	function _assembleTail( so )
	{
		if (!this.mTail)
		{
			return;
		}

		local body = so.mAssemblyData.bodyEntity;

		try
		{
			local e = this._createEntity(so, "Tail", this.mTail + ".mesh", this.mTailBaseTexture != null ? this.mTailBaseTexture : "head");
			e.setBendyPoseController(0.25);
			body.attachObjectToBone("Bone-TailEnd", e);
			so.onAttachmentPointChanged(e);
			e.getParentNode().rotate(this.Vector3(0, 1, 0), -this.Math.PI / 2);
		}
		catch( err )
		{
			this.log.error("Tail not found: " + this.mTail);
		}
	}

	function _assembleDetails(so, attachments)
	{
		if( !attachments || (so.mAssemblyData.detailIndex >= attachments.len( )) )
			return true;

		local entity = so.mAssemblyData.bodyEntity;

		// Attachment info is resolved in the following order:
		//   1. The "body type" defines the general attachment
		//      point info (bone and scale, etc).
		//   2. If it's an entry in the AttachableDef table, it
		//      can override settings per attachment point.
		//   3. Finally, the attachment record itself (i.e. the
		//      thing in the object's attachment table) has final
		//      say over things.

		// Any step in the above list can be omitted as long as
		// both "mesh" and one of either "bone" or "point" (which
		// in turn has a bone associated with it) are resolved by
		// the end. Also, in step 2, it implicitly defines the mesh
		// using a permutation on the "attachable" name.

		// Each attachment is a table with the following:
		//   attachable - The entry in the ::AttachableDef table for this
		//             attachment (this also implicitly defines the mesh
		//             to use if the AttachableDef does not specify).
		//   bone - (Optional) The bone to attach the resultant entity to.
		//   texture - (Optional) One of either "head" or "body" to use
		//             the creature's already configured texture, otherwise
		//             it is a full texture name to use.
		//   colors - (Optional) The colorizations to use
		//   position - (Optional) A positional offset for the attachment
		//   orientation - (Optional) An extra orientation for attaching
		//   scale - (Optional) A scaling to apply to the attachment

		local pointName = so.mAssemblyData.detailIndex++;
		local attachment = attachments[pointName];

		if( ("point" in attachment) && (attachment.point in mHiddenAttachPoints) )
			return false;

		local point = {};

		// Defaults are loaded from the "body type" attachment
		// point set (if that's provided).
		if( mAttachmentPointSet )
		{
			if( pointName in mAttachmentPointSet )
				Util.overrideSlots(point, mAttachmentPointSet[pointName]);

			if( "point" in attachment &&
			     attachment.point in mAttachmentPointSet )
			    Util.overrideSlots(point, mAttachmentPointSet[attachment.point]);
		}

		// If it's named as an attachable, then it references
		// an entry in the AttachmentDef table.
		local attachable = null;

		if( typeof attachment == "string" )
			attachable = attachment;
		else if( typeof attachment == "table" )
		{
			if( "attachable" in attachment )
				attachable = attachment.attachable;
		}
		else
		{
			log.error("Invalid attachment (" + attachment + ") in " +
				getAssemblerName());

			return false;
		}

		// If the attachment was a string or an entry in the
		// attachment had "attachable" defined, then we need
		// to look up something in the AttachableDef table.
		if( attachable )
		{
			// It also implicitly sets the mesh to use (may be
			// overridden later).
			point["mesh"] <- attachable + ".mesh";

			// If it's in the attachable table, it may have
			// overrides we need to consider.
			if( attachable in ::AttachableDef )
			{
				local adef = ::AttachableDef[attachable];

				// Default colors are stored at the "top" level.
				if( "colors" in adef )
					point.colors <- adef.colors;

				// TODO: How do we hook into and do a
				// per-item-per-body-type override? I.e.
				// allow an item to provide an override specific
				// to the body type being used. Not sure we need
				// this if the body type can encompass most changes
				// we'd need. (Pretty sure all helmets are standard
				// per body type for instance).

				// It defines a point-specific override
				if( pointName in adef )
					Util.overrideSlots(point, adef[pointName]);
			}
		}

		// Anything specified explicitly in the attachment
		// overrides both the per-item and per-body attachment
		// settings. (This includes things like the mesh!)
		if( typeof attachment == "table" )
			Util.overrideSlots(point, attachment);

		// TODO: Note, the above "overrides" may be too simplistic.
		// It's probably more intelligent to combine transforms for
		// things like scale/position/orientation. That way, you could
		// have, say, the body determine scale, but the item determine
		// an extra orientation. It remains to be seen if this will
		// actually be needed, though.

		// Need at least these two things to attach something.
		if( !("mesh" in point) || !("bone" in point) )
		{
			log.warn("Unable to resolve attachment (" + pointName +
				") for " + getAssemblerName());

			return false;
		}

		// Textures are usually only specified for details not
		// regular item attachments (but you could theoretically
		// have one).
		local tex = null;
		if( "texture" in point )
		{
			// This could be either a string (i.e. "head", "body" or
			// a texture name) or a ProceduralTexture
			tex = point.texture;
		}

		// TODO: If we allow multiple attachments per attachment point
		// then this naming scheme doesn't work.
		local attachmentEntitySuffix = "Attachment/" + pointName;

		if( "colors" in point )
		{
			if( tex == null )
			{
				// If they didn't supply a texture, we must infer
				// it from the attachable. If they didn't give us
				// -that- then we can't recolorize as we don't know
				// what texture to use as a base.

				// This is a pretty poor assumption though. :( It
				// doesn't let us use .jpgs for instance.
				if( attachable )
					tex = attachable + ".png";
			}

			if( tex != null )
			{
				// Create a colorized texture for this.
				tex = ::_root.createProceduralTexture(
					so.getNodeName() + "/" + attachmentEntitySuffix + "/PTexture",
					_resolveTextureName(tex), MIPS);
			}
			else
				log.warn("Cannot recolorize attachment (" + pointName +
					") in " + getAssemblerName() +
					" because either \"texture\" or \"attachable\" must " +
					" be specified");
		}

		// It's fine to have tex be null at this point. It will
		// just use the mesh unaltered in that case.
		local e = _createEntity(so, attachmentEntitySuffix,
						point.mesh + ".mesh", tex);

		// Finally, attach our created entity!
		entity.attachObjectToBone(point.bone, e);
		so.onAttachmentPointChanged(e);

		// Apply animation if necessary
		if ( ("animated" in attachment) && (attachment.animated == true) )	
		{
			e.setAnimationUnitCount(1);
			e.getAnimationUnit(0).setIdleState("Idle");
			e.getAnimationUnit(0).setEnabled(true);
		}

		if( "position" in point )
			e.getParentNode().translate(point.position);

		if( "orientation" in point )
			e.getParentNode().setOrientation(point.orientation);

		// Must set this after the fact due to API oddness
		if( "scale" in point )
			e.getParentNode().setScale(point.scale);

		return false;
	}

	function _isFullHelmetAttached()
	{
		if (this.mAttachments == null)
		{
			return false;
		}

		foreach( i, x in this.mAttachments )
		{
			if (x.node == "full_helmet")
			{
				if (x.type in ::AttachableDef)
				{
					local def = ::AttachableDef[x.type];

					if (this.Util.indexOf(def.attachPoints, "full_helmet") != null)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	function _checkHelmet()
	{
		if (this.mAttachments == null)
		{
			return false;
		}

		foreach( i, x in this.mAttachments )
		{
			if (x.node == "helmet")
			{
				if (x.type in ::AttachableDef)
				{
					local def = ::AttachableDef[x.type];

					if (this.Util.indexOf(def.attachPoints, "helmet") != null)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	function disassemble( so )
	{
		so.setAnimationHandler(null);

		if (so.getEffectsHandler())
		{
			so.getEffectsHandler().onDisassembled();
		}

		foreach( i, x in so.mAttachments )
		{
			x.disassemble();
		}

		this._resetAssemblyData(so);
		this.Assembler.Factory.disassemble(so);
	}

	function getBaseEntity( so )
	{
		if ("bodyEntity" in so.mAssemblyData)
		{
			return so.mAssemblyData.bodyEntity;
		}

		return null;
	}

	mCachedTextures = [];
}

