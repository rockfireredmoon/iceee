project('tawd', ['cpp','c'], version: '0.9.0', default_options : [
	'cpp_std=c++14',
	'prefix=/usr',
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

boost_dep = dependency('boost', modules : ['thread', 'system'])
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

#Compiler arguments 

# Temporary until i can be bother to update to the newer CivetWeb interfaces
#add_global_arguments('-DMG_LEGACY_INTERFACE=1', language: ['cpp','c'])

add_global_arguments('-DELPP_THREAD_SAFE=1', language: ['cpp','c'])
add_global_arguments('-DLOCALCONFIGDIR="' + confdirs + '"', language: ['cpp','c'])
if not openssl_dep.found() or not get_option('ssl')
	add_global_arguments('-DNO_SSL=1', language: ['cpp','c'])
endif
if pthread_dep.found()
	add_global_arguments('-DHAS_PTHREAD=1', language: ['cpp','c'])
endif

#Squirrel
squirrel_lib = library('tawd-squirrel',[
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
	'SOURCE/squirrel/squirrel/sqvm.cpp'],
  	install : true, 
  	version : meson.project_version(),
	include_directories : squirrel_inc)
	
#Squirrel stdlib
sqstdlib_lib = library('tawd-sqstdlib',[
	'SOURCE/squirrel/sqstdlib/sqstdaux.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdblob.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdio.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdmath.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdrex.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdstream.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdstring.cpp',
	'SOURCE/squirrel/sqstdlib/sqstdsystem.cpp'], 
	include_directories : [squirrel_inc, squirrel_lib_inc],
  	install : true,
  	version : meson.project_version(),
	link_with: [squirrel_lib])
	
#tacopie
tacopie_lib = library('tawd-tacopie',[
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
  	version: meson.project_version(),
  	install : true,
	include_directories : [tacopie_inc])
	
#cpp_redis
cppredis_lib = library('tawd-cpp_redis',[
	'SOURCE/LibCPPRedis/sources/misc/logger.cpp',
	'SOURCE/LibCPPRedis/sources/misc/dispatch_queue.cpp',
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
	'SOURCE/LibCPPRedis/sources/core/types.cpp',
	'SOURCE/LibCPPRedis/sources/core/consumer.cpp',
	'SOURCE/LibCPPRedis/sources/core/subscriber.cpp',
	'SOURCE/LibCPPRedis/sources/network/tcp_client.cpp',
	'SOURCE/LibCPPRedis/sources/network/redis_connection.cpp'], 
	dependencies: [thread_dep],
  	install : true,
  	version: meson.project_version(),
	include_directories : [cppredis_inc,tacopie_inc],
	link_with: [tacopie_lib])

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
	'SOURCE/Server/query/AssetCatalogueHandlers.cpp',
	'SOURCE/Server/util/base64.cpp',
	'SOURCE/Server/util/JsonHelpers.cpp',
	'SOURCE/Server/util/SquirrelObjects.cpp',	
	'SOURCE/Server/util/easylogging++.cc',
	'SOURCE/Server/util/Log.cpp'], 
	include_directories : [squirrel_inc, sqstdlib_lib_inc, cppredis_inc, tacopie_inc],
	dependencies: [thread_dep, curl_dep,boost_dep],
  	install : true,
  	version: meson.project_version(),
	link_with: [squirrel_lib,sqstdlib_lib,cppredis_lib,tacopie_lib])

#Main Daemon
server_lib_cppargs = []
if get_option('service') and host_machine.system() == 'windows'
	server_lib_cppargs += '-DWINDOWS_SERVICE=1'
endif
executable('tawd', 'SOURCE/Daemon/main.cpp', 
	include_directories : [server_inc, squirrel_inc, sqstdlib_lib_inc, cppredis_inc, tacopie_inc], 
	dependencies: [thread_dep, curl_dep, openssl_dep],
	link_with: [server_lib], install: true, cpp_args: server_lib_cppargs)
	
executable('eehttpd', 'SOURCE/EEHTTP/main.cpp', 
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
	'SOURCE/Server/util/easylogging++.h',
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
	'SOURCE/Server/BroadCast.h',
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
	'SOURCE/Server/StringUtil.h',
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
	'SOURCE/Server/TradingBoard.h',
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

install_headers(
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/tacopie.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/network/tcp_client.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/network/tcp_server.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/network/self_pipe.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/network/tcp_socket.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/network/io_service.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/utils/typedefs.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/utils/error.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/utils/thread_pool.hpp',
	'SOURCE/LibCPPRedis/tacopie/includes/tacopie/utils/logger.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/core/consumer.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/core/subscriber.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/core/types.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/core/sentinel.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/core/reply.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/core/client.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/helpers/variadic_template.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/helpers/generate_rand.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/network/redis_connection.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/network/tcp_client.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/network/tcp_client_iface.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/impl/types.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/reply_builder.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/simple_string_builder.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/bulk_string_builder.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/error_builder.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/integer_builder.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/builder_iface.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/array_builder.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/builders/builders_factory.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/optional.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/convert.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/dispatch_queue.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/error.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/macro.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/deprecated.hpp',
	'SOURCE/LibCPPRedis/includes/cpp_redis/misc/logger.hpp', subdir: 'tawd/LibCPPRedis')

install_headers(
	'SOURCE/squirrel/include/sqstdsystem.h',
	'SOURCE/squirrel/include/sqstdblob.h',
	'SOURCE/squirrel/include/sqconfig.h',
	'SOURCE/squirrel/include/sqstdstring.h',
	'SOURCE/squirrel/include/sqstdaux.h',
	'SOURCE/squirrel/include/sqstdio.h',
	'SOURCE/squirrel/include/squirrel.h',
	'SOURCE/squirrel/include/sqstdmath.h',
	'SOURCE/squirrel/squirrel/squserdata.h',
	'SOURCE/squirrel/squirrel/sqstring.h',
	'SOURCE/squirrel/squirrel/sqclass.h',
	'SOURCE/squirrel/squirrel/squtils.h',
	'SOURCE/squirrel/squirrel/sqpcheader.h',
	'SOURCE/squirrel/squirrel/sqlexer.h',
	'SOURCE/squirrel/squirrel/sqfuncstate.h',
	'SOURCE/squirrel/squirrel/sqcompiler.h',
	'SOURCE/squirrel/squirrel/sqarray.h',
	'SOURCE/squirrel/squirrel/sqfuncproto.h',
	'SOURCE/squirrel/squirrel/sqobject.h',
	'SOURCE/squirrel/squirrel/sqvm.h',
	'SOURCE/squirrel/squirrel/sqtable.h',
	'SOURCE/squirrel/squirrel/sqopcodes.h',
	'SOURCE/squirrel/squirrel/sqclosure.h',
	'SOURCE/squirrel/squirrel/sqstate.h',
	'SOURCE/squirrel/sqstdlib/sqstdstream.h',
	'SOURCE/squirrel/sqstdlib/sqstdblobimpl.h', subdir: 'tawd/squirrel')
    
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
	'Doc/ACCOUNTS.md', 
	'Doc/BUILD.md',  
	'Doc/DEVELOPERS.md',
	'Doc/GAME_CONFIGURATION.md', 
	'Doc/IN_GAME_COMMANDS.md', 
	'Doc/SERVE_AND_CLUSTER_CONFIGURATION.md',
	'Doc/SERVER.md',
	], install_dir : docpath)
install_data(sources: ['Web/Errors/404.html'], install_dir : webpath + '/Errors')
install_data(sources: ['Web/Pages/Account.html','Web/Pages/RemoteServerTools.html','Web/Pages/ResetPassword.html'], install_dir : webpath + '/Pages')
