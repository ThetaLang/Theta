#include <iostream>
#include <memory>
#include "binaryen-c.h"
#include "lexer/Lexemes.hpp"
#include "StandardLibrary.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include "CodeGen.hpp"
#include "DataTypes.hpp"

namespace Theta {
    BinaryenModuleRef CodeGen::generateWasmFromAST(shared_ptr<ASTNode> ast) {
        BinaryenModuleRef module = BinaryenModuleCreate();

        BinaryenModuleSetFeatures(module, BinaryenFeatureStrings());

        StandardLibrary::registerFunctions(module);

        generate(ast, module);

        BinaryenModuleAutoDrop(module);

        return module;
    }

    BinaryenExpressionRef CodeGen::generate(shared_ptr<ASTNode> node, BinaryenModuleRef &module) {
        if (node->getNodeType() == ASTNode::SOURCE) {
            generateSource(dynamic_pointer_cast<SourceNode>(node), module);
        } else if (node->getNodeType() == ASTNode::CAPSULE) {
            return generateCapsule(dynamic_pointer_cast<CapsuleNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BLOCK) {
            return generateBlock(dynamic_pointer_cast<ASTNodeList>(node), module);
        } else if (node->getNodeType() == ASTNode::RETURN) {
            return generateReturn(dynamic_pointer_cast<ReturnNode>(node), module); 
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

    BinaryenExpressionRef CodeGen::generateCapsule(shared_ptr<CapsuleNode> capsuleNode, BinaryenModuleRef &module) {
        vector<shared_ptr<ASTNode>> capsuleElements = dynamic_pointer_cast<ASTNodeList>(capsuleNode->getValue())->getElements();

        for (auto elem : capsuleElements) {
            string elemType = dynamic_pointer_cast<TypeDeclarationNode>(elem->getResolvedType())->getType();
            if (elem->getNodeType() == ASTNode::ASSIGNMENT) {
                shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(elem->getLeft());

                if (elemType == DataTypes::FUNCTION) {
                    shared_ptr<FunctionDeclarationNode> fnDeclNode = dynamic_pointer_cast<FunctionDeclarationNode>(elem->getRight());


                    string functionName = capsuleNode->getName() + "." + identNode->getIdentifier();
                    
                    cout << "it is" << functionName.c_str() << "  " << functionName.c_str() << endl;

                    BinaryenExpressionRef body = generate(fnDeclNode->getDefinition(), module);

                    BinaryenFunctionRef fn = BinaryenAddFunction(
                        module,
                        functionName.c_str(),
                        BinaryenTypeNone(),
                        getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getResolvedType()->getValue())),
                        NULL,
                        0,
                        body
                    );

                    BinaryenAddFunctionExport(module, functionName.c_str(), functionName.c_str());
                }
            }
        }
    }

    BinaryenExpressionRef CodeGen::generateBlock(shared_ptr<ASTNodeList> blockNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef* blockExpressions = new BinaryenExpressionRef[blockNode->getElements().size()];

        for (int i = 0; i < blockNode->getElements().size(); i++) {
            blockExpressions[i] = generate(blockNode->getElements().at(i), module);
        }

        return BinaryenBlock(
            module,
            NULL,
            blockExpressions,
            blockNode->getElements().size(),
            BinaryenTypeNone()
        );
    }

    BinaryenExpressionRef CodeGen::generateReturn(shared_ptr<ReturnNode> returnNode, BinaryenModuleRef &module) {
        return BinaryenReturn(module, generate(returnNode->getValue(), module));
    }

    BinaryenExpressionRef CodeGen::generateBinaryOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
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
        if (binOpNode->getLeft()->getNodeType() == ASTNode::STRING_LITERAL) {
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

    BinaryenExpressionRef CodeGen::generateUnaryOperation(shared_ptr<UnaryOperationNode> unaryOpNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef binaryenVal = generate(unaryOpNode->getValue(), module);

        if (!binaryenVal) {
            throw runtime_error("Invalid operand type for unary operation");
        }

        if (unaryOpNode->getOperator() == Lexemes::NOT) {
            return BinaryenUnary(module, BinaryenEqZInt64(), binaryenVal);
        }

        // Must be a negative. Multiply by negative 1
        return BinaryenBinary(
            module,
            BinaryenMulInt64(),
            binaryenVal,
            BinaryenConst(module, BinaryenLiteralInt64(-1))
        );
    }

    BinaryenExpressionRef CodeGen::generateNumberLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenConst(
            module,
            BinaryenLiteralInt64(stoi(literalNode->getLiteralValue()))
        );
    }

    BinaryenExpressionRef CodeGen::generateStringLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenStringConst(module, literalNode->getLiteralValue().c_str());
    }

    BinaryenExpressionRef CodeGen::generateBooleanLiteral(shared_ptr<LiteralNode> literalNode, BinaryenModuleRef &module) {
        return BinaryenConst(
            module,
            BinaryenLiteralInt32(literalNode->getLiteralValue() == "true" ? 1 : 0)
        );
    }

    BinaryenExpressionRef CodeGen::generateExponentOperation(shared_ptr<BinaryOperationNode> binOpNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef binaryenLeft = generate(binOpNode->getLeft(), module);
        BinaryenExpressionRef binaryenRight = generate(binOpNode->getRight(), module);

        if (!binaryenLeft || !binaryenRight) {
            throw runtime_error("Invalid operand types for binary operation");
        }

        return BinaryenCall(
            module,
            "Theta.Math.pow",
            (BinaryenExpressionRef[]){ binaryenLeft, binaryenRight },
            2,
            BinaryenTypeInt64()
        );
    }

    void CodeGen::generateSource(shared_ptr<SourceNode> sourceNode, BinaryenModuleRef &module) {
        if (sourceNode->getValue()->getNodeType() != ASTNode::CAPSULE) {
            BinaryenExpressionRef body = generate(sourceNode->getValue(), module);

            if (!body) {
                throw runtime_error("Invalid body type for source node");
            }

            shared_ptr<TypeDeclarationNode> returnType = dynamic_pointer_cast<TypeDeclarationNode>(sourceNode->getValue()->getResolvedType());

            BinaryenFunctionRef mainFn = BinaryenAddFunction(
                module,
                "main",
                BinaryenTypeNone(),
                getBinaryenTypeFromTypeDeclaration(returnType),
                NULL,
                0,
                body
            );

            BinaryenAddFunctionExport(module, "main", "main");
        } else {
            generate(sourceNode->getValue(), module);
        }
    }

    BinaryenOp CodeGen::getBinaryenOpFromBinOpNode(shared_ptr<BinaryOperationNode> binOpNode) {
        string op = binOpNode->getOperator();
        if (op == Lexemes::PLUS) return BinaryenAddInt64();
        if (op == Lexemes::MINUS) return BinaryenSubInt64();
        if (op == Lexemes::DIVISION) return BinaryenDivSInt64();
        if (op == Lexemes::TIMES) return BinaryenMulInt64();
        if (op == Lexemes::MODULO) return BinaryenRemSInt64();

        string resolvedType = dynamic_pointer_cast<TypeDeclarationNode>(binOpNode->getLeft()->getResolvedType())->getType();

        if (op == Lexemes::EQUALITY && resolvedType == DataTypes::NUMBER) return BinaryenEqInt64();
        if (op == Lexemes::EQUALITY && resolvedType == DataTypes::BOOLEAN) return BinaryenEqInt32();
        if (op == Lexemes::EQUALITY && resolvedType == DataTypes::STRING) return BinaryenStringEqEqual();
        if (op == Lexemes::INEQUALITY && resolvedType == DataTypes::NUMBER) return BinaryenNeInt64();
        if (op == Lexemes::INEQUALITY && resolvedType == DataTypes::BOOLEAN) return BinaryenEqInt32();
        if (op == Lexemes::INEQUALITY && resolvedType == DataTypes::STRING) return BinaryenStringEqEqual(); // FIXME: This is a stub
        if (op == Lexemes::LT && resolvedType == DataTypes::NUMBER) return BinaryenLtSInt64();
        if (op == Lexemes::GT && resolvedType == DataTypes::NUMBER) return BinaryenGtSInt64();


        // if (op == "**") return
    }

    BinaryenType CodeGen::getBinaryenTypeFromTypeDeclaration(shared_ptr<TypeDeclarationNode> typeDeclaration) {
        if (typeDeclaration->getType() == DataTypes::NUMBER) return BinaryenTypeInt64();
        if (typeDeclaration->getType() == DataTypes::STRING) return BinaryenTypeStringref();
        if (typeDeclaration->getType() == DataTypes::BOOLEAN) return BinaryenTypeInt32();
    }
}
