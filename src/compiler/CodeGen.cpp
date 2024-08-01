#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "binaryen-c.h"
#include "compiler/Compiler.hpp"
#include "lexer/Lexemes.hpp"
#include "StandardLibrary.hpp"
#include "CodeGen.hpp"
#include "DataTypes.hpp"
#include "parser/ast/AssignmentNode.hpp"

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
        if (node->hasOwnScope()) scope.enterScope();

        if (node->getNodeType() == ASTNode::SOURCE) {
            generateSource(dynamic_pointer_cast<SourceNode>(node), module);
        } else if (node->getNodeType() == ASTNode::CAPSULE) {
            return generateCapsule(dynamic_pointer_cast<CapsuleNode>(node), module);
        } else if (node->getNodeType() == ASTNode::ASSIGNMENT) {
            return generateAssignment(dynamic_pointer_cast<AssignmentNode>(node), module);
        } else if (node->getNodeType() == ASTNode::BLOCK) {
            return generateBlock(dynamic_pointer_cast<ASTNodeList>(node), module);
        } else if (node->getNodeType() == ASTNode::RETURN) {
            return generateReturn(dynamic_pointer_cast<ReturnNode>(node), module); 
        } else if (node->getNodeType() == ASTNode::FUNCTION_INVOCATION) {
            return generateFunctionInvocation(dynamic_pointer_cast<FunctionInvocationNode>(node), module);
        } else if (node->getNodeType() == ASTNode::IDENTIFIER) {
            return generateIdentifier(dynamic_pointer_cast<IdentifierNode>(node), module);
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

        if (node->hasOwnScope()) scope.exitScope();

        return nullptr;
    }

    BinaryenExpressionRef CodeGen::generateCapsule(shared_ptr<CapsuleNode> capsuleNode, BinaryenModuleRef &module) {
        vector<shared_ptr<ASTNode>> capsuleElements = dynamic_pointer_cast<ASTNodeList>(capsuleNode->getValue())->getElements();

        hoistCapsuleElements(capsuleElements);

        for (auto elem : capsuleElements) {
            string elemType = dynamic_pointer_cast<TypeDeclarationNode>(elem->getResolvedType())->getType();
            if (elem->getNodeType() == ASTNode::ASSIGNMENT) {
                shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(elem->getLeft());

                if (elemType == DataTypes::FUNCTION) {
                    generateFunctionDeclaration(
                        identNode->getIdentifier(),
                        dynamic_pointer_cast<FunctionDeclarationNode>(elem->getRight()),
                        module,
                        true
                    );
                }
            }
        }
    }

    BinaryenExpressionRef CodeGen::generateAssignment(shared_ptr<AssignmentNode> assignmentNode, BinaryenModuleRef &module) {
        string assignmentIdentifier = dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier();

        if (assignmentNode->getRight()->getNodeType() != ASTNode::FUNCTION_DECLARATION) {
            // Using a space in scope for an idx counter so we dont have to have a whole separate stack just to keep track of the current
            // local idx
            shared_ptr<LiteralNode> currentIdentIdx = dynamic_pointer_cast<LiteralNode>(scope.lookup(LOCAL_IDX_SCOPE_KEY));
            int idxOfAssignment = stoi(currentIdentIdx->getLiteralValue());

            currentIdentIdx->setLiteralValue(to_string(idxOfAssignment + 1));
            scope.insert(LOCAL_IDX_SCOPE_KEY, currentIdentIdx);

            shared_ptr<ASTNode> assignmentRhs = assignmentNode->getRight();
            assignmentRhs->setMappedBinaryenIndex(idxOfAssignment);
            scope.insert(assignmentIdentifier, assignmentRhs);

            return BinaryenLocalSet(
                module,
                idxOfAssignment,
                generate(assignmentRhs, module)
            );
        }

        // TODO: Functions will be defined as closures which take in the scope of the surrounding block as additional parameters
        throw new runtime_error("Lambda functions are not yet implemented.");
    }

    BinaryenExpressionRef CodeGen::generateFunctionDeclaration(
        string identifier,
        shared_ptr<FunctionDeclarationNode> fnDeclNode,
        BinaryenModuleRef &module,
        bool addToExports
    ) {
        scope.enterScope();
        BinaryenType parameterType = BinaryenTypeNone();
        int totalParams = fnDeclNode->getParameters()->getElements().size();

        scope.insert(LOCAL_IDX_SCOPE_KEY, make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, to_string(totalParams)));

        if (totalParams > 0) {
            BinaryenType* types = new BinaryenType[totalParams];

            for (int i = 0; i < totalParams; i++) {
                shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(fnDeclNode->getParameters()->getElements().at(i));

                identNode->setMappedBinaryenIndex(i);

                scope.insert(identNode->getIdentifier(), identNode);
                types[i] = getBinaryenTypeFromTypeDeclaration(

                    dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getParameters()->getElements().at(i)->getValue())
                );
            }

            parameterType = BinaryenTypeCreate(types, totalParams);
        }

        vector<shared_ptr<ASTNode>> localVariables = Compiler::findAllInTree(fnDeclNode->getDefinition(), ASTNode::ASSIGNMENT);

        BinaryenType* localVariableTypes = new BinaryenType[localVariables.size()];
        for (int i = 0; i < localVariables.size(); i++) {
            localVariableTypes[i] = getBinaryenTypeFromTypeDeclaration(
                dynamic_pointer_cast<TypeDeclarationNode>(localVariables.at(i)->getResolvedType())
            );
        }

        string functionName = Compiler::getQualifiedFunctionIdentifier(
            identifier,
            dynamic_pointer_cast<ASTNode>(fnDeclNode)
        );

        BinaryenFunctionRef fn = BinaryenAddFunction(
            module,
            functionName.c_str(),
            parameterType,
            getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getResolvedType()->getValue())),
            localVariableTypes,
            localVariables.size(),
            generate(fnDeclNode->getDefinition(), module)
        );

        if (addToExports) {
            BinaryenAddFunctionExport(module, functionName.c_str(), functionName.c_str());
        }

        scope.exitScope();
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

    BinaryenExpressionRef CodeGen::generateFunctionInvocation(shared_ptr<FunctionInvocationNode> funcInvNode, BinaryenModuleRef &module) {
        BinaryenExpressionRef* arguments = new BinaryenExpressionRef[funcInvNode->getParameters()->getElements().size()];

        string funcName = Compiler::getQualifiedFunctionIdentifier(
            dynamic_pointer_cast<IdentifierNode>(funcInvNode->getIdentifier())->getIdentifier(),
            funcInvNode
        );
    
        for (int i = 0; i < funcInvNode->getParameters()->getElements().size(); i++) {
            arguments[i] = generate(funcInvNode->getParameters()->getElements().at(i), module);
        }

        return BinaryenCall(
            module,
            funcName.c_str(),
            arguments,
            funcInvNode->getParameters()->getElements().size(),
            getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(funcInvNode->getResolvedType()))
        );
    }

    BinaryenExpressionRef CodeGen::generateIdentifier(shared_ptr<IdentifierNode> identNode, BinaryenModuleRef &module) {
        shared_ptr<ASTNode> identInScope = scope.lookup(identNode->getIdentifier());

        return BinaryenLocalGet(
            module,
            identInScope->getMappedBinaryenIndex(),
            getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(identNode->getResolvedType()))
        );
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

        if (dynamic_pointer_cast<TypeDeclarationNode>(binOpNode->getResolvedType())->getType() == DataTypes::STRING) {
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

    void CodeGen::hoistCapsuleElements(vector<shared_ptr<ASTNode>> elements) {
        scope.enterScope();

        for (auto ast : elements) bindIdentifierToScope(ast);
    }

    void CodeGen::bindIdentifierToScope(shared_ptr<ASTNode> ast) {
        string identifier = dynamic_pointer_cast<IdentifierNode>(ast->getLeft())->getIdentifier();

        if (ast->getRight()->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            identifier = Compiler::getQualifiedFunctionIdentifier(identifier, ast->getRight());
        } 

        scope.insert(identifier, ast->getRight());
    }
}
