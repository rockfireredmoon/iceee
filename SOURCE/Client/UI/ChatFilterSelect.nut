class this.GUI.ChatFilterSelect extends this.GUI.Container
{
	mMainWindow = null;
	mComboList = null;
	constructor()
	{
		this.GUI.Container.constructor();
		this.mComboList = [];
		this.mMainWindow = this.GUI.Container(this.GUI.GridLayout(1, 2));
		local leftSide = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mMainWindow.getLayoutManager().setColumns(65, 210);
		leftSide.add(this.GUI.Label("General"));
		this.mComboList.append(this.CheckboxLabelCombo("Say", "Say"));
		this.mComboList.append(this.CheckboxLabelCombo("Emote", "Emote"));
		this.mComboList.append(this.CheckboxLabelCombo("Tell", "Tell"));
		this.mComboList.append(this.CheckboxLabelCombo("Region", "Region"));
		this.mComboList.append(this.CheckboxLabelCombo("Trade", "Trade"));
		this.mComboList.append(this.CheckboxLabelCombo("Clan", "Clan"));
		this.mComboList.append(this.CheckboxLabelCombo("Friend Notifications", "Friend Notifications"));
		this.mComboList.append(this.CheckboxLabelCombo("Clan Officer", "Clan Officer"));
		this.mComboList.append(this.CheckboxLabelCombo("Party", "Party"));
		this.mComboList.append(this.CheckboxLabelCombo("Private Channel", "Private Channel"));
		this.mComboList.append(this.CheckboxLabelCombo("System", "System"));
		local lastIndex = 0;

		while (lastIndex < this.mComboList.len())
		{
			leftSide.add(this.mComboList[lastIndex]);
			lastIndex++;
		}

		local rightSide = this.GUI.Container(this.GUI.BoxLayoutV());
		rightSide.add(this.GUI.Label("Combat"));
		this.mComboList.append(this.CheckboxLabelCombo("My Incoming", "My Combat Incoming"));
		this.mComboList.append(this.CheckboxLabelCombo("My Outgoing", "My Combat Outgoing"));
		this.mComboList.append(this.CheckboxLabelCombo("Other Incoming", "Other Player Combat Incoming"));

		for( this.mComboList.append(this.CheckboxLabelCombo("Other Outgoing", "Other Player Combat Outgoing")); lastIndex < this.mComboList.len(); lastIndex++ )
		{
			rightSide.add(this.mComboList[lastIndex]);
		}

		this.mMainWindow.add(leftSide);
		this.mMainWindow.add(rightSide);
		this.add(this.mMainWindow);
	}

	function setCategoryChecked( category )
	{
		foreach( combo in this.mComboList )
		{
			if (combo.getChatChannelType() == category)
			{
				combo.setChecked(true);
				return;
			}
		}
	}

	function getChatFilters()
	{
		return this.mComboList;
	}

}

class this.CheckboxLabelCombo extends this.GUI.Container
{
	mLabelName = null;
	mCheckbox = null;
	mChatChannelType = null;
	constructor( labelName, channelBinding )
	{
		this.GUI.Container.constructor(this.GUI.GridLayout(1, 2));
		this.mLabelName = labelName;
		this.getLayoutManager().setColumns(20, 40);
		this.getLayoutManager().setRows(20);
		this.mCheckbox = this.GUI.CheckBox();
		this.add(this.mCheckbox);
		this.add(this.GUI.Label(labelName));
		this.mChatChannelType = channelBinding;
	}

	function getChatChannelType()
	{
		return this.mChatChannelType;
	}

	function isChecked()
	{
		return this.mCheckbox.getChecked();
	}

	function setChecked( value )
	{
		this.mCheckbox.setChecked(value);
	}

}

