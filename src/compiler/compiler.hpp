#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream> 
#include <iostream>
#include <memory>
#include <filesystem>
#include "../parser/ast/ast_node.hpp"

using namespace std;

/**
 * @brief Singleton class responsible for compiling Theta source code into an Abstract Syntax Tree (AST).
 */
class ThetaCompiler {
    public:
        /**
         * @brief Compiles the Theta source code starting from the specified entry point.
         * @param entrypoint The entry point file name or identifier.
         */
        void compile(string entrypoint);

        /**
         * @brief Builds the Abstract Syntax Tree (AST) for the Theta source code starting from the specified file.
         * @param file The file name of the Theta source code.
         * @return A shared pointer to the root node of the constructed AST.
         */
        shared_ptr<ASTNode> buildAST(string entrypoint);
        
        /**
         * @brief Gets the singleton instance of the ThetaCompiler.
         * @return Reference to the singleton instance of ThetaCompiler.
         */
        static ThetaCompiler& getInstance();

    private:
        /**
         * @brief Private constructor for ThetaCompiler. Initializes the compiler and discovers all capsules in the source files.
         */
        ThetaCompiler() {
            filesByCapsuleName = make_shared<map<string, string>>();
            discoverCapsules();
        }

        // Delete copy constructor and assignment operator to enforce singleton pattern
        ThetaCompiler(const ThetaCompiler&) = delete;
        ThetaCompiler& operator=(const ThetaCompiler&) = delete;

        shared_ptr<map<string, string>> filesByCapsuleName;

        /**
         * @brief Discovers all capsules in the Theta source code.
         * 
         * Scans the current directory and subdirectories to find all `.th` files and extracts capsule names.
         */
        void discoverCapsules();

        /**
         * @brief Finds the capsule name associated with the given file.
         * 
         * Reads the content of the file and searches for the `capsule` keyword to identify the capsule name.
         * 
         * @param file The file for which to find the capsule name.
         * @return The capsule name corresponding to the file.
         */
        string findCapsuleName(string file);
};