this.require("LoadGateTriggers");
this.LoadGate <- {};
this.LoadGate.Gates <- {
	Bootstrap = [
		"Catalogs",
		"GUI"
	],
	Core = [
		"#Bootstrap",
		"Manipulator",
		"Debug",
		"Materials",
		"Effects",
		"Lights",
		"#Login",
		"Sound-Interface"
	],
	Races = [],
	Icons = [
		"Icons-Abilities-32",
		"Icons-Animal-32",
		"Icons-Armor-32",
		"Icons-BG-32",
		"Icons-Containers-32",
		"Icons-Crafting-32",
		"Icons-Food-32",
		"Icons-Interface",
		"Icons-Jewelry-32",
		"Icons-Library-32",
		"Icons-Magic-32",
		"Icons-Minerals-32",
		"Icons-Misc-32",
		"Icons-Pets-32",
		"Icons-Plants-32",
		"Icons-Potions-32",
		"Icons-Unsorted",
		"Icons-Weapons-32"
	],
	BasicBiped = [
		"Biped-Male",
		"Biped-Female",
		"Biped-Male-Robed",
		"Biped-Female-Robed",
		"Biped-Male-Rotund",
		"Biped-Male-Rotund-Robed",
		"Biped-Female-Rotund",
		"Biped-Female-Rotund-Robed",
		"Biped-Male-Muscular",
		"Biped-Male-Muscular-Robed",
		"Biped-Female-Muscular",
		"Biped-Female-Muscular-Robed"
	],
	Login = [
		"#Core"
	],
	All = [],
	CharSelection = [
		"#Core",
		"#BasicBiped",
		"Environments",
		"Prop-CharSelect_BG",
		"Icons-Unsorted",
		"Icons-BG-32",
		"Armor-CC-Clothing1",
		"Armor-CC-Druid",
		"Armor-CC-Mage",
		"Armor-CC-Sneak",
		"Armor-CC-Warrior",
		"Item-1hSword-Basic5",
		"Item-2hAxe-Medium1",
		"Item-Bow-Basic4",
		"Item-Dagger-Basic1",
		"Item-Dagger-Basic6",
		"Item-Dagger-Medium5",
		"Item-Fireball",
		"Item-Shield-Basic3",
		"Item-Staff-Medium3",
		"Item-Wand-Basic1",
		"Item-Wand-High1"
	],
	CharCreation = [
		"#Races"
	],
	CreditShopPreviews = [
		"Preview-Armor"
	],
	PlayStage1_Dep = [
		"#Icons",
		"#Core",
		"#BasicBiped",
		"#Races",
		"#Map",
		"#Clutter",
		"#Sound1",
		"Music-Newb2",
		"Music-CheeZeeCave",
		"Terrain-Common",
		"Item-Fireball",
		"Biped-Anim-Combat",
		"Biped-Anim-Emote"
	],
	PlayStage1 = [
		"#PlayStage1_Dep",
		"Maps-Bastion",
		"ATS-Ruins",
		"Bldg-Bastion1",
		"Bldg-Bastion2",
		"Bldg-Ruins",
		"CL-Bastion_Woods",
		"CL-Battlefield",
		"CL-Junk",
		"CL-Rack",
		"CL-Training_Post1",
		"CL-Training_Post2",
		"CL-Waterfall2",
		"Manipulator",
		"Prop-Accessories1",
		"Prop-Anubian_Camp",
		"Prop-Anubian_Shipwreck",
		"Prop-Battlefield1",
		"Prop-Birds",
		"Prop-BonesTex_A",
		"Prop-Books",
		"Prop-Boulder",
		"Prop-Boulder_Arid",
		"Prop-Burlap_Sack",
		"Prop-Burnt_Pines",
		"Prop-Bush_Arid",
		"Prop-Bush_AridDead",
		"Prop-Campfires",
		"Prop-Cart1",
		"Prop-Castle1",
		"Prop-CharSelect_BG",
		"Prop-Chunks",
		"Prop-Containers1",
		"Prop-Crypt_Props",
		"Prop-Desk_Set",
		"Prop-Dungeon_Accents1",
		"Prop-Fences",
		"Prop-Flagon",
		"Prop-Flora",
		"Prop-Forest_Tree",
		"Prop-GlassBottles",
		"Prop-Grey_Rock",
		"Prop-Ground_Accents1",
		"Prop-Ground_Accents2",
		"Prop-Interact1",
		"Prop-Iron_Fences",
		"Prop-Lamp_Posts1",
		"Prop-Marker1",
		"Prop-Mats",
		"Prop-Mine_Supports",
		"Prop-Nordic_Props",
		"Prop-Normal_Bushes",
		"Prop-Palm-Tree",
		"Prop-Pan",
		"Prop-Rope_Coil",
		"Prop-Sanctuary_Stone",
		"Prop-Skulls",
		"Prop-Swineland_Junk",
		"Prop-Target1",
		"Prop-Tents1",
		"Prop-Underground_Roots",
		"Prop-Wagon1",
		"Prop-WaterFall",
		"Prop-Weapons",
		"Prop-Wooden_Furniture",
		"Prop-Wooden_Wall2",
		"Prop-ModAddons1",
		"Terrain-Starting_Zone2",
		"Terrain-Starting_Zone2_x4y4",
		"Terrain-Starting_Zone2_x4y5",
		"Terrain-Starting_Zone2_x4y6",
		"Terrain-Starting_Zone2_x4y7",
		"Terrain-Starting_Zone2_x5y4",
		"Terrain-Starting_Zone2_x5y5",
		"Terrain-Starting_Zone2_x5y6",
		"Terrain-Starting_Zone2_x5y7",
		"Terrain-Starting_Zone2_x6y4",
		"Terrain-Starting_Zone2_x6y5",
		"Terrain-Starting_Zone2_x6y6",
		"Terrain-Starting_Zone2_x6y7",
		"Terrain-Starting_Zone2_x7y4",
		"Terrain-Starting_Zone2_x7y5",
		"Terrain-Starting_Zone2_x7y6",
		"Terrain-Starting_Zone2_x7y7",
		"Armor-Heavy-Epic1",
		"Armor-Heavy-Epic2",
		"Armor-Heavy-Epic3",
		"Armor-Heavy-Epic4",
		"Armor-Heavy-Epic6",
		"Horde-Anubian",
		"Item-1hSword-Basic3",
		"Item-1hSword-Basic5",
		"Item-1hSword-Epic2",
		"Item-1hSword-Medium5",
		"Item-1hThrown-Dart1",
		"Item-2hAxe-Medium1",
		"Item-2hAxe-Medium3",
		"Item-2hAxe-Medium4",
		"Item-2hSword-Medium1",
		"Item-2hSword-Medium2",
		"Item-Bow-Basic4",
		"Item-Bow-Basic5",
		"Item-Bow-High1",
		"Item-Dagger-Basic1",
		"Item-Dagger-Basic6",
		"Item-Dagger-Medium5",
		"Item-Shield-Basic3",
		"Item-Spear-High1",
		"Item-Staff-Medium3",
		"Item-Wand-Basic1",
		"Item-Wand-High1",
		"Prop-Loot1",
		"Armor-Cloth-Epic1",
		"Armor-Cloth-Town_Crier",
		"Armor-Light-Epic1",
		"Armor-Light-Epic3",
		"Horde-Anubian_Catapult",
		"Horde-Anubian_Moon_Warrior",
		"Item-1hAxe-Med4",
		"Item-1hSword-Basic1",
		"Item-1hSword-Basic12",
		"Item-1hSword-Medium1",
		"Item-1hSword-Medium3",
		"Item-2hSword-Medium4",
		"Item-Particle-Elemental_Air_Base2",
		"Item-Shield-Medium5",
		"Item-Shield-Medium6"
	],
	PlayStage2_Dep = [
		"#PlayStage1_Dep",
		"#Sound2"
	],
	PlayStage2 = [
		"#PlayStage2_Dep",
		"Maps-Corsica",
		"ATS-Spanish",
		"ATS-Wooden",
		"Bldg-Corsica",
		"Bldg-Desert",
		"Bldg-Spanish-House1",
		"Bldg-Urban",
		"CL-Fence",
		"CL-Forest",
		"CL-Hills",
		"CL-Plants",
		"Prop-Acacia_Tree",
		"Prop-Alchemy1",
		"Prop-Aspen_Forest_Tree",
		"Prop-Autumn_Hill_Tree",
		"Prop-Autumn_Tree",
		"Prop-Bar",
		"Prop-Bookcases",
		"Prop-Braziers",
		"Prop-Bridge_a",
		"Prop-Bush-Burnt",
		"Prop-Bush-Swamp",
		"Prop-Butter_Churn",
		"Prop-Cactus",
		"Prop-Clothesline",
		"Prop-Cooking",
		"Prop-Dead-Forest-Tree",
		"Prop-Desert_Grass",
		"Prop-Fat_Bottle",
		"Prop-Fence-Stone",
		"Prop-Fence-Stone_Blue",
		"Prop-Fern1",
		"Prop-Gardens",
		"Prop-Giant_Forest_Tree",
		"Prop-Gourd",
		"Prop-Grasses1",
		"Prop-Haystack",
		"Prop-Hill_Tree",
		"Prop-Iron-Candles",
		"Prop-Log1",
		"Prop-Low_Boxes",
		"Prop-Mossy_Rock",
		"Prop-Mounted",
		"Prop-Mushrooms1",
		"Prop-Paintings1",
		"Prop-Pie",
		"Prop-Pine_Forest_Accessories",
		"Prop-Pine_Forest_Tree",
		"Prop-Pine-Sick_Tree",
		"Prop-Pumpkin",
		"Prop-Scarecrow",
		"Prop-Sick-Forest_Tree",
		"Prop-Smithing1",
		"Prop-Spider_Cocoon",
		"Prop-Spider_Eggsack",
		"Prop-Spider_Webs",
		"Prop-Tailor1",
		"Prop-Tree_Accessories1",
		"Prop-Tree_Platforms",
		"Prop-Treehouse_Support",
		"Prop-Trees",
		"Prop-Wall_Banners",
		"Terrain-Corsica",
		"Terrain-Corsica_x4y3",
		"Terrain-Corsica_x5y3",
		"Terrain-Corsica_x6y3",
		"Terrain-Corsica_x7y3",
		"Terrain-Corsica_x8y3",
		"Terrain-Corsica_x9y3",
		"Terrain-Corsica_x10y3",
		"Terrain-Corsica_x11y3",
		"Terrain-Corsica_x12y3",
		"Terrain-Corsica_x4y4",
		"Terrain-Corsica_x5y4",
		"Terrain-Corsica_x6y4",
		"Terrain-Corsica_x7y4",
		"Terrain-Corsica_x8y4",
		"Terrain-Corsica_x9y4",
		"Terrain-Corsica_x10y4",
		"Terrain-Corsica_x11y4",
		"Terrain-Corsica_x12y4",
		"Terrain-Corsica_x13y4",
		"Terrain-Corsica_x3y5",
		"Terrain-Corsica_x4y5",
		"Terrain-Corsica_x5y5",
		"Terrain-Corsica_x6y5",
		"Terrain-Corsica_x7y5",
		"Terrain-Corsica_x8y5",
		"Terrain-Corsica_x9y5",
		"Terrain-Corsica_x10y5",
		"Terrain-Corsica_x11y5",
		"Terrain-Corsica_x12y5",
		"Terrain-Corsica_x13y5",
		"Terrain-Corsica_x3y6",
		"Terrain-Corsica_x4y6",
		"Terrain-Corsica_x5y6",
		"Terrain-Corsica_x6y6",
		"Terrain-Corsica_x7y6",
		"Terrain-Corsica_x8y6",
		"Terrain-Corsica_x9y6",
		"Terrain-Corsica_x10y6",
		"Terrain-Corsica_x11y6",
		"Terrain-Corsica_x12y6",
		"Terrain-Corsica_x13y6",
		"Terrain-Corsica_x3y7",
		"Terrain-Corsica_x4y7",
		"Terrain-Corsica_x5y7",
		"Terrain-Corsica_x6y7",
		"Terrain-Corsica_x7y7",
		"Terrain-Corsica_x8y7",
		"Terrain-Corsica_x9y7",
		"Terrain-Corsica_x10y7",
		"Terrain-Corsica_x11y7",
		"Terrain-Corsica_x12y7",
		"Terrain-Corsica_x13y7",
		"Terrain-Corsica_x3y8",
		"Terrain-Corsica_x4y8",
		"Terrain-Corsica_x5y8",
		"Terrain-Corsica_x6y8",
		"Terrain-Corsica_x7y8",
		"Terrain-Corsica_x8y8",
		"Terrain-Corsica_x9y8",
		"Terrain-Corsica_x10y8",
		"Terrain-Corsica_x11y8",
		"Terrain-Corsica_x12y8",
		"Terrain-Corsica_x13y8",
		"Terrain-Corsica_x3y9",
		"Terrain-Corsica_x4y9",
		"Terrain-Corsica_x5y9",
		"Terrain-Corsica_x6y9",
		"Terrain-Corsica_x7y9",
		"Terrain-Corsica_x8y9",
		"Terrain-Corsica_x9y9",
		"Terrain-Corsica_x10y9",
		"Terrain-Corsica_x11y9",
		"Terrain-Corsica_x12y9",
		"Terrain-Corsica_x13y9",
		"Music-Corsica_Ambient1",
		"Music-Corsica_Ambient2",
		"Music-Corsica_Ambient3",
		"Music-Dungeon_Ambient1",
		"Music-Dungeon_Ambient2",
		"Armor-CC-Clothing5",
		"Armor-Cloth-High3",
		"Armor-Cloth-Male-Casual1",
		"Armor-Heavy-Low2",
		"Armor-Light-Low1",
		"Armor-Light-Low8",
		"Armor-Light-Mid4",
		"Item-1hHammer-Basic1",
		"Terrain-Corsica_x3y10",
		"Terrain-Corsica_x4y10",
		"Terrain-Corsica_x5y10",
		"Terrain-Corsica_x6y10",
		"Terrain-Corsica_x7y10",
		"Terrain-Corsica_x8y10",
		"Terrain-Corsica_x9y10",
		"Terrain-Corsica_x10y10",
		"Terrain-Corsica_x11y10",
		"Terrain-Corsica_x12y10",
		"Terrain-Corsica_x13y10",
		"Terrain-Corsica_x3y11",
		"Terrain-Corsica_x4y11",
		"Terrain-Corsica_x5y11",
		"Terrain-Corsica_x6y11",
		"Terrain-Corsica_x7y11",
		"Terrain-Corsica_x8y11",
		"Terrain-Corsica_x9y11",
		"Terrain-Corsica_x10y11",
		"Terrain-Corsica_x11y11",
		"Terrain-Corsica_x12y11",
		"Terrain-Corsica_x13y11",
		"Terrain-Corsica_x3y12",
		"Terrain-Corsica_x4y12",
		"Terrain-Corsica_x5y12",
		"Terrain-Corsica_x6y12",
		"Terrain-Corsica_x7y12",
		"Terrain-Corsica_x8y12",
		"Terrain-Corsica_x9y12",
		"Terrain-Corsica_x10y12",
		"Terrain-Corsica_x11y12",
		"Terrain-Corsica_x12y12",
		"Terrain-Corsica_x13y12"
	],
	PlayStage3_Dep = [
		"#PlayStage2_Dep",
		"#Sound3"
	],
	PlayStage3 = [
		"#PlayStage3_Dep",
		"ATS-Dungeon_Castle",
		"Bldg-Urban-Small_Facade3",
		"CL-Bramble",
		"CL-Camp",
		"CL-LightHouse",
		"CL-Mushroom",
		"CL-Rotted",
		"Dng-Ent_Down",
		"Dng-Room",
		"Prop-Bush_Rotted",
		"Prop-Crystals",
		"Prop-Dirt_Borough",
		"Prop-Fog",
		"Prop-Giant_Mushroom-Rotted",
		"Prop-Hide_Tent1",
		"Prop-Instance_Entrance",
		"Prop-Lighthouse1",
		"Prop-Loot1",
		"Prop-Mushrooms2",
		"Prop-Notes",
		"Prop-PustuleNode",
		"Prop-Rotted_Hedge",
		"Prop-Tribal_Furniture",
		"Armor-CC-Clothing2",
		"Armor-CC-Clothing8",
		"Armor-Cloth-Female-Fancy1",
		"Armor-Cloth-Low1",
		"Armor-Cloth-Low2",
		"Armor-Cloth-Low4",
		"Armor-Cloth-Low8",
		"Armor-Cloth-Male-Fancy1",
		"Armor-Cloth-Mid2",
		"Armor-Cloth-Mid3",
		"Armor-Cloth-Mid4",
		"Armor-Heavy-Mid2",
		"Armor-Light-High2",
		"Armor-Light-Low5",
		"Armor-Light-Low7",
		"Armor-Light-Mid1",
		"Armor-Medium-High1",
		"Armor-Medium-Low2",
		"Armor-Medium-Low3",
		"Horde-Anubian_Wrathhag",
		"Horde-Birdtrap",
		"Horde-Chicken",
		"Horde-Crabquatch",
		"Horde-Death_Blossom",
		"Horde-Dreamfly",
		"Horde-Flytrap",
		"Horde-Giant_Crab",
		"Horde-Guinea_Fowl",
		"Horde-Prickly_Boar",
		"Horde-Quack_a_Trice",
		"Horde-Spider",
		"Horde-Vulture",
		"Item-2hSword-Medium3",
		"Item-Dagger-Basic2",
		"Item-Fishing_Pole1",
		"Item-Wand-Basic4",
		"Item-Wand-Basic5",
		"ATS-Cave_Rock",
		"Cav-Hall",
		"Cav-Pillar",
		"Cav-Room",
		"Prop-Door",
		"Prop-Short_Palm",
		"Prop-Water_Planes",
		"Terrain-Corsica_x3y13",
		"Terrain-Corsica_x4y13",
		"Terrain-Corsica_x5y13",
		"Terrain-Corsica_x6y13",
		"Terrain-Corsica_x7y13",
		"Terrain-Corsica_x8y13",
		"Terrain-Corsica_x9y13",
		"Terrain-Corsica_x10y13",
		"Terrain-Corsica_x11y13",
		"Terrain-Corsica_x12y13",
		"Terrain-Corsica_x13y13",
		"Terrain-Corsica_x3y14",
		"Terrain-Corsica_x4y14",
		"Terrain-Corsica_x5y14",
		"Terrain-Corsica_x6y14",
		"Terrain-Corsica_x7y14",
		"Terrain-Corsica_x8y14",
		"Terrain-Corsica_x9y14",
		"Terrain-Corsica_x10y14",
		"Terrain-Corsica_x11y14",
		"Terrain-Corsica_x12y14",
		"Terrain-Corsica_x13y14",
		"Terrain-Corsica_x3y15",
		"Terrain-Corsica_x4y15",
		"Terrain-Corsica_x5y15",
		"Terrain-Corsica_x6y15",
		"Terrain-Corsica_x7y15",
		"Terrain-Corsica_x8y15",
		"Terrain-Corsica_x9y15",
		"Terrain-Corsica_x10y15",
		"Terrain-Corsica_x11y15",
		"Terrain-Corsica_x12y15",
		"Terrain-Corsica_x13y15",
		"Terrain-Corsica_x3y16",
		"Terrain-Corsica_x4y16",
		"Terrain-Corsica_x5y16",
		"Terrain-Corsica_x6y16",
		"Terrain-Corsica_x7y16",
		"Terrain-Corsica_x8y16",
		"Terrain-Corsica_x9y16",
		"Terrain-Corsica_x10y16",
		"Terrain-Corsica_x11y16",
		"Terrain-Corsica_x12y16",
		"Terrain-Corsica_x13y16",
		"Terrain-Corsica_x3y17",
		"Terrain-Corsica_x4y17",
		"Terrain-Corsica_x5y17",
		"Terrain-Corsica_x6y17",
		"Terrain-Corsica_x7y17",
		"Terrain-Corsica_x8y17",
		"Terrain-Corsica_x9y17",
		"Terrain-Corsica_x10y17",
		"Terrain-Corsica_x11y17",
		"Terrain-Corsica_x12y17",
		"Terrain-Corsica_x13y17",
		"Horde-Giant_Snail",
		"Horde-Thunder_Walker",
		"Horde-Vulcan_Walker",
		"Music-Earthrise_Ambient1",
		"Music-Earthrise_Ambient2",
		"Music-Earthrise_Ambient3",
		"Terrain-Corsica_x3y18",
		"Terrain-Corsica_x4y18",
		"Terrain-Corsica_x5y18",
		"Terrain-Corsica_x6y18",
		"Terrain-Corsica_x7y18",
		"Terrain-Corsica_x8y18",
		"Terrain-Corsica_x9y18",
		"Terrain-Corsica_x10y18",
		"Terrain-Corsica_x11y18",
		"Terrain-Corsica_x12y18",
		"Terrain-Corsica_x13y18",
		"Terrain-Corsica_x3y19",
		"Terrain-Corsica_x4y19",
		"Terrain-Corsica_x5y19",
		"Terrain-Corsica_x6y19",
		"Terrain-Corsica_x7y19",
		"Terrain-Corsica_x8y19",
		"Terrain-Corsica_x9y19",
		"Terrain-Corsica_x10y19",
		"Terrain-Corsica_x11y19",
		"Terrain-Corsica_x12y19",
		"Terrain-Corsica_x13y19",
		"Terrain-Corsica_x3y20",
		"Terrain-Corsica_x4y20",
		"Terrain-Corsica_x5y20",
		"Terrain-Corsica_x6y20",
		"Terrain-Corsica_x7y20",
		"Terrain-Corsica_x8y20",
		"Terrain-Corsica_x9y20",
		"Terrain-Corsica_x10y20",
		"Terrain-Corsica_x11y20",
		"Terrain-Corsica_x12y20",
		"Terrain-Corsica_x13y20",
		"Terrain-Corsica_x3y21",
		"Terrain-Corsica_x4y21",
		"Terrain-Corsica_x5y21",
		"Terrain-Corsica_x6y21",
		"Terrain-Corsica_x7y21",
		"Terrain-Corsica_x8y21",
		"Terrain-Corsica_x9y21",
		"Terrain-Corsica_x10y21",
		"Terrain-Corsica_x11y21",
		"Terrain-Corsica_x12y21"
	],
	PlayStage4_Dep = [
		"#PlayStage3_Dep",
		"#Sound4"
	],
	PlayStage4 = [
		"#PlayStage4_Dep",
		"ATS-Roman",
		"Bldg-Classical",
		"Bldg-Earthend",
		"Boss-Sheaya",
		"CL-Anubian_Control_Powered",
		"CL-VaultKeeper1",
		"Prop-Bread_Loaf",
		"Prop-Burnt_Palms",
		"Prop-Desert_Accent",
		"Prop-Dvergar_Accessories",
		"Prop-Fountain",
		"Prop-Mother_Shard",
		"Prop-NewVespin_Great_Hive",
		"Prop-NewVespin_Hive",
		"Prop-Siege_Weapons",
		"Terrain-Earthend",
		"Terrain-Earthend_x5y5",
		"Terrain-Earthend_x5y6",
		"Terrain-Earthend_x5y7",
		"Terrain-Earthend_x5y8",
		"Terrain-Earthend_x6y5",
		"Terrain-Earthend_x6y6",
		"Terrain-Earthend_x6y7",
		"Terrain-Earthend_x6y8",
		"Terrain-Earthend_x7y5",
		"Terrain-Earthend_x7y6",
		"Terrain-Earthend_x7y7",
		"Terrain-Earthend_x7y8",
		"Armor-CC-Clothing7",
		"Armor-Cloth-Epic2",
		"Armor-Cloth-Low3",
		"Armor-Cloth-Low7",
		"Armor-Cloth-Mid1",
		"Armor-Cloth-Mid5",
		"Armor-Heavy-Mid1",
		"Armor-Heavy-Mid3",
		"Armor-Light-High1",
		"Armor-Light-Low3",
		"Armor-Light-Mid3",
		"Armor-Medium-Mid1",
		"Horde-Easter_Egg",
		"Horde-Elemental_Air",
		"Horde-Elemental_Earth",
		"Horde-Scavenger_Crab",
		"Horde-Simian",
		"Horde-Stalagmite",
		"Horde-Vespin",
		"Horde-Vespin_Stinger",
		"Item-1hAxe-Basic2",
		"Item-1hAxe-Basic3",
		"Item-1hMace-Basic4",
		"Item-1hSword-Basic10",
		"Item-1hSword-Basic11",
		"Item-1hSword-Basic6",
		"Item-1hSword-High3",
		"Item-2hAxe-Basic5",
		"Item-2hMace-Basic3",
		"Item-2hSword-Basic3",
		"Item-Dagger-Basic4",
		"Item-Dagger-Basic7",
		"Item-Dagger-Medium1",
		"Item-Scavenger_Crab-Junk",
		"Item-Shield-Basic7",
		"Item-Shield-Medium2",
		"Item-Spear-Medium1",
		"Item-Staff-Basic1",
		"Item-Staff-Basic4",
		"Item-Talisman-Basic2",
		"Item-Talisman-High1",
		"Item-Wand-Basic3",
		"Item-Wand-Basic6",
		"Item-Wand-High2",
		"Item-Wand-Medium1",
		"Item-Wand-Medium3",
		"Music-Anglorum_Ambient1",
		"Music-Cave",
		"Music-Corsica_Ambient1",
		"Music-Corsica_Ambient2",
		"Music-Corsica_Ambient3",
		"Music-Desert_Ambient1",
		"Music-Desert_Ambient2",
		"Music-Dungeon_Ambient1",
		"Music-Dungeon_Ambient2",
		"Music-DragonAndToast",
		"Music-Earthrise_Ambient1",
		"Music-Earthrise_Ambient2",
		"Music-Earthrise_Ambient3",
		"Music-Explore1",
		"Music-Explore2_Part1",
		"Music-Explore2_Part2",
		"Music-Gothic",
		"Music-TheRule",
		"Music-GrimIdol",
		"Music-NightCave",
		"Music-Morningtime",
		"Music-Mystical_Ambient1",
		"Music-Mystical_Ambient2",
		"Music-Mystical_Ambient3",
		"Music-Newb2",
		"Music-Newbiebox",
		"Music-Nightstars",
		"Music-Northern",
		"Music-Religious",
		"Music-Rotted_Ambient1",
		"Music-Rotted_Ambient2",
		"Music-DarkWalk",
		"Music-DarkTimes",
		"Music-Hyperfun",
		"Music-LandOfPhantoms",
		"Music-SomeAmountOfEvil",
		"Music-FailingDefense",
		"Music-Killers",
		"Music-Tavern",
		"Music-Tap",
		"Music-CurseOfTheScarab",
		"Music-Theme",
		"Music-Tribal"
	],
	Sound1 = [
		"Sound-Test",
		"Sound-Interface",
		"Sound-Ambient-Stage1",
		"Sound-Combat-Stage1"
	],
	Sound2 = [
		"Sound-Ambient-Stage2",
		"Sound-Combat-Stage2"
	],
	Sound3 = [
		"Sound-Ambient-Stage3",
		"Sound-Combat-Stage3"
	],
	Sound4 = [
		"Sound-Ambient-Stage4",
		"Sound-Combat-Stage4"
	],
	Clutter = [
		"Prop-Clutter1"
	],
	Map = [
		"Maps-World",
		"Maps-Starting_Grove"
	]
};

foreach( i, x in ::Races )
{
	foreach( j, y in ::Genders )
	{
		this.LoadGate.Gates.Races.append("Biped-" + x + "_" + y);
	}
}

class this.ListenerWrapper 
{
	mListener = null;
	mGate = null;
	constructor( gate, listener )
	{
		this.mListener = listener;
		this.mGate = gate;
	}

	function onPackageComplete( package )
	{
		if ("onPackageComplete" in this.mListener)
		{
			this.mListener.onPackageComplete(package);
		}
	}

	function onPackageError( pkg, error )
	{
		if ("onPackageError" in this.mListener)
		{
			this.mListener.onPackageError(pkg, error);
		}
		else
		{
			::UI.FatalError("Load gate \'" + this.mGate + "\' failed to load.");
		}
	}

}

this.LoadGate._Load <- function ( pGate, priority, listener )
{
	local result = [];
	local added_gates = {};
	local pending_gates = [
		pGate
	];

	while (pending_gates.len() > 0)
	{
		local gate = pending_gates[0];
		pending_gates.remove(0);

		if (gate in added_gates)
		{
			continue;
		}

		added_gates[gate] <- true;

		foreach( a in this.LoadGate.Gates[gate] )
		{
			if (a[0] == 35)
			{
				pending_gates.append(a.slice(1, a.len()));
			}
			else
			{
				result.append(a);
			}
		}
	}

	this.log.debug("Loading Gate: " + pGate + " @ " + priority + " [" + this.Util.join(result, ",") + "]");
	::_contentLoader.load(result, priority, pGate, this.ListenerWrapper(pGate, listener));
};
this.LoadGate.Require <- function ( gate, ... )
{
	local listener = vargc > 0 ? vargv[0] : null;
	this.LoadGate._Load(gate, this.ContentLoader.PRIORITY_REQUIRED, listener);

	if (gate in this.GateTrigger.LoadTriggers)
	{
		this.GateTrigger.LoadTriggers[gate].loaded = true;
	}
};
this.LoadGate.Load <- function ( gate, ... )
{
	local extraPriority = vargc > 0 ? vargv[0] : 0;
	this.LoadGate._Load(gate, this.ContentLoader.PRIORITY_NORMAL + extraPriority, null);

	if (gate in this.GateTrigger.LoadTriggers)
	{
		this.GateTrigger.LoadTriggers[gate].loaded = true;
	}
};
this.LoadGate.Fetch <- function ( gate, ... )
{
	local extraPriority = vargc > 0 ? vargv[0] : 0;
	this.LoadGate._Load(gate, this.ContentLoader.PRIORITY_FETCH + extraPriority, null);
};
this.LoadGate.Prefetch <- function ( pGate )
{
	local result = [];
	local added_gates = {};
	local pending_gates = [
		pGate
	];

	while (pending_gates.len() > 0)
	{
		local gate = pending_gates[0];
		pending_gates.remove(0);

		if (gate in added_gates)
		{
			continue;
		}

		added_gates[gate] <- true;

		foreach( a in this.LoadGate.Gates[gate] )
		{
			if (a[0] == 35)
			{
				pending_gates.append(a.slice(1, a.len()));
			}
			else
			{
				result.append(a);
			}
		}
	}

	::_contentLoader.prefetch(result);
	this.log.debug("Prefetching Gate: " + pGate + " [" + this.Util.join(result, ",") + "]");

	if (pGate in this.GateTrigger.FetchTriggers)
	{
		this.GateTrigger.FetchTriggers[pGate].fetched = true;
	}
};
this.LoadGate.AutoFetchLoadGate <- function ( triggerType, data )
{
	local triggerData;

	if (triggerType == this.GateTrigger.TriggerTypes.LOCATION)
	{
		triggerData = ::LocationLoadGateTrigger();
	}
	else if (triggerType == this.GateTrigger.TriggerTypes.LEVEL)
	{
		triggerData = ::LevelLoadGateTrigger();
	}
	else if (triggerType == this.GateTrigger.TriggerTypes.QUEST_COMPLETE)
	{
		triggerData = ::QuestCompleteGateTrigger();
	}
	else if (triggerType == this.GateTrigger.TriggerTypes.QUEST_ACT_COMPLETE)
	{
		triggerData = ::QuestActCompleteGateTrigger();
	}

	if (triggerData && (triggerData instanceof this.LoadGateTrigger))
	{
		triggerData.fetch(data);
		triggerData.load(data);
	}
};
