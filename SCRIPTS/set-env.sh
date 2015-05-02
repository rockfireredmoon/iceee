#!/bin/bash

#
# This script sets the environment of a set of tiles depending
# on  a pattern in the terrain configuration. For example,
# can be used to set the environment to Swineland_Dead to all
# tiles that contain the text Grass_Undead
#
# Run the script for a directory that contains all the CAR files
# of the terrain you want to process 

# EXAMPLE
#
# set-env.sh Grass_Undead1 Swineland_Dead Terrain-Europe_

pattern="$1" ; shift
envname="$1" ; shift
terrain="$1" ; shift

# Convert all to car
for i in ${terrain}*.car ; do
	wine ~/Workspaces/PFServer/EEServer36/UTILITIES/CARDecode.exe $i
done

# Unpack zips
for i in ${terrain}*.zip ; do
	dn=$(basename $i .zip)
	rm -fr ${dn}
	mkdir ${dn}
	pushd ${dn}
	unzip ../$i
	popd
done

rm -f /tmp/ter.done
for i in $(grep -H "$pattern" ${terrain}*/*.cfg|awk -F: '{print $1}'|sort -u) ; do
	bn=$(basename $i .cfg)
	dn=$(dirname $i)
	nutfile=$(dirname $i)/${bn}.nut
	cnutfile=$(dirname $i)/${bn}.cnut
	tile=$(echo $bn|awk -F_ '{ print $2 }')
	echo "Doing $i"
	echo "this.TerrainPageDef.${tile} <- {
	Environment = \"${envname}\"
};" > ${nutfile}
	wine ~/Workspaces/PFServer/EEServer36/UTILITIES/sq.exe -o "${cnutfile}" -c  "${nutfile}"
	rm -f "${nutfile}"
	echo "${dn}.car" >> /tmp/ter.done
     echo $i
done

# Repack zips
for i in ${terrain}* ; do
	if [ -d "$i" ] ; then
		pushd "$i"
		zn=$(basename $i)
                echo "Rezipping $i"
		zip -r ../${zn}.zip * 
		popd	
		rm -fr ${i}
	fi
done

# And finally zips back to cars
for i in ${terrain}*.zip ; do
	echo "Re-carring $i"
	wine ~/Workspaces/PFServer/EEServer36/UTILITIES/CARDecode.exe $i
        rm $i
done

echo "Processed ........."
cat /tmp/ter.done|awk '{ print "   " $1 }'
