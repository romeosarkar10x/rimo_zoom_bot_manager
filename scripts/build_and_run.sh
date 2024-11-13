#!/bin/bash

set -e

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi


cmake -B build
cd build
make -j8

cd ..
./build/bin/rimo_zoom_bot_manager