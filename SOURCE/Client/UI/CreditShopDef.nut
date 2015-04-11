this.CreditScreenTypes <- {
	DEFAULT = 0,
	PREVIEW = 1,
	NAME = 2
};
this.CreditSelectType <- {
	CONSUMABLES = 0,
	CHARMS = 1,
	ARMOR = 2,
	BAGS = 3,
	RECIPES = 4,
	NAME_CHANGE = 5,
	NEW = 6
};
this.CreditSelectInfo <- {
	[this.CreditSelectType.CONSUMABLES] = {
		screenType = this.CreditScreenTypes.DEFAULT,
		imageName = "Consumables",
		name = "Consumables",
		key = "CONSUMABLES",
		items = [
			925,
			926,
			927,
			928,
			929,
			930,
			931,
			932,
			933,
			934,
			935
		]
	},
	[this.CreditSelectType.CHARMS] = {
		screenType = this.CreditScreenTypes.DEFAULT,
		imageName = "Charms",
		name = "Charms",
		key = "CHARMS",
		items = [
			937,
			938,
			939,
			940,
			941,
			942,
			943,
			944,
			945,
			946,
			947
		]
	},
	[this.CreditSelectType.ARMOR] = {
		screenType = this.CreditScreenTypes.PREVIEW,
		imageName = "Armor",
		name = "Armor Looks",
		key = "ARMOR",
		items = [
			953,
			954,
			955,
			956,
			957,
			958,
			959
		]
	},
	[this.CreditSelectType.BAGS] = {
		screenType = this.CreditScreenTypes.DEFAULT,
		imageName = "Bags",
		name = "Bags",
		key = "BAGS",
		items = [
			937,
			938
		]
	},
	[this.CreditSelectType.RECIPES] = {
		screenType = this.CreditScreenTypes.DEFAULT,
		imageName = "Recipes",
		name = "Miscellaneous",
		key = "RECIPES",
		items = [
			954
		]
	},
	[this.CreditSelectType.NAME_CHANGE] = {
		screenType = this.CreditScreenTypes.NAME,
		imageName = "NameText",
		name = "Change Last Name",
		key = "CHANGE_NAME"
	},
	[this.CreditSelectType.NEW] = {
		screenType = this.CreditScreenTypes.DEFAULT,
		imageName = "New",
		name = "New Items",
		key = "NEW",
		items = [
			950,
			951,
			952
		]
	}
};
