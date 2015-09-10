dnl Process this file with autoconf to produce a configure script.
AC_INIT(planetforeverd, 1.0, emerald.icemoon@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects])
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
AC_OUTPUT
