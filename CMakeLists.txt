cmake_minimum_required(VERSION 3.10)
project(Theta)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DBUILD_TESTS=OFF -Wno-maybe-uninitialized")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include ExternalProject module
include(ExternalProject)

# Define source directories using absolute paths
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(LEXER_DIR ${SRC_DIR}/lexer)
set(COMPILER_DIR ${SRC_DIR}/compiler)
set(PARSER_DIR ${SRC_DIR}/parser)
set(EXCEPTIONS_DIR ${SRC_DIR}/exceptions)
set(AST_DIR ${PARSER_DIR}/ast)
set(OPTIMIZATION_DIR ${COMPILER_DIR}/optimization)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
set(CATCH2_DIR ${TEST_DIR}/catch2)
set(BINARYEN_DIR ${CMAKE_SOURCE_DIR}/lib/binaryen)
set(WASMER_DIR ${CMAKE_SOURCE_DIR}/lib/wasmer)
set(V8_DIR ${CMAKE_SOURCE_DIR}/lib/v8)  # Absolute path to the V8 directory within your project

# Collect all source files except main.cpp
file(GLOB SRC_FILES
    "${SRC_DIR}/*.cpp"
    "${LEXER_DIR}/*.cpp"
    "${PARSER_DIR}/*.cpp"
    "${EXCEPTIONS_DIR}/*.cpp"
    "${COMPILER_DIR}/*.cpp"
    "${AST_DIR}/*.cpp"
    "${OPTIMIZATION_DIR}/*.cpp"
)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/theta.cpp")

# Add main source file separately
set(MAIN_SRC "${CMAKE_SOURCE_DIR}/theta.cpp")

# Add executable for the main program
add_executable(theta ${SRC_FILES} ${MAIN_SRC})

# Add a custom command to copy the WAT file
set(WAT_FILE_SRC "${CMAKE_SOURCE_DIR}/src/wasm/ThetaLangCore.wat")
set(WAT_FILE_DEST "${CMAKE_BINARY_DIR}/wasm")
add_custom_target(copy_wat ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WAT_FILE_DEST}
    COMMAND ${CMAKE_COMMAND} -E copy ${WAT_FILE_SRC} ${WAT_FILE_DEST}
    COMMENT "Copying WAT file to build directory"
)
add_dependencies(copy_wat theta)

# Add the readline library
if (WIN32)
    set(READLINE_INCLUDE_DIR "C:/msys64/mingw64/include")
    set(READLINE_LIBRARY "C:/msys64/mingw64/lib/libreadline.dll.a")

    # Check if readline is found
    if (EXISTS ${READLINE_INCLUDE_DIR}/readline/readline.h AND EXISTS ${READLINE_LIBRARY})
        include_directories(${READLINE_INCLUDE_DIR})
        link_directories("C:/msys64/mingw64/lib")
        target_link_libraries(theta ${READLINE_LIBRARY})
    else ()
        message(FATAL_ERROR "Readline library not found.")
    endif()
else()
    target_link_libraries(theta readline)
endif()

# Include directories
include_directories(${SRC_DIR} ${CATCH2_DIR} ${BINARYEN_DIR}/src ${WASMER_DIR}/include)

# Add Binaryen
add_subdirectory(${BINARYEN_DIR})


ExternalProject_Add(
    v8_external
    PREFIX ${CMAKE_SOURCE_DIR}/lib/v8
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib/v8
    DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/lib/v8
    DOWNLOAD_COMMAND
        bash ${CMAKE_SOURCE_DIR}/scripts/fetch_v8.sh > ${CMAKE_SOURCE_DIR}/fetch_v8.log 2>&1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND gclient sync
    BUILD_COMMAND 
        bash ${CMAKE_SOURCE_DIR}/scripts/build_v8.sh
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)

# Ensure theta depends on v8_external
add_dependencies(theta v8_external)

# Create imported target for V8 without INTERFACE_INCLUDE_DIRECTORIES
add_library(v8_monolith STATIC IMPORTED GLOBAL)

set_target_properties(v8_monolith PROPERTIES
    IMPORTED_LOCATION "${V8_DIR}/v8/out.gn/x64.release/obj/libv8_monolith.a"
)

# Create interface library for V8 includes
add_library(v8_includes INTERFACE)
add_dependencies(v8_includes v8_external)

# Use generator expression to set include directories
target_include_directories(v8_includes INTERFACE
    $<BUILD_INTERFACE:${V8_DIR}/include>
)

# Link theta against V8
target_link_libraries(theta v8_monolith v8_includes pthread dl)

# Link Wasmer library
find_library(WASMER_LIB wasmer HINTS ${WASMER_DIR}/lib)
if (WASMER_LIB)
    target_link_libraries(theta ${WASMER_LIB})
else()
    message(FATAL_ERROR "Wasmer library not found in ${WASMER_DIR}/lib")
endif()

# Link Binaryen
target_link_libraries(theta binaryen)

# Create build directories if they don't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test/fixtures)

# Add Catch2 files to the project
set(CATCH2_FILES ${CATCH2_DIR}/catch_amalgamated.cpp)

# Define a target for Catch2 with the main function
add_library(Catch2Main OBJECT ${CATCH2_FILES})
target_compile_definitions(Catch2Main PUBLIC CATCH_CONFIG_MAIN)

# Add test executables
file(GLOB TEST_SRC_FILES "${TEST_DIR}/*.cpp")
foreach(TEST_SRC ${TEST_SRC_FILES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC} ${SRC_FILES} $<TARGET_OBJECTS:Catch2Main>)
    target_link_libraries(${TEST_NAME} ${WASMER_LIB} readline binaryen)
endforeach()

# Custom target to copy fixtures
add_custom_target(copy-fixtures ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/test/fixtures ${CMAKE_BINARY_DIR}/test/fixtures
)

# Install target to create a symlink
install(CODE "execute_process(COMMAND ln -sf ${CMAKE_BINARY_DIR}/theta /usr/local/bin/theta)")
