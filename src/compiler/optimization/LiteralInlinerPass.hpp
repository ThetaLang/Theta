#pragma once

#include "OptimizationPass.hpp"
#include "parser/ast/ASTNode.hpp"
#include "compiler/SymbolTableStack.hpp"
#include <memory>

using namespace std;

/**
 * @brief A specific optimization pass that inlines literal values, replacing variable references
 * with their corresponding literal values when possible. It also unpacks enums into constants and
 * replaces enum references wherever applicable, and removes the enum nodes entirely.
 *
 * This optimization aims to improve both compile-time and runtime efficiency by reducing the need
 * for variable lookups and potential runtime evaluations.
 */
namespace Theta {
    class LiteralInlinerPass : public OptimizationPass {
    private:
        /**
         * @brief Processes different types of nodes such as identifiers, enums, and assignments,
         * substituting them with their literal values when applicable.
         *
         * @param ast Reference to the shared pointer of the AST node being optimized.
         */
        void optimizeAST(shared_ptr<ASTNode> &ast, bool isCapsuleDirectChild) override;

        /**
         * @brief Hoists capsule-level definitions before traversing the ast
         *
         * @param ast Reference to the shared pointer of the AST node where hoisting is necessary.
         */
        void hoistNecessary(shared_ptr<ASTNode> &ast) override;

        /**
         * @brief Substitutes identifiers in the AST with their literal values if available.
         *
         * @param ast Reference to the shared pointer of the AST node to be substituted, if applicable.
         */
        void substituteIdentifiers(shared_ptr<ASTNode> &ast);

        /**
         * @brief Binds an identifier to its value in the given scope.
         *
         * This method is used during the optimization process to bind literals directly to identifiers
         * in the current scope to facilitate faster access during subsequent operations. This way, when
         * an identifier is enountered later in the traversal, we have a record of its value.
         *
         * @param ast Reference to the shared pointer of the AST node representing an assignment.
         * @param scope Reference to the symbol table stack where the identifier will be bound.
         */
        void bindIdentifierToScope(shared_ptr<ASTNode> &ast, SymbolTableStack<shared_ptr<ASTNode>> &scope);

        /**
         * @brief Unpacks enum elements and adds them to the given scope.
         *
         * This method processes enum nodes by hoisting their values into the scope to make them
         * available as constants throughout the rest of the AST.
         *
         * @param ast Reference to the shared pointer of the AST node representing an enum.
         * @param scope Reference to the symbol table stack where the enum elements will be inserted.
         */
        void unpackEnumElementsInScope(shared_ptr<ASTNode> ast, SymbolTableStack<shared_ptr<ASTNode>> &scope);

        /**
         * @brief Remaps type references for enums by looking up the actual types from the scope. It basically
         * just checks if the given type declaration is a reference to an enum, and then changes the mapping to
         * be a Number instead.
         *
         * @param ast Reference to the shared pointer of the TypeDeclarationNode to be remapped.
         */
        void remapEnumTypeReferences(shared_ptr<ASTNode> &ast);

        /**
         * @brief Checks whether or not the given ast is an assignment of a literal
         *
         * @param ast The ast to check
         * @return true If the node is an assignment node and the type of the LHS and RHS are both literal types
         * @return false Otherwise
         */
        bool isLiteralAssignment(shared_ptr<ASTNode> ast);
    };
}
