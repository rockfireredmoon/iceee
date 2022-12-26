project('squirrel', ['cpp','c'], version: '0.0.1')

#Setup
cppredis_inc = include_directories('includes')
tacopie_inc = include_directories('tacopie/includes')

#Dependencies
thread_dep = dependency('threads')

add_project_arguments([
	], language: ['cpp','c'])
	
#tacopie
tacopie_lib = library(get_option('librarynameprefix') + 'tacopie',[
	'tacopie/sources/utils/error.cpp',
	'tacopie/sources/utils/thread_pool.cpp',
	'tacopie/sources/utils/logger.cpp',
	'tacopie/sources/network/tcp_server.cpp',
	'tacopie/sources/network/tcp_client.cpp',
	'tacopie/sources/network/io_service.cpp',
	'tacopie/sources/network/common/tcp_socket.cpp',
	'tacopie/sources/network/windows/windows_self_pipe.cpp',
	'tacopie/sources/network/windows/windows_tcp_socket.cpp',
	'tacopie/sources/network/unix/unix_tcp_socket.cpp',
	'tacopie/sources/network/unix/unix_self_pipe.cpp'], 
	dependencies: [thread_dep],
  	version: meson.project_version(),
  	install : true,
	include_directories : [tacopie_inc])
	
#cpp_redis
cppredis_lib = library(get_option('librarynameprefix') + 'cpp_redis',[
	'sources/misc/logger.cpp',
	'sources/misc/dispatch_queue.cpp',
	'sources/builders/error_builder.cpp',
	'sources/builders/builders_factory.cpp',
	'sources/builders/integer_builder.cpp',
	'sources/builders/bulk_string_builder.cpp',
	'sources/builders/array_builder.cpp',
	'sources/builders/simple_string_builder.cpp',
	'sources/builders/reply_builder.cpp',
	'sources/core/sentinel.cpp',
	'sources/core/client.cpp',
	'sources/core/reply.cpp',
	'sources/core/types.cpp',
	'sources/core/consumer.cpp',
	'sources/core/subscriber.cpp',
	'sources/network/tcp_client.cpp',
	'sources/network/redis_connection.cpp'], 
	dependencies: [thread_dep],
  	install : true,
  	version: meson.project_version(),
	include_directories : [cppredis_inc,tacopie_inc],
	link_with: [tacopie_lib])



install_headers(
	'tacopie/includes/tacopie/tacopie.hpp',
	'tacopie/includes/tacopie/network/tcp_client.hpp',
	'tacopie/includes/tacopie/network/tcp_server.hpp',
	'tacopie/includes/tacopie/network/self_pipe.hpp',
	'tacopie/includes/tacopie/network/tcp_socket.hpp',
	'tacopie/includes/tacopie/network/io_service.hpp',
	'tacopie/includes/tacopie/utils/typedefs.hpp',
	'tacopie/includes/tacopie/utils/error.hpp',
	'tacopie/includes/tacopie/utils/thread_pool.hpp',
	'tacopie/includes/tacopie/utils/logger.hpp',
	'includes/cpp_redis/core/consumer.hpp',
	'includes/cpp_redis/core/subscriber.hpp',
	'includes/cpp_redis/core/types.hpp',
	'includes/cpp_redis/core/sentinel.hpp',
	'includes/cpp_redis/core/reply.hpp',
	'includes/cpp_redis/core/client.hpp',
	'includes/cpp_redis/helpers/variadic_template.hpp',
	'includes/cpp_redis/helpers/generate_rand.hpp',
	'includes/cpp_redis/network/redis_connection.hpp',
	'includes/cpp_redis/network/tcp_client.hpp',
	'includes/cpp_redis/network/tcp_client_iface.hpp',
	'includes/cpp_redis/impl/types.hpp',
	'includes/cpp_redis/builders/reply_builder.hpp',
	'includes/cpp_redis/builders/simple_string_builder.hpp',
	'includes/cpp_redis/builders/bulk_string_builder.hpp',
	'includes/cpp_redis/builders/error_builder.hpp',
	'includes/cpp_redis/builders/integer_builder.hpp',
	'includes/cpp_redis/builders/builder_iface.hpp',
	'includes/cpp_redis/builders/array_builder.hpp',
	'includes/cpp_redis/builders/builders_factory.hpp',
	'includes/cpp_redis/misc/optional.hpp',
	'includes/cpp_redis/misc/convert.hpp',
	'includes/cpp_redis/misc/dispatch_queue.hpp',
	'includes/cpp_redis/misc/error.hpp',
	'includes/cpp_redis/misc/macro.hpp',
	'includes/cpp_redis/misc/deprecated.hpp',
	'includes/cpp_redis/misc/logger.hpp', subdir: 'tawd/LibCPPRedis')
    
    
libtacopie_dep = declare_dependency(include_directories : [tacopie_inc])
libcppredis_dep = declare_dependency(include_directories : [cppredis_inc, tacopie_inc], link_with: [cppredis_lib,tacopie_lib], dependencies: libtacopie_dep)