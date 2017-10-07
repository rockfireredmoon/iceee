this.require("UI/Screens");
this.require("UI/ActionContainer");

class Screens.ServerForm extends GUI.Frame {

	/* Screen class name */
	static mClassName = "Screens.ServerForm";
	
	mValueGetters = [];
	mFormId = null;
	
	constructor(formId, formTitle, formDescription, form) {
		GUI.Frame.constructor(formTitle);

		this.mFormId = formId;
				
		local rows = 0;
		local rowh = 0;
		local roww = 0;
		local cols = 0;
		
		/* First pass to get the grid size, total height / width etc */
		foreach(group in form) {
			rows += group.items.len();
			foreach(rowIdx, row in group.items) {
				if(row.height == 0)
					row.height = 24;
					
				rowh += row.height;
				local tw = 0;
				if(row.items.len() > cols)
					cols = row.items.len(); 
				foreach(colIdx, item in row.items) {
					if(item.width == 0) {
						switch(item.type) {
						case 1:
							item.width = 100;
							break;
						case 2:
							item.width = 100;
							break;
						case 3:
							item.width = 22;
							break;
						case 4:
							item.width = 80;
							break;
						}
					}
					tw += item.width;
				}
				if(tw > roww)
					roww = tw;
			}
		}
		local formPanel = GUI.Container(GUI.GridLayout(rows, cols));
		
		/* TODO make constants
		BLANK = 0,
	LABEL = 1,
	TEXTFIELD = 2,
	CHECKBOX = 3,
	BUTTON = 4
		*/
		
		formPanel.setInsets(4);
		
		/* Second pass to add the form components */
		local rowIdx = 0;
		foreach(group in form) {
			foreach(row in group.items) {
				local rh = row.height;
				foreach(colIdx, item in row.items) {
					local cons = {
						span = item.cells
					};
					switch(item.type) {
					case 0:
						formPanel.add(GUI.Label(), cons);
						break;			
					case 1:
						local lb = GUI.Label(item.value);
						formPanel.add(lb, cons);
						break;			
					case 2:
						local ia = this.GUI.InputArea();
						ia.setData(item);
						formPanel.add(ia, cons);						
						this.mValueGetters.append(ia);
						break;
					case 3:
						local cb = GUI.CheckBox(item.value == "true" || item.value == "TRUE" || item.value == "1");
						formPanel.add(cb, cons);
						cb.setData(item);
						rh = 22;
						item.width = 22;
						this.mValueGetters.append(cb);
						break;
					case 4:
						local btn = item.style == "narrow" ? GUI.NarrowButton(item.value) : GUI.Button(item.value);
						btn.setData(item);
						btn.setReleaseMessage("onButtonPressed");
						btn.addActionListener(this);
						formPanel.add(btn, cons);
						break;
					}
					formPanel.getLayoutManager().setColumnSize(colIdx, item.width);	
				}
				formPanel.getLayoutManager().setRowSize(rowIdx, rh);
				rowIdx++; 
			}
		}
		
		local main = GUI.Container(GUI.BorderLayout());
		local titleBar = GUI.Label(formTitle);
		titleBar.setFont(::GUI.Font("MaiandraOutline", 24));
		main.add(titleBar, GUI.BorderLayout.NORTH);
		main.add(formPanel, GUI.BorderLayout.CENTER);
		
		setContentPane(main);
		setSize(roww + 20, rowh + 64);
		setVisible(true);
	}
	
	function onButtonPressed(button) {
		local item = button.getData();
		local l = {};
		foreach(g in this.mValueGetters) {
			if(g.getData().type == 2)
				l[g.getData().name] <- g.getText();
			else if(g.getData().type == 3)
				l[g.getData().name] <- g.getChecked() ? "true" : "false";
		}
		::_Connection.sendQuery("form.submit", this, [
			mFormId, //form
			item.name, //clicked button
			System.encodeVars(l) //string list of values
		]);
	}
}
