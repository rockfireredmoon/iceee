/* Merge elements into the existing table. */

//  Adds or modifies creature model packages, optionally associating it with a base
//  asset package if the creature was derived from an existing type.  Creature models
//  will not be available in creaturetweak unless they are defined.

require("Creatures");

/*  PF additions: Torchlight 2 pet models  */

CreatureIndex["Pet-Alpaca"] <- "";
CreatureIndex["Pet-Bigcat"] <- "";
CreatureIndex["Pet-Bombot"] <- "";
CreatureIndex["Pet-Bulldog"] <- "";
CreatureIndex["Pet-Cat"] <- "";
CreatureIndex["Pet-Chakawary"] <- "";
CreatureIndex["Pet-Copterbot"] <- "";
CreatureIndex["Pet-Deer"] <- "";
CreatureIndex["Pet-Dog"] <- "";
CreatureIndex["Pet-Falcor"] <- "";
CreatureIndex["Pet-Ferret"] <- "";
CreatureIndex["Pet-Hammerbot"] <- "";
CreatureIndex["Pet-Hawk"] <- "";
CreatureIndex["Pet-Headcrab"] <- "";
CreatureIndex["Pet-Healbot"] <- "";
CreatureIndex["Pet-Honeybadger"] <- "";
CreatureIndex["Pet-Owl"] <- "";
CreatureIndex["Pet-Panda"] <- "";
CreatureIndex["Pet-Sentry"] <- "";
CreatureIndex["Pet-Wolf"] <- "";


/*  0.8.8 or higher  */

//CreatureIndex["Horde-Aggro"] <- "";          /* Not a model, just has sound files */
CreatureIndex["Horde-Beast_Catapult"] <- "";   /* Requires AssetDependencies/Horde-Beast_Catapult.deps.cnut*/
CreatureIndex["Horde-Strange_Device"] <- "";