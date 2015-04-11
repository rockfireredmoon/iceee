this.require("UI/UI");
this.require("UI/Screens");
local oldCT = this.Screens.get("CreatureTweakScreen", false);

if (oldCT)
{
	oldCT = oldCT.getCurrentType();
	this.Screens.close("CreatureTweakScreen");
}

class this.GUI.CreatureBodyList extends this.GUI.ColumnList
{
	constructor()
	{
		this.GUI.ColumnList.constructor();
		local maxWidth = 0;

		foreach( x in this.Util.tableKeys(::CreatureIndex) )
		{
			maxWidth = this.Math.max(maxWidth, this.getFont().getTextMetrics(x).width);
		}

		this.addColumn("Body Template", maxWidth + 10);
		this.refreshAll();
	}

	function removeAll()
	{
		this.removeAllRows();
	}

	function refreshAll()
	{
		this.addRow([
			"-Biped-"
		]);

		foreach( x in this.Util.tableKeys(::CreatureIndex, true) )
		{
			this.addRow([
				x
			]);
		}

		this.setSelectedRows([
			0
		]);
	}

}

class this.Screens.CreatureTweakScreen extends this.GUI.Frame
{
	static mClassName = "Screens.CreatureTweakScreen";
	mCurrentTypeId = null;
	mInitialCreatureId = null;
	mNameInput = null;
	mBodyType = null;
	mBodyTypeList = null;
	mGenderList = null;
	mBipedBodyTypeList = null;
	mBipedHeadList = null;
	mRaceList = null;
	mSizeInput = null;
	mConfigOpts = null;
	mSkinColors = null;
	mClothing = null;
	mClothingButtons = null;
	mDetails = null;
	mDeferUpdate = false;
	mDontUpdate = false;
	mTabs = null;
	mStatsPage = null;
	mBodyPage = null;
	mSkinPage = null;
	mDetailPage = null;
	mClothingPage = null;
	mClothingPresetList = null;
	mClothingPresetColorsRollout = null;
	mClothingColorRollout = null;
	mClothingFilter = null;
	mColorClothingSelectionComp = null;
	mAnimContainer = null;
	mAnimDropDown = null;
	mAnimLoopCheckbox = null;
	mAnimSpeed = null;
	mTargetToRestoreAnimState = null;
	mCreatureTypeToRestoreAnimState = null;
	mAnimStateToRestore = null;
	mAttachmentPage = null;
	mAvailableAttachmentsPage = null;
	mAvailableAttachmentList = null;
	mCurrentAttachmentList = null;
	mAddButton = null;
	mEditButton = null;
	mRemoveButton = null;
	mAttachmentRollout = null;
	mAttachmentColors = null;
	mAttachmentNameFilter = null;
	mAttachmentPointFilter = null;
	mAttachmentEffects = null;
	mAttachmentDocker = null;
	mAttachmentDockerEmptyLabel = null;
	mAttachmentPointCheckBox = null;
	mInventoriesPage = null;
	mInventoriesActionContainer = null;
	mInventoryList = null;
	mAddInventoryButton = null;
	mRemoveInventoryButton = null;
	mStatEditBoxes = null;
	mScreenInitialized = false;
	mCancelVendor = null;
	mVendorLabel = null;
	mWaitingForVendor = false;
	mDebugControlMapping = null;
	mCurrentCreature = null;
	mSizeLabel = null;
	mGenderLabel = null;
	mBodyTypeLabel = null;
	mRaceLabel = null;
	mHeadPropLabel = null;
	mBodyOpts = null;
	mPropInputBox = null;
	mProp = false;
	constructor()
	{
		this.GUI.Frame.constructor("Creature Tweak");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		::_Connection.addListener(this);
		::_ItemDataManager.addListener(this);
		this.mNameInput = this.GUI.InputArea();
		this.mNameInput.addActionListener(this);
		this.mNameInput.setLocked(true);
		this.mInitialCreatureId = this._getSelectedCharacter().getID();
		this.mStatEditBoxes = {};
		this.mSkinColors = {};
		this.mDetails = {};
		this.mClothing = {};
		this.mAttachmentColors = {};
		this.mAttachmentEffects = {};
		this.mConfigOpts = {
			g = "m",
			r = "a",
			h = "0",
			sz = "1"
		};
		this.mBodyType = "-Biped-";
		this.mBodyTypeList = this.GUI.CreatureBodyList();
		this.mBodyTypeList.addActionListener(this);
		this.mGenderList = this.GUI.DropDownList();
		this.mGenderList.addChoice("Male");
		this.mGenderList.addChoice("Female");
		this.mBipedHeadList = this.GUI.DropDownList();
		this.mBipedBodyTypeList = this.GUI.DropDownList();
		local bodies = [];

		foreach( i, x in ::BodyTypes )
		{
			bodies.append(x);
		}

		bodies.sort();
		this.mBipedBodyTypeList.addChoice("-Prop-");

		foreach( i, x in ::BodyTypes )
		{
			this.mBipedBodyTypeList.addChoice(x);
		}

		this.mBipedBodyTypeList.addSelectionChangeListener(this);
		this.mRaceList = this.GUI.DropDownList();
		this.mSizeInput = this.GUI.InputArea();
		this.mSizeInput.addActionListener(this);
		this.mSizeInput.setText(this.mConfigOpts.sz.tostring());
		this.mTabs = this.GUI.TabbedPane();
		this.mBodyPage = this._buildBodyPage();
		this.mBodyPage.mDebugName = "BodyPage";
		this.mSkinPage = this.GUI.Container(this.GUI.BoxLayoutV(true));
		this.mSkinPage.mDebugName = "SkinPage";
		this.mDetailPage = this.GUI.Container(this.GUI.BoxLayoutV(true));
		this.mDetailPage.mDebugName = "DetailPage";
		this.mClothingPage = this.GUI.Container(this.GUI.BoxLayoutV(true));
		this.mClothingPage.mDebugName = "ClothingPage";
		this.mAttachmentPage = this._buildAttachmentPage();
		this.mAttachmentPage.mDebugName = "AttachmentPage";
		this.mInventoriesPage = this._buildInventoriesPage();
		this.mInventoriesPage.mDebugName = "InventoriesPage";
		this._fillRaceList(null);
		this.mGenderList.addSelectionChangeListener({
			t = this,
			function onSelectionChange( list )
			{
				this.t.mDeferUpdate = true;
				this.t._fillRaceList(this.t.mRaceList.getCurrent());
				this.t.mDeferUpdate = false;
				this.t._fillDetailsAndColors(true);
				this.t._updateAssembler();
				this.t.fillHeadList();
			}

		});
		this.mRaceList.addSelectionChangeListener({
			t = this,
			function onSelectionChange( list )
			{
				if (this.t.mDeferUpdate)
				{
					return;
				}

				this.t.mDeferUpdate = false;
				this.t._fillDetailsAndColors(true);
				this.t._updateAssembler();
				this.t.fillHeadList();
			}

		});
		this.mBipedHeadList.addSelectionChangeListener({
			t = this,
			function onSelectionChange( list )
			{
				if (this.t.mDeferUpdate)
				{
					return;
				}

				this.t.mDeferUpdate = false;
				this.t._updateAssembler();
			}

		});
		this.mTabs.add(this.mBodyPage, "Body");
		this.mTabs.add(this.mSkinPage, "Skin");
		this.mTabs.add(this.mDetailPage, "Details");
		this.mTabs.add(this.mClothingPage, "Clothing");
		this.mTabs.add(this.mAttachmentPage, "Attachments");
		local chead = this.GUI.Container(this.GUI.BorderLayout());
		chead.setInsets(0, 0, 5, 0);
		local auditButton = this.GUI.Button("Audit");
		auditButton.addActionListener(this);
		auditButton.setPressMessage("onAudit");
		local label = this.GUI.Label("Display Name");
		label.setInsets(0, 5, 0, 0);
		chead.add(label, this.GUI.BorderLayout.WEST);
		chead.add(auditButton, this.GUI.BorderLayout.EAST);
		chead.add(this.mNameInput);
		local cmain = this.GUI.Container(this.GUI.BorderLayout());
		cmain.setInsets(5);
		cmain.add(chead, this.GUI.BorderLayout.NORTH);
		cmain.add(this.mTabs, this.GUI.BorderLayout.CENTER);
		this.setContentPane(cmain);
		this.setSize(500, 360);
		this.setPosition(10, 10);
	}

	function onAudit( button )
	{
		::System.openURL("https://secure.sparkplaymedia.com/ee/auditlog/logview.php?type=CreatureDef&id=" + this.mCurrentTypeId);
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function _createStatEditBox( statId )
	{
		local c = this.GUI.CancellableInputArea("???");
		local callback = {
			tweak = this,
			statId = statId,
			function onInputComplete( ib )
			{
				::_opHistory.execute(this.CreatureDefStatEditOp(this.tweak.mCurrentTypeId, this.statId, ib.getText()));
			}

		};
		c.addActionListener(callback);
		this.mStatEditBoxes[statId] <- c;
		return c;
	}

	function _buildBodyPage()
	{
		this.mBodyOpts = this.GUI.Container(this.GUI.GridLayout(5, 2));
		this.mSizeLabel = this.GUI.Label("Size");
		this.mGenderLabel = this.GUI.Label("Gender");
		this.mBodyTypeLabel = this.GUI.Label("Body Type");
		this.mRaceLabel = this.GUI.Label("Race");
		this.mBodyOpts.add(this.mSizeLabel);
		this.mBodyOpts.add(this.mSizeInput);
		this.mBodyOpts.add(this.mGenderLabel);
		this.mBodyOpts.add(this.mGenderList);
		this.mBodyOpts.add(this.mBodyTypeLabel);
		this.mBodyOpts.add(this.mBipedBodyTypeList);
		this.mBodyOpts.add(this.mRaceLabel);
		this.mBodyOpts.add(this.mRaceList);
		this.mHeadPropLabel = this.GUI.Label("Head");
		this.mBodyOpts.add(this.mHeadPropLabel);
		this.mBodyOpts.add(this.mBipedHeadList);
		local boxlayout = this.GUI.BoxLayoutV();
		boxlayout.setExpand(true);
		this.mAnimContainer = this.GUI.Panel(boxlayout);
		this.mAnimDropDown = this.GUI.DropDownList();
		this.mAnimContainer.add(this.mAnimDropDown);
		local AnimCallback = {
			CreatureTweak = this,
			function onPackageComplete( pkg )
			{
				this.CreatureTweak._updateAnimationList();
			}

			function onPackageError( pkg, error )
			{
				this.log.debug("Error loading package " + pkg + " - " + error);
				this.onPackageComplete(pkg);
			}

		};
		this._contentLoader.load([
			"Biped-Anim-Combat",
			"Biped-Anim-Emote"
		], this.ContentLoader.PRIORITY_REQUIRED, "Effect-AnimDeps", AnimCallback);
		local rowcontainer = this.GUI.Container(this.GUI.GridLayout(1, 2));
		local label = this.GUI.Label("Speed");
		this.mAnimSpeed = this.GUI.InputArea();
		this.mAnimSpeed.setText("1.0");
		rowcontainer.add(label);
		rowcontainer.add(this.mAnimSpeed);
		this.mAnimContainer.add(rowcontainer);
		rowcontainer = this.GUI.Container(this.GUI.GridLayout(1, 2));
		local cbcontainer = this.GUI.Container(this.GUI.BoxLayout());
		this.mAnimLoopCheckbox = this.GUI.CheckBox();
		this.mAnimLoopCheckbox.setReleaseMessage("onLoopChanged");
		this.mAnimLoopCheckbox.addActionListener(this);
		label = this.GUI.Label("Loop");
		cbcontainer.add(this.mAnimLoopCheckbox);
		cbcontainer.add(label);
		local button = this.GUI.Button("Run Animation");
		button.setReleaseMessage("onRunAnimation");
		button.addActionListener(this);
		this.mAnimContainer.add(button);
		rowcontainer.add(cbcontainer);
		rowcontainer.add(button);
		this.mAnimContainer.add(rowcontainer);
		local optsbox = this.GUI.Container(this.GUI.BoxLayoutV(true));
		optsbox.add(this.mBodyOpts);
		local optsbox2 = this.GUI.Container(this.GUI.BoxLayoutV(true));
		local loadsavebox = this.GUI.Container(this.GUI.GridLayout(1, 3));
		loadsavebox.add(this.GUI.Button("Copy", this, "onCopyPressed"));
		loadsavebox.add(this.GUI.Button("Paste", this, "onPastePressed"));
		loadsavebox.add(this.GUI.Button("Clear", this, "onClearPressed"));
		optsbox2.add(loadsavebox);
		optsbox2.add(this.mAnimContainer);
		local right = this.GUI.Container(this.GUI.BorderLayout());
		right.add(optsbox, this.GUI.BorderLayout.CENTER);
		right.add(optsbox2, this.GUI.BorderLayout.SOUTH);
		local c = this.GUI.Container(this.GUI.GridLayout(1, 2));
		c.add(this.GUI.ScrollPanel(this.mBodyTypeList));
		c.add(right);
		return c;
	}

	mRaceArchiveWaiter = null;
	function _loadRaceArchives( required )
	{
		if (this.mRaceArchiveWaiter != null)
		{
			return;
		}

		local archives = [];

		foreach( i, x in ::Races )
		{
			local male = "Biped-" + x + "_Male";
			archives.append(male);
			this.print("male: " + male);
			local female = "Biped-" + x + "_Female";
			archives.append(female);
			this.print("female: " + female);
		}

		local callback = {
			tweak = this,
			function onWaitComplete( info )
			{
				this.tweak.mDeferUpdate = true;
				this.tweak._fillRaceList(null);
				this.tweak.setCurrentType(this.tweak.mCurrentTypeId);
				this.tweak.mDeferUpdate = false;
			}

		};
		this.mRaceArchiveWaiter = this.Util.waitForAssets(archives, callback, required ? this.ContentLoader.PRIORITY_REQUIRED : this.ContentLoader.PRIORITY_LOW);
	}

	function _fillRaceList( selection )
	{
		local gender = "_" + this.mGenderList.getCurrent();
		this.mRaceList.removeAll();
		local races = [];
		local key;
		local val;

		foreach( key, val in ::Races )
		{
			local choice;
			choice = val;
			races.append(choice);
		}

		races.sort();

		foreach( choice in races )
		{
			this.mRaceList.addChoice(choice);

			if (selection == choice)
			{
				this.mRaceList.setCurrent(choice, false);
			}
		}

		this._fillDetailsAndColors();
	}

	function _fillDetailsAndColors( ... )
	{
		local resetRaceColors = false;

		if (vargc > 0)
		{
			resetRaceColors = vargv[0];
		}

		local def = {};

		if (this.mBodyType == "-Biped-")
		{
			local r_g = "Biped-" + this.mRaceList.getCurrent() + "_" + this.mGenderList.getCurrent();

			if (!(r_g in ::ContentDef))
			{
				local pkgName = "CreatureTweak-" + r_g + "-ContentDef";

				if (this._contentLoader.getLoadingPackage(pkgName) != null)
				{
					return;
				}

				local callback = {
					tweak = this,
					function onPackageComplete( pkg ) : ( resetRaceColors )
					{
						this.tweak._fillDetailsAndColors(resetRaceColors);
					}

				};
				this._contentLoader.load(r_g, this.ContentLoader.PRIORITY_NORMAL, pkgName, callback);
				return;
			}
			else
			{
				def = ::ContentDef[r_g];
			}
		}
		else if (this.mBodyType in ::ModelDef)
		{
			def = ::ModelDef[this.mBodyType];
		}

		local md = ::ModelDef;
		this._fillSkinPage(def, resetRaceColors);
		this._fillDetailsPage(def);
		this._fillClothingPage(def);
		this._fillAttachmentPage();
		local sz = this.getPreferredSize();

		if (sz.width < 400)
		{
			sz.width = 400;
		}

		if (sz.height < 300)
		{
			sz.height = 300;
		}

		this.setSize(500, 355);
		this.validate();
	}

	function _fillSkinPage( def, ... )
	{
		local resetColors = false;

		if (vargc > 0)
		{
			resetColors = vargv[0];
		}

		local rows = 0;
		local c = this.GUI.Container(this.GUI.GridLayout(7, 4));
		c.getLayoutManager().setGaps(10, 10);
		c.setInsets(10, 10, 10, 10);
		local oldSkinColors = this.mSkinColors;
		this.mSkinColors = {};

		if ("Skin" in def)
		{
			local part;
			local colors;

			foreach( part, colors in def.Skin )
			{
				local name = this.TXT("creation.layers." + part);
				local list = this.GUI.ColorPicker(name, name, this.ColorRestrictionType.DEVELOPMENT);
				list.setChangeMessage("onSelectionChange");
				local color;
				local palette = [];
				local haveOldChoice = false;
				local oldChoice;
				oldChoice = def.Skin[part].def;

				if (("sk" in this.mConfigOpts) && !resetColors)
				{
					if (part in this.mConfigOpts.sk)
					{
						oldChoice = this.mConfigOpts.sk[part].tolower();
					}
				}
				else if ((part in oldSkinColors) && !resetColors)
				{
					oldChoice = oldSkinColors[part].getCurrent();
				}

				list.setColor(oldChoice, false);
				list.addActionListener(this);
				this.mSkinColors[part] <- list;
				c.add(this.GUI.Label("Skin: " + part));
				c.add(list, this.GUI.GridLayout.CENTER);
				rows += 1;
			}
		}

		this.mSkinPage.removeAll();
		this.mSkinPage.add(c);

		if (rows == 0)
		{
			this.mSkinPage.add(this.GUI.Label("No colorizable skin regions for " + this._bodyTypeDesc()));
		}

		local key;
		local list;

		foreach( key, list in oldSkinColors )
		{
			list.destroy();
		}
	}

	function _fillDetailsPage( def )
	{
		local c = this.GUI.Container();
		local rows = 0;
		local oldDetails = this.mDetails;
		this.mDetails = {};

		if ("Details" in def)
		{
			local part;
			local sectionData;

			foreach( part, sectionData in def.Details )
			{
				local list = this.GUI.DropDownList();
				list.addChoice("-none-");
				local choice;
				local haveDefault = false;
				local haveOldChoice = false;
				local oldChoice;

				if (("d" in this.mConfigOpts) && part in this.mConfigOpts.d)
				{
					oldChoice = this.mConfigOpts.d[part];
				}
				else if (part in oldDetails)
				{
					oldChoice = oldDetails[part].getCurrent();
				}

				foreach( choice in this.Util.tableKeys(sectionData, true) )
				{
					if (choice == "default")
					{
						haveDefault = true;
					}

					if (choice == oldChoice)
					{
						haveOldChoice = true;
					}

					list.addChoice(choice);
				}

				if (haveOldChoice)
				{
					list.setCurrent(oldChoice, false);
				}
				else if (haveDefault)
				{
					list.setCurrent("default", false);
				}

				list.addSelectionChangeListener(this);
				this.mDetails[part] <- list;
				c.add(this.GUI.Label("Detail: " + part));
				c.add(list);
				rows += 1;
			}
		}

		this.mDetailPage.removeAll();
		c.setLayoutManager(this.GUI.GridLayout(rows, 2));
		this.mDetailPage.add(c);

		if (rows == 0)
		{
			this.mDetailPage.add(this.GUI.Label("No details defined for " + this._bodyTypeDesc()));
		}

		local key;
		local list;

		foreach( key, list in oldDetails )
		{
			list.destroy();
		}
	}

	function _fillClothingPage( def )
	{
		if (this.mBodyType != "-Biped-")
		{
			this.mClothingPage.removeAll();
			this.mClothingPage.add(this.GUI.Label("Clothing not supported for " + this._bodyTypeDesc()));
			return;
		}

		local c = this.GUI.Container();
		local rows = 0;
		local oldClothing = this.mClothing;
		this.mClothing = {};
		this.mClothingButtons = {};
		this.mColorClothingSelectionComp = {};
		this.mClothingPresetList = this.GUI.DropDownList();
		this.mClothingPresetList.addChoice("-none-");
		local choice;

		foreach( choice in this.Util.tableKeys(::ClothingDef, true) )
		{
			this.mClothingPresetList.addChoice(choice);
		}

		this.mClothingPresetList.addSelectionChangeListener({
			t = this,
			function onSelectionChange( list )
			{
				this.t.mDeferUpdate = true;
				local slot;

				foreach( slot in ::ClothingSlots )
				{
					this.t.mClothing[slot].setCurrent(list.getCurrent());
				}

				this.t.mDeferUpdate = false;
				this.t._updateAssembler();
			}

		});
		c.add(this.GUI.Label("Name Filter:"));
		this.mClothingFilter = this.GUI.InputArea();
		this.mClothingFilter.addActionListener(this);
		c.add(this.mClothingFilter);
		c.add(this.GUI.Label(""));
		c.add(this.GUI.Spacer());
		c.add(this.GUI.Spacer());
		rows++;
		c.add(this.GUI.Label("Clothing Set:"));
		c.add(this.mClothingPresetList);
		c.add(this.GUI.Label(""));
		c.add(this.GUI.Spacer());
		c.add(this.GUI.Spacer());
		rows++;
		local slot;

		foreach( slot in ::ClothingSlots )
		{
			local list = this.GUI.DropDownList();
			list.addChoice("-none-");
			local choice;
			local haveOldChoice = false;
			local oldChoice;

			if ("c" in this.mConfigOpts)
			{
				if (this.type(this.mConfigOpts.c) == "string")
				{
					this.mConfigOpts.c = this.System.decodeVars(this.mConfigOpts.c);
				}

				if (slot in this.mConfigOpts.c)
				{
					if (this.type(this.mConfigOpts.c[slot]) == "string")
					{
						this.mConfigOpts.c[slot] = this.System.decodeVars(this.mConfigOpts.c[slot]);
					}

					if ("type" in this.mConfigOpts.c[slot])
					{
						oldChoice = this.mConfigOpts.c[slot].type;
					}
				}
			}
			else if (slot in oldClothing)
			{
				oldChoice = oldClothing[slot].getCurrent();
			}

			foreach( choice in this.Util.tableKeys(::ClothingDef, true) )
			{
				if ("regions" in this.ClothingDef[choice])
				{
					if (slot in this.ClothingDef[choice].regions)
					{
						if (choice == oldChoice)
						{
							haveOldChoice = true;
						}

						list.addChoice(choice);
					}
				}
				else
				{
					if (choice == oldChoice)
					{
						haveOldChoice = true;
					}

					list.addChoice(choice);
				}
			}

			if (haveOldChoice)
			{
				list.setCurrent(oldChoice, false);
			}

			list.addSelectionChangeListener({
				t = this,
				function onSelectionChange( list )
				{
					local slot = list.getData();
					local type = list.getCurrent();

					for( local i = this.t.mColorClothingSelectionComp[slot].components.len() - 1; i >= 0; i = i - 1 )
					{
						this.t.mColorClothingSelectionComp[slot].remove(this.t.mColorClothingSelectionComp[slot].components[i]);
					}

					delete this.t.mClothingButtons[slot];

					if (("c" in this.t.mConfigOpts) && (slot in this.t.mConfigOpts.c) && "colors" in this.t.mConfigOpts.c[slot])
					{
						this.t.mClothingButtons[slot] <- this.GUI.ClothingColorRollout(type, this.t.mConfigOpts.c[slot].colors);
					}
					else
					{
						this.t.mClothingButtons[slot] <- this.GUI.ClothingColorRollout(type);
					}

					this.t.mClothingButtons[slot].addActionListener(this.t);
					this.t.mColorClothingSelectionComp[slot].add(this.t.mClothingButtons[slot]);
					this.t._updateAssembler();
				}

			});
			list.setData(slot);
			this.print("Slot on Selection change data " + slot);
			this.mClothing[slot] <- list;
			c.add(this.GUI.Label("" + slot));
			c.add(list);

			if (("c" in this.mConfigOpts) && (slot in this.mConfigOpts.c) && "colors" in this.mConfigOpts.c[slot])
			{
				this.mClothingButtons[slot] <- this.GUI.ClothingColorRollout(list.getCurrent(), this.mConfigOpts.c[slot].colors);
			}
			else
			{
				this.mClothingButtons[slot] <- this.GUI.ClothingColorRollout(list.getCurrent());
			}

			this.mClothingButtons[slot].addActionListener(this);
			this.mColorClothingSelectionComp[slot] <- this.GUI.Component(this.GUI.BoxLayout());
			this.mColorClothingSelectionComp[slot].add(this.mClothingButtons[slot]);
			c.add(this.mColorClothingSelectionComp[slot]);
			local b = this.GUI.Button("C", this, "onClothingSlotCopy");
			b.setData(slot);
			c.add(b);
			b = this.GUI.Button("P", this, "onClothingSlotPaste");
			b.setData(slot);
			c.add(b);
			rows += 1;
		}

		this.mClothingPage.removeAll();
		c.setLayoutManager(this.GUI.GridLayout(rows, 5));
		c.getLayoutManager().setColumns(70, 150, "*", 20, 20);
		this.mClothingPage.add(c);
		local key;
		local list;

		foreach( key, list in oldClothing )
		{
			list.destroy();
		}
	}

	function onClothingSlotCopy( button )
	{
		local slot = button.getData();
		local type = this.mClothing[slot].getCurrent();
		local colors = this.serialize(this.mConfigOpts.c[slot].colors);
		::System.setClipboard("{c={type=\"" + type + "\", colors=" + colors + "}}");
	}

	function onClothingSlotPaste( button )
	{
		local slot = button.getData();

		try
		{
			local data = this.unserialize(::System.getClipboard());

			if ("c" in data)
			{
				this.mClothingButtons[slot].setCurrent(data.c.colors);
				this.mClothing[slot].setCurrent(data.c.type);
			}
		}
		catch( err )
		{
			this.log.debug("Error while pasting clothing: " + err);
		}
	}

	function _filterClothingPageLists( term )
	{
		local oldChoice = this.mClothingPresetList.getCurrent();
		local oldChoiceFound = false;
		this.mClothingPresetList.removeAll();

		foreach( choice in this.Util.tableKeys(::ClothingDef, true) )
		{
			if (choice.toupper().find(term.toupper()) != null && choice == oldChoice)
			{
				oldChoiceFound = true;
			}
		}

		if (oldChoiceFound == false && oldChoice != "-none-")
		{
			this.mClothingPresetList.addChoice(oldChoice);
		}

		this.mClothingPresetList.addChoice("-none-");

		foreach( choice in this.Util.tableKeys(::ClothingDef, true) )
		{
			if (choice.toupper().find(term.toupper()) != null)
			{
				this.mClothingPresetList.addChoice(choice);
			}
		}

		if (oldChoice != "")
		{
			this.mClothingPresetList.setCurrent(oldChoice, false);
		}

		local slot;

		foreach( slot in ::ClothingSlots )
		{
			local list = this.mClothing[slot];
			oldChoiceFound = false;
			oldChoice = list.getCurrent();
			list.removeAll();

			foreach( choice in this.Util.tableKeys(::ClothingDef, true) )
			{
				if (choice.toupper().find(term.toupper()) != null && choice == oldChoice)
				{
					oldChoiceFound = true;
				}
			}

			if (oldChoiceFound == false && oldChoice != "-none-")
			{
				list.addChoice(oldChoice);
			}

			list.addChoice("-none-");
			local choice;

			foreach( choice in this.Util.tableKeys(::ClothingDef, true) )
			{
				if (choice.toupper().find(term.toupper()) != null)
				{
					if ("regions" in this.ClothingDef[choice])
					{
						if (slot in this.ClothingDef[choice].regions)
						{
							list.addChoice(choice);
						}
					}
					else
					{
						list.addChoice(choice);
					}
				}
			}

			if (oldChoice != "")
			{
				list.setCurrent(oldChoice, false);
			}
		}
	}

	mAttachmentWaiter = null;
	function _preloadAllAttachemnts()
	{
		if (this.mAttachmentWaiter != null)
		{
			return;
		}

		local deps = [];

		foreach( i, x in ::AttachableIndex )
		{
			deps.append(this.GetAssetArchive(i));
		}

		this.mAttachmentWaiter = this.Util.waitForAssets(deps, this, this.ContentLoader.PRIORITY_REQUIRED);
	}

	function _fillAvailableAttachments()
	{
		this._preloadAllAttachemnts();
		local aal = this.mAvailableAttachmentList = this.GUI.ColumnList();
		this.mAvailableAttachmentList.setPreferredSize(200, 400);
		local available = [];

		foreach( i, x in ::AttachableIndex )
		{
			available.append(i);
		}

		available.sort();
		local maxWidth = 0;

		foreach( x in available )
		{
			maxWidth = this.Math.max(maxWidth, aal.getFont().getTextMetrics(x).width);
		}

		aal.addColumn("Available", maxWidth + 10);

		foreach( x in available )
		{
			aal.addRow([
				x
			]);
		}

		this.mAvailableAttachmentsPage = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mAvailableAttachmentsPage.getLayoutManager().setExpand(true);
		local options = this.GUI.Container(this.GUI.GridLayout(2, 2));
		options.add(this.GUI.Label("Name Filter:"));
		this.mAttachmentNameFilter = this.GUI.InputArea();
		this.mAttachmentNameFilter.addActionListener(this);
		options.add(this.mAttachmentNameFilter);
		options.add(this.GUI.Label("Attachment Point Filter:"));
		this.mAttachmentPointFilter = this.GUI.DropDownList();
		this.mAttachmentPointFilter.addChoice("-all-");
		this.mAttachmentPointFilter.addSelectionChangeListener(this);
		available = [];

		foreach( def in ::AttachableDef )
		{
			if ("attachPoints" in def)
			{
				foreach( i, x in def.attachPoints )
				{
					local found = false;

					foreach( known in available )
					{
						if (known == x)
						{
							found = true;
							break;
						}
					}

					if (found == false)
					{
						available.append(x);
					}
				}
			}
		}

		available.sort();

		foreach( x in available )
		{
			this.mAttachmentPointFilter.addChoice(x.tostring());
		}

		options.add(this.mAttachmentPointFilter);
		this.mAvailableAttachmentsPage.add(options);
		this.mAvailableAttachmentsPage.add(this.GUI.ScrollPanel(this.mAvailableAttachmentList));
	}

	function _filterAttachmentsList( term, point )
	{
		this.mAvailableAttachmentList.removeAllRows();
		local available = [];

		foreach( i, x in ::AttachableIndex )
		{
			available.append(i);
		}

		available.sort();
		local maxWidth = 0;

		foreach( x in available )
		{
			maxWidth = this.Math.max(maxWidth, this.mAvailableAttachmentList.getFont().getTextMetrics(x).width);
		}

		foreach( x in available )
		{
			if (x.toupper().find(term.toupper()) != null)
			{
				if (point == "-all-")
				{
					this.mAvailableAttachmentList.addRow([
						x
					]);
				}
				else if ((x in ::AttachableDef) && "attachPoints" in ::AttachableDef[x])
				{
					foreach( p in ::AttachableDef[x].attachPoints )
					{
						if (p == point)
						{
							this.mAvailableAttachmentList.addRow([
								x
							]);
							break;
						}
					}
				}
			}
		}
	}

	function _buildAttachmentPage()
	{
		local page = this.GUI.Container(this.GUI.GridLayout(1, 3));
		page.getLayoutManager().setColumns("*", 160, 70);
		page.getLayoutManager().setGaps(5, 5);
		this.mAddButton = this.GUI.Button("Add", this, "onAddAttachment");
		this.mEditButton = this.GUI.Button("Edit", this, "onEditAttachment");
		this.mRemoveButton = this.GUI.Button("Remove", this, "onRemoveAttachment");
		local copyButton = this.GUI.Button("Copy", this, "onCopyAttachment");
		local pasteButton = this.GUI.Button("Paste", this, "onPasteAttachment");
		local cal = this.mCurrentAttachmentList = this.GUI.ColumnList();
		cal.addColumn("Attachment", 100);
		cal.addColumn("Point", 60);
		cal.addActionListener(this);
		local container = this.GUI.Container(this.GUI.BoxLayoutV(true));
		container.add(this.mAddButton);
		container.add(this.mRemoveButton);
		container.add(this.GUI.Spacer(5, 50));
		container.add(copyButton);
		container.add(pasteButton);
		this.mAttachmentDocker = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mAttachmentDocker.getLayoutManager().setAlignment(0.0);
		this.mAttachmentDockerEmptyLabel = this.GUI.Label("Select attachment\nto edit");
		this.mAttachmentDockerEmptyLabel.setTextAlignment(0.5, 0.5);
		this.mAttachmentDocker.add(this.mAttachmentDockerEmptyLabel);
		page.add(cal);
		page.add(this.mAttachmentDocker);
		page.add(container);
		this._fillAvailableAttachments();
		this._fillAttachmentPage();
		return page;
	}

	function _fillAttachmentPage()
	{
		local page = this.mAttachmentPage;
		local cal = this.mCurrentAttachmentList;
		this.mCurrentAttachmentList.removeAllRows();

		if (this.mProp == true)
		{
			return;
		}

		if (("a" in this.mConfigOpts) && this.mConfigOpts.a.len() > 0)
		{
			foreach( i, x in this.mConfigOpts.a )
			{
				cal.insertRow(i, [
					x.type,
					x.node
				]);

				if ("effect" in x)
				{
					this.mAttachmentEffects[i] <- x.effect;
				}
				else if ((i in this.mAttachmentEffects) && "effect" in this.mAttachmentEffects[i])
				{
					delete this.mAttachmentEffects[i];
				}

				if ("colors" in x)
				{
					this.mAttachmentColors[i] <- x.colors;
				}
				else if ((i in this.mAttachmentColors) && "colors" in this.mAttachmentColors[i])
				{
					delete this.mAttachmentColors[i].colors;
				}
			}
		}
	}

	function _buildInventoriesPage()
	{
		local page = this.GUI.Container(this.GUI.BoxLayoutV());
		local topcontainer = this.GUI.Container(this.GUI.BoxLayout());
		page.add(topcontainer);
		this.mInventoryList = this.GUI.DropDownList();
		this.mInventoryList.addChoice("shop");
		this.mInventoryList.addChoice("vendor");
		this.mInventoryList.setSize(240, 22);
		topcontainer.add(this.mInventoryList);
		this.mAddInventoryButton = this.GUI.Button("Add", this, "onAddInventory");
		topcontainer.add(this.mAddInventoryButton);
		this.mRemoveInventoryButton = this.GUI.Button("Remove", this, "onRemoveInventory");
		topcontainer.add(this.mRemoveInventoryButton);
		this.mInventoriesActionContainer = this.GUI.InventoryActionContainer("creaturetweak_inventory", 6, 12, 0, 0, this, this.mInitialCreatureId, "shop");
		this.mInventoriesActionContainer.addListener(this);
		this.mInventoriesActionContainer.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.MOVE, this));
		this.mInventoriesActionContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(null, this));
		this.mInventoriesActionContainer.setVisible(false);
		page.add(this.mInventoriesActionContainer);
		this.mVendorLabel = this.GUI.Label("Please select a shop owner or  ");
		topcontainer.add(this.mVendorLabel);
		this.mCancelVendor = this.GUI.Button("Cancel", this, "onCancelVendor");
		topcontainer.add(this.mCancelVendor);
		this.mVendorLabel.setVisible(false);
		this.mCancelVendor.setVisible(false);
		return page;
	}

	function getActionContainer()
	{
		return this.mInventoriesActionContainer;
	}

	function onActionButtonGained( newslot, oldslot )
	{
		local itemid = oldslot.getActionButton().getAction().getItemId();
		this._Connection.sendQuery("shop.add", null, [
			itemid,
			this.mInitialCreatureId
		]);
	}

	function onActionButtonLost( newslot, oldslot )
	{
		local itemid = newslot.getActionButton().getAction().getItemId();
		this._Connection.sendQuery("shop.remove", null, [
			itemid,
			this.mInitialCreatureId
		]);
	}

	function onAttemptedItemDisown( container, action, slotIndex )
	{
		local itemid = action.getItemId();
		this.mInventoriesActionContainer.removeAction(action);
		this._Connection.sendQuery("shop.remove", null, [
			itemid,
			this.mInitialCreatureId
		]);
	}

	function onAddInventory( button )
	{
		local containerName = this.mInventoryList.getCurrent();

		if (containerName == "vendor")
		{
			this.mCancelVendor.setVisible(true);
			this.mVendorLabel.setVisible(true);
			this.mInventoryList.setVisible(false);
			this.mAddInventoryButton.setVisible(false);
			this.mRemoveInventoryButton.setVisible(false);
			this.mWaitingForVendor = true;
		}
		else
		{
			this._Connection.sendQuery("creature.def.edit", null, [
				this.mCurrentTypeId,
				containerName,
				"true"
			]);
			this.mInventoriesActionContainer.setSourceContainer(this.mInitialCreatureId, containerName);
		}
	}

	function linkVendor( target )
	{
		if (!this.mCurrentCreature)
		{
			this.mCurrentCreature = this._avatar.getTargetObject();
		}

		if (this.mCurrentCreature && target)
		{
			this._Connection.sendQuery("trade.vendor", null, [
				target.getID(),
				this.mCurrentCreature.getID()
			]);
		}

		this.onCancelVendor(null);
	}

	function onCancelVendor( button )
	{
		this.mCancelVendor.setVisible(false);
		this.mVendorLabel.setVisible(false);
		this.mInventoryList.setVisible(true);
		this.mAddInventoryButton.setVisible(true);
		this.mRemoveInventoryButton.setVisible(true);
		this.mWaitingForVendor = false;
	}

	function onRemoveInventory( button )
	{
		local confirm = this.GUI.ConfirmationWindow();
		confirm.setConfirmationType(this.GUI.ConfirmationWindow.OK_CANCEL);
		confirm.setText("Are you sure you want to remove this inventory and destroy all items?");
		confirm.addActionListener(this);
	}

	function onConfirmation( window, confirm )
	{
		if (confirm)
		{
			local containerName = this.mInventoryList.getCurrent();
			this._Connection.sendQuery("creature.def.edit", null, [
				this.mCurrentTypeId,
				containerName,
				"false"
			]);
			this.mInventoriesActionContainer.setVisible(false);
		}
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (creatureId == this.mInitialCreatureId && containerName == this.mInventoryList.getCurrent())
		{
			this.mInventoriesActionContainer.setVisible(true);
		}
	}

	function onContainerInvalid( containerName, creatureId )
	{
		if (creatureId == this.mInitialCreatureId && containerName == this.mInventoryList.getCurrent())
		{
			this.mInventoriesActionContainer.setVisible(false);
		}
	}

	function _setConfigOpt( name, subopts )
	{
		local t = {};
		local key;
		local list;
		local haveDefault = false;

		foreach( key, list in subopts )
		{
			local value = list.getCurrent();

			if (value == "default")
			{
				continue;
			}

			if (value == "-none-")
			{
				local c;

				foreach( c in list.mChoices )
				{
					if (c == "default")
					{
						t[key] <- "";
						break;
					}
				}

				continue;
			}

			if (value == null)
			{
				if (list.mChoices.len() != 0)
				{
					value = list.mChoices[0].toHexString();
				}

				continue;
			}

			t[key] <- value;
		}

		if (t.len() > 0)
		{
			this.mConfigOpts[name] <- t;
		}
		else if (name in this.mConfigOpts)
		{
			delete this.mConfigOpts[name];
		}
	}

	function _setConfigClothingOpt()
	{
		local t = {};
		local key;
		local list;
		local value = this.mClothingPresetList.getCurrent();

		if (this.mClothingPresetColorsRollout && this.mClothingPresetColorsRollout.mType != value)
		{
			this.mClothingPresetColorsRollout.setType(value);
		}

		foreach( key, list in this.mClothing )
		{
			value = list.getCurrent();

			if (value == "-none-")
			{
				continue;
			}

			t[key] <- {};
			t[key].type <- value;

			if (this.mClothingButtons[key].mType != value)
			{
				this.mClothingButtons[key].setType(value);
			}

			t[key].colors <- [];

			foreach( i, c in this.mClothingButtons[key].getCurrent() )
			{
				t[key].colors.append(c);
			}
		}

		if (t.len() > 0)
		{
			this.mConfigOpts.c <- t;
		}
		else if ("c" in this.mConfigOpts)
		{
			delete this.mConfigOpts.c;
		}
	}

	function _setConfigAttachmentOpt()
	{
		local a = [];
		local list = this.mCurrentAttachmentList.mRowContents;
		local clist = this.mAttachmentColors;
		local elist = this.mAttachmentEffects;

		if (list.len() > 0)
		{
			foreach( key, row in list )
			{
				local t = {};
				t.type <- row[0];
				t.node <- row[1];

				if (key in elist)
				{
					t.effect <- elist[key];
				}

				if (key in clist)
				{
					t.colors <- clist[key];
				}

				a.insert(key, t);
			}
		}

		if (a.len() > 0)
		{
			this.mConfigOpts.a <- a;
		}
		else if ("a" in this.mConfigOpts)
		{
			delete this.mConfigOpts.a;
		}
	}

	function _getConfigurationC1()
	{
		this.mConfigOpts = {};
		local raceKey;
		local race;

		foreach( raceKey, race in ::Races )
		{
			if (race == this.mRaceList.getCurrent())
			{
				this.mConfigOpts.r <- raceKey;
				break;
			}
		}

		if (!("r" in this.mConfigOpts))
		{
			this.log.warn("Race not found: " + this.mRaceList.getCurrent());
			this.mConfigOpts.r <- "b";
		}

		this.mConfigOpts.g <- this.mGenderList.getCurrent() == "Male" ? "m" : "f";
		local szStr = this.mSizeInput.getText();

		if (szStr != "")
		{
			this.mConfigOpts.sz <- szStr;
		}

		this._setConfigOpt("d", this.mDetails);
		this._setConfigOpt("sk", this.mSkinColors);
		this._setConfigClothingOpt();
		return "c1:" + this.System.encodeVars(this.mConfigOpts);
	}

	function _getConfigurationC2()
	{
		this.mConfigOpts = {};
		local raceKey;
		local race;

		foreach( raceKey, race in ::Races )
		{
			if (race == this.mRaceList.getCurrent())
			{
				this.mConfigOpts.r <- raceKey;
				break;
			}
		}

		if (!("r" in this.mConfigOpts))
		{
			this.log.warn("Race not found: " + this.mRaceList.getCurrent());
			this.mConfigOpts.r <- "b";
		}

		this.mConfigOpts.g <- this.mGenderList.getCurrent() == "Male" ? "m" : "f";
		local bodyType = this.mBipedBodyTypeList.getCurrent();

		foreach( i, x in ::BodyTypes )
		{
			if (bodyType == x)
			{
				bodyType = i;
				break;
			}
		}

		this.mConfigOpts.b <- bodyType;
		local szStr = this.mSizeInput.getText();

		if (szStr != "")
		{
			this.mConfigOpts.sz <- szStr;
		}

		local headIndex = this.mBipedHeadList.getCurrentIndex();

		if (headIndex >= 0)
		{
			this.mConfigOpts.h <- headIndex;
		}

		this._setConfigOpt("d", this.mDetails);
		this._setConfigOpt("sk", this.mSkinColors);
		this._setConfigClothingOpt();
		this._setConfigAttachmentOpt();
		return "c2:" + this.serialize(this.mConfigOpts);
	}

	function _getConfigurationN4()
	{
		this.mConfigOpts = {};
		this.mConfigOpts.c <- this.mBodyType;
		local szStr = this.mSizeInput.getText();

		if (szStr != "")
		{
			this.mConfigOpts.sz <- szStr;
		}

		this._setConfigAttachmentOpt();
		this._setConfigOpt("sk", this.mSkinColors);
		return "n4:" + this.serialize(this.mConfigOpts);
	}

	function _getConfigurationP1()
	{
		this.mConfigOpts = {};
		this.mConfigOpts.a <- this.mPropInputBox.getText();
		local szStr = this.mSizeInput.getText();

		if (szStr != "")
		{
			this.mConfigOpts.sz <- szStr;
		}

		return "p1:" + this.serialize(this.mConfigOpts);
	}

	function getAppearanceConfig()
	{
		if (this.mProp)
		{
			return this._getConfigurationP1();
		}
		else if (this.mBodyType == "-Biped-")
		{
			return this._getConfigurationC2();
		}
		else
		{
			return this._getConfigurationN4();
		}
	}

	function fillHeadList()
	{
		local current = this.mBipedHeadList.getCurrent();
		local headIndex = 0;
		this.mBipedHeadList.removeAll();

		if (this.mBodyType == "-Biped-")
		{
			local r_g = "Biped-" + this.mRaceList.getCurrent() + "_" + this.mGenderList.getCurrent();

			if (r_g in ::ContentDef)
			{
				local def = ::ContentDef[r_g];

				if ("Heads" in def)
				{
					foreach( h in def.Heads )
					{
						this.mBipedHeadList.addChoice("Head " + headIndex++);
					}
				}
			}

			this.mBipedHeadList.setCurrent(current, false);
		}
	}

	function setAppearanceConfig( config )
	{
		if (config == null)
		{
			return false;
		}

		local protocal = config.slice(0, 3);
		this.mProp = false;

		switch(protocal)
		{
		case "c1:":
			this.mConfigOpts = this.System.decodeVars(config.slice(3));
			this._selectBodyType("-Biped-");
			break;

		case "c2:":
			this.mConfigOpts = this.unserialize(config.slice(3));
			this._selectBodyType("-Biped-");
			break;

		case "n4:":
			this.mConfigOpts = this.unserialize(config.slice(3));
			this._selectBodyType(this.mConfigOpts.c);
			break;

		case "p1:":
			this.mProp = true;
			this.updateScreenOnBodyChanged();
			this.mConfigOpts = this.unserialize(config.slice(3));
			this.mPropInputBox.setAsset(this.mConfigOpts.a);

			if ("sz" in this.mConfigOpts)
			{
				this.mSizeInput.setText(this.mConfigOpts.sz.tostring());
			}
			else
			{
				this.mSizeInput.setText("1");
			}

			this.mBodyType = "-Prop-";
			this.mConfigOpts = null;
			break;

		default:
			this.log.warn("Invalid config for tweak: " + config);
			return false;
		}

		this.fillHeadList();
		this.mDeferUpdate = true;

		if (this.mBodyType == "-Biped-")
		{
			this.mBipedHeadList.setCurrent("h" in this.mConfigOpts ? "Head " + this.mConfigOpts.h : "Head 0");

			if ("g" in this.mConfigOpts)
			{
				this.mGenderList.setCurrent(this.mConfigOpts.g == "m" ? "Male" : "Female", false);
			}

			if ("r" in this.mConfigOpts)
			{
				this.mRaceList.setCurrent(::Races[this.mConfigOpts.r], false);
			}
		}

		if (this.mProp == false)
		{
			if ("sz" in this.mConfigOpts)
			{
				this.mSizeInput.setText(this.mConfigOpts.sz.tostring());
			}
			else
			{
				this.mSizeInput.setText("1");
			}
		}

		local bodyType = "Normal";

		if ("b" in this.mConfigOpts)
		{
			if (this.mConfigOpts.b in ::BodyTypes)
			{
				bodyType = ::BodyTypes[this.mConfigOpts.b];
			}
		}

		if (this.mProp)
		{
			bodyType = "-Prop-";
		}

		this.mBipedBodyTypeList.setCurrent(bodyType, false);
		this._fillDetailsAndColors();
		this.mDeferUpdate = false;
		return true;
	}

	function _updateAssembler()
	{
		if (this.mDeferUpdate || this.mDontUpdate)
		{
			return;
		}

		local config = this.getAppearanceConfig();

		if (!this.GetAssembler("Creature", this.mCurrentTypeId, false) || config == this.Assembler.Creature.DEFAULT_APPEARANCE)
		{
			this.log.warn("Not creating new assembler for: " + this.mCurrentTypeId);
			return;
		}

		this._opHistory.execute(this.CreatureDefStatEditOp(this.mCurrentTypeId, this.Stat.APPEARANCE, config));
		local target;

		if (this._avatar)
		{
			if (!target)
			{
				target = this._avatar.getTargetObject();
			}

			if (!target)
			{
				target = this._avatar;
			}
		}

		if (target && target.getAnimationHandler())
		{
			this.mTargetToRestoreAnimState = target;
			this.mCreatureTypeToRestoreAnimState = target.mAssembler.mBody;
			this.mAnimStateToRestore = target.getAnimationHandler().getAnimationState();
		}

		this.mMessageBroadcaster.broadcastMessage("onInputComplete", this);
	}

	function isTweakingAvatarType()
	{
		return this._avatar && this._avatar.getType() == this.mCurrentTypeId;
	}

	function getCurrentType()
	{
		return this.mCurrentTypeId;
	}

	function setCurrentType( id, ... )
	{
		this.log.debug("CreatureTweak.setCurrentType(" + id + ")");
		this.mCurrentTypeId = id;
		local title = "Creature Tweak: ";

		if (vargc > 0)
		{
			title += "" + vargv[0] + " ";
		}

		title += "[DEF#" + id + "]";

		if (this.isTweakingAvatarType())
		{
			title += " (AVATAR)";
		}
		else if (this._avatar && this._avatar.getTargetObject() && this._avatar.getTargetObject().getType() == id)
		{
			title += " (SELECTED)";
		}

		this.setTitle(title);
		local a = ::_creatureDefManager.getCreatureDef(id);

		if (a)
		{
			this.mDontUpdate = true;
			this.setAppearanceConfig(a.getAssembler().getConfig());
			this.mDontUpdate = false;
			this.mNameInput.setText(a.getStat(this.Stat.DISPLAY_NAME));

			foreach( statId, inputbox in this.mStatEditBoxes )
			{
				local value = a.getStat(statId);

				if (value != null)
				{
					inputbox.setText("" + value);
				}
			}
		}
		else
		{
			if (::_Connection)
			{
				this._Connection.sendInspectCreatureDef(id.tointeger());
			}

			this.mNameInput.setText("???");

			foreach( statId, inputbox in this.mStatEditBoxes )
			{
				inputbox.setText("???");
			}
		}
	}

	function onInputComplete( inputbox )
	{
		if (inputbox == this.mNameInput)
		{
			this._opHistory.execute(this.CreatureDefStatEditOp(this.mCurrentTypeId, this.Stat.DISPLAY_NAME, this.mNameInput.getText()));
		}
		else if (inputbox == this.mClothingFilter)
		{
			this._filterClothingPageLists(this.mClothingFilter.getText());
		}
		else if (inputbox == this.mAttachmentNameFilter)
		{
			this._filterAttachmentsList(this.mAttachmentNameFilter.getText(), this.mAttachmentPointFilter.getCurrent());
		}
		else if (inputbox == this.mPropInputBox)
		{
			this._opHistory.execute(this.CreatureDefStatEditOp(this.mCurrentTypeId, this.Stat.APPEARANCE, "p1:{[\"a\"]=\"" + inputbox.getAsset() + "\"}"));
			this._updateAssembler();
		}
		else
		{
			this._updateAssembler();
		}
	}

	function onSelectionChange( list )
	{
		if (list == this.mBodyTypeList)
		{
			this._updateAssembler();
		}
		else if (list == this.mAttachmentPointFilter)
		{
			this._filterAttachmentsList(this.mAttachmentNameFilter.getText(), this.mAttachmentPointFilter.getCurrent());
		}
		else if (list == this.mBipedBodyTypeList)
		{
			if (this.mBipedBodyTypeList.getCurrent() == "-Prop-")
			{
				this.mProp = true;
			}
			else
			{
				this.mProp = false;
				this._updateAssembler();
			}

			this.updateScreenOnBodyChanged();
		}
		else
		{
			this._updateAssembler();
		}
	}

	function onRowSelectionChanged( list, index, selected )
	{
		if (list == this.mCurrentAttachmentList)
		{
			this.mAttachmentDocker.removeAll();

			if (selected)
			{
				local r = this.mCurrentAttachmentList.getRow(index);
				local type = r[0];
				local attachPoint = r[1];

				if (index in this.mAttachmentColors)
				{
					this.mAttachmentRollout = this.GUI.AttachmentEditPanel(this, type, attachPoint, this.mAttachmentColors[index]);
				}
				else
				{
					this.mAttachmentRollout = this.GUI.AttachmentEditPanel(this, type, attachPoint, null);
				}

				if (index in this.mAttachmentEffects)
				{
					this.mAttachmentRollout.setEffect(this.mAttachmentEffects[index]);
				}

				this.mAttachmentDocker.add(this.mAttachmentRollout);
				local attachmentLayout = this.GUI.Container(this.GUI.BoxLayout());
				this.mAttachmentDocker.add(attachmentLayout);
				this.mAttachmentPointCheckBox = this.GUI.CheckBox();
				this.mAttachmentPointCheckBox.setSize(16, 16);
				this.mAttachmentPointCheckBox.setFixedSize(16, 16);
				attachmentLayout.add(this.mAttachmentPointCheckBox);
				local attachmentLabel = this.GUI.Label("Override attachment point");
				attachmentLayout.add(attachmentLabel);
			}
			else
			{
				this.mAttachmentDocker.add(this.mAttachmentDockerEmptyLabel);
			}

			return;
		}

		if (!selected)
		{
			return;
		}

		local row = list.getRow(index);
		this.mBodyType = row[0];
		this.fillHeadList();

		if (this.mBodyType != "-Biped-")
		{
			local archive = this.GetAssetArchive(this.mBodyType);
			local pkgName = "CreatureTweak-" + this.mBodyType + "-Deps";
			local callback = {
				tweak = this,
				function onWaitComplete( info )
				{
					this.tweak.onBodyTypeReady();
					this.tweak._updateAssembler();
				}

			};
			this.Util.waitForAssets(archive, callback, this.ContentLoader.PRIORITY_REQUIRED);
		}
		else
		{
			this.onBodyTypeReady();
			this._updateAssembler();
		}
	}

	function onPackageComplete( pkg )
	{
	}

	function onAddAttachment( source )
	{
		this.GUI.MessageBox.showEx(this.mAvailableAttachmentsPage, [
			"Attach",
			"Cancel"
		], this, "onAttachmentSelected");
	}

	function onAttachmentSelected( source, action )
	{
		if (action != "Attach")
		{
			return;
		}

		local sri = this.mAvailableAttachmentList.getSelectedRows();

		if (sri.len() != 1)
		{
			return;
		}

		local r = this.mAvailableAttachmentList.mRowContents[sri[0]];
		local attachment = r[0];

		if (attachment in ::AttachableDef)
		{
			if (!("attachPoints" in ::AttachableDef[attachment]))
			{
				throw this.Exception("attachmentPoint not set for item: " + attachment);
			}

			local attachpoint = ::AttachableDef[attachment].attachPoints[0];

			if (this.mAttachmentPointFilter.getCurrent() != "-all-")
			{
				attachpoint = this.mAttachmentPointFilter.getCurrent();
			}

			this.mCurrentAttachmentList.addRow([
				attachment,
				attachpoint
			]);
			this._updateAssembler();
		}
		else
		{
			this.log.warn("Attachable not found or not loaded yet: " + attachment);
		}
	}

	function onEditAttachment( sender )
	{
	}

	function onRemoveAttachment( sender )
	{
		local sri = this.mCurrentAttachmentList.getSelectedRows();

		if (sri.len() == 1)
		{
			sri = sri[0];
			this.mCurrentAttachmentList.removeRow(sri);
			this.mCurrentAttachmentList.setSelectedRows(null);

			if (sri in this.mAttachmentColors)
			{
				delete this.mAttachmentColors[sri];
			}

			local newColors = {};

			foreach( i, x in this.mAttachmentColors )
			{
				if (i < sri)
				{
					newColors[i] <- x;
				}
				else
				{
					newColors[i - 1] <- x;
				}
			}

			this.mAttachmentColors = newColors;

			if (sri in this.mAttachmentEffects)
			{
				delete this.mAttachmentEffects[sri];
			}

			local newEffects = {};

			foreach( i, x in this.mAttachmentEffects )
			{
				if (i < sri)
				{
					newEffects[i] <- x;
				}
				else
				{
					newEffects[i - 1] <- x;
				}
			}

			this.mAttachmentEffects = newEffects;
			this._updateAssembler();
		}
	}

	function onCopyAttachment( sender )
	{
		local r = this.mCurrentAttachmentList.getSelectedRows();

		if (r.len() != 1)
		{
			return;
		}

		local index = r[0];
		local a = {
			type = this.mCurrentAttachmentList.getRow(index)[0]
		};

		if (this.mAttachmentPointCheckBox.getChecked())
		{
			local result = this.mAttachmentRollout.getCurrent();

			if ("attachPoint" in result)
			{
				a.point_override <- result.attachPoint;
			}
		}

		if (index in this.mAttachmentEffects)
		{
			a.effect <- this.mAttachmentEffects[index];
		}

		if (index in this.mAttachmentColors)
		{
			a.colors <- this.mAttachmentColors[index];
		}

		this.System.setClipboard(this.serialize({
			a = a
		}));
	}

	function onPasteAttachment( sender )
	{
		local r = this.mCurrentAttachmentList.getSelectedRows();

		if (r.len() != 1)
		{
			return;
		}

		local index = r[0];

		try
		{
			local row = this.mCurrentAttachmentList.getRow(index);
			local itemAppearance = this.unserialize(this.System.getClipboard());

			if (typeof itemAppearance == "table" && "a" in itemAppearance)
			{
				local a = itemAppearance.a;
				this.mCurrentAttachmentList.removeRow(index);
				row[0] = a.type;
				this.mCurrentAttachmentList.insertRow(index, row);

				if ("effect" in a)
				{
					this.mAttachmentEffects[index] <- a.effect;
				}
				else if (index in this.mAttachmentEffects)
				{
					delete this.mAttachmentEffects[index];
				}

				if ("colors" in a)
				{
					this.mAttachmentColors[index] <- a.colors;
				}
				else if (index in this.mAttachmentColors)
				{
					delete this.mAttachmentColors[index];
				}

				this._updateAssembler();
			}
		}
		catch( err )
		{
			this.log.error("Error pasting item appearance: " + err);
		}
	}

	function _bodyTypeDesc()
	{
		if (this.mBodyType != "-Biped-")
		{
			return this.mBodyType;
		}

		return this.mGenderList.getCurrent() + " " + this.mRaceList.getCurrent();
	}

	function _selectBodyType( type )
	{
		for( local i = 0; i < this.mBodyTypeList.getRowCount(); i++ )
		{
			local row = this.mBodyTypeList.getRow(i);

			if (row[0] == type)
			{
				this.mBodyType = type;
				this.mBodyTypeList.setSelectedRows([
					i
				]);
				this.mBodyTypeList._displayRow(i);
				return true;
			}
		}

		return false;
	}

	function onBodyTypeReady()
	{
		this._fillDetailsAndColors();
		this._updateAnimationList();
		this.setSize(500, 350);
	}

	function _getSelectedCharacter()
	{
		local tool = ::_tools.getActiveTool();
		local object = this._avatar;

		if (tool != null && tool.tostring() == "SceneryTool")
		{
			local selection = tool._selectedObjects();

			foreach( selected in selection )
			{
				if (selected.isCreature())
				{
					object = selected;
					break;
				}
			}
		}
		else
		{
			local selection = this._avatar.getTargetObject();

			if (selection != null && selection.isCreature())
			{
				object = selection;
			}
		}

		return object;
	}

	function _updateAnimationList()
	{
		local selection = this.mAnimDropDown.getCurrent();
		this.mAnimDropDown.removeAll();
		local object = this._getSelectedCharacter();

		if (object)
		{
			local entity = object.getEntity();

			if (entity)
			{
				foreach( i in entity.getAnimationStates() )
				{
					local name;
					name = i;

					foreach( p in [
						"_b",
						"_h",
						"_t"
					] )
					{
						local pos = i.find(p);

						if (pos && pos == i.len() - 2)
						{
							name = i.slice(0, pos);
							break;
						}
					}

					local found = false;

					foreach( n in this.mAnimDropDown.mChoices )
					{
						if (n == name)
						{
							found = true;
							break;
						}
					}

					if (found == false)
					{
						this.mAnimDropDown.addChoice(name);
					}
				}
			}
		}

		this.mAnimDropDown.setCurrent(selection);
	}

	function _restoreAnimationState( RestoreObject )
	{
		if (this.mAnimStateToRestore && this.mTargetToRestoreAnimState && this.mCreatureTypeToRestoreAnimState == RestoreObject.mAssembler.mBody && this.mTargetToRestoreAnimState.getAnimationHandler())
		{
			this.mTargetToRestoreAnimState.getAnimationHandler().setAnimationState(this.mAnimStateToRestore);
			this.mTargetToRestoreAnimState = null;
			this.mCreatureTypeToRestoreAnimState = null;
			this.mAnimStateToRestore = null;
		}
	}

	function onRunAnimation( button )
	{
		local speed = this.mAnimSpeed.getText().tofloat();
		local loop = this.mAnimLoopCheckbox.getChecked();
		local object = this._getSelectedCharacter();
		local anim_handler = object.getAnimationHandler();

		if (anim_handler != 0)
		{
			anim_handler.onFF(this.mAnimDropDown.getCurrent(), speed, loop);
		}
	}

	function onLoopChanged( button, val )
	{
		local loop = this.mAnimLoopCheckbox.getChecked();

		if (loop == false)
		{
			local object = this._getSelectedCharacter();
			local anim_handler = object.getAnimationHandler();
			anim_handler.fullStop();
		}
	}

	function onAttachmentChange( sender )
	{
		local results = this.mAttachmentRollout.getCurrent();
		local sri = this.mCurrentAttachmentList.getSelectedRows();

		if (sri.len() == 1)
		{
			sri = sri[0];
			this.mCurrentAttachmentList.setRow(sri, [
				results.type,
				results.attachPoint
			]);

			if (("effect" in results) && results.effect != null)
			{
				this.log.debug("Effect found in attachment change results");
				this.mAttachmentEffects[sri] <- results.effect;
			}
			else if (sri in this.mAttachmentEffects)
			{
				delete this.mAttachmentEffects[sri];
			}

			if ("colors" in results)
			{
				this.log.debug("Colors found in attachment change results");
				this.mAttachmentColors[sri] <- results.colors;
			}
			else if (sri in this.mAttachmentColors)
			{
				delete this.mAttachmentColors[sri];
			}

			this._updateAssembler();
		}
	}

	function _closeRollout()
	{
	}

	function _matchInstanceToSlot( group, instance )
	{
		foreach( i, x in group )
		{
			if (x == instance)
			{
				return i;
			}
		}

		return null;
	}

	mClothingArchiveWaiter = null;
	function _loadClothingArchives( required )
	{
		if (this.mClothingArchiveWaiter != null)
		{
			return;
		}

		local archives = this.GetAssetArchive(this.Util.tableKeys(::ClothingIndex));
		local callback = {
			tweak = this,
			function onWaitComplete( info )
			{
				this.tweak._fillDetailsAndColors(false);
			}

		};
		this.mClothingArchiveWaiter = this.Util.waitForAssets(archives, callback, required ? this.ContentLoader.PRIORITY_REQUIRED : this.ContentLoader.PRIORITY_LOW);
	}

	function statUpdate( statId, value )
	{
		switch(statId)
		{
		case this.Stat.DISPLAY_NAME:
			this.mNameInput.setText(value);
			break;

		case this.Stat.APPEARANCE:
			this.setAppearanceConfig(value);
			break;

		default:
			if (statId in this.mStatEditBoxes)
			{
				this.log.debug("Tweak stat update " + this.Stat[statId].name + " <- " + value);
				this.mStatEditBoxes[statId].setText("" + value);
			}
		}
	}

	function setVisible( value )
	{
		if (value && !this.isVisible())
		{
			if (!this.mScreenInitialized)
			{
				this._loadRaceArchives(true);
				this._loadClothingArchives(true);
				this.mScreenInitialized = true;

				if (this._avatar)
				{
					this._avatar.addListener(this);
				}
			}
		}

		this.GUI.Frame.setVisible(value);
	}

	function onCreatureUpdated( sender, object )
	{
	}

	function onClosePressed()
	{
		::Screens.close("CreatureTweakScreen");
	}

	function onAvatarChanged( oldAvatar, avatar )
	{
		if (oldAvatar)
		{
			oldAvatar.removeListener(this);
		}

		avatar.addListener(this);
	}

	function updateScreenOnBodyChanged()
	{
		local selected = this.mTabs.getSelectedTab();

		if (this.mProp)
		{
			if (this.mPropInputBox == null)
			{
				this.mPropInputBox = this.GUI.AssetRefInputBox();
				this.mPropInputBox.addActionListener(this);
				this.mPropInputBox.setShowShortName(false);
			}

			this.mBipedBodyTypeList.setCurrent("-Prop-");
			this.mHeadPropLabel.setText("Prop name");
			this.mBodyOpts.remove(this.mBipedHeadList);
			this.mBodyOpts.remove(this.mGenderList);
			this.mBodyOpts.remove(this.mRaceList);
			this.mBodyOpts.remove(this.mGenderLabel);
			this.mBodyOpts.remove(this.mRaceLabel);
			this.mBodyTypeList.removeAll();
			this.mTabs.removeAll();
			this.mTabs.add(this.mBodyPage, "Body");
			this.mTabs.selectTab(selected.name);
			this.mBodyOpts.add(this.mPropInputBox);
		}
		else
		{
			if (this.mPropInputBox)
			{
				this.mPropInputBox.setAsset("");
			}

			this.mTabs.removeAll();
			this.mHeadPropLabel.setText("Head");
			this.mBodyOpts.remove(this.mBipedHeadList);
			this.mBodyOpts.remove(this.mPropInputBox);
			this.mBodyOpts.remove(this.mBodyTypeLabel);
			this.mBodyOpts.remove(this.mBipedBodyTypeList);
			this.mBodyOpts.add(this.mSizeLabel);
			this.mBodyOpts.add(this.mSizeInput);
			this.mBodyOpts.add(this.mGenderLabel);
			this.mBodyOpts.add(this.mGenderList);
			this.mBodyOpts.add(this.mBodyTypeLabel);
			this.mBodyOpts.add(this.mBipedBodyTypeList);
			this.mBodyOpts.add(this.mRaceLabel);
			this.mBodyOpts.add(this.mRaceList);
			this.mBodyOpts.add(this.mHeadPropLabel);
			this.mBodyOpts.add(this.mBipedHeadList);
			this.mTabs.add(this.mBodyPage, "Body");
			this.mTabs.add(this.mSkinPage, "Skin");
			this.mTabs.add(this.mDetailPage, "Details");
			this.mTabs.add(this.mClothingPage, "Clothing");
			this.mTabs.add(this.mAttachmentPage, "Attachments");
			this.mTabs.add(this.mInventoriesPage, "Inventories");
			this.mTabs.selectTab(selected.name);
			this.mBodyTypeList.refreshAll();
		}
	}

	function onTargetObjectChanged( creature, target )
	{
		if (this.mWaitingForVendor)
		{
			this.linkVendor(target);
			return;
		}

		if (target && target.mCreatureDef.getStat(this.Stat.APPEARANCE).find("p1") != null)
		{
			this.mProp = true;
		}
		else
		{
			this.mProp = false;
		}

		this.mDontUpdate = true;
		this.mConfigOpts = {};
		this.updateScreenOnBodyChanged();
		this.mDontUpdate = false;
		this.mCurrentCreature = target;

		if (target)
		{
			this.mDontUpdate = true;
			this.setCurrentType(target.getType());
			this.mDontUpdate = false;
		}
		else if (this._avatar)
		{
			this.mDontUpdate = true;
			this.setCurrentType(this._avatar.getType());
			this.mDontUpdate = false;
		}
		else
		{
			this.setVisible(false);
		}
	}

	function onPastePressed( button )
	{
		local result;

		try
		{
			result = this.setAppearanceConfig(this.System.getClipboard());

			if (result)
			{
				this._updateAssembler();
			}
		}
		catch( err )
		{
			result = false;
		}

		if (!result)
		{
			this.IGIS.error("The clipboard does not contain a valid appearance configuration.");
		}
	}

	function onCopyPressed( button )
	{
		this.System.setClipboard(this.getAppearanceConfig());
		this.log.info("Saved appearance to clipboard.");
	}

	function onClearConfirmed( sender, value )
	{
		if (value)
		{
			this._opHistory.execute(this.CreatureDefStatEditOp(this.mCurrentTypeId, this.Stat.APPEARANCE, ""));
		}
	}

	function onClearPressed( button )
	{
		local confirm = this.GUI.ConfirmationWindow();
		confirm.setConfirmationType(this.GUI.ConfirmationWindow.YES_NO);
		confirm.setEventName("onClearConfirmed");
		confirm.addActionListener(this);
		confirm.setText("Are you sure want to clear the appearance?");
	}

	function destroy()
	{
		::_ItemDataManager.removeListener(this);
		::_Connection.removeListener(this);

		if (::_avatar)
		{
			::_avatar.removeListener(this);
		}

		this.GUI.Frame.destroy();
	}

}

class this.GUI.CreatureTweakButton extends this.GUI.Button
{
	mCreatureType = null;
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor(vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor("Tweak");
		}
	}

	function getValue()
	{
		return this.mCreatureType;
	}

	function setValue( value )
	{
		this.mCreatureType = value;
	}

	function _fireActionPerformed( message )
	{
		if (message == "onActionPerformed")
		{
			local ct = this.Screens.show("CreatureTweakScreen");
			ct.setCurrentType(this.mCreatureType);
		}
		else
		{
			this.GUI.Button._fireActionPerformed(message);
		}
	}

	function onInputComplete( source )
	{
		this._fireActionPerformed("onInputComplete");
	}

}


if (oldCT)
{
	local ct = this.Screens.show("CreatureTweakScreen");
	ct.setCurrentType(oldCT);
}
