#!/bin/bash
set -x

if [ ! -d v8/build ]; then
    stdbuf -oL echo "Fetching V8..."
    stdbuf -oL fetch v8
else
    stdbuf -oL echo "Syncing V8..."
    stdbuf -oL gclient sync
fi
