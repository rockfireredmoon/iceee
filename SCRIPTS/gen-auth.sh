#!/bin/bash

# MD5(username + ":" + MD5(password) + ":" + salt)

if [ ${#} -eq 3 ] ; then
	md5_pw=$(echo "$2"|md5sum|awk '{ print $1 }')
	txt="${1}:${md5_pw}:${3}"
	echo "${txt}"|md5sum|awk '{ print $1 }'
else
	echo "$0: requires 3 arguments. username, password and salt
fi