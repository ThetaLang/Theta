cmake_minimum_required(VERSION 3.10)
project(Theta)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Define source directories
set(SRC_DIR src)
set(LEXER_DIR ${SRC_DIR}/lexer)
set(COMPILER_DIR ${SRC_DIR}/compiler)
set(PARSER_DIR ${SRC_DIR}/parser)
set(UTIL_DIR ${SRC_DIR}/util)
set(AST_DIR ${PARSER_DIR}/ast)
set(TEST_DIR test)
set(CATCH2_DIR ${TEST_DIR}/catch2)

# Define build directories
set(BUILD_DIR build)
set(BUILD_TEST_DIR ${BUILD_DIR}/test)
set(BUILD_FIXTURES_DIR ${BUILD_TEST_DIR}/fixtures)

# Collect all source files except main.cpp
file(GLOB SRC_FILES
    "${SRC_DIR}/*.cpp"
    "${LEXER_DIR}/*.cpp"
    "${PARSER_DIR}/*.cpp"
    "${UTIL_DIR}/*.cpp"
    "${COMPILER_DIR}/*.cpp"
    "${AST_DIR}/*.cpp"  # Add this line if there are .cpp files in the AST directory
)
list(REMOVE_ITEM SRC_FILES "${CMAKE_SOURCE_DIR}/theta.cpp")

# Add main source file separately
set(MAIN_SRC "${CMAKE_SOURCE_DIR}/theta.cpp")

# Collect test source files
file(GLOB TEST_SRC_FILES "${TEST_DIR}/*.cpp")

# Add executable for the main program
add_executable(theta ${SRC_FILES} ${MAIN_SRC})

# Add the readline library
target_link_libraries(theta readline)

# Include directories
include_directories(${SRC_DIR} ${CATCH2_DIR})

# Create build directories if they don't exist
file(MAKE_DIRECTORY ${BUILD_DIR})
file(MAKE_DIRECTORY ${BUILD_TEST_DIR})
file(MAKE_DIRECTORY ${BUILD_FIXTURES_DIR})

# Add Catch2 files to the project
set(CATCH2_FILES ${CATCH2_DIR}/catch_amalgamated.cpp)

# Define a target for Catch2 with main function
add_library(Catch2Main OBJECT ${CATCH2_FILES})
target_compile_definitions(Catch2Main PUBLIC CATCH_CONFIG_MAIN)

# Add test executables
foreach(TEST_SRC ${TEST_SRC_FILES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC} ${SRC_FILES} $<TARGET_OBJECTS:Catch2Main>)
    target_link_libraries(${TEST_NAME} readline)
endforeach()

# Custom target to copy fixtures
add_custom_target(copy-fixtures ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/test/fixtures ${BUILD_FIXTURES_DIR}
)

# Install target to create a symlink
install(CODE "execute_process(COMMAND ln -sf ${CMAKE_BINARY_DIR}/theta /usr/local/bin/theta)")
