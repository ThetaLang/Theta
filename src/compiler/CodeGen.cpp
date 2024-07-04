#include "CodeGen.hpp"

namespace Theta {
    BinaryenModuleRef CodeGen::generateWasmFromAST(shared_ptr<ASTNode> ast) {
        BinaryenModuleRef module = BinaryenModuleCreate();

        BinaryenModuleSetFeatures(module, BinaryenFeatureStrings());

        StandardLibrary::registerFunctions(module);

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
        } else if (node->getNodeType() == ASTNode::STRING_LITERAL) {
            return generateStringLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        }

        return nullptr;
    }

    BinaryenExpressionRef CodeGen::generateBinaryOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
        if (binOpNode->getOperator() == Lexemes::EXPONENT) {
            return generateExponentOperation(binOpNode, module);
        }

        BinaryenOp op = getBinaryenOpFromBinOpNode(binOpNode);

        auto left = generate(binOpNode->getLeft(), module);
        auto right = generate(binOpNode->getRight(), module);

        BinaryenExpressionRef binaryenLeft;
        BinaryenExpressionRef binaryenRight;

        if (std::holds_alternative<BinaryenLiteral>(left)) {
            binaryenLeft = BinaryenConst(module, std::get<BinaryenLiteral>(left));
        } else if (std::holds_alternative<BinaryenExpressionRef>(left)) {
            binaryenLeft = std::get<BinaryenExpressionRef>(left);
        }

        if (std::holds_alternative<BinaryenLiteral>(right)) {
            binaryenRight = BinaryenConst(module, std::get<BinaryenLiteral>(right));
        } else if (std::holds_alternative<BinaryenExpressionRef>(right)) {
            binaryenRight =  std::get<BinaryenExpressionRef>(right);
        }

        if (!binaryenLeft || !binaryenRight) {
            throw std::runtime_error("Invalid operand types for binary operation");
        }

        // TODO: This wont work if we have nested operations on either side
        if (binOpNode->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL) {
            return BinaryenStringConcat(
                module,
                binaryenLeft,
                binaryenRight
            );
        }

        return BinaryenBinary(
            module,
            op,
            binaryenLeft,
            binaryenRight
        );
    }

    BinaryenLiteral CodeGen::generateNumberLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenLiteralInt64(stoi(literalNode->getLiteralValue()));
    }

    BinaryenExpressionRef CodeGen::generateStringLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenStringConst(module, literalNode->getLiteralValue().c_str());
    }

    BinaryenExpressionRef CodeGen::generateExponentOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
        auto left = generate(binOpNode->getLeft(), module);
        auto right = generate(binOpNode->getRight(), module);

        BinaryenExpressionRef binaryenLeft;
        BinaryenExpressionRef binaryenRight;

        if (std::holds_alternative<BinaryenLiteral>(left)) {
            binaryenLeft = BinaryenConst(module, std::get<BinaryenLiteral>(left));
        } else if (std::holds_alternative<BinaryenExpressionRef>(left)) {
            binaryenLeft = std::get<BinaryenExpressionRef>(left);
        }

        if (std::holds_alternative<BinaryenLiteral>(right)) {
            binaryenRight = BinaryenConst(module, std::get<BinaryenLiteral>(right));
        } else if (std::holds_alternative<BinaryenExpressionRef>(right)) {
            binaryenRight =  std::get<BinaryenExpressionRef>(right);
        }

        if (!binaryenLeft || !binaryenRight) {
            throw std::runtime_error("Invalid operand types for binary operation");
        }

        return BinaryenCall(
            module,
            "Theta.Math.pow",
            (BinaryenExpressionRef[]){ binaryenLeft, binaryenRight },
            2,
            BinaryenTypeInt64()
        );
    }

    int CodeGen::generateSource(shared_ptr<SourceNode> sourceNode, BinaryenModuleRef &module) {
        if (sourceNode->getValue()->getNodeType() != ASTNode::Types::CAPSULE) {
            auto body = generate(sourceNode->getValue(), module);

            if (std::holds_alternative<BinaryenExpressionRef>(body)) {
                BinaryenFunctionRef mainFn = BinaryenAddFunction(
                    module,
                    "main",
                    BinaryenTypeNone(),
                    // BinaryenTypeStringref(),
                    BinaryenTypeInt64(),
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









    BinaryenOp CodeGen::getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> binOpNode) {
        string op = binOpNode->getOperator();
        if (op == Lexemes::PLUS) return BinaryenAddInt64();
        if (op == Lexemes::MINUS) return BinaryenSubInt64();
        if (op == Lexemes::DIVISION) return BinaryenDivSInt64();
        if (op == Lexemes::TIMES) return BinaryenMulInt64();
        if (op == Lexemes::MODULO) return BinaryenRemSInt64();
        // if (op == "**") return
    }
}
