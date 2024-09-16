#!/bin/bash
cd v8

echo "PWD: $PWD"

echo "start"

# Generate build files
gn gen out.gn/x64.release --args='is_debug=false v8_static_library=true v8_monolithic=true is_component_build=false v8_enable_i18n_support=false v8_use_external_startup_data=false'

echo "here"
# Build V8
ninja -C out.gn/x64.release v8_monolith
