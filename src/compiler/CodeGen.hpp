#pragma once

#include <memory>
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/UnaryOperationNode.hpp"
#include "../parser/ast/LiteralNode.hpp"
#include "../parser/ast/SourceNode.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/CapsuleNode.hpp"
#include "parser/ast/ReturnNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include <binaryen-c.h>

using namespace std;

namespace Theta {
    class CodeGen {
        public:
            // using GenerateResult = std::variant<BinaryenExpressionRef, BinaryenLiteral, int>;

            static BinaryenModuleRef generateWasmFromAST(shared_ptr<ASTNode> ast);
            static BinaryenExpressionRef generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateCapsule(shared_ptr<CapsuleNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateBlock(shared_ptr<ASTNodeList> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateReturn(shared_ptr<ReturnNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateBinaryOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateUnaryOperation(shared_ptr<UnaryOperationNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateNumberLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateStringLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateBooleanLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateExponentOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            static void generateSource(shared_ptr<SourceNode> node, BinaryenModuleRef &module);

        private:
            static BinaryenOp getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> node);
            static BinaryenType getBinaryenTypeFromTypeDeclaration(shared_ptr<TypeDeclarationNode> node);
    };
}
