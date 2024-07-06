#pragma once

#include <binaryen-c.h>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../lexer/Lexemes.hpp"
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/LiteralNode.hpp"
#include "../parser/ast/SourceNode.hpp"
#include "../parser/ast/UnaryOperationNode.hpp"
#include "StandardLibrary.hpp"

using namespace std;

namespace Theta {
    class CodeGen {
    public:
        // using GenerateResult = std::variant<BinaryenExpressionRef, BinaryenLiteral, int>;

        static BinaryenModuleRef generateWasmFromAST(shared_ptr<ASTNode> ast);
        static BinaryenExpressionRef generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module);
        static BinaryenExpressionRef generateBinaryOperation(shared_ptr<BinaryOperationNode> node,
                                                             BinaryenModuleRef &module);
        static BinaryenExpressionRef generateUnaryOperation(shared_ptr<UnaryOperationNode> node,
                                                            BinaryenModuleRef &module);
        static BinaryenExpressionRef generateNumberLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
        static BinaryenExpressionRef generateStringLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
        static BinaryenExpressionRef generateBooleanLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
        static BinaryenExpressionRef generateExponentOperation(shared_ptr<BinaryOperationNode> node,
                                                               BinaryenModuleRef &module);
        static void generateSource(shared_ptr<SourceNode> node, BinaryenModuleRef &module);

    private:
        static BinaryenOp getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> node);
    };
}
