if WINSERVICE
bin_PROGRAMS = tawd tawdsvc
else
bin_PROGRAMS = tawd
endif

tawd_CPPFLAGS = -I../Server -I../squirrel/include -I../LibCPPRedis/includes -I../LibCPPRedis/tacopie/includes
tawd_LDADD = ../Server/libtawd.a ../squirrel/squirrel/libsquirrel.a ../squirrel/sqstdlib/libsqstdlib.a ../LibCPPRedis/libcpp_redis.a ../LibCPPRedis/tacopie/libtacopie.a
tawd_LDFLAGS = -v

tawd_SOURCES = main.cpp

if WINSERVICE

tawdsvc_CPPFLAGS = -I../Server -I../squirrel/include -I../LibCPPRedis/includes -I../LibCPPRedis/tacopie/includes -DWINDOWS_SERVICE=1
tawdsvc_LDADD = ../Server/libtawd.a ../squirrel/squirrel/libsquirrel.a ../squirrel/sqstdlib/libsqstdlib.a ../LibCPPRedis/libcpp_redis.a ../LibCPPRedis/tacopie/libtacopie.a
tawdsvc_SOURCES = main.cpp

endif