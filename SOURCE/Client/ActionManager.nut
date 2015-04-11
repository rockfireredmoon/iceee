this.require("Constants");
class this.ActionManager 
{
	constructor()
	{
	}

}

class this.ActionSound 
{
	mAction = null;
	constructor()
	{
	}

	function setAction( action )
	{
		this.mAction = action;
	}

	function getSound()
	{
		return null;
	}

}

class this.CoolDownAbilityActionSound extends this.ActionSound
{
	mCoolDownInEffectSound = null;
	constructor( action )
	{
		this.ActionSound.constructor();
		this.mAction = action;
		this.mCoolDownInEffectSound = "Sound-Spelltimer.ogg";
	}

	function getSound()
	{
		local remainingTime = ::_AbilityManager.getRemainingCoolDownTime(this.mAction);

		if (!remainingTime)
		{
			return null;
		}

		return remainingTime > 0 ? this.mCoolDownInEffectSound : null;
	}

}

class this.ToggleMeleeAbilityActionSound extends this.ActionSound
{
	mToggleOnSound = null;
	mToggleOffSound = null;
	constructor( action )
	{
		this.ActionSound.constructor();
		this.mAction = action;
	}

	function getSound()
	{
		return ::_avatar.isMeleeAutoAttackActive() ? this.mToggleOnSound : this.mToggleOffSound;
	}

}

class this.Action 
{
	mName = "";
	mForegroundImage = "";
	mBackgroundImage = "";
	mBroadcaster = null;
	mActionSound = null;
	constructor( name, image )
	{
		this.mName = name;
		this.setImage(image);
		this.mBroadcaster = ::MessageBroadcaster();
	}

	function activate()
	{
		return false;
	}

	function setActionSound( actionSound )
	{
		this.mActionSound = actionSound;
		this.mActionSound.setAction(this);
	}

	function getSound()
	{
		if (this.mActionSound)
		{
			return this.mActionSound.getSound();
		}
		else
		{
			return null;
		}
	}

	function addActionListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function broadcastMessage( messageName, sender, ... )
	{
		local args = [];

		for( local i = 0; i < vargc; i++ )
		{
			args.append(vargv[i]);
		}

		this.mBroadcaster.broadcastMessage(messageName, sender, args);
	}

	function cancel()
	{
	}

	function modifiedAction( actionbutton, shift, alt, control )
	{
		return;
	}

	function isAvailableForUse()
	{
		return true;
	}

	function isAwaitingServerResponse()
	{
		return false;
	}

	function isUsable()
	{
		return false;
	}

	function getIsValid()
	{
		return true;
	}

	function getCooldownCategory()
	{
		return null;
	}

	function isWarmingUp()
	{
		return false;
	}

	function getBackgroundImage()
	{
		return this.mBackgroundImage;
	}

	function getEquipmentType()
	{
		return this.ItemEquipSlot.NONE;
	}

	function getForegroundImage()
	{
		return this.mForegroundImage;
	}

	function getImage()
	{
		return this.mForegroundImage + "|" + this.mBackgroundImage;
	}

	function getInfoPanel( mods )
	{
		return null;
	}

	function getName()
	{
		return this.mName;
	}

	function getNumStacks()
	{
		return 1;
	}

	function getPopupGui()
	{
		return null;
	}

	function getRange()
	{
		return -1;
	}

	function getTooltip( mods )
	{
		return "";
	}

	function getTimeUntilAvailable()
	{
		return 0;
	}

	function getTimeUsed()
	{
		return 0;
	}

	function getType()
	{
		return "unknown";
	}

	function getQuickbarString()
	{
		return null;
	}

	function getQuickBarAction()
	{
		return this;
	}

	function getUseType()
	{
		return this.ItemType.UNKNOWN;
	}

	function getWarmupDuration()
	{
		return 0;
	}

	function getWarmupEndTime()
	{
		return 0;
	}

	function getWarmupStartTime()
	{
		return 0;
	}

	function getWarmupTimeLeft()
	{
		return 0;
	}

	function removeActionListener( listener )
	{
		if (this.mBroadcaster)
		{
			this.mBroadcaster.removeListener(listener);
		}
	}

	function setImage( image )
	{
		local splitImages = this.Util.split(image, "|");
		this.mForegroundImage = splitImages[0];

		if (splitImages.len() > 1 && splitImages[1] != "")
		{
			if (splitImages[1].find(".png") != null)
			{
				this.mBackgroundImage = splitImages[1];
			}
			else
			{
				this.mBackgroundImage = this.BackgroundImages[splitImages[1].toupper()];
			}
		}
		else
		{
			this.mBackgroundImage = this.BackgroundImages.GREY;
		}
	}

	function setName( name )
	{
		this.mName = name;
	}

	function split( newStackSize )
	{
		throw this.Exception("Splitting not supported");
	}

	function showExtraDataScreen()
	{
		return null;
	}

}

