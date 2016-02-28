dnl
dnl Initialisation
dnl

AC_INIT(tawd, 0.1, emerald.icemoon@gmail.com)
	
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign])

dnl
dnl Include external macros
dnl

m4_include([ax_check_openssl.m4])
m4_include([ax_pthread.m4])
m4_include([libcurl.m4])
m4_include([ax_cxx_compile_stdcxx_11.m4])


dnl
dnl Environment detectection
dnl 
AC_CANONICAL_HOST
case $host_os in
  *mingw*) mingw=true ;;
  		*) mingw=false;;
esac

dnl
dnl Arguments
dnl


AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_ARG_ENABLE(ssl,
AS_HELP_STRING([--enable-ssl],
               [enable ssl, default: no]),
[case "${enableval}" in
             yes) ssl=true ;;
             no)  ssl=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-ssl]) ;;
esac],
[ssl=false])

AC_ARG_ENABLE(winservice,
AS_HELP_STRING([--enable-winservice],
               [enable windows service, default: $mingw ]),
[case "${enableval}" in
             yes) winservice=true ;;
             no)  winservice=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-winservice]) ;;
esac],
[winservice=$mingw])

AC_ARG_ENABLE(selfcontained,
AS_HELP_STRING([--enable-selfcontained],
               [link statically to produce self contained executable: $mingw ]),
[case "${enableval}" in
             yes) selfcontained=true ;;
             no)  selfcontained=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-selfcontained]) ;;
esac],
[selfcontained=$mingw])
	

dnl
dnl Secondary initialisation
dnl

if test "x${selfcontained}" = xtrue; then
	AM_DISABLE_SHARED
	AM_ENABLE_STATIC
fi


AC_PROG_CXX
LT_INIT
AC_SUBST([LIBTOOL_DEPS])
AC_PROG_LIBTOOL

dnl
dnl Token replacment
dnl
AM_CONDITIONAL([MINGW], [test x$mingw = xtrue])
AM_CONDITIONAL([WINSERVICE], [test x$winservice = xtrue])

dnl AX_CXX_COMPILE_STDCXX_11([noext],[optional])

dnl
dnl Compiler and link setup
dnl

# Stops Civet trying to dynamically load OpenSSL
#CPPFLAGS="$CPPFLAGS -DNO_SSL_DL=1"
if test "x${mingw}" = xtrue; then
	CPPFLAGS="$CPPFLAGS -DWINDOWS_PLATFORM=1"
fi

# Static linking
static=""
if test "x${selfcontained}" = xtrue; then
	CPPFLAGS="$CPPFLAGS -DPTW32_STATIC_LIB"
	if test "x${mingw}" = xtrue; then
		LDFLAGS="-static-libgcc -static-libstdc++ $LDFLAGS"
	fi
fi


LDFLAGS="-rdynamic $LDFLAGS"

if test "x${ssl}" = xtrue; then
	AX_CHECK_OPENSSL()
	CPPFLAGS="$CPPFLAGS $OPENSSL_INCLUDES"
	LIBS="$LIBS $OPENSSL_LIBS"
	LDFLAGS="$LD_FLAGS $OPENSSL_LDFLAGS"
	AC_CHECK_LIB([dl], [dlsym], [dl=true], [dl=false])
	if test "x${dl}" = xtrue; then
		LIBS="$LIBS -ldl"
	fi
else
	CPPFLAGS="$CPPFLAGS -DNO_SSL=1"
fi

LIBCURL_CHECK_CONFIG()
CPPFLAGS="$CPPFLAGS $LIBCURL_CPPFLAGS"
LIBS="$LIBS $LIBCURL"

AX_PTHREAD([has_pthread=true],[has_pthread=false])
if test "x${has_pthread}" != xtrue; then
	AC_MSG_ERROR([could not find a pthreads library])
fi
CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS -DHAS_PTHREAD=1"
LIBS="$LIBS $PTHREAD_LIBS $PTHREAD_CFLAGS"
#LDFLAGS="$LD_FLAGS $PTHREAD_CFLAGS $PTHREAD_LIBS "

if test "x${mingw}" != xtrue; then
	AC_CHECK_LIB([ws2_32],[ws2=true],[ws=false])
fi

dnl
dnl Tail
dnl

AC_CONFIG_FILES([
	Makefile
	asset/Makefile
	AIScript/Makefile
	Data/Makefile
	Instance/Makefile
	ItemMod/Makefile
	ItemMod/Tables/Makefile
	Loot/Makefile
	Packages/Makefile
	QuestScripts/Makefile
	SpawnPackages/Makefile
	Etc/Makefile
	Etc/Linux/Makefile
	Scenery/Makefile
	SOURCE/Makefile
	SOURCE/squirrel/Makefile
	SOURCE/squirrel/squirrel/Makefile
	SOURCE/squirrel/sqstdlib/Makefile
	SOURCE/Server/Makefile
])
AC_OUTPUT

dnl
dnl Report
dnl

AS_ECHO("Available features :-")
AS_ECHO("")
AS_ECHO("General")
AS_ECHO("-------")
AS_ECHO_N("Static linked build         - ")
AS_IF([test "x$selfcontained" = xtrue ],[AS_ECHO("Yes")],[AS_ECHO("No")])
AS_ECHO_N("Windows (MinGW)             - ")
AS_IF([test "x$mingw" = xtrue ],[AS_ECHO("Yes")],[AS_ECHO("No")])
AS_ECHO_N("Windows service             - ")
AS_IF([test "x$winservice" = xtrue ],[AS_ECHO("Yes")],[AS_ECHO("No")])
AS_ECHO_N("pthread                     - ")
AS_IF([test "x$has_pthread" = xtrue ],[AS_ECHO("Yes")],[AS_ECHO("No")])
AS_ECHO("")
AS_ECHO("HTTP Server")
AS_ECHO("-----------------")
AS_ECHO_N("SSL                         - ")
AS_IF([test "x$ssl" = xtrue ],[AS_ECHO("Yes")],[AS_ECHO("No")])
