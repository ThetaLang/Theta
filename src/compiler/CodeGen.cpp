#include "CodeGen.hpp"

namespace Theta {
    BinaryenModuleRef CodeGen::generateWasmFromAST(shared_ptr<ASTNode> ast) {
        BinaryenModuleRef module = BinaryenModuleCreate();

        BinaryenModuleSetFeatures(module, BinaryenFeatureStrings());

        StandardLibrary::registerFunctions(module);

        generate(ast, module);

        return module;
    }

    BinaryenExpressionRef CodeGen::generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module) {
        if (node->getNodeType() == ASTNode::SOURCE) {
            generateSource(dynamic_pointer_cast<SourceNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
            return generateBinaryOperation(dynamic_pointer_cast<BinaryOperationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::UNARY_OPERATION) {
            return generateUnaryOperation(dynamic_pointer_cast<UnaryOperationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::NUMBER_LITERAL) {
            return generateNumberLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        } else if (node->getNodeType() == ASTNode::STRING_LITERAL) {
            return generateStringLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BOOLEAN_LITERAL) {
            return generateBooleanLiteral(dynamic_pointer_cast<LiteralNode>(node), module);
        }

        return nullptr;
    }

    BinaryenExpressionRef CodeGen::generateBinaryOperation(shared_ptr<BinaryOperationNode> binOpNode,
                                                           BinaryenModuleRef &module) {
        if (binOpNode->getOperator() == Lexemes::EXPONENT) {
            return generateExponentOperation(binOpNode, module);
        }

        BinaryenOp op = getBinaryenOpFromBinOpNode(binOpNode);

        BinaryenExpressionRef binaryenLeft = generate(binOpNode->getLeft(), module);
        BinaryenExpressionRef binaryenRight = generate(binOpNode->getRight(), module);

        if (!binaryenLeft || !binaryenRight) {
            throw runtime_error("Invalid operand types for binary operation");
        }

        // TODO: This wont work if we have nested operations on either side
        if (binOpNode->getLeft()->getNodeType() == ASTNode::Types::STRING_LITERAL) {
            return BinaryenStringConcat(module, binaryenLeft, binaryenRight);
        }

        return BinaryenBinary(module, op, binaryenLeft, binaryenRight);
    }

    BinaryenExpressionRef CodeGen::generateUnaryOperation(shared_ptr<UnaryOperationNode> unaryOpNode,
                                                          BinaryenModuleRef &module) {
        BinaryenExpressionRef binaryenVal = generate(unaryOpNode->getValue(), module);

        if (!binaryenVal) {
            throw runtime_error("Invalid operand type for unary operation");
        }

        if (unaryOpNode->getOperator() == Lexemes::NOT) {
            return BinaryenUnary(module, BinaryenEqZInt64(), binaryenVal);
        }

        // Must be a negative. Multiply by negative 1
        return BinaryenBinary(module, BinaryenMulInt64(), binaryenVal, BinaryenConst(module, BinaryenLiteralInt64(-1)));
    }

    BinaryenExpressionRef CodeGen::generateNumberLiteral(shared_ptr<LiteralNode> literalNode,
                                                         BinaryenModuleRef &module) {
        return BinaryenConst(module, BinaryenLiteralInt64(stoi(literalNode->getLiteralValue())));
    }

    BinaryenExpressionRef CodeGen::generateStringLiteral(shared_ptr<LiteralNode> literalNode,
                                                         BinaryenModuleRef &module) {
        return BinaryenStringConst(module, literalNode->getLiteralValue().c_str());
    }

    BinaryenExpressionRef CodeGen::generateBooleanLiteral(shared_ptr<LiteralNode> literalNode,
                                                          BinaryenModuleRef &module) {
        return BinaryenConst(module, BinaryenLiteralInt32(literalNode->getLiteralValue() == "true" ? 1 : 0));
    }

    BinaryenExpressionRef CodeGen::generateExponentOperation(shared_ptr<BinaryOperationNode> binOpNode,
                                                             BinaryenModuleRef &module) {
        BinaryenExpressionRef binaryenLeft = generate(binOpNode->getLeft(), module);
        BinaryenExpressionRef binaryenRight = generate(binOpNode->getRight(), module);

        if (!binaryenLeft || !binaryenRight) {
            throw runtime_error("Invalid operand types for binary operation");
        }

        return BinaryenCall(
            module, "Theta.Math.pow", (BinaryenExpressionRef[]){binaryenLeft, binaryenRight}, 2, BinaryenTypeInt64());
    }

    void CodeGen::generateSource(shared_ptr<SourceNode> sourceNode, BinaryenModuleRef &module) {
        if (sourceNode->getValue()->getNodeType() != ASTNode::Types::CAPSULE) {
            BinaryenExpressionRef body = generate(sourceNode->getValue(), module);

            if (!body) {
                throw std::runtime_error("Invalid body type for source node");
            }

            BinaryenFunctionRef mainFn = BinaryenAddFunction(module,
                                                             "main",
                                                             BinaryenTypeNone(),
                                                             // BinaryenTypeStringref(),
                                                             // BinaryenTypeInt64(),
                                                             BinaryenTypeInt32(),
                                                             NULL,
                                                             0,
                                                             body);

            BinaryenAddFunctionExport(module, "main", "main");
        } else {
            // generate(sourceNode->getValue(), module);
        }
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
