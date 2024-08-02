#pragma once

#include <memory>
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/UnaryOperationNode.hpp"
#include "../parser/ast/LiteralNode.hpp"
#include "../parser/ast/SourceNode.hpp"
#include "compiler/SymbolTableStack.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/AssignmentNode.hpp"
#include "parser/ast/CapsuleNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/ReturnNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include "parser/ast/FunctionInvocationNode.hpp"
#include "parser/ast/ControlFlowNode.hpp"
#include <binaryen-c.h>

using namespace std;

namespace Theta {
    class CodeGen {
        public:
            // using GenerateResult = std::variant<BinaryenExpressionRef, BinaryenLiteral, int>;

            BinaryenModuleRef generateWasmFromAST(shared_ptr<ASTNode> ast);
            BinaryenExpressionRef generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateCapsule(shared_ptr<CapsuleNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateAssignment(shared_ptr<AssignmentNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateBlock(shared_ptr<ASTNodeList> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateReturn(shared_ptr<ReturnNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateFunctionDeclaration(string identifier, shared_ptr<FunctionDeclarationNode> node, BinaryenModuleRef &module, bool addToExports = false);
            BinaryenExpressionRef generateFunctionInvocation(shared_ptr<FunctionInvocationNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateControlFlow(shared_ptr<ControlFlowNode> controlFlowNode, BinaryenModuleRef &module);
            BinaryenExpressionRef generateIdentifier(shared_ptr<IdentifierNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateBinaryOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateUnaryOperation(shared_ptr<UnaryOperationNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateNumberLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateStringLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateBooleanLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            BinaryenExpressionRef generateExponentOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            void generateSource(shared_ptr<SourceNode> node, BinaryenModuleRef &module);

        private:
            SymbolTableStack scope;

            string LOCAL_IDX_SCOPE_KEY = "ThetaLang.internal.localIdxCounter";

            BinaryenExpressionRef generateStringBinaryOperation(string op, BinaryenExpressionRef left, BinaryenExpressionRef right, BinaryenModuleRef &module);
        
            static BinaryenOp getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> node);
            static BinaryenType getBinaryenTypeFromTypeDeclaration(shared_ptr<TypeDeclarationNode> node);

            void hoistCapsuleElements(vector<shared_ptr<ASTNode>> elements);
            void bindIdentifierToScope(shared_ptr<ASTNode> ast);
    };
}
