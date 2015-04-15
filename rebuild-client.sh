#!/bin/bash

cd "$(dirname $0)"
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
echo "Compiling abilities"
wine ${base}/UTILITIES/sq.exe -o Abilities.cnut -c AbilityTable.nut
rm AbilityTable.nut
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
find . -name '*.nut' | cpio -pdm "${SCRATCH}/content"
find . -name '*.cnut' | cpio -pdm "${SCRATCH}/content"
popd

echo "Copying patch scripts"
pushd SOURCE/ClientMainScriptPatch
find . -name '*.nut' | cpio -pdm "${SCRATCH}/content"
popd

pushd "${SCRATCH}/content"
echo "Compiling all scripts"
find . -name '*.nut'|while read line ; do
	echo "Compiling ${line} to "$(dirname ${line})/$(basename ${line} .nut).cnut
	if wine ${base}/UTILITIES/sq.exe -o $(dirname ${line})/$(basename ${line} .nut).cnut -c ${line} ; then
		rm "${line}"
	else
		echo "Failed to compile ${line}" >&2
		exit 1
    fi
done
echo "Decompressing original scripts (only those that don't already exist)"
unzip -qn "${SCRATCH}/archives/EarthEternal.zip"
count=$(find -name '*.nut'|wc -l|awk '{ print $1 }')
if [ $count -gt 0 ]; then
	echo "There are .nut files remaining, this might mean the script failed." >&2
	exit 1
fi
echo "Compressing earth eternal"
zip -dg -q -r "${SCRATCH}/archives/EarthEternal.zip" *
popd

echo "Archiving earth eternal"
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/EarthEternal.zip"

rm -fr "${SCRATCH}/content"
mkdir -p "${SCRATCH}/content"

echo "Extracting PF sound mods"
cp SOURCE/CAR/Sound-ModSound.car "${SCRATCH}/archives"
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/Sound-ModSound.car"

echo "Decompressing PF sounds mods"
pushd "${SCRATCH}/content"
unzip -q "${SCRATCH}/archives/Sound-ModSound.zip"
popd

echo "Copying in new sounds"
pushd SOURCE/ExtraSounds
for i in $(find .) ; do
	target="${SCRATCH}/content/"$(dirname ${i})
	mkdir -p "${target}"
	echo "FILE: $i $(file $i)"
	case "${i}" in
		*".wav") out="${target}/$(basename $i .wav).ogg"
				echo "Converting ${i} from Wav to ${out}"
				if ! oggenc -Q "${i}" -o "${out}" ; then
					echo "Failed to encode ${i}" >&2
					exit 1
				 fi ;;
		*".mp3") out="${target}/$(basename $i .mp3).ogg"
				 echo "Converting ${i} from MP3 to ${out}"
				 if ! ffmpeg -i "${i}" -acodec libvorbis "${out}" ; then
					echo "Failed to encode ${i}" >&2
					exit 1
				 fi ;;
	*".ogg"|*".sound.cfg") echo "Copying ${i} to ${target}"
			 cp "${i}" "${target}" ;; 
	esac
done
popd
pushd "${SCRATCH}/content"
echo "Compressing sounds"
zip -dg -q -r "${SCRATCH}/archives/Sound-ModSound.zip" *
popd

echo "Archiving sounds"
wine UTILITIES/CARDecode.exe "${SCRATCH}/archives/Sound-ModSound.zip"

echo "Checksumming"
rm -f "${SCRATCH}/HTTPChecksum.txt"
 
echo "/Release/Current/EarthEternal.car="$(wine UTILITIES/MD5.exe "${SCRATCH}/archives/EarthEternal.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"
echo "/Release/Current/Media/Catalogs.car="$(wine UTILITIES/MD5.exe "${SCRATCH}/archives/Catalogs.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"
echo "/Release/Current/Media/Prop-ModAddons1.car="$(wine UTILITIES/MD5.exe "SOURCE/CAR/Prop-ModAddons1.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"
echo "/Release/Current/Media/Sound-ModSound.car="$(wine UTILITIES/MD5.exe "${SCRATCH}/archives/Sound-ModSound.car"|tr -d '\r') >> "${SCRATCH}/HTTPChecksum.txt"

if [ -d asset -a -d Data ]; then
	echo -n "Install to assets dir? y/n: "
	read yesno
	case "${yesno}" in
		y|Y|yes|Yes) :
			cp "${SCRATCH}/archives/EarthEternal.car" asset/Release/Current
			cp "${SCRATCH}/archives/Catalogs.car" asset/Release/Current/Media
			cp "SOURCE/CAR/Prop-ModAddons1.car" asset/Release/Current/Media
			cp "${SCRATCH}/archives/Sound-ModSound.car" asset/Release/Current/Media
			cp "${SCRATCH}/HTTPChecksum.txt" Data
			echo "All files copied" ;;
	esac
fi
