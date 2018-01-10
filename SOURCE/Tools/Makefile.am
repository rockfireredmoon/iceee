bin_PROGRAMS = text2redis eeaccount

text2redis_CPPFLAGS = -I../Server -I../squirrel/include -I../LibCPPRedis/includes -I../LibCPPRedis/tacopie/includes
text2redis_LDADD = ../Server/libtawd.a ../squirrel/squirrel/libsquirrel.a ../squirrel/sqstdlib/libsqstdlib.a ../LibCPPRedis/libcpp_redis.a ../LibCPPRedis/tacopie/libtacopie.a
text2redis_LDFLAGS = -v

text2redis_SOURCES = TextToRedis.cpp

eeaccount_CPPFLAGS = -I../Server -I../squirrel/include -I../LibCPPRedis/includes -I../LibCPPRedis/tacopie/includes
eeaccount_LDADD = ../Server/libtawd.a ../squirrel/squirrel/libsquirrel.a ../squirrel/sqstdlib/libsqstdlib.a ../LibCPPRedis/libcpp_redis.a ../LibCPPRedis/tacopie/libtacopie.a
eeaccount_LDFLAGS = -v

eeaccount_SOURCES = EEAccount.cpp