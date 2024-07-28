#!/bin/bash
if [ ! -d "build" ]; then
  mkdir build
fi
cd build
CC=gcc CXX=g++ cmake .. -G "Unix Makefiles" \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,--stack,8388608" \
  -DCMAKE_SHARED_LINKER_FLAGS="-Wl,--stack,8388608" \
  -DCMAKE_EXE_LINKER_FLAGS_DEBUG="-Wl,--stack,8388608" \
  -DCMAKE_EXE_LINKER_FLAGS_MINSIZEREL="-Wl,--stack,8388608" \
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-Wl,--stack,8388608" \
  -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="-Wl,--stack,8388608" \
  -DCMAKE_SHARED_LINKER_FLAGS_DEBUG="-Wl,--stack,8388608" \
  -DCMAKE_SHARED_LINKER_FLAGS_MINSIZEREL="-Wl,--stack,8388608" \
  -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="-Wl,--stack,8388608" \
  -DCMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO="-Wl,--stack,8388608"
make
cp lib/binaryen/bin/libbinaryen.dll .

echo "Please copy libreadline8.dll and libtermcap-0.dll into build/ root to finalize installation"

cd ..
