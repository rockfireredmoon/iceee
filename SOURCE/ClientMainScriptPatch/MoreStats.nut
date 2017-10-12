class Screens.MoreStats extends GUI.Frame
{
	static mClassName = "Screens.MoreStats";

	mButtonRefresh = null;
	mButtonClose = null;
	mColumnListResults = null;

	constructor()
	{
		GUI.Frame.constructor("More Character Stats");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		setContentPane(cmain);
		setSize(520, 350);

		cmain.add(GUI.Spacer(0, 5));
		cmain.add(_buildResultList());
		cmain.add(_buildButtonRow());

		Refresh();
	}
	function _buildButtonRow()
	{
		mButtonRefresh = GUI.NarrowButton("Refresh");
		mButtonRefresh.addActionListener(this);
		mButtonRefresh.setReleaseMessage("onButtonPressed");

		mButtonClose = GUI.NarrowButton("Close");
		mButtonClose.addActionListener(this);
		mButtonClose.setReleaseMessage("onButtonPressed");

		local container = GUI.Container();
		container.add(mButtonRefresh);
		container.add(mButtonClose);
		return container;
	}
	function _buildResultList()
	{
		local container = GUI.Container(GUI.GridLayout(1,1));
		container.getLayoutManager().setColumns(480);
		container.getLayoutManager().setRows(280);

		mColumnListResults = GUI.ColumnList();
		mColumnListResults.addColumn("Stat", 240);
		mColumnListResults.addColumn("Value", 240);
		mColumnListResults.addActionListener(this);

		container.add(GUI.ScrollPanel(mColumnListResults));
		return container;
	}
	function onQueryComplete(qa, results)
	{
		if(qa.query == "mod.morestats")
		{
			mColumnListResults.removeAllRows();

			foreach(i, r in results)
			{
				local name = r[0];
				local stat = r[1];
				mColumnListResults.addRow([name, stat]);
			}
		}
	}
	function Refresh()
	{
		::_Connection.sendQuery("mod.morestats", this, [] );
	}
	function onButtonPressed(button)
	{
		if(button == mButtonRefresh)
			Refresh();
		else if(button == mButtonClose)
			setVisible(false);
	}
}

function InputCommands::MoreStats(args)
{
	Screens.toggle("MoreStats");
}
