#!/usr/bin/env python3
import sys
with open(sys.argv[1], 'r') as file:
	filedata = file.read()
v = 3
while v < len(sys.argv):
	filedata = filedata.replace(sys.argv[v], sys.argv[v + 1])
	v += 2
with open(sys.argv[2], 'w') as file:
  	file.write(filedata)