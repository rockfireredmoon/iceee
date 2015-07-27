#!/bin/bash

pushd SOURCE/AssetPatches >/dev/null
ls -d Terrain-${1}_*|awk '{ print "\t[\"" $0 "\"] = true," '}
popd >/dev/null
