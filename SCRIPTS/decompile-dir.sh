#!/bin/bash

DISABLE_BASESKEL=XXXX
DISABLE_MESH=XXXXX
DISABLE_CNUT=XXXXX
DISABLE_SKEL=

# 
# Re-sequence prop IDs (using current scenery ID by default) 
#

base=$(dirname "${0}")/..
base=$(realpath "${base}")

for dir in $* ; do
	if [ ! -d "${dir}" ] ; then
		echo "Is not a directory" >&2
		exit 1
	fi
	echo "Processing ${dir}"
	find ${dir} > /tmp/$$.list
	while read line ; do
		case "${line}" in
		*.skeleton${DISABLE_BASESKEL}) ${base}/SCRIPTS/convert.sh "${line}" ;;
		*.skeleton${DISABLE_SKEL}) ${base}/SCRIPTS/convert.sh "${line}"
		
					# NO LONGER NEEDED, ANIM FILES CAN BE LOADED BY JME3
					
					# If there is a .skeletonlist file in this directory, see if the skel we are
					# decompiling is in it
					skeldir=$(dirname "${line}")
					foundskel=
					anim_prefix=$(basename ${line} .skeleton)
					
					for sk in $(ls "${skeldir}"|grep ".skeletonlist"); do
						based=$(basename ${line})
						if grep "${based}" "${skeldir}/${sk}" >/dev/null ; then
							anim_prefix=$(basename $sk  .skeletonlist)
							break
						fi
					done
					anims=$(ls ${skeldir}/${anim_prefix}*.anim)
					if [ -n "${anims}" ]; then
						outskel=$(dirname ${line})/$(basename ${line}).xml.tmp
						inskel=$(dirname ${line})/$(basename ${line}).xml
						echo "Processing animations ${anims} for ${inskel} to ${outskel}" 
	 					java -classpath ${base}/UTILITIES/icetools.jar \
	 						org.icetools.anim.AnimToXML \
	 						${inskel} \
	 						${outskel} \
	 						${anims}
	 					if [ -f "${outskel}" ]; then
	 						mv "${outskel}" "${inskel}"
	 					else
	 						echo "$0: No output skeleton XML" >&2
	 						exit 1
	 					fi
	 				fi

					;;			
			*.mesh${DISABLE_MESH}) ${base}/SCRIPTS/convert.sh "${line}" ;;
			*.cnut${DISABLE_CNUT}) nutf=$(dirname "${line}")/$(basename "${line}" .cnut).nut
					if [ -f "${nutf}" ] ; then
						echo "$0: warning. ${nutf} already exists, will not decompile" >&2
					else
						wine ${base}/UTILITIES/nutcracker.exe "${line}" > "${nutf}" 
					fi  ;;
		esac
	done < /tmp/$$.list
done