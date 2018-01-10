#!/usr/bin/env python
with open(sys.argv[1], 'r') as file:
	filedata = file.read()
filedata = filedata.replace(sys.argv[3], sys.argv[4])
with open(sys.argv[2], 'w') as file:
  	file.write(filedata)