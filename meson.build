project('tawd', ['cpp','c'], version: '0.9.0', default_options : [
	'cpp_std=c++20',
	'prefix=/usr',
	'sysconfdir=/etc',
	'localstatedir=/var'])

#Setup
server_inc = include_directories('SOURCE/Server')

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
	webpath = get_option('prefix') + '/share/tawd/Web'
else
	webpath = get_option('webdir')
endif

# CAR path (default path for client CAR files)
if get_option('cfg_WebPath') != ''
	webpath = get_option('cfg_WebPath')
endif

# CAR path (default path for client CAR files)
if get_option('cfg_HTTPCARFolder') == ''
	carpath = get_option('prefix') + '/share/tawd'
else
	carpath = get_option('cfg_HTTPCARFolder')
endif

# Documentation
if get_option('docdir') == ''
	docpath = 'share/doc/tawd'
else
	docpath = get_option('docdir')
endif

#Dependencies
boost_dep = dependency('boost', modules : ['thread', 'system'])
thread_dep = dependency('threads')
curl_dep = dependency('libcurl')
openssl_dep = dependency('openssl', required: false)
pthread_dep = meson.get_compiler('c').find_library('pthread', required: false)

#Compiler arguments 

# Temporary until i can be bother to update to the newer CivetWeb interfaces
#add_global_arguments('-DMG_LEGACY_INTERFACE=1', language: ['cpp','c'])

add_project_arguments('-DELPP_THREAD_SAFE=1', language: ['cpp','c'])

add_project_arguments('-DLOCALCONFIGDIR="' + confdirs + '"', language: ['cpp','c'])
if not openssl_dep.found() or not get_option('ssl')
	add_project_arguments('-DNO_SSL=1', language: ['cpp','c'])
endif
if pthread_dep.found()
	add_project_arguments('-DHAS_PTHREAD=1', language: ['cpp','c'])
endif

# TODO squirrel 3.2 (sqRat?) is making this necessary. Can it be fixed?
#if meson.get_compiler('c').get_id() == 'gcc'
#	add_project_arguments('-fpermissive', language: ['cpp','c'])
#endif

#Other Dependencies

squirrel_dep = dependency('squirrel', fallback: ['squirrel','libsquirrel_dep'])
sqstdlib_dep = dependency('sqstdlib', fallback: ['squirrel','libsqstdlib_dep'])
tacopie_dep = dependency('tacopie', fallback: ['LibCPPRedis','libtacopie_dep'])
cppredis_dep = dependency('cppredis', fallback: ['LibCPPRedis','libcppredis_dep'])

#Server library
server_lib = library('tawd-server',[
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
	'SOURCE/Server/AssetCatalogue.cpp',
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
	'SOURCE/Server/GameConfig.cpp',
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
	'SOURCE/Server/Random.cpp',
	'SOURCE/Server/Report.cpp',
	'SOURCE/Server/Router.cpp',
	'SOURCE/Server/Scenery2.cpp',
	'SOURCE/Server/Scheduler.cpp',
	'SOURCE/Server/ScriptCore.cpp',
	'SOURCE/Server/SimulatorBase.cpp',
	'SOURCE/Server/Simulator.cpp',
	'SOURCE/Server/SocketClass3.cpp',
	'SOURCE/Server/Stats.cpp',
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
	'SOURCE/Server/query/AssetCatalogueHandlers.cpp',
	'SOURCE/Server/util/base64.cpp',
	'SOURCE/Server/util/JsonHelpers.cpp',
	'SOURCE/Server/util/SquirrelObjects.cpp',	
	'SOURCE/Server/util/Log.cpp'], 
	dependencies: [thread_dep, curl_dep,boost_dep, squirrel_dep, sqstdlib_dep, tacopie_dep, cppredis_dep],
  	install : true,
  	version: meson.project_version())

#Main Daemon
server_lib_cppargs = []
if get_option('service') and host_machine.system() == 'windows'
	server_lib_cppargs += '-DWINDOWS_SERVICE=1'
endif
executable('tawd', 'SOURCE/Daemon/main.cpp', 
	include_directories : [server_inc], 
	dependencies: [thread_dep, curl_dep, squirrel_dep, sqstdlib_dep, openssl_dep, tacopie_dep, cppredis_dep],
	link_with: [server_lib], install: true, cpp_args: server_lib_cppargs)
	
executable('eehttpd', 'SOURCE/EEHTTP/main.cpp', 
	include_directories : [server_inc], 
	dependencies: [thread_dep, curl_dep, squirrel_dep, sqstdlib_dep, openssl_dep, tacopie_dep, cppredis_dep],
	link_with: [server_lib], install: true, cpp_args: server_lib_cppargs)
	
#Pre-TAWD text file user data to TAWD Redis database
executable('text2redis', 'SOURCE/Tools/TextToRedis.cpp', 
	include_directories : [server_inc], 
	dependencies: [thread_dep, curl_dep, squirrel_dep, sqstdlib_dep, openssl_dep, tacopie_dep, cppredis_dep],
	link_with: [server_lib], install: true)
	
#Account management command
executable('eeaccount', 'SOURCE/Tools/EEAccount.cpp', 
	include_directories : [server_inc], 
	dependencies: [thread_dep, curl_dep, squirrel_dep, sqstdlib_dep, openssl_dep, tacopie_dep, cppredis_dep],
	link_with: [server_lib], install: true)

# Create actual configuration files from samples, changing the default
# locations where appropraite 
srchrepl_prog = find_program('Meson/srchrepl.py') 
copy_prog = find_program('Meson/filecopy.py')
serverconfig_target = custom_target('ServerConfig',
    input : 'Local/ServerConfig.txt.sample',
    output : 'ServerConfig.txt',
    command : [srchrepl_prog, '@INPUT@', '@OUTPUT@',
    		'HTTPBaseFolder=Web','HTTPBaseFolder=' + webpath,
    		'HTTPCARFolder=../sparkplayer-eartheternal/bin/assets','HTTPCARFolder=' + carpath,
    		'StaticDataPath=../iceee-data/Static','StaticDataPath=' + staticdatapath,
    		'TmpDataPath=Tmp','TmpDataPath=' + tmpdir,
    		'VariableDataPath=../iceee-data/Variable','VariableDataPath=' + variabledatapath,
    		'LogPath=Logs','LogPath=' + logpath + '/Server'],
  	install : true,
  	install_dir : confdir)
httpconfig_target = custom_target('HTTPConfig',
    input : 'Local/HTTPConfig.txt.sample',
    output : 'HTTPConfig.txt',
    command : [srchrepl_prog, '@INPUT@', '@OUTPUT@',
    		'HTTPBaseFolder=Web','HTTPBaseFolder=' + webpath,
    		'HTTPCARFolder=../sparkplayer-eartheternal/bin/assets','HTTPCARFolder=' + carpath,
    		'LogPath=Logs','LogPath=' + logpath + '/HTTP'],
  	install : true,
  	install_dir : confdir)
httplogconfig_target = custom_target('HTTPLogConfig',
    input : 'Local/HTTPLogConfig.txt.sample',
    output : 'HTTPLogConfig.txt',
    command : [srchrepl_prog, '@INPUT@', '@OUTPUT@',
    		'FILENAME             =  "Logs/',
    		'FILENAME             =  "' + logpath  + '/HTTP'+ '/'],
  	install : true,
  	install_dir : confdir)
logconfig_target = custom_target('LogConfig',
    input : 'Local/LogConfig.txt.sample',
    output : 'LogConfig.txt',
    command : [srchrepl_prog, '@INPUT@', '@OUTPUT@',
    		'FILENAME             =  "Logs/',
    		'FILENAME             =  "' + logpath  + '/Server'+ '/'],
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
  	
install_headers(
	'SOURCE/Server/Chat.h',
	'SOURCE/Server/EssenceShop.h',
	'SOURCE/Server/Globals.h',
	'SOURCE/Server/EliteMob.h',
	'SOURCE/Server/IntArray.h',
	'SOURCE/Server/QuestScript.h',
	'SOURCE/Server/Books.h',
	'SOURCE/Server/ZoneDef.h',
	'SOURCE/Server/http/HTTPService.h',
	'SOURCE/Server/http/HTTPClient.h',
	'SOURCE/Server/http/GameInfo.h',
	'SOURCE/Server/http/OAuth2.h',
	'SOURCE/Server/http/TAWApi.h',
	'SOURCE/Server/http/CAR.h',
	'SOURCE/Server/http/CivetServer.h',
	'SOURCE/Server/http/WebControlPanel.h',
	'SOURCE/Server/http/LegacyAccounts.h',
	'SOURCE/Server/http/TAWClient.h',
	'SOURCE/Server/http/civetweb.h',
	'SOURCE/Server/http/SiteClient.h',
	'SOURCE/Server/http/HTTP.h',
	'SOURCE/Server/FileReader.h',
	'SOURCE/Server/VirtualItem.h',
	'SOURCE/Server/CompilerEnvironment.h',
	'SOURCE/Server/ByteBuffer.h',
	'SOURCE/Server/Cluster.h',
	'SOURCE/Server/DebugProfiler.h',
	'SOURCE/Server/ZoneObject.h',
	'SOURCE/Server/Account.h',
	'SOURCE/Server/Trade.h',
	'SOURCE/Server/AbilityTime.h',
	'SOURCE/Server/Daily.h',
	'SOURCE/Server/Gamble.h',
	'SOURCE/Server/GameConfig.h',
	'SOURCE/Server/Audit.h',
	'SOURCE/Server/RotatingList.h',
	'SOURCE/Server/util/JsonHelpers.h',
	'SOURCE/Server/util/base64.h',
	'SOURCE/Server/util/SquirrelObjects.h',
	'SOURCE/Server/util/Log.h',
	'SOURCE/Server/Quest.h',
	'SOURCE/Server/Packet.h',
	'SOURCE/Server/json/json.h',
	'SOURCE/Server/json/json-forwards.h',
	'SOURCE/Server/AIScript.h',
	'SOURCE/Server/SocketClass3.h',
	'SOURCE/Server/Ability2.h',
	'SOURCE/Server/Combat.h',
	'SOURCE/Server/AuctionHouse.h',
	'SOURCE/Server/Formula.h',
	'SOURCE/Server/Forms.h',
	'SOURCE/Server/Random.h',
	'SOURCE/Server/ItemSet.h',
	'SOURCE/Server/ScriptCore.h',
	'SOURCE/Server/IGForum.h',
	'SOURCE/Server/InstanceScale.h',
	'SOURCE/Server/md5.hh',
	'SOURCE/Server/AssetCatalogue.h',
	'SOURCE/Server/Scheduler.h',
	'SOURCE/Server/DebugTracer.h',
	'SOURCE/Server/NPC.h',
	'SOURCE/Server/FriendStatus.h',
	'SOURCE/Server/Config.h',
	'SOURCE/Server/Util.h',
	'SOURCE/Server/Debug.h',
	'SOURCE/Server/Fun.h',
	'SOURCE/Server/ConfigStr.h',
	'SOURCE/Server/FileReader3.h',
	'SOURCE/Server/Components.h',
	'SOURCE/Server/Leaderboard.h',
	'SOURCE/Server/Creature.h',
	'SOURCE/Server/Instance.h',
	'SOURCE/Server/Guilds.h',
	'SOURCE/Server/Item.h',
	'SOURCE/Server/Clan.h',
	'SOURCE/Server/sqrat.h',
	'SOURCE/Server/Mail.h',
	'SOURCE/Server/CommonTypes.h',
	'SOURCE/Server/sqrat/sqratObject.h',
	'SOURCE/Server/sqrat/sqratGlobalMethods.h',
	'SOURCE/Server/sqrat/sqratVM.h',
	'SOURCE/Server/sqrat/sqratClass.h',
	'SOURCE/Server/sqrat/sqratOverloadMethods.h',
	'SOURCE/Server/sqrat/sqratUtil.h',
	'SOURCE/Server/sqrat/sqratConst.h',
	'SOURCE/Server/sqrat/sqratFunction.h',
	'SOURCE/Server/sqrat/sqratTypes.h',
	'SOURCE/Server/sqrat/sqratScript.h',
	'SOURCE/Server/sqrat/sqratArray.h',
	'SOURCE/Server/sqrat/sqratAllocator.h',
	'SOURCE/Server/sqrat/sqratClassType.h',
	'SOURCE/Server/sqrat/sqratTable.h',
	'SOURCE/Server/sqrat/sqratMemberMethods.h',
	'SOURCE/Server/Interact.h',
	'SOURCE/Server/Router.h',
	'SOURCE/Server/Info.h',
	'SOURCE/Server/SimulatorBase.h',
	'SOURCE/Server/ConfigString.h',
	'SOURCE/Server/PartyManager.h',
	'SOURCE/Server/message/LobbyMessage.h',
	'SOURCE/Server/message/GameMessage.h',
	'SOURCE/Server/message/MessageHandler.h',
	'SOURCE/Server/message/SharedMessage.h',
	'SOURCE/Server/Simulator.h',
	'SOURCE/Server/GM.h',
	'SOURCE/Server/CreatureSpawner2.h',
	'SOURCE/Server/Callback.h',
	'SOURCE/Server/RemoteAction.h',
	'SOURCE/Server/PVP.h',
	'SOURCE/Server/ChatChannel.h',
	'SOURCE/Server/Crafting.h',
	'SOURCE/Server/Achievements.h',
	'SOURCE/Server/ActiveCharacter.h',
	'SOURCE/Server/URL.h',
	'SOURCE/Server/DropTable.h',
	'SOURCE/Server/MainWindow.h',
	'SOURCE/Server/query/AbilityHandlers.h',
	'SOURCE/Server/query/TradeHandlers.h',
	'SOURCE/Server/query/MarkerHandlers.h',
	'SOURCE/Server/query/SceneryHandlers.h',
	'SOURCE/Server/query/SupportHandlers.h',
	'SOURCE/Server/query/AssetCatalogueHandlers.h',
	'SOURCE/Server/query/FormHandlers.h',
	'SOURCE/Server/query/BookHandlers.h',
	'SOURCE/Server/query/GMHandlers.h',
	'SOURCE/Server/query/CommandHandlers.h',
	'SOURCE/Server/query/ScriptHandlers.h',
	'SOURCE/Server/query/Lobby.h',
	'SOURCE/Server/query/PreferenceHandlers.h',
	'SOURCE/Server/query/LootHandlers.h',
	'SOURCE/Server/query/CreditShopHandlers.h',
	'SOURCE/Server/query/ItemHandlers.h',
	'SOURCE/Server/query/Query.h',
	'SOURCE/Server/query/VaultHandlers.h',
	'SOURCE/Server/query/ClanHandlers.h',
	'SOURCE/Server/query/PetHandlers.h',
	'SOURCE/Server/query/IGFHandlers.h',
	'SOURCE/Server/query/AuctionHouseHandlers.h',
	'SOURCE/Server/query/FriendHandlers.h',
	'SOURCE/Server/query/SpawnHandlers.h',
	'SOURCE/Server/query/PlayerHandlers.h',
	'SOURCE/Server/query/ZoneHandlers.h',
	'SOURCE/Server/query/StatusHandlers.h',
	'SOURCE/Server/query/QuestHandlers.h',
	'SOURCE/Server/query/SidekickHandlers.h',
	'SOURCE/Server/query/CreatureHandlers.h',
	'SOURCE/Server/Arena.h',
	'SOURCE/Server/sqmodule.h',
	'SOURCE/Server/Entities.h',
	'SOURCE/Server/sqratimport.h',
	'SOURCE/Server/DecodePackets.h',
	'SOURCE/Server/DirectoryAccess.h',
	'SOURCE/Server/auth/ClusterAuthentication.h',
	'SOURCE/Server/auth/DevAuthentication.h',
	'SOURCE/Server/auth/ServiceAuthentication.h',
	'SOURCE/Server/auth/Auth.h',
	'SOURCE/Server/CreditShop.h',
	'SOURCE/Server/Report.h',
	'SOURCE/Server/QuestAction.h',
	'SOURCE/Server/Scenery2.h',
	'SOURCE/Server/Stats.h',
	'SOURCE/Server/InstanceScript.h',
	'SOURCE/Server/Character.h',
	'SOURCE/Server/AIScript2.h',
	'SOURCE/Server/Preferences.h',
	'SOURCE/Server/PlayerStats.h',
	'SOURCE/Server/Inventory.h', subdir: 'tawd/Server')

if get_option('service') and host_machine.system() == 'linux'
	if get_option('sysvinit')
		install_data(sources: 'Etc/Linux/tawd', install_dir : get_option('sysconfdir') + '/etc/init.d')
		install_data(sources: 'Etc/Linux/eehttpd', install_dir : get_option('sysconfdir') + '/etc/init.d')
	else
		install_data(sources: 'Etc/Linux/tawd.service', install_dir : get_option('systemdservicedir'))
		install_data(sources: 'Etc/Linux/eehttpd.service', install_dir : get_option('systemdservicedir'))
	endif
endif

install_data(sources: ['Local/ServerConfig.txt.sample','Local/LogConfig.txt.sample','Local/Cluster.txt.sample','Local/URL.txt.sample', 'Local/HTTPConfig.txt.sample'], install_dir : docpath)
install_data(sources: [
	'README.md',
	'LICENSE', 
    'Doc/API.md', 
	'Doc/ACCOUNTS.md', 
	'Doc/BUILD.md',  
	'Doc/DEVELOPERS.md',
	'Doc/GAME_CONFIGURATION.md', 
	'Doc/IN_GAME_COMMANDS.md', 
	'Doc/SERVER_AND_CLUSTER_CONFIGURATION.md',
	'Doc/SERVER.md',
	], install_dir : docpath)
install_data(sources: ['Web/Errors/404.html'], install_dir : webpath + '/Errors')
install_data(sources: [
    'Web/Pages/index.html',
    'Web/Pages/Account.html',
    'Web/Pages/RemoteServerTools.html',
    'Web/Pages/ResetPassword.html'
    ], install_dir : webpath + '/Pages')
install_data(sources: [
    'Web/Pages/css/bootstrap.min.css',
    'Web/Pages/css/darkly.min.css'
    ], install_dir : webpath + '/Pages/css')
install_data(sources: [
    'Web/Pages/js/bootstrap.bundle.min.js',
    'Web/Pages/js/util.js'
    ], install_dir : webpath + '/Pages/css')
install_data(sources: [
    'Web/Pages/images/server-banner.png'
    ], install_dir : webpath + '/Pages/images')
