this.require("GUI/Panel");
this.require("UI/Screens");
class this.Screens.HengeSelectionScreen extends this.GUI.Frame
{
	mSourceHengeId = -1;
	mHengeList = null;
	mHengeColumnList = null;
	mAcceptButton = null;
	mDeclineButton = null;
	constructor()
	{
		this.GUI.Frame.constructor("Henge Selection");
		this.setSize(300, 250);
		this.mHengeList = [];
		local pageContainer = this.GUI.Container(this.GUI.GridLayout(2, 1));
		pageContainer.setSize(300, 200);
		pageContainer.getLayoutManager().setRows(200, 25);
		this.mHengeColumnList = this.GUI.ColumnList();
		this.mHengeColumnList.addColumn("Destination", 150);
		this.mHengeColumnList.addColumn("Cost", 150);
		local buttonContainer = this.GUI.Container(this.GUI.GridLayout(1, 2));
		this.mAcceptButton = this.GUI.Button("Accept");
		this.mAcceptButton.setReleaseMessage("onAccept");
		this.mAcceptButton.addActionListener(this);
		this.mDeclineButton = this.GUI.Button("Decline");
		this.mDeclineButton.setReleaseMessage("onDecline");
		this.mDeclineButton.addActionListener(this);
		buttonContainer.add(this.mAcceptButton);
		buttonContainer.add(this.mDeclineButton);
		pageContainer.add(this.mHengeColumnList);
		pageContainer.add(buttonContainer);
		this.setContentPane(pageContainer);
	}

	function addHenges( sourceHengeId, hengeList )
	{
		this.mSourceHengeId = sourceHengeId;
		this.mHengeList = hengeList;
		this.mHengeColumnList.removeAllRows();

		foreach( henge in this.mHengeList )
		{
			this.mHengeColumnList.addRow([
				henge.name,
				this.GUI.Currency(henge.cost)
			]);
		}
	}

	function onAccept( button )
	{
		if (button == this.mAcceptButton)
		{
			local selectedRow = this.mHengeColumnList.getSelectedRows();

			if (selectedRow.len() > 0)
			{
				local hengeDest = this.mHengeList[selectedRow[0]];
				local callback = {
					selectedDest = hengeDest,
					sourceHengeId = this.mSourceHengeId,
					function onActionSelected( mb, alt )
					{
						if (alt == "Yes")
						{
							local test = ::_avatar.getStat(this.Stat.COPPER);

							if (::_avatar.getStat(this.Stat.COPPER) >= this.selectedDest.cost)
							{
								::_Connection.sendQuery("henge.setDest", this, [
									this.selectedDest.name,
									this.sourceHengeId
								]);
								local hengescreen = this.Screens.get("HengeSelectionScreen", false);

								if (hengescreen)
								{
									hengescreen.setVisible(false);
								}
							}
							else
							{
								this.IGIS.error("You lack the funds to travel to " + this.selectedDest.name + "!");
							}
						}
					}

				};
				local messageContainer = this.GUI.Container();
				messageContainer.add(this.GUI.Label("Traveling to " + hengeDest.name + " will cost"));
				local currency = this.GUI.Currency(hengeDest.cost);
				currency.setAlignment(1);
				messageContainer.add(currency);
				messageContainer.add(this.GUI.Label(". Do you wish to proceed?"));
				this.GUI.MessageBox.showYesNo(messageContainer, callback);
			}
		}
	}

	function onDecline( button )
	{
		if (button == this.mDeclineButton)
		{
			this.setVisible(false);
		}
	}

}

