#!/bin/bash

source "$(dirname $0)"/shelllib.sh

cd "$(dirname $0)"/..
base=$(pwd)

SCRATCH="${base}/scratch"
BASE_ASSETS="${base}/../../Common/eeassets/original-client/assets/Release/Current/Media"
TARGET_ASSETS="${base}/asset"

# Test for Base directory. I wasted two days because I forgot about this
# directory when moving stuff around :\
if [ ! -d "${BASE_ASSETS}" -o -z "$(ls -A ${BASE_ASSETS})" ] ; then
	echo "$0: Base assets directory '${BASE_ASSETS}' does not exist or does not contain any files" >&2
	exit 1
fi


#
# Functions
#
function compile_sq() {
	script="${1}"
	case "${script}" in
		*.nut) : ;;
			*) 	echo "Can only compile .nut files to Squirrel. ${script}" >&2
				tput bel
				return 1 ;;
	esac
	output="$(dirname "$1")/$(basename "$1" .nut).cnut"
	rm -f "${output}"
	if ! run_win_tool ${base}/UTILITIES/sq.exe -o "${output}" -c "${script}" ; then
		echo "$0: Failed to compile ${script}" >&2
		tput bel
		return 1
	fi
	if [ ! -f "${output}" ] ; then
		echo "$0: No output ${output} for ${script}. Failed to compile?" >&2
		tput bel
		return 1
	fi	
	return 0
}

#
# Main Body
#


rm -fr "${SCRATCH}"
mkdir -p "${SCRATCH}"
mkdir -p "${SCRATCH}/archives"
mkdir -p "${SCRATCH}/content"

echo "Extracting catalog"
cp SOURCE/CAR/Catalogs.car "${SCRATCH}/archives"
if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/Catalogs.car" ; then
	echo "$0: Failed to decode ${SCRATCH}/archives/Catalogs.car" >&2
	exit 1
fi

echo "Decompressing catalog"
pushd "${SCRATCH}/content"
unzip -q "${SCRATCH}/archives/Catalogs.zip"
echo "Generating abilities"
cp ${base}/Data/AbilityTable.txt .
run_win_tool ${base}/UTILITIES/EEUtilAbilityTable.exe
ls
popd
echo "Copying other catalog patches"
pushd ${base}/SOURCE/CatalogPatches
find . -name '*.nut' | cpio -updm "${SCRATCH}/content"
popd
pushd "${SCRATCH}/content"
echo "Compiling catalogs"
find . -name '*.nut'|while read script ; do
	output=$(basename $script .nut).cnut
	if [ "${script}" = "./AbilityTable.nut" ] ; then
		echo "Using Abilities.cnut instead of AbilityTable.cnut"
		cp ${script} ${base}/scratch 
		output="./Abilities.cnut"
	fi
	echo "Compiling ${script} to ${output}"
	if ! compile_sq "${script}" ; then
		echo "$0: Failed to compile ${script}" >&2
		exit 1
	else rm ${script}
	fi
done
zip -dg -q -r "${SCRATCH}/archives/Catalogs.zip" *
popd

echo "Archiving catalog"
if ! run_win_tool ${base}/UTILITIES/CARDecode.exe "${SCRATCH}/archives/Catalogs.zip" ; then
	echo "$0: Failed to decode ${SCRATCH}/archives/Catalogs.zip" >&2
	exit 1
fi

rm -fr "${SCRATCH}/content"
mkdir -p "${SCRATCH}/content"

echo "Extracting original scripts"
cp SOURCE/CAR/EarthEternal.car "${SCRATCH}/archives"
if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/EarthEternal.car" ; then
	echo "$0: Failed to extract orginal" >&2
	exit 1
fi

echo "Copying mod scripts"
pushd SOURCE/ClientMod/EarthEternal
find . -name '*.nut' | cpio -updm "${SCRATCH}/content"
find . -name '*.cnut' | cpio -updm "${SCRATCH}/content"
popd

echo "Copying mod resources"
pushd SOURCE/ClientMod/EarthEternal
find . -name '*.jpg' | cpio -updm "${SCRATCH}/content"
find . -name '*.jpeg' | cpio -updm "${SCRATCH}/content"
find . -name '*.png' | cpio -updm "${SCRATCH}/content"
popd

echo "Copying patch scripts"
pushd SOURCE/ClientMainScriptPatch
find . -name '*.nut' | cpio -updm "${SCRATCH}/content"
find . -name '*.jpg' | cpio -updm "${SCRATCH}/content"
find . -name '*.jpeg' | cpio -updm "${SCRATCH}/content"
find . -name '*.png' | cpio -updm "${SCRATCH}/content"
popd

MAJOR_ICEEE_VERSION=ICE1
BUILD_NUMBER=$(date +%Y%m%d_%H%M)
FULL_VERSION="0.8.6-${MAJOR_ICEEE_VERSION}-PF-${BUILD_NUMBER}"

#// DO NOT EDIT, THIS IS A GENERATED FILE
#///gVersion <- "0.8.9H-r16373-o214-20110513_1240";
#gVersion <- "0.8.6-r12-PF-20150205_2026";
echo "// DO NOT EDIT, THIS IS A GENERATED FILE
//gVersion <- \"0.8.9H-r16373-o214-20110513_1240\";
gVersion <- \"${FULL_VERSION}\"" > "${SCRATCH}/content/Version.nut";

pushd "${SCRATCH}/content"
echo "*******************************************************"
echo "Compiling all scripts"
echo "*******************************************************"
rm -f /tmp/scrfailed$$
find . -name '*.nut'|while read line ; do
	echo "Compiling ${line} to "$(dirname ${line})/$(basename ${line} .nut).cnut
	if compile_sq "${line}" ; then
		rm "${line}"
	else
		echo "Failed to compile ${line}" >&2
		touch /tmp/scrfailed$$
		exit
    fi
done
if [ -f /tmp/scrfailed$$ ]; then
	rm -f /tmp/scrfailed$$
	exit 1
fi
echo "*******************************************************"
echo "Decompressing original scripts (only those that don't already exist)"
echo "*******************************************************"
unzip -qn "${SCRATCH}/archives/EarthEternal.zip"
count=$(find -name '*.nut'|wc -l|awk '{ print $1 }')
if [ $count -gt 0 ]; then
	echo "There are .nut files remaining, this might mean the script failed." >&2
	exit 1
fi
echo "*******************************************************"
echo "Compressing earth eternal"
echo "*******************************************************"
zip -dg -q -r "${SCRATCH}/archives/EarthEternal.zip" *
popd

echo "*******************************************************"
echo "Archiving earth eternal"
echo "*******************************************************"
if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/EarthEternal.zip" ; then
	echo "$0: Failed to create new main archive ${SCRATCH}/archives/EarthEternal.zip" >&2
	exit 1
fi

rm -fr "${SCRATCH}/content"
mkdir -p "${SCRATCH}/content"

echo "*******************************************************"
echo "Extracting PF sound mods"
echo "*******************************************************"
cp SOURCE/CAR/Sound-ModSound.car "${SCRATCH}/archives"
if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/Sound-ModSound.car" ; then
	echo "$0: Failed to create new sound archive ${SCRATCH}/archives/Sound-ModSound.car" >&2
	exit 1
fi

echo "*******************************************************"
echo "Decompressing PF sounds mods"
echo "*******************************************************"
pushd "${SCRATCH}/content" >/dev/null
unzip -q "${SCRATCH}/archives/Sound-ModSound.zip"
popd >/dev/null

echo "*******************************************************"
echo "Copying in new sounds"
echo "*******************************************************"
pushd SOURCE/ExtraSounds >/dev/null
for i in $(find .) ; do
	target="${SCRATCH}/content/"$(dirname ${i})
	mkdir -p "${target}"
	case "${i}" in
		*".wav") out="${target}/$(basename $i .wav).ogg"
				echo "Converting ${i} from Wav to ${out}"
				if ! oggenc -Q "${i}" -o "${out}" ; then
					echo "Failed to encode ${i}" >&2
					exit 1
				 fi ;;
		*".mp3") out="${target}/$(basename $i .mp3).ogg"
				 echo "Converting ${i} from MP3 to ${out}"
				if ! ffmpeg -v 0 -i "${i}" -acodec libvorbis "${out}" ; then
					echo "Failed to encode ${i}" >&2
					exit 1
				 fi ;;
	*".ogg"|*".sound.cfg") echo "Copying ${i} to ${target}"
			 cp "${i}" "${target}" ;; 
	esac
done
popd >/dev/null
pushd "${SCRATCH}/content" >/dev/null
echo "*******************************************************"
echo "Compressing sounds"
echo "*******************************************************"
zip -dg -q -r "${SCRATCH}/archives/Sound-ModSound.zip" *
popd >/dev/null

echo "*******************************************************"
echo "Archiving sounds"
echo "*******************************************************"
if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/Sound-ModSound.zip" ; then
	echo "$0: Failed to archive new sounds ${SCRATCH}/archives/Sound-ModSound.zip" >&2
	exit 1
fi

rm -fr "${SCRATCH}/ap"
mkdir "${SCRATCH}/ap"
cp -Rp SOURCE/AssetPatches/* "${SCRATCH}/ap"
pushd "${SCRATCH}/ap" >/dev/null
echo "*******************************************************"
echo "Compiling patch scripts"
echo "*******************************************************"
for line in $(find . -type f) ; do
	dn=$(dirname ${line})
	bn=$(basename ${line})
	pushd "${dn}" >/dev/null
	case "${bn}" in
		*".mesh")	echo "Unfixing ${bn}"
					${base}/SCRIPTS/fix.sh -u ${bn} ;;
	*".mesh.xml")	echo "Converting XML mesh"
					${base}/SCRIPTS/convert.sh ${bn}
					rm ${bn} ;; 
*".skeleton.xml")	echo "Converting XML skeleton mesh"
					${base}/SCRIPTS/convert.sh ${bn}
					rm  ${bn} ;;
		*".nut")	echo "Compiling asset patch script ${line}"
					if ! compile_sq "${bn}" ; then
						echo "$0: Failed to compile $bn" >&2
						exit 1
					else rm ${bn}
					fi ;;
		*".wav") 	out="$(basename $bn .wav).ogg"
					echo "Converting ${bn} from Wav to ${out}"
					if ! oggenc -Q "${bn}" -o "${out}" ; then
						echo "Failed to encode ${bn}" >&2
						exit 1
					else rm ${bn}
					fi ;;
		*".mp3") 	out="$(basename $bn .mp3).ogg"
					echo "Converting ${bn} from MP3 to ${out}"
					if ! ffmpeg -v 0 -i "${bn}" -acodec libvorbis "${out}" ; then
						echo "Failed to encode ${line}" >&2
						exit 1
					else rm ${bn}
					fi ;;
	esac
	popd >/dev/null
done
find . -name OgreXMLConverter.log -exec rm {} \;
popd >/dev/null
echo "*******************************************************"
echo "Other Asset patches"
echo "*******************************************************"
for i in ${SCRATCH}/ap/*; do
	if [ -d "${i}" ] ; then
		carbase=$(basename ${i}).car
		carfile=${BASE_ASSETS}/${carbase}
		rm -fr "${SCRATCH}/content"
		mkdir -p "${SCRATCH}/content"
			zipbase=$(basename ${i}).zip
		if [ ! -f "${carfile}" ]; then
			echo "New car file for ${i}"
		else
			echo "Add to car file for ${i}"			
			cp "${carfile}" "${SCRATCH}/archives"
			if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/${carbase}" ; then
				echo "$0 : Failed decode ${SCRATCH}/archives/${carbase}" >&2
				exit 1
			fi
			pushd "${SCRATCH}/content" >/dev/null
			unzip -q "${SCRATCH}/archives/${zipbase}"
			popd >/dev/null
		fi		
		pushd "${i}" >/dev/null
		find . | cpio -updm "${SCRATCH}/content"
		popd >/dev/null
		pushd "${SCRATCH}/content" >/dev/null
		echo "Compressing ${zipbase}"
		zip -dg -q -r "${SCRATCH}/archives/${zipbase}" *
		popd >/dev/null
		if ! run_win_tool UTILITIES/CARDecode.exe "${SCRATCH}/archives/${zipbase}" ; then
				echo "$0 : Failed decode ${SCRATCH}/archives/${zipbase}" >&2
				exit 1
		fi
	fi
done

echo "*******************************************************"
echo "Others"
echo "*******************************************************"
for i in SOURCE/CAR/*.car ; do
	if [ ! -f "${SCRATCH}/archives/$(basename $i)" ] ;then
		cp "${i}" "${SCRATCH}/archives"
		echo "${i} -> ${SCRATCH}/archives"
	fi
done


#echo "*******************************************************"
#echo "Checksumming"
#echo "*******************************************************"
#rm -f "${SCRATCH}/HTTPChecksum.txt"
 
#echo "/Release/Current/EarthEternal.car=\""$(run_win_tool UTILITIES/MD5.exe "${SCRATCH}/archives/EarthEternal.car"|tr -d '\r')"\"" >> "${SCRATCH}/HTTPChecksum.txt"
#for i in ${SCRATCH}/archives/*.car; do
	#if [ $(basename $i) != "EarthEternal.car" ] ; then
		#echo "/Release/Current/Media/$(basename ${i})=\""$(run_win_tool UTILITIES/MD5.exe "${i}"|tr -d '\r')"\"" >> "${SCRATCH}/HTTPChecksum.txt"
	#fi
#done

echo "*******************************************************"
echo "Done!"
echo "*******************************************************"
if [ -d asset -a -d Data ]; then
	echo -n "Install to target assets dir? y/n: "
	read yesno
	case "${yesno}" in
		y|Y|yes|Yes) :
			cp "${SCRATCH}/archives/EarthEternal.car" ${TARGET_ASSETS}/Release/Current
			echo "Copied EarthEternal.car"
			for i in ${SCRATCH}/archives/*.car; do
				if [ $(basename $i) != "EarthEternal.car" ] ; then
					cp "${i}" ${TARGET_ASSETS}/Release/Current/Media
					echo "Copied $(basename ${i})"
				fi
			done
			
			${base}/SCRIPTS/checksum.sh
			
			echo "All files copied for version ${FULL_VERSION}" ;;
	esac
fi

