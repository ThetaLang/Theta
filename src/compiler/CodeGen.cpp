#include <deque>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "binaryen-c.h"
#include "compiler/Compiler.hpp"
#include "lexer/Lexemes.hpp"
#include "StandardLibrary.hpp"
#include "CodeGen.hpp"
#include "DataTypes.hpp"
#include "parser/ast/AssignmentNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"

namespace Theta {
    BinaryenModuleRef CodeGen::generateWasmFromAST(shared_ptr<ASTNode> ast) {
        BinaryenModuleRef module = BinaryenModuleCreate();

        BinaryenModuleSetFeatures(module, BinaryenFeatureStrings());

        StandardLibrary::registerFunctions(module);

        generate(ast, module);

        registerModuleFunctions(module);

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
        } else if (node->getNodeType() == ASTNode::CONTROL_FLOW) {
            return generateControlFlow(dynamic_pointer_cast<ControlFlowNode>(node), module);
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
                string identifier = dynamic_pointer_cast<IdentifierNode>(elem->getLeft())->getIdentifier();

                if (elemType == DataTypes::FUNCTION) {
                    generateFunctionDeclaration(
                        identifier,
                        dynamic_pointer_cast<FunctionDeclarationNode>(elem->getRight()),
                        module,
                        true
                    );
                } else {
                    shared_ptr<ASTNode> assignmentRhs = elem->getRight();
                    assignmentRhs->setMappedBinaryenIndex(-1); //Index of -1 means its a global
                    scope.insert(identifier, assignmentRhs);

                    BinaryenGlobalSet(
                        module,
                        identifier.c_str(),
                        generate(assignmentRhs, module)
                    );
                }
            }
        }
    }

    BinaryenExpressionRef CodeGen::generateAssignment(shared_ptr<AssignmentNode> assignmentNode, BinaryenModuleRef &module) {
        string assignmentIdentifier = dynamic_pointer_cast<IdentifierNode>(assignmentNode->getLeft())->getIdentifier();

        // Function declarations dont get generated generically like the rest of the AST elements, they are not part of the "generate" method,
        // because they behave differently depending on where the function was declared. A function declared at the top level of capsule will
        // be hoisted and will have no inherent scope bound to it. 
        //
        // A function declared within another function body OR within any other structure will be turned into a closure that contains the scope
        // of anything outside of that function.
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

        generateClosure(dynamic_pointer_cast<FunctionDeclarationNode>(assignmentNode->getRight()), module);

        // TODO: Functions will be defined as closures which take in the scope of the surrounding block as additional parameters
        throw new runtime_error("Lambda functions are not yet implemented.");
    }

    // Transforms nested function declarations and generates an anonymous function in the function table
    void CodeGen::generateClosure(shared_ptr<FunctionDeclarationNode> fnDeclNode, BinaryenModuleRef &module) {
        // Capture the outer scope
        set<string> requiredScopeIdentifiers;
        set<string> paramIdentifiers;

        cout << "GENERATING CLOSURE FOR FUNCTION, AST NODE ID: " << to_string(fnDeclNode->getId()) << endl;
        
        for (auto param : fnDeclNode->getParameters()->getElements()) {
            paramIdentifiers.insert(dynamic_pointer_cast<IdentifierNode>(param)->getIdentifier());
        }

        vector<shared_ptr<ASTNode>> identifiersInBody = Compiler::findAllInTree(fnDeclNode->getDefinition(), ASTNode::IDENTIFIER);
    
        for (auto ident : identifiersInBody) {
            string identifierName = dynamic_pointer_cast<IdentifierNode>(ident)->getIdentifier();

            // Only add identifiers that are not present in the function params
            if (paramIdentifiers.find(identifierName) != paramIdentifiers.end()) continue;

            // If an identifier is globally available we dont need to include it either
            shared_ptr<ASTNode> inScope = scope.lookup(identifierName);
            if (inScope->getMappedBinaryenIndex() == -1) continue;

            requiredScopeIdentifiers.insert(identifierName);
        }
    
        // Find any identifiers that were passed in as parameters
        deque<shared_ptr<ASTNode>> identifiersFromParams = findParameterizedIdentifiersFromAncestors(fnDeclNode, requiredScopeIdentifiers);

        // If we've traversed the tree for parameters and we still have some missing identifiers, they must be defined in bodies
        if (requiredScopeIdentifiers.size() > 0) {

        }
    }

    deque<shared_ptr<ASTNode>> CodeGen::findParameterizedIdentifiersFromAncestors(shared_ptr<ASTNode> node, set<string> &identifiersToFind, deque<shared_ptr<ASTNode>> found) {
        if (identifiersToFind.size() == 0 || node->getParent()->getNodeType() == ASTNode::CAPSULE) return found;

        cout << "TEEHEE" << endl;
        cout << "PARENT IS: " << node->getParent()->toJSON() << endl;

        if (node->getParent()->getNodeType() != ASTNode::FUNCTION_DECLARATION) {
            return findParameterizedIdentifiersFromAncestors(node->parent, identifiersToFind, found);
        }

        shared_ptr<FunctionDeclarationNode> parent = dynamic_pointer_cast<FunctionDeclarationNode>(node->getParent());

        unordered_map<string, shared_ptr<ASTNode>> paramIdentifiers;

        for (auto param : parent->getParameters()->getElements()) {
            paramIdentifiers.insert(make_pair(
                dynamic_pointer_cast<IdentifierNode>(param)->getIdentifier(),
                param
            ));
        }

        for (auto ident : identifiersToFind) {
            auto param = paramIdentifiers.find(ident);

            if (param == paramIdentifiers.end()) continue;

            found.push_front(param->second);
            identifiersToFind.erase(ident);
        }

        return findParameterizedIdentifiersFromAncestors(parent, identifiersToFind, found);
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

        scope.insert(LOCAL_IDX_SCOPE_KEY, make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, to_string(totalParams), nullptr));

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

        functionNameToClosureMap.insert(make_pair(
            functionName,
            WasmClosure(functionNameToClosureMap.size(), totalParams)
        ));

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

    BinaryenExpressionRef CodeGen::generateControlFlow(shared_ptr<ControlFlowNode> controlFlowNode, BinaryenModuleRef &module) {
        controlFlowNode->getConditionExpressionPairs();

        BinaryenExpressionRef expr = NULL;

        // WASM doesnt support else-if structured natively, so we merge the else-ifs into nested else blocks that have ifs
        // inside of them. So the following:
        // if (x == 1) {
        // } else if (x == 2) {
        // } else if (x == 3) {
        // } else {
        // }
        //
        // becomes this:
        // if (x == 1) {
        // } else {
        //   if (x == 2) {
        //   } else {
        //     if (x == 3) {
        //     } else {
        //     }
        //   }
        // }
        for (int i = controlFlowNode->getConditionExpressionPairs().size() - 1; i >= 0; i--) {
            pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>> cndExprPair = controlFlowNode->getConditionExpressionPairs().at(i);

            // Handle the else case
            if (cndExprPair.first == nullptr) {
                expr = generate(cndExprPair.second, module);
                continue;
            }

            expr = BinaryenIf(
                module,
                generate(cndExprPair.first, module),
                generate(cndExprPair.second, module),
                expr
            );
        }

        return expr;
    }

    BinaryenExpressionRef CodeGen::generateIdentifier(shared_ptr<IdentifierNode> identNode, BinaryenModuleRef &module) {
        shared_ptr<ASTNode> identInScope = scope.lookup(identNode->getIdentifier());

        if (identInScope->getMappedBinaryenIndex() == -1) {
            string identName = identNode->getIdentifier();
        
            return BinaryenGlobalGet(
                module,
                identName.c_str(),
                getBinaryenTypeFromTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(identInScope->getResolvedType()))
            );
        }

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

        BinaryenExpressionRef binaryenLeft = generate(binOpNode->getLeft(), module);
        BinaryenExpressionRef binaryenRight = generate(binOpNode->getRight(), module);

        if (!binaryenLeft || !binaryenRight) {
            throw runtime_error("Invalid operand types for binary operation");
        }

        if (dynamic_pointer_cast<TypeDeclarationNode>(binOpNode->getLeft()->getResolvedType())->getType() == DataTypes::STRING) {
            return generateStringBinaryOperation(binOpNode->getOperator(), binaryenLeft, binaryenRight, module);
        } 

        BinaryenOp op = getBinaryenOpFromBinOpNode(binOpNode);

        return BinaryenBinary(
            module,
            op,
            binaryenLeft,
            binaryenRight
        );
    }

    BinaryenExpressionRef CodeGen::generateStringBinaryOperation(string op, BinaryenExpressionRef left, BinaryenExpressionRef right, BinaryenModuleRef &module) {
        if (op == Lexemes::PLUS) {
            return BinaryenStringConcat(module, left, right);
        } else if (op == Lexemes::INEQUALITY) {
            // Binaryen supports string equality checks but not inequality checks, so we need to wrap an equality check
            // in a unary NOT to achieve the same effect
            return BinaryenUnary(
                module,
                BinaryenEqZInt32(),
                BinaryenStringEq(module, BinaryenStringEqEqual(), left, right)
            );
        }

        return BinaryenStringEq(module, BinaryenStringEqEqual(), left, right);
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

        string dataType = dynamic_pointer_cast<TypeDeclarationNode>(binOpNode->getLeft()->getResolvedType())->getType();

        if (op == Lexemes::EQUALITY && dataType == DataTypes::NUMBER) return BinaryenEqInt64();
        if (op == Lexemes::EQUALITY && dataType == DataTypes::BOOLEAN) return BinaryenEqInt32();
        if (op == Lexemes::INEQUALITY && dataType == DataTypes::NUMBER) return BinaryenNeInt64();
        if (op == Lexemes::INEQUALITY && dataType == DataTypes::BOOLEAN) return BinaryenEqInt32();
        if (op == Lexemes::LT && dataType == DataTypes::NUMBER) return BinaryenLtSInt64();
        if (op == Lexemes::GT && dataType == DataTypes::NUMBER) return BinaryenGtSInt64();
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

    void CodeGen::registerModuleFunctions(BinaryenModuleRef &module) {
        BinaryenAddTable(
            module,
            FN_TABLE_NAME.c_str(),
            functionNameToClosureMap.size(),
            functionNameToClosureMap.size(),
            BinaryenTypeFuncref()
        );

        const char** fnNames = new const char*[functionNameToClosureMap.size()];

        for (auto& [fnName, fnRef] : functionNameToClosureMap) {
            fnNames[fnRef.getFunctionIndex()] = fnName.c_str();
        }

        BinaryenAddActiveElementSegment(
            module,
            FN_TABLE_NAME.c_str(),
            "0",
            fnNames,
            functionNameToClosureMap.size(),
            BinaryenConst(module, BinaryenLiteralInt32(0))
        );
    }
}
