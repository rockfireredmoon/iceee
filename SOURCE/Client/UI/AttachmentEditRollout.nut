this.require("GUI/AnchorPanel");
this.AttachmentEditRolloutFlags <- {
	ATTACH_POINT = 1,
	PARTICLE_EFFECT = 2,
	COLORS = 4
};
this.AttachmentEditRolloutMode <- {
	CLOTHING = 1,
	ATTACHMENTS = 2
};
class this.GUI.AttachmentEditRollout extends this.GUI.AnchorPanel
{
	constructor( anchor, type, attachPoint, colors, ... )
	{
		this.GUI.AnchorPanel.constructor(anchor);
		this.setAnchor(anchor);
		this.setInsets(10, 10, 10, 10);
		local typeLabel = this.mTypeLabel = this.GUI.Label(type);
		typeLabel.setTextAlignment(0.5, 0.5);
		this.add(typeLabel);
		this.mAttachPointDD = null;
		this.mColorSelectors = [];
		this.GUI._Manager.addTransientToplevel(this);
		local tmp = vargc;
		this.mMode = vargc > 1 ? vargv[1] : this.AttachmentEditRolloutMode.ATTACHMENTS;
		this.mFlags = vargc > 0 ? vargv[0] : 4294967295;
		this.setType(type, this.mFlags, colors);

		if (this.mAttachPointDD != null && attachPoint != null)
		{
			this.mAttachPointDD.setCurrent(attachPoint, false);
		}

		this.setLayoutManager(this.GUI.GridLayout(this.components.len(), 1));
		this.setSize(this.getPreferredSize());
	}

	function _createAttachPointSelector( type, ... )
	{
		local ap;

		if (vargc > 0)
		{
			ap = vargv[0];
		}

		if (type in ::AttachableDef)
		{
			this.mAttachPointDD = this.GUI.DropDownList();

			foreach( i, x in ::AttachableDef[type].attachPoints )
			{
				if (!ap)
				{
					ap = x;
				}

				this.mAttachPointDD.addChoice(x);
			}

			this.mAttachPointDD.setCurrent(ap);
			this.mAttachPointDD.addSelectionChangeListener(this);
			this.mAttachPointDD.setChangeMessage("onAttachmentChange");
			this.add(this.mAttachPointDD);
		}
		else
		{
			throw this.mClassName + " - Type was not found in AttachableDef: " + this.mType;
		}
	}

	function _createColorSelector( colorArea, palette, colorDefault, selection )
	{
		local sc = this.GUI.ColorPicker(colorArea, colorArea, this.ColorRestrictionType.DEVELOPMENT);
		this.mColorSelectors.append(sc);

		if (selection != null)
		{
			sc.setColor(selection, false);
		}
		else
		{
			sc.setColor(colorDefault, false);
		}

		sc.setLabelVisible(true);
		sc.addActionListener(this);
		sc.setChangeMessage("onAttachmentChange");
		this.add(sc);
	}

	function removeAllSelectors()
	{
		if (this.mAttachPointDD)
		{
			this.mAttachPointDD.removeSelectionChangeListener(this);
			this.remove(this.mAttachPointDD);
		}

		if (this.mColorSelectors.len() == 0)
		{
			return;
		}

		foreach( i, x in this.mColorSelectors )
		{
			x.removeActionListener(this);
			this.remove(x);
		}

		this.mColorSelectors = [];
		this.mAttachPointDD = null;
	}

	function _createEffectSelector()
	{
		this.mEffectDD = this.GUI.DropDownList();
		this.mEffectDD.addChoice("-none-");
		local type = this.mTypeLabel.getText();

		if (type in ::AttachableDef)
		{
			local def = ::AttachableDef[type];

			if ("particles" in def)
			{
				foreach( i in def.particles )
				{
					this.mEffectDD.addChoice(i);
				}
			}
		}

		this.mEffectDD.addSelectionChangeListener(this);
		this.mEffectDD.setChangeMessage("onAttachmentChange");
		this.add(this.mEffectDD);
		this.mRow++;
	}

	function getDefinitionTable()
	{
		if (this.mMode == this.AttachmentEditRolloutMode.CLOTHING)
		{
			return ::ClothingDef;
		}

		return ::AttachableDef;
	}

	function setType( type, flags, ... )
	{
		local defTable = this.getDefinitionTable();

		if (type in defTable)
		{
			this.mTypeLabel.setText(type);
			this.removeAllSelectors();
			local adef = defTable[type];

			if (flags & this.AttachmentEditRolloutFlags.ATTACH_POINT)
			{
				this._createAttachPointSelector(type);
			}

			if (flags & this.AttachmentEditRolloutFlags.PARTICLE_EFFECT)
			{
				this._createEffectSelector();
			}

			if (flags & this.AttachmentEditRolloutFlags.COLORS && "colors" in adef)
			{
				local colors = vargc > 0 && vargv[0] != null ? clone vargv[0] : adef.colors;

				if (adef.colors.len() > colors.len())
				{
					local tmp;

					for( tmp = colors.len(); tmp < adef.colors.len(); tmp++ )
					{
						colors.append(adef.colors[tmp]);
					}
				}

				local colorDef = colors;

				foreach( i, color in colorDef )
				{
					if (i < adef.palette.len())
					{
						local palette = this.ColorPalette.unionPalettes(this.ColorPalette[adef.palette[i]], color);
						local selections = [];

						if (vargc > 0 && vargv[0] != null)
						{
							selections = vargv[0];
						}

						this._createColorSelector(this.ColorSlotNames[i], palette, color, selections.len() > i ? selections[i] : null);
					}
				}
			}
		}
	}

	function getCurrent()
	{
		local results = {};
		results.type <- this.mTypeLabel.getText();

		if (this.mAttachPointDD)
		{
			results.attachPoint <- this.mAttachPointDD.getCurrent();
		}

		if (this.mEffectDD)
		{
			local current = this.mEffectDD.getCurrent();
			results.effect <- current != "-none-" ? this.mEffectDD.getCurrent() : null;
		}

		if (this.mColorSelectors.len() > 0)
		{
			results.colors <- [];

			foreach( i, x in this.mColorSelectors )
			{
				results.colors.append(x.getCurrent());
			}
		}

		return results;
	}

	function onOutsideClick( evt )
	{
		this.mAnchor._closeRollout();
	}

	function onRolloutClosed( sender )
	{
		this.print("onRolloutClose called");
	}

	function onAttachmentChange( sender )
	{
		if (this.mAnchor)
		{
			if ("onAttachmentChange" in this.mAnchor)
			{
				this.mAnchor.onAttachmentChange(this);
			}
		}
	}

	function setEffect( name )
	{
		this.mEffectDD.setCurrent(name, false);
	}

	function destroy()
	{
		this.removeAllSelectors();
		this.setOverlay(null);
		this.GUI.AnchorPanel.destroy();
	}

	mFlags = 0;
	mMode = 0;
	mShowAttachPoint = true;
	mTypeLabel = null;
	mAttachPointDD = null;
	mColorSelectors = [];
	mEffectDD = null;
	mRow = 0;
	static mClassName = "AttachmentEditRollout";
}

class this.GUI.AttachmentEditPanel extends this.GUI.Container
{
	mFlags = 0;
	mMode = 0;
	mShowAttachPoint = true;
	mTypeLabel = null;
	mAttachPointDD = null;
	mColorSelectors = [];
	mEffectDD = null;
	mRow = 0;
	mListener = null;
	static mClassName = "AttachmentEditPanel";
	constructor( listener, type, attachPoint, colors, ... )
	{
		this.GUI.Container.constructor();
		this.mListener = listener;
		local typeLabel = this.mTypeLabel = this.GUI.Label(type);
		typeLabel.setTextAlignment(0.5, 0.5);
		this.add(typeLabel);
		this.mAttachPointDD = null;
		this.mColorSelectors = [];
		local tmp = vargc;
		this.mMode = vargc > 1 ? vargv[1] : this.AttachmentEditRolloutMode.ATTACHMENTS;
		this.mFlags = vargc > 0 ? vargv[0] : 4294967295;
		this.setType(type, this.mFlags, colors);

		if (this.mAttachPointDD != null && attachPoint != null)
		{
			this.mAttachPointDD.setCurrent(attachPoint, false);
		}

		this.setLayoutManager(this.GUI.BoxLayoutV(true));
	}

	function _createAttachPointSelector( type, ... )
	{
		local ap;

		if (vargc > 0)
		{
			ap = vargv[0];
		}

		if (type in ::AttachableDef)
		{
			this.mAttachPointDD = this.GUI.DropDownList();

			foreach( i, x in ::AttachableDef[type].attachPoints )
			{
				if (!ap)
				{
					ap = x;
				}

				this.mAttachPointDD.addChoice(x);
			}

			this.mAttachPointDD.setCurrent(ap);
			this.mAttachPointDD.addSelectionChangeListener(this);
			this.mAttachPointDD.setChangeMessage("onAttachmentChange");
			this.add(this.mAttachPointDD);
		}
		else
		{
			throw this.mClassName + " - Type was not found in AttachableDef: " + this.mType;
		}
	}

	function _createColorSelector( colorArea, palette, colorDefault, selection )
	{
		local sc = this.GUI.ColorPicker(colorArea, colorArea, this.ColorRestrictionType.DEVELOPMENT);
		this.mColorSelectors.append(sc);

		if (selection != null)
		{
			sc.setColor(selection, false);
		}
		else
		{
			sc.setColor(colorDefault, false);
		}

		sc.setLabelVisible(true);
		sc.addActionListener(this);
		sc.setChangeMessage("onAttachmentChange");
		this.add(sc);
	}

	function removeAllSelectors()
	{
		if (this.mAttachPointDD)
		{
			this.mAttachPointDD.removeSelectionChangeListener(this);
			this.remove(this.mAttachPointDD);
		}

		if (this.mColorSelectors.len() == 0)
		{
			return;
		}

		foreach( i, x in this.mColorSelectors )
		{
			x.removeActionListener(this);
			this.remove(x);
		}

		this.mColorSelectors = [];
		this.mAttachPointDD = null;
	}

	function _createEffectSelector()
	{
		this.mEffectDD = this.GUI.DropDownList();
		this.mEffectDD.addChoice("-none-");
		local type = this.mTypeLabel.getText();

		if (type in ::AttachableDef)
		{
			local def = ::AttachableDef[type];

			if ("particles" in def)
			{
				foreach( i in def.particles )
				{
					this.mEffectDD.addChoice(i);
				}
			}
		}

		this.mEffectDD.addSelectionChangeListener(this);
		this.mEffectDD.setChangeMessage("onAttachmentChange");
		this.add(this.mEffectDD);
		this.mRow++;
	}

	function getDefinitionTable()
	{
		if (this.mMode == this.AttachmentEditRolloutMode.CLOTHING)
		{
			return ::ClothingDef;
		}

		return ::AttachableDef;
	}

	function setType( type, flags, ... )
	{
		local defTable = this.getDefinitionTable();

		if (type in defTable)
		{
			this.mTypeLabel.setText(type);
			this.removeAllSelectors();
			local adef = defTable[type];

			if (flags & this.AttachmentEditRolloutFlags.ATTACH_POINT)
			{
				this._createAttachPointSelector(type);
			}

			if (flags & this.AttachmentEditRolloutFlags.PARTICLE_EFFECT)
			{
				this._createEffectSelector();
			}

			if (flags & this.AttachmentEditRolloutFlags.COLORS && "colors" in adef)
			{
				local colors = vargc > 0 && vargv[0] != null ? clone vargv[0] : adef.colors;

				if (adef.colors.len() > colors.len())
				{
					local tmp;

					for( tmp = colors.len(); tmp < adef.colors.len(); tmp++ )
					{
						colors.append(adef.colors[tmp]);
					}
				}

				local colorDef = colors;

				foreach( i, color in colorDef )
				{
					if (i < adef.palette.len())
					{
						local palette = this.ColorPalette.unionPalettes(this.ColorPalette[adef.palette[i]], color);
						local selections = [];

						if (vargc > 0 && vargv[0] != null)
						{
							selections = vargv[0];
						}

						this._createColorSelector(this.ColorSlotNames[i], palette, color, selections.len() > i ? selections[i] : null);
					}
				}
			}
		}
	}

	function getCurrent()
	{
		local results = {};
		results.type <- this.mTypeLabel.getText();

		if (this.mAttachPointDD)
		{
			results.attachPoint <- this.mAttachPointDD.getCurrent();
		}

		if (this.mEffectDD)
		{
			local current = this.mEffectDD.getCurrent();
			results.effect <- current != "-none-" ? this.mEffectDD.getCurrent() : null;
		}

		if (this.mColorSelectors.len() > 0)
		{
			results.colors <- [];

			foreach( i, x in this.mColorSelectors )
			{
				results.colors.append(x.getCurrent());
			}
		}

		return results;
	}

	function onAttachmentChange( sender )
	{
		if (this.mListener)
		{
			if ("onAttachmentChange" in this.mListener)
			{
				this.mListener.onAttachmentChange(this);
			}
		}
	}

	function setEffect( name )
	{
		this.mEffectDD.setCurrent(name, false);
	}

	function destroy()
	{
		this.removeAllSelectors();
		this.GUI.Container.destroy();
	}

}

