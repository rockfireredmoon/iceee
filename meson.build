project('tawd', ['cpp','c'], default_options : [
	'cpp_std=c++11',
	'prefix=/usr',
	'tmpdir=/tmp',
	'sysconfdir=/etc',
	'localstatedir=/var'])

#Setup
server_inc = include_directories('SOURCE/Server')
squirrel_inc = include_directories('SOURCE/squirrel/include')
squirrel_lib_inc = include_directories('SOURCE/squirrel/squirrel')
sqstdlib_lib_inc = include_directories('SOURCE/squirrel/sqstdlib')
cppredis_inc = include_directories('SOURCE/LibCPPRedis/includes')
tacopie_inc = include_directories('SOURCE/LibCPPRedis/tacopie/includes')

#Dependencies
thread_dep = dependency('threads')
curl_dep = dependency('libcurl')
openssl_dep = dependency('openssl', required: false)
pthread_dep = meson.get_compiler('c').find_library('pthread', required: false)

#Other Installation
datadir = get_option('serverdatadir')
confdir = get_option('localconfigdir')
if get_option('configdirs') == ''
	confdirs = confdir
else
	confdirs = get_option('configdirs')
endif

# Logs
if get_option('cfg_LogPath') == ''
	logpath = get_option('localstatedir') + '/log/tawd'
else
	logpath = get_option('cfg_LogPath')
endif
	
# Variable Data (Scenery, Instance Data, Creature Edits)
if get_option('cfg_VariableDataPath') == ''
	variabledatapath = get_option('localstatedir') + '/lib/tawd/Variable'
else
	variabledatapath = get_option('cfg_VariableDataPath')
endif
	
# Static Data (Creature Definitions, Items, Loot Tables and lots more)
if get_option('cfg_StaticDataPath') == ''
	staticdatapath = get_option('prefix') + '/share/tawd/Static'
else
	staticdatapath = get_option('cfg_StaticDataPath')
endif

# Temporary data (compiled scripts)
tmpdir = get_option('cfg_TmpDir')

# Wed directory (default path for web resources)
if get_option('webdir') == ''
	webpath = get_option('webdir') + '/share/tawd/Web'
else
	webpath = get_option('webdir')
endif

# Base path (default path for web resources)
if get_option('cfg_BasePath') == ''
	basepath = get_option('basedir') + '/share/tawd/Web'
else
	basepath = get_option('cfg_BasePath')
endif

# CAR path (default path for client CAR files)
if get_option('cfg_HTTPCARFolder') == ''
	carpath = get_option('prefix') + '/share/tawd/Release'
else
	carpath = get_option('cfg_HTTPCARFolder')
endif

# Documentation
if get_option('docdir') == ''
	docpath = 'share/doc/tawd'
else
	docpath = get_option('docdir')
endif

#Compiler arguments 
add_global_arguments('-DELPP_THREAD_SAFE=1', language: ['cpp','c'])
add_global_arguments('-DELPP_NO_GLOBAL_LOCK=1', language: ['cpp','c'])
add_global_arguments('-DLOCALCONFIGDIR="' + confdirs + '"', language: ['cpp','c'])
if not openssl_dep.found() or not get_option('ssl')
	add_global_arguments('-DNO_SSL=1', language: ['cpp','c'])
endif
if pthread_dep.found()
	add_global_arguments('-DHAS_PTHREAD=1', language: ['cpp','c'])
endif

#Squirrel
squirrel_lib = library('squirrel',[
	'SOURCE/squirrel/squirrel/sqapi.cpp',
	'SOURCE/squirrel/squirrel/sqbaselib.cpp',
	'SOURCE/squirrel/squirrel/sqclass.cpp',
	'SOURCE/squirrel/squirrel/sqcompiler.cpp',
	'SOURCE/squirrel/squirrel/sqdebug.cpp',
	'SOURCE/squirrel/squirrel/sqfuncstate.cpp',
	'SOURCE/squirrel/squirrel/sqlexer.cpp',
	'SOURCE/squirrel/squirrel/sqmem.cpp',
	'SOURCE/squirrel/squirrel/sqobject.cpp',
	'SOURCE/squirrel/squirrel/sqstate.cpp',
	'SOURCE/squirrel/squirrel/sqtable.cpp',
	'SOURCE/squirrel/squirrel/sqvm.cpp'], include_directories : squirrel_inc)
	
#Squirrel stdlib
sqstdlib_lib = library('sqstdlib',[
	'SOURCE/squirrel/sqstdlib/sqstdaux.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdblob.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdio.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdmath.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdrex.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdstream.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdstring.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdsystem.cpp'], 
	include_directories : [squirrel_inc, squirrel_lib_inc],
	link_with: [squirrel_lib])
	
#tacopie
tacopie_lib = library('tacopie',[
	'SOURCE/LibCPPRedis/tacopie/sources/utils/error.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/utils/thread_pool.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/utils/logger.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/tcp_server.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/tcp_client.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/io_service.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/common/tcp_socket.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/windows/windows_self_pipe.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/windows/windows_tcp_socket.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/unix/unix_tcp_socket.cpp',
	'SOURCE/LibCPPRedis/tacopie/sources/network/unix/unix_self_pipe.cpp'], 
	dependencies: [thread_dep],
	include_directories : [tacopie_inc])
	
#cpp_redis
cppredis_lib = library('cpp_redis',[
	'SOURCE/LibCPPRedis/sources/misc/logger.cpp',
	'SOURCE/LibCPPRedis/sources/builders/error_builder.cpp',
	'SOURCE/LibCPPRedis/sources/builders/builders_factory.cpp',
	'SOURCE/LibCPPRedis/sources/builders/integer_builder.cpp',
	'SOURCE/LibCPPRedis/sources/builders/bulk_string_builder.cpp',
	'SOURCE/LibCPPRedis/sources/builders/array_builder.cpp',
	'SOURCE/LibCPPRedis/sources/builders/simple_string_builder.cpp',
	'SOURCE/LibCPPRedis/sources/builders/reply_builder.cpp',
	'SOURCE/LibCPPRedis/sources/core/sentinel.cpp',
	'SOURCE/LibCPPRedis/sources/core/client.cpp',
	'SOURCE/LibCPPRedis/sources/core/reply.cpp',
	'SOURCE/LibCPPRedis/sources/core/subscriber.cpp',
	'SOURCE/LibCPPRedis/sources/network/tcp_client.cpp',
	'SOURCE/LibCPPRedis/sources/network/redis_connection.cpp'], 
	dependencies: [thread_dep],
	include_directories : [cppredis_inc,tacopie_inc],
	link_with: [tacopie_lib])

#Server library
server_lib = library('server',[
	'SOURCE/Server/md5.cc',
	'SOURCE/Server/Ability2.cpp',
	'SOURCE/Server/AbilityTime.cpp',
	'SOURCE/Server/Account.cpp',
	'SOURCE/Server/Achievements.cpp',
	'SOURCE/Server/ActiveCharacter.cpp',
	'SOURCE/Server/AIScript.cpp',
	'SOURCE/Server/AIScript2.cpp',
	'SOURCE/Server/Arena.cpp',
	'SOURCE/Server/Audit.cpp',
	'SOURCE/Server/AuctionHouse.cpp',
	'SOURCE/Server/BroadCast.cpp',
	'SOURCE/Server/Books.cpp',
	'SOURCE/Server/ByteBuffer.cpp',
	'SOURCE/Server/Character.cpp',
	'SOURCE/Server/ChatChannel.cpp',
	'SOURCE/Server/Chat.cpp',
	'SOURCE/Server/Clan.cpp',
	'SOURCE/Server/Cluster.cpp',
	'SOURCE/Server/Combat.cpp',
	'SOURCE/Server/Components.cpp',
	'SOURCE/Server/Config.cpp',
	'SOURCE/Server/ConfigString.cpp',
	'SOURCE/Server/Crafting.cpp',
	'SOURCE/Server/Creature.cpp',
	'SOURCE/Server/CreditShop.cpp',
	'SOURCE/Server/CreatureSpawner2.cpp',
	'SOURCE/Server/Daily.cpp',
	'SOURCE/Server/DebugProfiler.cpp',
	'SOURCE/Server/DebugTracer.cpp',
	'SOURCE/Server/DirectoryAccess.cpp',
	'SOURCE/Server/DropTable.cpp',
	'SOURCE/Server/Entities.cpp',
	'SOURCE/Server/EliteMob.cpp',
	'SOURCE/Server/EssenceShop.cpp',
	'SOURCE/Server/FileReader3.cpp',
	'SOURCE/Server/FileReader.cpp',
	'SOURCE/Server/FriendStatus.cpp',
	'SOURCE/Server/Forms.cpp',
	'SOURCE/Server/Fun.cpp',
	'SOURCE/Server/Gamble.cpp',
	'SOURCE/Server/Globals.cpp',
	'SOURCE/Server/Guilds.cpp',
	'SOURCE/Server/GM.cpp',
	'SOURCE/Server/Info.cpp',
	'SOURCE/Server/IGForum.cpp',
	'SOURCE/Server/Instance.cpp',
	'SOURCE/Server/InstanceScale.cpp',
	'SOURCE/Server/InstanceScript.cpp',
	'SOURCE/Server/Interact.cpp',
	'SOURCE/Server/Inventory.cpp',
	'SOURCE/Server/Item.cpp',
	'SOURCE/Server/ItemSet.cpp',
	'SOURCE/Server/jsoncpp.cpp',
	'SOURCE/Server/Leaderboard.cpp',
	'SOURCE/Server/Mail.cpp',
	'SOURCE/Server/NPC.cpp',
	'SOURCE/Server/Packet.cpp',
	'SOURCE/Server/PartyManager.cpp',
	'SOURCE/Server/PlayerStats.cpp',
	'SOURCE/Server/Preferences.cpp',
	'SOURCE/Server/PVP.cpp',
	'SOURCE/Server/QuestAction.cpp',
	'SOURCE/Server/Quest.cpp',
	'SOURCE/Server/QuestScript.cpp',
	'SOURCE/Server/RemoteAction.cpp',
	'SOURCE/Server/Report.cpp',
	'SOURCE/Server/Router.cpp',
	'SOURCE/Server/Scenery2.cpp',
	'SOURCE/Server/Scheduler.cpp',
	'SOURCE/Server/ScriptCore.cpp',
	'SOURCE/Server/SimulatorBase.cpp',
	'SOURCE/Server/Simulator.cpp',
	'SOURCE/Server/SocketClass3.cpp',
	'SOURCE/Server/Stats.cpp',
	'SOURCE/Server/StringUtil.cpp',
	'SOURCE/Server/Trade.cpp',
	'SOURCE/Server/URL.cpp',
	'SOURCE/Server/Util.cpp',
	'SOURCE/Server/VirtualItem.cpp',
	'SOURCE/Server/ZoneDef.cpp',
	'SOURCE/Server/ZoneObject.cpp',
	'SOURCE/Server/http/HTTP.cpp',
	'SOURCE/Server/http/CAR.cpp',
	'SOURCE/Server/http/OAuth2.cpp',
	'SOURCE/Server/http/TAWApi.cpp',
	'SOURCE/Server/http/SiteClient.cpp',
	'SOURCE/Server/http/GameInfo.cpp',
	'SOURCE/Server/http/TAWClient.cpp',
	'SOURCE/Server/http/LegacyAccounts.cpp',
	'SOURCE/Server/http/WebControlPanel.cpp',
	'SOURCE/Server/http/CivetServer.cpp',
	'SOURCE/Server/http/civetweb.c',
	'SOURCE/Server/http/HTTPService.cpp',
	'SOURCE/Server/auth/Auth.cpp',
	'SOURCE/Server/auth/DevAuthentication.cpp',
	'SOURCE/Server/auth/ClusterAuthentication.cpp',
	'SOURCE/Server/auth/ServiceAuthentication.cpp',
	'SOURCE/Server/message/MessageHandler.cpp',
	'SOURCE/Server/message/LobbyMessage.cpp',
	'SOURCE/Server/message/SharedMessage.cpp',
	'SOURCE/Server/message/GameMessage.cpp',
	'SOURCE/Server/query/ClanHandlers.cpp',
	'SOURCE/Server/query/Query.cpp',
	'SOURCE/Server/query/Lobby.cpp',
	'SOURCE/Server/query/PreferenceHandlers.cpp',
	'SOURCE/Server/query/GMHandlers.cpp',
	'SOURCE/Server/query/CreditShopHandlers.cpp',
	'SOURCE/Server/query/VaultHandlers.cpp',
	'SOURCE/Server/query/FormHandlers.cpp',
	'SOURCE/Server/query/AuctionHouseHandlers.cpp',
	'SOURCE/Server/query/ScriptHandlers.cpp',
	'SOURCE/Server/query/MarkerHandlers.cpp',
	'SOURCE/Server/query/SidekickHandlers.cpp',
	'SOURCE/Server/query/QuestHandlers.cpp',
	'SOURCE/Server/query/IGFHandlers.cpp',
	'SOURCE/Server/query/TradeHandlers.cpp',
	'SOURCE/Server/query/SceneryHandlers.cpp',
	'SOURCE/Server/query/SupportHandlers.cpp',
	'SOURCE/Server/query/BookHandlers.cpp',
	'SOURCE/Server/query/ItemHandlers.cpp',
	'SOURCE/Server/query/CommandHandlers.cpp',
	'SOURCE/Server/query/LootHandlers.cpp',
	'SOURCE/Server/query/FriendHandlers.cpp',
	'SOURCE/Server/query/AbilityHandlers.cpp',
	'SOURCE/Server/query/CreatureHandlers.cpp',
	'SOURCE/Server/query/PetHandlers.cpp',
	'SOURCE/Server/query/SpawnHandlers.cpp',
	'SOURCE/Server/query/ZoneHandlers.cpp',
	'SOURCE/Server/query/StatusHandlers.cpp',
	'SOURCE/Server/query/PlayerHandlers.cpp',
	'SOURCE/Server/util/base64.cpp',
	'SOURCE/Server/util/JsonHelpers.cpp',
	'SOURCE/Server/util/SquirrelObjects.cpp',
	'SOURCE/Server/util/Log.cpp'], 
	include_directories : [squirrel_inc, sqstdlib_lib_inc, cppredis_inc, tacopie_inc],
	dependencies: [thread_dep, curl_dep],
	link_with: [squirrel_lib,sqstdlib_lib,cppredis_lib,tacopie_lib])

#Main Daemon
server_lib_cppargs = []
if get_option('service') and build_machine.system() == 'windows'
	server_lib_cppargs += '-DWINDOWS_SERVICE=1'
endif
executable('tawd', 'SOURCE/Daemon/main.cpp', 
	include_directories : [server_inc, squirrel_inc, sqstdlib_lib_inc, cppredis_inc, tacopie_inc], 
	dependencies: [thread_dep, curl_dep, openssl_dep],
	link_with: [server_lib], install: true, cpp_args: server_lib_cppargs)
	
#Pre-TAWD text file user data to TAWD Redis database
executable('text2redis', 'SOURCE/Tools/TextToRedis.cpp', 
	include_directories : [server_inc, squirrel_inc, sqstdlib_lib_inc, cppredis_inc, tacopie_inc], 
	dependencies: [thread_dep, curl_dep, openssl_dep],
	link_with: [server_lib], install: true)
	
#Account management command
executable('eeaccount', 'SOURCE/Tools/EEAccount.cpp', 
	include_directories : [server_inc, squirrel_inc, sqstdlib_lib_inc, cppredis_inc, tacopie_inc], 
	dependencies: [thread_dep, curl_dep, openssl_dep],
	link_with: [server_lib], install: true)
	

# Create actual configuration files from samples, changing the default
# locations where appropraite 
srchrepl_prog = find_program('Meson/srchrepl.py') 
copy_prog = find_program('Meson/filecopy.py')
serverconfig_target = custom_target('ServerConfig',
    input : 'Local/ServerConfig.txt.sample',
    output : 'ServerConfig.txt',
    command : [srchrepl_prog, '@INPUT@', '@OUTPUT@',
    		'HTTPBaseFolder=Web','HTTPBaseFolder=' + basepath,
    		'HTTPCARFolder=../sparkplayer-eartheternal/bin/assets','HTTPCARFolder=' + carpath,
    		'StaticDataPath=../iceee-data/Static','StaticDataPath=' + staticdatapath,
    		'TmpDataPath=Tmp','TmpDataPath=' + tmpdir,
    		'VariableDataPath=../iceee-data/Variable','VariableDataPath=' + variabledatapath,
    		'LogPath=Logs','LogPath=' + logpath],
  	install : true,
  	install_dir : confdir)
logconfig_target = custom_target('LogConfig',
    input : 'Local/LogConfig.txt.sample',
    output : 'LogConfig.txt',
    command : [srchrepl_prog, '@INPUT@', '@OUTPUT@',
    		'FILENAME             =  "Logs/',
    		'FILENAME             =  "' + logpath + '/'],
  	install : true,
  	install_dir : confdir)
cluster_target = custom_target('Cluster', 
	input : 'Local/Cluster.txt.sample', 
	output : 'Cluster.txt', 
	command : [copy_prog, '@INPUT@', '@OUTPUT@'],
  	install : true,
  	install_dir : confdir)
url_target = custom_target('URL', 
	input : 'Local/URL.txt.sample', 
	output : 'URL.txt', 
	command : [copy_prog, '@INPUT@', '@OUTPUT@'],
  	install : true,
  	install_dir : confdir)
    
if get_option('service') and build_machine.system() == 'linux'
	systemd_dep = dependency('systemd')
	if systemd_dep.found()
		install_data(sources: 'Etc/Linux/tawd.service', install_dir : get_option('systemdservicedir'))
	else
		install_data(sources: 'Etc/Linux/tawd', install_dir : get_option('sysconfdir') + '/etc/init.d')
	endif
endif

install_data(sources: ['Local/ServerConfig.txt.sample','Local/LogConfig.txt.sample','Local/Cluster.txt.sample','Local/URL.txt.sample'], install_dir : docpath)
install_data(sources: ['README.md','LICENSE'], install_dir : docpath)
install_data(sources: ['Web/Errors/404.html'], install_dir : webpath + '/Errors')
install_data(sources: ['Web/Pages/Account.html','Web/Pages/RemoteServerTools.html','Web/Pages/ResetPassword.html'], install_dir : webpath + '/Pages')
