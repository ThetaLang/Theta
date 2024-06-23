# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -I src -I test/catch2 -MMD -MP

# Define the source directories
SRC_DIR = src
LEXER_DIR = $(SRC_DIR)/lexer
COMPILER_DIR = $(SRC_DIR)/compiler
PARSER_DIR = $(SRC_DIR)/parser
UTIL_DIR = $(SRC_DIR)/util
AST_DIR = $(PARSER_DIR)/ast

# Define the build directories
BUILD_DIR = build
BUILD_TEST_DIR = $(BUILD_DIR)/test
BUILD_FIXTURES_DIR = $(BUILD_TEST_DIR)/fixtures

# Define the build subdirectories
BUILD_LEXER_DIR = $(BUILD_DIR)/$(LEXER_DIR)
BUILD_PARSER_DIR = $(BUILD_DIR)/$(PARSER_DIR)
BUILD_COMPILER_DIR = $(BUILD_DIR)/$(COMPILER_DIR)
BUILD_UTIL_DIR = $(BUILD_DIR)/$(UTIL_DIR)
BUILD_AST_DIR = $(BUILD_DIR)/$(AST_DIR)

BUILD_TEST_LEXER_DIR = $(BUILD_TEST_DIR)/$(LEXER_DIR)
BUILD_TEST_PARSER_DIR = $(BUILD_TEST_DIR)/$(PARSER_DIR)
BUILD_TEST_COMPILER_DIR = $(BUILD_TEST_DIR)/$(COMPILER_DIR)
BUILD_TEST_UTIL_DIR = $(BUILD_TEST_DIR)/$(UTIL_DIR)
BUILD_TEST_AST_DIR = $(BUILD_TEST_DIR)/$(AST_DIR)

# Find all the source and header files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp $(LEXER_DIR)/*.cpp $(PARSER_DIR)/*.cpp $(UTIL_DIR)/*.cpp $(COMPILER_DIR)/*.cpp)
AST_FILES = $(wildcard $(AST_DIR)/*.hpp)

# Add the main file (theta.cpp)
MAIN_SRC = theta.cpp
SRC_FILES += $(MAIN_SRC)

# Define the object files
OBJ_FILES = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(filter-out $(MAIN_SRC),$(SRC_FILES)))

# Define the main executable
TARGET = $(BUILD_DIR)/theta

# Define test source files and their corresponding object files
TEST_DIR = test
TEST_SRC_FILES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ_FILES = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_TEST_DIR)/%.o,$(TEST_SRC_FILES))
TEST_BINARIES = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_TEST_DIR)/%,$(TEST_SRC_FILES))

# Catch2 source files
CATCH2_DIR = $(TEST_DIR)/catch2
CATCH2_SRC_FILES = $(CATCH2_DIR)/catch_amalgamated.cpp
CATCH2_OBJ_FILES = $(patsubst $(CATCH2_DIR)/%.cpp,$(BUILD_TEST_DIR)/%.o,$(CATCH2_SRC_FILES))

TARGET_EXECUTABLE = $(PWD)/$(TARGET)
SYMLINK_PATH = /usr/local/bin/theta

install: all
	@echo "Creating symlink $(SYMLINK_PATH) -> $(TARGET_EXECUTABLE)"
	ln -sf $(TARGET_EXECUTABLE) $(SYMLINK_PATH)

# Default target
all: $(TARGET)

# Create the build directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_LEXER_DIR) $(BUILD_PARSER_DIR) $(BUILD_COMPILER_DIR) $(BUILD_UTIL_DIR) $(BUILD_AST_DIR) $(BUILD_TEST_DIR) $(BUILD_FIXTURES_DIR) $(BUILD_TEST_LEXER_DIR) $(BUILD_TEST_PARSER_DIR) $(BUILD_TEST_COMPILER_DIR) $(BUILD_TEST_UTIL_DIR) $(BUILD_TEST_AST_DIR):
	mkdir -p $@

# Link the main executable
$(TARGET): $(OBJ_FILES) $(BUILD_DIR)/$(MAIN_SRC:.cpp=.o) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lreadline

# Compile the .cpp files to .o files
$(BUILD_DIR)/%.o: %.cpp $(AST_FILES) | $(BUILD_LEXER_DIR) $(BUILD_PARSER_DIR) $(BUILD_COMPILER_DIR) $(BUILD_UTIL_DIR) $(BUILD_AST_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile Catch2 source files
$(BUILD_TEST_DIR)/%.o: $(CATCH2_DIR)/%.cpp | $(BUILD_TEST_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Compile test files
$(BUILD_TEST_DIR)/%.o: $(TEST_DIR)/%.cpp $(CATCH2_OBJ_FILES) | $(BUILD_TEST_DIR) $(BUILD_TEST_LEXER_DIR) $(BUILD_TEST_PARSER_DIR) $(BUILD_TEST_COMPILER_DIR) $(BUILD_TEST_UTIL_DIR) $(BUILD_TEST_AST_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build test binaries
$(BUILD_TEST_DIR)/%: $(BUILD_TEST_DIR)/%.o $(OBJ_FILES) $(CATCH2_OBJ_FILES) | $(BUILD_TEST_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Test target depends on all test binaries
test: $(TEST_BINARIES) copy-fixtures

# Copy .th files to build/test/fixtures
copy-fixtures: $(BUILD_FIXTURES_DIR)
	cp test/fixtures/*.th $(BUILD_FIXTURES_DIR)/

# Clean up object files, executables, and build directories
clean:
	rm -rf $(BUILD_DIR) $(BUILD_TEST_DIR)

# Phony targets
.PHONY: all clean test copy-fixtures
