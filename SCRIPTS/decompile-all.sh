#!/bin/bash
#
# Decompile all media
#

cd "$(dirname $0)"/..
base=$(pwd)
scratch=${base}/scratch
rm -fr ${scratch}
mkdir -p ${scratch}/archives

assets=${base}/../../Common/eeassets/original-client/assets/Release/Current/Media

for i in ${assets}/*.car ; do
	bn=$(basename $i .car)
	cp "$i" "${scratch}/archives/${bn}.car"
	pushd "${scratch}/archives"
	wine ${base}/UTILITIES/CARDecode.exe "${bn}.car"
	mkdir "${bn}"
	pushd "${bn}"
	unzip "../${bn}.zip"
	popd
	rm "${bn}.zip" "${bn}.car"
	"${base}/SCRIPTS/decompile-dir.sh" "${bn}"
	popd
done

