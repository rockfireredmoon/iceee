#!/bin/bash

#
# Configure wine. It is important we use a 32 bit prefix. Wine will
# not be used on Win32/MinGW
#

export WINEPREFIX=$HOME/.tawtools
#export WINEARCH=win32

#
# Use this function to run windows binary tools, as it will check
# if MinGW is in use
#

run_win_tool() {
	case "${MSYSTEM}" in
		"MINGW32") 	$@ 
					return $? ;;
				*) 	wine $@ 
					return $? ;;
	esac
}