this.require("GUI/FlowLayout");
this.require("GUI/Panel");
this.require("GUI/HTML");
class this.GUI.ChatBubble extends this.GUI.AnchorPanel
{
	constructor( so, anchor )
	{
		this.GUI.AnchorPanel.constructor();
		this.mAnchor = anchor;
		this.mGap = {
			x = 3,
			y = 3
		};
		this.mAnchorPos = {
			x = 15,
			y = 0
		};
		this.mAlignment = {
			horz = "left",
			vert = "top"
		};
		this.mOffset = {
			x = 0,
			y = 0
		};
		this.mSceneObject = so;
		this.setAppearance("ChatBubble");
		this.setPassThru(true);
		this.setReverseAlignment(false, false);
		this.setAlignment({
			horz = "center",
			vert = "bottom"
		});
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.mArrow = this.GUI.ChatBubbleArrow();
		this.mArrow.setSize(23, 24);
		this.mArrow.setBlendColor(this.Color(1, 1, 1, 0));
		this.add(this.mArrow);
	}

	function _addNotify()
	{
		this.GUI.AnchorPanel._addNotify();
		this.setOverlay("GUI/ChatBubbleOverlay");
	}

	function _removeNotify()
	{
		this.GUI.AnchorPanel._removeNotify();
	}

	function fadeIn()
	{
		this.mFadeOut = false;
		this.mFadeIn = true;
		this.mFadeOutComplete = false;
	}

	function fadeOut()
	{
		this.mFadeIn = false;
		this.mFadeOut = true;
		this.mFadeOutComplete = false;
	}

	function isFadeOutComplete()
	{
		return this.mFadeOutComplete;
	}

	function updateVisibility()
	{
		local pos = this.mSceneObject.getPosition();

		if (pos == null)
		{
			this.setVisible(false);
			return;
		}

		local forward = ::_camera.getParentNode()._getDerivedOrientation().rotate(this.Vector3(0.0, 0.0, -1.0));
		forward.normalize();
		local dir = pos - ::_camera.getParentNode().getWorldPosition();
		dir.normalize();
		this.setVisible(forward.dot(dir) > 0.69999999);
	}

	function onExitFrame()
	{
		this.GUI.AnchorPanel.onExitFrame();
		this.updateVisibility();

		if (this.mArrow)
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
				this.mArrow.setBlendColor(this.Color(1, 1, 1, this.mAlpha));
			}

			if (this.isObjectOnScreen())
			{
				local arrowx = this.mWidth * 0.64999998 - 12;

				if (arrowx > this.mWidth - 27)
				{
					arrowx = this.mWidth - 27;
				}

				this.mArrow.setPosition(arrowx, this.mHeight - 4);
				this.mArrow.setVisible(true);
			}
			else
			{
				this.mArrow.setVisible(false);
			}
		}
	}

	mArrow = null;
	mAlpha = 0;
	mFadeIn = true;
	mFadeOut = false;
	mFadeOutComplete = false;
	mFadeTime = 300;
	mSceneObject = null;
	static mClassName = "ChatBubble";
}

class this.GUI.ChatBubbleArrow extends this.GUI.Panel
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setLayoutManager(this.GUI.FlowLayout());
		this.mAppearance = "ChatBubbleArrow";
		this.setInsets(5);

		if (vargc > 0)
		{
			if (vargv[0] == null)
			{
				this.setLayoutManager(null);
			}
			else if (typeof vargv[0] == "instance" && (vargv[0] instanceof this.GUI.LayoutManager))
			{
				this.setLayoutManager(vargv[0]);
			}
			else if (typeof vargv[0] == "string")
			{
				this.mAppearance = vargv[0];
			}
		}
	}

	static mClassName = "ChatBubbleArrow";
}

