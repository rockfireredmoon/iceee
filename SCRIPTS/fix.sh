
#
# Fix the serializer version so meshes can be converted
#

cd "$(dirname $0)"/..
base=$(pwd)

if [ "$1" = "-u" ] ; then 
	op=UNFIX 
	shift
elif [ "$1" = "-f" ] ; then 
	op=FIX 
	shift
else
	echo "$0: $0 [-f|-u] file1 file2 ..." >&2
	exit 2
fi 

java -classpath ${base}/UTILITIES/icetools.jar org.icetools.modelman.Fixer $*