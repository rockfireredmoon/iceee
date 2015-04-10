require("InputCommands");


/* COMMANDS:
  None of these commands change anything on the server, they're purely client side.
  The client can detect some cheating movement (warps or increased run speed), but
  can't do anything about it.

  /vwarp elev                  Vertical warp to elevation (Y axis)
  /cwarp                       Warp to selected target
  /cwarp {n,s,e,w}             Warp 200 units in a direction
  /cwarp {n,s,e,w} dist        Warp a distance in direction
  /cwarp x z                   Warp to coordinates
  /swim elev                   Swim at elevation (Y axis)
  /swim                        Stop swimming
  /SetMaxSlope amount          Use 0 to walk on any slopes no matter how steep
  /SetMaxSlope                 Set default walking slope
  /LookStats                   See vital stats of current target
  /NukeAround radius           Remove all props around radius (default to 50 units)
  /Nuke ID                     Remove prop by ID
  /cspeed amount               Set bonus run speed
  /cspeed                      Return to default speed
  /SetAdmin                    Sets client admin permission to unlock client commands and speed via numpad +/- keys

*/

function SetElevation(y)
{
	local pos = _avatar.getPosition();
	pos.y = y;
	_avatar.setPosition(pos);
}

function InputCommands::vwarp(args)
{
	if(args.len() >= 1)
	{
		SetElevation(args[0].tofloat());
		IGIS.info("Warped.");
	}
}

function IsDirectional(str)
{
	if(str == "n" || str == "s" || str == "e" || str == "w")
		return true

	return false;
}

function ModifyPosition(dir, dist)
{
	local pos = _avatar.getPosition();
	if(dir == "n")
	{
		pos.z -= dist;
	}
	else if(dir == "s")
	{
		pos.z += dist;
	}
	else if(dir == "e")
	{
		pos.x += dist;
	}
	else if(dir == "w")
	{
		pos.x -= dist;
	}
	_avatar.setPosition(pos);
}

// Try to set position.
function InputCommands::cwarp(args)
{
	if(args.len() == 0)
	{
		local target = ::_avatar.getTargetObject();
		if(target)
		{
			_avatar.setPosition(target.getPosition());
			IGIS.info("Warped.");
		}
	}
	else if(args.len() == 1)
	{
		ModifyPosition(args[0].tostring(), 200);
		IGIS.info("Warped.");
	}
	else if(args.len() == 2)
	{
		if(IsDirectional(args[0].tostring()) == true)
		{
			ModifyPosition(args[0].tostring(), args[1].tointeger());
			IGIS.info("Warped.");
		}
		else
		{
			local pos = _avatar.getPosition();
			pos.x = args[0].tofloat();
			pos.z = args[1].tofloat();	
			_avatar.setPosition(pos);
			IGIS.info("Warped.");			
		}
	}
}

function InputCommands::Swim(args)
{
	if(args.len() == 1)
	{
		local elev = args[0].tofloat();
		_avatar.mWaterElevation = elev;
		_avatar.mController.onStartSwimming(_avatar.mWaterElevation);
		_avatar.mSwimming = true;
		SetElevation(elev);
		IGIS.info("Swim");
	}
	else
	{
		_avatar.mController.onStopSwimming();
		_avatar.mSwimming = false;
		IGIS.info("Sink");
	}
}


function InputCommands::SetMaxSlope(args)
{
	local value = 0.65;
	if(args.len() == 1)
		value = args[0].tofloat();

	::gMaxSlope = value;
	::IGIS.info("Slope changed to to " + value);
}



function AddQuickStats(ostr,creature)
{
	ostr += (" Str:" + creature.getStat(Stat.STRENGTH, true));
	ostr += (" Dex:" + creature.getStat(Stat.DEXTERITY, true));
	ostr += (" Con:" + creature.getStat(Stat.CONSTITUTION, true));
	ostr += (" Psy:" + creature.getStat(Stat.PSYCHE, true));
	ostr += (" Spi:" + creature.getStat(Stat.SPIRIT, true));
	ostr += (" HP:" + creature.getStat(Stat.HEALTH, true));
	ostr += (" Dmg:" + creature.getStat(Stat.BASE_DAMAGE_MELEE, true));
	ostr += (" Arm:" + creature.getStat(Stat.DAMAGE_RESIST_MELEE, true));
	return ostr;
}

function InputCommands::LookStats(args)
{
	local obj = ::_avatar.getTargetObject();
	if(obj == null)
	{
		::IGIS.info("No target selected.");
		return;
	}

	local ostr = "";
	ostr = AddQuickStats(ostr, obj);
	::IGIS.info(ostr);
}

function InputCommands::NukeAround(args)
{
	local Radius = 50.0;
	if(args.len() == 1)
		Radius = args[0].tofloat();

	local avatarPos = ::_avatar.getNode().getWorldPosition();
	local AssetList = _sceneObjectManager.getAssetsAroundRadius(avatarPos, Radius);
	foreach(i, d in AssetList)
	{
		d.destroy();
	}
	IGIS.info("Nuked " + AssetList.len() + " objects.");
}

function InputCommands::Nuke(args)
{
	if(args.len() == 1)
	{
		local ID = args[0].tointeger();
		if(_sceneObjectManager.hasScenery(ID))
		{
			local so = _sceneObjectManager.getSceneryByID(ID);
			so.destroy();
			IGIS.info("Nuked.");
		}
	}
}


function InputCommands::cspeed(args)
{
	local s = 0;
	if(args.len() == 1)
		s = args[0].tointeger();
	::_avatar.setStat(Stat.MOD_MOVEMENT, s);
	IGIS.info("Speed set: " + s);
}

function InputCommands::SetAdmin(args)
{
	::_accountPermissionGroup = "admin";
	IGIS.info("Permission set.");
}

