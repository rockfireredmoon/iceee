#!/bin/bash
# 
# For copying and renaming texture asset archives. Takes an 
# exist terrain template and gives it a new name, renaming all
# images and config files and contents. 
#

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

name="$1"
if [ -z "$name" ] ; then
	echo  "$0: <name>" >&2
	exit 1
fi

pushd temp >/dev/null
for i in *.car ; do
	bn=$(basename $i .car)
	loc=$(get_loc $bn)
	oldname=$(get_terrain_name $bn)
	
	tdir=Terrain-${name}
	if [ -n "${loc}" ]; then
		tdir=${tdir}_${loc}
	fi
	
	wine ../UTILITIES/CARDecode.exe $i

	rm -fr "${tdir}"
	mkdir -p "${tdir}"
	pushd ${tdir} > /dev/null
	unzip ../${bn}.zip
	
	for j in * ; do
		
		newfname=$(echo $j|sed 's/'${oldname}'/'${name}'/g')
		
		if ! mv ${j} ${newfname} ; then
			echo "$0: failed moving ${j} to ${newfname}" >&2
			exit 1
		fi
		
		echo "$oldname"
		case "${newfname}" in
			*.cfg) 	sed -i "s/^Texture\\.Base=${oldname}_Base/Texture.Base=${name}_Base/g" ${newfname} 
					sed -i "s/^Texture\\.Coverage=${oldname}_Coverage/Texture.Coverage=${name}_Coverage/g" ${newfname} 
					sed -i "s/^PerPageConfig=${oldname}_x/PerPageConfig=${name}_x/g" ${newfname} 
					sed -i "s/^Heightmap\\.image=${oldname}_Height/Heightmap.image=${name}_Height/g" ${newfname} ;;
			*.cnut) wine ${base}/UTILITIES/nutcracker.exe  ${newfname} > $(basename ${newfname} .cnut).nut 
					rm -f ${newfname} ;;
		esac
		
	done
	
	
	 
	popd > /dev/null
	
	rm -f ${bn}.zip 
done
echo
echo "Snippet for TerrainPages.nut"
echo "---------------------------------------------"
ls -d Terrain-${name}_*|awk '{ print "\t[\"" $0 "\"] = true," '}

echo
echo "Snippet for MediaEx.nut"
echo "---------------------------------------------"

popd >/dev/null
