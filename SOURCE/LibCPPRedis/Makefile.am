SUBDIRS = tacopie

noinst_LIBRARIES = libcpp_redis.a
libcpp_redis_a_CPPFLAGS = -Iincludes
libcpp_redis_a_SOURCES = sources/misc/logger.cpp \
	sources/builders/error_builder.cpp \
	sources/builders/builders_factory.cpp \
	sources/builders/integer_builder.cpp \
	sources/builders/bulk_string_builder.cpp \
	sources/builders/array_builder.cpp \
	sources/builders/simple_string_builder.cpp \
	sources/builders/reply_builder.cpp \
	sources/core/sentinel.cpp \
	sources/core/client.cpp \
	sources/core/reply.cpp \
	sources/core/subscriber.cpp \
	sources/network/tcp_client.cpp \
	sources/network/redis_connection.cpp
