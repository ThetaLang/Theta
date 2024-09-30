#!/bin/bash

set -e

g++ --version

if [ ! -d "build" ]; then
  mkdir build
fi
cd build
g++ --version
cmake ..
make
cd ..
