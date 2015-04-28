this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.MarkerTweakScreen extends this.GUI.Frame
{
	static mClassName = "Screens.MarkerTweakScreen";
	mScreenInitialized = false;
	mRefreshButton = null;
	mNewButton = null;
	mDelButton = null;
	mGoButton = null;
	mList = null;
	mNameEdit = null;
	mComment = null;
	mEditTimer = null;
	mMarkers = [];
	mCurrentMarker = null;
	constructor()
	{
		this.GUI.Frame.constructor("World Markers");
		this.mRefreshButton = this._createButton("Refresh", "onRefresh");
		this.mNewButton = this._createButton("New", "onNewMarker");
		this.mDelButton = this._createButton("Del", "onDeleteMarker");
		this.mGoButton = this._createButton("Go", "onGoToMarker");
		this.mMarkers = [];
		this.mEditTimer = this.GUI.Timer("_submitEdit");
		this.mEditTimer.addListener(this);
		this.mList = this.GUI.ColumnList();
		this.mList.addColumn("Marker", 150);
		this.mList.addActionListener(this);
		this.mComment = this.GUI.InputArea();
		this.mComment.addActionListener(this);
		this.mNameEdit = this.GUI.InputArea();
		this.mNameEdit.addActionListener(this);
		local top = this.GUI.Container();
		top.getLayoutManager().setGaps(2, 1);
		top.add(this.mRefreshButton);
		top.add(this.mNewButton);
		top.add(this.mDelButton);
		top.add(this.mGoButton);
		local bottom = this.GUI.Container(this.GUI.BorderLayout());
		bottom.add(this.GUI.LabelContainer("Marker:", this.mNameEdit), this.GUI.BorderLayout.NORTH);
		bottom.add(this.GUI.ScrollPanel(this.mComment), this.GUI.BorderLayout.CENTER);
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

	function onTextChanged( input )
	{
		this.mEditTimer.setDelay(10000);
	}

	function onRefresh( button )
	{
		this.refresh();
	}

	function onNewMarker( button )
	{
		this.mNameEdit.setText("[New Marker name]");
		this.mComment.setText("[Comment here]");
		this.mEditTimer.cancel();
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
			this._submitEdit();
			this.mNameEdit.setText("");
			this.mComment.setText("");
			this.mEditTimer.cancel();
		}
	}

	function onQueryComplete( qa, rows )
	{
		if (qa.query == "marker.list")
		{
			if (qa.args[0] == "zone")
			{
				this._submitEdit();
				this.mMarkers = [];
				this.mCurrentMarker = null;
				this.mList.removeAllRows();
				rows.sort(function ( a, b )
				{
					return this.strcasecmp(a[0], b[0]);
				});
				local row;

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
					//if (::_minimap)
					//{
					//	local xyz = this.split(m.position, " ");
					//	::_minimap.addStaticSticker(m.name, "red_paw", xyz[0].tofloat(), xyz[2].tofloat());
					//}

					this.mMarkers.append(m);

					//if (m.comment != "")
					//{
					//	this.mList.addRow([
					//		m.comment
					//	]);
					//}
					//else
					//{
					//	this.mList.addRow([
					//		m.name
					//	]);
					//}
				}
			}
			else if (this.mCurrentMarker && this.mCurrentMarker.name == rows[0][0])
			{
				this.mCurrentMarker.fullComment <- rows[0][3];
				this.mComment.setText(this.mCurrentMarker.fullComment);
				this.mEditTimer.cancel();
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
			m.name = qa.args[2];
			m.comment = qa.args[4];
			m.fullComment = qa.args[4];
			this.mList.removeRow(m.index);
			this.mList.insertRow(m.index, m);

			if (this.mCurrentMarker && this.mCurrentMarker.name == m.name)
			{
				this.mList.selectRow(m.index);
			}
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
				this.mEditTimer.cancel();
			}
		}
		else if(qa.query == "marker.comment") 
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
				this.mComment.setText(rows[0][0]);
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
		this._submitEdit();
		this.mCurrentMarker = this.mMarkers[index];
		this.mNameEdit.setText(this.mCurrentMarker.name);

		if ("fullComment" in this.mCurrentMarker)
		{
			this.mComment.setText(this.mCurrentMarker.fullComment);
		}
		else
		{
			this.mComment.setText("Fetching full comment...");
			this.mEditTimer.cancel();
			local args = [];
			args.append(this.mCurrentMarker.name);
			this._Connection.sendQuery("marker.comment", this, args);
		}
	}

	function _submitEdit()
	{
		if (this.mCurrentMarker == null || this.mEditTimer.getTimeUntilFire() == null)
		{
			return;
		}

		this.mEditTimer.cancel();
		local args = [];
		args.append(this.mCurrentMarker.name);
		args.append("n");
		args.append(this.mNameEdit.getText());
		args.append("c");
		args.append(this.mComment.getText());
		this._Connection.sendQuery("marker.edit", {}, args);
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

