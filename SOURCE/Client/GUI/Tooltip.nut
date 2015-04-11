this.require("GUI/FlowLayout");
this.require("GUI/Panel");
this.require("GUI/HTML");
class this.GUI.Tooltip extends this.GUI.Panel
{
	constructor( contents )
	{
		this.GUI.Panel.constructor(this.GUI.BoxLayout());
		this.setResize(true);
		this.setPassThru(true);

		if (typeof contents == "instance")
		{
			this.mContents = contents;
			this.add(this.mContents);
		}
		else
		{
			this.mContents = this.GUI.HTML();
			this.mContents.setText(contents);
			this.add(this.mContents);
		}

		this.setBlendColor(this.Color(1, 1, 1, 0));
	}

	function destroy()
	{
		this.remove(this.mContents);
		this.GUI.Panel.destroy();
	}

	function fadeIn()
	{
		this.mFadeOut = false;
		this.mFadeIn = true;
		this.mFadeOutComplete = false;
	}

	function fadeOut()
	{
		this.mAlpha = 0;
		this.mFadeOutComplete = true;
		return;
		this.mFadeIn = false;
		this.mFadeOut = true;
		this.mFadeOutComplete = false;
	}

	function isFadeOutComplete()
	{
		return this.mFadeOutComplete;
	}

	function updateChildAlpha( child, alpha )
	{
		local col = child.getBlendColor();
		col.a = alpha;
		child.setBlendColor(col);
		col = child.getFontColor();
		col.a = alpha;
		child.setFontColor(col);

		foreach( c in child.components )
		{
			this.updateChildAlpha(c, alpha);
		}
	}

	function onExitFrame()
	{
		local delta = ::_deltat / 1000.0;
		local alphachanged = false;

		if (this.mFadeIn)
		{
			this.mAlpha += delta / (this.mFadeTime / 1000.0);
			alphachanged = true;

			if (this.mAlpha > 0.99000001)
			{
				this.mAlpha = 1;
				this.mFadeIn = false;
			}
		}

		if (this.mFadeOut)
		{
			this.mAlpha -= delta / (this.mFadeTime / 1000.0);
			alphachanged = true;

			if (this.mAlpha < 0.0099999998)
			{
				this.mAlpha = 0;
				this.mFadeOut = false;
				this.mFadeOutComplete = true;
			}
		}

		if (alphachanged)
		{
			this.setBlendColor(this.Color(1, 1, 1, this.mAlpha));

			foreach( c in this.components )
			{
				this.updateChildAlpha(c, this.mAlpha);
			}
		}
	}

	mAlpha = 1;
	mFadeIn = true;
	mFadeOut = false;
	mFadeOutComplete = false;
	mFadeTime = 120;
	mContents = null;
	static mClassName = "ToolTip";
}

