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
#include "parser/ast/EnumNode.hpp"
#include "SymbolTableStack.hpp"

using namespace std;

namespace Theta {
    class Compiler;

    class TypeChecker {
        public:
            bool checkAST(shared_ptr<ASTNode> ast);

            static bool isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2);

        private:
            SymbolTableStack identifierTable;
            SymbolTableStack capsuleDeclarationsTable;
        
            bool checkNode(shared_ptr<ASTNode> node);

            bool checkTypeDeclarationNode(shared_ptr<TypeDeclarationNode> node);

            bool checkAssignmentNode(shared_ptr<AssignmentNode> node);

            bool checkIdentifierNode(shared_ptr<IdentifierNode> node);

            bool checkBinaryOperationNode(shared_ptr<BinaryOperationNode> node);

            bool checkUnaryOperationNode(shared_ptr<UnaryOperationNode> node);

            bool checkBlockNode(shared_ptr<BlockNode> node);

            bool checkFunctionDeclarationNode(shared_ptr<FunctionDeclarationNode> node);

            bool checkFunctionInvocationNode(shared_ptr<FunctionInvocationNode> node);

            bool checkControlFlowNode(shared_ptr<ControlFlowNode> node);

            bool checkListNode(shared_ptr<ListNode> node);

            bool checkTupleNode(shared_ptr<TupleNode> node);

            bool checkDictionaryNode(shared_ptr<DictionaryNode> node);

            bool checkStructDefinitionNode(shared_ptr<StructDefinitionNode> node);

            bool checkStructDeclarationNode(shared_ptr<StructDeclarationNode> node);

            bool checkEnumNode(shared_ptr<EnumNode> node);

            void hoistCapsuleDeclarations(shared_ptr<CapsuleNode> node);

            void hoistFunction(shared_ptr<ASTNode> node);

            void hoistIdentifier(shared_ptr<ASTNode> node);

            static bool isHomogenous(vector<shared_ptr<TypeDeclarationNode>> types);

            static bool isLanguageDataType(string type);

            static vector<shared_ptr<ASTNode>> findAllInTree(shared_ptr<ASTNode> node, ASTNode::Types type);

            static shared_ptr<TypeDeclarationNode> makeVariadicType(vector<shared_ptr<TypeDeclarationNode>> types);
    
            static string getDeterministicFunctionIdentifier(string variableName, shared_ptr<ASTNode> declarationNode);

            static shared_ptr<TypeDeclarationNode> deepCopyTypeDeclaration(shared_ptr<TypeDeclarationNode> node);
    };
}
