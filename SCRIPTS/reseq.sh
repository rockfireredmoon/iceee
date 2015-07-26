#!/bin/bash

# 
# Re-sequence prop IDs (using current scenery ID by default) 
#

cd "$(dirname $0)"/..
base=$(pwd)

zone_id="$1"
seq="$2"

if [ -z "$zone_id" ] ; then
	echo "usage: $0 <zoneId> [<seq>]" >&2
	exit 1
fi

if [ -z "$seq" ] ; then
	seq=$(grep "SceneryAdditive=" SessionVars.txt|tr -d '\r'|awk -F= '{ print $2 }')
	if [ -z "$seq" ] ; then
		echo "Could not find sequential ID from ServerConfig.txt." >&2
		exit 1
	fi
fi

dir=Scenery/$zone_id
if [ ! -d $dir ] ; then
	echo "Zone $zone_id does not exist in $dir" >&2
	exit 1
fi

pushd $dir
echo "Will start at ID $seq on Zone $zone_id in $dir"
echo "WARNING: The server should not be running when running this. Please check. Press RETURN to continue"
read dummy

for i in $(ls|egrep "\\.txt$"); do
	echo $i
	while read line ; do
		case "$line" in
			"ID="*) echo -e "ID=${seq}\r" 
					seq=$(expr $seq + 1) ;;
				*) echo -e "${line}" ;;
		esac
	done < ${i} > /tmp/resequ${i}
	chmod u+w ${i}
	cat /tmp/resequ${i} > ${i}
	rm /tmp/resequ${i}
done
popd 

echo "Resetting SceneryAdditive in SessionVars.txt to $seq"
sed -i 's/SceneryAdditive=.*/SceneryAdditive='${seq}'\r/g' SessionVars.txt
	
	