this.require("MessageTimer");
this.require("ErrorChecking");
this.require("SceneObject");
this.require("Constants");
this.require("ServerConstants");
this.require("Items/ItemManager");
this.require("AbilityManager");
this.require("PartyManager");

if (false)
{
	this._print <- this.print;
	function print( str )
	{
		this._print(str);
		this._print(this.GetBacktraceString());
	}

}

function ResetNetworkCachedData()
{
	::_ItemDataManager.reset();
	::_ItemManager.reset();
	::Pref.resetRemote();
	::_creatureDefManager.reset();
	::Assembler.flush(true);
	::_AbilityManager.reset();
	::_questManager.reset();
	::_root.clearMinimapMarkers();
}

this.ProtocolDef <- {
	VERSION_ID = 33,
	[0] = {
		_name_ = "Lobby",
		[255] = "_handleProtocolChangedMsg",
		[0] = "_handleInfoMsg",
		[1] = "_handleQueryResultMsg",
		[40] = "_handleAuthenticationRequestMsg",
		[50] = "_handleLoginQueueMessage",
		[60] = "_handleEncryptMessage",
		[4] = "_handleItemDefUpdateMsg",
		inspectItemDef = 4,
		authenticate = 1,
		selectPersona = 2,
		query = 3
	},
	[1] = {
		_name_ = "Play",
		[255] = "_handleProtocolChangedMsg",
		[0] = "_handleInfoMsg",
		[1] = "_handleQueryResultMsg",
		[4] = "_handleCreatureEventMsg",
		[5] = "_handleCreatureUpdateMsg",
		[6] = "_handlePartyUpdateMsg",
		[7] = "_handleQuestEventMsg",
		[8] = "_handleLoginQueueMessage",
		[41] = "_handleSceneryUpdateMsg",
		[42] = "_handleEnvironmentUpdateMsg",
		[43] = "_handleFriendNotificationMsg",
		[44] = "_handleSpecialOfferMsg",
		[50] = "_handleCommunicationMsg",
		[51] = "_handleTradeMsg",
		[52] = "_handleForm",
		[53] = "_handleRefashion",
		[60] = "_handleAbilityActivationMsg",
		[70] = "_handleItemUpdateMsg",
		[71] = "_handleItemDefUpdateMsg",
		[80] = "_handlePVPStatUpdateMessage",
		[90] = "_handleHeartbeatMessage",
		[91] = "_handleShake",
		[96] = "_handleBooks",
		[98] = "_handleSceneryEffectMsg",
		[99] = "_handleGuildUpdateMsg",
		inspectCreatureDef = 0,
		updateVelocity = 1,
		query = 2,
		selectTarget = 3,
		communicate = 4,
		inspectCreature = 5,
		abilityActivate = 6,
		abilityCancel = 7,
		inspectItem = 8,
		inspectItemDef = 9,
		inWater = 10,
		mouseClick = 21
	}
};
this.PingDelay <- 300.0;
this.CreatureUpdateMsgCombat <- false;
this.ItemUpdateDefMsgCraft <- false;
class this.DefaultQueryHandler 
{
	function resultAsString( qa, rows )
	{
		local i;
		local j;
		local row;
		local value;
		local str = qa.query;

		if (qa.args.len() > 0)
		{
			str += "(" + this.Util.join(qa.args, ",") + ")";
		}

		str += " result:\n";
		str += "----------------------------------------------\n";

		foreach( i, row in rows )
		{
			foreach( j, value in row )
			{
				if (j > 0)
				{
					str += " | ";
				}

				str += value;
			}

			str += "\n";
		}

		str += "----------------------------------------------\n";
		str += "Total: " + rows.len();
		return str;
	}

	function onQueryComplete( qa, results )
	{
		if (results.len() == 0)
		{
			return;
		}

		local str = this.resultAsString(qa, results);
		this.log.info(str);
		this.IGIS.info(str);
	}

	function onQueryError( qa, error )
	{
		this.log.debug("" + qa.query + " [" + qa.correlationId + "] failed: " + error);
	}

	function onQueryTimeout( qa )
	{
		this.log.warn("Query " + qa.query + " [" + qa.correlationId + "] timed out");
		this.onQueryError(qa, "timed out");
	}

}

class this.ClanInviteCallback 
{
	mName = null;
	constructor( name )
	{
		this.mName = name;
	}

	function onActionSelected( mb, alt )
	{
		if (alt == "Yes")
		{
			::_Connection.sendQuery("clan.invite.accept", this, [
				this.mName
			]);
		}
	}

	function onQueryError( qa, reason )
	{
		this.IGIS.error(reason);
	}

}

class this.NullQueryHandler 
{
	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error(error);
	}

	function onQueryComplete( qa, results )
	{
	}

}

class this.DefaultActionHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		if (results.len() == 0)
		{
			return;
		}

		this.log.debug(this.resultAsString(qa, results));
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error("" + qa.query + " failed: " + error);
	}

}

class this.Connection extends this.MessageBroadcaster
{
	mProtocolVersionId = 0;
	mChatMessages = null;
	mIgnorePacket = false;
	mRouterSelectSocket = null;
	mRouterSelect = false;
	mRouterAddress = "";
	mUseEncryption = false;
	mEncryptionKey = "";
	mEncryptor = null;
	mDecryptor = null;
	mAuthToken = null;
	mAuthData = null;
	mXCSRFToken = null;
	mCurrentRegionChannel = "";
	mForms = {};
	
	constructor()
	{
		this.mSceneObjectManager = ::_sceneObjectManager;
		::MessageBroadcaster.constructor();
		this.mPendingQueries = {};
		this.mQueryQueue = [];
		this.mChatMessages = [];
		this.socket = ::Socket();
		this.socket.addListener(this);
		this.mServerIndex = 0;
		this.connectionState = 0;
		this.mCurrentHost = null;
		this.messageBuf = this.ByteBuffer(65536);
	}

	function resetQueries()
	{
		this.mPendingQueries = {};
		this.mQueryQueue = [];
	}

	function getProtocolVersionId()
	{
		return this.mProtocolVersionId;
	}

	function ignorePacket()
	{
		this.mIgnorePacket = true;
	}

	function setAuthToken( token )
	{
		this.mAuthToken = token;
	}

	function connect()
	{
		this.mRouterSelect = true;
		this.mRouterAddress = "";
		this.attemptToConnect();
	}
	
	
	// Hack to set the proper domain domain name when connecting to an arbitrary
	// port (other than HTTP port 80).
	// Ex: http://example.com:81/Release/Current/EarthEternal.car
	// Originally it doesn't support arbitrary port numbers.
	// Derived from code used in the original attemptToConnect() function.
	// Also allows a router port override hack.
	function ModGetCustomRouterPort()
	{
		if("router" in ::_args)
		{
			return ::_args["router"].tointeger();
		}
	
		try
		{
			local t = unserialize( _cache.getCookie("Router") );
			if(t)
			{
				if("mod.router" in t)
					return t["mod.router"].tointeger();
			}
		}
		catch(e)
		{
		}
		
		return 4242;   //Corresponds to hardcoded default.
	}
	
	function attemptToConnect()
	{
		local domain = _cache.getBaseURL();
		local custom = false;
	
		//Base URL will look something like this
		//http://localhost/Release/Current
	
		if( domain.find("://") != null )
		{
			domain = Util.split(domain, "://")[1];
			
			if( domain.find("@") != null )
			{
				domain = Util.split(domain, "@")[1];
			}
			
			if( domain.find("/") != null )
			{
				domain = Util.split(domain, "/")[0];
			}
			
			if(domain.find(":") != null)
			{
				custom = true;
				domain = Util.split(domain, ":")[0];
			}
	
			local routerPort = ModGetCustomRouterPort();
	
			if(custom == true)
			{
				mCurrentHost = domain + ":" + routerPort;
				
				log.info( "Connecting to " + mCurrentHost );
				log.info( "[MOD] Custom domain port detected.");
				if(routerPort != 4242)
					log.info( "[MOD] Custom router port detected.");
					
				if( Util.isDevMode() )
					Screen.setTitle("Earth Eternal (" + mCurrentHost + ")");
					
				socket.connect(domain, routerPort, 0);
				return;
			}
		}
		
		//Hack didn't work, call original
		attemptToConnectOriginal();
	}


	function attemptToConnectOriginal()
	{
		this.log.info("Cache URL: " + this._cache.getBaseURL());

		if (this.Util.startsWith(this._cache.getBaseURL(), "file:///"))
		{
			if (this.mServerIndex >= this.gServer.address.len())
			{
				this.mServerIndex = 0;
			}

			this.mCurrentHost = this.gServer.address[this.mServerIndex] + ":" + this.gServer.port[this.mServerIndex];
			this.log.info("Connecting to " + this.mCurrentHost);

			if (this.Util.isDevMode())
			{
				this.Screen.setTitle("Earth Eternal (" + this.mCurrentHost + ")");
			}

			this.socket.connect(this.gServer.address[this.mServerIndex], this.gServer.port[this.mServerIndex], 0);
		}
		else
		{
			local domain = this._cache.getBaseURL();

			if (domain.find("://") != null)
			{
				domain = this.Util.split(domain, "://")[1];

				if (domain.find("@") != null)
				{
					domain = this.Util.split(domain, "@")[1];
				}

				if (domain.find("/") != null)
				{
					domain = this.Util.split(domain, "/")[0];
				}
			}

			if (this.Util.startsWith(domain, "static."))
			{
				domain = "router." + this.Util.split(domain, "static.")[1];
			}

			this.mCurrentHost = domain + ":4242";
			this.log.info("Connecting to " + this.mCurrentHost);

			if (this.Util.isDevMode())
			{
				this.Screen.setTitle("Earth Eternal (" + this.mCurrentHost + ")");
			}

			this.socket.connect(domain, 4242, 0);
		}
	}

	function connectToRouter()
	{
		if (this.mRouterAddress == "")
		{
			return;
		}

		try
		{
			local brk = this.mRouterAddress.find(":");
			local host = this.mRouterAddress.slice(0, brk);
			local port = this.mRouterAddress.slice(brk + 1).tointeger();
			this.socket.connect(host, port, 0);
		}
		catch( err )
		{
			this.log.debug("Connection error: " + err);
			this.connect();
		}
	}

	function close( ... )
	{
		local event = vargc > 0 ? vargv[0] : true;
		this.cancelPersonaList();
		this.connectionState = -1;
		this.mRouterSelect = false;
		this.mRouterAddress = "";
		this.socket.close();

		if (event)
		{
			::States.event("Disconnect");
		}

		this.ResetNetworkCachedData();
		this.resetQueries();
	}

	function isConnected()
	{
		return this.connectionState > 0;
	}

	function rot2rad( rot )
	{
		return rot.tofloat() * ::Math.TWO_PI / 256.0;
	}

	function rad2rot( rads )
	{
		return rads.tofloat() * 256.0 / ::Math.TWO_PI;
	}

	function onConnectStart( newSocket )
	{
		this.connectionState = 1;
	}

	function onConnect( newSocket )
	{
		this.connectionState = 2;

		if (this.mRouterSelect)
		{
			return;
		}

		if (this.mPingTask)
		{
			::_eventScheduler.cancel(this.mPingTask);
		}

		this.mPingTask = ::_eventScheduler.fireIn(::PingDelay, this, "ping");
		this.mCurrentProtocol = this.ProtocolDef[0];
		this.mProtocolChangePending = false;
		this.mServerIndex = 0;
		this.broadcastMessage("onConnect");
	}

	function onError( newSocket )
	{
		this.mRouterSelect = false;
		this.mRouterAddress = "";

		if (this.connectionState == 2)
		{
			this._clear();
			this.broadcastMessage("onDisconnect");
			::_stateManager.onEvent("Disconnect");
			this.ResetNetworkCachedData();
		}
		else
		{
			this._tryNextServer();
		}
	}

	function onClose( socket )
	{
		if (this.mRouterSelect)
		{
			this._clear();
			this.connectToRouter();
			return;
		}

		if (this.connectionState > 0)
		{
			::_stateManager.onEvent("Disconnect");
			this.ResetNetworkCachedData();
		}

		this._clear();
	}

	function onTimeOut( newSocket )
	{
		this._clear();
		this.log.info("Connection timed out, trying next alternative...");
		this._tryNextServer();
	}

	function _tryNextServer()
	{
		this.mServerIndex++;
		this.connect();
	}

	function _clear()
	{
		this.connectionState = 0;
		this.mRouterSelect = false;
		this.mUseEncryption = false;

		if (this.mPingTask)
		{
			this._eventScheduler.cancel(this.mPingTask);
			this.mPingTask = null;
		}
	}

	function enableEncryption( iv )
	{
		this.mEncryptor = this.Encryptor();
		this.mEncryptor.setIV(iv);
		this.mDecryptor = this.Encryptor();
		this.mDecryptor.setIV(iv);
		this.mUseEncryption = true;
	}

	function isPlaying()
	{
		if (this.mCurrentProtocol == null)
		{
			return false;
		}

		return this.mCurrentProtocol._name_ == "Play";
	}

	function isLobby()
	{
		return this.mCurrentProtocol._name_ == "Lobby";
	}

	function sendComm( newMessage, ... )
	{
		local channel = "";

		if (vargc > 0)
		{
			channel = "" + vargv[0];
		}

		this._beginSend("communicate");
		this.mOutBuf.putStringUTF(channel);
		this.mOutBuf.putStringUTF(newMessage);
		this._send();
	}

	function sendAllEval( cmd )
	{
		this.sendComm(cmd, "_eval_");
	}

	function sendVelocityUpdate( pos, pHeading, pRotation, pSpeed )
	{
		this._beginSend("updateVelocity");
		local pX = pos.x.tointeger();
		local pY = pos.y.tointeger();
		local pZ = pos.z.tointeger();
		this.mOutBuf.putShort(pX);
		this.mOutBuf.putShort(pZ);
		this.mOutBuf.putShort(pY);
		this.mOutBuf.putByte(this.rad2rot(pHeading));
		this.mOutBuf.putByte(this.rad2rot(pRotation));
		this.mOutBuf.putByte(pSpeed);
		this._send();
	}

	function _beginSend( msgType )
	{
		if (this.mProtocolChangePending)
		{
			throw this.Exception("Cannot send message while changing protocols");
		}

		if (this.mOutBuf == null)
		{
			this.mOutBuf = this.ByteBuffer(4096);
		}

		this.mOutBuf.clear();
		this.mOutBuf.putByte(this.mCurrentProtocol[msgType]);
		this.mOutBuf.putShort(0);
	}

	function _send()
	{
		local size = this.mOutBuf.position();
		this.mOutBuf.position(1);
		this.mOutBuf.putShort(size - 3);

		if (this.mUseEncryption)
		{
			this.mOutBuf.position(0);
			this.mEncryptor.process(this.mOutBuf, 1);
			this.mOutBuf.position(1);
			this.mEncryptor.process(this.mOutBuf, 2);
			this.mOutBuf.position(3);
			this.mEncryptor.process(this.mOutBuf, size - 3);
		}

		this.mOutBuf.position(size);
		this.mOutBuf.flip();
		this.socket.write(this.mOutBuf);
	}

	function onRecv( newSocket, buf )
	{
		local notComplete = false;
		local limit = buf.limit();

		if (this.mRouterSelect)
		{
			local x;

			for( x = 0; x < limit; x++ )
			{
				this.mRouterAddress += buf.getByte().tochar();
			}

			return;
		}

		while (true)
		{
			if (this.messageType == null)
			{
				if (buf.remaining() < 1)
				{
					break;
				}

				if (this.mUseEncryption)
				{
					this.mDecryptor.process(buf, 1);
				}

				this.messageType = buf.getByte() & 255;
			}

			if (this.messageSize == null)
			{
				if (buf.remaining() < 2)
				{
					break;
				}

				if (this.mUseEncryption)
				{
					this.mDecryptor.process(buf, 2);
				}

				this.messageSize = buf.getShort();
				this.messageBuf.clear();
				this.messageBuf.limit(this.messageSize);
			}

			this.messageBuf.putBuffer(buf);

			if (this.messageBuf.remaining() > 0)
			{
				break;
			}

			this.messageBuf.flip();

			if (this.mUseEncryption)
			{
				this.mDecryptor.process(this.messageBuf, this.messageBuf.remaining());
			}

			local type = this.messageType;
			local size = this.messageSize;
			this.messageType = null;
			this.messageSize = null;

			if (!(type in this.mCurrentProtocol) || this.mCurrentProtocol[type] == null)
			{
				this.log.warn("Current protocol doesn\'t handle message type: " + type);
			}
			else
			{
				local handler = this.mCurrentProtocol[type];
				this.mIgnorePacket = false;
				this[handler](this.messageBuf);

				if (!this.mIgnorePacket && this.messageBuf.remaining() != 0)
				{
					this.log.warn("Message handler (" + handler + ") did not fully parse message (unread bytes = " + this.messageBuf.remaining() + ").");
				}
			}
		}
	}

	function _decodeTruncatedCoord( viewer_float, entity_short )
	{
		local viewer_full = viewer_float.tointeger();
		local delta = entity_short - (viewer_full & 65535);

		if (delta & 32768)
		{
			delta = delta | 4294901760;
		}

		if (delta > 61439)
		{
			delta -= 65535;
		}
		else if (delta < -61439)
		{
			delta += 65535;
		}

		return viewer_float + delta;
	}

	function _handleProtocolChangedMsg( data )
	{
		local newProto = data.getByte() & 255;

		if (!(newProto in this.ProtocolDef))
		{
			this.log.error("Received invalid message protocol switch id: " + newProto);
			this.close();
			return;
		}

		if (this.ProtocolDef[newProto] == this.mCurrentProtocol)
		{
			this.log.warn("Received message protocol change request, but already in that protocol: " + newProto);
			return;
		}

		this.mCurrentProtocol = this.ProtocolDef[newProto];
		this.mProtocolChangePending = false;
		this.log.debug("Switching to message protocol: " + this.mCurrentProtocol._name_);
		this.broadcastMessage("onProtocolChanged", this.mCurrentProtocol._name_);
	}

	function _handleAuthenticationRequestMsg( data )
	{
		this.mProtocolVersionId = data.getInteger();

		if (this.mProtocolVersionId >= 2)
		{
			this.CreatureUpdateMsgCombat = true;
		}

		if (this.mProtocolVersionId >= 7)
		{
			this.ItemUpdateDefMsgCraft = true;
		}

		if (this.mProtocolVersionId == 1)
		{
		}
		else if (this.mProtocolVersionId == 2)
		{
		}
		else if (this.mProtocolVersionId == 3)
		{
		}
		else if (this.mProtocolVersionId == 4)
		{
		}
		else if (this.mProtocolVersionId == 5)
		{
		}
		else if (this.mProtocolVersionId == 6)
		{
		}
		else if (this.mProtocolVersionId == 7)
		{
		}
		else if (this.mProtocolVersionId == 8)
		{
		}
		else if (this.mProtocolVersionId == 9)
		{
		}
		else if (this.mProtocolVersionId == 10)
		{
		}
		else if (this.mProtocolVersionId == 11)
		{
		}
		else if (this.mProtocolVersionId == 12)
		{
		}
		else if (this.mProtocolVersionId == 13)
		{
		}
		else if (this.mProtocolVersionId == 14)
		{
		}
		else if (this.mProtocolVersionId == 15)
		{
		}
		else if (this.mProtocolVersionId == 16)
		{
		}
		else if (this.mProtocolVersionId == 17)
		{
		}
		else if (this.mProtocolVersionId == 18)
		{
		}
		else if (this.mProtocolVersionId == 19)
		{
		}
		else if (this.mProtocolVersionId == 20)
		{
		}
		else if (this.mProtocolVersionId == 21)
		{
		}
		else if (this.mProtocolVersionId == 22)
		{
		}
		else if (this.mProtocolVersionId == 23)
		{
		}
		else if (this.mProtocolVersionId == 24)
		{
		}
		else if (this.mProtocolVersionId == 25)
		{
		}
		else if (this.mProtocolVersionId == 26)
		{
		}
		else if (this.mProtocolVersionId == 27)
		{
		}
		else if (this.mProtocolVersionId == 28)
		{
		}
		else if (this.mProtocolVersionId == 29)
		{
		}
		else if (this.mProtocolVersionId == 30)
		{
		}
		else if (this.mProtocolVersionId == 31)
		{
		}
		else if (this.mProtocolVersionId == 32)
		{
		}
		else if (this.mProtocolVersionId != this.ProtocolDef.VERSION_ID)
		{
			this.log.error("Protocol version mismatch! (expecting " + this.ProtocolDef.VERSION_ID + ", got: " + this.mProtocolVersionId + ")");
			this.close();
			return;
		}

		local nonce;
		local auth_method = this.AuthMethod.DEV;

		if (this.mProtocolVersionId >= 11)
		{
			auth_method = data.getInteger();
		}

		local auth_data = data.getStringUTF();
		mAuthData = auth_data;

		if (auth_method == this.AuthMethod.SERVICE)
		{
			local self = this;
			local req = this.XMLHttpRequest();
			
			this.log.debug("AUTH URL: " + auth_data);

			if (this.mAuthToken)
			{
				/*req.open("POST", auth_data + "/user/token.json", false);
				req.send(this.System.encodeVars({
					web_auth_token = this.mAuthToken,
					browser = isBrowser
				}));
				*/
			}
			else
			{
				mXCSRFToken = null;
				
				req.onreadystatechange = function () {
					if (this.readyState == 4) {
						 ::_Connection._handleServiceToken(this);
					}
				};
				
				// Get the X-CSRF-Token (sent as header on next request)
				req.setRequestHeader("Content-Type", "application/json");
				req.setRequestHeader("User-Agent", "EETAW");
				req.setRequestHeader("Host", Util.extractHostnameAndPortFromUrl(mAuthData));
				req.open("POST", mAuthData + "/user/token.json", false);
				req.send();
			}
		
		}
		else if (auth_method == this.AuthMethod.DEV)
		{
			if (::_username && ::_password)
			{
				this._beginSend("authenticate");

				if (this.mProtocolVersionId >= 11)
				{
					this.mOutBuf.putByte(this.AuthMethod.DEV);
				}

				this.mOutBuf.putStringUTF(::_username);
				this.mOutBuf.putStringUTF(this.md5(::_username + ":" + this.md5(::_password) + ":" + auth_data));
				this._send();
				this.mPersonaSchedule = ::_eventScheduler.fireIn(5.0, this, "requestPersonaList");
			}
		}
		else
		{
			local self = this;
			local req = this.XMLHttpRequest();
			req.onreadystatechange = function () : ( self )
			{
				if (this.readyState == 4)
				{
					local errorMsg;
					local token;
					local accountId;
					local encryptIV;
					local account;
					this.log.debug("Auth Result: " + this.status + " " + this.statusText);

					if (this.status == 200)
					{
						try
						{
							local results = ::json(this.responseText);
							token = results.authentication_token;
							accountId = results.account.id;

							if ("email" in results)
							{
								::_username = results.email;
							}

							if ("encryption_key" in results)
							{
								this.log.debug("ENCRYPT KEY: " + results.encryption_key);
								encryptIV = results.encryption_key;
							}
							else
							{
								encryptIV = "0123456789abcdef0123456789abcdef";
							}
						}
						catch( err )
						{
							token = null;
							this.log.error("Error with auth response: " + err);
							errorMsg = "Authentication failed (Internal error)";
						}
					}
					else if (this.status == 400)
					{
						errorMsg = "Login Failed.";
					}
					else if (this.status == 404)
					{
						errorMsg = "Account does not exist.";
					}
					else if (this.status == 403)
					{
						errorMsg = "Your username or password is incorrect.";
					}
					else
					{
						errorMsg = "Authentication failed (Error Code: " + this.status + ")";
					}

					self.mAuthToken = null;

					if (token != null)
					{
						::_Connection._beginSend("authenticate");
						::_Connection.mOutBuf.putByte(this.AuthMethod.EXTERNAL);
						::_Connection.mOutBuf.putStringUTF("" + accountId);
						::_Connection.mOutBuf.putStringUTF(token);
						::_Connection._send();
						::_Connection.mEncryptionKey = encryptIV;
					}
					else
					{
						this.States.event("AuthFailure", errorMsg);
						::_Connection.close(false);
					}
				}
			};
			local isBrowser = this.System.isBrowserEmbedded() ? "1" : "0";
			this.log.debug("AUTH URL: " + auth_data);

			if (this.mAuthToken)
			{
				req.open("POST", auth_data + "/login_with_token.json", false);
				req.send(this.System.encodeVars({
					web_auth_token = this.mAuthToken,
					browser = isBrowser
				}));
			}
			else
			{
				req.open("POST", auth_data + "/login.json", false);
				req.send(this.System.encodeVars({
					email = ::_username,
					password = ::_password,
					browser = isBrowser
				}));
			}
		}
	}
	
	function _handleServiceLogin(req) {
		local errorMsg = "No user details returned.";
		this.log.debug("Auth 2nd Result: " + req.status + " " + req.statusText);
		if (req.status == 200) {
			
			local results = ::json(req.responseText);
			local tkn = mXCSRFToken + ":" + results.sessid + ":" + results.session_name + ":" + results.user.uid;
			
			mOutBuf._beginSend("authenticate");

			if (this.mProtocolVersionId >= 11)
			{
				mOutBuf.putByte(this.AuthMethod.SERVICE);
			}

			mOutBuf.putStringUTF(::_username);
			mOutBuf.putStringUTF(tkn);
			mOutBuf._send();
			mPersonaSchedule = ::_eventScheduler.fireIn(5.0, this, "requestPersonaList");
		}
		else {
			errorMsg = "Authentication failed (Error Code: " + req.status + ")";
			this.States.event("AuthFailure", errorMsg);
			close(false);
		}
	}
	
	function _handleServiceToken(req) {
	
		local errorMsg = "No token returned";
		this.log.debug("Auth Result: " + req.status + " " + req.statusText);
		if (req.status == 200) {
			local results = ::json(req.responseText);
			mXCSRFToken = results.token;
		}
		else {
			errorMsg = "Failed to get access token from (Error Code: " + req.status + "/" + mAuthData + ")";
		}
		
		if(mXCSRFToken == null) {
			this.States.event("AuthFailure", errorMsg);
			close(false);
		}
		else {
			// Now we can authenticate the username and password							
			local innerReq = this.XMLHttpRequest();
			innerReq.onreadystatechange = function () : ( self )	{
				if (this.readyState == 4) {
					::_Connection._handleServiceLogin(this);
				}
			};
			innerReq.open("POST", mAuthData + "/user/login.json", false);
			innerReq.send(this.System.encodeVars({
				username = ::_username,
				password = ::_password,
			}));
			
		}
	}

	mPersonaSchedule = null;
	function requestPersonaList()
	{
		::_Connection.sendQuery("persona.list", this.PersonaListHandler());
	}

	function cancelPersonaList()
	{
		if (this.mPersonaSchedule)
		{
			::_eventScheduler.cancel(this.mPersonaSchedule);
			this.mPersonaSchedule = null;
		}
	}

	function _handleInfoMsg( data )
	{
		local text = data.getStringUTF();
		local type = data.getByte();

		switch(type)
		{
		case 0:
			this.IGIS.info(text);
			break;

		case 1:
			this.IGIS.error(text);
			break;

		case 2:
			this.IGIS.error(text);
			break;

		case 3:
			this.IGIS.info(text);
			break;

		case 4:
			this.IGIS.info(text);
			break;

		case 5:
			if (text.find("DELTA_TOO_LARGE") == null)
			{
				this.IGIS.error(text);
			}

			break;

		case 6:
			this.log.info("[SYSTEM] " + text);

			if (text == "ExternalAccountEvent")
			{
				this.sendQuery("account.fulfill", this.AccountFulfillmentHandler());
				this.sendQuery("persona.currency", this.PersonaCurrencyHandler());
			}

			break;

		case 7:
			local oldLocation = ::_sceneObjectManager.getLocationName();
			::_sceneObjectManager.setLocationName(text);

			if (text != oldLocation)
			{
				::Audio.playSound("Sound-Warning.ogg");
				this.IGIS.info("Entering " + text);
			}

			local miniMapLocation = this.Screens.get("MiniMapLocation", false);

			if (miniMapLocation)
			{
				miniMapLocation.setLocationText(text);
			}

			this.LoadGate.AutoFetchLoadGate(this.GateTrigger.TriggerTypes.LOCATION, text);
			break;

		case 8:
			local miniMapLocation = this.Screens.get("MiniMapLocation", false);

			if (miniMapLocation)
			{
				miniMapLocation.setLocationShard(text);
			}

			break;

		case 9:
			local mapScreen = this.Screens.get("MapWindow", true);

			if (mapScreen)
			{
				mapScreen.updateMapBaseImage(text);
			}

			break;

		case 10:
			this.IGIS.nonLoggingError(text);
			break;

		case 11:
			local damage = data.getInteger().tostring();
			::_ChatManager.addMessage("mci", "You have fallen for " + damage + " damage.");

			if (::_avatar != null)
			{
				::Audio.playSound("Sound-Falldamage.ogg");
				::_avatar.addFloatie("-" + damage, this.IGIS.FLOATIE_STATUS_RED);
			}

			break;

		case 12:
			this.mCurrentRegionChannel = text;
			break;

		case 13:
			local id = data.getInteger();
			local scenery = ::_sceneObjectManager.peekSceneryByID(id);

			if (scenery)
			{
				scenery.destroy();
			}

			break;
		case 14:
			this.IGIS.broadcast(text);
			break;
		default:
			this.IGIS.info(text);
		}
	}

	function _handleSpecialOfferMsg( data )
	{
		local type = data.getByte();

		if (type == 1)
		{
			local id = data.getInteger();
			local percentDiscount = data.getInteger();
			local offerItemCost = data.getInteger();
			local creditOfferId = data.getInteger();
			local title = data.getStringUTF();
			local desc = data.getStringUTF();
			local offerItemTitle = data.getStringUTF();
			local offerItemDesc = data.getStringUTF();
			local offerItemProto = data.getStringUTF();
			local hashCode = data.getStringUTF();
			local offer = this.SpecialOfferItem(id, percentDiscount, offerItemCost, creditOfferId, title, desc, offerItemTitle, offerItemDesc, offerItemProto, hashCode);
			::_specialOfferManager.addSpecialOffer(offer);
		}
		else
		{
			  // [038]  OP_JMP            0      0    0    0
		}
	}

	function getCurrentRegionChannel()
	{
		return this.mCurrentRegionChannel;
	}

	function _handleQuestEventMsg( data )
	{
		local questId = data.getInteger();
		local event = data.getByte();

		switch(event)
		{
		case 0:
			local objective = data.getByte();
			local complete = data.getByte() != 0;
			local text = data.getStringUTF();
			this.broadcastMessage("onQuestObjectiveUpdate", questId, objective, complete, text);
			break;

		case 1:
			local act = data.getInteger();
			this.broadcastMessage("onQuestActCompleted", questId, act);
			break;

		case 2:
			this.broadcastMessage("onQuestCompleted", questId);
			break;

		case 3:
			local creatureId = data.getInteger();
			this.broadcastMessage("onQuestJournal", questId, creatureId);
			break;

		case 4:
			this.broadcastMessage("onQuestRemoteAbandoned", questId);
			this.broadcastMessage("onQuestAbandoned", questId);
			break;

		case 5:
			this.broadcastMessage("onQuestJoined", questId);
			::_questManager.questJoined(questId);
			::_tutorialManager.questAccepted(questId);
			break;

		case 6:
			local creatureId = data.getInteger();
			this.broadcastMessage("onQuestTurnedIn", questId, creatureId);
			break;
		}
	}

	function _handleCreatureEventMsg( data )
	{
		local id = data.getInteger();

		switch(data.getByte())
		{
		case 0:
			local entity = this.mSceneObjectManager.peekCreatureByID(id);

			if (entity)
			{
				if (entity != ::_avatar)
				{
					if (entity.isInvisible())
					{
						entity.updateFade();
					}
					else
					{
						entity.gone();
					}
				}
			}

			break;

		case 1:
			local oldAvatar = ::_avatar;
			this.mSceneObjectManager.setAvatar(id);
			this.broadcastMessage("onAvatarChanged", oldAvatar, ::_avatar);
			this.sendQuery("account.fulfill", this.AccountFulfillmentHandler());
			break;

		case 2:
			break;

		case 3:
			local c = this.mSceneObjectManager.getCreatureByID(id);

			if (c != ::_avatar)
			{
				c.onJump();
			}

			break;

		case 4:
			local effect = data.getStringUTF();
			local c = this.mSceneObjectManager.getCreatureByID(id);
			c.cue(effect);
			break;

		case 5:
			local targetId = data.getInteger();

			if (this._avatar && id == this._avatar.getID())
			{
				local target = targetId ? this.mSceneObjectManager.getCreatureByID(targetId) : null;
				this._avatar.setTargetObject(target);

				if (target)
				{
					::Audio.playSound("Sound-Tabtarget.ogg");
				}
			}

			break;

		case 7:
			local target = this.mSceneObjectManager.getCreatureByID(id);
			local source = this.mSceneObjectManager.getCreatureByID(data.getInteger());
			local damageString = data.getStringUTF();
			local abilityName = data.getStringUTF();
			local criticalHit = data.getByte();
			local absorbedDamage = 0;

			if (this.mProtocolVersionId >= 21)
			{
				absorbedDamage = data.getInteger();
			}

			local damage = this.array(this.DamageType.len(), 0);
			local immunity = this.array(this.DamageType.len(), false);
			local splitDamageString = this.Util.split(damageString, "|");
			local damageOutputString = "";
			local targetHasSomeImmunity = false;

			foreach( damgeType in splitDamageString )
			{
				if (damgeType == "")
				{
					continue;
				}

				if (damageOutputString != "")
				{
					damageOutputString += ", ";
				}

				local splitString = this.Util.split(damgeType, ":");
				local damageType = splitString[0].tointeger();

				if (splitString[1] == "IMMUNE")
				{
					immunity[damageType] = true;
					damage[damageType] = 0;
					targetHasSomeImmunity = true;
				}
				else
				{
					damage[damageType] = splitString[1].tointeger();
				}

				damageOutputString += damage[damageType] + " points of " + this.DamageTypeNameMappingLower[damageType];
			}

			local totalDamage = 0;

			for( local i = 0; i < damage.len(); i++ )
			{
				totalDamage += damage[i];
			}

			local criticalHitModifier = " ";
			local hitPunctuation = ".";

			if (criticalHit == 1)
			{
				criticalHitModifier = " critically ";
				hitPunctuation = "!";

				if (target)
				{
					if (target == ::_avatar)
					{
						this.IGIS.floatie("Critical", this.IGIS.FLOATIE_STATUS_ORANGE_BIG, target);
					}
					else
					{
						this.IGIS.floatie("Critical", this.IGIS.FLOATIE_STATUS_YELLOW_BIG, target);
					}
				}
			}

			local abilityDamage = true;

			if (abilityName == "melee" || abilityName == "ranged_melee")
			{
				abilityDamage = false;
			}

			local recipientIsNPC = false;

			if (target.mCreatureDef != null)
			{
				recipientIsNPC = !target.mCreatureDef.getMeta("persona");
			}

			if (targetHasSomeImmunity)
			{
				this.IGIS.floatie("Immune", this.IGIS.FLOATIE_STATUS_RED, target);
			}

			if (target && totalDamage > 0)
			{
				target.addFloatie("-" + totalDamage.tostring(), target == ::_avatar ? this.IGIS.FLOATIE_STATUS_RED : this.IGIS.FLOATIE_STATUS_WHITE);
			}

			local playerId = ::_avatar.getID();
			local channelName = "";
			local combatMessage = "";
			local immunityMessage = "";

			if (source == ::_avatar)
			{
				for( local i = 0; i < immunity.len(); i++ )
				{
					if (immunity[i] == true)
					{
						immunityMessage += " " + target.getName() + " is immune to " + this.DamageTypeNameMappingLower[i] + "!";
					}
				}

				if (!abilityDamage)
				{
					combatMessage = "You" + criticalHitModifier + "hit " + target.getName() + " for " + damageOutputString + " damage";
				}
				else
				{
					combatMessage = "You perform " + abilityName + criticalHitModifier + "hitting " + target.getName() + " for " + damageOutputString + " damage";
				}

				channelName = "mco";
				local dpsScreen = this.Screens.get("DPSMeter", false);

				if (dpsScreen)
				{
					dpsScreen.addDamageOut(this.meleeDamage, damage[this.DamageType.MELEE]);
					dpsScreen.addDamageOut(this.fireDamage, damage[this.DamageType.FIRE]);
					dpsScreen.addDamageOut(this.frostDamage, damage[this.DamageType.FROST]);
					dpsScreen.addDamageOut(this.mysticDamage, damage[this.DamageType.MYSTIC]);
					dpsScreen.addDamageOut(this.deathDamage, damage[this.DamageType.DEATH]);
					dpsScreen.addDamageOut(this.unblockableDamage, damage[this.DamageType.UNBLOCKABLE]);
					dpsScreen.addSwingOut(true);
				}
			}
			else if (target == ::_avatar)
			{
				for( local i = 0; i < immunity.len(); i++ )
				{
					if (immunity[i] == true)
					{
						immunityMessage += " You are immune to " + this.DamageTypeNameMappingLower[i] + "!";
					}
				}

				if (!abilityDamage)
				{
					combatMessage = source.getName() + criticalHitModifier + "hits you for " + damageOutputString + " damage";
				}
				else
				{
					combatMessage = source.getName() + " performs " + abilityName + criticalHitModifier + "hitting you for " + damageOutputString + " damage";
				}

				local currentTarget = ::_avatar.getTargetObject();

				if (!currentTarget)
				{
					::_avatar.setResetTabTarget(true);
					::_avatar.setTargetObject(source);
					::_Connection.sendSelectTarget(source.getID());
				}

				local dpsScreen = this.Screens.get("DPSMeter", false);

				if (dpsScreen)
				{
					dpsScreen.addDamageIn(this.meleeDamage, damage[this.DamageType.MELEE]);
					dpsScreen.addDamageIn(this.fireDamage, damage[this.DamageType.FIRE]);
					dpsScreen.addDamageIn(this.frostDamage, damage[this.DamageType.FROST]);
					dpsScreen.addDamageIn(this.mysticDamage, damage[this.DamageType.MYSTIC]);
					dpsScreen.addDamageIn(this.deathDamage, damage[this.DamageType.DEATH]);
					dpsScreen.addDamageIn(this.unblockableDamage, damage[this.DamageType.UNBLOCKABLE]);
					dpsScreen.addSwingIn(true);
				}

				channelName = "mci";
			}
			else if (recipientIsNPC)
			{
				for( local i = 0; i < immunity.len(); i++ )
				{
					if (immunity[i] == true)
					{
						immunityMessage += " " + target.getName() + " is immune to " + this.DamageTypeNameMappingLower[i] + "!";
					}
				}

				if (!abilityDamage)
				{
					combatMessage = source.getName() + criticalHitModifier + "hits " + target.getName() + " for " + damageOutputString + " damage";
				}
				else
				{
					combatMessage = source.getName() + " performs " + abilityName + criticalHitModifier + "hitting " + target.getName() + " for " + damageOutputString + " damage";
				}

				channelName = "oco";
			}
			else
			{
				for( local i = 0; i < immunity.len(); i++ )
				{
					if (immunity[i] == true)
					{
						immunityMessage += " " + target.getName() + " is immune to " + this.DamageTypeNameMappingLower[i] + "!";
					}
				}

				if (!abilityDamage)
				{
					combatMessage = source.getName() + criticalHitModifier + "hits " + target.getName() + " for " + damageOutputString + " damage";
				}
				else
				{
					combatMessage = source.getName() + " performs " + abilityName + criticalHitModifier + "hitting " + target.getName() + " for " + damageOutputString + " damage";
				}

				channelName = "oci";
			}

			if (absorbedDamage > 0)
			{
				combatMessage += " (absorbed " + absorbedDamage + ")";
			}

			combatMessage += hitPunctuation;
			combatMessage += immunityMessage;
			target.addCombatMessage(channelName, combatMessage);
			break;

		case 8:
			if (this.mProtocolVersionId < 15)
			{
				break;
			}

			local target = this.mSceneObjectManager.getCreatureByID(id);

			if (target)
			{
				local source = this.mSceneObjectManager.getCreatureByID(data.getInteger());
				local recipientIsNPC = false;

				if (target.mCreatureDef != null)
				{
					recipientIsNPC = !target.mCreatureDef.getMeta("persona");
				}

				if (source == ::_avatar)
				{
					this.IGIS.floatie("Miss", this.IGIS.FLOATIE_STATUS_WHITE, source);
				}
				else
				{
					this.IGIS.floatie("Miss", this.IGIS.FLOATIE_STATUS_RED, source);
				}

				source.cue("MeleeMiss");
				local playerId = ::_avatar.getID();
				local channelName = "";
				local combatMessage = "";

				if (source == ::_avatar)
				{
					combatMessage = "You miss " + target.getName() + "!";
					channelName = "mco";
					local dpsScreen = this.Screens.get("DPSMeter", false);

					if (dpsScreen)
					{
						dpsScreen.addSwingOut(false);
					}
				}
				else if (target == ::_avatar)
				{
					combatMessage = source.getName() + " misses you!";
					local currentTarget = ::_avatar.getTargetObject();

					if (!currentTarget)
					{
						::_avatar.setResetTabTarget(true);
						::_avatar.setTargetObject(source);
						::_Connection.sendSelectTarget(source.getID());
					}

					local dpsScreen = this.Screens.get("DPSMeter", false);

					if (dpsScreen)
					{
						dpsScreen.addSwingIn(false);
					}

					channelName = "mci";
				}
				else if (recipientIsNPC)
				{
					combatMessage = source.getName() + " misses " + target.getName() + "!";
					channelName = "oco";
				}
				else
				{
					combatMessage = source.getName() + " misses " + target.getName() + "!";
					channelName = "oci";
				}

				::_ChatManager.addMessage(channelName, combatMessage);
			}

			break;

		case 9:
			local creature = this.mSceneObjectManager.getCreatureByID(id);

			if (creature)
			{
				if (creature == ::_avatar)
				{
					this.IGIS.floatie("Parry", this.IGIS.FLOATIE_STATUS_YELLOW, creature);
				}
				else
				{
					this.IGIS.floatie("Parry", this.IGIS.FLOATIE_STATUS_WHITE, creature);
				}
			}

			break;

		case 10:
			break;

		case 11:
			local name = data.getStringUTF();
			local delay = data.getInteger().tofloat();
			local countdown = this.Screens.get("CreatureActionCountdown", true);

			if (countdown)
			{
				if (delay == -1)
				{
					countdown.interruptAction();
				}
				else
				{
					countdown.setAction(delay, name);
					this.Screens.show("CreatureActionCountdown");
				}
			}

			break;

		case 12:
			local effect = data.getStringUTF();
			local targetId = data.getInteger();

			if (!this.mSceneObjectManager.hasCreature(id) || !this.mSceneObjectManager.hasCreature(targetId))
			{
				break;
			}

			local c1 = this.mSceneObjectManager.getCreatureByID(id);
			local c2 = this.mSceneObjectManager.getCreatureByID(targetId);
			c1.cue(effect, [
				c2
			]);
			break;

		case 13:
			local reason = data.getStringUTF();
			this.IGIS.info(reason);
			local lootScreen = this.Screens.get("LootScreen", true);
			lootScreen.setVisible(false, true);
			break;

		case 14:
			local sourceHengeId = data.getInteger();
			local hengeListSize = data.getByte();
			local hengeList = [];

			for( local i = 0; i < hengeListSize; i++ )
			{
				hengeList.append({
					name = data.getStringUTF(),
					cost = data.getInteger()
				});
			}

			local hengeScreen = this.Screens.get("HengeSelectionScreen", true);
			hengeScreen.setVisible(true);

			if (hengeScreen)
			{
				hengeScreen.addHenges(sourceHengeId, hengeList);
			}

			local mapWindow = this.Screens.get("MapWindow", true);

			if (mapWindow)
			{
				mapWindow.refetchMarkers();
			}

			break;

		case 15:
			local target = this.mSceneObjectManager.getCreatureByID(id);
			local source;

			if (this.mProtocolVersionId > 25)
			{
				local sourceId = data.getInteger();
				source = this.mSceneObjectManager.getCreatureByID(sourceId);
			}

			local amount = data.getInteger();
			local playerId = ::_avatar.getID();
			local channelName = "";
			local combatMessage = "";
			local recipientIsNPC = false;

			if (target.mCreatureDef != null)
			{
				recipientIsNPC = !target.mCreatureDef.getMeta("persona");
			}

			if (source == ::_avatar)
			{
				combatMessage = "You heal ";

				if (target == ::_avatar)
				{
					combatMessage += "yourself";
				}
				else
				{
					combatMessage += target.getName();
				}

				channelName = "mco";
			}
			else if (target == ::_avatar)
			{
				combatMessage = target.getName() + " heals you ";
				channelName = "mco";
			}
			else
			{
				combatMessage = source.getName() + " heals " + target.getName();
				channelName = "oci";
			}

			combatMessage += " for " + amount + " points of Health!";

			if (target && amount > 0)
			{
				if (target == ::_avatar)
				{
					this.IGIS.floatie(amount.tostring(), this.IGIS.FLOATIE_STATUS_GREEN_MEDIUM, target);
				}
				else
				{
					this.IGIS.floatie(amount.tostring(), this.IGIS.FLOATIE_STATUS_GREEN_MEDIUM, target);
				}
			}

			::_ChatManager.addMessage(channelName, combatMessage);
			break;

		case 16:
			local creature = this.mSceneObjectManager.getCreatureByID(id);

			if (creature)
			{
				if (creature == ::_avatar)
				{
					this.IGIS.floatie("Silenced!", this.IGIS.FLOATIE_STATUS_ORANGE_BIG, creature);
				}
				else
				{
					this.IGIS.floatie("Silenced!", this.IGIS.FLOATIE_STATUS_YELLOW_BIG, creature);
				}
			}

			break;

		case 17:
			local creature = this.mSceneObjectManager.getCreatureByID(id);

			if (creature)
			{
				if (creature == ::_avatar)
				{
					this.IGIS.floatie("Disarmed!", this.IGIS.FLOATIE_STATUS_ORANGE_BIG, creature);
				}
				else
				{
					this.IGIS.floatie("Disarmed!", this.IGIS.FLOATIE_STATUS_YELLOW_BIG, creature);
				}
			}

			break;

		case 18:
			if (this.mProtocolVersionId < 15)
			{
				break;
			}

			local dodger = this.mSceneObjectManager.getCreatureByID(id);

			if (dodger)
			{
				local attacker = this.mSceneObjectManager.getCreatureByID(data.getInteger());

				if (dodger == ::_avatar)
				{
					this.IGIS.floatie("Dodge", this.IGIS.FLOATIE_STATUS_YELLOW, dodger);
				}
				else
				{
					this.IGIS.floatie("Dodge", this.IGIS.FLOATIE_STATUS_WHITE, attacker);
				}

				local channelName = "";
				local combatMessage = "";

				if (attacker == ::_avatar)
				{
					combatMessage = dodger.getName() + " dodges your attack!";
					channelName = "mco";
					local dpsScreen = this.Screens.get("DPSMeter", false);

					if (dpsScreen)
					{
						dpsScreen.addSwingOut(false);
					}
				}
				else if (dodger == ::_avatar)
				{
					combatMessage = "You dodge " + attacker.getName() + "\'s attack!";
					local currentTarget = ::_avatar.getTargetObject();

					if (!currentTarget)
					{
						::_avatar.setResetTabTarget(true);
						::_avatar.setTargetObject(attacker);
						::_Connection.sendSelectTarget(attacker.getID());
					}

					local dpsScreen = this.Screens.get("DPSMeter", false);

					if (dpsScreen)
					{
						dpsScreen.addSwingIn(false);
					}

					channelName = "mci";
				}

				::_ChatManager.addMessage(channelName, combatMessage);
			}

			break;

		case 19:
			local creature = this.mSceneObjectManager.getCreatureByID(id);

			if (creature)
			{
				if (creature == ::_avatar)
				{
					this.IGIS.floatie("Block", this.IGIS.FLOATIE_STATUS_YELLOW, creature);
				}
				else
				{
					this.IGIS.floatie("Block", this.IGIS.FLOATIE_STATUS_WHITE, creature);
				}
			}

			break;

		case 20:
			local creature = this.mSceneObjectManager.getCreatureByID(id);

			if (creature)
			{
				if (creature == ::_avatar)
				{
					this.IGIS.floatie("Spell Failure", this.IGIS.FLOATIE_STATUS_YELLOW, creature);
					::_ChatManager.addMessage("mco", "Your spell failed");
				}
				else
				{
					this.IGIS.floatie("Spell Failure", this.IGIS.FLOATIE_STATUS_WHITE, creature);
					::_ChatManager.addMessage("oco", creature.getName() + " has missed a spell");
				}

				local visualEffect = creature.cue("SpellFailure");
			}

			break;

		case 21:
			local creature = this.mSceneObjectManager.getCreatureByID(id);
			local amount = data.getInteger();

			if (creature)
			{
				this.IGIS.floatie("XP: " + amount.tostring(), this.IGIS.FLOATIE_STATUS_BLUE, creature);
			}

			break;

		case 22:
			local creature = this.mSceneObjectManager.getCreatureByID(id);
			local category = data.getStringUTF();

			if (::_avatar == creature)
			{
				::_AbilityManager.setCategoryCooldownTime(category, 0);
				::_quickBarManager.setCategoryUsable(category, true);
			}

			break;

		case 23:
			local abPointScreen = this.Screens.get("AbilityPointBuyScreen", false);

			if (abPointScreen)
			{
				abPointScreen.onAbilityPointBought();
			}

			break;

		case 24:
			local portalRequestScreen = this.Screens.get("PortalRequest", true);
			local teleporter = data.getStringUTF();
			local destination = data.getStringUTF();

			if (portalRequestScreen)
			{
				portalRequestScreen.showPortalRequest(teleporter + " requests to teleport you to " + destination + ".");
			}

			break;

		case 25:
			local newSize = data.getInteger();
			// TODO - Added for ICEE need a way to differentiate between a base protocol 33 and ICE one. need a 'modversion' 
			local newSlots = data.getInteger();
			local vault = this.Screens.get("Vault", false);
			if (vault)
			{
				vault.setVaultSize(newSize);
				vault.setDeliveryBoxSlots(newSlots);
			}

			break;

		case 26:
			local hb = data.getByte();
			local rb = data.getByte();
			local heading = this.rot2rad(hb);
			local rotation = this.rot2rad(rb);
			local speed = data.getByte() & 255;
			
			if (::_avatar)
			{
				// TODO ... not suure about this, just trying to force an update
				::_avatar.mLastServerUpdate = null;
				::_avatar.onServerVelocity(heading, rotation, speed);
			}

			break;

		case 27:
			local attacker = this.mSceneObjectManager.getCreatureByID(id);
			local reflector = this.mSceneObjectManager.getCreatureByID(data.getInteger());
			local damageString = data.getStringUTF();
			local absorbedDamage = data.getInteger();
			local damage = this.array(this.DamageType.len(), 0);
			local splitDamageString = this.Util.split(damageString, "|");
			local damageOutputString = "";
			local combatMessage = "";
			local channelName = "";

			foreach( damgeType in splitDamageString )
			{
				if (damgeType == "")
				{
					continue;
				}

				if (damageOutputString != "")
				{
					damageOutputString += ", ";
				}

				local splitString = this.Util.split(damgeType, ":");
				local damageType = splitString[0].tointeger();
				damage[damageType] = splitString[1].tointeger();
				damageOutputString += damage[damageType] + " points of " + this.DamageTypeNameMappingLower[damageType];
			}

			local recipientIsNPC = false;

			if (reflector.mCreatureDef != null)
			{
				recipientIsNPC = !reflector.mCreatureDef.getMeta("persona");
			}

			local totalDamage = 0;

			for( local i = 0; i < damage.len(); i++ )
			{
				totalDamage += damage[i];
			}

			if (reflector == ::_avatar)
			{
				combatMessage = "You reflect " + damageOutputString + " damage to " + attacker.getName();
				channelName = "mco";
			}
			else if (attacker == ::_avatar)
			{
				combatMessage = reflector.getName() + " reflects " + damageOutputString + " damage to you";
				channelName = "mci";
			}
			else
			{
				combatMessage = reflector.getName() + " reflects " + damageOutputString + " damage to " + attacker.getName();
				channelName = "oco";
			}

			if (absorbedDamage > 0)
			{
				combatMessage += " (absorbed " + absorbedDamage + ")";
			}

			combatMessage += ".";

			if (attacker && totalDamage > 0)
			{
				attacker.addFloatie("-" + totalDamage.tostring(), this.IGIS.FLOATIE_STATUS_RED);
			}

			attacker.addCombatMessage(channelName, combatMessage);
			break;

		case 28:
			local regenerator = this.mSceneObjectManager.getCreatureByID(id);
			local amountRegened = data.getInteger();
			local combatMessage = "";
			local channelName = "";

			if (regenerator == ::_avatar)
			{
				combatMessage = "You regenerate " + amountRegened + " points of health.";
				channelName = "mco";
			}
			else
			{
				combatMessage = regenerator.getName() + " regenerates " + amountRegened + " points of health.";
				channelName = "oco";
			}

			regenerator.addCombatMessage(channelName, combatMessage);

			if (regenerator == ::_avatar)
			{
				this.IGIS.floatie(amountRegened.tostring(), this.IGIS.FLOATIE_STATUS_GREEN_MEDIUM, regenerator);
			}
			else
			{
				this.IGIS.floatie(amountRegened.tostring(), this.IGIS.FLOATIE_STATUS_GREEN_MEDIUM, regenerator);
			}

			break;
		}
	}

	function _handleCreatureUpdateMsg( data )
	{
		local creatureId = data.getInteger();
		local mask = ::_Connection.getProtocolVersionId() < 22 ? data.getByte() : data.getShort();
		local creatureDef;
		local creature;
		local pos;
		local x;
		local y;
		local z;
		local defHints = 0;
		local defHintsExtraData;

		if (creatureId == 0)
		{
			creature = null;
			defHints = ::_Connection.getProtocolVersionId() < 13 ? data.getByte() : data.getShort();

			if (::_Connection.getProtocolVersionId() > 26)
			{
				defHintsExtraData = data.getStringUTF();
			}
		}
		else
		{
			creature = this.mSceneObjectManager.getCreatureByID(creatureId);
			creature.setZoneID(this.mSceneObjectManager.getCurrentZoneID());
			creature.resetTimeSinceLastUpdate();
			pos = creature.getPosition();
			x = pos.x;
			y = pos.y;
			z = pos.z;
		}

		local positionUpdated = false;

		if (mask & this.CREATURE_UPDATE_TYPE)
		{
			local creatureDefId = data.getInteger();
			creatureDef = ::_creatureDefManager.getCreatureDef(creatureDefId);

			if (creature != null)
			{
				creature.setShowName(true);
				creature.setType(creatureDefId);
			}
			else
			{
				if ((defHints & this.CDEF_HINT_PERSONA) != 0)
				{
					creatureDef.setMeta("persona", true);
					creatureDef.setPlayer(true);
				}

				if ((defHints & this.CDEF_HINT_COPPER_SHOPKEEPER) != 0)
				{
					creatureDef.setMeta("copper_shopkeeper", true);
				}

				if ((defHints & this.CDEF_HINT_CREDIT_SHOPKEEPER) != 0)
				{
					creatureDef.setMeta("credit_shopkeeper", true);
				}

				if ((defHints & this.CDEF_HINT_ESSENCE_VENDOR) != 0)
				{
					creatureDef.setMeta("essence_vendor", true);
				}

				if ((defHints & this.CDEF_HINT_VAULT) != 0)
				{
					creatureDef.setMeta("vault", true);
				}

				if ((defHints & this.CDEF_HINT_QUEST_GIVER) != 0)
				{
					creatureDef.setMeta("quest_giver", true);
				}

				if ((defHints & this.CDEF_HINT_QUEST_ENDER) != 0)
				{
					creatureDef.setMeta("quest_ender", true);
				}

				if ((defHints & this.CDEF_HINT_CRAFTER) != 0)
				{
					creatureDef.setMeta("crafter", true);
				}

				if ((defHints & this.CDEF_HINT_CLANREGISTRAR) != 0)
				{
					creatureDef.setMeta("clan_registrar", true);
				}

				if ((defHints & this.CDEF_HINT_CREDIT_SHOP) != 0)
				{
					creatureDef.setMeta("credit_shop", defHintsExtraData);
				}
			}
		}

		if (mask & this.CREATURE_UPDATE_ZONE)
		{
			local zoneId = this.mProtocolVersionId > 16 ? data.getStringUTF() : data.getInteger();
			x = data.getInteger().tofloat();
			z = data.getInteger().tofloat();
			positionUpdated = true;

			if (creature != ::_avatar)
			{
				creature.mLastServerUpdate = null;
			}
		}

		if (mask & this.CREATURE_UPDATE_ELEVATION)
		{
			y = data.getShort().tofloat();
			positionUpdated = true;
		}

		if (positionUpdated)
		{
			if (creature == ::_avatar)
			{
				local np = this.Vector3(x, y, z);
				local dist = creature.getPosition().distance(np);

				if (dist > 150.0)
				{
					::_loadScreenManager.setLoadScreenVisible(true, true);
					::_avatar.reassemble();
				}
			}
		}

		if (mask & this.CREATURE_UPDATE_ELEVATION && mask & this.CREATURE_UPDATE_ZONE)
		{
			local pos = this.Util.safePointOnFloor(this.Vector3(x, y, z), creature.getNode());

			if (creature != ::_avatar || pos.distance(creature.getPosition()) > 50)
			{
				creature.setPosition(pos);
			}
		}

		if (mask & this.CREATURE_UPDATE_POSITION_INC)
		{
			local esx = data.getShort() & 65535;
			local esz = data.getShort() & 65535;

			if (::_avatar != null)
			{
				local vpos = ::_avatar.getPosition();
				x = this._decodeTruncatedCoord(vpos.x, esx);
				z = this._decodeTruncatedCoord(vpos.z, esz);
				positionUpdated = true;
			}
		}

		if (positionUpdated)
		{
			creature.onServerPosition(x, y, z);
			creature._notifyUpdateReceived();

			if (::_avatar != null)
			{
				local target = ::_avatar.getTargetObject();

				if (target && (target.getID() == creature.getID() || creature.getID() == ::_avatar.getID()))
				{
					this.broadcastMessage("onTargetPositionUpdated", target);
				}
			}
		}

		if (mask & this.CREATURE_UPDATE_VELOCITY)
		{
			local hb = data.getByte();
			local rb = data.getByte();
			local heading = this.rot2rad(hb);
			local rotation = this.rot2rad(rb);
			local speed = data.getByte() & 255;

			if (creature != ::_avatar)
			{
				creature.onServerVelocity(heading, rotation, speed);
			}
		}

		if (this.mProtocolVersionId < 25 && mask & this.CREATURE_UPDATE_LOGIN_POSITION)
		{
			local hb = data.getByte();
			local rb = data.getByte();
			local heading = this.rot2rad(hb);
			local rotation = this.rot2rad(rb);
			local speed = data.getByte() & 255;

			if (creature == ::_avatar)
			{
				::_avatar.onServerVelocity(heading, rotation, speed);
			}
		}

		if (mask & this.CREATURE_UPDATE_MOD)
		{
			local modCount = data.getShort();
			local mods = [];

			for( local x = 0; x < modCount; x++ )
			{
				local id;
				local abilityId;
				local amount;
				local priority;
				local description;
				priority = data.getInteger();
				id = data.getShort();
				abilityId = data.getShort();

				if (priority == 1)
				{
					amount = data.getFloat();
				}
				else
				{
					amount = data.getShort();
				}

				local duration = data.getInteger();

				if (this.mProtocolVersionId >= 24)
				{
					description = data.getStringUTF();
				}

				local statusModifier;
				
				statusModifier = this.StatusModifier(id, abilityId, amount, duration, description);
				mods.append(statusModifier);
			}

			local effectCount = data.getShort();
			local effects = {};

			for( local x = 0; x < effectCount; x++ )
			{
				effects[data.getShort()] <- true;
			}

			creature.setStatusModifiers(mods, effects);

			if (creature == ::_avatar)
			{
				local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

				if (selfTargetWindow)
				{
					selfTargetWindow.updateStatus();
				}

				this.broadcastMessage("onStatusUpdate", creature);
			}
		}

		if (mask & this.CREATURE_UPDATE_STAT)
		{
			local count = data.getShort();
			local i;
			local statTarget;

			if (creature == null)
			{
				statTarget = creatureDef;
			}
			else
			{
				statTarget = creature;
			}

			local isAvatarUpdate = ::_avatar && (::_avatar == statTarget || ::_avatar.getCreatureDef() == statTarget);

			for( i = 0; i < count; i++ )
			{
				local statId = data.getShort();
				local value;

				if (!(statId in ::Stat))
				{
					throw this.Exception("Unknown stat ID: " + statId);
				}

				local statDef = ::Stat[statId];

				if (this.mProtocolVersionId < 16)
				{
					if (statId == this.Stat.WILL_REGEN)
					{
						statDef.type = "short";
					}
					else if (statId == this.Stat.MIGHT_REGEN)
					{
						statDef.type = "short";
					}
				}

				switch(statDef.type)
				{
				case "string":
					value = data.getStringUTF();
					break;

				case "short":
					value = data.getShort();
					break;

				case "int":
					value = data.getInteger();
					break;

				case "float":
					value = data.getFloat();
					break;

				default:
					throw this.Exception("Unknown data type: " + statDef.type);
				}

				if (isAvatarUpdate)
				{
					if (statId == this.Stat.CREDITS)
					{
						this.broadcastMessage("onCreditUpdated", value);
					}

					if (statId == this.Stat.COPPER)
					{
						local oldCopper = ::_avatar.getStat(this.Stat.COPPER);

						if (oldCopper == null)
						{
							oldCopper = 0;
						}

						local difference = oldCopper - value;
						local coinMessage = "You lose";

						if (value > oldCopper)
						{
							difference = value - oldCopper;
							coinMessage = "You gain";
						}

						local currencyComponent = this.GUI.Currency();
						currencyComponent.setCurrentValue(difference);
						local gold = currencyComponent.getGold();
						local silver = currencyComponent.getSilver();
						local copper = currencyComponent.getCopper();

						if (gold > 0 || silver > 0 || copper > 0)
						{
							if (gold > 0)
							{
								coinMessage += " " + gold + " gold";
							}

							if (silver > 0)
							{
								coinMessage += " " + silver + " silver";
							}

							if (copper > 0)
							{
								coinMessage += " " + copper + " copper";
							}

							coinMessage += ".";
							this.IGIS.info(coinMessage);
						}

						this.broadcastMessage("onCopperUpdated", value);
					}
				}

				statTarget.setStat(statId, value);

				if (isAvatarUpdate)
				{
					if (statId == this.Stat.LEVEL)
					{
						this.broadcastMessage("onLevelUpdate", value);
						this.LoadGate.AutoFetchLoadGate(this.GateTrigger.TriggerTypes.LEVEL, value);
					}
					else if (statId == this.Stat.TOTAL_ABILITY_POINTS || statId == this.Stat.CURRENT_ABILITY_POINTS)
					{
						this.broadcastMessage("onAbilityPointsUpdate");
					}
					else if (statId == this.Stat.BASE_LUCK)
					{
					}
					else if (statId == this.Stat.PROFESSION)
					{
						this.broadcastMessage("onProfessionUpdate", value);
					}
				}

				if (statId == this.Stat.HIDE_NAMEBOARD)
				{
					statTarget._positionName();
				}
			}

			if (isAvatarUpdate)
			{
				this._AbilityManager.avatarStatsUpdated();
				local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

				if (selfTargetWindow)
				{
					selfTargetWindow.fillOut();
				}

				local mainScreen = this.Screens.get("MainScreen", false);

				if (mainScreen)
				{
					mainScreen.fillOut();
				}

				this.broadcastMessage("onStatusUpdate", statTarget);
			}
		}

		if (creature != null)
		{
			creature.mLastServerUpdate = this._time;
			creature.fireUpdate();
			this.broadcastMessage("onCreatureUpdated", this, creature);

			if (!creature.isAssembled())
			{
				::_sceneObjectManager.queueAssembly(creature);
			}
		}
	}

	function _handlePartyUpdateMsg( data )
	{
		this.log.debug("_handlePartyUpdateMsg()");

		switch(data.getByte())
		{
		case this.PartyUpdateOpTypes.INVITE:
			local leaderId = data.getInteger();
			local leaderName = data.getStringUTF();
			this.partyManager.invited(leaderName, leaderId);
			break;

		case this.PartyUpdateOpTypes.INVITE_REJECTED:
			this.partyManager.invitationRejected(data.getStringUTF());
			break;

		case this.PartyUpdateOpTypes.JOINED_PARTY:
			local size = data.getByte();
			local leader = data.getInteger();
			local memberId = data.getInteger();
			local party = [];

			for( local i = 0; i < size; i++ )
			{
				local id = data.getInteger();
				local name = data.getStringUTF();
				party.append({
					id = id,
					name = name
				});
			}

			this.partyManager.joinParty(party, leader, memberId);
			break;

		case this.PartyUpdateOpTypes.IN_CHARGE:
			local leaderId = data.getInteger();
			local leaderName = data.getStringUTF();
			this.partyManager.inCharge(leaderId, leaderName);
			break;

		case this.PartyUpdateOpTypes.LEFT_PARTY:
			this.partyManager.leaveParty();
			break;

		case this.PartyUpdateOpTypes.ADD_MEMBER:
			local memberId = data.getInteger();
			local memberName = data.getStringUTF();
			this.partyManager.addMember({
				name = memberName,
				id = memberId
			});
			break;

		case this.PartyUpdateOpTypes.REMOVE_MEMBER:
			local memberId = data.getInteger();
			this.partyManager.removeMember(memberId);
			break;

		case this.PartyUpdateOpTypes.PROPOSE_INVITE:
			local proposeeId = data.getInteger();
			local proposeeName = data.getStringUTF();
			local proposerId = data.getInteger();
			local proposerName = data.getStringUTF();
			this.partyManager.proposeInvitation(proposeeId, proposeeName, proposerId, proposerName);
			break;

		case this.PartyUpdateOpTypes.STRATEGY_CHANGE:
			local newMode = data.getInteger();
			this.partyManager.strategyChange(newMode);
			break;

		case this.PartyUpdateOpTypes.STRATEGYFLAGS_CHANGE:
			local newFlags = data.getInteger();
			this.partyManager.strategyFlagsChange(newFlags);
			break;

		case this.PartyUpdateOpTypes.OFFER_LOOT:
			local lootTag = data.getStringUTF();
			local itemDefId = data.getInteger();
			local needed = data.getByte() == 0 ? false : true;
			this.partyManager.offerLoot(lootTag, itemDefId, needed);
			break;

		case this.PartyUpdateOpTypes.LOOT_ROLL:
			local itemDefName = data.getStringUTF();
			local roll = data.getByte();
			local pname = data.getStringUTF();
			this.IGIS.info(pname + " rolled " + roll + " bidding for " + itemDefName);
			break;

		case this.PartyUpdateOpTypes.LOOT_WIN:
			local tag = data.getStringUTF();
			local originalTag = data.getStringUTF();
			local winner = data.getStringUTF();
			local parts = this.Util.split(tag, ":");
			local creatureId = parts[0].tointeger();
			local slotIndex = parts[1].tointeger();
			this.broadcastMessage("onLootGone", creatureId, slotIndex);
			this.partyManager.lootWon(tag, originalTag, winner);
			break;

		case this.PartyUpdateOpTypes.QUEST_INVITE:
			local questName = data.getStringUTF();
			local questId = data.getInteger();
			local callback = {
				quest = questName,
				id = questId,
				function onActionSelected( mb, alt )
				{
					if (alt == "Yes")
					{
						::_Connection.sendQuery("quest.share", this, [
							this.id
						]);
					}
				}

				function onQueryComplete( qa, rows )
				{
				}

			};
			this.GUI.MessageBox.showYesNo("You have been invited to join quest: " + questName, callback);
			break;
		}

		this.broadcastMessage("onPartyUpdated", this);
	}

	function _handleTradeMsg( data )
	{
		local traderId = data.getInteger();
		local eventType = data.getByte();

		if (eventType == this.TradeEventTypes.REQUEST)
		{
			::_TradeManager.askToTrade(traderId);
		}
		else if (eventType == this.TradeEventTypes.REQUEST_ACCEPTED)
		{
			::_TradeManager.onTradeAccepted();
		}
		else if (eventType == this.TradeEventTypes.ITEM_ADDED)
		{
			local itemProto = data.getStringUTF();
			::_TradeManager.onItemAdded(itemProto);
		}
		else if (eventType == this.TradeEventTypes.ITEM_REMOVED)
		{
			local itemId = data.getStringUTF();
			::_TradeManager.onItemRemoved(itemId);
		}
		else if (eventType == this.TradeEventTypes.CURRENCY_OFFERED)
		{
			local currenciesUpdated = data.getByte();

			for( local i = 0; i < currenciesUpdated.tointeger(); i++ )
			{
				local currencyType = data.getByte();
				local currencyAmt = data.getInteger();
				::_TradeManager.onCurrencyChanged(currencyType, currencyAmt);
			}

			if (currenciesUpdated == 0)
			{
				::_TradeManager.onCurrencyChanged(this.CurrencyCategory.COPPER, 0);
			}
		}
		else if (eventType == this.TradeEventTypes.OFFER_MADE)
		{
			::_TradeManager.onOfferMade();
		}
		else if (eventType == this.TradeEventTypes.OFFER_ACCEPTED)
		{
		}
		else if (eventType == this.TradeEventTypes.OFFER_CANCELED)
		{
			::_TradeManager.offerCancel();
		}
		else if (eventType == this.TradeEventTypes.REQUEST_CLOSED)
		{
			local closeReason = data.getByte();
			::_TradeManager.onTradeCancelled(closeReason);
		}
		else if (eventType == this.TradeEventTypes.ITEMS_OFFERED)
		{
			local itemProtos = this._readStringArray(data);
			::_TradeManager.addItems(itemProtos);
		}
	}

	function _handleCommunicationMsg( data )
	{
		local speakerID = data.getInteger();
		local speakerName = data.getStringUTF();
		local channel = data.getStringUTF();
		local text = data.getStringUTF();

		if (channel == "emote")
		{
			local c = ::_sceneObjectManager.hasCreature(speakerID);

			if (c)
			{
				local animHandler = c.getAnimationHandler();

				if (animHandler)
				{
					animHandler.onFF(text);
				}
			}
		}
		else if (::_ChatManager)
		{
			::_ChatManager.addMessage(channel, text, speakerID, speakerName);
		}
	}

	function chatError( argument )
	{
		this.log.debug("******************* CHAT ERROR *****************************");
		this.log.debug(argument);
		this.log.debug("******************* END CHAT ERROR **************************");
	}

	function _handleSceneryUpdateMsg( data )
	{
		local scenery = this.mSceneObjectManager.getSceneryByID(data.getInteger());
		local mask = data.getByte();
		local asset;

		if ((mask & this.SCENERY_UPDATE_ASSET) != 0)
		{
			asset = data.getStringUTF();

			if (this.mProtocolVersionId >= 23)
			{
				scenery.setSceneryLayer(data.getStringUTF());
			}
		}

		local xformUpdate = false;
		this.log.debug("Scenery update: " + scenery + ", " + asset);

		if ((mask & this.SCENERY_UPDATE_POSITION) != 0)
		{
			local x;
			local y;
			local z;
			x = data.getFloat() * this.gServerScale;
			y = data.getFloat() * this.gServerScale;
			z = data.getFloat() * this.gServerScale;
			scenery.setPosition(this.Vector3(x, y, z));
			xformUpdate = true;
		}

		if ((mask & this.SCENERY_UPDATE_ORIENTATION) != 0)
		{
			local x;
			local y;
			local z;
			local w;
			x = data.getFloat();
			y = data.getFloat();
			z = data.getFloat();
			w = data.getFloat();
			scenery.setOrientation(this.Quaternion(w, x, y, z));
			xformUpdate = true;
		}

		if ((mask & this.SCENERY_UPDATE_SCALE) != 0)
		{
			local x;
			local y;
			local z;
			x = data.getFloat();
			y = data.getFloat();
			z = data.getFloat();
			scenery.setScale(this.Vector3(x, y, z));
			xformUpdate = true;
		}

		if ((mask & this.SCENERY_UPDATE_FLAGS) != 0)
		{
			scenery.setFlags(data.getInteger());
		}

		if ((mask & this.SCENERY_UPDATE_LINKS) != 0)
		{
			local count = data.getShort();

			for( local i = 0; i < count; i++ )
			{
				local targetId = data.getInteger();
				local type = data.getByte();

				if (this.mSceneObjectManager.hasScenery(targetId))
				{
					local target = this._sceneObjectManager.getSceneryByID(targetId);
					this._scene.addLink(scenery.getNodeName(), target.getNodeName(), type == 0 ? this.Color(1.0, 0.0, 1.0, 1.0) : this.Color(0.0, 1.0, 1.0, 1.0));
				}
			}
		}

		if ((mask & this.SCENERY_UPDATE_PROPERTIES) != 0)
		{
			if (data.remaining() > 0)
			{
				local name = data.getStringUTF();
				local count = data.getInteger();
				local props = {};
				props.NAME <- name;
				local x;

				for( x = 0; x < count; x++ )
				{
					local name = data.getStringUTF();
					local type = data.getByte();
					local value;

					switch(type)
					{
					case this.PROPERTY_INTEGER:
						value = data.getInteger();
						break;

					case this.PROPERTY_FLOAT:
						value = data.getFloat();
						break;

					case this.PROPERTY_STRING:
						value = data.getStringUTF();
						break;

					case this.PROPERTY_SCENERY:
						value = data.getInteger();
						break;
					}

					props[name] <- value;
				}

				scenery.setProperties(props);
			}
		}

		if (this.mSceneObjectManager.getSceneryInRange(scenery) == false)
		{
			this.log.debug("Received update for out of range scenery " + scenery + " (" + asset + ")");
			scenery.destroy();
			return;
		}

		if ((mask & this.SCENERY_UPDATE_ASSET) != 0)
		{
			if (asset.len() == 0)
			{
				scenery.destroy();
				return;
			}
			else
			{
				local a = this.AssetReference(asset);
				local ass = a.getAsset();

				if (ass == "")
				{
					this.IGIS.error("Invalid asset string, please copy this and bug it!!: " + asset);
					return;
				}

				if (scenery.setType(a.getAsset(), a.getVars()))
				{
					scenery.reassemble();
				}
			}

			xformUpdate = false;
		}

		scenery.fireUpdate();

		if (xformUpdate && scenery.isAssembled())
		{
			scenery.reassemble();
		}
	}

	function _handleQueryResultMsg( data )
	{
		local id = data.getInteger();
		local count = data.getShort();
		local error;
		local rows;

		if (count & 28672)
		{
			error = data.getStringUTF();
		}
		else
		{
			rows = [];
			local i;

			for( i = 0; i < count; i++ )
			{
				rows.append(this._readStringArray(data));
			}
		}

		if (id in this.mPendingQueries)
		{
			local q = this.mPendingQueries[id];
			delete this.mPendingQueries[id];
			::_eventScheduler.cancel(q._timeoutEvent);
			delete q._timeoutEvent;

			if (this.mQueryQueue.len() > 0)
			{
				this._sendQuery(this.mQueryQueue[0]);
				this.mQueryQueue.remove(0);
			}

			local handler = q._handler;

			if (error)
			{
				if ("onQueryError" in handler)
				{
					handler.onQueryError(q, error);
				}
				else
				{
					this.log.warn("Query " + q.query + " failed: " + error);
				}
			}
			else if ("onQueryComplete" in handler)
			{
				handler.onQueryComplete(q, rows);
			}
		}
		else
		{
			this.log.error("Received query result for non-pending query: " + id);
		}
	}

	function _handleQueryTimeout( qa )
	{
		if (qa.correlationId in this.mPendingQueries)
		{
			delete this.mPendingQueries[qa.correlationId];
		}

		if (this.mQueryQueue.len() > 0)
		{
			this._sendQuery(this.mQueryQueue[0]);
			this.mQueryQueue.remove(0);
		}

		this.log.warn("Query " + qa.query + " [" + qa.correlationId + "] timed out");

		if ("onQueryTimeout" in qa._handler)
		{
			qa._handler.onQueryTimeout(qa);
		}
	}
	
	function _handleShake(data) {
		local v1 = data.getFloat();
		local v2 = data.getFloat();
		local v3 = data.getFloat();
		::_playTool.addShaky(::_avatar.getPosition(), v1, v2, v3);
	}
	
	function _handleSceneryEffectMsg(data)
	{
		local type = data.getByte();
		switch(type)
		{
		case 1:
			// Type 1 - Scenery affect creation
			
			local sceneryId = data.getInteger();
			local effectType = data.getInteger();
			local effectTag = data.getInteger();
			local scenery = this.mSceneObjectManager.getSceneryByID(sceneryId);
			if(scenery) {
				local effectName = data.getStringUTF();
				// TODO implement these as offsets from the prop
				local effectX = data.getFloat();
				local effectY = data.getFloat();
				local effectZ = data.getFloat();
				local size = data.getFloat();
				if(effectType == 1) {
				
					// Effect Type 1 - Add a particle effect to the prop				
					scenery.attachParticleSystem(effectName, effectTag, size)
				}
				else if(effectType == 2) {
					// TODO store the old asset somewhere
					// TODO handle multiple asset changes
					local previousAsset = scenery.mPreviousAsset;
					if(previousAsset == null) {
						scenery.mPreviousAsset = scenery.getTypeString();
						scenery.mPreviousScale = scenery.getScale();
					}
					this.log.debug("Scenery update: " + scenery + ", " + effectName + " (" + previousAsset + ")\n");
					local a = this.AssetReference(effectName);
					local ass = a.getAsset();
					if (ass == "")
					{
						this.IGIS.error("Invalid asset string, please copy this and bug it!!: " + asset);
						return;
					}
					if (scenery.setType(a.getAsset(), a.getVars()))
					{
						scenery.reassemble();
					}
					scenery.setScale(this.Vector3(size, size, size));
					scenery.fireUpdate();
				}
				else if (effectType == 3) {
					scenery.setTransformationSequence(unserialize(effectName));
					scenery.fireUpdate();
				}
			}
			else {
				print("ICE! No scenery with ID of " + sceneryId + " (" + effectTag + ")\n");
			}
			break;
		case 2:
			local sceneryId = data.getInteger();
			local effectType = data.getInteger();
			local effectTag = data.getInteger();
			local scenery = this.mSceneObjectManager.getSceneryByID(sceneryId);
			if(scenery) {
				if(effectType == 1) {
					scenery.detachParticleSystem(effectTag)
				}
				else if(effectType == 2) {
					local previousAsset = scenery.mPreviousAsset;
					if(previousAsset != null) {
						if (scenery.setTypeFromString(previousAsset))
						{
							scenery.reassemble();
						}
						scenery.setScale(scenery.mPreviousScale);
						scenery.fireUpdate();
					}
				} else if(effectType == 3) {
					// TODO allow multiple sequences
					scenery.stopTransformationSequence();
				}
			}
			break;
		default:
			print("ICE! Unknown effect message message type " + type);
			break;
		}
	}
	
	function _handleForm(data) {
		local formId = data.getInteger();
		local op = data.getShort();
		
		if(op == 0) {
			local formTitle = data.getStringUTF();
			local formDescription = data.getStringUTF();
			local rows = data.getShort();
			local lastGroup = "ZZZZZZZZZZZZ";
			local form = [];
			local group;
			for(local i = 0 ; i < rows ; i++) {
				local rowGroup = data.getStringUTF();
				if(rowGroup != lastGroup) {
					group = {
						["name"] = rowGroup,
						["items"] = []
					};
					lastGroup = rowGroup;
					form.append(group);
				}
				local rowHeight = data.getShort();
				local itemCount = data.getShort();
				local row = {
					["height"] = rowHeight,
					["items"] = [],
				};
				for(local j = 0 ; j < itemCount; j++) {
					local name = data.getStringUTF();
					local type = data.getShort();
					local value = data.getStringUTF();
					local cells = data.getShort();
					local width = data.getShort();
					local style = data.getStringUTF();
					row["items"].append({
						["name"] = name,
						["type"] = type,
						["value"] = value,
						["width"] = width,
						["cells"] = cells,
						["style"] = style,
					});				
				}
				group["items"].append(row);				
			}
			mForms[formId] <- Screens.ServerForm(formId, formTitle, formDescription, form);
		}
		else if(op == 1) {
			if(formId in mForms) {
				mForms[formId].setVisible(false);
				mForms[formId].destroy();
				delete mForms[formId];			
			}
		}
	}
	
	function _handleRefashion(data) {
		local type = data.getByte();
		local mscreen = this.Screens.get("MorphItemScreen", true);
		mscreen.reset();
		mscreen.setMorpherId(::_avatar.getID());
		this.Screens.show("MorphItemScreen");
	}
	
	function _handleBooks(data) {
		local type = data.getByte();
		switch(type) {
			case 1:
			case 2: 	 			
				local bookId = data.getInteger();
				local pageNumber = data.getInteger();
				local bookScreen = this.Screens.get("Books", true);
				if(bookScreen) {
					Screens.show("Books");
					if(type == 2)
						bookScreen.refreshAndShowBookPage(bookId, pageNumber);
					else
						bookScreen.showBookPage(bookId, pageNumber);
				}
				break;
			case 3:
				local bookId = data.getInteger();
				local pageNumber = data.getInteger();
				local bookScreen = this.Screens.get("Books", true);
				if(bookScreen) {
					bookScreen.refresh();
				}
				break;
		}
	}
	
	function _handleGuildUpdateMsg( data )
	{
		local type = data.getByte();
		switch(type)
		{
		case 1:
			local defId = data.getInteger();
			this.broadcastMessage("onGuildChange", defId);
			break;
		}
	}

	function _handleFriendNotificationMsg( data )
	{
		local type = data.getByte();

		switch(type)
		{
		case 1:
			local name = data.getStringUTF();
			this.broadcastMessage("onPlayerLoggedIn", name);
			::_ChatManager.addMessage("friends", name + " has logged in.");
			break;

		case 2:
			local name = data.getStringUTF();
			this.broadcastMessage("onPlayerLoggedOut", name);
			::_ChatManager.addMessage("friends", name + " has logged out.");
			break;

		case 3:
			local name = data.getStringUTF();
			local status = data.getStringUTF();
			this.broadcastMessage("onPlayerStatusChanged", name, status);
			::_ChatManager.addMessage("friends", name + "\'s status is now \'" + status + "\'.");
			break;

		case 4:
			local name = data.getStringUTF();
			this.broadcastMessage("onFriendAdded", name);
			break;

		case 5:
			local name = data.getStringUTF();
			this.broadcastMessage("onClanMOTDChanged", name);
			::_ChatManager.addMessage("clan", "The clan MoTD is now \'" + name + "\'.");
			break;

		case 6:
			local name = data.getStringUTF();
			this.broadcastMessage("onClanPlayerLoggedIn", name);
			::_ChatManager.addMessage("clan", name + " has logged in.");
			break;

		case 7:
			local name = data.getStringUTF();
			this.broadcastMessage("onClanPlayerLoggedOut", name);
			::_ChatManager.addMessage("clan", name + " has logged out.");
			break;

		case 8:
			this.broadcastMessage("onClanDisbanded");
			::_ChatManager.addMessage("clan", "Your clan has been disbanded.");
			break;

		case 9:
			local name = data.getStringUTF();
			this.broadcastMessage("onClanJoined", name);
			::_ChatManager.addMessage("clan", "You have joined the clan \'" + name + "\'.");
			break;

		case 10:
			local name = data.getStringUTF();
			local leader = data.getStringUTF();
			this.GUI.MessageBox.showYesNo(leader + " has invited you to join the clan \'" + name + "\'.", this.ClanInviteCallback(name));
			::_tutorialManager.onSocialEvent("ClanInvite");
			break;

		case 11:
			local name = data.getStringUTF();
			local level = data.getInteger();
			local proff = data.getInteger();
			this.broadcastMessage("onClanMemberJoined", name, level, proff);
			::_ChatManager.addMessage("clan", name + " has joined your clan.");
			break;

		case 12:
			local name = data.getStringUTF();
			this.broadcastMessage("onClanMemberLeft", name);
			::_ChatManager.addMessage("clan", name + " has left your clan.");
			break;

		case 13:
			::_ChatManager.addMessage("clan", "You have left your clan.");
			this.broadcastMessage("onClanLeft");
			break;

		case 14:
			local name = data.getStringUTF();
			local rank = data.getStringUTF();
			::_ChatManager.addMessage("clan", name + " has changed rank to " + rank);
			this.broadcastMessage("onClanRankChanged", name, rank);
			break;

		case 15:
			local name = data.getStringUTF();
			local shard = data.getStringUTF();
			this.broadcastMessage("onPlayerShardChanged", name, shard);
			break;
		}
	}

	function _handleEnvironmentUpdateMsg( data )
	{
		local mask = data.getByte();
		
		if(mask == 4) 
		{
			local weight = data.getShort();
			this.broadcastMessage("onThunder", weight);
			return;
		}
		
		if(mask == 3) 
		{
			local type = data.getStringUTF();
			local weight = data.getShort();
			this.broadcastMessage("onWeatherUpdate", type, weight);
			return;
		}
		
		local zoneId = this.mProtocolVersionId > 16 ? data.getStringUTF() : data.getInteger();
		local zoneDefId = data.getInteger();
		local zonePageSize = data.getShort(); 
		local terrain = data.getStringUTF();
		local envType = data.getStringUTF();
		
		print("ICE! mask: " +mask + " zoneId: " + zoneId + " zoneDefId: " + zoneDefId + " zonePageSize: " + zonePageSize + " terrain: " + terrain + " envType: " + envType + "\n"); 

		if (mask == 2)
		{
			if (::_Connection.isPlaying())
			{
				::_Environment.setTimeOfDay(envType);
			}

			return;
		}

		local mapName;

		try
		{
			mapName = data.getStringUTF();
		}
		catch( err )
		{
			mapName = "Maps-Europe";
		}

		this._Environment.setDefault(envType);

		if (this.mSceneObjectManager.onZoneUpdate(zoneId, zoneDefId, zonePageSize, mask))
		{
			this._Environment.update(0);
		}

		::_sceneObjectManager.setCurrentTerrain(terrain);
		this.broadcastMessage("onEnvironmentUpdate", zoneId, zoneDefId, zonePageSize, mapName, envType);
	}

	function _handleAbilityActivationMsg( data )
	{
		local actorId = data.getInteger();
		local abId = data.getShort();
		local event = data.getByte();
		local target_len = data.getInteger();
		local targets = [];
		local willCharges = 0;
		local mightCharges = 0;

		for( local i = 0; i < target_len; i++ )
		{
			local tar = ::_sceneObjectManager.hasCreature(data.getInteger());
			targets.append(tar);
		}

		local secondary_len = data.getInteger();
		local secondaries = [];

		for( local is = 0; is < secondary_len; is++ )
		{
			local tar = ::_sceneObjectManager.hasCreature(data.getInteger());
			secondaries.append(tar);
		}

		local has_ground = data.getByte();
		local ground = this.Vector3(0, 0, 0);

		if (has_ground != 0)
		{
			ground.x = data.getFloat();
			ground.y = data.getFloat();
			ground.z = data.getFloat();
		}

		if (this.AbilityStatus.ACTIVATE == event || this.AbilityStatus.CHANNELING == event)
		{
			willCharges = data.getByte();
			mightCharges = data.getByte();
		}

		local c = ::_sceneObjectManager.hasCreature(actorId);

		if (!c)
		{
			return;
		}

		if (event == this.AbilityStatus.ABILITY_ERROR || event == this.AbilityStatus.INITIAL_FAIL)
		{
			::_quickBarManager.abilityUsed(abId);

			if (abId == 10000 || abId == 10001 || abId == 10002 || abId == 10003)
			{
				local rezScreen = this.Screens.get("RezScreen", true);

				if (rezScreen)
				{
					rezScreen.setResurrectButtonsEnabled(true);
				}
			}

			return;
		}

		if (this.AbilityStatus.WARMUP == event)
		{
			local ab = this._AbilityManager.getAbilityById(abId);

			if (ab)
			{
				c.startCasting(abId);
				local warmupCue = ab.getWarmupCue();
				local warmupEffect;

				if (warmupCue != "" && warmupCue != "NULL")
				{
					warmupEffect = c.cue(warmupCue);
				}

				ab.warmup(c == ::_avatar, c);
				c.setAbilityEffect(warmupEffect);
			}
			else
			{
				c.setAbilityEffect(null);
			}
		}
		else if (this.AbilityStatus.ACTIVATE == event || event == this.AbilityStatus.CHANNELING)
		{
			if (::_avatar.getID() == actorId)
			{
				local ab = this._AbilityManager.getAbilityById(abId);
				local abilityClass = ab.getAbilityClass();

				if (abilityClass && abilityClass != "" && abilityClass != "Passive" && abilityClass != "Use")
				{
					::_AbilityManager.setCategoryCooldownTime("Global", 1000);
					::_quickBarManager.setCategoryUsable("Global", false);
				}
			}

			c.warmupComplete();
			local ab = this._AbilityManager.getAbilityById(abId);

			if (ab)
			{
				local visualCue = ab.getVisualCue();
				local visualEffect;

				if (visualCue != "" && visualCue != "NULL")
				{
					if (mightCharges > 0)
					{
						visualCue = visualCue + mightCharges.tostring();
					}

					if (willCharges > 0)
					{
						visualCue = visualCue + willCharges.tostring();
					}

					if (target_len > 0 || secondary_len > 0)
					{
						if (has_ground)
						{
							visualEffect = c.cue(visualCue, targets, secondaries, ground);
						}
						else
						{
							visualEffect = c.cue(visualCue, targets, secondaries);
						}
					}
					else if (has_ground)
					{
						visualEffect = c.cue(visualCue, ground);
					}
					else
					{
						visualEffect = c.cue(visualCue);
					}
				}

				if (c == ::_avatar)
				{
					ab.activate();
				}

				if (event == this.AbilityStatus.CHANNELING)
				{
					local channelLengthMultiplier = 1;
					local actions = ab.getActions();

					if (actions && actions.find("Duration"))
					{
						if (mightCharges > 0)
						{
							channelLengthMultiplier = mightCharges;
						}
						else if (willCharges > 0)
						{
							channelLengthMultiplier = willCharges;
						}
					}

					ab.channel(c == ::_avatar, c, channelLengthMultiplier);
					c.setAbilityEffect(visualEffect);
				}
				else
				{
					if (visualEffect)
					{
						local secs = ab.getDuration() / 1000.0;
						visualEffect.fireIn(secs, "dispatch", "onAbilityComplete");
					}

					c.setAbilityEffect(null);
				}
			}
			else
			{
				c.setAbilityEffect(null);
			}
		}
		else if (this.AbilityStatus.INTERRUPTED == event)
		{
			local ab = this._AbilityManager.getAbilityById(abId);

			if (ab && c == ::_avatar)
			{
				ab.cancel();
			}

			c.interruptAbility(abId);
		}
		else if (this.AbilityStatus.INITIAL_FAIL == event)
		{
			local ab = this._AbilityManager.getAbilityById(abId);

			if (ab && c == ::_avatar)
			{
				ab.cancel();
				::_AbilityManager.setCategoryCooldownTime("Global", 1000);
				::_quickBarManager.setCategoryUsable("Global", false);
			}

			c.interruptAbility(abId);
		}
		else if (this.AbilityStatus.SETBACK == event)
		{
			local ab = this._AbilityManager.getAbilityById(abId);

			if (ab && c == ::_avatar)
			{
				ab.setback();
			}

			c.setbackAbility();
		}
	}

	function _handleItemUpdateMsg( data )
	{
		local itemId = data.getStringUTF();
		local mask = data.getByte();
		local flags = data.getByte();
		
		if (mask == 0)
		{
			this.log.debug("Item gone: " + itemId);
			::_ItemDataManager._onItemRemoved(itemId);
		}
		else
		{
			local update = {};

			if ((mask & this.ITEM_DEF) != 0)
			{
				update.mItemDefId <- data.getInteger();
			}

			if ((mask & this.ITEM_LOOK_DEF) != 0)
			{
				update.mItemLookDefId <- data.getInteger();
			}

			if ((mask & this.ITEM_CONTAINER) != 0)
			{
				local hexId = ::atoi(itemId, 16);
				update.mContainerSlot <- hexId & this.ItemData.CONTAINER_SLOT;
				update.mContainerItemId <- (hexId & this.ItemData.CONTAINER_ID) >> 16;
			}

			if ((mask & this.ITEM_IV1) != 0)
			{
				update.mIv1 <- data.getShort();
			}

			if ((mask & this.ITEM_IV2) != 0)
			{
				update.mIv2 <- data.getShort();
			}

			if ((flags & this.FLAG_ITEM_BOUND) != 0)
			{
				update.mBound <- data.getByte() == 1 ? true : false;
			}

			if ((flags & this.FLAG_ITEM_TIME_REMAINING) != 0)
			{
				update.timeRemaining <- data.getInteger();
				update.timeRemainingRecievedAt <- ::_gameTime.getGameTimeSeconds();
			}

			if ((mask & this.ITEM_ID_CHANGE) != 0)
			{
				try
				{
					local oldId = data.getStringUTF();
					local lootedItem = ::_ItemDataManager.getItem(oldId);

					if (lootedItem && lootedItem.isValid())
					{
						::_ItemDataManager.updateItemId(itemId, oldId);
					}
				}
				catch( err )
				{
				}
			}

			this.log.debug("Item update: " + itemId + " <- " + this.serialize(update));
			::_ItemDataManager._onItemUpdate(itemId, update);
		}
	}

	function _handleItemDefUpdateMsg( data )
	{
		local itemDefId = data.getInteger();
		local update = this.ItemDefData(itemDefId);
		update.mType = data.getByte();
		update.mDisplayName = data.getStringUTF();
		update.mAppearance = data.getStringUTF();
		update.mIcon = data.getStringUTF();

		if ("" == update.mIcon)
		{
			update.mIcon = "Icon/QuestionMark";
		}

		update.mIvType1 = data.getByte();
		update.mIvMax1 = data.getShort();
		update.mIvType2 = data.getByte();
		update.mIvMax2 = data.getShort();
		update.mSv1 = data.getStringUTF();

		if (this.mProtocolVersionId < 5)
		{
			data.getInteger();
		}

		update.mContainerSlots = data.getShort();
		update.mAutoTitleType = data.getByte();
		update.mLevel = data.getShort();
		update.mBindingType = data.getByte();
		update.mEquipType = data.getByte();
		update.mWeaponType = data.getByte();

		if (update.mWeaponType != 0)
		{
			if (this.mProtocolVersionId == 7)
			{
				update.mWeaponDamageMin = data.getByte();
				update.mWeaponDamageMax = data.getByte();
				data.getByte();
				update.mWeaponExtraDamangeRating = data.getByte();
				update.mWeaponExtraDamageType = data.getByte();
			}
			else
			{
				update.mWeaponDamageMin = data.getInteger();
				update.mWeaponDamageMax = data.getInteger();
				update.mWeaponExtraDamangeRating = data.getByte();
				update.mWeaponExtraDamageType = data.getByte();
			}

			switch(update.mWeaponType)
			{
			case this.WeaponType.SMALL:
				update.mWeaponSpeed = 1.5;
				break;

			case this.WeaponType.ONE_HAND:
				update.mWeaponSpeed = 2.0;
				break;

			case this.WeaponType.TWO_HAND:
			case this.WeaponType.POLE:
				update.mWeaponSpeed = 2.5;
				break;

			case this.WeaponType.BOW:
			case this.WeaponType.WAND:
			case this.WeaponType.THROWN:
				update.mWeaponSpeed = 3.0;
				break;
			}
		}

		update.mEquipEffectId = data.getInteger();
		update.mUseAbilityId = data.getInteger();

		if (update.mUseAbilityId != 0)
		{
			local itemAction = ::_ItemManager.getItemDef(itemDefId);

			if (itemAction)
			{
				itemAction.setAbility(::_AbilityManager.getAbilityById(update.mUseAbilityId));
			}
		}

		update.mActionAbilityId = data.getInteger();
		update.mArmorType = data.getByte();

		if (update.mArmorType != 0)
		{
			if (this.mProtocolVersionId == 7)
			{
				update.mArmorResistMelee = data.getByte();
				update.mArmorResistFire = data.getByte();
				update.mArmorResistFrost = data.getByte();
				update.mArmorResistMystic = data.getByte();
				update.mArmorResistDeath = data.getByte();
			}
			else
			{
				update.mArmorResistMelee = data.getInteger();
				update.mArmorResistFire = data.getInteger();
				update.mArmorResistFrost = data.getInteger();
				update.mArmorResistMystic = data.getInteger();
				update.mArmorResistDeath = data.getInteger();
			}
		}

		update.mBonusStrength = data.getInteger();
		update.mBonusDexterity = data.getInteger();
		update.mBonusConstitution = data.getInteger();
		update.mBonusPsyche = data.getInteger();
		update.mBonusSpirit = data.getInteger();

		if (this.mProtocolVersionId < 32)
		{
			update.mBonusHealth = data.getInteger();
		}

		update.mBonusWill = data.getInteger();

		if (this.mProtocolVersionId >= 4)
		{
			local isCharm = data.getByte();

			if (isCharm)
			{
				update.mMeleeHitMod = data.getFloat();
				update.mMeleeCritMod = data.getFloat();
				update.mMagicHitMod = data.getFloat();
				update.mMagicCritMod = data.getFloat();
				update.mParryMod = data.getFloat();
				update.mBlockMod = data.getFloat();
				update.mRunSpeedMod = data.getFloat();
				update.mRegenHealthMod = data.getFloat();
				update.mAttackSpeedMod = data.getFloat();
				update.mCastSpeedMod = data.getFloat();
				update.mHealingMod = data.getFloat();
			}
		}

		if (this.mProtocolVersionId >= 5)
		{
			update.mValue = data.getInteger();
			update.mValueType = data.getByte();
		}

		this.log.debug(update.mDisplayName);
		this.log.debug(update.mType);

		if (this.ItemUpdateDefMsgCraft == true)
		{
			local resultItemId = data.getInteger();
			local resultItemDef = ::_ItemDataManager.getItemDef(resultItemId);
			update.mResultItem = resultItemId;
			local keyComponentId = data.getInteger();
			local keyComponentDef = ::_ItemDataManager.getItemDef(keyComponentId);
			update.mKeyComponent = keyComponentId;
			local numberOfItems = data.getInteger();

			for( local i = 0; i < numberOfItems; ++i )
			{
				local startCount = 1;
				local craftItemDefId = data.getInteger();

				if (craftItemDefId in update.mCraftComponents)
				{
					local numItemDefs = update.mCraftComponents[craftItemDefId];
					numItemDefs = numItemDefs + 1;
					update.mCraftComponents[craftItemDefId] = numItemDefs;
				}
				else
				{
					local componentDef = ::_ItemDataManager.getItemDef(craftItemDefId);
					update.mCraftComponents[craftItemDefId] <- startCount;
				}
			}
		}

		if (this.mProtocolVersionId >= 9)
		{
			update.mFlavorText = data.getStringUTF();
		}

		if (this.mProtocolVersionId >= 18)
		{
			update.mSpecialItemType = data.getByte();
		}

		if (this.mProtocolVersionId >= 30)
		{
			update.mOwnershipRestriction = data.getByte();
		}

		if (this.mProtocolVersionId >= 31)
		{
			update.mQualityLevel = data.getByte();
			update.mMinUseLevel = data.getShort();
		}

		::_ItemDataManager._onItemDefUpdate(itemDefId, update);
	}

	function _handlePVPStatUpdateMessage( data )
	{
		local updateType = data.getInteger();
		local gameType = data.getByte();
		local pvpScreen;

		switch(gameType)
		{
		case this.PVPGameType.MILLEAGE:
			break;

		case this.PVPGameType.MASSACRE:
			break;

		case this.PVPGameType.TEAMSLAYER:
			pvpScreen = this.Screens.get("TeamSlayerScreen", true);
			break;

		case this.PVPGameType.CTF:
			pvpScreen = this.Screens.get("CTFScreen", true);
			break;

		default:
			return;
			break;
		}

		if (updateType & this.PVP_STATE_UPDATE)
		{
			local gameState = data.getByte();
			if (pvpScreen)
			{
				if(gameState == this.PVPGameState.WAITING_TO_START)
				{
					Screens.show("CTFScreen");
				}
				pvpScreen.updateState(gameState);
			}
		}

		if (updateType & this.PVP_TEAM_UPDATED)
		{
			local playerId = data.getInteger();
			local team = data.getByte();

			if (pvpScreen)
			{
				if (team == this.PVPTeams.NONE)
				{
					pvpScreen.removePlayer(playerId);
				}
				else
				{
					local playerName = data.getStringUTF();
					pvpScreen.addPlayer(team, playerId);
					pvpScreen.initPlayerStats(playerId, playerName);
				}
			}
		}

		if (updateType & this.PVP_STAT_UPDATED)
		{
			local playerId = data.getInteger();
			local statUpdates = data.getByte();

			for( local i = 0; i < statUpdates; i++ )
			{
				pvpScreen.setStat(playerId, i + 1, data.getInteger());
			}
		}

		if (updateType & this.PVP_TIME_UPDATED)
		{
			local timeLeft = data.getFloat();

			if (pvpScreen)
			{
				pvpScreen.updateTime(timeLeft);
			}
		}

		if (updateType & this.PVP_FLAG_EVENT)
		{
			local playerName = data.getStringUTF();
			local flagType = data.getByte();
			local flagEvent = data.getByte();

			if (pvpScreen)
			{
				pvpScreen.handleFlagEvent(playerName, flagType, flagEvent);
			}
		}
	}

	function _handleHeartbeatMessage( data )
	{
		local timeElapsed = data.getInteger();
		::_gameTime.updateGameTime(timeElapsed);
	}

	function _handleLoginQueueMessage( data )
	{
		local queuePosition = data.getInteger();
		local mode = this.QueueType.QUEUE_UNKNOWN;

		if (data.remaining() > 0)
		{
			mode = data.getByte();
		}

		this.log.debug("GOT LOGIN QUEUE");
		this.States.event("queueChanged", [
			queuePosition,
			mode
		]);
	}

	function _handleEncryptMessage( data )
	{
		local which = data.getByte() != 0;

		if (which)
		{
			this.enableEncryption(this.mEncryptionKey);
		}
		else
		{
			this.mUseEncryption = false;
		}
	}

	function _writeStringArray( a )
	{
		if (a.len() > 255)
		{
			throw "Array length too long";
		}

		this.mOutBuf.putByte(a.len());

		foreach( i, s in a )
		{
			this.mOutBuf.putStringUTF("" + s);
		}
	}

	function _readStringArray( data )
	{
		local count = data.getByte();
		local result = [];
		local i;

		for( i = 0; i < count; i++ )
		{
			result.append(data.getStringUTF());
		}

		return result;
	}

	function _sendQuery( query )
	{
		if (!this.isConnected())
		{
			return;
		}

		this.mPendingQueries[query.correlationId] <- query;
		query._sendTime <- this.System.currentTimeMillis();
		query._timeoutEvent <- this._eventScheduler.fireIn(25.0, this, "_handleQueryTimeout", query);
		this._beginSend("query");
		this.mOutBuf.putInteger(query.correlationId);
		this.mOutBuf.putStringUTF(query.query);
		this._writeStringArray(query.args);
		this._send();
	}

	function sendAction( action, ... )
	{
		local args = [];

		if (vargc > 0 && typeof vargv[0] == "array")
		{
			args = vargv[0];
		}
		else
		{
			local i;

			for( i = 0; i < vargc; ++i )
			{
				args.append(vargv[i]);
			}
		}

		this.sendQuery(action, this.DefaultActionHandler(), args);
	}

	function sendQuery( query, handler, ... )
	{
		local args;

		if (vargc > 0)
		{
			if (vargc == 1 && typeof vargv[0] == "array")
			{
				args = vargv[0];
			}
			else
			{
				local i;
				args = [];

				for( local i = 0; i < vargc; i++ )
				{
					args.append(vargv[i]);
				}
			}
		}

		if (args == null)
		{
			args = [];
		}

		if (handler == null)
		{
			this.log.warn("No query handler specified for " + query + ", using a default (not likely correct).");
			handler = this.DefaultQueryHandler();
		}
		else
		{
			this.Assert.isTableOrInstance(handler);
		}

		this.Assert.isArray(args);
		local q = {
			connection = this,
			query = query,
			args = args,
			correlationId = this.mNextQueryId,
			_handler = handler,
			_startTime = this.System.currentTimeMillis(),
			_sendTime = null
		};
		this.mNextQueryId += 1;

		if (this.mPendingQueries.len() > 5)
		{
			this.mQueryQueue.append(q);
		}
		else
		{
			this._sendQuery(q);
		}

		return q;
	}

	function sendGo( x, y, z )
	{
		this.sendAction("go", x / this.gServerScale, y / this.gServerScale, z / this.gServerScale);
	}

	function sendInspectCreature( creatureId )
	{
		this._beginSend("inspectCreature");
		this.mOutBuf.putInteger(creatureId);
		this._send();
	}

	function sendSelectPersona( index )
	{
		this._beginSend("selectPersona");
		this.mOutBuf.putShort(index);
		this._send();
		this.mProtocolChangePending = true;
	}

	function sendAbilityActivate( id, ... )
	{
		local flags = vargc > 0 ? vargv[0] : 0;
		local ground = vargc > 1 ? vargv[1] : null;
		this._beginSend("abilityActivate");
		this.mOutBuf.putShort(id);
		this.mOutBuf.putByte(flags);

		if (ground)
		{
			this.mOutBuf.putByte(1);
			this.mOutBuf.putFloat(ground.x);
			this.mOutBuf.putFloat(ground.y);
			this.mOutBuf.putFloat(ground.z);
		}
		else
		{
			this.mOutBuf.putByte(0);
		}

		this._send();
	}

	function sendAbilityCancel()
	{
		this._beginSend("abilityCancel");
		this._send();
	}

	function sendSelectTarget( creatureId )
	{
		this._beginSend("selectTarget");
		this.mOutBuf.putInteger(creatureId);
		this._send();
	}

	function sendMouseClick(x, y, z)
	{
		this._beginSend("mouseClick");
		this.mOutBuf.putInteger(x);
		this.mOutBuf.putInteger(y);
		this.mOutBuf.putInteger(z);
		this._send();
	}

	function sendInspectCreatureDef( creatureDefId )
	{
		this._beginSend("inspectCreatureDef");
		this.mOutBuf.putInteger(creatureDefId);
		this._send();
	}

	function ping()
	{
		if (this.isConnected())
		{
			this.mPingTask = ::_eventScheduler.fireIn(::PingDelay, this, "ping");

			if (!this.mProtocolChangePending)
			{
				this.sendQuery("util.ping", this);
			}
		}
		else
		{
			this.mPingTask = null;
		}
	}

	function onQueryComplete( qa, results )
	{
		local time = this.System.currentTimeMillis() - qa._startTime;

		if (qa.query == "util.ping")
		{
			this.mPing = this.mPing * 0.80000001 + time * 0.2;
		}
	}

	function onQueryError( qa, error )
	{
		this.DefaultQueryHandler.onQueryError(qa, error);
	}

	function sendInspectItem( itemID )
	{
		this._beginSend("inspectItem");
		this.mOutBuf.putStringUTF(itemID);
		this._send();
	}

	function sendInspectItemDef( itemDefID )
	{
		this._beginSend("inspectItemDef");
		this.mOutBuf.putInteger(itemDefID);
		this._send();
	}

	function sendInWater( which )
	{
		this._beginSend("inWater");
		this.mOutBuf.putByte(which ? 1 : 0);
		this._send();
	}

	socket = null;
	mServerIndex = 0;
	connectionState = 0;
	retryTimer = this.Timer();
	messageType = null;
	messageSize = null;
	messageBuf = null;
	readHeader = true;
	mNotComplete = false;
	mSceneObjectManager = null;
	mErrorRestorePos = null;
	mErrorRestoreStart = null;
	mCurrentProtocol = null;
	mProtocolChangePending = false;
	mOutBuf = null;
	mPendingQueries = null;
	mQueryQueue = null;
	mNextQueryId = 0;
	mPing = 0;
	mPingSim = 0;
	mPingTask = null;
	mCurrentHost = null;
	static CREATURE_UPDATE_TYPE = 1;
	static CREATURE_UPDATE_ZONE = 2;
	static CREATURE_UPDATE_POSITION_INC = 4;
	static CREATURE_UPDATE_VELOCITY = 8;
	static CREATURE_UPDATE_ELEVATION = 16;
	static CREATURE_UPDATE_STAT = 32;
	static CREATURE_UPDATE_MOD = 64;
	static CREATURE_UPDATE_COMBAT = 128;
	static CREATURE_UPDATE_LOGIN_POSITION = 256;
	static PVP_STATE_UPDATE = 1;
	static PVP_TEAM_UPDATED = 2;
	static PVP_STAT_UPDATED = 4;
	static PVP_TIME_UPDATED = 8;
	static PVP_FLAG_EVENT = 16;
	static COMBAT_MELEE = 1;
	static COMBAT_FIRE = 2;
	static COMBAT_FROST = 4;
	static COMBAT_MYSTIC = 8;
	static COMBAT_DEATH = 16;
	static COMBAT_UNBLOCKABLE = 32;
	static SCENERY_UPDATE_ASSET = 1;
	static SCENERY_UPDATE_LINKS = 2;
	static SCENERY_UPDATE_POSITION = 4;
	static SCENERY_UPDATE_ORIENTATION = 8;
	static SCENERY_UPDATE_SCALE = 16;
	static SCENERY_UPDATE_PROPERTIES = 32;
	static SCENERY_UPDATE_FLAGS = 64;
	static PROPERTY_INTEGER = 0;
	static PROPERTY_FLOAT = 1;
	static PROPERTY_STRING = 2;
	static PROPERTY_SCENERY = 3;
	static PROPERTY_NULL = 4;
	static CDEF_HINT_PERSONA = 1;
	static CDEF_HINT_COPPER_SHOPKEEPER = 2;
	static CDEF_HINT_CREDIT_SHOPKEEPER = 4;
	static CDEF_HINT_ESSENCE_VENDOR = 8;
	static CDEF_HINT_QUEST_GIVER = 16;
	static CDEF_HINT_QUEST_ENDER = 32;
	static CDEF_HINT_CRAFTER = 64;
	static CDEF_HINT_CLANREGISTRAR = 128;
	static CDEF_HINT_VAULT = 256;
	static CDEF_HINT_CREDIT_SHOP = 512;
	static ITEM_DEF = 1;
	static ITEM_LOOK_DEF = 2;
	static ITEM_CONTAINER = 4;
	static ITEM_IV1 = 8;
	static ITEM_IV2 = 16;
	static ITEM_ID_CHANGE = 32;
	static ITEM_ALL = 255;
	static FLAG_ITEM_BOUND = 1;
	static FLAG_ITEM_TIME_REMAINING = 2;
	static FLAG_ALL = 255;
}

class this.PersonaListHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		this.log.info("Persona list query returned, with " + results.len() + " personas found.");
		::_stateManager.onEvent("PersonaList", results);
	}

}

class this.PersonaCreateHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		this.log.debug("PersonaCreateHandler Complete Called");
		local index = results[0][0].tointeger();
		::_stateManager.onEvent("NewPersona", index);
	}

	function onQueryError( qa, error )
	{
		this.DefaultQueryHandler.onQueryError(qa, error);
		::_stateManager.onEvent("NewPersonaError", error);
	}

}

class this.PersonaDeleteHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		this.log.debug("PersonaDeleteHandler Complete Called");
		::_stateManager.onEvent("DeletePersonaComplete");
	}

	function onQueryError( qa, error )
	{
		this.DefaultQueryHandler.onQueryError(qa, error);
	}

}

class this.AccountFulfillmentHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
		if (results[0][0] == "0")
		{
			return;
		}

		this._Connection.sendQuery("account.fulfill", this);
	}

}

class this.PersonaCurrencyHandler extends this.DefaultQueryHandler
{
	function onQueryComplete( qa, results )
	{
	}

}

