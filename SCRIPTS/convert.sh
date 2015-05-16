#!/bin/bash
#
# Convert to and from Ogre XML and binary formats, also fixing the files
# serialize

base="$(dirname $0)"/..
base=$(realpath $base)
echo $base
for i in $* ; do
	case "$1" in
		*.skeleton.xml) :
			java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Converter XML_TO_BINARY $1 ;;  
		*.mesh.xml) :
			java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Converter XML_TO_BINARY $1  
			java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Fixer UNFIX $(basename $1 .xml) 
			;;
		*.mesh) :
			java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Fixer FIX $*
			java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Converter BINARY_TO_XML $1 ;;
		*.skeleton) :
			java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Converter BINARY_TO_XML $1 ;;
	esac
done
