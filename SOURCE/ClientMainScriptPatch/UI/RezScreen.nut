
require( "UI/Screens" );

/**
	This screen will offer the user ways to resurrect.
		
	@author Emilio Santos
*/
class Screens.RezScreen extends GUI.Container
{
	ResurrectInfo =
	{
		[0] = { title = "Revive", cost = 0, onPress = "onAcceptWeakRez", currencyComponent = null, button = null,
				tooltip = "Penalty: Heroism is reduced by 50%.<br />No penalty in groves."},
		[1] = { title = "Resurrect", cost = 0, onPress = "onAcceptNormalRez",  currencyComponent = null, button = null,
				tooltip = "Penalty: Heroism is reduced by 10%. Moderate coin fee.<br />No penalty in groves."},
		[2] = { title = "Rebirth", cost = 0, onPress = "onAcceptImprovedRez", currencyComponent = null, button = null,
				tooltip = "Penalty: No heroism loss, but highest coin fee.<br />No penalty in groves."},
	}
	
	constructor( )
    {   	   	   	 	
    	GUI.Container.constructor(null);
    	setPassThru(false);
		setSize( 516, 412 );
		setPreferredSize( 516, 412 );
		centerOnScreen();
		setAppearance("ResurrectionBG");
		
		local baseHolder = GUI.Container(GUI.BoxLayoutV());
		baseHolder.setSize(223, 216);
		baseHolder.setPreferredSize(223, 216);
		baseHolder.setPosition(147, 197);
		baseHolder.getLayoutManager().setGap(7);
		add(baseHolder);
		
		local titleLabel = GUI.Label("You have been defeated!");
		titleLabel.setFontColor(Colors["white"]);
		titleLabel.setFont(::GUI.Font("Maiandra", 20));
		baseHolder.add(titleLabel);
		
		foreach(resOption in ResurrectInfo)
		{
			local button = GUI.Button("");
			button.setLayoutManager(null);
			button.setUseMouseOverEffect(true);
			button.setFixedSize(191, 54);
			button.setAppearance("ResurrectionButton");
			local toolTipComp = GUI.HTML(resOption.tooltip);
			button.setTooltip(toolTipComp);
			button.setPressMessage( resOption.onPress )
			button.setSelection(true);
			button.addActionListener(this);
			button.setEnabled(false);
			
			resOption.button = button;
			
			local insideComp = GUI.Container(GUI.BoxLayoutV());
			insideComp.setSize(170, 54);
			insideComp.setPreferredSize(170, 54);
			insideComp.setPosition(10, 0);
			insideComp.getLayoutManager().setGap(-5);
			insideComp.setPassThru(true);
			button.add(insideComp);
			
			local buttonTitle = GUI.Label(resOption.title);
			buttonTitle.setFontColor(Colors["white"]);
			buttonTitle.setFont(::GUI.Font("Maiandra", 36));
			insideComp.add(buttonTitle);
			
			resOption.currencyComponent = GUI.Currency();
			insideComp.add(resOption.currencyComponent);
			
			baseHolder.add(button);
		}
    }
    
    function setVisible(value)
    {
    	GUI.Container.setVisible(value);
    	if(value == true)
    	{
    		setResurrectButtonsEnabled(false);
    		//Need to query the server to figure out how much it will cost to revive
    		::_Connection.sendQuery("persona.resCost", this);
    	}
    }
    
    function onQueryComplete(qa, rows)
    {
    	if(qa.query == "persona.resCost")
    	{
    		ResurrectInfo[1].currencyComponent.setCurrentValue(rows[0][0].tointeger());
    		ResurrectInfo[2].currencyComponent.setCurrentValue(rows[1][0].tointeger());
    		
    		setResurrectButtonsEnabled(true);
    	}
    }
    
    function onQueryError()
    {
    }
    
    function start()
    {
    	_enterFrameRelay.addListener(this);
    }
    
    function close()
    {
		_enterFrameRelay.removeListener(this);

    	GUI.Frame.close();
    }
    
    /**
    	This will set all of the resurrection buttons either enabled or not disabled, though
    	there is an exception with setting them visible.  If you pass in true to this function
    	it will first consider the amount of copper the player has.  If they don't have 
    	enough to resurrect, the button will set not disabled.
    	
    	@param value - true to set buttons enabled (see the notes above), false to set them disabled.
    	
    	@author Ryne Anderson
    */
    function setResurrectButtonsEnabled(value)
    {
	    foreach(resOption in ResurrectInfo)
		{
			//Need to ensure they have the money to choose this option though...
			//otherwise we cannot enable it
			local copper = ::_avatar.getStat(Stat.COPPER);

			//PlanetForever note: probably not needed, but a failsafe.
			if(copper == null)
				copper = 0;

			//if(copper && copper >= resOption.currencyComponent.getCurrentValue())

			//PlanetForever note: The above condition was failing if
			//copper was zero, disabling ALL rez options.  Very bad
			//for new players.
			if(copper >= resOption.currencyComponent.getCurrentValue())
				resOption.button.setEnabled(value);
			else
				resOption.button.setEnabled(false);
		}
    }
    
    function _doResurrect(abId)
    {
    	local ab = _AbilityManager.getAbilityById( abId );
    	if (ab)
    	{
    		ab.mValid = true; //we know these are good
    		ab.sendActivationRequest();
    		
  //  		setResurrectButtonsEnabled(false);
    	}
    }

    function onAcceptWeakRez(evt)
    {
    	_doResurrect(10000);
    }
    
    function onAcceptNormalRez(evt)
    {
    	_doResurrect(10001);
    }
    
    function onAcceptImprovedRez(evt)
    {
    	_doResurrect(10002);
    }
    
	function onClosePressed( )
	{
   		_doResurrect(10000);
   		GUI.Frame.onClosePressed( );
	}
	
	/**
		Every frame we must check if the rez timer is up.
	
		@author Emilio Santos
	*/
	function onEnterFrame()
	{
		if ( ::_avatar == null )
			return;

		if( !::_avatar.isDead() )
		{
			close();
		}
	}
	
	function destroy( )
	{
		_enterFrameRelay.removeListener(this);
		GUI.Panel.destroy( );
	}
	
}
