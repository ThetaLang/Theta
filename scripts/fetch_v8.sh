#!/bin/bash
set -x

if [ ! -d v8/build ]; then
    echo "Fetching V8..."
    fetch v8
else
    echo "Syncing V8..."
    gclient sync
fi
