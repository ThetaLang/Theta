#!/bin/bash

set -e

g++ --version

if [ ! -d "build" ]; then
  mkdir build
fi
cd build
cmake ..
make
cd ..
