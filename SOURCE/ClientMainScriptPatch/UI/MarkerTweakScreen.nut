this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.MarkerTweakScreen extends this.GUI.Frame
{
	static mClassName = "Screens.MarkerTweakScreen";
	mScreenInitialized = false;
	mRefreshButton = null;
	mNewButton = null;
	mSaveButton = null;
	mDelButton = null;
	mGoButton = null;
	mList = null;
	mNameEdit = null;
	mComment = null;
	mMarkers = [];
	mCurrentMarker = null;
	constructor()
	{
		this.GUI.Frame.constructor("World Markers");
		this.mRefreshButton = this._createButton("Refresh", "onRefresh");
		this.mNewButton = this._createButton("New", "onNewMarker");
		this.mDelButton = this._createButton("Del", "onDeleteMarker");
		this.mGoButton = this._createButton("Go", "onGoToMarker");
		this.mSaveButton = this._createButton("Save", "onSaveMarker");
		this.mMarkers = [];
		this.mList = this.GUI.ColumnList();
		this.mList.addColumn("Marker", 150);
		this.mList.addActionListener(this);
		this.mComment = this.GUI.InputArea();
		this.mNameEdit = this.GUI.InputArea();
		local top = this.GUI.Container();
		top.getLayoutManager().setGaps(2, 1);
		top.add(this.mRefreshButton);
		top.add(this.mNewButton);
		top.add(this.mDelButton);
		top.add(this.mGoButton);
		local bottom = this.GUI.Container(this.GUI.BorderLayout());
		bottom.add(this.GUI.LabelContainer("Marker:", this.mNameEdit), this.GUI.BorderLayout.NORTH);
		bottom.add(this.GUI.ScrollPanel(this.mComment), this.GUI.BorderLayout.CENTER);
		bottom.add(this.mSaveButton, this.GUI.BorderLayout.SOUTH);
		bottom.insets.top += 5;
		local cmain = this.GUI.Container(this.GUI.BorderLayout());
		cmain.setInsets(5);
		cmain.add(top, this.GUI.BorderLayout.NORTH);
		cmain.add(this.GUI.ScrollPanel(this.mList), this.GUI.BorderLayout.CENTER);
		cmain.add(bottom, this.GUI.BorderLayout.SOUTH);
		this.setContentPane(cmain);
		local sz = this.getPreferredSize();
		sz.height += 50;
		this.setSize(sz);
		this.setPosition(10, 10);
	}

	function _createButton( label, msg )
	{
		local b = this.GUI.Button(label);
		b.setReleaseMessage(msg);
		b.addActionListener(this);
		return b;
	}

	function refresh()
	{
		this._Connection.sendQuery("marker.list", this, [
			"zone"
		]);
	}

	function onComment(field) 
	{
	}

	function onRefresh( button )
	{
		this.refresh();
	}
	
	function onSaveMarker( button )
	{
		this._doSubmit();
	}

	function onNewMarker( button )
	{
		this.mNameEdit.setText("[New Marker name]");
		this.mComment.setText("[Comment here]");
	}

	function onDeleteMarker( button )
	{
		if (this.mCurrentMarker)
		{
			this._Connection.sendQuery("marker.del", this, [
				this.mCurrentMarker.name
			]);
		}
	}

	function onGoToMarker( button )
	{
		if (this.mCurrentMarker)
		{
			local xyz = this.split(this.mCurrentMarker.position, " ");
			this._Connection.sendGo(xyz[0].tofloat(), xyz[1].tofloat(), xyz[2].tofloat());
		}
	}

	function onRowSelectionChanged( list, row, selected )
	{
		if (selected)
		{
			this._setCurrentMarker(row);
		}
		else
		{
			this.mNameEdit.setText("");
			this.mComment.setText("");
		}
	}

	function onQueryComplete( qa, rows )
	{
		if (qa.query == "marker.list")
		{
			if (qa.args[0] == "zone")
			{
				this.mMarkers = [];
				local mCurrentMarkerName = this.mCurrentMarker == null ? null : this.mCurrentMarker.name;
				this.mCurrentMarker = null;
				this.mList.removeAllRows();
				rows.sort(function ( a, b )
				{
					return this.strcasecmp(a[0], b[0]);
				});
				local row;
				local selIdx = -1;
				local idx = 0;

				foreach( row in rows )
				{
					local m = {
						name = row[0],
						zone = row[1],
						position = row[2],
						comment = row[3],
						index = this.mMarkers.len()
					};

					// Em - 'addStaticSticker' does not seem to exist. Is this a native bound method
					if (::_minimap)
					{
						try {
							local xyz = this.split(m.position, " ");
							::_minimap.addStaticSticker(m.name, "red_paw", xyz[0].tofloat(), xyz[2].tofloat());
						}
						catch(ex) {
							print("ICE! FAILED marker: " + ex);
						}						
					}

					this.mMarkers.append(m);

					this.mList.addRow([
						m.name
					]);
					
					if(m.name == mCurrentMarkerName) {
						this.mComment.setText(m.comment);
						selIdx = idx;
					}
					idx++;
				}
				if(selIdx != -1) {
					this.mList.setSelectedRows(selIdx);
				}
			}
		}
		else if (qa.query == "marker.edit")
		{
			local index = this._findMarker(qa.args[0]);
			if (index == null)
			{
				return;
			}
			local m = this.mMarkers[index];
			m.name = this.mNameEdit.getText();
			m.comment =  this.mComment.getText();
			this.mList.removeRow(m.index);
			this.mList.insertRow(m.index, [
				m.name
			]);
			this.mList.setSelectedRows(m.index);
		}
		else if (qa.query == "marker.del")
		{
			local index = this._findMarker(qa.args[0]);

			if (index == null)
			{
				return;
			}

			local m = this.mMarkers[index];
			this.mList.removeRow(m.index);

			if (this.mCurrentMarker == m)
			{
				this.mCurrentMarker = null;
				this.mComment.setText("");
				this.mNameEdit.setText("");
			}
		}
	}

	function _findMarker( name )
	{
		local i;
		local m;

		foreach( i, m in this.mMarkers )
		{
			if (m.name == name)
			{
				return i;
			}
		}

		return null;
	}

	function _setCurrentMarker( index )
	{
		this.mCurrentMarker = this.mMarkers[index];
		this.mNameEdit.setText(this.mCurrentMarker.name);
		this.mComment.setText(this.mCurrentMarker.comment);
	}

	
	function _doSubmit()
	{
		if (this.mCurrentMarker == null)
		{
			return;
		}
		
		local args = [];
		args.append(this.mCurrentMarker.name);
		args.append("n");
		args.append(this.mNameEdit.getText());
		args.append("c");
		args.append(this.mComment.getText());
		this._Connection.sendQuery("marker.edit", this, args);
	}

	function setVisible( value )
	{
		if (value && !this.isVisible())
		{
			if (!this.mScreenInitialized)
			{
				this.refresh();
				this.mScreenInitialized = true;
			}
		}

		this.GUI.Frame.setVisible(value);
	}

}

