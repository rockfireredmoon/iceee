::ClothingDef["Armor-Light-Mid3"] <- {
	palette = [
		"rainbow",
		"rainbow",
		"rainbow",
		"rainbow",
		"rainbow",
		"rainbow"
	],
	colors = [
		"54595c",
		"766752",
		"4e4536",
		"383c3f",
		"a19682",
		"846e57"
	],
	regions = {
		arms = "Core",
		collar = "Accessories",
		boots = "Accessories",
		gloves = "Core",
		chest = "Core",
		leggings = "Core",
		belt = "Accessories"
	}
};
::AttachableDef["Armor-Light-Mid3-Hat"] <- delegate this.AttachableTemplates["Item.Hat"] : {
	palette = [
		"rainbow",
		"rainbow",
		"rainbow"
	],
	colors = [
		"a19682",
		"846e56",
		"766752"
	]
};
