#!/bin/bash
x=0
while [ $x != 100 ] ; do
	< /dev/urandom tr -dc A-Za-z0-9 | head -c${1:-32};echo; 
	x=$(expr $x + 1)
done