#pragma once

#include "parser/ast/ASTNode.hpp"
#include "compiler/SymbolTableStack.hpp"

/**
 * @brief Abstract base class for optimization passes in the Theta compiler.
 *
 * Provides mechanisms for walking through an Abstract Syntax Tree (AST) and applying transformations
 * or optimizations to AST nodes.
 */
namespace Theta {
    class OptimizationPass {
    public:
        /**
         * @brief Initiates the optimization process on an AST node.
         *
         * This function orchestrates the optimization of a given AST node and its children,
         * ensuring scope rules are followed and necessary hoisting is performed. It serves as the
         * entry point for the recursive optimization process.
         *
         * @param ast Reference to the shared pointer of the root AST node to be optimized.
         * @param isCapsuleDirectChild Is this ast node the direct child of the capsule node?
         */
        void optimize(shared_ptr<ASTNode> &ast, bool isCapsuleDirectChild = false);

        /**
         * @brief Cleans up and resets scope variables for the pass. Should always be called after the optimization pass finishes
         */
        void cleanup() {
            localScope = SymbolTableStack();
            hoistedScope = SymbolTableStack();
        }

    protected:
        SymbolTableStack localScope;
        SymbolTableStack hoistedScope;

        /**
         * @brief Retrieves an AST node based on an identifier from the available scopes.
         *
         * This method searches the local and hoisted scopes for an identifier and returns the corresponding AST node if found.
         *
         * @param identifier Name of the identifier to lookup in the scopes.
         * @return Shared pointer to the AST node if found, otherwise null.
         */
        shared_ptr<ASTNode> lookupInScope(string identifier);

        /**
         * @brief Generates a unique function identifier based on the function's name and its parameters to handle overloading.
         * 
         * @param variableName The base name of the function.
         * @param declarationNode The function declaration node containing the parameters.
         * @return string The unique identifier for the function.
         */
        string getDeterministicFunctionIdentifier(string variableName, shared_ptr<ASTNode> node);

    private:
        /**
         * @brief Pure virtual function to be implemented by derived classes for performing specific optimizations on the AST.
         *
         * Each derived optimization pass class must implement this method to define its specific optimization behavior.
         *
         * @param ast Reference to the shared pointer of the AST node to be optimized.
         * @param isCapsuleDirectChild Is this ast node the direct child of the capsule node?
         */
        virtual void optimizeAST(shared_ptr<ASTNode> &ast, bool isCapsuleDirectChild = false) = 0;
        
        /**
         * @brief Optionally implemented by derived classes to perform necessary hoisting of declarations to support 
         * correct scope visibility.
         *
         * This method can be overridden to handle the hoisting of variables or functions to higher scopes based on 
         * the specific requirements of the optimization pass.
         *
         * @param ast Reference to the shared pointer of the AST node where hoisting might be necessary.
         */
        virtual void hoistNecessary(shared_ptr<ASTNode> &ast) {}
    };
}
