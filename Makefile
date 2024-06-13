# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -I src -MMD -MP

# Define the source directories
SRC_DIR = src
LEXER_DIR = $(SRC_DIR)/lexer
COMPILER_DIR = $(SRC_DIR)/compiler
PARSER_DIR = $(SRC_DIR)/parser
UTIL_DIR = $(SRC_DIR)/util
AST_DIR = $(PARSER_DIR)/ast

# Define the build directory
BUILD_DIR = build

# Define the build subdirectories
BUILD_LEXER_DIR = $(BUILD_DIR)/$(LEXER_DIR)
BUILD_PARSER_DIR = $(BUILD_DIR)/$(PARSER_DIR)
BUILD_COMPILER_DIR = $(BUILD_DIR)/$(COMPILER_DIR)
BUILD_UTIL_DIR = $(BUILD_DIR)/$(UTIL_DIR)
BUILD_AST_DIR = $(BUILD_DIR)/$(AST_DIR)

# Find all the source and header files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp $(LEXER_DIR)/*.cpp $(PARSER_DIR)/*.cpp $(UTIL_DIR)/*.cpp $(COMPILER_DIR)/*.cpp)
AST_FILES = $(wildcard $(AST_DIR)/*.hpp)

# Add the main file (theta.cpp)
MAIN_SRC = theta.cpp
SRC_FILES += $(MAIN_SRC)

# Define the object files directory
OBJ_FILES = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Define the output executable
TARGET = $(BUILD_DIR)/theta

# Default target
all: $(TARGET)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create all necessary subdirectories
$(BUILD_LEXER_DIR) $(BUILD_PARSER_DIR) $(BUILD_COMPILER_DIR) $(BUILD_UTIL_DIR) $(BUILD_AST_DIR):
	mkdir -p $@

# Link the object files to create the executable
$(TARGET): $(OBJ_FILES) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile the .cpp files to .o files
$(BUILD_DIR)/%.o: %.cpp $(AST_FILES) | $(BUILD_LEXER_DIR) $(BUILD_PARSER_DIR) $(BUILD_COMPILER_DIR) $(BUILD_UTIL_DIR) $(BUILD_AST_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up object files and executable
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean