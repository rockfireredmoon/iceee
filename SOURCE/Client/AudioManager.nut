this.Audio <- {
	AMBIENT_CHANNEL = "Ambient",
	AMBIENT_CHANNEL_2 = "Ambient2",
	DEFAULT_CHANNEL = "Default",
	COMBAT_CHANNEL = "Combat",
	NOISE_CHANNEL = "Noise"
};
class this.AudioTrack 
{
	constructor( archive, emitter, name )
	{
		this.mLoader = archive != "" ? this.Util.waitForAssets(archive, null) : null;
		this.mEmitter = emitter;
		this.mName = name;
	}

	function play()
	{
		if (this.mPlaying || ::LoadScreen.isVisible())
		{
			return;
		}

		if (this.mLoader && this.mLoader.isReady() == false)
		{
			return;
		}

		this.mEmitter.setSound(this.mName);
		this.mEmitter.play();
		this.mPlaying = true;
		this.mLoader = null;
	}

	function fade( delta )
	{
		this.play();

		if (this.mPlaying == false)
		{
			return true;
		}

		if (this.mGainAdjust > 0 && ::LoadScreen.isVisible() && ::_playTool != null)
		{
			this.mEmitter.setMuted(true);
			return true;
		}
		else
		{
			this.mEmitter.setMuted(false);
		}

		local gain = this.mEmitter.getGain();
		this.mEmitter.setGain(gain + this.mGainAdjust * delta);

		if (this.mGainAdjust < 0 && this.mEmitter.getGain() <= 0)
		{
			return false;
		}

		return this.mEmitter.isPlaying();
	}

	function setGainAdjust( amount )
	{
		this.mGainAdjust = amount;

		if (amount < 0.0)
		{
			this.mEmitter.lockChannelVolume();
		}
		else
		{
			this.mEmitter.unlockChannelVolume();
		}
	}

	function getFadingOut()
	{
		return this.mGainAdjust < 0.0;
	}

	function setMuted( which )
	{
		this.mEmitter.setMuted(which);
	}

	function destroy()
	{
		this.mEmitter.destroy();
		this.mEmitter = null;
	}

	function getName()
	{
		return this.mName;
	}

	mPlaying = false;
	mGainAdjust = 0.0;
	mEmitter = null;
	mLoader = null;
	mName = null;
}

class this.AudioManager 
{
	constructor()
	{
		this._enterFrameRelay.addListener(this);
		this.mTracks = {};
	}

	function playMusic( name, channel )
	{
		local archive;

		if (!this.Util.isDevMode() && name.find("Music-") == 0)
		{
			local extension = name.find(".ogg");

			if (extension == null)
			{
				return;
			}

			archive = name.slice(0, extension);
			local exists = ::_cache.exists(::GetFullPath(archive));

			if (!exists)
			{
				::_contentLoader.prefetch(archive);
				return;
			}
		}
		else
		{
			archive = "";
		}

		local channelName = channel != null ? channel : this.Audio.DEFAULT_CHANNEL;

		if ((channelName in this.mTracks) == false)
		{
			this.mTracks[channelName] <- [];
		}

		foreach( m in this.mTracks[channelName] )
		{
			if (m.getName() == name)
			{
				m.setGainAdjust(this.mFadeSpeed);
				return true;
			}
		}

		local crossFade = true;

		if (vargc > 0)
		{
			crossFade = vargv[0];
		}

		if (!crossFade)
		{
			foreach( m in this.mTracks[channelName] )
			{
				m.destroy();
			}

			delete this.mTracks[channelName];
		}
		else
		{
			foreach( m in this.mTracks[channelName] )
			{
				m.setGainAdjust(-this.mFadeSpeed);
			}
		}

		local id = "Music_" + this.mNextTrackID++.tostring();
		local emitter;

		try
		{
			emitter = this._scene.createSoundEmitter(id);
			emitter.setMuted(this.mMusicMuted);
			emitter.setGain(0.0);
		}
		catch( err )
		{
			this.log.error("Error playing music " + name + ": " + err);
			emitter.destroy();
			return false;
		}

		local track = this.AudioTrack(archive, emitter, name);
		track.setGainAdjust(this.mFadeSpeed);
		this.mTracks[channelName].append(track);
		return true;
	}

	function stopMusic( channel, ... )
	{
		local fadeTime = vargc > 0 ? vargv[0] : this.mFadeSpeed;

		if (fadeTime == null)
		{
			fadeTime = this.mFadeSpeed;
		}

		if (channel == null)
		{
			channel = this.Audio.DEFAULT_CHANNEL;
		}

		if (channel in this.mTracks)
		{
			foreach( m in this.mTracks[channel] )
			{
				m.setGainAdjust(-fadeTime);
			}
		}
	}

	function onEnterFrame()
	{
		local delta = this._deltat / 1000.0;
		local tracks = {};

		foreach( k, v in this.mTracks )
		{
			local l = [];

			foreach( m in v )
			{
				if (m.fade(delta))
				{
					l.append(m);
				}
				else
				{
					m.destroy();
				}
			}

			if (l.len() > 0)
			{
				tracks[k] <- l;
			}
		}

		this.mTracks = tracks;
		local finished = [];
		local x;

		for( x = 0; x < this.mSounds.len(); x++ )
		{
			if (this.mSounds[x].isPlaying() == false)
			{
				finished.append(this.mSounds[x]);
			}
		}

		foreach( f in finished )
		{
			this.mSounds.remove(this.Util.indexOf(this.mSounds, f));
		}
	}

	function setMusicMuted( which, ... )
	{
		local channel = vargc > 0 ? vargv[0] : null;

		if (channel != null)
		{
			if (channel in this.mTracks)
			{
				foreach( m in this.mTracks[channel] )
				{
					m.setMuted(which);
				}
			}
		}
		else
		{
			foreach( k, v in this.mTracks )
			{
				foreach( t in v )
				{
					t.setMuted(which);
				}
			}
		}

		this.mMusicMuted = which;
	}

	function isMusicPlaying( channel )
	{
		if ((channel in this.mTracks) == false)
		{
			return false;
		}

		foreach( m in this.mTracks[channel] )
		{
			if (m.getFadingOut() == false)
			{
				return true;
			}
		}

		return false;
	}

	function setForceAmbientMuted( which )
	{
		local mute = which ? true : this.mAmbientMuted;
		this._root.setAudioChannelMuted(::Audio.AMBIENT_CHANNEL, mute);
		this._root.setAudioChannelMuted(::Audio.AMBIENT_CHANNEL_2, mute);
		this.mForceAmbientMuted = which;
	}

	function setAmbientMuted( which, ... )
	{
		local mute = this.mForceAmbientMuted ? true : which;
		this._root.setAudioChannelMuted(::Audio.AMBIENT_CHANNEL, mute);
		this._root.setAudioChannelMuted(::Audio.AMBIENT_CHANNEL_2, mute);
		this.mAmbientMuted = which;
	}

	function setMuted( which )
	{
		this._root.setAudioMuted(which);
	}

	function setVolume( which )
	{
		this.mVolume = which;
	}

	function createSoundEmitter( sound )
	{
		local id = "Sound_" + this.mNextTrackID++.tostring();
		local emitter = this._scene.createSoundEmitter(id);
		emitter.setSound(sound);
		return emitter;
	}

	function playSound( sound, group, ... )
	{
		local emitter = this.createSoundEmitter(sound);
		emitter.setGain(1.0);
		emitter.play();
		this.mSounds.append(emitter);
	}

	mForceAmbientMuted = false;
	mAmbientMuted = false;
	mMusicMuted = false;
	mFadeSpeed = 0.44999999;
	mNextTrackID = 0;
	mSounds = [];
	mTracks = {};
}

this.Audio.playMusic <- function ( name, ... )
{
	this._audioManager.playMusic(name, vargc > 0 ? vargv[0] : null);
	return true;
};
this.Audio.playSound <- function ( name )
{
	this._audioManager.playSound(name, "");
};
this.Audio.isMusicPlaying <- function ( channel )
{
	return this._audioManager.isMusicPlaying(channel);
};
this.Audio.stopMusic <- function ( ... )
{
	this._audioManager.stopMusic(vargc > 0 ? vargv[0] : null, vargc > 1 ? vargv[1] : null);
};
this.Audio.setAmbientMuted <- function ( which )
{
	this._audioManager.setAmbientMuted(which);
};
this.Audio.setForceAmbientMuted <- function ( which )
{
	this._audioManager.setForceAmbientMuted(which);
};
this.Audio.setVolume <- function ( value )
{
	this._audioManager.setVolume(value);
};
this.Audio.setMusicMuted <- function ( which, ... )
{
	this._audioManager.setMusicMuted(which, vargc > 0 ? vargv[0] : null);
};
this.Audio.setMuted <- function ( which )
{
	this._audioManager.setMuted(which);
};
