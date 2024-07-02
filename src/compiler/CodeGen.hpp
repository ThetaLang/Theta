#pragma once

#include <vector>
#include <deque>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/LiteralNode.hpp"
#include "../parser/ast/SourceNode.hpp"
#include <binaryen-c.h>

using namespace std;

namespace Theta {
    class CodeGen {
        public:
            using GenerateResult = std::variant<BinaryenExpressionRef, BinaryenLiteral, int>;

            static BinaryenModuleRef generateWasmFromAST(shared_ptr<ASTNode> ast);
            static GenerateResult generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module);
            static BinaryenExpressionRef generateBinaryOperation(shared_ptr<BinaryOperationNode> node, BinaryenModuleRef &module);
            static BinaryenLiteral generateNumberLiteral(shared_ptr<LiteralNode> node, BinaryenModuleRef &module);
            static int generateSource(shared_ptr<SourceNode> node, BinaryenModuleRef &module);
    };
}
