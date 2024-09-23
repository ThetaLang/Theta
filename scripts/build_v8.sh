#!/bin/bash

echo "pwd is $PWD"
cd src/v8


# Generate build files
gn gen out.gn/x64.release --args='dcheck_always_on=false is_component_build=false is_debug=false target_cpu="x64" use_custom_libcxx=false v8_monolithic=true v8_use_external_startup_data=false v8_enable_pointer_compression=false'
# Build V8
ninja -C out.gn/x64.release v8_monolith #platform v8_init v8_libbase
