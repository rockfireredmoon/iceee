this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.SceneryObjectBrowser extends this.GUI.Frame
{
	mFilterInput = null;
	mRadiusInput = null;
	mOutputList = null;
	mIDList = [
		null
	];
	constructor()
	{
		this.GUI.Frame.constructor("SceneryObjectBrowser");
		local chead = this.GUI.Container(this.GUI.BorderLayout());
		chead.setInsets(0, 0, 5, 0);
		local cmain = this.GUI.Container(this.GUI.BorderLayout());
		cmain.setInsets(5);
		this.setContentPane(cmain);
		this.setSize(500, 340);
		this.setPosition(10, 10);
		local SceneryBrowserOptions = this.GUI.Container(this.GUI.GridLayout(1, 5));
		SceneryBrowserOptions.getLayoutManager().setColumns("*", 50, 25, "*", 50);
		SceneryBrowserOptions.add(this.GUI.Label("Filter"));
		this.mFilterInput = this.GUI.CancellableInputArea("");
		SceneryBrowserOptions.add(this.mFilterInput);
		SceneryBrowserOptions.add(this.GUI.Spacer(0, 0));
		SceneryBrowserOptions.add(this.GUI.Label("Radius"));
		this.mRadiusInput = this.GUI.CancellableInputArea("500");
		SceneryBrowserOptions.add(this.mRadiusInput);
		local button = this.GUI.Button("Refresh");
		button.setReleaseMessage("onRefresh");
		button.addActionListener(this);
		local floorButton = this.GUI.Button("Snap to Floor");
		floorButton.setReleaseMessage("onSnapToFloor");
		floorButton.addActionListener(this);
		local c = this.GUI.Container(this.GUI.BoxLayout(), [
			SceneryBrowserOptions,
			this.GUI.Spacer(10, 10),
			button,
			floorButton
		]);
		cmain.add(c, this.GUI.BorderLayout.NORTH);
		this.mOutputList = this.GUI.ColumnList();
		this.mOutputList.addColumn("ID", 50);
		this.mOutputList.addColumn("Type", 120);
		this.mOutputList.addColumn("Position", 200);
		this.mOutputList.setMultipleSelectionCapable(true);
		this.mOutputList.addActionListener(this);
		cmain.add(this.GUI.ScrollPanel(this.mOutputList), this.GUI.BorderLayout.CENTER);

		if (::_buildTool == null || ::_buildTool.mBuildNode == null)
		{
			this.log.error("The Scenery Object Browser tool can only be used in Build Mode");
			this.mOutputList.addRow([
				"",
				"The scenery object",
				"browser tool can only be used in Build Mode"
			]);
		}
		else
		{
			button._fireActionPerformed("onRefresh");
		}
	}

	function onRefresh( button )
	{
		local AssetList = this._sceneObjectManager.findSceneryObjects(this.mRadiusInput.getText());
		this.mOutputList.removeAllRows();
		this.mIDList.clear();

		if (AssetList == null)
		{
			return;
		}

		foreach( asset in AssetList )
		{
			if (this.mFilterInput.getText() != "" && asset.getType().tostring().tolower().find(this.mFilterInput.getText().tolower()) == null)
			{
				continue;
			}

			local parens = false;
			local floorY = this.Util.getFloorHeightAt(asset.getPosition(), 400000.0, this.QueryFlags.FLOOR);

			if (floorY != null)
			{
				if (floorY > asset.getPosition().y)
				{
					parens = true;
				}
			}

			local Position = "";

			if (parens == true)
			{
				Position += "(";
			}

			Position += asset.getPosition().x.tostring() + "     " + asset.getPosition().y.tostring() + "     " + asset.getPosition().z.tostring();

			if (parens == true)
			{
				Position += ")";
			}

			this.mIDList.push(asset.getID());
			this.mOutputList.addRow([
				asset.getID().tostring(),
				asset.getType().tostring(),
				Position
			]);
		}
	}

	function onSnapToFloor( button )
	{
		foreach( id in this.mIDList )
		{
			local so = this._sceneObjectManager.getSceneryByID(id);

			if (so)
			{
				local newPos = this.Util.pointOnFloor(so.getPosition());
				so.setPosition(newPos);
			}
		}
	}

	function onRowSelectionChanged( list, index, selected )
	{
		if (list == this.mOutputList)
		{
			foreach( SceneNode in ::_buildTool.getSelectedObjects() )
			{
				::_buildTool.selectionRemove(SceneNode);
			}

			foreach( entry in this.mOutputList.getSelectedRows() )
			{
				local Asset = this._sceneObjectManager.getSceneryByID(this.mIDList[entry]);
				::_buildTool.selectionAdd(Asset);
			}
		}
	}

	function isVisible()
	{
		return this.GUI.Frame.isVisible();
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);
	}

	function destroy()
	{
		return this.GUI.Frame.destroy();
	}

}

