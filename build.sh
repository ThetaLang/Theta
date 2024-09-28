#!/bin/bash

set -e

if [ ! -d "build" ]; then
  mkdir build
fi
cd build
g++ --version
cmake ..
make
cd ..
