this.AbilityScreenFlags <- {
	OWN = 1,
	CAN_PURCHASE = 2,
	PASSIVE = 4
};
this.AbilityScreenHighlightType <- {
	OWN = 0,
	[0] = {
		name = "None",
		r = 0,
		g = 0,
		b = 0
	},
	OWN_CAN_PURCHASE_NEXT = 1,
	[1] = {
		name = "Green",
		r = 0,
		g = 1.0,
		b = 0
	},
	CAN_PURCHASE = 2,
	[2] = {
		name = "Orange",
		r = 1.0,
		g = 0.63999999,
		b = 0
	},
	CAN_NEVER_PURCHASE = 3,
	[3] = {
		name = "Red",
		r = 1.0,
		g = 0,
		b = 0
	},
	MAX = 4
};
this.AbilityTreeType <- {
	KNIGHT = 0,
	[0] = {
		name = "Knight",
		profession = "K",
		indexPage = 1
	},
	ROGUE = 1,
	[1] = {
		name = "Rogue",
		profession = "R",
		indexPage = 2
	},
	MAGE = 2,
	[2] = {
		name = "Mage",
		profession = "M",
		indexPage = 3
	},
	DRUID = 3,
	[3] = {
		name = "Druid",
		profession = "D",
		indexPage = 4
	},
	RESTORATION = 4,
	[4] = {
		name = "Restoration",
		indexPage = 9
	},
	TRAVEL = 5,
	[5] = {
		name = "Travel",
		indexPage = 10
	},
	WEAPONS = 6,
	[6] = {
		name = "Weapons",
		indexPage = 11
	},
	PROTECTION = 7,
	[7] = {
		name = "Protection",
		indexPage = 12
	}
};
this.AbilityTierType <- {
	TIER_ONE = 1,
	[1] = {
		minLevel = 0,
		maxLevel = 5,
		labelText = " TIER 1\nLevel 1"
	},
	TIER_TWO = 2,
	[2] = {
		minLevel = 6,
		maxLevel = 19,
		labelText = " TIER 2\nLevel 6"
	},
	TIER_THREE = 3,
	[3] = {
		minLevel = 20,
		maxLevel = 29,
		labelText = " TIER 3\nLevel 20"
	},
	TIER_FOUR = 4,
	[4] = {
		minLevel = 30,
		maxLevel = 39,
		labelText = " TIER 4\nLevel 30"
	},
	TIER_FIVE = 5,
	[5] = {
		minLevel = 40,
		maxLevel = 49,
		labelText = " TIER 5\nLevel 40"
	},
	TIER_SIX = 6,
	[6] = {
		minLevel = 50,
		maxLevel = 59,
		labelText = " TIER 6\nLevel 50"
	},
	MAX = 6
};
this.AbilityCategoryType <- {
	corek = 0,
	corer = 1,
	corem = 2,
	cored = 3,
	crossk = 0,
	crossr = 1,
	crossm = 2,
	crossd = 3,
	restoration = 8,
	travel = 9,
	weapon = 10,
	protection = 11
};
