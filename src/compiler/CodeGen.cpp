#include "CodeGen.hpp"
#include <memory>

namespace Theta {
    // void CodeGen::gen2() {
    //     cout << "Hello world" << endl;

    //     BinaryenModuleRef module = BinaryenModuleCreate();

    //     // Create a function type for  i32 (i32, i32)
    //     BinaryenType ii[2] = {BinaryenTypeInt32(), BinaryenTypeInt32()};
    //     BinaryenType params = BinaryenTypeCreate(ii, 2);
    //     BinaryenType results = BinaryenTypeInt32();

    //     // Get the 0 and 1 arguments, and add them
    //     BinaryenExpressionRef x = BinaryenLocalGet(module, 0, BinaryenTypeInt32()),
    //                         y = BinaryenLocalGet(module, 1, BinaryenTypeInt32());
    //     BinaryenExpressionRef add = BinaryenBinary(module, BinaryenAddInt32(), x, y);

    //     // Create the add function
    //     // Note: no additional local variables
    //     // Note: no basic blocks here, we are an AST. The function body is just an
    //     // expression node.
    //     BinaryenFunctionRef adder =
    //     BinaryenAddFunction(module, "adder", params, results, NULL, 0, add);

    //     // Print it out
    //     BinaryenModulePrint(module);

    //     // Clean up the module, which owns all the objects we created above
    //     BinaryenModuleDispose(module);
    // }

    BinaryenModuleRef CodeGen::generateWasmFromAST(shared_ptr<ASTNode> ast) {
        BinaryenModuleRef module = BinaryenModuleCreate();

        generate(ast, module);

        return module;
    }

    CodeGen::GenerateResult CodeGen::generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module) {
        if (node->getNodeType() == ASTNode::SOURCE) {
            return generateSource(dynamic_pointer_cast<SourceNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
            return generateBinaryOperation(dynamic_pointer_cast<BinaryOperationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::NUMBER_LITERAL) {
            return generateNumberLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        }

        return nullptr;
    }

    BinaryenExpressionRef CodeGen::generateBinaryOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
        BinaryenOp op;
        if (binOpNode->getOperator() == "+") {
            op = BinaryenAddInt32();
        }

        auto left = generate(binOpNode->getLeft(), module);
        auto right = generate(binOpNode->getRight(), module);

        if (std::holds_alternative<BinaryenLiteral>(left) && std::holds_alternative<BinaryenLiteral>(right)) {
            BinaryenExpressionRef leftTemp = BinaryenConst(module, std::get<BinaryenLiteral>(left));
            BinaryenExpressionRef rightTemp = BinaryenConst(module, std::get<BinaryenLiteral>(right));

            return BinaryenBinary(
                module,
                op,
                leftTemp,
                rightTemp
            );
        } else {
            throw std::runtime_error("Invalid operand types for binary operation");
        }
    }

    BinaryenLiteral CodeGen::generateNumberLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenLiteralInt32(stoi(literalNode->getLiteralValue()));
    }

    int CodeGen::generateSource(shared_ptr<SourceNode> sourceNode, BinaryenModuleRef &module) {
        if (sourceNode->getValue()->getNodeType() != ASTNode::Types::CAPSULE) {
            auto body = generate(sourceNode->getValue(), module);

            if (std::holds_alternative<BinaryenExpressionRef>(body)) {
                BinaryenFunctionRef mainFn = BinaryenAddFunction(
                    module,
                    "main",
                    BinaryenTypeNone(),
                    BinaryenTypeInt32(),
                    NULL,
                    0,
                    std::get<BinaryenExpressionRef>(body)
                );

                BinaryenAddFunctionExport(module, "main", "main");

                return 0;
            } else {
                throw std::runtime_error("Invalid body type for source node");
            }
        } else {
            // generate(sourceNode->getValue(), module);
        }

        return 0;
    }

}
