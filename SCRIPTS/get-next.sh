#!/bin/bash
#
# Script to find the highest ID's in a data file

type=
usage="$0 <quest|itemdef|creaturedef|prop|groveprop>" 
while [ $# -gt 0 ] ; do
    case "$1" in
    quest|itemdef|creaturedef|prop|groveprop) type=$1 ;;
                *) echo "$usage" >&2
                   exit 1 ;;
    esac
    shift
done

if [ -z "$type" ] ; then
   echo "$usage" >&2
   exit 1
fi

if [ "$type" = quest ] ; then
    for i in $(cat Packages/QuestPack.txt|dos2unix|egrep -v "^;"|tr '\\' '/'); 
    do 
        low=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|head -1)
        high=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|tail -1)
        echo "$i: $low - $high"
    done
elif [ "$type" = creaturedef ] ; then
    for i in $(cat Packages/CreaturePack.txt|dos2unix|egrep -v "^;"|tr '\\' '/'); 
    do 
        low=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|head -1)
        high=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|tail -1)
        echo "$i: $low - $high"
    done
elif [ "$type" = itemdef ] ; then
    for i in $(cat Packages/ItemPack.txt|dos2unix|egrep -v "^;"|tr '\\' '/'); 
    do 
        low=$(cat $i|dos2unix|egrep "^mID="|awk -F= '{ print $2 }'|sort -nu|head -1)
        high=$(cat $i|dos2unix|egrep "^mID="|awk -F= '{ print $2 }'|sort -nu|tail -1)
        echo "$i: $low - $high"
    done
elif [ "$type" = prop ] ; then
    mlow=
    mhigh=
    find Scenery -name '*.txt' > /tmp/scn.tmp
    while read i ; do
        low=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|head -1)
        high=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|tail -1)
        echo "$i: $low - $high"
	if [ -z "${mlow}" ] ; then
            mlow=${low}
        elif [ ${low} -lt ${mlow} ] ; then
            mlow=${low}
        fi
	if [ -z "${mhigh}" ] ; then
            mhigh=${high}
        elif [ ${high} -gt ${mhigh} ] ; then
            mhigh=${high}
        fi
    done < /tmp/scn.tmp
    rm /tmp/scn.tmp
    echo "Overall: $mlow - $mhigh"
elif [ "$type" = groveprop ] ; then
    mlow=
    mhigh=
    find Grove -name '*.txt' > /tmp/scn.tmp
    while read i ; do
        low=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|head -1)
        high=$(cat $i|dos2unix|egrep "^ID="|awk -F= '{ print $2 }'|sort -nu|tail -1)
        echo "$i: $low - $high"
	if [ -z "${mlow}" ] ; then
            mlow=${low}
        elif [ ${low} -lt ${mlow} ] ; then
            mlow=${low}
        fi
	if [ -z "${mhigh}" ] ; then
            mhigh=${high}
        elif [ ${high} -gt ${mhigh} ] ; then
            mhigh=${high}
        fi
    done < /tmp/scn.tmp
    rm /tmp/scn.tmp
    echo "Overall: $mlow - $mhigh"
fi
