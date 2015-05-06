#!/bin/bash

cd "$(dirname $0)"/..
base=$(pwd)

SCRATCH="${base}/scratch"

rm -fr "${SCRATCH}"
mkdir -p "${SCRATCH}"
mkdir -p "${SCRATCH}/archives"
mkdir -p "${SCRATCH}/content"

echo "Extracting catalog"
cp SOURCE/CAR/Catalogs.car "${SCRATCH}/archives"
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/Catalogs.car"

echo "Decompressing catalog"
pushd "${SCRATCH}/content"
unzip -q "${SCRATCH}/archives/Catalogs.zip"
echo "Generating abilities"
cp ${base}/Data/AbilityTable.txt .
wine ${base}/UTILITIES/EEUtilAbilityTable.exe
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
		output="./Abilities.cnut"
	fi
	echo "Compiling ${script} to ${output}"
	if ! wine ${base}/UTILITIES/sq.exe -o ${output} -c ${script} ; then
		echo "$0: Failed to compile ${script}" >&2
		exit 1
	else rm ${script}
	fi
done
zip -dg -q -r "${SCRATCH}/archives/Catalogs.zip" *
popd

echo "Archiving catalog"
wine ${base}/UTILITIES/CARDecode.exe "${SCRATCH}/archives/Catalogs.zip"


rm -fr "${SCRATCH}/content"
mkdir -p "${SCRATCH}/content"

echo "Extracting original scripts"
cp SOURCE/CAR/EarthEternal.car "${SCRATCH}/archives"
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/EarthEternal.car"

echo "Copying mod scripts"
pushd SOURCE/ClientMod/EarthEternal
find . -name '*.nut' | cpio -updm "${SCRATCH}/content"
find . -name '*.cnut' | cpio -updm "${SCRATCH}/content"
popd

echo "Copying patch scripts"
pushd SOURCE/ClientMainScriptPatch
find . -name '*.nut' | cpio -updm "${SCRATCH}/content"
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
find . -name '*.nut'|while read line ; do
	echo "Compiling ${line} to "$(dirname ${line})/$(basename ${line} .nut).cnut
	if wine ${base}/UTILITIES/sq.exe -o $(dirname ${line})/$(basename ${line} .nut).cnut -c ${line} ; then
		rm "${line}"
	else
		echo "Failed to compile ${line}" >&2
		exit 1
    fi
done
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
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/EarthEternal.zip"

rm -fr "${SCRATCH}/content"
mkdir -p "${SCRATCH}/content"

echo "*******************************************************"
echo "Extracting PF sound mods"
echo "*******************************************************"
cp SOURCE/CAR/Sound-ModSound.car "${SCRATCH}/archives"
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/Sound-ModSound.car"

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
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/Sound-ModSound.zip"

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
		*".nut")	echo "Compiling asset patch script ${line}"
					if ! wine ${base}/UTILITIES/sq.exe -o $(basename $bn .nut).cnut -c ${bn} ; then
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
popd >/dev/null
echo "*******************************************************"
echo "Other Asset patches"
echo "*******************************************************"
for i in ${SCRATCH}/ap/*; do
	if [ -d "${i}" ] ; then
		carbase=$(basename ${i}).car
		carfile=${base}/asset/Release/Current/Media/${carbase}
		rm -fr "${SCRATCH}/content"
		mkdir -p "${SCRATCH}/content"
			zipbase=$(basename ${i}).zip
		if [ ! -f "${carfile}" ]; then
			echo "New car file for ${i}"
		else
			echo "Add to car file for ${i}"			
			cp "${carfile}" "${SCRATCH}/archives"
			wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/${carbase}"
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
		wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/${zipbase}"
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


echo "*******************************************************"
echo "Checksumming"
echo "*******************************************************"
rm -f "${SCRATCH}/HTTPChecksum.txt"
 
echo "/Release/Current/EarthEternal.car=\""$(wine UTILITIES/MD5.exe "${SCRATCH}/archives/EarthEternal.car"|tr -d '\r')"\"" >> "${SCRATCH}/HTTPChecksum.txt"
for i in ${SCRATCH}/archives/*.car; do
	if [ $(basename $i) != "EarthEternal.car" ] ; then
		echo "/Release/Current/Media/$(basename ${i})=\""$(wine UTILITIES/MD5.exe "${i}"|tr -d '\r')"\"" >> "${SCRATCH}/HTTPChecksum.txt"
	fi
done
#echo "/Release/Current/Media/Catalogs.car="$(wine UTILITIES/MD5.exe "${SCRATCH}/archives/Catalogs.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"
#echo "/Release/Current/Media/Prop-ModAddons1.car="$(wine UTILITIES/MD5.exe "SOURCE/CAR/Prop-ModAddons1.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"
#echo "/Release/Current/Media/Sound-ModSound.car="$(wine UTILITIES/MD5.exe "${SCRATCH}/archives/Sound-ModSound.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"

echo "*******************************************************"
echo "Done!"
echo "*******************************************************"
if [ -d asset -a -d Data ]; then
	echo -n "Install to assets dir? y/n: "
	read yesno
	case "${yesno}" in
		y|Y|yes|Yes) :
			cp "${SCRATCH}/archives/EarthEternal.car" ${base}/asset/Release/Current
			echo "Copied EarthEternal.car"
			for i in ${SCRATCH}/archives/*.car; do
				if [ $(basename $i) != "EarthEternal.car" ] ; then
					cp "${i}" ${base}/asset/Release/Current/Media
					echo "Copied $(basename ${i})"
				fi
			done
			#cp "${SCRATCH}/archives/Catalogs.car" ${base}/asset/Release/Current/Media
			#cp "SOURCE/CAR/Prop-ModAddons1.car" ${base}/asset/Release/Current/Media
			#cp "${SCRATCH}/archives/Sound-ModSound.car" ${base}/asset/Release/Current/Media
			cp "${SCRATCH}/HTTPChecksum.txt" ${base}/Data
			echo "All files copied for version ${FULL_VERSION}" ;;
	esac
fi
