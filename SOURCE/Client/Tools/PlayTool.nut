this.require("Tools");
class this.ShakyEntry 
{
	constructor( center, amount, time, range )
	{
		this.center = center;
		this.amount = amount;
		this.time = time;

		if (range <= 0.0)
		{
			range = 10.0;
		}

		this.range = range;
		this.velocity = this.Vector3();
		this.direction = this.Vector3(this.Util.randomRange(-1.0, 1.0), this.Util.randomRange(-1.0, 1.0), this.Util.randomRange(-1.0, 1.0));
		this.direction.normalize();
	}

	direction = null;
	velocity = null;
	center = null;
	amount = null;
	time = null;
	range = 1.0;
	fade = 1.0;
}

class this.PlayTool extends this.AbstractCameraTool
{
	mDefaultKeybindings = null;
	Commands = {};
	mShakyList = [];
	mOldX = 0;
	mOldY = 0;
	mMouseOriginX = 0;
	mMouseOriginY = 0;
	mDragDist = 0;
	mRotateButton = 1;
	mMoveButton = 2;
	mOrientButton = 3;
	mSelectingSO = null;
	mOrientPressed = false;
	mRotatePressed = false;
	mTwoMousePressed = false;
	mDragged = false;
	mOrienting = false;
	mRotating = false;
	mMoving = false;
	mDestDistance = 30.0;
	mDistance = 30.0;
	mPitch = 0.0;
	mMenu = null;
	mYaw = 0.0;
	mDestYaw = 0.0;
	mAutoRun = false;
	mCurrentTargetId = null;
	MAIN_COMMAND_START = 0;
	MAIN_COMMAND_END = 0;
	CONTROL_COMMAND_START = 0;
	CONTROL_COMMAND_END = 0;
	TARGETING_COMMAND_START = 0;
	TARGETING_COMMAND_END = 0;
	QUICKBAR_COMMAND_START = 0;
	QUICKBAR_COMMAND_END = 0;
	mForwardKey = null;
	mBackwardKey = null;
	mStrafeLeftKey = null;
	mStrafeRightKey = null;
	mRotateLeftKey = null;
	mRotateRightKey = null;
	constructor()
	{
		this.AbstractCameraTool.constructor(null, false);
		::_sceneObjectManager.addListener(this);
		this.mForwardKey = {
			primary = [
				this.Key.VK_W
			],
			secondary = [
				this.Key.VK_UP
			]
		};
		this.mBackwardKey = {
			primary = [
				this.Key.VK_S
			],
			secondary = [
				this.Key.VK_DOWN
			]
		};
		this.mStrafeLeftKey = {
			primary = [
				this.Key.VK_Q
			],
			secondary = []
		};
		this.mStrafeRightKey = {
			primary = [
				this.Key.VK_E
			],
			secondary = []
		};
		this.mRotateLeftKey = {
			primary = [
				this.Key.VK_A
			],
			secondary = [
				this.Key.VK_LEFT
			]
		};
		this.mRotateRightKey = {
			primary = [
				this.Key.VK_D
			],
			secondary = [
				this.Key.VK_RIGHT
			]
		};
		this.mDefaultKeybindings = {};
		this.setDistance(this.gCamera.initalZoom);
		this.setYaw(this.gCamera.initialYaw);
		this.Commands = {
			UNBOUNDED = 0,
			[0] = {
				kFunction = "",
				event = ""
			},
			MAP = 1,
			[1] = {
				kFunction = "Map",
				event = "toggleMap"
			},
			CHARACTER_SHEET = 2,
			[2] = {
				kFunction = "Character Sheet",
				event = "toggleCharacterSheet"
			},
			INVENTORY = 3,
			[3] = {
				kFunction = "Inventory",
				event = "toggleInventory"
			},
			ABILITY_WINDOW = 4,
			[4] = {
				kFunction = "Ability Window",
				event = "toggleAbility"
			},
			QUEST_JOURNAL = 5,
			[5] = {
				kFunction = "Quest Journal",
				event = "toggleJournal"
			},
			SOCIAL = 6,
			[6] = {
				kFunction = "Social",
				event = "toggleSocial"
			},
			CREDIT_SHOP = 7,
			[7] = {
				kFunction = "Credit Shop",
				event = "toggleCreditShop"
			},
			QUICK_BARS = 8,
			[8] = {
				kFunction = "Quick Bars",
				event = "toggleQuickbarSwitches"
			},
			TOGGLE_SHOW_WEAPONS = 9,
			[9] = {
				kFunction = "Toggle Show Weapons",
				event = "_toggleWeapons"
			},
			REPLY = 10,
			[10] = {
				kFunction = "Reply Last Tell",
				event = "reply"
			},
			MOVE_FORWARD = 50,
			[50] = {
				kFunction = "Move Forward",
				event = "_forwardStart"
			},
			MOVE_BACKWARD = 51,
			[51] = {
				kFunction = "Move Backward",
				event = "_backwardStart"
			},
			TURN_LEFT = 52,
			[52] = {
				kFunction = "Move Left",
				event = "_leftStart"
			},
			TURN_RIGHT = 53,
			[53] = {
				kFunction = "Move Right",
				event = "_rightStart"
			},
			STRAFE_LEFT = 54,
			[54] = {
				kFunction = "Strafe Left",
				event = "_strafeLeftStart"
			},
			STRAFE_RIGHT = 55,
			[55] = {
				kFunction = "Strafe Right",
				event = "_strafeRightStart"
			},
			AUTO_RUN = 56,
			[56] = {
				kFunction = "Toggle Auto-Run",
				event = "toggleAutoRun"
			},
			JUMP = 57,
			[57] = {
				kFunction = "Jump",
				event = "jump"
			},
			SELF = 100,
			[100] = {
				kFunction = "Self",
				event = "targetPartyMember",
				data = [
					0
				]
			},
			PARTY_1 = 101,
			[101] = {
				kFunction = "Party Member 1",
				event = "targetPartyMember",
				data = [
					1
				]
			},
			PARTY_2 = 102,
			[102] = {
				kFunction = "Party Member 2",
				event = "targetPartyMember",
				data = [
					2
				]
			},
			PARTY_3 = 103,
			[103] = {
				kFunction = "Party Member 3",
				event = "targetPartyMember",
				data = [
					3
				]
			},
			PARTY_4 = 104,
			[104] = {
				kFunction = "Party Member 4",
				event = "targetPartyMember",
				data = [
					4
				]
			},
			NEXT_ENEMY = 105,
			[105] = {
				kFunction = "Next Enemy",
				event = "_selectNextCreature"
			},
			QUICKBAR_1_SLOT_1 = 150,
			[150] = {
				kFunction = "Use Action Bar - Slot 1",
				event = "useQuickbarSlot",
				data = [
					0,
					0
				]
			},
			QUICKBAR_1_SLOT_2 = 151,
			[151] = {
				kFunction = "Use Action Bar - Slot 2",
				event = "useQuickbarSlot",
				data = [
					0,
					1
				]
			},
			QUICKBAR_1_SLOT_3 = 152,
			[152] = {
				kFunction = "Use Action Bar - Slot 3",
				event = "useQuickbarSlot",
				data = [
					0,
					2
				]
			},
			QUICKBAR_1_SLOT_4 = 153,
			[153] = {
				kFunction = "Use Action Bar - Slot 4",
				event = "useQuickbarSlot",
				data = [
					0,
					3
				]
			},
			QUICKBAR_1_SLOT_5 = 154,
			[154] = {
				kFunction = "Use Action Bar - Slot 5",
				event = "useQuickbarSlot",
				data = [
					0,
					4
				]
			},
			QUICKBAR_1_SLOT_6 = 155,
			[155] = {
				kFunction = "Use Action Bar - Slot 6",
				event = "useQuickbarSlot",
				data = [
					0,
					5
				]
			},
			QUICKBAR_1_SLOT_7 = 156,
			[156] = {
				kFunction = "Use Action Bar - Slot 7",
				event = "useQuickbarSlot",
				data = [
					0,
					6
				]
			},
			QUICKBAR_1_SLOT_8 = 157,
			[157] = {
				kFunction = "Use Action Bar - Slot 8",
				event = "useQuickbarSlot",
				data = [
					0,
					7
				]
			},
			QUICKBAR_2_SLOT_1 = 158,
			[158] = {
				kFunction = "Use Quick Bar 1 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					1,
					0
				]
			},
			QUICKBAR_2_SLOT_2 = 159,
			[159] = {
				kFunction = "Use Quick Bar 1 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					1,
					1
				]
			},
			QUICKBAR_2_SLOT_3 = 160,
			[160] = {
				kFunction = "Use Quick Bar 1 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					1,
					2
				]
			},
			QUICKBAR_2_SLOT_4 = 161,
			[161] = {
				kFunction = "Use Quick Bar 1 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					1,
					3
				]
			},
			QUICKBAR_2_SLOT_5 = 162,
			[162] = {
				kFunction = "Use Quick Bar 1 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					1,
					4
				]
			},
			QUICKBAR_2_SLOT_6 = 163,
			[163] = {
				kFunction = "Use Quick Bar 1 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					1,
					5
				]
			},
			QUICKBAR_2_SLOT_7 = 164,
			[164] = {
				kFunction = "Use Quick Bar 1 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					1,
					6
				]
			},
			QUICKBAR_2_SLOT_8 = 165,
			[165] = {
				kFunction = "Use Quick Bar 1 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					1,
					7
				]
			},
			QUICKBAR_3_SLOT_1 = 166,
			[166] = {
				kFunction = "Use Quick Bar 2 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					2,
					0
				]
			},
			QUICKBAR_3_SLOT_2 = 167,
			[167] = {
				kFunction = "Use Quick Bar 2 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					2,
					1
				]
			},
			QUICKBAR_3_SLOT_3 = 168,
			[168] = {
				kFunction = "Use Quick Bar 2 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					2,
					2
				]
			},
			QUICKBAR_3_SLOT_4 = 169,
			[169] = {
				kFunction = "Use Quick Bar 2 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					2,
					3
				]
			},
			QUICKBAR_3_SLOT_5 = 170,
			[170] = {
				kFunction = "Use Quick Bar 2 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					2,
					4
				]
			},
			QUICKBAR_3_SLOT_6 = 171,
			[171] = {
				kFunction = "Use Quick Bar 2 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					2,
					5
				]
			},
			QUICKBAR_3_SLOT_7 = 172,
			[172] = {
				kFunction = "Use Quick Bar 2 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					2,
					6
				]
			},
			QUICKBAR_3_SLOT_8 = 173,
			[173] = {
				kFunction = "Use Quick Bar 2 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					2,
					7
				]
			},
			QUICKBAR_4_SLOT_1 = 174,
			[174] = {
				kFunction = "Use Quick Bar 3 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					3,
					0
				]
			},
			QUICKBAR_4_SLOT_2 = 175,
			[175] = {
				kFunction = "Use Quick Bar 3 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					3,
					1
				]
			},
			QUICKBAR_4_SLOT_3 = 176,
			[176] = {
				kFunction = "Use Quick Bar 3 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					3,
					2
				]
			},
			QUICKBAR_4_SLOT_4 = 177,
			[177] = {
				kFunction = "Use Quick Bar 3 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					3,
					3
				]
			},
			QUICKBAR_4_SLOT_5 = 178,
			[178] = {
				kFunction = "Use Quick Bar 3 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					3,
					4
				]
			},
			QUICKBAR_4_SLOT_6 = 179,
			[179] = {
				kFunction = "Use Quick Bar 3 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					3,
					5
				]
			},
			QUICKBAR_4_SLOT_7 = 180,
			[180] = {
				kFunction = "Use Quick Bar 3 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					3,
					6
				]
			},
			QUICKBAR_4_SLOT_8 = 181,
			[181] = {
				kFunction = "Use Quick Bar 3 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					3,
					7
				]
			},
			QUICKBAR_5_SLOT_1 = 182,
			[182] = {
				kFunction = "Use Quick Bar 4 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					4,
					0
				]
			},
			QUICKBAR_5_SLOT_2 = 183,
			[183] = {
				kFunction = "Use Quick Bar 4 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					4,
					1
				]
			},
			QUICKBAR_5_SLOT_3 = 184,
			[184] = {
				kFunction = "Use Quick Bar 4 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					4,
					2
				]
			},
			QUICKBAR_5_SLOT_4 = 185,
			[185] = {
				kFunction = "Use Quick Bar 4 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					4,
					3
				]
			},
			QUICKBAR_5_SLOT_5 = 186,
			[186] = {
				kFunction = "Use Quick Bar 4 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					4,
					4
				]
			},
			QUICKBAR_5_SLOT_6 = 187,
			[187] = {
				kFunction = "Use Quick Bar 4 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					4,
					5
				]
			},
			QUICKBAR_5_SLOT_7 = 188,
			[188] = {
				kFunction = "Use Quick Bar 4 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					4,
					6
				]
			},
			QUICKBAR_5_SLOT_8 = 189,
			[189] = {
				kFunction = "Use Quick Bar 4 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					4,
					7
				]
			},
			QUICKBAR_6_SLOT_1 = 190,
			[190] = {
				kFunction = "Use Quick Bar 5 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					5,
					0
				]
			},
			QUICKBAR_6_SLOT_2 = 191,
			[191] = {
				kFunction = "Use Quick Bar 5 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					5,
					1
				]
			},
			QUICKBAR_6_SLOT_3 = 192,
			[192] = {
				kFunction = "Use Quick Bar 5 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					5,
					2
				]
			},
			QUICKBAR_6_SLOT_4 = 193,
			[193] = {
				kFunction = "Use Quick Bar 5 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					5,
					3
				]
			},
			QUICKBAR_6_SLOT_5 = 194,
			[194] = {
				kFunction = "Use Quick Bar 5 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					5,
					4
				]
			},
			QUICKBAR_6_SLOT_6 = 195,
			[195] = {
				kFunction = "Use Quick Bar 5 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					5,
					5
				]
			},
			QUICKBAR_6_SLOT_7 = 196,
			[196] = {
				kFunction = "Use Quick Bar 5 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					5,
					6
				]
			},
			QUICKBAR_6_SLOT_8 = 197,
			[197] = {
				kFunction = "Use Quick Bar 5 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					5,
					7
				]
			},
			QUICKBAR_7_SLOT_1 = 198,
			[198] = {
				kFunction = "Use Quick Bar 6 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					6,
					0
				]
			},
			QUICKBAR_7_SLOT_2 = 199,
			[199] = {
				kFunction = "Use Quick Bar 6 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					6,
					1
				]
			},
			QUICKBAR_7_SLOT_3 = 200,
			[200] = {
				kFunction = "Use Quick Bar 6 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					6,
					2
				]
			},
			QUICKBAR_7_SLOT_4 = 201,
			[201] = {
				kFunction = "Use Quick Bar 6 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					6,
					3
				]
			},
			QUICKBAR_7_SLOT_5 = 202,
			[202] = {
				kFunction = "Use Quick Bar 6 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					6,
					4
				]
			},
			QUICKBAR_7_SLOT_6 = 203,
			[203] = {
				kFunction = "Use Quick Bar 6 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					6,
					5
				]
			},
			QUICKBAR_7_SLOT_7 = 204,
			[204] = {
				kFunction = "Use Quick Bar 6 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					6,
					6
				]
			},
			QUICKBAR_7_SLOT_8 = 205,
			[205] = {
				kFunction = "Use Quick Bar 6 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					6,
					7
				]
			},
			QUICKBAR_8_SLOT_1 = 206,
			[206] = {
				kFunction = "Use Quick Bar 7 - Slot 1",
				event = "useQuickbarSlot",
				data = [
					7,
					0
				]
			},
			QUICKBAR_8_SLOT_2 = 207,
			[207] = {
				kFunction = "Use Quick Bar 7 - Slot 2",
				event = "useQuickbarSlot",
				data = [
					7,
					1
				]
			},
			QUICKBAR_8_SLOT_3 = 208,
			[208] = {
				kFunction = "Use Quick Bar 7 - Slot 3",
				event = "useQuickbarSlot",
				data = [
					7,
					2
				]
			},
			QUICKBAR_8_SLOT_4 = 209,
			[209] = {
				kFunction = "Use Quick Bar 7 - Slot 4",
				event = "useQuickbarSlot",
				data = [
					7,
					3
				]
			},
			QUICKBAR_8_SLOT_5 = 210,
			[210] = {
				kFunction = "Use Quick Bar 7 - Slot 5",
				event = "useQuickbarSlot",
				data = [
					7,
					4
				]
			},
			QUICKBAR_8_SLOT_6 = 211,
			[211] = {
				kFunction = "Use Quick Bar 7 - Slot 6",
				event = "useQuickbarSlot",
				data = [
					7,
					5
				]
			},
			QUICKBAR_8_SLOT_7 = 212,
			[212] = {
				kFunction = "Use Quick Bar 7 - Slot 7",
				event = "useQuickbarSlot",
				data = [
					7,
					6
				]
			},
			QUICKBAR_8_SLOT_8 = 213,
			[213] = {
				kFunction = "Use Quick Bar 7 - Slot 8",
				event = "useQuickbarSlot",
				data = [
					7,
					7
				]
			},
			SPEED_INCREASE = 500,
			[500] = {
				kFunction = "Increase Speed",
				event = "_increaseAvatarSpeed"
			},
			SPEED_DECREASE = 501,
			[501] = {
				kFunction = "Decrease Speed",
				event = "_decreaseAvatarSpeed"
			},
			ITEM_APPEARANCE = 502,
			[502] = {
				kFunction = "Item Appearance",
				event = "_showItemAppearanceEditor"
			},
			MAIN_MENU = 503,
			[503] = {
				kFunction = "Main Menu",
				event = "_onOptionsMenuToggle"
			}
		};
		this.MAIN_COMMAND_START = this.Commands.MAP;
		this.MAIN_COMMAND_END = this.Commands.REPLY;
		this.CONTROL_COMMAND_START = this.Commands.MOVE_FORWARD;
		this.CONTROL_COMMAND_END = this.Commands.JUMP;
		this.TARGETING_COMMAND_START = this.Commands.SELF;
		this.TARGETING_COMMAND_END = this.Commands.NEXT_ENEMY;
		this.QUICKBAR_COMMAND_START = this.Commands.QUICKBAR_1_SLOT_1;
		this.QUICKBAR_COMMAND_END = this.Commands.QUICKBAR_8_SLOT_8;
		this.mDefaultKeybindings[this.KB(this.Key.VK_A)] <- {
			command = this.Commands.TURN_LEFT,
			primary = true,
			vkCombo = [
				this.Key.VK_A
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_B)] <- {
			command = this.Commands.ABILITY_WINDOW,
			primary = true,
			vkCombo = [
				this.Key.VK_B
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_C)] <- {
			command = this.Commands.CHARACTER_SHEET,
			primary = true,
			vkCombo = [
				this.Key.VK_C
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_D)] <- {
			command = this.Commands.TURN_RIGHT,
			primary = true,
			vkCombo = [
				this.Key.VK_D
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_E)] <- {
			command = this.Commands.STRAFE_RIGHT,
			primary = true,
			vkCombo = [
				this.Key.VK_E
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F)] <- {
			command = this.Commands.SOCIAL,
			primary = true,
			vkCombo = [
				this.Key.VK_F
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_I)] <- {
			command = this.Commands.INVENTORY,
			primary = true,
			vkCombo = [
				this.Key.VK_I
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_J)] <- {
			command = this.Commands.QUEST_JOURNAL,
			primary = true,
			vkCombo = [
				this.Key.VK_J
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_K)] <- {
			command = this.Commands.CREDIT_SHOP,
			primary = true,
			vkCombo = [
				this.Key.VK_K
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_M)] <- {
			command = this.Commands.MAP,
			primary = true,
			vkCombo = [
				this.Key.VK_M
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_Q)] <- {
			command = this.Commands.STRAFE_LEFT,
			primary = true,
			vkCombo = [
				this.Key.VK_Q
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_R)] <- {
			command = this.Commands.REPLY,
			primary = true,
			vkCombo = [
				this.Key.VK_R
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_S)] <- {
			command = this.Commands.MOVE_BACKWARD,
			primary = true,
			vkCombo = [
				this.Key.VK_S
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_W)] <- {
			command = this.Commands.MOVE_FORWARD,
			primary = true,
			vkCombo = [
				this.Key.VK_W
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_X)] <- {
			command = this.Commands.TOGGLE_SHOW_WEAPONS,
			primary = true,
			vkCombo = [
				this.Key.VK_X
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_Z)] <- {
			command = this.Commands.AUTO_RUN,
			primary = false,
			vkCombo = [
				this.Key.VK_Z
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_ESCAPE)] <- {
			command = this.Commands.MAIN_MENU,
			primary = false,
			vkCombo = [
				this.Key.VK_ESCAPE
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_Q)] <- {
			command = this.Commands.QUICK_BARS,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_Q
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_UP)] <- {
			command = this.Commands.MOVE_FORWARD,
			primary = false,
			vkCombo = [
				this.Key.VK_UP
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_DOWN)] <- {
			command = this.Commands.MOVE_BACKWARD,
			primary = false,
			vkCombo = [
				this.Key.VK_DOWN
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_RIGHT)] <- {
			command = this.Commands.TURN_RIGHT,
			primary = false,
			vkCombo = [
				this.Key.VK_RIGHT
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_LEFT)] <- {
			command = this.Commands.TURN_LEFT,
			primary = false,
			vkCombo = [
				this.Key.VK_LEFT
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_SPACE)] <- {
			command = this.Commands.JUMP,
			primary = true,
			vkCombo = [
				this.Key.VK_SPACE
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_ADD)] <- {
			command = this.Commands.SPEED_INCREASE,
			primary = true,
			vkCombo = [
				this.Key.VK_ADD
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_SUBTRACT)] <- {
			command = this.Commands.SPEED_DECREASE,
			primary = true,
			vkCombo = [
				this.Key.VK_SUBTRACT
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F1)] <- {
			command = this.Commands.SELF,
			primary = true,
			vkCombo = [
				this.Key.VK_F1
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F2)] <- {
			command = this.Commands.PARTY_1,
			primary = true,
			vkCombo = [
				this.Key.VK_F2
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F3)] <- {
			command = this.Commands.PARTY_2,
			primary = true,
			vkCombo = [
				this.Key.VK_F3
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F4)] <- {
			command = this.Commands.PARTY_3,
			primary = true,
			vkCombo = [
				this.Key.VK_F4
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F5)] <- {
			command = this.Commands.PARTY_4,
			primary = true,
			vkCombo = [
				this.Key.VK_F5
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_F8)] <- {
			command = this.Commands.ITEM_APPEARANCE,
			primary = true,
			vkCombo = [
				this.Key.VK_F8
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_TAB)] <- {
			command = this.Commands.NEXT_ENEMY,
			primary = true,
			vkCombo = [
				this.Key.VK_TAB
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_1_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.c_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_2_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.a_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_3_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_ALT,
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.s_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_4_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.C_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_5_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_SHIFT,
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.A_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_6_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_ALT,
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.ca_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_7_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_8
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_1)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_1,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_1
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_2)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_2,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_2
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_3)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_3,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_3
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_4)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_4,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_4
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_5)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_5,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_5
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_6)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_6,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_6
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_7)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_7,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_7
			]
		};
		this.mDefaultKeybindings[this.CSA_KB(this.Key.VK_8)] <- {
			command = this.Commands.QUICKBAR_8_SLOT_8,
			primary = true,
			vkCombo = [
				this.Key.VK_SHIFT,
				this.Key.VK_CONTROL,
				this.Key.VK_ALT,
				this.Key.VK_8
			]
		};
		this.mKeyBindings = this.deepClone(this.mDefaultKeybindings);
		this.setCustomKeybindings(::Pref.get("control.Keybindings"));
	}

	function activate()
	{
	}

	function addShaky( center, val, speed, range )
	{
		this.mShakyList.append(this.ShakyEntry(center, val, speed, range));
	}

	function beginClanCreation()
	{
		local request = ::GUI.PopupInputBox("Enter desired Clan name:");
		request.setActionName("onClanNameEntered");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function beginClanTransfer()
	{
		local request = ::GUI.PopupQuestionBox("Do you wish to transfer Clan Leadership?");
		request.setAcceptActionName("onClanTransferEnter");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function deactivate()
	{
		if (::_cursor)
		{
			::_cursor.setState(this.GUI.Cursor.DEFAULT);
			::_cursor.setLocked(false);
		}
	}

	function decreaseAvatarSpeed()
	{
		this._avatar.getController().onDecreaseAvatarSpeed();
	}

	function getActiveKeybindings()
	{
		return this.mKeyBindings;
	}

	function getCursor()
	{
		if (!this.mRotating && !this.mMoving && !this.mOrienting)
		{
			local cursorpos = this.Screen.getCursorPos();
			local so = this._sceneObjectManager.pickSceneObject(cursorpos.x, cursorpos.y, this.QueryFlags.ANY, false);

			if (so && so.isCreature())
			{
				if (::_avatar.getID() != so.getID())
				{
					if (so.getMeta("copper_shopkeeper") || so.getMeta("credit_shopkeeper") || so.getMeta("vault"))
					{
						return "Cursor/Vendor";
					}

					if (so.getMeta("essence_vendor"))
					{
						return "Cursor/Vendor";
					}

					if (so.getMeta("clan_registrar"))
					{
						return "Cursor/Use";
					}

					if (so.hasStatusEffect(this.StatusEffects.TRANSFORMER))
					{
						return "Cursor/Vendor";
					}

					if (so.getMeta("crafter"))
					{
						return "Cursor/Vendor";
					}

					if (so.getMeta("credit_shop") != null)
					{
						return "Cursor/Vendor";
					}
				}
			}
		}

		return null;
	}

	function getCommands()
	{
		return this.Commands;
	}

	function getDefaultKeybindings()
	{
		return this.mDefaultKeybindings;
	}

	function getDistance()
	{
		return this.mDistance;
	}

	function getOrbiting()
	{
		return this.mOrienting || this.mRotating;
	}

	function getPitch()
	{
		return this.mPitch;
	}

	function getYaw()
	{
		return this.mYaw;
	}

	function handlePlayersMovement()
	{
		local forwardKeyDown = this.isKeyComboDown(this.mForwardKey.primary) || this.isKeyComboDown(this.mForwardKey.secondary);
		local backwardKeyDown = this.isKeyComboDown(this.mBackwardKey.primary) || this.isKeyComboDown(this.mBackwardKey.secondary);
		local strafeLeftKeyDown = this.isKeyComboDown(this.mStrafeLeftKey.primary) || this.isKeyComboDown(this.mStrafeLeftKey.secondary);
		local strafeRightKeyDown = this.isKeyComboDown(this.mStrafeRightKey.primary) || this.isKeyComboDown(this.mStrafeRightKey.secondary);
		local rotateLeftKeyDown = this.isKeyComboDown(this.mRotateLeftKey.primary) || this.isKeyComboDown(this.mRotateLeftKey.secondary);
		local rotateRightKeyDown = this.isKeyComboDown(this.mRotateRightKey.primary) || this.isKeyComboDown(this.mRotateRightKey.secondary);
		local autoMove = false;
		local avatarController = ::_avatar.getController();

		if (avatarController)
		{
			autoMove = avatarController.isAutoMoving();
		}

		if (forwardKeyDown && backwardKeyDown)
		{
			this._forwardBackwardStop();
		}
		else
		{
			if ((!forwardKeyDown && !this.mTwoMousePressed) && !this.mAutoRun && !autoMove)
			{
				this._forwardStop();
			}

			if (!backwardKeyDown)
			{
				this._backwardStop();
			}
		}

		if (strafeLeftKeyDown && strafeRightKeyDown)
		{
			this._strafeLeftRightStop();
		}
		else
		{
			if (!strafeLeftKeyDown)
			{
				if (rotateLeftKeyDown && this.mOrientPressed)
				{
				}
				else
				{
					this._strafeLeftStop();
				}
			}

			if (!strafeRightKeyDown)
			{
				if (rotateRightKeyDown && this.mOrientPressed)
				{
				}
				else
				{
					this._strafeRightStop();
				}
			}
		}

		if (rotateLeftKeyDown || rotateRightKeyDown)
		{
			if (rotateLeftKeyDown && rotateRightKeyDown)
			{
				this._rotateStop();
			}
		}
		else
		{
			this._rotateStop();
		}

		local movementKeys = [
			forwardKeyDown,
			backwardKeyDown,
			strafeLeftKeyDown,
			strafeRightKeyDown,
			rotateLeftKeyDown,
			rotateRightKeyDown
		];
		local isRightButtonDown = this.Mouse.isMouseDown(this.mRotateButton);
		local isLeftButtonDown = this.Mouse.isMouseDown(this.mOrientButton);
		local isAnyKeyDown = false;

		foreach( key in movementKeys )
		{
			if (key)
			{
				isAnyKeyDown = true;
				break;
			}
		}

		if (this.mAutoRun && this._avatar.getController().isForwardDirectionStopped() && !this._loadScreenManager.getLoadScreenVisible())
		{
			this.toggleAutoRun();
		}

		if (!isAnyKeyDown && !isRightButtonDown && !isLeftButtonDown && !this.mAutoRun && !autoMove)
		{
			this._forwardBackwardStop();
			this._strafeLeftRightStop();
			this._rotateStop();
		}
	}

	function increaseAvatarSpeed()
	{
		this._avatar.getController().onIncreaseAvatarSpeed();
	}

	function isKeyComboDown( keyCombo )
	{
		if (keyCombo.len() == 0)
		{
			return false;
		}

		foreach( key in keyCombo )
		{
			if (!this.Key.isDown(key))
			{
				return false;
			}
		}

		return true;
	}

	function jump()
	{
		if (!this._avatar.isDead() && !this._avatar.isGMFrozen())
		{
			this._avatar.setJumping(true);
			this._avatar.getController().onAvatarJump();
		}
	}

	function onAddFriend( id )
	{
		::_Connection.sendQuery("friends.add", this.AddFriendHandler(id), [
			id
		]);
	}

	function onAvatarSet()
	{
	}

	function onClanCreateAccept( window )
	{
		window.destroy();

		if (::_Connection.getProtocolVersionId() < 28)
		{
			local request = ::GUI.PopupInputBox("Enter Clan leader password:");
			request.setActionName("onClanPasswordInput");
			request.setData(window.getData());
			request.setPassword(true);
			request.addActionListener(this);
			request.showInputBox();
			request.center();
		}
		else
		{
			this.onClanPasswordInput(window.getData());
		}
	}

	function onClanGetName( window )
	{
		local nameNextLeader = window.getText();
		local socialWindow = ::Screens.get("SocialWindow", true);

		if (socialWindow)
		{
			local clanInfo = socialWindow.findClanMember(nameNextLeader);

			if (clanInfo == null)
			{
				local nameInputBox = ::GUI.PopupInputBox("This name must be a current member of the Clan.<br>Please enter the name of the new Clan Leader");
				nameInputBox.setActionName("onClanGetName");
				nameInputBox.addActionListener(this);
				nameInputBox.showInputBox();
				nameInputBox.center();
				window.destroy();
				return;
			}

			local request = ::GUI.PopupQuestionBox("Are you sure you wish to transfer leadership of " + socialWindow.getClanName() + " to " + nameNextLeader + "?");
			request.setAcceptActionName("onClanTransferLeadership");
			request.showInputBox();
			request.addActionListener(this);
			request.setData(nameNextLeader);
			request.center();
			window.destroy();
		}
	}

	function onClanNameEntered( window )
	{
		local text = this.Util.trim(window.getText());

		if (text.len() == 0)
		{
			local request = ::GUI.PopupInputBox("The Clan name is too short. Please choose a name<br>that is 20 characters or less:");
			request.setActionName("onClanMOTDInput");
			request.addActionListener(this);
			request.showInputBox();
			request.center();
			return;
		}

		if (text.len() > 20)
		{
			local request = ::GUI.PopupInputBox("The Clan name is too long. Please choose a name<br>that is 20 characters or less:");
			request.setActionName("onClanMOTDInput");
			request.addActionListener(this);
			request.showInputBox();
			request.center();
			return;
		}

		window.destroy();
		local request = ::GUI.PopupQuestionBox("You wish to create a Clan named \'" + text + "\'.<br>Creating a Clan costs <font color=\"ffdd66\"><b>10 gold</b></font>. Do you wish to proceed?");
		request.setAcceptActionName("onClanCreateAccept");
		request.addActionListener(this);
		request.showInputBox();
		request.setData(text);
		request.center();
	}

	function onClanPasswordInput( window )
	{
		if (::_Connection.getProtocolVersionId() < 28)
		{
			local password = window.getText();

			if (password == "")
			{
				local request = ::GUI.PopupInputBox("Password cannot be blank. Please enter a Clan leader password:");
				request.setActionName("onClanPasswordInput");
				request.setData(window.getData());
				request.setPassword(true);
				request.addActionListener(this);
				request.showInputBox();
				request.center();
				return;
			}

			::_Connection.sendQuery("clan.create", this, [
				window.getData(),
				password
			]);
			window.destroy();
		}
		else
		{
			::_Connection.sendQuery("clan.create", this, [
				window
			]);
		}
	}

	function onClanTransferEnter( window )
	{
		local nameInputBox = ::GUI.PopupInputBox("Enter the name of the new Clan Leader");
		nameInputBox.setActionName("onClanGetName");
		nameInputBox.addActionListener(this);
		nameInputBox.showInputBox();
		nameInputBox.center();
		window.destroy();
	}

	function onClanTransferLeadership( window )
	{
		local nameNextLeader = window.getData();
		::_Connection.sendQuery("clan.transfer", this, [
			nameNextLeader
		]);
		window.destroy();
	}

	function onEnterFrame()
	{
		if (::_avatar != null)
		{
			local controller = ::_avatar.getController();

			if (controller == null)
			{
				return;
			}

			local delta = this._deltat / 1000.0;
			local avatarPos = ::_avatar.getPosition();
			local avatarRot = ::_avatar.getRotation();
			local camNode = this._getCameraNode();
			this.mYaw = this.Math.GravitateAngle(this.mYaw, this.mDestYaw, delta, 6.0);

			if ((this.mOrienting || this.mMoving) && ::_avatar.isDead() == false && ::_avatar.isGMFrozen() == false)
			{
				::_avatar.setHeading(this.Math.deg2rad(this.Math.FloatModulos(this.mYaw - 180.0, 360.0)));
				this.mDestYaw = this.mYaw;
			}
			else if (this.mRotating == false && this.mMoving == false && controller.getInMotion())
			{
				this.mDestYaw = this.Math.FloatModulos(this.Math.rad2deg(::_avatar.getRotation()) + 180.0, 360.0);
			}

			local qx = this.Quaternion(this.Math.deg2rad(this.mPitch), this.Vector3(1.0, 0.0, 0.0));
			local qy = this.Quaternion(this.Math.deg2rad(this.mYaw), this.Vector3(0.0, 1.0, 0.0));
			local qyaw = qy * qx;
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
			local wh = this.Util.getWaterHeightAt(camPos);

			if (wh && camPos.y - wh < 5)
			{
				camPos.y = wh + 5;
			}

			if (!this._avatar.getAssembler())
			{
				return;
			}

			local opacity;

			if (this.mDistance < this.gCamera.transparencyDistance)
			{
				opacity = this.Math.lerp(0.75, 0.0, (this.gCamera.transparencyDistance - this.mDistance) / this.gCamera.transparencyDistance);
			}
			else
			{
				opacity = 1.0;
			}

			::_avatar.setOpacity(::_avatar.getOpacity() * opacity);
			this.handlePlayersMovement();
			local shakyOffset = this.updateShakyCam(delta);
			camNode.setPosition(camPos + shakyOffset);
			camNode.setOrientation(qyaw);
		}
	}

	function updateShakyCam( delta )
	{
		local finalOffset = this.Vector3();
		local removeList = [];

		foreach( s in this.mShakyList )
		{
			s.velocity.x += s.direction.x * (delta * s.amount);
			s.velocity.y += s.direction.y * (delta * s.amount);
			s.velocity.z += s.direction.z * (delta * s.amount);
			local dist = this.Math.max(0.0, 1.0 - ::_avatar.getPosition().distance(s.center) / s.range);
			finalOffset.x += this.sin(s.velocity.x) * s.fade * dist;
			finalOffset.y += this.sin(s.velocity.y) * s.fade * dist;
			finalOffset.z += this.sin(s.velocity.z) * s.fade * dist;
			s.fade = this.Math.max(s.fade - delta * (1.0 / s.time), 0.0);

			if (s.fade <= 0.0)
			{
				removeList.append(s);
			}
		}

		foreach( s in removeList )
		{
			local index = this.Util.indexOf(this.mShakyList, s);
			this.mShakyList.remove(index);
		}

		return finalOffset;
	}

	function onKeyPressed( evt )
	{
		local bindKey = this.KeyHelper.getKeyText(evt, true);

		if (bindKey in this.mKeyBindings)
		{
			if (this.mKeyBindings[bindKey].command != this.Commands.UNBOUNDED)
			{
				local command = this.Commands[this.mKeyBindings[bindKey].command];
				local event = command.event;
				local data;

				if ("data" in command)
				{
					data = command.data;
				}

				if (data != null)
				{
					this[event].call(this, data);
				}
				else
				{
					this[event].call(this);
				}
			}

			evt.consume();
		}
	}

	function onMenuItemPressed( menu, menuID )
	{
		switch(menuID)
		{
		case "Shop":
			this.onShopMenuItemPressed(menu);
			break;

		case "AddFriend":
			this.onAddFriend(this._sceneObjectManager.getCreatureByID(menu.getData()).getStat(this.Stat.DISPLAY_NAME));
			break;

		case "Trade":
			this.onTradeMenuItemPressed(menu);
			break;

		case "InviteToClan":
			::_Connection.sendQuery("clan.invite", this, [
				this._sceneObjectManager.getCreatureByID(menu.getData()).getStat(this.Stat.DISPLAY_NAME)
			]);
			break;

		case "InviteToParty":
			::partyManager.invite(menu.getData());
			break;

		case "IM":
			::_ChatWindow.onStartChatInput();
			::_ChatWindow.addStringInput("/tell \"" + this._sceneObjectManager.getCreatureByID(menu.getData()).getStat(this.Stat.DISPLAY_NAME) + "\" ");
			break;

		case "Ignore":
			::_ChatManager.ignorePlayer(this._sceneObjectManager.getCreatureByID(menu.getData()).getStat(this.Stat.DISPLAY_NAME));
			break;

		case "Unignore":
			::_ChatManager.unignorePlayer(this._sceneObjectManager.getCreatureByID(menu.getData()).getStat(this.Stat.DISPLAY_NAME));
			break;

		case "Follow":
			::_avatar.getController().startFollowing(this._sceneObjectManager.getCreatureByID(menu.getData()), false);
			break;
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mOrientPressed)
		{
			this.mOrienting = true;

			if (this.Pref.get("gameplay.mousemovement") == true)
			{
				if (::_avatar.getController().isMouseMoving())
				{
					::_avatar.getController().stopFollowing();
				}
			}
		}

		if (this.mRotatePressed)
		{
			this.mRotating = true;
		}

		if (this.mRotating || this.mMoving || this.mOrienting)
		{
			if (::_cursor)
			{
				::_cursor.setState(this.GUI.Cursor.ROTATE);
				::_cursor.setLocked(true, this.mMouseOriginX, this.mMouseOriginY);
			}

			local dx = evt.x - this.mMouseOriginX;
			local dy = evt.y - this.mMouseOriginY;

			if (dx == 0 && dy == 0)
			{
				return;
			}

			local delta = 1.0;
			local amountX = dx.tofloat() * this.gCamera.sensitivity * delta;
			local amountY = dy.tofloat() * this.gCamera.sensitivity * delta;
			this.mPitch = this.Math.clamp(this.mPitch - amountY, -75.0, 75.0);
			this.mDestYaw = this.Math.FloatModulos(this.mDestYaw - amountX, 360.0);
			this.mYaw = this.mDestYaw;
			this.mDragged = true;
			this.mDragDist++;

			if (this.mOrienting)
			{
				::_avatar.serverVelosityUpdate();
			}
		}
		else
		{
			local so = this._sceneObjectManager.pickCreature(evt.x, evt.y, true);
			this.updateMouseCursor(so);
		}

		local cursorVisible = this.Screen.isCursorVisible();

		if (cursorVisible)
		{
			if (::_cursor && ::_cursor.isLocked())
			{
				this.unlockAndClearCursor();
			}
		}
		else if (this.mOrienting || this.mRotating)
		{
			local isRightButtonDown = this.Mouse.isMouseDown(this.mRotateButton);
			local isLeftButtonDown = this.Mouse.isMouseDown(this.mOrientButton);

			if (!isRightButtonDown && !isLeftButtonDown)
			{
				this.unlockAndClearCursor();
			}
		}
	}

	function onMousePressed( evt )
	{
		this.GUI._Manager.addMouseCapturer(this);

		if (evt.clickCount != 1)
		{
			return;
		}

		if (this.mDragged == false)
		{
			this.mMouseOriginX = evt.x;
			this.mMouseOriginY = evt.y;
		}

		if (evt.button == this.mRotateButton)
		{
			this.mDestYaw = this.mYaw;
			this.mRotating = false;
			this.mRotatePressed = true;
		}
		else if (evt.button == this.mMoveButton)
		{
			if (::_avatar)
			{
				::_avatar.getController().onAvatarForwardStart();
			}

			this.mMoving = true;
		}
		else if (evt.button == this.mOrientButton)
		{
			this.mOrientPressed = true;
			this.mOrienting = false;
		}

		if (::_avatar != null && (this.mOrienting || this.mMoving))
		{
			::_avatar.getController().onAvatarRotateStop();
		}

		if (this.mOrientPressed && this.mRotatePressed)
		{
			if (::_cursor)
			{
				::_cursor.setState(this.GUI.Cursor.ROTATE);
				::_cursor.setLocked(true, evt.x, evt.y);
			}

			this.mTwoMousePressed = true;
			this._forwardStart();
		}

		this.mDragDist = 0;
		this.mSelectingSO = this._sceneObjectManager.pickCreature(evt.x, evt.y, evt.button != this.mRotateButton);
		this.GUI._Manager.releaseKeyboardFocus();
	}

	function onMouseReleased( evt )
	{
		this.GUI._Manager.removeMouseCapturer(this);

		if (::_avatar == null)
		{
			return;
		}

		local so;

		if (evt.button == this.mRotateButton)
		{
			so = this._sceneObjectManager.pickCreature(evt.x, evt.y);
		}
		else
		{
			so = this._sceneObjectManager.pickCreature(evt.x, evt.y, true);
		}

		if (this.mDragDist < 4)
		{
			if (so != null && so != ::_avatar.getTargetObject() && so == this.mSelectingSO)
			{
				::_avatar.setResetTabTarget(true);
				::_avatar.setTargetObject(so);
				::_Connection.sendSelectTarget(so.getID());
			}
			else if (so == null)
			{
				::_avatar.setResetTabTarget(true);
				::_avatar.setTargetObject(null);
				::_Connection.sendSelectTarget(0);
			}
		}

		if (evt.button == this.mOrientButton)
		{
			if (this.mDragged == false)
			{
				if (so && this.mDragDist < 4)
				{
					if (::_avatar.getID() != so.getID())
					{
						::_avatar.useCreature(so);
					}
				}
				else if (this.Pref.get("gameplay.mousemovement") == true && this.mDragDist < 4)
				{
					local pointClickedOn = this.pickTerrainPoint(evt.x, evt.y);

					if (pointClickedOn != null)
					{
						::_avatar.getController().startMovingToPoint(pointClickedOn);
					}
				}
			}
			else
			{
				::_avatar.getController().stopFollowing();
			}
		}

		if (evt.button == this.mMoveButton && this.mMoving)
		{
			::_avatar.getController().onAvatarForwardStop();
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

		local isRightButtonDown = this.Mouse.isMouseDown(this.mRotateButton);
		local isLeftButtonDown = this.Mouse.isMouseDown(this.mOrientButton);

		if (!this.mMoving && !this.mRotating && !this.mOrienting || !isRightButtonDown && !isLeftButtonDown)
		{
			if (::_cursor)
			{
				this.unlockAndClearCursor();
			}
		}

		this.mDragged = false;
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

			this.mDestDistance = this.Math.clamp(this.mDestDistance + evt.units_v * (this.gCamera.sensitivity * 25) * -1, this.gCamera.minZoom, this.gCamera.maxZoom);
			this.mDragDist += 40;
		}
	}

	function onResume()
	{
		this._update(this.mCurrentDistance);
	}

	function onShopMenuItemPressed( menu )
	{
		local target = menu.getData();

		if (target)
		{
			this.Screens.get("ItemShop", true).setMerchantId(target);
			this.Screens.show("ItemShop", true);
		}
	}

	function onTradeMenuItemPressed( menu )
	{
		local targetID = menu.getData();
		local targetSO = this._sceneObjectManager.getCreatureByID(targetID);

		if (targetSO)
		{
			local targetPos = targetSO.getPosition();
			local avatarPos = ::_avatar.getPosition();

			if (targetSO.isDead())
			{
				this.IGIS.error("You cannot trade with a dead player.");
				return;
			}

			if (::_avatar.isDead())
			{
				this.IGIS.error("You cannot trade while dead.");
				return;
			}

			if (this.Math.DetermineDistanceBetweenTwoPoints(targetPos, avatarPos) < this.Screens.TradeScreen.tradeDistance)
			{
				::_TradeManager.requestTrade(targetID);
			}
			else
			{
				this.IGIS.error("Too far away to trade.");
			}
		}
	}

	function onQueryError( qa, error )
	{
		::IGIS.error(error);
	}

	function onUpdate()
	{
		if (this.mIsActivated)
		{
			this._update(this.mCurrentDistance);
		}
	}

	function targetPartyMember( args )
	{
		local partyMember = args[0];

		if (partyMember == 0)
		{
			::_avatar.setTargetObject(::_avatar);
			::_avatar.setResetTabTarget(true);
			::_Connection.sendSelectTarget(::_avatar.getID());
		}
		else
		{
			local member = ::partyManager.getMemberSceneObject(partyMember - 1);

			if (member)
			{
				::_avatar.setTargetObject(member);
				::_avatar.setResetTabTarget(true);
				::_Connection.sendSelectTarget(member.getID());
			}
		}
	}

	function sendConMessage( avatarLevel, monsterLevel )
	{
		if (avatarLevel == null || monsterLevel == null || !::_ChatManager)
		{
			return;
		}

		local levelDifference = monsterLevel - avatarLevel;
		local conString = "Grey";

		if (levelDifference <= -10)
		{
			conString = "Grey";
		}
		else if (levelDifference <= -7)
		{
			conString = "Dark Green";
		}
		else if (levelDifference <= -4)
		{
			conString = "Light Green";
		}
		else if (levelDifference <= -1)
		{
			conString = "Bright Green";
		}
		else if (0 == levelDifference)
		{
			conString = "White";
		}
		else if (1 == levelDifference)
		{
			conString = "Blue";
		}
		else if (2 == levelDifference)
		{
			conString = "Yellow";
		}
		else if (3 == levelDifference)
		{
			conString = "Orange";
		}
		else if (4 == levelDifference)
		{
			conString = "Red";
		}
		else if (5 == levelDifference)
		{
			conString = "Purple";
		}
		else if (levelDifference >= 6)
		{
			conString = "Bright Purple";
		}

		::_ChatManager.addMessage(this.ConMessages[conString].channel, this.ConMessages[conString].message);
	}

	function setCustomKeybindings( bindingArray )
	{
		::_quickBarManager.setDefaultKeybindings();

		if (bindingArray.len() == 0)
		{
			this.mKeyBindings = this.deepClone(this.mDefaultKeybindings);
		}
		else
		{
			foreach( binding in bindingArray )
			{
				this.updateKeybinding(binding[0], binding[1], binding[2], binding[3], binding[4], binding[5]);
			}
		}
	}

	function setDestDistance( value )
	{
		this.mDestDistance = value;
	}

	function setDistance( value )
	{
		this.mDestDistance = value;
		this.mDistance = value;
	}

	function setPitch( value )
	{
		this.mPitch = value;
	}

	function setupCreatureMenu( so )
	{
		if (this.mMenu)
		{
			this.mMenu.destroy();
		}

		this.mMenu = this.GUI.PopupMenu();
		this.mMenu.addActionListener(this);
		this.mMenu.addMenuOption("IM", "Instant Message");
		local name = so.getStat(this.Stat.DISPLAY_NAME);

		if (this.Util.isInIgnoreList(name))
		{
			this.mMenu.addMenuOption("Unignore", "Unignore Player");
		}
		else
		{
			this.mMenu.addMenuOption("Ignore", "Ignore Player");
		}

		this.mMenu.addMenuOption("AddFriend", "Add Friend");
		this.mMenu.addMenuOption("Trade", "Trade");
		this.mMenu.addMenuOption("InviteToClan", "Invite to Join Clan");
		this.mMenu.addMenuOption("Follow", "Follow");

		if (!::partyManager.isCreaturePartyMember(so))
		{
			this.mMenu.addMenuOption("InviteToParty", "Invite to Join Party");
		}

		this.mMenu.setData(so.getID());
	}

	function setYaw( value )
	{
		this.mYaw = value;
		this.mDestYaw = value;
	}

	function sortTargetDistances( m1, m2 )
	{
		if (m1 == m2)
		{
			return 0;
		}

		if (m1.distance < m2.distance)
		{
			return -1;
		}

		if (m1.distance > m2.distance)
		{
			return 1;
		}

		return 0;
	}

	function reply()
	{
		local lastTell = ::_ChatManager.getLastTellFrom();

		if (lastTell)
		{
			local chatWindow = this.Screens.get("ChatWindow", true);

			if (chatWindow)
			{
				chatWindow.startChatInputOnChannel("t/" + "\"" + lastTell + "\"", "r");
			}
		}
	}

	function toggleAbility()
	{
		::Screens.toggle("AbilityFrame");
	}

	function toggleAutoRun()
	{
		local controller = ::_avatar.getController();

		if (controller.getMovingForward())
		{
			controller.onAvatarForwardStop();
			this.mAutoRun = false;
		}
		else
		{
			controller.onAvatarForwardStart();
			this.mAutoRun = true;
		}
	}

	function toggleCreditShop()
	{
		::Screens.toggle("CreditShop");
	}

	function toggleCharacterSheet()
	{
		::Screens.toggle("Equipment");
	}

	function toggleInventory()
	{
		::Screens.toggle("Inventory");
	}

	function toggleJournal()
	{
		::Screens.toggle("QuestJournal");
	}

	function toggleMap()
	{
		::Screens.toggle("MapWindow");
	}

	function toggleSocial()
	{
		::Screens.toggle("SocialWindow");
	}

	function toggleQuickbarSwitches()
	{
		::Screens.toggle("QuickbarSwitches");
	}

	function useQuickbarSlot( args )
	{
		local quickbarNum = args[0];
		local quickbarIndex = args[1];
		local quickbar = this._quickBarManager.getQuickBar(quickbarNum);

		if (quickbar != null)
		{
			quickbar.activateIndex(quickbarIndex);
		}
	}

	function updateMouseCursor( so )
	{
		local revert = true;

		if (so && so.isCreature())
		{
			if (so.getMeta("persona"))
			{
				if (so.hasStatusEffect(this.StatusEffects.PVPABLE))
				{
					revert = false;
					::_cursor.setState(this.GUI.Cursor.ATTACK);
				}
			}
			else if (::_avatar.getID() != so.getID() && !so.getMeta("copper_shopkeeper") && !so.getMeta("credit_shopkeeper") && !so.getMeta("essence_vendor") && !so.getMeta("vendor") && so.getMeta("credit_shop") == null)
			{
				if (!so.isDead())
				{
					if (so.getQuestIndicator() && (so.getQuestIndicator().hasValidQuest() || so.getQuestIndicator().hasCompletedNotTurnInQuest()))
					{
						revert = false;
						::_cursor.setState(this.GUI.Cursor.USE);
					}
					else if (::_useableCreatureManager.isUseable(so.getID()))
					{
						revert = false;
						::_cursor.setState(this.GUI.Cursor.USE);
					}
					else if (!so.hasStatusEffect(this.StatusEffects.INVINCIBLE) && !so.hasStatusEffect(this.StatusEffects.UNATTACKABLE))
					{
						revert = false;
						::_cursor.setState(this.GUI.Cursor.ATTACK);
					}
				}
				else if (so.hasLoot())
				{
					if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.MAX_USE_DISTANCE)
					{
						local lootScreen = this.Screens.get("LootScreen", true);

						if (lootScreen && lootScreen.checkLootingPermissions(so))
						{
							revert = false;
							::_cursor.setState(this.GUI.Cursor.USE);
						}
					}
				}
			}
		}

		if (revert && ::_cursor.getState() != this.GUI.Cursor.DRAG)
		{
			::_cursor.setState(this.GUI.Cursor.DEFAULT);
		}
	}

	function updateKeybinding( keyCode, controlDown, altDown, shiftDown, newCommand, primary )
	{
		local vkeyCombo = [];

		if (controlDown)
		{
			vkeyCombo.append(this.Key.VK_CONTROL);
		}

		if (altDown)
		{
			vkeyCombo.append(this.Key.VK_ALT);
		}

		if (shiftDown)
		{
			vkeyCombo.append(this.Key.VK_SHIFT);
		}

		vkeyCombo.append(keyCode);
		local key = this.KeyHelper.keyBindText(keyCode, controlDown, altDown, shiftDown, true);

		if (key in this.mKeyBindings)
		{
			this.mKeyBindings[key].command = newCommand;
			this.mKeyBindings[key].primary = primary;
			this.mKeyBindings[key].vkCombo = vkeyCombo;
		}
		else
		{
			this.mKeyBindings[key] <- {
				command = newCommand,
				primary = primary,
				vkCombo = vkeyCombo
			};
		}

		if (newCommand >= this.QUICKBAR_COMMAND_START && newCommand <= this.QUICKBAR_COMMAND_END)
		{
			local command = this.Commands[newCommand];
			local quickbar = ::_quickBarManager.getQuickBar(command.data[0]);

			if (quickbar)
			{
				quickbar.updateDisplayedBinding(command.data[1], key);
			}
		}
		else if (newCommand == this.Commands.UNBOUNDED)
		{
			::_quickBarManager.removeBindingFromAllQuickbars(key);
		}

		switch(newCommand)
		{
		case this.Commands.MOVE_FORWARD:
			if (primary)
			{
				this.mForwardKey.primary = vkeyCombo;
			}
			else
			{
				this.mForwardKey.secondary = vkeyCombo;
			}

			break;

		case this.Commands.MOVE_BACKWARD:
			if (primary)
			{
				this.mBackwardKey.primary = vkeyCombo;
			}
			else
			{
				this.mForwardKey.secondary = vkeyCombo;
			}

			break;

		case this.Commands.STRAFE_LEFT:
			if (primary)
			{
				this.mStrafeLeftKey.primary = vkeyCombo;
			}
			else
			{
				this.mStrafeLeftKey.secondary = vkeyCombo;
			}

			break;

		case this.Commands.STRAFE_RIGHT:
			if (primary)
			{
				this.mStrafeRightKey.primary = vkeyCombo;
			}
			else
			{
				this.mStrafeRightKey.secondary = vkeyCombo;
			}

			break;

		case this.Commands.TURN_LEFT:
			if (primary)
			{
				this.mRotateLeftKey.primary = vkeyCombo;
			}
			else
			{
				this.mRotateLeftKey.secondary = vkeyCombo;
			}

			break;

		case this.Commands.TURN_RIGHT:
			if (primary)
			{
				this.mRotateRightKey.primary = vkeyCombo;
			}
			else
			{
				this.mRotateRightKey.secondary = vkeyCombo;
			}

			break;
		}
	}

	function unlockAndClearCursor()
	{
		::_cursor.setLocked(false);
		this.mRotating = false;
		this.mOrienting = false;
		this.mDragged = false;
		this.mRotatePressed = false;
		this.mOrientPressed = false;
	}

	function _backwardStart()
	{
		if (!this.isKeyComboDown(this.mForwardKey.primary) && !this.isKeyComboDown(this.mForwardKey.secondary))
		{
			this.mAutoRun = false;
			this._avatar.getController().onAvatarForwardStop();
			this._avatar.getController().onAvatarBackwardStart();
		}
	}

	function _backwardStop()
	{
		this._avatar.getController().onAvatarBackwardStop();
	}

	function _decreaseAvatarSpeed()
	{
		this._avatar.getController().onDecreaseAvatarSpeed();
	}

	function _findNextTarget( creatureList )
	{
		for( local i = 0; i < creatureList.len(); i++ )
		{
			local creature = creatureList[i];

			if (this.mCurrentTargetId == creature.getID())
			{
				local so;

				if (i == creatureList.len() - 1)
				{
					so = creatureList[0];
				}
				else
				{
					so = creatureList[i + 1];
				}

				this.mCurrentTargetId = so.getID();
				return so;
			}
		}

		this.mCurrentTargetId = 0;
		return null;
	}

	function _forwardStart()
	{
		if (!this.isKeyComboDown(this.mBackwardKey.primary) && !this.isKeyComboDown(this.mBackwardKey.secondary))
		{
			this.mAutoRun = false;
			this._avatar.getController().onAvatarForwardStart();
		}
	}

	function _forwardBackwardStop()
	{
		this._avatar.getController().onAvatarForwardBackwardStop();
	}

	function _forwardStop()
	{
		this._avatar.getController().onAvatarForwardStop();
	}

	function _increaseAvatarSpeed()
	{
		this._avatar.getController().onIncreaseAvatarSpeed();
	}

	function _leftStart()
	{
		if ((this.isKeyComboDown(this.mRotateLeftKey.primary) || this.isKeyComboDown(this.mRotateLeftKey.primary)) && this.mOrientPressed)
		{
			this._strafeLeftStart();
		}
		else if (!this.isKeyComboDown(this.mRotateRightKey.primary) && !this.isKeyComboDown(this.mRotateRightKey.secondary) && this.mOrienting == false && this.mMoving == false)
		{
			this._avatar.getController().onAvatarLeftStart();
		}
	}

	function _onOptionsMenuToggle()
	{
		this.Screens.toggle("OptionsScreen");
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
		local camRadius = ::_CameraObject.getNearClipDistance();
		local result = ::_scene.sweepSphere(2.0, avatarPos, avatarPos + dir * dist);

		if (result.distance >= 1.0)
		{
			local hits = ::_scene.rayQuery(avatarPos, dir, this.QueryFlags.FLOOR, true, true, 1, ::_avatar.getNode());

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
			dist * result.distance,
			this.Vector3()
		];
	}

	function _rotateStop()
	{
		this._avatar.getController().onAvatarRotateStop();
	}

	function _rightStart()
	{
		if ((this.isKeyComboDown(this.mRotateRightKey.primary) || this.isKeyComboDown(this.mRotateRightKey.secondary)) && this.mOrientPressed)
		{
			this._strafeRightStart();
		}
		else if (!this.isKeyComboDown(this.mRotateLeftKey.primary) && !this.isKeyComboDown(this.mRotateLeftKey.secondary) && this.mOrienting == false && this.mMoving == false)
		{
			this._avatar.getController().onAvatarRightStart();
		}
	}

	function _selectNextCreature()
	{
		local currentlySelectId = this.mCurrentTargetId;
		local creatureTable = ::_sceneObjectManager.getCreatures();
		local rawCreatureList = [];

		foreach( k, so in creatureTable )
		{
			if (so.isInRange() && ::_avatar != so && !so.isPropCreature() && !so.isDead() && !so.getMeta("quest_giver") && !so.getMeta("quest_ender") && !so.getMeta("copper_shopkeeper") && !so.getMeta("credit_shopkeeper") && !so.getMeta("vault") && so.getMeta("credit_shop") == null && !so.isPlayer() && !so.hasStatusEffect(this.StatusEffects.UNATTACKABLE) && !so.hasStatusEffect(this.StatusEffects.INVINCIBLE))
			{
				local distanceToTarget = this.Math.DetermineDistanceBetweenTwoPoints(::_avatar.getPosition(), so.getPosition());
				rawCreatureList.append({
					sceneObject = so,
					distance = distanceToTarget
				});
			}
		}

		this.Util.bubbleSort(rawCreatureList, this.sortTargetDistances);
		local finalCreatureList = [];
		local tempBehindCreatureList = [];

		foreach( creature in rawCreatureList )
		{
			local myHeading = this.Math.ConvertRadToVector(::_avatar.getRotation());
			myHeading.normalize();
			local vecToCreature = creature.sceneObject.getPosition() - ::_avatar.getPosition();
			vecToCreature.normalize();

			if (myHeading.dot(vecToCreature) > 0.2)
			{
				finalCreatureList.append(creature.sceneObject);
			}
			else if (creature.distance < 50.0)
			{
				tempBehindCreatureList.append(creature.sceneObject);
			}
		}

		foreach( creature in tempBehindCreatureList )
		{
			finalCreatureList.append(creature);
		}

		local creature;

		if (finalCreatureList.len() > 0)
		{
			if (::_avatar.getResetTabTarget() || !this.mCurrentTargetId)
			{
				creature = finalCreatureList[0];
				this.mCurrentTargetId = creature.getID();
			}
			else
			{
				creature = this._findNextTarget(finalCreatureList);

				if (!creature && finalCreatureList.len() > 0)
				{
					creature = finalCreatureList[0];
					this.mCurrentTargetId = creature.getID();
				}
			}

			if (::_avatar.getResetTabTarget() || currentlySelectId != this.mCurrentTargetId)
			{
				::_avatar.setTargetObject(creature);
				::_Connection.sendSelectTarget(this.mCurrentTargetId);
			}
		}

		::_avatar.setResetTabTarget(false);
	}

	function _showItemAppearanceEditor()
	{
		if (this.Util.hasPermission("tweakScreens") == false)
		{
			return;
		}

		this.Screens.toggle("ItemAppearanceTweak");
	}

	function _strafeLeftStart()
	{
		if (!this.isKeyComboDown(this.mStrafeRightKey.primary) && !this.isKeyComboDown(this.mStrafeRightKey.secondary))
		{
			this._avatar.getController().onAvatarStrafeLeftStart();
		}
	}

	function _strafeLeftRightStop()
	{
		this._avatar.getController().onAvatarLeftRightStop();
	}

	function _strafeLeftStop()
	{
		this._avatar.getController().onAvatarStrafeLeftStop();
	}

	function _strafeRightStart()
	{
		if (!this.isKeyComboDown(this.mStrafeLeftKey.primary) && !this.isKeyComboDown(this.mStrafeLeftKey.secondary))
		{
			this._avatar.getController().onAvatarStrafeRightStart();
		}
	}

	function _strafeRightStop()
	{
		this._avatar.getController().onAvatarStrafeRightStop();
	}

	function _tostring()
	{
		return "PlayTool";
	}

	function _toggleWeapons()
	{
		::EvalCommand("/toggleWeapons");
	}

}

