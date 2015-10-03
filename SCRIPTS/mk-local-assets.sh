#!/bin/bash
#
# Take a copy of the assets directory and expand it so it may be used as a local dev client
#
#

do_car() {
	if [ "$1" = "--exclude" ] ; then
			shift
			exclude=$2
			shift
fi
	bdir="$1" ; shift
	for car in $* ; do
		#echo "XX: $car"
		#read dummy
		fulldir=${bdir}/$(basename $car .car)
		if [ -n "$exclude" ] ; then
			if echo "$car"|egrep "$exclude" >/dev/null ; then
				continue
			fi
		fi 
		echo "Expanding ${car} in ${fulldir}"
		if ! mkdir ${fulldir} ; then
			echo "Failed!"
			exit 1
		fi
		pushd ${fulldir} >/dev/null
		cp "${car}" .	
		wine ${base}/UTILITIES/CARDecode.exe $(basename ${car})
		rm $(basename ${car})
		unzip $(basename ${car} .car).zip
		rm $(basename ${car} .car).zip
		popd
	done
}

do_single_cars() {
	#do_car --exclude ".*Prop-Clutter.*" ${scratch}/Media/Prop ${base}/asset/Release/Current/Media/Prop-*.car
	do_car ${scratch}/Media/Armor ${base}/asset/Release/Current/Media/Armor-*.car
	do_car ${scratch}/Media/Bldg ${base}/asset/Release/Current/Media/Bldg-*.car 
	do_car ${scratch}/Media/ATS ${base}/asset/Release/Current/Media/ATS-*.car 
	do_car ${scratch}/Media/Biped ${base}/asset/Release/Current/Media/Biped-*.car
	do_car ${scratch}/Media/Boss ${base}/asset/Release/Current/Media/Boss-*.car
	do_car ${scratch}/Media/Cav ${base}/asset/Release/Current/Media/Cav*.car
	do_car ${scratch}/Media/CL ${base}/asset/Release/Current/Media/CL-*.car
	do_car ${scratch}/Media/Dng ${base}/asset/Release/Current/Media/Dng-*.car
	do_car ${scratch}/Media/Horde ${base}/asset/Release/Current/Media/Horde-*.car
	do_car ${scratch}/Media/Icons ${base}/asset/Release/Current/Media/Icons-*.car
	do_car ${scratch}/Media/Item ${base}/asset/Release/Current/Media/Item-*.car
	do_car ${scratch}/Media/Maps ${base}/asset/Release/Current/Media/Maps-*.car
	do_car ${scratch}/Media/Music ${base}/asset/Release/Current/Media/Music-*.car
	do_car ${scratch}/Media/Pet ${base}/asset/Release/Current/Media/Pet-*.car
	do_car ${scratch}/Media/Sound ${base}/asset/Release/Current/Media/Sound-*.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Effects.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Lights.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Debug.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Manipulator.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Materials.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Environments.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Epic_Axe_Wing.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Flash_GUI.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/GUI.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Source.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Stitched_Maps.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Temp_Ground.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Catalogs.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Sky-Cartoon.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Refashion_Files.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Preview.car
	do_car ${scratch}/Media ${base}/asset/Release/Current/Media/Preview-Armor.car
}

do_multi() {
	
	#do_multi ${scratch}/Media/Prop ${base}/asset/Release/Current/Media/Prop-*.car
	 
	bdir="$1" ; shift
	
	mkdir "${bdir}.tmp"
	pushd "${bdir}.tmp"
	for car in $* ; do
		echo "Expanding ${car} in ${bdir}.tmp"
		cp "${car}" .	
		wine ${base}/UTILITIES/CARDecode.exe $(basename ${car})
		rm $(basename ${car})
		unzip $(basename ${car} .car).zip
		rm $(basename ${car} .car).zip 
	done
	
	for f in * ; do
		bn=$(basename $f)
		bn=$(echo $bn|awk -F. '{OFS="_";print $1;}')
		#fn=$(echo $bn|awk -F_ '{OFS="_";print $1;}')
		fn=$bn
		
		mkdir -p ${bdir}/${fn}
		mv ${f} ${bdir}/${fn}
	done
	
	popd
	rm -fr "${bdir}.tmp"
	
	
}

	
do_terrain() {
	for i in ${base}/asset/Release/Current/Media/Terrain-Common.car ; do
		bn=$(basename $i .car)
		if [ "$bn" = "Terrain-Common" ]; then
			fn="Terrain-Common"
		else
			fn=$(echo $bn|awk -F_ '{OFS="_";NF--;print $0;}')
		fi
		mkdir -p ${scratch}/Media/Terrain/${fn}
		cp ${i} ${scratch}/Media/Terrain/${fn}
		pushd ${scratch}/Media/Terrain/${fn} >/dev/null
		wine ${base}/UTILITIES/CARDecode.exe ${bn}.car
		unzip ${bn}.zip
		rm ${bn}.zip ${bn}.car
		popd
	done
}

cd "$(dirname $0)"/..
base=$(pwd)
scratch=${base}/locasset/EE
rm -fr ${scratch}

mkdir -p ${scratch}/Media \
	${scratch}/Release \
	${scratch}/Media/Armor \
	${scratch}/Media/ATS \
	${scratch}/Media/Biped \
	${scratch}/Media/Bldg \
	${scratch}/Media/Prop \
	${scratch}/Media/Boss \
	${scratch}/Media/Cav \
	${scratch}/Media/CL \
	${scratch}/Media/Dng \
	${scratch}/Media/Horde \
	${scratch}/Media/Icons \
	${scratch}/Media/Item \
	${scratch}/Media/Maps \
	${scratch}/Media/Music \
	${scratch}/Media/Pet \
	${scratch}/Media/Sound \
	${scratch}/Media/Terrain \

#do_car ${scratch}/Release ${base}/asset/Release/Current/EarthEternal.car  
do_single_cars
#do_multi ${scratch}/Media/Prop ${base}/asset/Release/Current/Media/Prop-Clutter*.car 
do_multi ${scratch}/Media/Prop ${base}/asset/Release/Current/Media/Prop-*.car

# Special handling for terrain
do_terrain

# Some annoying exceptions
#mv ${scratch}/Media/Prop/Prop-Clutter1 ${scratch}/Media/Prop/Prop-Clutter  

# The bootstrap car file needs to be in a certain place
mkdir -p ${scratch}/Release/Current
cp ${base}/asset/Release/Current/EarthEternal.car ${scratch}/Release/Current
