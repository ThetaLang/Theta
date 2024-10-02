#!/bin/bash

cd src/v8


# Generate build files
#gn gen out.gn/x64.release --args='dcheck_always_on=false is_component_build=false is_debug=false target_cpu="x64" use_custom_libcxx=false v8_monolithic=true v8_use_external_startup_data=false v8_enable_pointer_compression=false'


# Build V8
#ninja -C out.gn/x64.release v8_monolith #platform v8_init v8_libbase

gn gen out.gn/wee8 --args='is_component_build=false use_custom_libcxx=false v8_enable_fast_mksnapshot=true v8_enable_i18n_support=false v8_use_external_startup_data=false is_debug=false symbol_level=1 v8_enable_handle_zapping=false'

ninja -C out.gn/wee8 wee8
