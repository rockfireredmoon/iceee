this.require("UI/Screens");
local wasVisible = this.Screens.close("PetitionScreen");
class this.Screens.PetitionScreen extends this.GUI.Frame
{
	mMainScreen = null;
	mPetitionCategory = null;
	mPetitionDescription = null;
	mSubmitButton = null;
	constructor()
	{
		this.GUI.Frame.constructor("Petition");
		this.mMainScreen = this._buildMainScreen();
		this.setSize(550, 400);
		this.setContentPane(this.mMainScreen);
	}

	function _buildMainScreen()
	{
		local content = this.GUI.Container(this.GUI.BorderLayout());
		local font = ::GUI.Font("Maiandra", 22);
		local info = ::GUI.HTML("<font size=\"22\">Please enter a detailed description of the problem you encountered. Please be as specific as possible." + " More detailed descriptions will help Earth Sages solve the problem quickly.</font>");
		info.setResize(true);
		info.setMaximumSize(500, null);
		info.setInsets(0, 4, 0, 4);
		content.add(info, this.GUI.BorderLayout.NORTH);
		content.setInsets(4, 4, 4, 4);
		local container = this.GUI.Container(this.GUI.GridLayout(2, 2));
		container.getLayoutManager().setColumns(75, 400);
		container.getLayoutManager().setRows(25, 200);
		container.getLayoutManager().setGaps(0, 4);
		container.setInsets(4, 8, 8, 4);
		local petitionLabel = ::GUI.Label("Category:");
		petitionLabel.setFont(font);
		this.mPetitionCategory = ::GUI.DropDownList("");

		for( local i = 0; i < this.PetitionCategory.len(); i++ )
		{
			this.mPetitionCategory.addChoice(this.PetitionCategory[i].category);
		}

		container.add(petitionLabel);
		container.add(this.mPetitionCategory);
		local descLabel = ::GUI.Label("Description:");
		descLabel.setFont(font);
		descLabel.setTextAlignment(0.0, 0.0);
		this.mPetitionDescription = ::GUI.InputArea("");
		this.mPetitionDescription.setMultiLine(true);
		container.add(descLabel);
		container.add(this.mPetitionDescription);
		content.add(container, this.GUI.BorderLayout.CENTER);
		local buttons = this.GUI.Container(this.GUI.BoxLayout());
		buttons.getLayoutManager().setPackAlignment(0.5);
		this.mSubmitButton = this.GUI.NarrowButton("Submit");
		this.mSubmitButton.setReleaseMessage("onSubmitPressed");
		this.mSubmitButton.addActionListener(this);
		buttons.add(this.mSubmitButton);
		buttons.setInsets(0, 0, 4, 0);
		content.add(buttons, this.GUI.BorderLayout.SOUTH);
		this.add(content);
		this.setContentPane(content);
		this.setInsets(4, 4, 4, 4);
		this.setSize(500, 375);
		this.center();
		this.mPetitionDescription.setText("");
		return content;
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (value)
		{
			this.mPetitionCategory.setCurrent("Character Stuck");
			this.mPetitionDescription.setText("");
			this.mSubmitButton.setEnabled(true);
		}
	}

	function onSubmitPressed( button )
	{
		if (button == this.mSubmitButton)
		{
			::_Connection.sendQuery("petition.send", this, [
				this.mPetitionCategory.getCurrentIndex(),
				this.mPetitionDescription.getText()
			]);
			this.mSubmitButton.setEnabled(false);
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "petition.send")
		{
			this.IGIS.info("Thank you, your petition has been sent.  An Earth Sage will contact you soon.");
			this.close();
			this.mSubmitButton.setEnabled(true);
		}
	}

	function onQueryError( qa, results )
	{
		if (qa.query == "petition.send")
		{
			this.IGIS.error(results);
			this.mSubmitButton.setEnabled(true);
		}
	}

}


if (wasVisible)
{
	this.Screens.toggle("PetitionScreen");
}
