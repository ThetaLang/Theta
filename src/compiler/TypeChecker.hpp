#pragma once

#include <vector>
#include <memory>
#include "parser/ast/ASTNode.hpp"
#include "parser/ast/AssignmentNode.hpp"
#include "parser/ast/BinaryOperationNode.hpp"
#include "parser/ast/BlockNode.hpp"
#include "parser/ast/ControlFlowNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include "parser/ast/UnaryOperationNode.hpp"
#include "parser/ast/ListNode.hpp"
#include "parser/ast/CapsuleNode.hpp"
#include "parser/ast/DictionaryNode.hpp"
#include "parser/ast/FunctionInvocationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/StructDeclarationNode.hpp"
#include "parser/ast/StructDefinitionNode.hpp"
#include "parser/ast/TupleNode.hpp"
#include "SymbolTableStack.hpp"

using namespace std;

/**
 * @brief Class responsible for type checking within the Theta programming language. 
 * It ensures that the types of all AST nodes conform to the language's type rules.
 */
namespace Theta {
    class Compiler;

    class TypeChecker {
        public:
            /**
             * @brief Checks the types of all nodes within an AST recursively
             * 
             * @param ast The root node of the AST to be checked.
             * @param bindToScope Optional bindings for initial scope definitions.
             * @return true If type checking succeeds without any errors.
             * @return false If there are any type mismatches or errors.
             */
            bool checkAST(shared_ptr<ASTNode> ast, vector<pair<string, shared_ptr<ASTNode>>> bindToScope = {});
            /**
             * @brief Determines if two AST nodes represent the same type.
             * 
             * @param type1 The first type node. This should be an ASTNode that is a TypeDeclarationNode
             * @param type2 The second type node. This should be an ASTNode that is a TypeDeclarationNode
             * @return true If both nodes represent the same type.
             * @return false Otherwise.
             */
            static bool isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2);

        private:
            SymbolTableStack identifierTable;
            SymbolTableStack capsuleDeclarationsTable;
            
            /**
             * @brief Performs type checking on a single AST node.
             * 
             * @param node The AST node to check.
             * @return true If the node is correctly typed.
             * @return false If there is a type mismatch.
             */
            bool checkNode(shared_ptr<ASTNode> node);

            /**
             * @brief Checks a type declaration node to ensure it represents a valid type.
             * 
             * @param node The type declaration node to check.
             * @return true If the type is valid.
             * @return false If the type is undefined or invalid.
             */
            bool checkTypeDeclarationNode(shared_ptr<TypeDeclarationNode> node);

            /**
             * @brief Checks an assignment node to ensure that the types of the left-hand side and right-hand side match.
             * 
             * @param node The assignment node to check.
             * @return true If the assignment is valid.
             * @return false If the types do not match or if a reassignment was attempted.
             */
            bool checkAssignmentNode(shared_ptr<AssignmentNode> node);

            /**
             * @brief Checks an identifier node to ensure it is defined within the current scope.
             * 
             * @param node The identifier node to check.
             * @return true If the identifier is defined.
             * @return false If the identifier is undefined.
             */
            bool checkIdentifierNode(shared_ptr<IdentifierNode> node);

            /**
             * @brief Checks a binary operation node to ensure the types of the operands match.
             * 
             * @param node The binary operation node to check.
             * @return true If the operands are of the same type.
             * @return false If the operands are of different types.
             */
            bool checkBinaryOperationNode(shared_ptr<BinaryOperationNode> node);

            /**
             * @brief Checks a unary operation node for type correctness based on the operation.
             * 
             * @param node The unary operation node to check.
             * @return true If the operation is valid for the operand's type.
             * @return false Otherwise.
             */
            bool checkUnaryOperationNode(shared_ptr<UnaryOperationNode> node);

            /**
             * @brief Sets the resolvedType of the block to whatever the return values of the block are.
             * Always returns true
             * 
             * @param node The block node to check.
             * @return true Always
             */
            bool checkBlockNode(shared_ptr<BlockNode> node);

            /**
             * @brief Checks a function declaration node to set the resolvedType of the function. Also
             * typechecks the function body statements
             * 
             * @param node The function declaration node to check.
             * @return true If the function body statements are valid
             * @return false Otherwise.
             */
            bool checkFunctionDeclarationNode(shared_ptr<FunctionDeclarationNode> node);

            /**
             * @brief Checks a function invocation node to ensure that the arguments match the parameters of the called function.
             * 
             * @param node The function invocation node to check.
             * @return true If the arguments match the function's parameters.
             * @return false If the referenced function cant be found, or otherwise.
             */
            bool checkFunctionInvocationNode(shared_ptr<FunctionInvocationNode> node);

            /**
             * @brief Checks a control flow node (e.g., if statements) to ensure that the conditions resolve to a boolean.
             * Also checks each conditional's block to ensure type correctness
             *
             * @param node The control flow node to check.
             * @return true If all conditions are boolean.
             * @return false If any condition is not boolean or if any blocks have invalid types in them.
             */
            bool checkControlFlowNode(shared_ptr<ControlFlowNode> node);

            /**
             * @brief Checks a list node to ensure all elements are of the same type.
             * 
             * @param node The list node to check.
             * @return true If all elements are of the same type.
             * @return false If elements are of different types.
             */
            bool checkListNode(shared_ptr<ListNode> node);

            /**
             * @brief Checks a tuple node to ensure the types of its elements are valid.
             * 
             * @param node The tuple node to check.
             * @return true If the tuple's elements are correctly typed.
             * @return false Otherwise.
             */
            bool checkTupleNode(shared_ptr<TupleNode> node);

            /**
             * @brief Checks a dictionary node to ensure that all keys are symbols and all values are of one type.
             * 
             * @param node The dictionary node to check.
             * @return true If the dictionary is correctly typed.
             * @return false If there are type mismatches.
             */
            bool checkDictionaryNode(shared_ptr<DictionaryNode> node);

            /**
             * @brief Checks a struct definition node to ensure it hasn't already been defined, and all types
             * used in the definition exist
             * 
             * @param node The struct definition node to check.
             * @return true If the definition has not been made already and it uses valid types.
             * @return false Otherwise.
             */
            bool checkStructDefinitionNode(shared_ptr<StructDefinitionNode> node);

            /**
             * @brief Checks a struct declaration node to ensure all required fields are present and correctly typed.
             * 
             * @param node The struct declaration node to check.
             * @return true If the struct is correctly declared.
             * @return false If there are missing fields or type mismatches.
             */
            bool checkStructDeclarationNode(shared_ptr<StructDeclarationNode> node);

            /**
             * @brief Processes capsule declarations, hoisting them into the appropriate scope.
             * 
             * @param node The capsule node containing declarations to hoist.
             */
            void hoistCapsuleDeclarations(shared_ptr<CapsuleNode> node);

            /**
             * @brief Hoists function declarations to the appropriate scope to make them globally accessible within the capsule.
             * 
             * @param node The function declaration node to hoist.
             */
            void hoistFunction(shared_ptr<ASTNode> node);

            /**
             * @brief Hoists identifiers (typically variables or constants) to the scope of a capsule.
             * 
             * @param node The identifier node to hoist.
             */
            void hoistIdentifier(shared_ptr<ASTNode> node);

            /**
             * @brief Hoists struct definitions to make them accessible within the capsule scope.
             * 
             * @param node The struct definition node to hoist.
             */
            void hoistStructDefinition(shared_ptr<ASTNode> node);

            /**
             * @brief Looks up an identifier within the current scopes, prioritizing the local scope over the capsule scope.
             * 
             * @param identifier The name of the identifier to look up.
             * @return shared_ptr<ASTNode> The node associated with the identifier, if found; otherwise, nullptr.
             */
            shared_ptr<ASTNode> lookupInScope(string identifier);

            /**
             * @brief Determines if a vector of type declaration nodes is homogenous, i.e., all elements are of the same type.
             * 
             * @param types The vector of type declaration nodes to check.
             * @return true If all types are the same.
             * @return false If there are one or more differing types.
             */
            static bool isHomogenous(vector<shared_ptr<TypeDeclarationNode>> types);

            /**
             * @brief Checks if a given type is a built-in language data type.
             * 
             * @param type The type to check.
             * @return true If the type is a built-in data type.
             * @return false Otherwise.
             */
            static bool isLanguageDataType(string type);

            /**
             * @brief Finds all AST nodes of a specific type within the tree rooted at a given node.
             * 
             * @param node The root node to search from.
             * @param type The type of nodes to find.
             * @return vector<shared_ptr<ASTNode>> A vector of found nodes.
             */
            static vector<shared_ptr<ASTNode>> findAllInTree(shared_ptr<ASTNode> node, ASTNode::Types type);

            /**
             * @brief Creates a variadic type node from a vector of type declaration nodes, useful for functions that can return
             * multiple types.
             * 
             * @param types The vector of type declaration nodes.
             * @return shared_ptr<TypeDeclarationNode> The created variadic type node.
             */
            static shared_ptr<TypeDeclarationNode> makeVariadicType(vector<shared_ptr<TypeDeclarationNode>> types);
    
            /**
             * @brief Generates a unique function identifier based on the function's name and its parameters to handle overloading.
             * 
             * @param variableName The base name of the function.
             * @param declarationNode The function declaration node containing the parameters.
             * @return string The unique identifier for the function.
             */
            static string getDeterministicFunctionIdentifier(string variableName, shared_ptr<ASTNode> declarationNode);

            /**
             * @brief Creates a deep copy of a type declaration node, useful for cases where type information 
             * needs to be duplicated without referencing the original.
             * 
             * @param original The original type declaration node to copy.
             * @return shared_ptr<TypeDeclarationNode> The deep-copied type declaration node.
             */
            static shared_ptr<TypeDeclarationNode> deepCopyTypeDeclaration(shared_ptr<TypeDeclarationNode> node);
    };
}
