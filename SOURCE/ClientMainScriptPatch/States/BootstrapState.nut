this.require("States/StateManager");
class this.States.BootstrapState extends this.State
{
	static mClassName = "BootstrapState";
	constructor()
	{
	}

	function onEnter()
	{
	
		::LoadGate.Require("Login", this);
		::setLocale("en");
		local camera = ::_scene.getCamera("Default");
		local cameraNode = ::_scene.getRootSceneNode().createChildSceneNode("Default");
		cameraNode.attachObject(camera);
		::_CameraObject <- camera;
		::_Camera <- cameraNode;
		camera.setQueryFlags(0);
		camera.setFarClipDistance(this.gCamera.farClippingDistance);
		camera.setVisibilityFlags(0);
		::_scene.setComponentDefaults({
			renderDist = 2500.0,
			Entity = {
				queryFlags = this.QueryFlags.LIGHT_OCCLUDER | this.QueryFlags.ANY
			}
		});
	}

	function onPackageComplete( name )
	{
		local asd = ::AssetDependencies;
		::MediaCaseIndex <- {};

		foreach( k, v in ::MediaIndex )
		{
			::MediaCaseIndex[k.tolower()] <- k;
		}

		if (this.Util.isDevMode())
		{
			local t0 = this.System.currentTimeMillis();
			this.updateDependencies();
			local t1 = this.System.currentTimeMillis();
			this.log.info("Asset Dependencies calculated in " + (t1 - t0) / 1000.0 + " second(s)");

			if ("generate_deps" in ::_args)
			{
				local out = "// This is an automatically generated file. Do not touch!\n";
				out += "::AssetDependencies <- " + this.serialize(::AssetDependencies) + ";";
				::System.writeToFile("../../../EE/Media/Catalogs/AssetDependencies.nut", out);
				::System.exit();
			}
		}

		switch(0)
		{
		case 1:
			this.require("GUI/KitchenSink");
			this.KitchenSink();
			break;

		case 2:
			this.require("GUI/CreatureTweak");
			this.GetAssembler("Creature", "Foo");
			::GUI.CreatureTweak.show("Foo");
			break;

		case 3:
			this.require("UI/BuildScreen");
			this.Screens.show("BuildScreen");
			break;

		default:
			local nextState;

			if (this.quickLogin())
			{
				nextState = this.States.LoginState();
			}
			else if ("web_auth_token" in ::_args)
			{
				nextState = this.States.WebAuthState();
				this.log.debug("ENTERING WEB AUTH STATE");
			}
			else
			{
				nextState = this.States.LoginState();
				this.log.debug("ENTERING LOGIN STATE");
			}

			::_stateManager.pushState(nextState);
		}
	}

	mArchive = "";
	mAsset = "";
	mDeps = {};
	function updateDependencies()
	{
		this.log.debug("Updating asset dependencies...");

		foreach( k, v in ::ComponentIndex )
		{
			if (k.find("Par-") == 0 || k.find("Sound-") == 0 || k.find("Light-") == 0 || k.find("Manipulator-") == 0)
			{
				continue;
			}

			local dash = k.find("-");

			if (dash == null)
			{
				continue;
			}

			local n = k.slice(0, dash);
			local path = "../../../EE/Media/" + n + "/" + k + "/" + k + ".csm.xml";
			this.mArchive = v == "" ? k : v;
			this.mAsset = k;

			try
			{
				local csm = ::System.readFile(path);
				this.parseXML(csm, this);
				local l = [];

				foreach( dn, dv in this.mDeps )
				{
					l.append(dn);
				}

				if (l.len() > 0)
				{
					::AssetDependencies[k] <- l;
				}
				else if (k in ::AssetDependencies)
				{
					delete ::AssetDependencies[k];
				}

				this.mDeps = {};
			}
			catch( err )
			{
				this.log.debug("WARNING: Unable to load CSM file: " + path + ", " + err);
			}
		}

		foreach( k, v in ::AssetDependencies )
		{
			foreach( d in v )
			{
				if ((d in ::MediaIndex) == false)
				{
					this.log.debug("ERROR - BAD DEPENDENCY: asset " + k + ", package: " + d);
				}
			}
		}
	}

	function onXmlAttribute( key, value )
	{
		local ovalue = value;
		
		if (key == "cref" || key == "mesh")
		{
			if (value.find("Par-") == 0 || value.find("Light-") == 0 || value.find("Manipulator-") == 0 || value.find("Sound-") == 0 || value.find("-LOD") != null || value.find("$(") != null || value.find("-WalkMesh.mesh") != null || value.find("-Blocking.mesh") != null || value == "")
			{
				return;
			}

			local dm = value.find(".mesh");

			if (dm != null)
			{
				local packageEnd = this.Util.rfind(value, "-");

				if (packageEnd != null)
				{
					local dash = value.find("-");
					local n = value.slice(0, dash);
					local packageName = value.slice(0, packageEnd);

					if (this.Util.fileExists("../../../EE/Media/" + n + "/" + this.mAsset + "/" + value))
					{
						value = this.mAsset;
					}
					else if (this.Util.fileExists("../../../EE/Media/" + n + "/" + packageName + "/" + value))
					{
						value = packageName;
					}
					else
					{
						value = value.slice(0, dm);
					}
				}
				else
				{
					value = value.slice(0, dm);
				}
			}
			
			local archive = this.GetArchiveName(value);

			if (archive != this.mArchive)
			{
				if ((archive in ::MediaIndex) == false)
				{
					this.log.debug("WARNING: Invalid archive name: " + archive + ", resolved for ref: " + ovalue + " in asset " + this.mAsset);
					return;
				}

				this.mDeps[archive] <- true;
			}
		}
	}

	function onPackageError( name, error )
	{
		::Screen.setBackgroundColor(this.Color(0.89999998, 0.0, 0.0));
		this.log.debug("[ERROR] Error loading package: " + name + ": " + error);
		this.UI.FatalError(error);
	}

	function quickLogin()
	{
		if (!("gQuickLogin" in this.getroottable()) || !::gQuickLogin)
		{
			return false;
		}

		local creds = ::Pref.get("login.Credentials");

		if (creds != "")
		{
			return true;
		}

		return false;
	}

	function onConfirmation( sender, bool )
	{
		if (("_StatusWindow" in this.getroottable()) && sender == ::_StatusWindow)
		{
			if (!bool)
			{
				this.onLoginCancel();
			}
		}
	}

	function onDestroy()
	{
	}

	function onScreenResize( width, height )
	{
	}

}

