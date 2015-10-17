#!/bin/bash

#
# Configure wine. 
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

