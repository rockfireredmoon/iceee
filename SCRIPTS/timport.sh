#!/bin/bash
# 
# Imports a flat folder of terrain assets (coverage, heightmap, cfg etc) and
# places them into the correct structure is the asset patches folders  
#
source "$(dirname $0)"/shelllib.sh

function get_loc() {
	case "$1" in
		*_x*) echo $1|awk -F_ '{ print $NF }' ;;
			*) echo "";
	esac
}

function get_terrain_name() {
	tn="${1}"
	case "$1" in
		"Terrain-"*) tn=$(echo ${tn}|awk '{ print substr($0, 9) }') ;;
	esac
	last=$(echo $tn|awk -F_ '{ print $NF }')
	case "${last}" in
		x*) echo $tn|awk '{print substr($0, 0, index($0,"_x") - 1) }';;
		*) echo $tn ;;
	esac
}

cd "$(dirname $0)"/..
base=$(pwd)

folder="$1"
if [ -z "$folder" ] ; then
	echo  "$0: <name> <folder>" >&2
	exit 1
fi

for i in "${folder}"/* ; do
	echo "Processing $i"
	base=$(basename "${i}")
	name=$(echo "${base}"|awk -F. '{ print $1 }')
	pos=$(echo "${name}"|awk -F_ '{ print $NF }')
	terrain=$(echo "${name}"|awk -F_ '{ print $1 }')
	mkdir -p SOURCE/AssetPatches/Terrain-${terrain}_${pos}
	echo "Moving $pos / $terrain"
	mv "${i}" SOURCE/AssetPatches/Terrain-${terrain}_${pos}
done
