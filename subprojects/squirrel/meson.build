project('squirrel', ['cpp','c'], version: '3.2.0')

#Setup
squirrel_inc = include_directories('include')
squirrel_lib_inc = include_directories('squirrel')
sqstdlib_lib_inc = include_directories('sqstdlib')

add_project_arguments([
	], language: ['cpp','c'])

if meson.get_compiler('c').get_id() == 'gcc'
	add_project_arguments([
			'-fno-strict-aliasing',
			'-fno-exceptions',
			'-fno-rtti',
			'-pedantic',
			'-Wno-deprecated-copy',
			'-Wno-implicit-fallthrough',
			'-Wno-class-memaccess',
			'-Wno-cast-qual'
		], language: ['cpp','c'])
endif

#Squirrel
squirrel_lib = library(get_option('librarynameprefix') + 'squirrel',[
	'squirrel/sqapi.cpp',
	'squirrel/sqbaselib.cpp',
	'squirrel/sqclass.cpp',
	'squirrel/sqcompiler.cpp',
	'squirrel/sqdebug.cpp',
	'squirrel/sqfuncstate.cpp',
	'squirrel/sqlexer.cpp',
	'squirrel/sqmem.cpp',
	'squirrel/sqobject.cpp',
	'squirrel/sqstate.cpp',
	'squirrel/sqtable.cpp',
	'squirrel/sqvm.cpp'],
  	install : true, 
  	version : meson.project_version(),
	include_directories : squirrel_inc)
	
#Squirrel stdlib
sqstdlib_lib = library(get_option('librarynameprefix') + 'sqstdlib',[
	'sqstdlib/sqstdaux.cpp',
	'sqstdlib/sqstdblob.cpp',
	'sqstdlib/sqstdio.cpp',
	'sqstdlib/sqstdmath.cpp',
	'sqstdlib/sqstdrex.cpp',
	'sqstdlib/sqstdstream.cpp',
	'sqstdlib/sqstdstring.cpp',
	'sqstdlib/sqstdsystem.cpp'], 
	include_directories : [squirrel_inc, squirrel_lib_inc],
  	install : true,
  	version : meson.project_version(),
	link_with: [squirrel_lib])

install_headers(
	'include/sqstdsystem.h',
	'include/sqstdblob.h',
	'include/sqconfig.h',
	'include/sqstdstring.h',
	'include/sqstdaux.h',
	'include/sqstdio.h',
	'include/squirrel.h',
	'include/sqstdmath.h',
	'squirrel/squserdata.h',
	'squirrel/sqstring.h',
	'squirrel/sqclass.h',
	'squirrel/squtils.h',
	'squirrel/sqpcheader.h',
	'squirrel/sqlexer.h',
	'squirrel/sqfuncstate.h',
	'squirrel/sqcompiler.h',
	'squirrel/sqarray.h',
	'squirrel/sqfuncproto.h',
	'squirrel/sqobject.h',
	'squirrel/sqvm.h',
	'squirrel/sqtable.h',
	'squirrel/sqopcodes.h',
	'squirrel/sqclosure.h',
	'squirrel/sqstate.h',
	'sqstdlib/sqstdstream.h',
	'sqstdlib/sqstdblobimpl.h', subdir: 'tawd/squirrel')
    
    
libsquirrel_dep = declare_dependency(include_directories : [squirrel_inc, squirrel_lib_inc], link_with: squirrel_lib)
libsqstdlib_dep = declare_dependency(include_directories : [squirrel_inc, squirrel_lib_inc, sqstdlib_lib_inc], link_with: [squirrel_lib,sqstdlib_lib], dependencies: libsquirrel_dep)