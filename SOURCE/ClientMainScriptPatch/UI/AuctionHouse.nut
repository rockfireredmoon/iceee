require("UI/ActionContainer");
require("UI/Equipment");
require("UI/Screens");
class Screens.AuctionHouse extends GUI.BigFrame
{
	static mClassName = "Screens.AuctionHouse";
	
	mScreenContainer = null;
	
	constructor()
	{
		GUI.BigFrame.constructor("AuctionHouse", true, {
			x = 373,
			y = 1
		});
		mScreenContainer = GUI.Container(GUI.BoxLayout());
		mScreenContainer.setInsets(5);
		
		setPosition(400, 50);
		setContentPane(mScreenContainer);
		::_Connection.addListener(this);
		
		mTitleBar.setAppearance("VaultTop");
		mContentPane.setAppearance("VaultSides");
		
		setCached(::Pref.get("video.UICache"));
		
		::_ItemDataManager.addListener(this);
	}

	function close()
	{
		setVisible(false);
	}

	function fitToScreen()
	{
		local pos = getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - getWidth() ? pos.x : ::Screen.getWidth() - getWidth();
		pos.y = pos.y < ::Screen.getHeight() - getHeight() ? pos.y : ::Screen.getHeight() - getHeight();
		setPosition(pos);
	}

	function onQueryComplete( qa, rows )
	{
	}

	function onQueryError( qa, error )
	{
	}

	function refreshAuctionHouse()
	{
	}

	function setVisible( visible )
	{
		if (visible)
		{
			::Audio.playSound("Sound-InventoryOpen.ogg");
				GUI.Frame.setVisible(true);
		}
		else
		{
			GUI.Frame.setVisible(false);
			::Audio.playSound("Sound-InventoryClose.ogg");
		}
	}

	function _addNotify()
	{
		GUI.ContainerFrame._addNotify();
	}

	function _removeNotify() {
		close();
		GUI.ContainerFrame._removeNotify();
	}

}

