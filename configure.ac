dnl Process this file with autoconf to produce a configure script.
AC_INIT(tawd, 1.0, emerald.icemoon@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign])

AC_PROG_CXX
LT_INIT
AC_SUBST([LIBTOOL_DEPS])
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL


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

AC_ARG_ENABLE(winservice,
AS_HELP_STRING([--enable-winservice],
               [enable windows service, default: $mingw ]),
[case "${enableval}" in
             yes) winservice=true ;;
             no)  winservice=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-winservice]) ;;
esac],
[winservice=$mingw])
AM_CONDITIONAL([WINSERVICE], [test x$winservice = xtrue])

# Stops Civet trying to dynamically load OpenSSL
CPPFLAGS="$CPPFLAGS -DNO_SSL_DL=1"
if test "x${mingw}" = xtrue; then
	CPPFLAGS="$CPPFLAGS -DWINDOWS_PLATFORM=1"
fi

AC_LIB_LINKFLAGS([crypto])
AC_LIB_LINKFLAGS([curl],[ssl crypto])
AC_LIB_LINKFLAGS([ssl],[crypto])

AX_PTHREAD()
CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS -DHAS_PTHREAD=1"
LIBS="$PTHREAD_LIBS $LIBS $PTHREAD_CFLAGS"

AC_OUTPUT

