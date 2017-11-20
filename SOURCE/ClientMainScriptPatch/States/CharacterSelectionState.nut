require("States/StateManager");
require("Environment");
require("UI/UI");
require("GUI/SimpleList");
require("NameSuggestions");
AVATAR_SCALE <- 3.5;

class GUI.Logo extends GUI.Component {
	constructor() {
		local ICON_SIZE = 100;
		GUI.Component.constructor();
		
		local g = GUI.Component(null);
		g.setAppearance("EELogo/Glow");
		g.setSize(ICON_SIZE, ICON_SIZE);
		add(g);
		
		local l = GUI.Component(null);
		l.setAppearance("EELogo");
		l.setSize(ICON_SIZE, ICON_SIZE);
		g.add(l);
	}
}

class GUI.InfoPanel extends GUI.Panel {
	constructor() {
		GUI.Panel.constructor();
		setInsets(5, 5, 5, 5);
		setLayoutManager(this.GUI.BorderLayout());
		
		mHTML = this.GUI.HTML("This has not been set");
		mHTML.setAppearance("PaperBack");
		mHTML.setInsets(15, 15, 15, 15);
		mHTML.setFontColor("2b1b00");
		
		local scroll = GUI.ScrollPanel(mHTML);
		add(scroll, GUI.BorderLayout.CENTER);
	}

	function setText( text ) {
		mHTML.setText(text);
	}

	mHTML = null;
}

class GUI.DownloadPanel extends GUI.Panel {
}

class GUI.ArrowButton extends GUI.Button
{
	constructor( direction, ... )
	{
		if (vargc > 0)
			GUI.Button.constructor("", vargv[0]);
		else
			GUI.Button.constructor("");

		remove(mLabel);
		setAppearance("RotateButton");

		if (direction == this.CLOCKWISE)
			setMaterial(mAppearance + "/Clockwise");
		else if (direction == this.COUNTERCLOCKWISE)
			setMaterial(mAppearance + "/Counter-Clockwise");

		setFixedSize(63, 50);
		mUseOffsetEffect = true;
		mUseMouseOverEffect = false;
	}

	static CLOCKWISE = 0;
	static COUNTERCLOCKWISE = 1;
}

class GUI.CharacterSelectionListObject extends GUI.Component {
	constructor( name, level, race, profession ) {
		GUI.Component.constructor(GUI.BoxLayoutV());
		getLayoutManager().setExpand(true);
		setInsets(0, 20, -1, 20);
		
		mName = name;

		if (level == 0)
			level = 1;

		mLevel = level;
		mProfession = profession;
		_setRace(race);
		mNameLabel = GUI.Label("");
		mNameLabel.setInsets(5, 0, -10, 0);
		add(mNameLabel);
		mInfoHTML = GUI.HTML("");
		mInfoHTML.setInsets(0, 0, 8, 0);
		add(mInfoHTML);
		mSeperator = GUI.Spacer(50, 1);
		mSeperator.setAppearance("ColumnList/HeadingDivider");
		_buildDisplayString();
	}

	function _setRace( race ) {
		if (race) {
			foreach( i, x in ::Races ) {
				if (race == i || race == x)	{
					mRace = ::RaceData[i].prettyName;
					return;
				}
			}
		}
		else
			mRace = null;
	}

	function _buildDisplayString() {
		mNameLabel.setFont(::GUI.Font("Maiandra", 24, true));
		mNameLabel.setText(this.mName);
		mNameLabel.setFontColor("ffffff");
		mNameLabel.setAutoFit(true);
		local info = "<font color=\"fff77f\">Level: " + mLevel;

		if (mRace)
			info += "   " + mRace;

		info += "   " + mProfession + "</font>";
		mInfoHTML.setText(info);
	}

	function hideSeperator( bool ) {
		if (bool)
			mSeperator.setVisible(false);
		else
			mSeperator.setVisible(true);
	}

	mName = "";
	mLevel = 0;
	mRace = "";
	mProfession = "";
	
	mNameLabel = null;
	mInfoHTML = null;
	mSeperator = null;
}

class GUI.CharacterSelectionList extends GUI.ColumnList
{
	function onKeyPressed( evt ) {
		print("List Keypress called\n");
		switch(evt.keyCode) {
		case 46: // Delete key
			print("Delete Key Pressed");
			mMessageBroadcaster.broadcastMessage("onDeleteConfirm", this);
			evt.consume();
			break;
		}
	}

	function onRowSelect( row, evt ) {
		GUI._Manager.requestKeyboardFocus(this);
		GUI.ColumnList.onRowSelect(row, evt);
	}

	function onMousePressed( evt ) {
		GUI._Manager.requestKeyboardFocus(this);
		GUI.ColumnList.onMousePressed(evt);
	}

}

class CharacterCreationPersonaHandler extends PersonaCreateHandler
{
	mOwner = null;
	
	constructor( vOwner ) {
		this.mOwner = vOwner;
	}

	function onQueryComplete( qa, results )	{
		PersonaCreateHandler.onQueryComplete(qa, results);
	}

	function onQueryError( qa, error ) {
		mOwner.onNewPersonaError(error);
	}
}

CharacterClassName <- {
	KNIGHT = 0,
	ROGUE = 1,
	MAGE = 2,
	DRUID = 3
};

BodyStyleType <- {
	NORMAL = 1,
	MUSCULAR = 2,
	ROTUND = 3
};
FaceStyleType <- {
	NORMAL = 1,
	INTENSE = 2,
	OLD = 3
};

StageBits <- {
	CHARACTER_SELECT = 1,
	PICK_RACE = 2,
	CUSTOM_COLORS = 4,
	PICK_CLASS = 8,
	PICK_NAME = 16
};

class States.CharacterSelectionState extends State
{
	static MAX_CHARACTERS = 8;
	static mClassName = "CharacterSelectionState";
	
	mCard = null;
	
	mCharacterSelection = null;
	mCharacterList = null;
	mCharacterSelectionInfo = null;
	
	mNewAppearance = null;
	
	mDeleteIndex = null;
	
	mCreationReady = false;
	mProgressComponent = null;
	mBackToSelection = null;
	mStartCreation = null;
	mCreationStarted = false;
	mRaceGender = null;
	mRaceGenderInfo = null;
	mGenderRadio = null;
	mGenderDefault = null;
	mRaceRadio = null;
	mRaceDefault = null;
	mRaceIcons = null;
	mBodyRadio = null;
	mBodyDefault = null;
	mFaceRadio = null;
	mFaceDefault = null;
	
	mEarSlider = null;
	mTailSlider = null;
	
	mInitialClothing = null;
	
	mChestGroup = null;
	mLeggingsGroup = null;
	mBootsGroup = null;
	
	mSkin = null;
	mSkinInfo = null;
	mSizeInput = null;
	mSkinColors = null;
	mOtherDetails = null;
	
	mFinishButton = null;
	mPreviousButton = null;
	
	mClassPage = null;
	mNameInfo = null;
	mNameInput = null;
	
	mFirstNameInput = null;
	mLastNameInput = null;
	mFirstName = "";
	mLastName = "";
	
	mPersonas = null;
	
	mPaperDoll = null;
	mArrowLeft = null;
	mArrowRight = null;
	
	mScene = null;
	
	mConfirm = null;
	mConfirmType = null;
	mFirstNameSuggestions = null;
	mLastNameSuggestions = null;
	mClassDefaultImageButton = null;
	
	mCurrentSelection = 0;
	mMaxCharacters = 8;
	
	mUseBackground = true;
	mBackground = "Prop-CharSelect_BG";
	mProfesssion = null;
	
	mClassSummary = null;
	
	Clothingids = null;
	
	static LEFT_PANEL_WIDTH = 380;
	static DEFAULT_CHEST_CLOTHES = 0;
	static DEFAULT_LEGGING_CLOTHES = 0;
	static DEFAULT_BOOT_CLOTHES = 0;
	static DEFAULT_TITLE_BG = "CC_Title_BG";
	static APPEARANCE_NUM = 2;
	static EQUIPMENT_NUM = 3;
	
	BodyStyle = null;
	FaceStyle = null;
	CharacterClassType = null;
	
	// Size multiplier for what the starting size race/gender of the character is
	mDefaultSizeMultiplier = 1;
	
	constructor( results ) {
		CharacterClassType = {};
		BodyStyle = {};
		FaceStyle = {};
		
		//Information needed for each class when player selects their class	
		CharacterClassType = {
			[::CharacterClassName.KNIGHT] = {
				profession = "Knight",
				iconName = "KnightClass",
				clothes = "Warrior",
				selectedChest = null,
				selectedLeg = null,
				selectedBoot = null,
				summary = "<font color=\"fff77f\" size=\"28\">KNIGHT </font><br /><br /><font color=\"fff77f\" size=\"20\">The mighty Knight is the heavy hitter of melee combat, and often a walking armory. The only class capable of wearing platemail and of wielding the largest, two-handed weapons, a Knight can both dish it out up close and take it.<br /><br /> No other class is able to absorb damage like the Knight and no other class can stand toe-to-toe with the most powerful of foes and successfully trade blows.</font>",
				startEquip = {
					[0] = {
						slot = ItemEquipSlot.WEAPON_MAIN_HAND,
						node = "right_hand",
						type = "Item-1hSword-Basic5",
						colors = [
							"4F5C5C",
							"436A6A",
							"FFDD59"
						]
					},
					[1] = {
						slot = ItemEquipSlot.WEAPON_OFF_HAND,
						node = "left_hand",
						type = "Item-Shield-Basic3",
						colors = [
							"D49053",
							"6A6A6A",
							"AAAAAA"
						]
					}
				}
			},
			[::CharacterClassName.ROGUE] = {
				profession = "Rogue",
				iconName = "RogueClass",
				clothes = "Sneak",
				selectedChest = null,
				selectedLeg = null,
				selectedBoot = null,
				summary = "<font color=\"fff77f\" size=\"28\">ROGUE</font><br /><br /><font color=\"fff77f\" size=\"20\">The flash of a pair of blades out of the corner of your eye may be the last thing you see if a deadly Rogue is hunting you. Even a heavily-armored Knight swinging his massive two-handed swords cannot deal out destruction from up-close like a Rogue can. <br /><br />Unlike a Knight, however, a Rogue inflicts damage from the shadows, finding the right time to strike, perhaps with daggers, perhaps with claws, perhaps even with the ancient katar.</font>",
				startEquip = {
					[0] = {
						slot = ItemEquipSlot.WEAPON_MAIN_HAND,
						node = "right_hand",
						type = "Item-Dagger-Basic6",
						colors = [
							"5A6268",
							"87D4D4",
							"365555"
						]
					},
					[1] = {
						slot = ItemEquipSlot.WEAPON_OFF_HAND,
						node = "left_hand",
						type = "Item-Dagger-Basic1",
						colors = [
							"5A6268",
							"94653A",
							"949494"
						]
					}
				}
			},
			[::CharacterClassName.MAGE] = {
				profession = "Mage",
				iconName = "MageClass",
				clothes = "Mage",
				selectedChest = null,
				selectedLeg = null,
				selectedBoot = null,
				summary = "<font color=\"fff77f\" size=\"28\">MAGE</font><br /><br /><font color=\"fff77f\" size=\"20\">The mystical Mage is the master of spellborn combat. Though Mages can wear only the lightest of armors they more than compensate for this with their ability to call down the mightiest of magics on their enemies.<br /><br />Between their powerful wands and feared spellcasting abilities, Mages can deal out more destruction in a short amount of time than anyone else, and few wish to be on the receiving end of their most powerful sorcery.</font>",
				startEquip = {
					[0] = {
						slot = ItemEquipSlot.WEAPON_RANGED,
						node = "right_hand",
						type = "Item-Wand-Basic1",
						colors = [
							"555555"
						]
					}
				}
			},
			[::CharacterClassName.DRUID] = {
				profession = "Druid",
				iconName = "DruidClass",
				clothes = "Druid",
				selectedChest = null,
				selectedLeg = null,
				selectedBoot = null,
				summary = "<font color=\"fff77f\" size=\"28\">DRUID</font><br /><br /><font color=\"fff77f\" size=\"20\">The staff-wielding, bow-using, pet-summoning Druid has perhaps the broadest range of capabilities of any class. The Druid can both wear the second heaviest type of armor-chainmail and fight capably and fearsomely with their staves and spears as well as attack from range with bows.<br /><br />The Druid is also a master of life and death magic, able to summon a range of creatures to aid in combat as well as employ spells both defensive and offensive.</font>",
				startEquip = {
					[0] = {
						slot = ItemEquipSlot.WEAPON_RANGED,
						node = "right_hand",
						type = "Item-Bow-Basic5",
						colors = [
							"7F420B",
							"BF3C21",
							"00D40D"
						]
					}
				}
			}
		};
		BodyStyle = {
			[::BodyStyleType.NORMAL] = {
				iconName = "BodyType-Normal",
				data = "n",
				tooltip = "creation.tooltip.bodytype.normal"
			},
			[::BodyStyleType.MUSCULAR] = {
				iconName = "BodyType-Muscular",
				data = "m",
				tooltip = "creation.tooltip.bodytype.muscular"
			},
			[::BodyStyleType.ROTUND] = {
				iconName = "BodyType-Rotund",
				data = "r",
				tooltip = "creation.tooltip.bodytype.rotund"
			}
		};
		FaceStyle = {
			[::FaceStyleType.NORMAL] = {
				iconName = "FaceStyle-Normal",
				data = 0,
				tooltip = "creation.tooltip.facestyle.normal"
			},
			[::FaceStyleType.INTENSE] = {
				iconName = "FaceStyle-Intense",
				data = 1,
				tooltip = "creation.tooltip.facestyle.intense"
			},
			[::FaceStyleType.OLD] = {
				iconName = "FaceStyle-Old",
				data = 2,
				tooltip = "creation.tooltip.facestyle.old"
			}
		};
		
		mPersonas = results;
		mProfesssion = ::CharacterClassName.KNIGHT;
		Clothingids = [];

		foreach( key, value in CharacterClassType )
			Clothingids.append(value.clothes);

		Clothingids.append("Clothing1");
	}

	function onPackageLoaded( Name ) {
	}

	function _showBackground( name ) {
		local sc = mScene = SceneObject(name, "Scenery");
		sc.setPosition(Vector3(12.0, 0.0, -2.0));
		sc.setFlags(SceneObject.PRIMARY);
		sc.setScale(0.64999998);
		sc.setType(name);
		sc.reassemble();
	}

	function _adjustCamera()
	{
		local height = Screen.getHeight();
		local z = height / 600.0 * 25.0;
		
		// Setup the camera settings we want to preview the character mesh
		
		::_Camera.setPosition(Vector3(8.5, 6.8000002, z + 13.5));
		::_Camera.setOrientation(Quaternion());
		
		local camera = ::_scene.getCamera("Default");
		local farClippingDistance = gCamera.farClippingDistance;
		camera.setFarClipDistance(gCamera.farClippingDistance);
		camera.setFOVy(33.5 * Math.PI / 180);
		
		// Reset visibility mast
		::_scene.setVisibilityMask(VisibilityFlags.DEFAULT);
	}

	function onKeyPressed( evt )
	{
		if (evt.keyCode == 46) {
			if (mConfirm == null)
				onDeleteConfirm();
		}

		if (evt == mFirstNameInput)	{
			mFirstNameSuggestions.setCurrent(null);
			mFirstName = mFirstNameInput.getText();
		}

		if (evt == this.mLastNameInput)	{
			mLastNameSuggestions.setCurrent(null);
			mLastName = mLastNameInput.getText();
		}
	}

	function onEnter() {
		mRaceIcons = [];

		if (mUseBackground)
			this._showBackground(this.mBackground);

		_createCharacter();

		// Listen for load screen show requests		
		::LoadScreen.setListener(this);
		
		// Card Layout
		
		local c = mCard = GUI.Component(GUI.CardLayout());
		c.setSticky("center", "center");
		c.setSize(800, 600);
		c.setPosition(-400, -300);
		c.setInsets(10, 10, 0, 10);
		
		_createCharacterSelection(this.mPersonas);
		
		mNewAppearance = {};

		if (this.mPersonas.len() != 0) {
			::GUI._Manager.requestKeyboardFocus(mCharacterList);
			c.getLayoutManager().show("CharacterSelection", c);
		}
		else
			onStartCreation();

		c.setOverlay("GUI/Overlay");
		
		// Setup the sky
		
		::_Environment.setOverride("CloudyDay");
		::_Environment.setTimeOfDay("Daytime");
		this.Audio.playMusic("Music-Theme.ogg", Audio.DEFAULT_CHANNEL);
		
		// Select the last picked character
		
		local lastIndex = ::Pref.get("login.LastCharacter");

		if (lastIndex < this.mPersonas.len())
			this.mCharacterList.setSelectedRows([
				lastIndex
			]);
		else
			::Pref.set("login.LastCharacter", null);
			
		// Enter game immediate if we want a quick login
		
		if (("gQuickLogin" in getroottable()) && ::gQuickLogin == true && mPersonas.len() > 0)
			onEnterGame(null);
	}

	function _createCharacter() 	{
		_adjustCamera();
		
		local so = mPaperDoll = SceneObject("PaperDoll", "Creature");
		so.setFadeEnabled(false);
		so.setPosition(this.Vector3(17, 0.0, 0.0));
		so.setHeading(-0.89999998);
		so.setOrientation(-0.89999998);
		so.setForceShowWeapon(true);
		so.setTimeoutEnabled(false);
		
		local a = GetAssembler("Creature", "Avatar");
		so.setType("Avatar");
		so.setShowName(false);
		so.setController("AvatarCreation");
		
		_configureChar("c2:{[\"r\"]=\"a\",[\"g\"]=\"m\"}");
		
		local leftArrowButton = mArrowLeft = GUI.ArrowButton(GUI.ArrowButton.CLOCKWISE);
		leftArrowButton.setSticky("center", "center");
		leftArrowButton.setPosition(50, 160);
		leftArrowButton.setPressMessage("onAvatarRightStart");
		leftArrowButton.setReleaseMessage("onAvatarRotateStop");
		leftArrowButton.addActionListener(so.getController());
		leftArrowButton.setOverlay("GUI/Overlay2");
		
		local rightArrowButton = mArrowRight = GUI.ArrowButton(GUI.ArrowButton.COUNTERCLOCKWISE);
		rightArrowButton.setSticky("center", "center");
		rightArrowButton.setPosition(310, 160);
		rightArrowButton.setPressMessage("onAvatarLeftStart");
		rightArrowButton.setReleaseMessage("onAvatarRotateStop");
		rightArrowButton.addActionListener(so.getController());
		rightArrowButton.setOverlay("GUI/Overlay2");
	}

	function _configureChar( apperance, ... ) {
		if (apperance == null) {
			mPaperDoll.disassemble();
			return;
		}

		log.info("Reconfigure paperdoll to " + apperance);
		local a = mPaperDoll.getAssembler();

		if (a != null) {
			a.configure(apperance);
			a.setEquipmentAppearance(vargc > 0 ? vargv[0] : "");
		}

		mPaperDoll.reassemble();
	}

	/**
		Creates the bottom component of the character creation screens.
		The bottom component holds the back, next button, and the filler update bar on the bottom
		
	*/
	function _createBottomPreviousNextComp( leftButtonText, leftButtonCallback, rightButtonText, rightButtonCallback, lastStep, ... ) {
		local hasFillBar = false;
		local fillAmount = 0;

		if (vargc > 1) {
			hasFillBar = vargv[0];
			fillAmount = vargv[1];
		}

		//Holds the bottom component, back button finished amount and next button
		local bottomComp = GUI.Component(GUI.GridLayout(1, 3));
		bottomComp.getLayoutManager().setColumns(200, 350, 200);
		bottomComp.getLayoutManager().setRows("*");
		bottomComp.getLayoutManager().setGaps(5, 0);
		
		local backButton = mBackToSelection = GUI.BigButton(leftButtonText);
		backButton.addActionListener(this);
		backButton.setReleaseMessage(leftButtonCallback);
		bottomComp.add(backButton);
		local fillerComp;

		if (hasFillBar)	{
			fillerComp = GUI.CCProgressBar();
			fillerComp.fillProgressBar(fillAmount);
		}
		else
			fillerComp = GUI.Component();

		bottomComp.add(fillerComp);
		
		local nextButton = GUI.BigButton(rightButtonText);
		nextButton.addActionListener(this);
		nextButton.setReleaseMessage(rightButtonCallback);
		bottomComp.add(nextButton);

		if (lastStep == true) {
			this.mFinishButton = nextButton;
			this.mPreviousButton = backButton;
		}

		return bottomComp;
	}

	/** Create generic character creation panel used in multiple character creation screens */
	function _createCharacterCreationPanel() {
		local outerPanel = GUI.Component();
		outerPanel.setSize(425, 60);
		outerPanel.setPreferredSize(425, 60);
		
		local characterCreatePanel = GUI.Panel(GUI.BoxLayoutV());
		characterCreatePanel.setAppearance(DEFAULT_TITLE_BG);
		characterCreatePanel.setSize(425, 60);
		characterCreatePanel.setPreferredSize(425, 60);
		characterCreatePanel.setPosition(-24, 0);
		characterCreatePanel.setInsets(5, 0, 0, 0);
		characterCreatePanel.getLayoutManager().setAlignment(0.5);
		outerPanel.add(characterCreatePanel);
		
		local characterTitle = GUI.HTML("<font color=\"fff77f\" size=\"56\">Character Creation</font>");
		characterCreatePanel.add(characterTitle);
		return outerPanel;
	}

	/** First panel after you login to either enter to play or create a new character */
	function _createCharacterSelection( results ) {
		if (::_Connection.getProtocolVersionId() > 27)
			::_Connection.sendQuery("account.tracking", this, StageBits.CHARACTER_SELECT);

		//Fullscreen ui for character selection
		local characterSelectComp = mCharacterSelection = GUI.Component(GUI.BoxLayoutV());
		mCard.add(characterSelectComp, "CharacterSelection");
		
		//Top Comp of the character selection screen everything except for the buttons on the bottom
		
		local topComp = GUI.Component(GUI.GridLayout(1, 2));
		topComp.getLayoutManager().setRows(515);
		topComp.getLayoutManager().setColumns(LEFT_PANEL_WIDTH, "*");
		topComp.setInsets(5, 0, 15, 30);
		characterSelectComp.add(topComp);
		
		// False to set the close button invisible
		local bigFrame = GUI.BigFrame("Character Selection", false);
		bigFrame.setVisible(true);
		bigFrame.setMovable(false);
		
		local selectCharacterPanel = GUI.Panel(GUI.GridLayout(2, 1));
		selectCharacterPanel.getLayoutManager().setRows("*", 42);
		selectCharacterPanel.getLayoutManager().setColumns(359);
		selectCharacterPanel.setInsets(10, 10, 5, 10);
		bigFrame.setContentPane(selectCharacterPanel);
		
		local listComp = GUI.InnerPanel(GUI.GridLayout(1, 1));
		listComp.getLayoutManager().setRows(370);
		listComp.getLayoutManager().setColumns("*");
		listComp.setInsets(10, 10, 10, 10);
		
		//selectCharacterPanel.add(listComp);
		
		selectCharacterPanel.add(listComp);
		
		
		// 
		//local scroller = GUI.ScrollPanel();
		//scroller.attach(list);
		//scroller.setPreferredSize(336, 370);
		//listComp.setPreferredSize(336, 370);
		//listComp.add(scroller);
		
		local list = mCharacterList = GUI.CharacterSelectionList();
		list.setAppearance(null);
		list.setShowingHeaders(false);
		list.setRowAppearance("ColumnSelection");
		list.addColumn("", 1.0);

		foreach( i, x in results )
			displayPersona(x);



		listComp.add(list);
		topComp.add(bigFrame);
		
		local createDeleteComp = GUI.Component(this.GUI.BoxLayout());
		createDeleteComp.getLayoutManager().setGap(10);
		selectCharacterPanel.add(createDeleteComp);
		
		local createCharacterButton = mStartCreation = GUI.NarrowButton("Create Character");
		createCharacterButton.setFixedSize(174, 32);

		if (results.len() == mMaxCharacters)
			createCharacterButton.setEnabled(false);
		else
			createCharacterButton.setEnabled(true);

		createCharacterButton.addActionListener(this);
		createCharacterButton.setReleaseMessage("onStartCreation");
		createDeleteComp.add(createCharacterButton);
		
		local deleteCharacterButton = GUI.NarrowButton("Delete Character");
		deleteCharacterButton.setFixedSize(174, 32);
		deleteCharacterButton.addActionListener(this);
		deleteCharacterButton.setReleaseMessage("onDeleteCharacter");
		createDeleteComp.add(deleteCharacterButton);
		
		// Create the progress animation component
		
		mProgressComponent = GUI.Component(GUI.BoxLayout());
		mProgressComponent.getLayoutManager().setAlignment(0.5);
		mProgressComponent.getLayoutManager().setPackAlignment(0.5);
		mProgressComponent.add(GUI.ProgressAnimation());
		mProgressComponent.add(GUI.Spacer(10, 0));
		
		local loadingLabel = GUI.Label("Loading...");
		loadingLabel.setFont(::GUI.Font("MaiandraOutline", 24));
		mProgressComponent.add(loadingLabel);
		
		topComp.add(mProgressComponent);
		
		// Create the logo component
		
		topComp.add(_createRightLogoComp());
		
		//Holds the logout and Play! button
		
		local bottomComp = _createBottomPreviousNextComp("Logout", "onLoggedOut", "Play!", "onEnterGame", false);
		characterSelectComp.add(bottomComp);
		list.setOneSelectMin(true);
		this.mCurrentSelection = 0;
		list.addActionListener(this);

		if (results.len() != 0)
			_configureChar(results[0][this.APPEARANCE_NUM], results[0][this.EQUIPMENT_NUM]);
	}

	function setVisible( which )
	{
		mCharacterSelection.setVisible(which);
	}

	function getOverlay() {
		return mCharacterSelection.getOverlay();
	}

	function setOverlay( overlay ) {
		mCharacterSelection.setOverlay(overlay);
	}

	function isVisible() {
		return mCharacterSelection.isVisible();
	}

	function onLoggedOut( sender ) {
		// Close the connection.
		::_Connection.close();
	}

	function event_Disconnect( args ) {
		::gQuickLogin <- false;
		
		// Return to login state
		
		this.States.set(this.States.LoginState());
	}

	function _getCreationArchives()	{
		local archives = [];

		foreach( i, x in ::Races ) {
			foreach( j, y in ::Genders ) {
				local filename = "Biped-" + x + "_" + y;
				archives.append(filename);
			}
		}

		return archives;
	}

	function _isRaceHidden( race ) {
		local def = ::ContentDef["Biped-" + ::Races[race] + "_Male"];

		if ("Restricted" in def)
			if (typeof def.Restricted == "bool")
				return def.Restricted;

		return false;
	}

	function _isRaceAvailable( race ) {
		local def = ::ContentDef["Biped-" + ::Races[race] + "_Male"];

		if ("Restricted" in def) {
			if (typeof def.Restricted == "string") {
				//design desicions must be made on how to interpret
				//the restriction string for a particular group of users
				return false;
			}
		}

		return true;
	}

	/** Creates the right side component of the character creation screens, that holds the EE logo 
		Extra parm is how much you want to offset the position of the logo of the x to be minus the width
		of the component
	*/
	function _createRightLogoComp( ... ) {
		local width = 290;

		if (vargc > 0)
			width = width - vargv[0];

		local rightComp = GUI.Container(null);
		rightComp.setSize(width, 515);
		rightComp.setPreferredSize(width, 515);
		
		local size = rightComp.getSize();
		
		local logo = GUI.Logo();
		logo.setPosition(width - logo.getSize().width - 20, -20);
		rightComp.add(logo);
		return rightComp;
	}

	/** Creates the race and gender screen */
	function _createRaceGender()
	{
		local GENDER_ICON_SIZE = 28;
		
		//Full screen invisible component that centers the UI
		local genderRaceComp = mRaceGender = GUI.Component(GUI.BoxLayoutV());
		mCard.add(genderRaceComp, "RaceGender");
		
		local topGenderRaceComp = GUI.Component(GUI.GridLayout(1, 2));
		topGenderRaceComp.getLayoutManager().setRows(525);
		topGenderRaceComp.getLayoutManager().setColumns(LEFT_PANEL_WIDTH, "*");
		topGenderRaceComp.setInsets(-5, 0, 15, 30);
		genderRaceComp.add(topGenderRaceComp);
		
		local leftComp = GUI.Component(GUI.BoxLayoutV());
		leftComp.getLayoutManager().setExpand(true);
		leftComp.getLayoutManager().setGap(10);
		topGenderRaceComp.add(leftComp);
		
		leftComp.add(_createCharacterCreationPanel());
		
		local genderPanel = GUI.Panel(GUI.BoxLayout());
		genderPanel.setInsets(10, 20, 10, 20);
		genderPanel.getLayoutManager().setGap(25);
		leftComp.add(genderPanel);
		
		local genderTitle = GUI.HTML("<font color=\"e4e4e4\" size=\"36\">Gender</font>");
		genderPanel.add(genderTitle);
		mGenderRadio = GUI.RadioGroup();
		mGenderRadio.addListener(this);
		
		local maleComp = GUI.Component(GUI.BoxLayout());
		maleComp.setInsets(0, 0, 0, 10);
		maleComp.getLayoutManager().setGap(10);
		genderPanel.add(maleComp);
		
		local maleTitle = GUI.HTML("<font color=\"fff77f\" size=\"28\">Male</font>");
		maleComp.add(maleTitle);
		
		local maleIcon = mGenderDefault = GUI.Icon();
		maleIcon.setRadioGroup(mGenderRadio);
		maleIcon.setIconImage("Male");
		maleIcon.setData({
			g = "m"
		});
		maleIcon.setTooltip(::TXT("creation.tooltip.gender.male"));
		maleIcon.setSelectedBackground("StandardCyan");
		maleIcon.setIconSize(GENDER_ICON_SIZE, GENDER_ICON_SIZE);
		maleComp.add(maleIcon);
		
		local femaleComp = GUI.Component(GUI.BoxLayout());
		femaleComp.getLayoutManager().setGap(10);
		genderPanel.add(femaleComp);
		local femaleTitle = this.GUI.HTML("<font color=\"fff77f\" size=\"28\">Female</font>");
		femaleComp.add(femaleTitle);
		
		local femaleIcon = GUI.Icon();
		femaleIcon.setRadioGroup(mGenderRadio);
		femaleIcon.setIconImage("Female");
		femaleIcon.setData({
			g = "f"
		});
		femaleIcon.setTooltip(::TXT("creation.tooltip.gender.female"));
		femaleIcon.setSelectedBackground("Pink");
		femaleIcon.setIconSize(GENDER_ICON_SIZE, GENDER_ICON_SIZE);
		femaleComp.add(femaleIcon);
		
		local raceBodyComp = GUI.Component(GUI.BoxLayoutV());
		raceBodyComp.getLayoutManager().setGap(-3);
		leftComp.add(raceBodyComp);
		
		local raceFrame = GUI.BigFrame("Player Race", false);
		raceFrame.setVisible(true);
		raceFrame.setMovable(false);
		raceFrame.setTitleFont(this.GUI.Font("Maiandra", 42));
		raceBodyComp.add(raceFrame);
		
		local rcc = GUI.Component(GUI.BoxLayoutV());
		rcc.getLayoutManager().setAlignment(0.5);
		rcc.setInsets(10, 39, 10, 39);
		raceFrame.setContentPane(rcc);
		
		local rc = GUI.Component(GUI.GridLayout(4, 6));
		rc.getLayoutManager().setGaps(10, 10);
		rcc.add(rc);
		
		local rrg = mRaceRadio = GUI.RadioGroup();
		rrg.addListener(this);
		local first = true;

		foreach( i, x in ::RaceOrder ) {
		
			if (_isRaceHidden(x))
				continue;

			local race = GUI.Icon();

			if (first) {
				mRaceDefault = race;
				first = false;
			}

			mRaceIcons.append(race);

			if (!_isRaceAvailable(x))
				race.setEnabled(false);

			//This is here to move the bottom row to the center
			if (Races[x] == "Troblin") {
				local spacer = GUI.Spacer();
				spacer.setSize(42, 42);
				rc.add(spacer);
			}

			race.setIconImage("Biped-" + ::Races[x] + "_Male");
			race.setRadioGroup(rrg);
			race.setData({
				r = x
			});
			race.setSelectedBackground("StandardBlue");
			race.setTooltip(::TXT("creation.tooltip.race." + ::Races[x].tolower()));
			rc.add(race);
		}

		//This is a place holder for the eventual interface that will allow you to select a
		
		local bodyFacePanel = GUI.Panel(GUI.BoxLayout());
		bodyFacePanel.getLayoutManager().setGap(40);
		bodyFacePanel.setInsets(10, 25, 10, 25);
		raceBodyComp.add(bodyFacePanel);
		
		local bodyComp = GUI.Component(GUI.BoxLayoutV());
		bodyComp.getLayoutManager().setGap(5);
		bodyFacePanel.add(bodyComp);
		
		local bodyGridComp = GUI.Component(GUI.GridLayout(1, 3));
		bodyGridComp.getLayoutManager().setGaps(10, 10);
		bodyComp.add(bodyGridComp);
		mBodyRadio = GUI.RadioGroup();
		mBodyRadio.addListener(this);

		//Adds the body types icons into the component normal, musclar, round
		
		foreach( bodyName, bodyData in BodyStyle ) {
			local body = GUI.Icon();
			body.setRadioGroup(this.mBodyRadio);
			body.setIconImage(bodyData.iconName);
			body.setData({
				b = bodyData.data
			});
			body.setTooltip(::TXT(bodyData.tooltip));
			bodyGridComp.add(body);

			if (bodyName == 1)
				mBodyDefault = body;
		}

		local bodyTitle = GUI.HTML("<font color=\"e4e4e4\" size=\"32\">Body Type</font>");
		bodyTitle.getLayoutManager().setAlignment("center");
		bodyComp.add(bodyTitle);
		
		local faceComp = GUI.Component(GUI.BoxLayoutV());
		faceComp.getLayoutManager().setGap(5);
		bodyFacePanel.add(faceComp);
		
		local faceGridComp = GUI.Component(GUI.GridLayout(1, 3));
		faceGridComp.getLayoutManager().setGaps(10, 0);
		faceComp.add(faceGridComp);
		
		mFaceRadio = GUI.RadioGroup();
		mFaceRadio.addListener(this);

		foreach( faceType, faceData in FaceStyle )	{
			local face = GUI.Icon();
			face.setRadioGroup(mFaceRadio);
			face.setIconImage(faceData.iconName);
			face.setData({
				h = faceData.data
			});
			face.setTooltip(::TXT(faceData.tooltip));
			faceGridComp.add(face);

			if (faceType == 1)
				mFaceDefault = face;
		}

		local faceTitle = GUI.HTML("<font color=\"e4e4e4\" size=\"32\">Face</font>");
		faceTitle.getLayoutManager().setAlignment("center");
		faceComp.add(faceTitle);
	
		//Creates generic right component with logo
		topGenderRaceComp.add(_createRightLogoComp());
		
		//Holds the bottom component, back button finished amount and next button
		local fillAmount = 1;
		local bottomComp = _createBottomPreviousNextComp("< Back  ", "onStartSelection", "  Next >", "onSkin", false, true, fillAmount);
		genderRaceComp.add(bottomComp);
		_createSkin();
		onStartCreation();
	}

	/** Creates the skin screen for character creation */
	function _createSkin()
	{
		local skinComp = mSkin = GUI.Component(GUI.BoxLayoutV());
		mCard.add(skinComp, "Skin");
		
		local topSkinComp = GUI.Component(GUI.GridLayout(1, 2));
		topSkinComp.getLayoutManager().setRows(525);
		topSkinComp.getLayoutManager().setColumns(this.LEFT_PANEL_WIDTH, "*");
		//topSkinComp.setInsets(-5, 0, 15, 30);
		skinComp.add(topSkinComp);
		
		local leftComp = GUI.Component(GUI.BoxLayoutV());
		leftComp.getLayoutManager().setExpand(true);
		topSkinComp.add(leftComp);
		
		leftComp.add(_createCharacterCreationPanel());
		local colorBigFrame = GUI.BigFrame("Detail", false);
		colorBigFrame.setVisible(true);
		colorBigFrame.setMovable(false);
		colorBigFrame.setTitleFont(this.GUI.Font("Maiandra", 42));
		leftComp.add(colorBigFrame);
		
		local detailPanel = GUI.Panel(GUI.BoxLayoutV());
		colorBigFrame.setContentPane(detailPanel);
		
		local topDetailPanel = GUI.Component(GUI.BoxLayout());
		//topDetailPanel.getLayoutManager().setGap(10);
		
		local otherComp = mOtherDetails = GUI.InnerPanel(GUI.FlowLayout());
		otherComp.setInsets(12,0,0,0);
		otherComp.getLayoutManager().setAlignment("center");
		otherComp.setPreferredSize(300, 48);
		
		local mainDetailPanel = GUI.Panel(GUI.BorderLayout());
		mainDetailPanel.add(topDetailPanel, GUI.BorderLayout.CENTER);
		mainDetailPanel.add(otherComp, GUI.BorderLayout.SOUTH);
		detailPanel.add(mainDetailPanel);
		
		local colorpanel = GUI.InnerPanel(GUI.GridLayout(1, 1));
		colorpanel.setInsets(5, 0, 5, 20);
		colorpanel.setPreferredSize(250, 350);
		colorpanel.getLayoutManager().setRows("*");
		colorpanel.getLayoutManager().setColumns(250);		
		topDetailPanel.add(colorpanel);
		
		// Ugh.  A GUI.BoxLayoutV inside a GridLayout won't expadn properly.
		// So, lets get stupid!
		local colors = mSkinColors = GUI.Component(GUI.GridLayout(16, 1));
		colors.getLayoutManager().setRows(18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18);
		colors.setSize(colors.getPreferredSize());
		colorpanel.add(colors);
		
		local bodySizeComp = GUI.InnerPanel(GUI.BoxLayoutV());
		bodySizeComp.setSize(80, 350);
		bodySizeComp.setPreferredSize(80, 350);
		topDetailPanel.add(bodySizeComp);
		
		local heightTitle = GUI.HTML("<font color=\"fff77f\" size=\"32\">Height</font>");
		bodySizeComp.add(heightTitle);
		
		local DIFFERENCE = 0.1;
		local START_BODY_SIZE = 1;
		
		local slider = GUI.Slider(this.gVERTICAL_SLIDER, START_BODY_SIZE + DIFFERENCE, START_BODY_SIZE - DIFFERENCE, 230);
		slider.addChangeListener(this);
		slider.setValue(START_BODY_SIZE);		
		bodySizeComp.add(slider);
		
		local randomizeComp = GUI.Component(GUI.FlowLayout());
		randomizeComp.getLayoutManager().setAlignment("center");
		detailPanel.add(randomizeComp);
		
		local randomizebutton = GUI.NarrowButton("Randomize");
		randomizebutton.setReleaseMessage("onRandomColors");
		randomizebutton.addActionListener(this);
		randomizeComp.add(randomizebutton);
		
		//Adds the right component to the top of the window, which includes a logo on the upper
		//right side of the screen	
		topSkinComp.add(_createRightLogoComp());
		
		local fillAmount = 2;
		local bottomComp = _createBottomPreviousNextComp("< Back  ", "onRetrunRaceGender", "  Next >", "onRacePage", false, true, fillAmount);
		skinComp.add(bottomComp);
		_createInitialClothing();
	}

	/** When the body size slider gets update, update players body size 
		@param - slider - the slider for the body size when it gets passed in
	*/
	function onSliderUpdated( slider )	{
		if(slider == mEarSlider) {
			local number  = 1 + ( ( slider.getValue() - 5 ) * 0.2 );
			if (mNewAppearance && ("es" in mNewAppearance) && mNewAppearance.es == number)
				return;
			this.alterAppearance({
				es = number
			});
			local app = this.serializeAppearance(this.mNewAppearance);
			this._configureChar(app);
		}
		else if(slider == mTailSlider) {
			local number  = 1 + ( ( slider.getValue() - 5 ) * 0.2 );
			if (mNewAppearance && ("ts" in mNewAppearance) && mNewAppearance.ts == number)
				return;
			this.alterAppearance({
				ts = number
			});
			local app = this.serializeAppearance(this.mNewAppearance);
			this._configureChar(app);
		}
		else {		
			local number = slider.getValue();
	
			if (mNewAppearance && ("sz" in mNewAppearance) && mNewAppearance.sz == number)
				return;
	
			this.alterAppearance({
				sz = number
			});
			this.mPaperDoll.setScale(mDefaultSizeMultiplier * number);
		}
	}
	
	/** Creates the initial clothing and name screen */
	function _createInitialClothing()
	{
		local clothingNameComp = this.mInitialClothing = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mCard.add(clothingNameComp, "InitialClothing");
		local topClothingNameComp = this.GUI.Component(this.GUI.GridLayout(1, 2));
		topClothingNameComp.getLayoutManager().setRows(550);
		topClothingNameComp.getLayoutManager().setColumns(this.LEFT_PANEL_WIDTH + 50, 355);
		topClothingNameComp.setInsets(-5, 0, -10, 8);
		clothingNameComp.add(topClothingNameComp);
		local leftComp = this.GUI.Component(this.GUI.BoxLayoutV());
		topClothingNameComp.add(leftComp);
		local titlePanel = this._createCharacterCreationPanel();

		foreach( comp in titlePanel.components )
		{
			comp.setPosition(-4, 0);
		}

		leftComp.add(titlePanel);
		local clothingNamePanel = this.GUI.Panel(this.GUI.BoxLayoutV());
		leftComp.add(clothingNamePanel);
		local nameComp = this.GUI.Component(this.GUI.BoxLayout());
		clothingNamePanel.add(nameComp);
		local firstNameComp = this._createGeneratedNameComp("First Name", "onSuggestedFirstName", "onNewFirstNameList");
		nameComp.add(firstNameComp);
		local lastNameComp = this._createGeneratedNameComp("Last Name", "onSuggestedLastName", "onNewLastNameList");
		nameComp.add(lastNameComp);
		this.mFirstNameInput.setTabOrderTarget(this.mLastNameInput);
		this.mLastNameInput.setTabOrderTarget(this.mFirstNameInput);
		local spacerSep = this.GUI.Spacer(425, 1);
		spacerSep.setAppearance("ColumnList/HeadingDivider");
		clothingNamePanel.add(spacerSep);
		local clothingComponent = this.GUI.Component(this.GUI.BoxLayoutV());
		clothingComponent.getLayoutManager().setExpand(true);
		clothingComponent.getLayoutManager().setGap(10);
		clothingNamePanel.add(clothingComponent);
		local clothingTitle = this.GUI.HTML("<font color=\"fff77f\" size=\"38\">            Clothing</font>");
		clothingComponent.add(clothingTitle);
		local chestComp = this.GUI.Component(this.GUI.BoxLayout());
		chestComp.getLayoutManager().setGap(1);
		clothingComponent.add(chestComp);
		local chestTitle = this.GUI.HTML("<font color=\"fff77f\" size=\"32\">Chest</font>");
		chestComp.add(chestTitle);
		local spacerSep = this.GUI.Spacer(20, 10);
		chestComp.add(spacerSep);
		this.mChestGroup = this.GUI.RadioGroup();
		this.mChestGroup.addListener(this);

		for( local i = 0; i < this.Clothingids.len(); i++ )
		{
			local icon = this.GUI.Icon();
			icon.setIconImage(this.Clothingids[i] + "_C");
			icon.setData({
				c_c = i
			});
			icon.setRadioGroup(this.mChestGroup);
			chestComp.add(icon);

			if (i < ::CharacterClassName.len())
			{
				this.CharacterClassType[i].selectedChest <- icon;
			}
		}

		local legComp = this.GUI.Component(this.GUI.BoxLayout());
		legComp.getLayoutManager().setGap(1);
		clothingComponent.add(legComp);
		local legTitle = this.GUI.HTML("<font color=\"fff77f\" size=\"32\">Legs</font>");
		legComp.add(legTitle);
		local spacerSep = this.GUI.Spacer(30, 10);
		legComp.add(spacerSep);
		this.mLeggingsGroup = this.GUI.RadioGroup();
		this.mLeggingsGroup.addListener(this);

		for( local i = 0; i < this.Clothingids.len(); i++ )
		{
			local icon = this.GUI.Icon();
			icon.setIconImage(this.Clothingids[i] + "_L");
			icon.setData({
				c_l = i
			});
			icon.setRadioGroup(this.mLeggingsGroup);
			legComp.add(icon);

			if (i < ::CharacterClassName.len())
			{
				this.CharacterClassType[i].selectedLeg <- icon;
			}
		}

		local feetComp = this.GUI.Component(this.GUI.BoxLayout());
		feetComp.getLayoutManager().setGap(1);
		clothingComponent.add(feetComp);
		local feetTitle = this.GUI.HTML("<font color=\"fff77f\" size=\"32\">Feet</font>");
		feetComp.add(feetTitle);
		local spacerSep = this.GUI.Spacer(30, 10);
		feetComp.add(spacerSep);
		this.mBootsGroup = this.GUI.RadioGroup();
		this.mBootsGroup.addListener(this);

		for( local i = 0; i < this.Clothingids.len(); i++ )
		{
			local icon = this.GUI.Icon();
			icon.setIconImage(this.Clothingids[i] + "_B");
			icon.setData({
				c_b = i
			});
			icon.setRadioGroup(this.mBootsGroup);
			feetComp.add(icon);

			if (i < ::CharacterClassName.len())
			{
				this.CharacterClassType[i].selectedBoot <- icon;
			}
		}

		clothingComponent.add(this.GUI.Spacer(1, 1));
		topClothingNameComp.add(this._createRightLogoComp(28));
		local fillAmount = 4;
		local bottomComp = this._createBottomPreviousNextComp("< Back  ", "onRacePage", "  Finish", "onSubmitPersona", true, true, fillAmount);
		clothingNameComp.add(bottomComp);
		this._createClassScreen();
	}

	function _createGeneratedNameComp( title, suggestedNameCallback, newNameListCallback )
	{
		local nameHolder = this.GUI.Component(this.GUI.BoxLayoutV());
		nameHolder.getLayoutManager().setExpand(true);
		nameHolder.getLayoutManager().setGap(0);
		nameHolder.setInsets(0, 0, 0, 0);
		local nameComp = this.GUI.Component(this.GUI.BoxLayoutV());
		nameComp.getLayoutManager().setAlignment(0.5);
		nameComp.setInsets(2, 2, 2, 2);
		nameComp.getLayoutManager().setGap(0);
		nameComp.getLayoutManager().setExpand(false);
		nameHolder.add(nameComp);
		local titleHTML = this.GUI.HTML("<font color=\"fff77f\" size=\"32\">" + title + "</font>");
		titleHTML.getLayoutManager().setAlignment("center");
		nameComp.add(titleHTML);
		local NameInputC = this.GUI.Component();
		NameInputC.setMinimumSize(200, 35);
		nameComp.add(NameInputC);
		local nameInputArea = this.GUI.InputArea();
		nameInputArea.setFont(this.GUI.Font("Maiandra", 32));
		nameInputArea.setText("");
		nameInputArea.addActionListener(this);
		nameInputArea.setSize(200, 35);
		NameInputC.add(nameInputArea);

		if (title == "First Name")
		{
			this.mFirstNameInput = nameInputArea;
			this.mFirstNameInput.setAutoCapitalize(true);
			this.mFirstNameInput.setAllowOnlyLetters(true);
			this.mFirstNameInput.setAllowSpaces(false);
		}
		else if (title == "Last Name")
		{
			this.mLastNameInput = nameInputArea;
			this.mLastNameInput.setAutoCapitalize(true);
			this.mLastNameInput.setAllowOnlyLetters(true);
			this.mLastNameInput.setAllowSpaces(false);
		}

		local suggestionlabel = this.GUI.HTML("<font color=\"fff77f\" size=\"26\">Suggestions</font>");
		suggestionlabel.getLayoutManager().setAlignment("center");
		nameComp.add(suggestionlabel);
		local suggestionboxc = this.GUI.Component();
		suggestionboxc.setMinimumSize(180, 100);
		nameComp.add(suggestionboxc);
		local suggestionbox = this.GUI.SimpleList();
		suggestionbox.setSize(180, 100);
		suggestionbox.setResize(false);
		suggestionbox.setAlignment(0.5);
		suggestionbox.addSelectionChangeListener(this);
		suggestionbox.setChangeMessage(suggestedNameCallback);
		suggestionboxc.add(suggestionbox);

		if (title == "First Name")
		{
			this.mFirstNameSuggestions = suggestionbox;
		}
		else if (title == "Last Name")
		{
			this.mLastNameSuggestions = suggestionbox;
		}

		local newlistbuttonc = this.GUI.Component(this.GUI.BoxLayoutV());
		newlistbuttonc.setInsets(10, 10, 0, 10);
		nameComp.add(newlistbuttonc);
		local newlistbutton = this.GUI.NarrowButton("Generate New List", this, newNameListCallback);
		newlistbutton.setFixedSize(140, 32);
		newlistbuttonc.add(newlistbutton);
		return nameHolder;
	}

	function _createClassScreen()
	{
		local ICON_SIZE = 97;
		local characterClassComp = this.mClassPage = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mCard.add(characterClassComp, "CharacterClass");
		local topClassComp = this.GUI.Component(this.GUI.GridLayout(1, 2));
		topClassComp.getLayoutManager().setRows(525);
		topClassComp.getLayoutManager().setColumns(this.LEFT_PANEL_WIDTH, "*");
		topClassComp.setInsets(-5, 0, 15, 30);
		characterClassComp.add(topClassComp);
		local leftPanel = this.GUI.Component(this.GUI.BoxLayoutV());
		leftPanel.getLayoutManager().setGap(40);
		topClassComp.add(leftPanel);
		leftPanel.add(this._createCharacterCreationPanel());
		local innerClassComp = this.GUI.Container(null);
		innerClassComp.setSize(400, ICON_SIZE * this.CharacterClassType.len() + 16);
		innerClassComp.setPreferredSize(400, ICON_SIZE * this.CharacterClassType.len() + 16);
		leftPanel.add(innerClassComp);
		local bigClassFrame = this.GUI.BigFrame("Class", false);
		bigClassFrame.setVisible(true);
		bigClassFrame.setMovable(false);
		bigClassFrame.setTitleFont(this.GUI.Font("Maiandra", 42));
		bigClassFrame.setPosition(ICON_SIZE / 2, 0);
		bigClassFrame.setSize(325, ICON_SIZE * this.CharacterClassType.len() + 16);
		bigClassFrame.setPreferredSize(325, ICON_SIZE * this.CharacterClassType.len() + 16);
		innerClassComp.add(bigClassFrame);
		local summaryPanel = this.GUI.Container(null);
		summaryPanel.setPosition(ICON_SIZE / 2, 38);
		summaryPanel.setSize(275, (ICON_SIZE - 10) * this.CharacterClassType.len());
		summaryPanel.setPreferredSize(275, (ICON_SIZE - 10) * this.CharacterClassType.len());
		bigClassFrame.setContentPane(summaryPanel);
		local classDescription = this.GUI.InnerPanel(null);
		classDescription.setPosition(ICON_SIZE / 2, 5);
		classDescription.setSize(265, (ICON_SIZE - 12) * this.CharacterClassType.len());
		classDescription.setPreferredSize(265, (ICON_SIZE - 12) * this.CharacterClassType.len());
		summaryPanel.add(classDescription);
		local summaryText = this.mClassSummary = this.GUI.HTML();
		summaryText.setPosition(10, 10);
		summaryText.setSize(250, (ICON_SIZE - 12) * this.CharacterClassType.len() - 10);
		summaryText.setPreferredSize(250, (ICON_SIZE - 12) * this.CharacterClassType.len() - 10);
		classDescription.add(summaryText);
		local classComp = this.GUI.Container(this.GUI.BoxLayoutV());
		classComp.setPosition(0, 38);
		classComp.setSize(ICON_SIZE, ICON_SIZE * this.CharacterClassType.len());
		classComp.setPreferredSize(ICON_SIZE, ICON_SIZE * this.CharacterClassType.len());
		classComp.getLayoutManager().setGap(-6);
		local classRadioGroup = this.GUI.RadioGroup();
		classRadioGroup.addListener(this);

		foreach( className, classData in this.CharacterClassType )
		{
			local button = this.GUI.ImageButton();
			button.setSize(ICON_SIZE, ICON_SIZE);
			button.setPreferredSize(ICON_SIZE, ICON_SIZE);
			button.setAppearance(classData.iconName);
			button.setGlowImageName("SelectGlowClass");
			button.setRadioGroup(classRadioGroup);
			button.setData({
				c = className
			});

			if (className == ::CharacterClassName.KNIGHT)
			{
				this.mClassDefaultImageButton = button;
				button.setSelection(true);
				summaryText.setText(classData.summary);
			}

			classComp.add(button);
		}

		innerClassComp.add(classComp);
		topClassComp.add(this._createRightLogoComp());
		local fillAmount = 3;
		local bottomComp = this._createBottomPreviousNextComp("< Back  ", "onSkin", "  Next >", "onClothing", false, true, fillAmount);
		characterClassComp.add(bottomComp);
	}
	
	function _buildOther() {
		mOtherDetails.removeAll();
		
		// TODO only add what is available
		
		local earsLabel = GUI.HTML("<font color=\"fff77f\" size=\"24\">Ears</font>");
		earsLabel.getLayoutManager().setAlignment("center");
		mOtherDetails.add(earsLabel);
		
		mEarSlider = GUI.Slider(this.gHORIZONTAL_SLIDER, 0, 10, 40);
		mEarSlider.addChangeListener(this);
		mEarSlider.setValue(5);
		mOtherDetails.add(mEarSlider);
		
		local tailLabel = GUI.HTML("<font color=\"fff77f\" size=\"24\">Tail</font>");
		tailLabel.getLayoutManager().setAlignment("center");
		mOtherDetails.add(tailLabel);
		
		mTailSlider = GUI.Slider(this.gHORIZONTAL_SLIDER, 0, 10, 40);
		mTailSlider.addChangeListener(this);
		mTailSlider.setValue(5);		
		mOtherDetails.add(mTailSlider);
	}

	function _buildColors() {

		mSkinColors.removeAll();
		local mainlabel = GUI.HTML("<font color=\"fff77f\" size=\"22\">Main Colors</font>");
		mainlabel.getLayoutManager().setAlignment("center");
		mSkinColors.add(mainlabel);
		local font = ::GUI.Font("Maiandra", 24);

		foreach( part, x in mNewAppearance.sk ) {
			local def = ::ContentDef["Biped-" + ::Races[mNewAppearance.r] + "_" + ::Genders[mNewAppearance.g]].Skin[part];
			if ("primary" in def)
				_createOneColorPalette(def, "title" in def ? def.title : part, part, x);
		}

		local secondarylabel = GUI.HTML("<font color=\"fff77f\" size=\"22\">Secondary Colors</font>");
		secondarylabel.getLayoutManager().setAlignment("center");
		mSkinColors.add(secondarylabel);

		foreach( part, x in mNewAppearance.sk ) {
			local def = ::ContentDef["Biped-" + ::Races[mNewAppearance.r] + "_" + ::Genders[mNewAppearance.g]].Skin[part];
			if (("primary" in def) == false)
				_createOneColorPalette(def, "title" in def ? def.title : part, part, x);
		}
		
	}

	function _createOneColorPalette( def, title, part, color )
	{
		local font = ::GUI.Font("Maiandra", 24);
		local list = this.GUI.ColorPicker(title, part, this.ColorRestrictionType.CHARACTER_CREATION);
		list.setChangeMessage("onSkinColorChange");
		list.setColor(color, false);
		list.addActionListener(this);
		list.setLabelVisible(true);
		list.setLabelFont(font);
		this.mSkinColors.add(list);
	}

	function getRandomUnique( pool, used )
	{
		local name;
		local found = true;
		local tries = 50;

		while (found && tries > 0)
		{
			found = false;
			tries--;
			local index = this.rand() / this.RAND_MAX.tofloat() * pool.len();
			name = pool[index];

			foreach( na in used )
			{
				if (na == name)
				{
					found = true;
					break;
				}
			}
		}

		return name;
	}

	function populateFirstNameList()
	{
		local race = "Anura";

		if ("r" in this.mNewAppearance)
		{
			race = this.Races[this.mNewAppearance.r];
		}

		this.mFirstNameSuggestions.removeAll();
		local Used = [];
		local combinedNameList = this.deepClone(::NameSuggestions.General);
		local raceNameList = this.deepClone(::NameSuggestions[race]);
		combinedNameList.extend(raceNameList);

		for( local x = 0; x < 6; x++ )
		{
			local name = this.getRandomUnique(combinedNameList, Used);
			this.mFirstNameSuggestions.addChoice(name);
			Used.append(name);
		}
	}

	function populateLastNameList()
	{
		this.mLastNameSuggestions.removeAll();
		local Used = [];
		local race = "Anura";

		if ("r" in this.mNewAppearance)
		{
			race = this.Races[this.mNewAppearance.r];
		}

		local combinedNameList = this.deepClone(::NameSuggestions.General);
		local raceNameList = this.deepClone(::NameSuggestions[race]);
		combinedNameList.extend(raceNameList);

		for( local x = 0; x < 6; x++ )
		{
			local name = this.getRandomUnique(combinedNameList, Used);
			this.mLastNameSuggestions.addChoice(name);
			Used.append(name);
		}
	}

	function onNewFirstNameList( button )
	{
		this.populateFirstNameList();
	}

	function onNewLastNameList( button )
	{
		this.populateLastNameList();
	}

	function onSkinColorChange( list )
	{
		local name = list.getName();
		this.alterAppearance({
			sk = {
				[list.getName().tolower()] = list.getCurrent()
			}
		});
		local app = this.serializeAppearance(this.mNewAppearance);
		this._configureChar(app);
	}

	function onSkinChange( list )
	{
		local myData = list.getData();
		this.alterAppearance({
			sk = {
				[list.getData()] = list.getCurrent()
			}
		});
		local app = this.serializeAppearance(this.mNewAppearance);
		this._configureChar(app);
	}

	function onRandomColors( button )
	{
		foreach( i, x in this.mNewAppearance.sk )
		{
			local def = ::ContentDef["Biped-" + ::Races[this.mNewAppearance.r] + "_" + ::Genders[this.mNewAppearance.g]].Skin[i];
			local r = def.palette.len() > 0 ? (this.rand() % (def.palette.len() - 1)).tointeger() : 0;
			this.mNewAppearance.sk[i] = def.palette[r];
		}

		this._buildColors();
		local app = this.serializeAppearance(this.mNewAppearance);
		this._configureChar(app);
	}

	function onSuggestedFirstName( list )
	{
		this.mFirstNameInput.setText(list.getCurrent());
		this.mFirstName = list.getCurrent();
	}

	function onSuggestedLastName( list )
	{
		this.mLastNameInput.setText(list.getCurrent());
		this.mLastName = list.getCurrent();
	}

	function onConfirmation( sender, bool )
	{
		if (this.mConfirmType == "Delete")
		{
			this.mConfirmType = "";
			this.mConfirm.closeWindow();
			this.mConfirm = null;

			if (bool)
			{
				this.onDeletePersona();
			}
		}
		else if (this.mConfirm)
		{
			this.mConfirmType = "";
			this.mConfirm.closeWindow();
			this.mConfirm = null;
		}

		::GUI._Manager.requestKeyboardFocus(this.mCharacterList);
	}

	function addPersona( persona, ... )
	{
		if (this.mPersonas.len() >= this.mMaxCharacters)
		{
			this.log.error("Max Character\'s reach, can\'t add new persona: " + persona[0]);
			return;
		}

		if (vargc > 0)
		{
			this.mPersonas.insert(vargv[0], persona);
		}
		else
		{
			this.mPersonas.append(persona);
		}

		this.displayPersona(persona);
	}

	function removePersona( index )
	{
		if (!(this.mPersonas.len() > index))
		{
			this.log.error("GUI.CharacterSelection.removePersona was sent an index out or range.");
			return;
		}

		this.mPersonas.remove(index);
		this.mCharacterList.removeRow(index);
		local rows = this.mCharacterList.getSelectedRows();

		if (rows.len() > 0)
		{
			this.mCurrentSelection = rows[0];
			this._configureChar(this.mPersonas[this.mCurrentSelection][this.APPEARANCE_NUM], this.mPersonas[this.mCurrentSelection][this.EQUIPMENT_NUM]);
		}
		else
		{
			this.mCurrentSelection = 0;
			this._configureChar(null);
		}
	}

	function displayPersona( persona )
	{
		local race;

		if ("c2" == persona[2].slice(0, 2))
		{
			local look = this.unserialize(persona[2].slice(3, persona[2].len()));
			race = look.r;
		}
		else
		{
			race = null;
		}

		local level = 0;

		if (persona.len() > 4)
		{
			level = persona[4];
		}

		local profession;
		local professionStr = "";

		if (persona.len() > 5)
		{
			if (persona[5].tointeger() == 0)
			{
				profession = 0;
			}
			else
			{
				profession = persona[5].tointeger() - 1;
			}

			if (profession >= ::CharacterClassName.KNIGHT && profession <= ::CharacterClassName.DRUID)
			{
				professionStr = this.CharacterClassType[profession].profession;
			}
		}

		local selection = this.GUI.CharacterSelectionListObject(persona[1], level, race, professionStr);
		this.mCharacterList.addRow([
			selection
		]);

		if (this.mPersonas.len() == this.mMaxCharacters)
		{
			if (this.mPersonas[this.mMaxCharacters - 1] == persona)
			{
				selection.hideSeperator(true);
			}
		}
	}

	function onRowSelectionChanged( sender, index, add )
	{
		if (!add)
		{
			return;
		}

		if (index == this.mCurrentSelection)
		{
			return;
		}

		if (index == null)
		{
			index = this.mCurrentSelection;
		}

		if (this.mPersonas.len() != 0)
		{
			this.mCurrentSelection = index;
			this._configureChar(this.mPersonas[index][this.APPEARANCE_NUM], this.mPersonas[index][this.EQUIPMENT_NUM]);
		}
	}

	function onScreenResize( width, height )
	{
		this._adjustCamera();
	}

	function onStartCreation( ... )
	{
		if (this.mPersonas.len() >= this.MAX_CHARACTERS)
		{
			local message = "You are only allowed " + this.MAX_CHARACTERS + " characters.";
			this.GUI.MessageBox.show(message);
			return;
		}

		if (!this.mCreationStarted)
		{
			this.mCreationStarted = true;
			::LoadGate.Require("CharCreation", this);
			::_contentLoader.resetProgress(true);
			return;
		}

		if (!this.mCreationReady)
		{
			return;
		}

		this.mCard.validate();
		this.mNewAppearance = {};
		this.mFirstName = "";

		if (this.mFirstNameInput)
		{
			this.mFirstNameInput.setText("");
		}

		this.mLastName = "";

		if (this.mLastNameInput)
		{
			this.mLastNameInput.setText("");
		}

		this.alterAppearance({
			g = "m",
			r = "a",
			es = "1.0",
			ts = "1.0"
		});
		this.mGenderRadio.setSelected(this.mGenderDefault);
		this.mRaceRadio.setSelected(this.mRaceDefault);
		this.mBodyRadio.setSelected(this.mBodyDefault);
		this.mFaceRadio.setSelected(this.mFaceDefault);
		this.mBodyDefault.setSelectionVisible(true);
		this.mFaceDefault.setSelectionVisible(true);
		local app = this.serializeAppearance(this.mNewAppearance);
		this._configureChar(app);
		this.mCard.getLayoutManager().show("RaceGender", this.mCard);
		::GUI._Manager.releaseKeyboardFocus(this.mCharacterList);

		if (::_Connection.getProtocolVersionId() > 27)
		{
			::_Connection.sendQuery("account.tracking", this, this.StageBits.PICK_RACE);
		}
	}

	function onDeleteCharacter( ... )
	{
		if (vargc > 0)
		{
			local button = vargv[0];
			button.setSelectionVisible(false);
		}

		this.onDeleteConfirm();
	}

	function onRetrunRaceGender( ... )
	{
		if (vargc > 0)
		{
			local button = vargv[0];
			button.setSelectionVisible(false);
		}

		this.mCard.getLayoutManager().show("RaceGender", this.mCard);
	}

	function onStartSelection( ... )
	{
		if (vargc > 0)
		{
			local button = vargv[0];
			button.setSelectionVisible(false);
		}

		if (this.mClassDefaultImageButton)
		{
			this.mClassDefaultImageButton.setSelection(true);
		}

		if (this.mPersonas.len() > 0)
		{
			this._configureChar(this.mPersonas[this.mCurrentSelection][this.APPEARANCE_NUM], this.mPersonas[this.mCurrentSelection][this.EQUIPMENT_NUM]);
		}

		this.mCard.getLayoutManager().show("CharacterSelection", this.mCard);
		::GUI._Manager.requestKeyboardFocus(this.mCharacterList);

		if (this.mPersonas.len() == 0)
		{
			this.onStartCreation();
		}
	}

	function onSkin( ... )
	{
		if (vargc > 0)
		{
			local button = vargv[0];
			button.setSelectionVisible(false);
		}

		if ("c" in this.mNewAppearance)
		{
			delete this.mNewAppearance.c;
			delete this.mNewAppearance.a;
			local app = this.serializeAppearance(this.mNewAppearance);
			this._configureChar(app);
		}

		this.mCard.getLayoutManager().show("Skin", this.mCard);

		if (::_Connection.getProtocolVersionId() > 27)
		{
			::_Connection.sendQuery("account.tracking", this, this.StageBits.CUSTOM_COLORS);
		}
	}

	function onClothing( ... )
	{
		if (vargc > 0)
		{
			local button = vargv[0];
			button.setSelectionVisible(false);
		}

		if (("c" in this.mNewAppearance) == false)
		{
			local defaultClass = this.updateSelectedClothingIcon();
			this.alterAppearance({
				c = defaultClass
			});
			local app = this.serializeAppearance(this.mNewAppearance);
			this._configureChar(app);
		}

		this.populateFirstNameList();
		this.populateLastNameList();
		this.mCard.getLayoutManager().show("InitialClothing", this.mCard);

		if (::_Connection.getProtocolVersionId() > 27)
		{
			::_Connection.sendQuery("account.tracking", this, this.StageBits.PICK_NAME);
		}
	}

	function onRacePage( ... )
	{
		if (("c" in this.mNewAppearance) == false)
		{
			local defaultClass = this.updateSelectedClothingIcon();
			this.alterAppearance({
				c = defaultClass
			});
			local app = this.serializeAppearance(this.mNewAppearance);
			this._configureChar(app);
		}

		this.mCard.getLayoutManager().show("CharacterClass", this.mCard);

		if (::_Connection.getProtocolVersionId() > 27)
		{
			::_Connection.sendQuery("account.tracking", this, this.StageBits.PICK_CLASS);
		}
	}

	function updateSelectedClothingIcon()
	{
		local defaultClass = ::CharacterClassName.KNIGHT;

		if (this.mProfesssion != null)
		{
			defaultClass = this.mProfesssion;
		}

		this.mChestGroup.setSelected(this.CharacterClassType[defaultClass].selectedChest);
		this.mLeggingsGroup.setSelected(this.CharacterClassType[defaultClass].selectedLeg);
		this.mBootsGroup.setSelected(this.CharacterClassType[defaultClass].selectedBoot);
		local chestIcon = this.CharacterClassType[defaultClass].selectedChest;
		chestIcon.setSelectionVisible(true);
		local legIcon = this.CharacterClassType[defaultClass].selectedLeg;
		legIcon.setSelectionVisible(true);
		local bootIcon = this.CharacterClassType[defaultClass].selectedBoot;
		bootIcon.setSelectionVisible(true);
		return defaultClass;
	}

	function onEnterGame( sender )
	{
		::Audio.playSound("Sound-EnterGame.ogg");
		local index = this.mCharacterList.getSelectedRows()[0];
		local camera = ::_scene.getCamera("Default");
		camera.setFOVy(this.gCamera.fov * this.Math.PI / 180);
		::Pref.set("login.LastCharacter", index);
		::_stateManager.setState(this.States.GameState(index, this.mPersonas[index]));
	}

	function event_NewPersona( index )
	{
		this.addPersona([
			this.mFirstName + " " + this.mLastName,
			this.mFirstName + " " + this.mLastName,
			this.serializeAppearance(this.mNewAppearance),
			""
		]);
		this.mCard.getLayoutManager().show("CharacterSelection", this.mCard);
		this.mBackToSelection.setEnabled(true);

		if (this.mMaxCharacters == this.mPersonas.len())
		{
			this.mStartCreation.setEnabled(false);
		}

		::GUI._Manager.requestKeyboardFocus(this.mCharacterList);
		this.mCurrentSelection = this.mPersonas.len() - 1;
		this.mCharacterList.setSelectedRows([
			this.mCurrentSelection
		]);
		this.mFinishButton.setEnabled(true);
		this.mPreviousButton.setEnabled(true);
	}

	function event_NewPersonaError( data )
	{
	}

	function onSubmitPersona( sender )
	{
		local profession = this.CharacterClassType[::CharacterClassName.KNIGHT].profession;

		if (this.mProfesssion != null)
		{
			profession = this.CharacterClassType[this.mProfesssion].profession;
		}

		this.mFirstName = this.mFirstNameInput.getText();
		this.mLastName = this.mLastNameInput.getText();

		if (::_Connection.getProtocolVersionId() < 3)
		{
			::_Connection.sendQuery("", this.CharacterCreationPersonaHandler(this), [
				this.mFirstName,
				this.mLastName,
				this.serializeAppearance(this.mNewAppearance)
			]);
		}
		else
		{
			local appearanceArgs = this._generateAppearanceToServer(this.mNewAppearance);
			local args = [
				this.mFirstName,
				this.mLastName,
				profession
			];

			foreach( a in appearanceArgs )
			{
				args.append(a);
			}

			::_Connection.sendQuery("persona.create", this.CharacterCreationPersonaHandler(this), args);
		}

		this.mFinishButton.setEnabled(false);
		this.mPreviousButton.setEnabled(false);
	}

	function onNewPersonaError( error )
	{
		this.mConfirm = this.GUI.ConfirmationWindow();
		this.mConfirm.setConfirmationType(this.GUI.ConfirmationWindow.OK);
		this.mConfirm.addActionListener(this);
		this.mConfirmType = "PersonaError";
		this.mConfirm.setText(::TXT(error));
		this.mFinishButton.setEnabled(true);
		this.mPreviousButton.setEnabled(true);
	}

	function _generateAppearanceToServer( appearance )
	{
		local appearanceArgs = [
			::RaceOrder[0],
			"m",
			this.BodyStyle[::BodyStyleType.NORMAL].data,
			this.FaceStyle[::FaceStyleType.NORMAL].data,
			0.85000002,
			this.DEFAULT_CHEST_CLOTHES,
			this.DEFAULT_LEGGING_CLOTHES,
			this.DEFAULT_BOOT_CLOTHES,
			1.0,
			1.0
		];

		if ("r" in appearance)
		{
			appearanceArgs[0] = appearance.r;
		}

		if ("g" in appearance)
		{
			appearanceArgs[1] = appearance.g;
		}

		if ("b" in appearance)
		{
			appearanceArgs[2] = appearance.b;
		}

		if ("h" in appearance)
		{
			appearanceArgs[3] = appearance.h;
		}

		if ("sz" in appearance)
		{
			appearanceArgs[4] = appearance.sz;
		}

		if (("c" in appearance) && "chest" in appearance.c)
		{
			appearanceArgs[5] = appearance.c.chest.data;
		}

		if (("c" in appearance) && "leggings" in appearance.c)
		{
			appearanceArgs[6] = appearance.c.leggings.data;
		}

		if (("c" in appearance) && "boots" in appearance.c)
		{
			appearanceArgs[7] = appearance.c.boots.data;
		}

		if ("ts" in appearance)
		{
			appearanceArgs[8] = appearance.ts;
		}
		else
		{
			appearanceArgs[8] = "1";
		}

		if ("es" in appearance)
		{
			appearanceArgs[9] = appearance.es;
		}
		else
		{
			appearanceArgs[9] = "1";
		}

		if ("sk" in appearance)
		{
			foreach( part, color in appearance.sk )
			{
				appearanceArgs.append(part);
				appearanceArgs.append(color);
			}
		}

		return appearanceArgs;
	}

	function itemSelected( sender )
	{
		local v = sender.getData();
		this.alterAppearance(v);
		local app = this.serializeAppearance(this.mNewAppearance);
		this._configureChar(app);
	}

	function alterAppearance( value )
	{
		local raceGenderChange = false;

		foreach( i, x in value )
		{
			if (i == "g")
			{
				this.print("g updated");
				this.mNewAppearance.g <- x;
				raceGenderChange = true;
				local idx = 0;

				foreach( j, y in ::RaceOrder )
				{
					if (this._isRaceHidden(y))
					{
						continue;
					}

					this.mRaceIcons[idx].setIconImage("Biped-" + ::Races[y] + "_" + this.Genders[x]);
					idx++;
				}
			}

			if (i == "r")
			{
				this.print("r updated to " + x + "\n");
				this.mNewAppearance.r <- x;
				raceGenderChange = true;
			}

			if (i == "sz")
			{
				this.print("sz updated");
				this.mNewAppearance.sz <- x;
			}

			if (i == "ts")
			{
				this.print("ts updated");
				this.mNewAppearance.ts <- x;
			}

			if (i == "es")
			{
				this.print("es updated");
				this.mNewAppearance.es <- x;
			}

			if (i == "sk")
			{
				this.print("sk updated");
				raceGenderChange = false;

				foreach( j, y in x )
				{
					this.mNewAppearance.sk[j] <- y;
				}
			}

			if (i == "b")
			{
				this.print("b updated");
				raceGenderChange = true;
				this.mNewAppearance.b <- x;
			}

			if (i == "c_c")
			{
				this.print("c_c updated");

				if ("c" in this.mNewAppearance)
				{
					this.mNewAppearance.c.chest <- {
						type = "Armor-CC-" + this.Clothingids[x],
						colors = [],
						data = x
					};
				}
				else
				{
					this.mNewAppearance.c <- {
						chest = {
							type = "Armor-CC-" + this.Clothingids[x],
							colors = [],
							data = x
						}
					};
				}
			}

			if (i == "c_l")
			{
				this.print("c_l updated");

				if ("c" in this.mNewAppearance)
				{
					this.mNewAppearance.c.leggings <- {
						type = "Armor-CC-" + this.Clothingids[x],
						colors = [],
						data = x
					};
				}
				else
				{
					this.mNewAppearance.c <- {
						leggings = {
							type = "Armor-CC-" + this.Clothingids[x],
							colors = [],
							data = x
						}
					};
				}
			}

			if (i == "c_b")
			{
				this.print("c_b updated");

				if ("c" in this.mNewAppearance)
				{
					this.mNewAppearance.c.boots <- {
						type = "Armor-CC-" + this.Clothingids[x],
						colors = [],
						data = x
					};
				}
				else
				{
					this.mNewAppearance.c <- {
						boots = {
							type = "Armor-CC-" + this.Clothingids[x],
							colors = [],
							data = x
						}
					};
				}
			}

			if (i == "h")
			{
				this.print("h updated");
				this.mNewAppearance.h <- x;
			}

			if (i == "c")
			{
				this.mProfesssion = x;
				this.mClassSummary.setText(this.CharacterClassType[x].summary);
				local defaultClass = this.updateSelectedClothingIcon();
				local defaultClothesStr = this.CharacterClassType[x].clothes;

				if ("c" in this.mNewAppearance)
				{
					this.mNewAppearance.c = {
						leggings = {
							type = "Armor-CC-" + defaultClothesStr,
							colors = [],
							data = x
						},
						chest = {
							type = "Armor-CC-" + defaultClothesStr,
							colors = [],
							data = x
						},
						boots = {
							type = "Armor-CC-" + defaultClothesStr,
							colors = [],
							data = x
						}
					};
				}
				else
				{
					this.mNewAppearance.c <- {
						leggings = {
							type = "Armor-CC-" + defaultClothesStr,
							colors = [],
							data = x
						},
						chest = {
							type = "Armor-CC-" + defaultClothesStr,
							colors = [],
							data = x
						},
						boots = {
							type = "Armor-CC-" + defaultClothesStr,
							colors = [],
							data = x
						}
					};
				}

				if ("a" in this.mNewAppearance)
				{
					this.mNewAppearance.a = this.CharacterClassType[x].startEquip;
				}
				else
				{
					this.mNewAppearance.a <- this.CharacterClassType[x].startEquip;
				}
			}
		}

		if (raceGenderChange == true)
		{
			this.mNewAppearance.sk <- {};
			this.mNewAppearance.es <- 1.0;
			this.mNewAppearance.ts <- 1.0;
			local skin = ::ContentDef["Biped-" + this.Races[this.mNewAppearance.r] + "_" + this.Genders[this.mNewAppearance.g]].Skin;

			foreach( i, x in skin )
			{
				this.mNewAppearance.sk[i] <- x.def;
			}

			this._buildColors();
			this._buildOther();
			this.mDefaultSizeMultiplier = ::ContentDef["Biped-" + this.Races[this.mNewAppearance.r] + "_" + this.Genders[this.mNewAppearance.g]].SizeMultiplier;
		}
	}

	function serializeAppearance( table )
	{
		local string = this.serialize(table);
		string = "c2:" + string;
		return string;
	}

	function onDeleteConfirm( ... )
	{
		if (this.mConfirm != null)
		{
			return;
		}

		this.mConfirm = this.GUI.ConfirmationWindow();
		this.mConfirm.setConfirmationType(this.GUI.ConfirmationWindow.YES_NO);
		this.mConfirm.addActionListener(this);
		this.mConfirmType = "Delete";
		local deleteIndex = this.mCharacterList.getSelectedRows()[0];
		this.mConfirm.setText("Are you sure you want to delete <font color=\"888888\">" + this.mPersonas[deleteIndex][1] + "</font>");
	}

	function onDeletePersona( ... )
	{
		this.print("OnDeletePersona called");

		if (this.mPersonas.len() == 0)
		{
			this.log.error("GUI.CharacterSelection.onDeletePersona - persona list is empty");
		}

		this.mDeleteIndex = this.mCharacterList.getSelectedRows()[0];
		::_Connection.sendQuery("persona.delete", this.PersonaDeleteHandler(), [
			this.mDeleteIndex
		]);
	}

	function event_DeletePersonaComplete( data )
	{
		this.print("onDeletePersonaComplete called");
		this.removePersona(this.mDeleteIndex);
		this.mDeleteIndex = null;
		this.mStartCreation.setEnabled(true);

		if (this.mPersonas.len() == 0)
		{
			this.onStartCreation();
		}
		else
		{
			this.mCharacterList.mRowContents[this.mPersonas.len() - 1][0].hideSeperator(false);
		}

		this.print("onDeletePersonaComplete complete");
	}

	function onPackageComplete( pkg )
	{
		if (pkg == "CharCreation")
		{
			::LoadGate.Prefetch(this.GateTrigger.GateTypes.PLAY_STAGE_1);
			this.mCreationReady = true;
			this._createRaceGender();
			this.onStartCreation();
		}
	}

	function onPackageError( pkg, error )
	{
		this.onPackageComplete(pkg);
	}

	function event_onUpdateLoadScreen( data )
	{
		if (this.mProgressComponent != null)
		{
			if (this.mPaperDoll == null || this.mPaperDoll.isAssembled() == false)
			{
				this.mProgressComponent.setVisible(true);
			}
			else
			{
				this.mProgressComponent.setVisible(false);
			}
		}

		if (this.mScene == null || !this.mScene.isAssembled() || this.mPaperDoll == null || this.mCreationStarted && this.mRaceGender == null)
		{
			this._debug.setText("Loading", "CharCreation (Races), Packages still loading:\n" + this._contentLoader.getRequestStatusDebug());
			::_contentLoader.updateLoadScreen();
		}
		else
		{
			this._loadScreenManager.setLoadScreenVisible(false);
		}
	}

	function onDestroy()
	{
		::Audio.stopMusic();
		::LoadScreen.setListener(null);
		this.mCard.setOverlay(null);
		this.mArrowLeft.setOverlay(null);
		this.mArrowRight.setOverlay(null);
		this.mCharacterSelection.destroy();
		this.mPaperDoll.destroy();

		if (this.mScene)
		{
			this.mScene.destroy();
		}

		::_root.removeListener(this);
	}

}

