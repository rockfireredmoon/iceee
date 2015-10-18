#!/bin/bash

source "$(dirname $0)"/shelllib.sh

cd "$(dirname $0)"/..
base=$(pwd)

pushd ${base}/asset
find Release -type f|while read line ; do md5sum $line| \
	awk '{ print "/" substr($2,1) "=\"" $1 "\"" }' ; done | \
	sort -u > ${base}/Data/HTTPChecksum.txt
popd

echo "*******************************************************"
echo "Done!"
echo "*******************************************************"

