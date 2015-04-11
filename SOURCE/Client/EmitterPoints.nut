this.EmitterAliases <- {};
function InitEmitterSets()
{
	this.EmitterAliases.Scenery <- {
		primary = [
			"node"
		]
	};
	this.EmitterAliases.Creature <- {
		primary = [
			"node"
		],
		secondary = [
			"head"
		],
		core = [
			"spell_target"
		],
		casting = [
			"left_hand",
			"right_hand",
			"horde_caster",
			"horde_caster2"
		],
		attacking = [
			"left_hand",
			"right_hand",
			"horde_attacker",
			"horde_attacker2"
		],
		limbs = [
			"left_arm",
			"right_arm"
		],
		feet = [
			"left_foot",
			"right_foot"
		],
		joints = [
			"right_calf",
			"left_calf",
			"right_forearm",
			"left_forearm"
		]
	};
	this.EmitterAliases.Item <- {
		primary = [
			"node"
		]
	};
	this.EmitterAliases.Dummy <- {
		primary = [
			"node"
		]
	};
}

