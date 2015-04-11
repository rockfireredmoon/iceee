this.require("ServerConstants");
this.require("Combat/CombatConstants");
this.AssemblerData <- {};
this.Colors <- {
	Steel = "5a6268",
	Bronze = "835336",
	["Blue Steel"] = "abc1c9",
	["Dark Steel"] = "362f2d",
	["Wood Orange"] = "f7941d",
	Maroon = "561f20",
	["Soft Grey"] = "ccc1b2",
	["Medium Grey"] = "bbaca1",
	["Medium Dark Grey"] = "7a6f6d",
	["Purple-Grey"] = "8d86a1",
	["Rich Orange"] = "913e14",
	["Chocolate Brown"] = "2c1e17",
	["Fawn Brown"] = "65492c",
	["Orange Brown"] = "784925",
	["Yellow Brown"] = "8e6938",
	Flesh = "ac735a",
	["Dusty Brown"] = "785f49",
	["Grey Brown"] = "fff1df",
	["Red Orange"] = "b13411",
	["Deep Blue"] = "002157",
	["Light Green"] = "76FF7A",
	["Bright Orange"] = "f26522",
	["Bright Red"] = "b90007",
	["Bright Blue"] = "0054a6",
	["Light Blue"] = "00adef",
	["Yellow Green"] = "598527",
	["Dark Green"] = "018509",
	["Bright Green"] = "00FF00",
	["Muted Red"] = "cd4c4e",
	["Bright Purple"] = "5749E4",
	["GUI Gold"] = "EDED8F",
	Violet = "1b1464",
	black = "000000",
	white = "FFFFFF",
	grey20 = "333333",
	grey40 = "666666",
	grey60 = "999999",
	grey80 = "CCCCCC",
	red = "FF0000",
	lime = "00FF00",
	blue = "0000FF",
	yellow = "FFFF00",
	cyan = "00FFFF",
	fuschia = "FF00FF",
	["dark orange"] = "FF8C00",
	maroon = "800000",
	green = "008000",
	navy = "000080",
	olive = "808000",
	teal = "008080",
	purple = "800080",
	silver = "C0C0C0",
	peach = "FFDAB9",
	turquoise = "48D1CC",
	["ugly brown"] = "A8760A",
	orange = "FFA500",
	sky = "436EEE",
	mint = "5DFC0A",
	lavender = "8066CC",
	purple1 = "912CEE",
	purple2 = "ba71ff",
	coral = "ffb499",
	["Item Grey"] = "858585",
	["Item White"] = "fefefe",
	["Item Green"] = "1ddd2a",
	["Item Blue"] = "3b93ff",
	["Item Purple"] = "c24dff",
	["Item Yellow"] = "fbec00",
	["Item Orange"] = "e98d20"
};
this.BackgroundImages <- {
	AQUA = "Icon-32-BG-Aqua.png",
	BLACK = "Icon-32-BG-Black.png",
	BLUE = "Icon-32-BG-Blue.png",
	CYAN = "Icon-32-BG-Cyan.png",
	GREEN = "Icon-32-BG-Green.png",
	GREY = "Icon-32-BG-Grey.png",
	PURPLE = "Icon-32-BG-Purple.png",
	PINK = "Icon-32-BG-Pink.png",
	RED = "Icon-32-BG-Red.png",
	YELLOW = "Icon-32-BG-Yellow.png"
};
this.QueueType <- {
	QUEUE_UNKNOWN = 0,
	QUEUE_LOGIN = 1,
	QUEUE_SHARD = 2,
	QUEUE_INSTANCE = 3
};
local name;
local hex;

foreach( name, hex in this.Colors )
{
	this.Color.names[name] <- hex;
}

this.QueryFlags <- {
	ANYTHING = 1,
	ANY = 1,
	CAMERA = 1,
	FLOOR = 2,
	BLOCKING = 4,
	KEEP_ON_FLOOR = 8,
	LIGHT_OCCLUDER = 16,
	SOUND_OCCLUDER = 32,
	ENVIRONMENT_OCCLUDER = 64,
	SHADOW_CASTER = 128,
	MANIPULATOR = 256,
	CLUTTER_OCCLUDER = 512,
	VISUAL_FLOOR = 1024
};
this.VisibilityFlags <- {
	ANYTHING = 1,
	ANY = 1,
	COLLISION = 2,
	HELPER_GEOMETRY = 4,
	SCENERY = 16,
	CREATURE = 32,
	ATTACHMENT = 64,
	WATER = 128,
	FEEDBACK = 256,
	PROPS = 512,
	PARTY = 1024,
	PROP_NO_MINIMAP = 2048,
	LIGHT_GROUP_0 = 16777216,
	LIGHT_GROUP_1 = 33554432,
	LIGHT_GROUP_2 = 67108864,
	LIGHT_GROUP_3 = 134217728
};
this.PSystemFlags <- {
	SIZE = 1,
	TTL = 2,
	VELOCITY = 4
};
this.VisibilityFlags.LIGHT_GROUP_ANY <- this.VisibilityFlags.LIGHT_GROUP_0 | this.VisibilityFlags.LIGHT_GROUP_1 | this.VisibilityFlags.LIGHT_GROUP_2 | this.VisibilityFlags.LIGHT_GROUP_3;
this.VisibilityFlags.DEFAULT <- ~(this.VisibilityFlags.COLLISION | this.VisibilityFlags.HELPER_GEOMETRY | this.VisibilityFlags.LIGHT_GROUP_0 | this.VisibilityFlags.LIGHT_GROUP_1 | this.VisibilityFlags.LIGHT_GROUP_2 | this.VisibilityFlags.LIGHT_GROUP_3);
this.Races <- {
	a = "Anura",
	v = "Atavian",
	d = "Bandicoon",
	b = "Bounder",
	r = "Broccan",
	c = "Caprican",
	w = "Clockwork",
	p = "Cyclops",
	e = "Daemon",
	q = "Dryad",
	g = "Fangren",
	f = "Feline",
	x = "Foxen",
	h = "Hart",
	o = "Longtail",
	l = "Lisian",
	n = "Noctari",
	t = "Taurian",
	s = "Troblin",
	k = "Tusken",
	u = "Ursine",
	y = "Yeti"
};
this.RaceOrder <- [
	"a",
	"v",
	"d",
	"b",
	"r",
	"c",
	"w",
	"p",
	"e",
	"q",
	"g",
	"f",
	"x",
	"h",
	"l",
	"o",
	"n",
	"t",
	"s",
	"k",
	"u",
	"y"
];
this.RaceData <- {
	a = {
		prettyName = "Anura"
	},
	v = {
		prettyName = "Atavian"
	},
	d = {
		prettyName = "Bandicoon"
	},
	b = {
		prettyName = "Bounder"
	},
	r = {
		prettyName = "Broccan"
	},
	c = {
		prettyName = "Caprican"
	},
	w = {
		prettyName = "Clockwork"
	},
	p = {
		prettyName = "Cyclops"
	},
	e = {
		prettyName = "Daemon"
	},
	q = {
		prettyName = "Dryad"
	},
	g = {
		prettyName = "Fangren"
	},
	f = {
		prettyName = "Feline"
	},
	x = {
		prettyName = "Foxen"
	},
	h = {
		prettyName = "Hart"
	},
	o = {
		prettyName = "Longtail"
	},
	l = {
		prettyName = "Lisian"
	},
	n = {
		prettyName = "Noctari"
	},
	t = {
		prettyName = "Taurian"
	},
	s = {
		prettyName = "Grumkin"
	},
	k = {
		prettyName = "Tusken"
	},
	u = {
		prettyName = "Ursine"
	},
	y = {
		prettyName = "Yeti"
	}
};
this.ActionItemCategory <- {
	INVALID_CATEGORY = -1,
	POTION = 0,
	SPELL = 1,
	ATTACK = 2
};
this.ConMessages <- {
	Grey = {
		channel = "conGrey",
		message = "Why don\'t you pick on someone your own size?"
	},
	["Dark Green"] = {
		channel = "conDarkGreen",
		message = "Honestly! Killing me is like shooting fish in a barrel."
	},
	["Light Green"] = {
		channel = "conLightGreen",
		message = "Not really much of a challenge, but maybe you will learn something by killing me."
	},
	["Bright Green"] = {
		channel = "conBrightGreen",
		message = "Relatively easy fight. Victory should be assured."
	},
	White = {
		channel = "conWhite",
		message = "I am a good match for your skill level and training. You should win handily."
	},
	Blue = {
		channel = "conBlue",
		message = "Slight challenge. But you should still win this fight."
	},
	Yellow = {
		channel = "conYellow",
		message = "Challenging fight. Victory is no longer assured unless you are prepared."
	},
	Orange = {
		channel = "conOrange",
		message = "Whoa! Slow down big fella. If you want to win you better make sure your buffs are in order."
	},
	Red = {
		channel = "conRed",
		message = "Do you have any friends? Maybe you should call one to give you a hand."
	},
	Purple = {
		channel = "conPurple",
		message = "Well, I suppose today is as good a day to die as any other."
	},
	["Bright Purple"] = {
		channel = "conBrightPurple",
		message = "I will pound you into the ground like a nail."
	}
};
this.LevelRequirements <- {
	[1] = 500,
	[2] = 700,
	[3] = 1100,
	[4] = 1700,
	[5] = 2500,
	[6] = 3700,
	[7] = 5500,
	[8] = 8200,
	[9] = 12100,
	[10] = 13200,
	[11] = 14400,
	[12] = 15700,
	[13] = 17200,
	[14] = 18700,
	[15] = 20400,
	[16] = 22300,
	[17] = 24400,
	[18] = 26600,
	[19] = 29000,
	[20] = 31700,
	[21] = 34600,
	[22] = 37700,
	[23] = 41200,
	[24] = 44900,
	[25] = 49000,
	[26] = 53500,
	[27] = 58400,
	[28] = 63700,
	[29] = 69600,
	[30] = 75900,
	[31] = 82900,
	[32] = 90400,
	[33] = 98700,
	[34] = 107700,
	[35] = 117600,
	[36] = 128300,
	[37] = 140000,
	[38] = 152800,
	[39] = 166800,
	[40] = 182100,
	[41] = 198700,
	[42] = 216900,
	[43] = 236700,
	[44] = 258300,
	[45] = 281900,
	[46] = 307700,
	[47] = 335800,
	[48] = 366500,
	[49] = 400000,
	[50] = 404500,
	[51] = 409000,
	[52] = 413600,
	[53] = 418300,
	[54] = 422900,
	[55] = 427700,
	[56] = 432500,
	[57] = 437300,
	[58] = 442300,
	[59] = 447200,
	[60] = 452200,
	[61] = 457300,
	[62] = 462400,
	[63] = 467600,
	[64] = 472900,
	[65] = 478200,
	[66] = 483500,
	[67] = 489000,
	[68] = 494500,
	[69] = 500000,
	[70] = 504800
};
this.ItemTypeNames <- {
	[this.ItemType.UNKNOWN] = this.TXT("Unknown"),
	[this.ItemType.SYSTEM] = this.TXT("System"),
	[this.ItemType.WEAPON] = this.TXT("Weapon"),
	[this.ItemType.ARMOR] = this.TXT("Armor"),
	[this.ItemType.CHARM] = this.TXT("Charm"),
	[this.ItemType.CONSUMABLE] = this.TXT("Consumable"),
	[this.ItemType.CONTAINER] = this.TXT("Container"),
	[this.ItemType.BASIC] = this.TXT("Basic"),
	[this.ItemType.SPECIAL] = this.TXT("Special"),
	[this.ItemType.QUEST] = this.TXT("Quest"),
	[this.ItemType.RECIPE] = this.TXT("Recipe")
};
this.WeaponTypeClassRestrictions <- {
	[this.WeaponType.NONE] = {
		none = false,
		knight = false,
		rogue = false,
		mage = false,
		druid = false
	},
	[this.WeaponType.SMALL] = {
		none = false,
		knight = false,
		rogue = true,
		mage = true,
		druid = false
	},
	[this.WeaponType.ONE_HAND] = {
		none = false,
		knight = true,
		rogue = true,
		mage = false,
		druid = true
	},
	[this.WeaponType.TWO_HAND] = {
		none = false,
		knight = true,
		rogue = false,
		mage = false,
		druid = false
	},
	[this.WeaponType.POLE] = {
		none = false,
		knight = false,
		rogue = false,
		mage = false,
		druid = true
	},
	[this.WeaponType.WAND] = {
		none = false,
		knight = false,
		rogue = false,
		mage = true,
		druid = false
	},
	[this.WeaponType.BOW] = {
		none = false,
		knight = true,
		rogue = false,
		mage = false,
		druid = true
	},
	[this.WeaponType.THROWN] = {
		none = false,
		knight = false,
		rogue = true,
		mage = false,
		druid = false
	},
	[this.WeaponType.ARCANE_TOTEM] = {
		none = false,
		knight = true,
		rogue = true,
		mage = true,
		druid = true
	}
};
this.WeaponTypeNames <- {
	[this.WeaponType.NONE] = this.TXT("None"),
	[this.WeaponType.SMALL] = this.TXT("Small"),
	[this.WeaponType.ONE_HAND] = this.TXT("One Hand"),
	[this.WeaponType.TWO_HAND] = this.TXT("Two Hand"),
	[this.WeaponType.POLE] = this.TXT("Pole"),
	[this.WeaponType.WAND] = this.TXT("Wand"),
	[this.WeaponType.BOW] = this.TXT("Bow"),
	[this.WeaponType.THROWN] = this.TXT("Throw"),
	[this.WeaponType.ARCANE_TOTEM] = this.TXT("Talisman")
};
this.ArmorTypeNames <- {
	[this.ArmorType.NONE] = this.TXT("None"),
	[this.ArmorType.CLOTH] = this.TXT("Cloth"),
	[this.ArmorType.LIGHT] = this.TXT("Light"),
	[this.ArmorType.MEDIUM] = this.TXT("Medium"),
	[this.ArmorType.HEAVY] = this.TXT("Heavy"),
	[this.ArmorType.SHIELD] = this.TXT("Shield")
};
this.ArmorTypeClassRestrictions <- {
	[this.ArmorType.NONE] = {
		none = false,
		knight = false,
		rogue = false,
		mage = false,
		druid = false
	},
	[this.ArmorType.CLOTH] = {
		none = false,
		knight = true,
		rogue = true,
		mage = true,
		druid = true
	},
	[this.ArmorType.LIGHT] = {
		none = false,
		knight = true,
		rogue = true,
		mage = true,
		druid = true
	},
	[this.ArmorType.MEDIUM] = {
		none = false,
		knight = true,
		rogue = true,
		mage = true,
		druid = true
	},
	[this.ArmorType.HEAVY] = {
		none = false,
		knight = true,
		rogue = true,
		mage = true,
		druid = true
	},
	[this.ArmorType.SHIELD] = {
		none = false,
		knight = true,
		rogue = true,
		mage = true,
		druid = true
	}
};
this.Genders <- {
	m = "Male",
	f = "Female"
};
this.BodyTypes <- {
	n = "Normal",
	m = "Muscular",
	r = "Rotund"
};
this.ClothingSlots <- [
	"chest",
	"leggings",
	"belt",
	"boots",
	"arms",
	"gloves",
	"collar"
];
this.ChannelColors <- {
	Default = "ffffff",
	Err = "ff0000",
	Sys = "fff2ba",
	Info = "00ff00",
	s = "ffffff",
	emote = "bc8f8f",
	party = "7dd0ff",
	mci = "ff0033",
	mco = "ffffff",
	oci = "8B3A3A",
	oco = "808080",
	clan = "89ff7d",
	friends = this.Colors["Light Blue"],
	conGrey = this.Colors.grey60,
	conDarkGreen = this.Colors["Dark Green"],
	conLightGreen = this.Colors["Light Green"],
	conBrightGreen = this.Colors["Bright Green"],
	conWhite = this.Colors.white,
	conBlue = this.Colors.sky,
	conYellow = this.Colors.yellow,
	conOrange = this.Colors["Bright Orange"],
	conRed = this.Colors.red,
	conPurple = this.Colors.purple,
	conBrightPurple = this.Colors["Bright Purple"],
	["external.wink"] = "ffffff",
	["*SysChat"] = "fff2ba"
};
this.ChannelNoFilter <- {
	["sys/info"] = "",
	["err/error"] = "",
	info = "",
	error = "",
	friends = "",
	mci = "",
	mco = "",
	oci = "",
	oco = "",
	conGrey = "",
	conDarkGreen = "",
	conLightGreen = "",
	conBrightGreen = "",
	conWhite = "",
	conBlue = "",
	conYellow = "",
	conOrange = "",
	conRed = "",
	conPurple = "",
	conBrightPurple = ""
};
this.ChannelScope <- {
	s = "Say",
	emote = "Emote",
	city = "City",
	clan = "Clan",
	co = "Clan Officer",
	party = "Party",
	friends = "Friend Notifications",
	mci = "My Combat Incoming",
	mco = "My Combat Outgoing",
	oci = "Other Player Combat Incoming",
	oco = "Other Player Combat Outgoing",
	["*SysChat"] = "System"
};
this.ChannelBracket <- {
	s = "",
	party = "[Party]",
	emote = "[Emote]",
	friends = "[Friends]",
	clan = "[Clan]",
	mci = "[My incoming damage]",
	mco = "[My outgoing damage]",
	oci = "[OtherPlayer incoming damage]",
	oco = "[OtherPlayer outgoing damage]",
	["*SysChat"] = "[SysChat]"
};
this.ChannelHeaders <- {
	Default = "Not Registered",
	Err = "Error",
	Sys = "System",
	Info = "Info",
	s = "says",
	party = "says",
	clan = "says",
	conGrey = "Consider",
	conDarkGreen = "Consider",
	conLightGreen = "Consider",
	conBrightGreen = "Consider",
	conWhite = "Consider",
	conBlue = "Consider",
	conYellow = "Consider",
	conOrange = "Consider",
	conRed = "Consider",
	conPurple = "Consider",
	conBrightPurple = "Consider",
	["external.wink"] = "Wink",
	["*SysChat"] = "says"
};
this.OverheadNames <- {
	a = "Always",
	s = "Only when selected",
	n = "Never"
};
this.ChatWindowSizes <- {
	[1] = "Small",
	[2] = "Medium",
	[3] = "Large"
};
this.BASE_AVATAR_HEIGHT <- 25;
this.PI = 3.1415927;
::BUY_VALUE_MULTIPLIER <- 1.25;
this.MAX_HEROISM <- 1000;
this.USE_OLD_SCREEN <- true;
this.MAX_USE_DISTANCE <- 50.0;
this.MAX_USE_DISTANCE_SQ <- this.MAX_USE_DISTANCE * this.MAX_USE_DISTANCE;
this.AUTO_FOLLOW_DISTANCE <- 25.0;
