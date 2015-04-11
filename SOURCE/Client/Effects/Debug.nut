this.require("EffectDef");
class this.EffectDef.DiffuseTest extends this.EffectDef.TemplateMelee
{
	static mEffectName = "DiffuseTest";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local test = this.createGroup("Test", this.getTarget());
		test.add("DiffusePulse", {
			color1 = "ff0000",
			color2 = "000000",
			rate = 2.0,
			inherit = true
		});
		test.add("AmbientPulse", {
			color1 = "ff0000",
			color2 = "000000",
			rate = 2.0,
			inherit = true
		});
		this.fireIn(20.0, "onDone");
	}

}

