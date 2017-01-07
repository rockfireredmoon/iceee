#!/bin/bash
#

trap "rm -f /tmp/$$-gf-tmp.tgz /tmp/$$.tarlist" 0 1 2 3 15

DATA_DIRS="Instance AIScript Scenery QuestScripts Creatures"

incoming=n


listen_port=$(grep "^HTTPListenPort=" -- ServerConfig.txt|tr -d '\r'|awk -F= '{ print $2 }')
if [ -n "${listen_port}" ] ; then
	listen_port=":${listen_port}"
fi
listen_address=$(grep "^BindAddress=" -- ServerConfig.txt|tr -d '\r'|awk -F= '{ print $2 }')
if [ -z "${listen_address}" ] ; then
	listen_address="127.0.0.1"
fi
server_url="http://${listen_address}${listen_port}"
auth_token=$(grep "^RemoteAuthenticationPassword=" -- ServerConfig.txt|tr -d '\r'|awk -F= '{ print $2 }')

message_server() {
	if ! curl --data "action=syschat&authtoken=${auth_token}&data=$*" ${server_url}/remoteaction >/dev/null 2>&1 ; then
		echo "$0: Failed to send message to server." &2
		exit 1
	fi
}

# Parse arguments
if [ $# != 5 ] ; then
    echo "usage: $0 <gitDir> <gameDir> <branch> <serviceName> <updates>" >&2
    exit 2
fi

GIT_DIR="$1"
if [ ! -d "${GIT_DIR}/.git" ] ; then
    echo "$0:$1 is not a Git workspace" >&2
    exit 1
fi

GAME_DIR="$2"
if [ ! -f "${GAME_DIR}/SessionVars.txt" ] ; then
    echo "$0:$2 is not a game instance data directory" >&2
    exit 1
fi

BRANCH="$3"
if [ -z "$BRANCH" ] ; then
    echo "$0: Branh not specified" >&2
    exit 1
fi

SERVICE="$4"
if [ -z "$SERVICE" ] ; then
    echo "$0: Service not specified" >&2
    exit 1
fi

UPDATES="$5"
if [ -z "$UPDATES" ] ; then
    echo "$0: Updates not specified" >&2
    exit 1
fi

pushd "${GIT_DIR}"

# First fetch
if ! git fetch >&2 ; then
	echo "$0: Fetch failed!" >&2
	exit 1
fi

# See if there are any incoming changes
lines=$(git log "..origin/${BRANCH}" >&1|wc -l)
if [ ${lines} -gt 0 ] ; then
	message_server "There are incoming Git changes. The Rebuild/Restart procedure will start in 10 minutes, please logoff before then."
	sleep 5m
	message_server "There are incoming Git changes. The Rebuild/Restart procedure will start in 5 minutes, please logoff before then."
	sleep 4m
	message_server "There are incoming Git changes. The Rebuild/Restart procedure will start in 1 minute, please logoff before then."
	sleep 30
	message_server "There are incoming Git changes. The Rebuild/Restart procedure will start in 30 seconds, please logoff before then."
	sleep 30
	incoming=y
	service $SERVICE stop
fi
popd

# Tar up the files from the instance
pushd "${GAME_DIR}"
find -L ${DATA_DIRS} > /tmp/$$.tarlist
if ! tar czhf /tmp/$$-gf-tmp.tgz -T /tmp/$$.tarlist ; then
    echo "$0: failed to archive game files" >&2
    rm -f /tmp/$$.tarlist /tmp/$$-gf-tmp.tgz
    exit 1
fi
rm -f /tmp/$$.tarlist
popd

# Build message from audits
echo "Building audit message .."
pushd ${GAME_DIR}/Audit
echo "AutoSync from ${GAME_DIR}" > /tmp/$$.msg
for i in ""$(ls) ; do
    if [ -n "$i" ] ; then
        echo "------${i}------" >> /tmp/$$.msg
        echo "Adding $i"
        cat ${i} >> /tmp/$$.msg
    fi
done
popd

# Extract new files over Git ones
pushd "${GIT_DIR}"
echo "Extracting changed files .."
if ! tar xzf /tmp/$$-gf-tmp.tgz ; then
    echo "$0: failed to extract files onto workspac" >&2
    rm -f /tmp/$$-gf-tmp.tgz
    exit 1
fi
rm -f /tmp/$$-gf-tmp.tgz

# Commit
echo "Committing .."
git commit -a -m "${MESG}"
ret=$?
echo "Committed $ret"
if [ $ret != 0 -a $ret != 1 ] ; then
    echo "$0: failed to commit changes - $ret" >&2
    exit 1
fi
if [ $ret = 0 ] ; then
    rm -f ${GAME_DIR}/Audit/*
fi

# Pull
if ! git pull >&2 ; then
    echo "$0: Pull failed!" >&2
    exit 1
fi

# Push
if ! git push ; then
    echo "$0: failed to push" >&2
    exit 1
fi

#

popd

# Rebuild/restart the server if we stopped it

if [ "$incoming" = "y" ] ; then
	pushd $GIT_DIR
	
	if ! make ; then
		echo "$0: Failed to make" >&2
		exit 1
	fi

	if [ -f $UPDATES/EarthEternal.car ] ; then
        mv $UPDATES/EarthEternal.car $GAME_DIR/asset/Release/Current/EarthEternal.car
	fi
	if [ $(ls $UPDATES|wc -l) -gt 0 ] ; then
	        mv $UPDATES/*.car $GAME_DIR/asset/Release/Current/Media/
	fi

	# TODO not right for TAWD
	if ! cp SOURCE/Server/$SERVICE  /opt/$SERVICE/$SERVICE ; then
	   echo "Failed to copy server binary" >&2
	   exit 1
	fi
	
	service $SERVICE start
	
	popd
fi














cd "$(dirname $0)"/..
base=$(pwd)

server_url="http://127.0.0.1:8080"
auth_token=$(grep "^RemoteAuthenticationPassword=" -- ServerConfig.txt|tr -d '\r'|awk -F= '{ print $2 }')

message_to_server() {
	if ! curl --data "action=syschat&authtoken=${auth_token}&data=$*" ${server_url}/remoteaction ; then
		echo "ERR: Failed to send the test message."
		exit 1
	fi
}

# First fetch
if ! git fetch ; then
	message_to_server "Git Fetch failed. Current status unknown." >&2
	exit 1
fi



	

