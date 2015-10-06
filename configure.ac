dnl Process this file with autoconf to produce a configure script.
AC_INIT(tawd, 1.0, emerald.icemoon@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign])

AC_PROG_CXX
LT_INIT
AC_CONFIG_FILES([
	Makefile
	SOURCE/Makefile
	SOURCE/squirrel/Makefile
	SOURCE/squirrel/squirrel/Makefile
	SOURCE/squirrel/sqstdlib/Makefile
	SOURCE/Server/Makefile
])
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
AC_CANONICAL_HOST
case $host_os in
  *mingw*) mingw=true ;;
  		*) mingw=false;;
	esac
AM_CONDITIONAL([MINGW], [test x$mingw = xtrue])

PKG_CHECK_MODULES([libcurl],[libcurl])
AC_SUBST([libcurl_CFLAGS])
AC_SUBST([libcurl_LIBS])

PKG_CHECK_MODULES([libssl], [libssl]) 
AC_CHECK_LIB(ssl, SSL_library_init, [libssl_LIBS="-lssl" libssl_CFLAGS="" ], AC_MSG_ERROR([libssl library is required]))
AC_SUBST(libssl_CFLAGS)
AC_SUBST(libssl_LIBS)
AC_CHECK_LIB(crypto, DES_xcbc_encrypt, [crypto=true], [crypto=false])
if test "x${crypto}" != xtrue; then
   AC_MSG_ERROR([required library crypto missing])
fi

AC_CHECK_LIB(pthread, pthread_create,[pthread=true], [pthread=false])
if test "x${pthread}" != xtrue; then
   AC_MSG_ERROR([required library pthread missing])
fi
AM_CONDITIONAL([HAS_PTHREAD], [test x$pthread = xtrue])

AC_OUTPUT

