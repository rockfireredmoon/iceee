this.require("LightingManager");
this.require("Globals");
this.require("EventScheduler");
this.Environments <- {};
this.Environments.Default <- {
	Sun = [
		0.1,
		0.1,
		0.1
	],
	Ambient = [
		0.60000002,
		0.60000002,
		0.60000002
	],
	Sky = [],
	Fog = {
		color = [
			0.69,
			0.92000002,
			1.0
		],
		exp = 0.001,
		start = 0.30000001,
		end = 0.94999999
	},
	Adjust_Channels = [],
	Ambient_Sound = [],
	Ambient_Music = [],
	Activate_Music = []
};
class this.Environment 
{
	DEFAULT_FADE_TIME = 2.5;
	mSun = null;
	mSkyIndex = 0;
	mDefault = "Default";
	mOverride = null;
	mTimeOfDay = "Day";
	mMarkers = {};
	mUpdateTimer = null;
	mBlendStart = 0;
	mBlendPos = 0;
	mBlendEnd = 0;
	mCurrentName = null;
	mCurrent = null;
	mCurrentSkies = null;
	mPrevious = null;
	mPreviousSkies = null;
	mLastMarkerSeq = null;
	mLastTimeOfDay = null;
	mNextAmbientTime = null;
	mNextAmbientMusicDelay = 0.0;
	mAmbientMusicSchedule = null;
	mCurrentAmbientMusic = null;
	mCurrentAmbientSound = null;
	mRandom = null;
	mCurrentMusic = null;
	mCurrentNoise = null;
	mForceNextUpdate = false;
	mForceFogUpdate = false;
	mUsingDefault = false;
	mLastTerrain = null;
	mZoneEnv = "";
	mWeatherType = this.WeatherType.FINE;
	mWeatherWeight = this.WeatherWeight.LIGHT;
	mWeatherEffect = null;
	mMusicCooldowns = null;
	mLastTimeOfDayOveride = null;
	mOverideTimeOfDay = false;
	mSetWeatherOnAvatar = false;
	
	constructor()
	{
		this.mRandom = this.Random();
		this.mCurrentSkies = [];
		this.mPreviousSkies = [];
		this.mForceNextUpdate = true;
		this.setCurrent(this.mDefault);
		this.mMarkers = {};
		this.mMusicCooldowns = {};

		if (!this.mSun)
		{
			this.mSun = ::_LightingManager.createSource("Sun", this.VisibilityFlags.LIGHT_GROUP_0);
			this.mSun.setLightType(this.Light.DIRECTIONAL);
			local sunAngle = ::Quaternion(::Math.ConvertPercentageToRad(0.75), this.Vector3(0.0, 1.0, 0.0)) * ::Quaternion(::Math.ConvertPercentageToRad(0.35499999), this.Vector3(1.0, 0.0, 0.0));
			this.mSun.getParentSceneNode().setOrientation(sunAngle);
			this.mSun.setSpecularColor(::Color(1.0, 1.0, 1.0, 1.0));
		}

		::_Connection.addListener(this);
	}

	function onPackageComplete( pkg )
	{
		this.update(true);
	}

	function setAutoUpdate( value )
	{
		if (value && this.mUpdateTimer == null)
		{
			this.mUpdateTimer = this._eventScheduler.repeatIn(1.0, 1.0, this, "update");
		}
		else if (!value && this.mUpdateTimer != null)
		{
			this._eventScheduler.cancel(this.mUpdateTimer);
			this.mUpdateTimer = null;
			::_audioManager.stopMusic(::Audio.AMBIENT_CHANNEL_2, 1.0);
			::_audioManager.stopMusic(::Audio.AMBIENT_CHANNEL, 1.0);
			::_audioManager.stopMusic(::Audio.DEFAULT_CHANNEL, 1.0);
			::_audioManager.stopMusic(::Audio.NOISE_CHANNEL, 1.0);

			if (this.mAmbientMusicSchedule != null)
			{
				::_eventScheduler.cancel(this.mAmbientMusicSchedule);
				this.mAmbientMusicSchedule = null;
			}

			this.mMusicCooldowns = {};
			this.mCurrentNoise = null;
			this.mCurrentAmbientSound = null;
			this.mCurrentAmbientMusic = null;
			this.mCurrentMusic = null;
		}
	}

	function setDefault( name )
	{
		if (this.mDefault == name)
		{
			return;
		}

		this.log.debug("Setting Environment Default: " + name);
		this.mDefault = name;

		if (!this.mDefault)
		{
			this.mDefault = "Default";
		}

		if (!(this.mDefault in ::Environments))
		{
			this.mDefault = "Default";
		}

		this.mForceNextUpdate = true;
	}

	function setOverride( name, ... )
	{
		this.log.debug("Setting Environment Override: " + name);

		if (name == null)
		{
			this.mOverride = null;
			this.update();
			return;
		}

		if (!(name in ::Environments))
		{
			name = "Default";
		}

		this.mOverride = name;
		this.mPrevious = this.mCurrent;
		this.mForceNextUpdate = true;
		this.setCurrent(name);
		this._activate(0);
	}

	function setTimeOfDay( name )
	{
		if (!this.mOverideTimeOfDay)
		{
			this.mTimeOfDay = name;
			this.mForceNextUpdate = true;
		}
		this.mLastTimeOfDayOveride = name;
	}

	function setOverrideTimeOfDay( name )
	{
		if (!this.Util.isDevMode())
		{
			return;
		}

		if (!this.mOverideTimeOfDay)
		{
			this.mLastTimeOfDayOveride = this.mTimeOfDay;
		}

		this.mTimeOfDay = name;
		this.mForceNextUpdate = true;
		this.mOverideTimeOfDay = true;
	}

	function turnOffTimeOfDayOveride()
	{
		if (!this.Util.isDevMode())
		{
			return;
		}

		this.mTimeOfDay = this.mLastTimeOfDayOveride;
		this.mForceNextUpdate = true;
		this.mOverideTimeOfDay = false;
	}

	function isMarker( so )
	{
		return so && so.isScenery() && so.getType() == "Environment-Marker";
	}

	function addMarker( marker )
	{
		local vars = marker.getVars();

		if (!vars || !("type" in vars))
		{
			this.IGIS.error("Environment marker does not specify \'type\': " + marker);
			return;
		}

		this.mMarkers[marker.getID()] <- marker;
	}

	function onThunder( thunderWeight )
	{
		local number = this._random.nextInt(3);
		local emitter = this.Audio.prepareSound("Sound-Ambient-Thunder" + (number + 1) + ".ogg");
		emitter.setGain(0.33 * (thunderWeight + 1).tofloat() );
		emitter.play();
	}
	
	function onAvatarSet() {
		if(this.mSetWeatherOnAvatar) {
			print("ICE! now got avatar, setting weather to " + this.mWeatherType);
			if(this.mWeatherType != this.WeatherType.FINE)  
				_doSetWeather();
			this.mSetWeatherOnAvatar = false;
		}
	}
	
	function _doSetWeather() {
		
		local fx = "";
		switch(this.mWeatherWeight) {
		case this.WeatherWeight.LIGHT:
			fx += "Light";
			break;
		case this.WeatherWeight.MEDIUM:
			fx += "Medium";
			break;
		case this.WeatherWeight.HEAVY:
			fx += "Heavy";
			break;
		}
		
		switch(this.mWeatherType) {
		case this.WeatherType.RAIN:
			fx += "Rain";
			break;
		case this.WeatherType.SNOW:
			fx += "Snow";
			break;
		case this.WeatherType.SAND:
			fx += "Sand";
			break;
		case this.WeatherType.HAIL:
			fx += "Hail";
			break;
		case this.WeatherType.LAVA:
			fx += "Lava";
			break;
		}
		
		print("ICE! cueing effect " + fx);
		mWeatherEffect = ::_avatar.cue(fx);
		if(mWeatherEffect != null) {
			mWeatherEffect.loopForever();
			mSetWeatherOnAvatar = false;
		}
		else {
			print("ICE! huh? no effect " + fx);
			mSetWeatherOnAvatar = true;
		}
		
		local snd = "Sound-Ambient-" + fx + ".ogg";
		this.Audio.playMusic(snd, this.Audio.WEATHER_CHANNEL);
	}

	function onWeatherUpdate( weatherType, weatherWeight )
	{
		if(weatherType != mWeatherType || weatherWeight != mWeatherWeight) 
		{
			this.mWeatherType = weatherType;
			this.mWeatherWeight = weatherWeight;
			
			if(this.mWeatherEffect != null) {
				print("ICE! Stopping weather");
				foreach(e in this.mWeatherEffect.mEffects) {
					e.stop();
				}
				this.mWeatherEffect.stop();
				this.mWeatherEffect = null;
				print("ICE! Stopped weather");
			}
			else {
				print("ICE! no weather to stop!?");
			}
				
			if(this.mWeatherType != this.WeatherType.FINE) {
				this._doSetWeather();				
			}
			else
				this.Audio.stopMusic(this.Audio.WEATHER_CHANNEL);
		}
	}
	
	function onEnvironmentUpdate( zoneId, zoneDefId, zonePageSize, mapName, envType )
	{
		if (this.mZoneEnv != envType)
		{
			this.mZoneEnv = envType;
			this.mForceNextUpdate = true;
		}
	}

	function removeMarker( marker )
	{
		if (marker.getID() in this.mMarkers)
		{
			delete this.mMarkers[marker.getID()];
		}
	}

	function clearMarkers()
	{
		this.mMarkers = {};
	}

	function setForceFogUpdate( value )
	{
		this.mForceFogUpdate = value;
	}

	function setForceNextUpdate( value )
	{
		this.mForceNextUpdate = value;
	}

	function setCurrent( name )
	{
		if (name == null)
		{
			name = this.mDefault;
		}

		if ((name in ::Environments) == false)
		{
			this.IGIS.error("Invalid environment name: " + name);
			return;
		}

		if (name == this.mCurrentName && this.mForceNextUpdate == false)
		{
			return;
		}

		if (this.mCurrentName && ("Activate_Music_Cooldown" in this.mCurrent) && !(this.mCurrentName in this.mMusicCooldowns))
		{
			this.mMusicCooldowns[this.mCurrentName] <- ::System.currentTimeMillis() + this.mCurrent.Activate_Music_Cooldown * 1000.0;
		}

		if (this.mAmbientMusicSchedule)
		{
			::_eventScheduler.cancel(this.mAmbientMusicSchedule);
			this.mAmbientMusicSchedule = null;
		}

		this.mPrevious = this.mCurrent;
		this.mCurrent = delegate ::Environments[name] : {};
		this.mCurrentName = name;
		this.mForceNextUpdate = true;
		return true;
	}

	function getNearbyMarkers()
	{
		local avatarPos = this._avatar.getPosition();
		local results = [];

		foreach( id, m in this.mMarkers )
		{
			local d = (avatarPos - m.getPosition()).length();

			if (d > m.getScale().x * 100.0)
			{
				continue;
			}

			local vars = m.getVars();
			local type = vars.type;
			local blendTime = -1.0;

			if ("blendTime" in vars)
			{
				blendTime = vars.blendTime.tofloat();
			}

			results.append([
				d,
				type,
				blendTime
			]);
		}

		results.sort(function ( a, b )
		{
			a = a[0];
			b = b[0];

			if (a < b)
			{
				return -1;
			}

			if (a > b)
			{
				return 1;
			}

			return 0;
		});
		return results;
	}

	function getTimeOfDay( env )
	{
		if (("TimeOfDay" in env) && this.mTimeOfDay in env.TimeOfDay)
		{
			local e = env.TimeOfDay[this.mTimeOfDay];

			if (e in ::Environments)
			{
				return ::Environments[e];
			}
		}

		return env;
	}

	function update( ... )
	{
		this._updateAmbientSound();

		if (!this._avatar || this.mOverride != null)
		{
			return;
		}

		if (this.mBlendPos != null && this.mBlendPos < this.mBlendEnd)
		{
			return;
		}

		local results = this.getNearbyMarkers();
		local newEnv;
		local pageEnv;
		local zoneEnv;
		this.mLastTimeOfDay = this.mTimeOfDay;
		this.mLastMarkerSeq = results;
		local avatarPos = this._avatar.getPosition();
		local dx = avatarPos.x;
		local dz = avatarPos.z;
		local tpos = this.Util.getTerrainPageIndex(avatarPos);

		if (tpos)
		{
			local pageStr = "x" + tpos.x + "y" + tpos.z;
			local terrain = ::_sceneObjectManager.getCurrentTerrainBase();
			local terrainDef = ::TerrainEnvDef;

			if (terrain in ::TerrainEnvDef)
			{
				local pageMap = ::TerrainEnvDef[terrain];
				if (pageStr in pageMap)
				{
					newEnv = pageMap[pageStr].Environment;
				}
			}
			this.mLastTerrain = tpos;
		}
		else {
			newEnv = this.mZoneEnv;
		}
		

		if (newEnv in ::Environments)
		{
			local env = ::Environments[newEnv];

			if (("TimeOfDay" in env) && this.mTimeOfDay in env.TimeOfDay)
			{
				newEnv = env.TimeOfDay[this.mTimeOfDay];
			}

			pageEnv = env;
		}
		else if (this.mZoneEnv in ::Environments)
		{
			local env = ::Environments[this.mZoneEnv];

			if (("TimeOfDay" in env) && this.mTimeOfDay in env.TimeOfDay)
			{
				newEnv = env.TimeOfDay[this.mTimeOfDay];
			}
			else
			{
				newEnv = this.mZoneEnv;
			}

			if (!(newEnv in ::Environments))
			{
				newEnv = "Default";
			}
			else
			{
				zoneEnv = ::Environments[newEnv];
			}
		}

		if (this.mForceFogUpdate)
		{
			this.mForceFogUpdate = false;
		}

		if (results.len() > 0)
		{
			newEnv = results[0][1];
		}

		this.setCurrent(newEnv);

		if (!this.mForceNextUpdate)
		{
			return;
		}

		this.mForceNextUpdate = false;
		local found = {
			Sun = false,
			Ambient = false,
			Sky = false,
			Fog = false,
			Adjust_Channels = false,
			Ambient_Noise = false,
			Ambient_Music = false,
			Activate_Music_Cooldown = 0,
			Ambient_Sound = false,
			Activate_Music = false,
			Ambient_Music_Delay = false,
			Fade_Time = false
		};
		local fieldsRemaining = found.len();
		local markersChecked = 0;

		foreach( r in results )
		{
			if ((r[1] in ::Environments) == false)
			{
				continue;
			}

			local e = this.getTimeOfDay(::Environments[r[1]]);

			foreach( f, v in e )
			{
				if (!(f in found))
				{
					continue;
				}

				if (found[f])
				{
					continue;
				}

				this.log.debug("Setting Environment.Current[" + f + "] <- " + v);
				this.mCurrent[f] <- v;
				found[f] = true;
				fieldsRemaining -= 1;
			}

			markersChecked += 1;

			if (fieldsRemaining == 0)
			{
				break;
			}
		}

		if (fieldsRemaining > 0)
		{
			pageEnv = this.getTimeOfDay(pageEnv);
			zoneEnv = this.getTimeOfDay(zoneEnv);

			foreach( f, v in ::Environments.Default )
			{
				if (!found[f] && !(f in this.mCurrent))
				{
					if (pageEnv && f in pageEnv)
					{
						this.mCurrent[f] <- pageEnv[f];
					}
					else if (zoneEnv && f in zoneEnv)
					{
						this.mCurrent[f] <- zoneEnv[f];
					}
					else
					{
						this.mCurrent[f] <- v;
					}
				}
			}
		}

		local blendTime;

		if (vargc > 0)
		{
			blendTime = vargv[0];
		}
		else if (results.len() > 0 && results[0][2] >= 0.0)
		{
			blendTime = results[0][2];
		}
		else if ("Fade_Time" in this.mCurrent)
		{
			blendTime = this.mCurrent.Fade_Time;
		}
		else
		{
			blendTime = this.DEFAULT_FADE_TIME;
		}

		this._activate(blendTime * 1000);
	}

	function _activate( blendMillis )
	{
		if (blendMillis < 0)
		{
			throw this.Exception("Invalid blend time (must be >= 0)");
		}

		foreach( node in this.mPreviousSkies )
		{
			node.destroy();
		}

		if (blendMillis > 0)
		{
			this.mPreviousSkies = this.mCurrentSkies;
		}
		else
		{
			this.mPreviousSkies = [];

			foreach( node in this.mCurrentSkies )
			{
				node.destroy();
			}
		}

		this.mCurrentSkies = [];

		foreach( sky in this.mCurrent.Sky )
		{
			this._addSky(this.mCurrentSkies, sky);
		}

		if (blendMillis > 0)
		{
			this.mBlendStart = this.System.currentTimeMillis();
			this.mBlendPos = this.mBlendStart;
			this.mBlendEnd = this.mBlendStart + blendMillis;
			this._enterFrameRelay.addListener(this);
			this._blend(0);
		}
		else
		{
			if (this.mBlendPos != null && this.mBlendPos < this.mBlendEnd)
			{
				this._enterFrameRelay.removeListener(this);
			}

			this.mBlendStart = null;
			this.mBlendPos = null;
			this.mBlendEnd = null;
			this._blend(1);
		}

		this.mNextAmbientMusicDelay = "Ambient_Music_Delay" in this.mCurrent ? this.mCurrent.Ambient_Music_Delay : 0;

		if (this.mAmbientMusicSchedule != null)
		{
			::_eventScheduler.cancel(this.mAmbientMusicSchedule);
			this.mAmbientMusicSchedule = null;
		}

		this.Audio.stopMusic(this.Audio.NOISE_CHANNEL);
		this.mCurrentNoise = null;

		if ("Ambient_Noise" in this.mCurrent)
		{
			local noise = this._randomElement(this.mCurrent.Ambient_Noise);

			if (noise != this.mCurrentNoise)
			{
				if (noise && noise != "")
				{
					this.Audio.playMusic(noise, this.Audio.NOISE_CHANNEL);
					this.mCurrentNoise = noise;
				}
			}
		}

		this.Audio.stopMusic(this.Audio.DEFAULT_CHANNEL);
		this.mCurrentMusic = null;

		if ("Activate_Music" in this.mCurrent)
		{
			local time = this.System.currentTimeMillis();
			local coolingDown = false;

			if (this.mCurrentName in this.mMusicCooldowns)
			{
				coolingDown = time < this.mMusicCooldowns[this.mCurrentName];

				if (coolingDown == false)
				{
					delete this.mMusicCooldowns[this.mCurrentName];
				}
			}

			local m = this._randomElement(this.mCurrent.Activate_Music);

			if (m && m != "")
			{
				if (!coolingDown)
				{
					this.Audio.playMusic(m, this.Audio.DEFAULT_CHANNEL);
					this.mCurrentMusic = m;
				}
			}
		}

		::_root.resetAudioChannelAdjusts();

		if ("Adjust_Channels" in this.mCurrent)
		{
			foreach( m in this.mCurrent.Adjust_Channels )
			{
				::_root.setAudioChannelAdjust(m[0], m[1]);
			}
		}

		this._audioManager.stopMusic(this.Audio.AMBIENT_CHANNEL_2);
		this._audioManager.stopMusic(this.Audio.AMBIENT_CHANNEL);
	}

	function _randomElement( array, ... )
	{
		if (typeof array == "array")
		{
			local elem = this.Util.randomElement(array);

			if (vargc > 0 && array.len() > 1)
			{
				if (elem == vargv[0])
				{
					return this._randomElement(array, vargv[0]);
				}
			}

			return elem;
		}

		return array;
	}

	function _lerpColor( a, b, t )
	{
		return this.Math.lerpColor(this._parseColor(a), this._parseColor(b), t);
	}

	function _blend( ... )
	{
		local t;

		if (vargc == 0)
		{
			t = (this.mBlendPos - this.mBlendStart).tofloat() / (this.mBlendEnd - this.mBlendStart);
		}
		else
		{
			t = this.Math.clamp(vargv[0], 0, 1);
		}

		this.mSun.setDiffuseColor(this._lerpColor(this.mPrevious.Sun, this.mCurrent.Sun, t));
		::_scene.setAmbientLight(this._lerpColor(this.mPrevious.Ambient, this.mCurrent.Ambient, t));
		local fogColor = this._lerpColor(this.mPrevious.Fog.color, this.mCurrent.Fog.color, t);
		::_scene.setFog(::Scene.FOG_LINEAR, fogColor, this.Math.lerp(this.mPrevious.Fog.exp, this.mCurrent.Fog.exp, t), ::gCamera.farClippingDistance * this.Math.lerp(this.mPrevious.Fog.start, this.mCurrent.Fog.start, t), ::gCamera.farClippingDistance * this.Math.lerp(this.mPrevious.Fog.end, this.mCurrent.Fog.end, t));
		::Screen.setBackgroundColor(fogColor);
		this._blendSkies(this.mPreviousSkies, 1.0 - t);
		this._blendSkies(this.mCurrentSkies, t.tofloat());
	}

	function _blendSkies( array, t )
	{
		foreach( node in array )
		{
			local a = node.getAttachedObjects();
			a[0].setOpacity(t);
		}
	}

	function _addSky( array, type )
	{
		this.mSkyIndex++;
		local cameraNode = this._camera.getParentSceneNode();
		local node = this._scene.createSceneNode();
		local sky = this._scene.createEntity("Sky-Sphere" + this.mSkyIndex, "Env-Sky-" + type + ".mesh");
		sky.setQueryFlags(::QueryFlags.ENVIRONMENT_OCCLUDER);
		sky.setVisibilityFlags(this.VisibilityFlags.ANY);
		node.setInheritOrientation(false);
		local sc = 1.0;
		node.setScale(this.Vector3(sc, sc, sc));
		cameraNode.addChild(node);
		node.attachObject(sky);
		array.append(node);
		sky.setRenderQueueGroup(array.len() + 1);
	}

	function _updateAmbientSound()
	{
		local time = this.System.currentTimeMillis();

		if (!this._audioManager.isMusicPlaying(this.Audio.AMBIENT_CHANNEL_2) && !this.LoadScreen.isVisible())
		{
			if (this.mNextAmbientTime == null)
			{
				this.mNextAmbientTime = time;
				local period;
				local base;

				if ("Ambient_Sound_Random" in this.mCurrent)
				{
					period = this.mCurrent.Ambient_Sound_Random[0] * 1000;
					base = this.mCurrent.Ambient_Sound_Random[1] * 1000;
				}
				else
				{
					period = 25000;
					base = 35000;
				}

				this.mNextAmbientTime += this.mRandom.nextInt(period.tointeger()) + base.tointeger();
			}
			else if (this.mNextAmbientTime >= this.System.currentTimeMillis())
			{
				if ("Ambient_Sound" in this.mCurrent)
				{
					local snd = this._randomElement(this.mCurrent.Ambient_Sound, this.mCurrentAmbientSound);

					if (snd && snd != "")
					{
						this.log.debug("Playing ambient sound: " + snd);
						this._audioManager.playMusic(snd, this.Audio.AMBIENT_CHANNEL_2);
					}

					this.mCurrentAmbientSound = snd;
					this.mNextAmbientTime = null;
				}
			}
		}

		if (this.mAmbientMusicSchedule == null && !this._audioManager.isMusicPlaying(this.Audio.AMBIENT_CHANNEL) && !this.LoadScreen.isVisible())
		{
			this.mAmbientMusicSchedule = ::_eventScheduler.fireIn(this.mNextAmbientMusicDelay, this, "playAmbientMusic");
		}
	}

	function playAmbientMusic()
	{
		local music = "Ambient_Music" in this.mCurrent ? this._randomElement(this.mCurrent.Ambient_Music, this.mCurrentAmbientMusic) : null;

		if (music && music != "")
		{
			this._audioManager.playMusic(music, this.Audio.AMBIENT_CHANNEL);
			this.mCurrentAmbientMusic = music;
		}

		this.mAmbientMusicSchedule = null;
	}

	function _parseColor( value )
	{
		switch(typeof value)
		{
		case "array":
			return this.Color(value[0], value[1], value[2]);

		case "table":
			return this.Color(value.r, value.g, value.b);

		case "string":
			return this.Color(value);

		case "instance":
			if (value instanceof this.Color)
			{
				return value;
			}
		}

		throw this.Exception("Invalid color specification: " + value);
	}

	function getSunPosition()
	{
		return this.mSun.getParentSceneNode().getPosition();
	}

	function getSunDirection()
	{
		return this.mSun.getParentSceneNode().getOrientation().rotate(this.Vector3(0, 0, -1));
	}

	function onEnterFrame()
	{
		this.mBlendPos = this.System.currentTimeMillis();

		if (this.mBlendPos >= this.mBlendEnd)
		{
			this.mBlendPos = this.mBlendEnd;
			this._enterFrameRelay.removeListener(this);

			foreach( node in this.mPreviousSkies )
			{
				node.destroy();
			}

			this.mPreviousSkies = [];
		}

		this._blend();
	}

}

