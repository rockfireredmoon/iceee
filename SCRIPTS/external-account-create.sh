#!/bin/bash
#
# Script to externally create game server accounts. Intended to be called by the
# registration system on the TAW website.

if [ $# -lt 3 ] ; then
	echo "ERR: Incorrect number of arguments"
	exit 1
fi

cd "$(dirname $0)"/..
base=$(pwd)

username="$1"
password="$2"
grove_name="$3"
reg_key="$4"
server_url="http://127.0.0.1:8080"
auth_token=$(grep "^RemoteAuthenticationPassword=" -- ServerConfig.txt|tr -d '\r'|awk -F= '{ print $2 }')

# Check arguments

if [ -z "$username" ] ; then
	echo "ERR: must supply a username"
	exit 1
fi
if [ -z "$password" ] ; then
	echo "ERR: must supply a password"
	exit 1
fi
if [ -z "$grove_name" ] ; then
	echo "M ust supply a grove name"
	exit 1
fi

# Prevent this script being used more than once at any time

for i in 1 2 3 4 5 6 7 8 9 10 ; do
	if [ ! -f /tmp/import-in-progress.tmp ] ; then
		break
	else
		sleep 1
	fi
done
if [ -f /tmp/import-in-progress.tmp ] ; then
	echo "ERR: another account import is still in progress. Somethinig must have gone, please inform an administrator."
	exit 1
fi
touch /tmp/import-in-progress.tmp
trap "rm -f /tmp/import-in-progress.tmp ; exit $?" 0 1 2 3 15 

# First check if the user already exists
# The grep is weird because these are DOS files and we want EXACT matches
user_account_file=$(egrep -H "^Name=${username}" -- Accounts/*.txt|cat -v|fgrep "=${username}^M"|awk -F: '{ print $1 }')
grove_account_file=$(egrep -H "^GroveName=${grove_name}" -- Accounts/*.txt|cat -v|fgrep "=${grove_name}^M"|awk -F: '{ print $1 }')

# If we were supplied a reg_key, then assume this is a password update for an existing user
if [ -n "$reg_key" ] ; then
	
	if [ -z  "$user_account_file" ] ; then
		echo "ERR: Account for ${username} does not exist. Please contact an administrator "
		exit 1
	fi

	# Make sure the registration key is the same
	existing_reg_key=$(grep "^RegKey=" -- ${user_account_file}|tr -d '\r'|awk -F= '{ print $2 }')
	if [ "$reg_key" != "$existing_reg_key" ] ; then
		echo "ERR: Account for ${username} exists, but has a different registration key. Please contact an administrator "
		exit 1
	fi
	
	# Get existing auth hash
	existing_auth=$(grep "^Auth=" -- ${user_account_file}|tr -d '\r'|awk -F= '{ print $2 }')
	
	# Generate new auth hash
	new_auth=$(java -classpath UTILITIES/icetools.jar org.icetools.pw.MakePw "${username}" "${password}" S)
	
	# Swap them
	if ! sed -i "s/Auth=${existing_auth}/Auth=${new_auth}/g"  ${user_account_file} ; then
		echo "ERR: Passworrd reset for ${username} failed. Please contact an administrator "
		exit 1
	fi
	
	echo "OK"
else # If no reg key, assume we are creating a new account 
	if [ -n "$user_account_file" ] ; then
		echo "ERR: Account already exists" >&2
		exit 1
	fi
	if [ -n "$grove_account_file" ] ; then
		echo "ERR: A user with the same grove name already exists, please choose another"
		exit 1
	fi

	# Generate a registration key and have it imported
	reg_key=$(cat /proc/sys/kernel/random/uuid|tr -d '-')
	echo -e "$reg_key\r" > ImportKeys.txt
	if ! curl --data "action=importkeys&authtoken=${auth_token}" ${server_url}/remoteaction >/dev/null 2>&1 ; then
		echo "ERR: Failed to import new registration key. Please contact an administrator for assistance."
		exit 1
	fi

	# Now post to the account creation page
	output=$(curl --data "action=createaccount&regkey=${reg_key}&username=${username}&grove=${grove_name}&password=${password}" ${server_url}/newaccount 2>/dev/null)
	if [ "$output" != "Account creation was successful." ] ; then
		echo "ERR: Failed to create account for ${username} (${reg_key}). ${output}\nPlease contact an administrator for assistance."
		exit 1
	fi

	echo -e "${reg_key}"
fi
	

