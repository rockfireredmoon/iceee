#!/bin/bash

source "$(dirname $0)"/shelllib.sh

cd "$(dirname $0)"/..
base=$(pwd)

run_win_tool ${base}/UTILITIES/CARDecode.exe $@