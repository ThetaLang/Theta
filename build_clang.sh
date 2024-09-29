#!/bin/bash

set -e

if [ ! -d "build" ]; then
  mkdir build
fi
cd build
clang++ --version
CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake ..
make
cd ..
