this.require("GUI/GUI");
this.require("GUI/Panel");
class this.GUI.PropertySheet extends this.GUI.Component
{
	constructor()
	{
		this.GUI.Component.constructor(this.GUI.GridLayout(1, 2));
		this.mProperties = [];
		this.mAppearance = "Container";
	}

	function onUpdate()
	{
		local idx;
		local prop;

		foreach( idx, prop in this.mProperties )
		{
			local control = this.components[idx * 2 + 1];

			if (prop.getServerGetQuery() != null)
			{
				continue;
			}

			if (prop.getType().getEnumValues() != null)
			{
				control.setCurrent(prop.getAsText(this.mBean));
			}
			else if (this.IsInstanceOf(control, this.GUI.InputArea))
			{
				control.setText(prop.getAsText(this.mBean));
			}
			else if ("setValue" in control)
			{
				control.setValue(prop.getValue(this.mBean));
			}
			else
			{
				this.log.warn("Cannot update control for property: " + prop.getDisplayName());
			}
		}
	}

	function setBean( bean )
	{
		this.removeAll();

		if (bean == null)
		{
			if (this.IsInstanceOf(this.mBean, this.MessageBroadcaster))
			{
				this.mBean.removeListener(this);
			}

			this.mBean = null;
			this.mProperties = null;
			return;
		}

		try
		{
			this.mProperties = this.Bean.getPropertyDescriptors(bean);
		}
		catch( err )
		{
			this.log.error("Error setting bean: " + err);
			return;
		}

		local i;
		local count = this.mProperties.len();
		local grid = this.GUI.GridLayout(count, 2);
		grid.setColumns(80, 120);
		this.setLayoutManager(grid);

		foreach( prop in this.mProperties )
		{
			this.add(this.GUI.Label(prop.getDisplayName()));
			local type = prop.getType();
			local sgq = prop.getServerGetQuery();
			local asynch = sgq != null;
			local asynch_editor;
			local asynch_text_editor = false;
			local enums = type.getEnumValues();

			if (enums != null)
			{
				local list = this.GUI.DropDownList();
				local e;

				foreach( e in enums )
				{
					list.addChoice(e);
				}

				list.setCurrent(prop.getAsText(bean));
				local callback = {
					bean = bean,
					prop = prop,
					function onSelectionChange( list )
					{
						::_opHistory.execute(this.BeanSetPropertyOp(this.bean, this.prop, this.prop.getType().toValue(list.getCurrent())));
					}

				};
				list.addSelectionChangeListener(callback);
				this.add(list);
				asynch_editor = list;
			}
			else
			{
				local editor = type.createEditorComponent();

				if (editor != null)
				{
					local callback = {
						bean = bean,
						prop = prop,
						function onInputComplete( editor )
						{
							if (this.prop.isReadOnly())
							{
								return;
							}

							::_opHistory.execute(this.BeanSetPropertyOp(this.bean, this.prop, editor.getValue()));
						}

					};

					if (asynch)
					{
						editor.setValue(prop.getDefaultValue());
					}
					else
					{
						editor.setValue(prop.getValue(bean));
					}

					if (!prop.isReadOnly())
					{
						editor.addActionListener(callback);
					}

					this.add(editor);
				}
				else
				{
					local callback = {
						bean = bean,
						prop = prop,
						prevText = prop.getAsText(bean),
						function onInputComplete( inputbox )
						{
							if (this.prop.isReadOnly())
							{
								inputbox.setText(this.prevText);
							}
							else
							{
								this.prevText = inputbox.getText();
								::_opHistory.execute(this.BeanSetPropertyOp(this.bean, this.prop, this.prop.getType().toValue(this.prevText)));
							}
						}

						function onInputCancelled( inputbox )
						{
							inputbox.setText(this.prevText);
						}

					};
					asynch_text_editor = true;

					if (asynch)
					{
						editor = this.GUI.InputArea("Fetching...");
					}
					else
					{
						editor = this.GUI.InputArea(prop.getAsText(bean));
					}

					editor.addActionListener(callback);

					if (prop.isReadOnly())
					{
						editor.setEnabled(false);
					}

					this.add(editor);
				}

				asynch_editor = editor;
			}

			if (asynch)
			{
				local queryCallback = {
					sheet = this,
					bean = bean,
					editor = asynch_editor,
					is_text = asynch_text_editor,
					property = prop,
					function onQueryComplete( qa, results )
					{
						if (this.sheet.mBean == this.bean)
						{
							local result = results[0][0];

							if (this.is_text)
							{
								this.editor.setText(result);
							}
							else
							{
								this.property.getType().setEditorValue(this.editor, result);
							}
						}
					}

				};
				::_Connection.sendQuery(sgq, queryCallback, [
					bean.getID(),
					prop.getName()
				]);
			}
		}

		this.mBean = bean;

		if (this.IsInstanceOf(this.mBean, this.MessageBroadcaster))
		{
			this.mBean.addListener(this);
		}

		if (this.mParentComponent)
		{
			this.mParentComponent.invalidate();
		}
	}

	function getBean()
	{
		return this.mBean;
	}

	function destroy()
	{
		if (this.IsInstanceOf(this.mBean, this.MessageBroadcaster))
		{
			this.mBean.removeListener(this);
		}

		this.GUI.Component.destroy();
	}

	function _debugstring()
	{
		return "bean=" + this.bean;
	}

	mBean = null;
	mProperties = null;
	static mClassName = "PropertySheet";
}

class this.GUI.PropertyPanel extends this.GUI.Panel
{
	constructor()
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		this.mSheet = this.GUI.PropertySheet();
		this.add(this.mSheet);
		this.setVisible(false);
		this.setOverlay(this.GUI.OVERLAY);
	}

	function setBean( bean )
	{
		this.mSheet.setBean(bean);
		this.setSize(this.getPreferredSize());
	}

	function getBean()
	{
		return this.mSheet.getBean();
	}

	mSheet = null;
}

