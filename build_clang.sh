#!/bin/bash

set -e

if [ ! -d "build" ]; then
  mkdir build
fi
cd build
clang++ --version
cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ..
make
cd ..
