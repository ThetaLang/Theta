#include "TypeChecker.hpp"
#include "Compiler.hpp"
#include <algorithm>
#include <memory>
#include <array>
#include <string>
#include <utility>
#include "DataTypes.hpp"
#include "exceptions/IllegalReassignmentError.hpp"
#include "exceptions/ReferenceError.hpp"
#include "exceptions/TypeError.hpp"
#include "exceptions/IntegrityError.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/CapsuleNode.hpp"
#include "parser/ast/DictionaryNode.hpp"
#include "parser/ast/FunctionInvocationNode.hpp"
#include "parser/ast/StructDeclarationNode.hpp"
#include "parser/ast/StructDefinitionNode.hpp"
#include "parser/ast/SymbolNode.hpp"
#include "parser/ast/TupleNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "lexer/Lexemes.hpp"

using namespace std;

namespace Theta {
    bool TypeChecker::checkAST(shared_ptr<ASTNode> ast, vector<pair<string, shared_ptr<ASTNode>>> bindToScope) {
        if (ast->hasOwnScope()) identifierTable.enterScope();

        // Sometimes, like in the case of function declarations, we need to bind the parameters into the scope of the
        // definition block
        for (auto scopeBinding : bindToScope) {
            identifierTable.insert(scopeBinding.first, scopeBinding.second);
        }

        if (ast->getNodeType() == ASTNode::CAPSULE) {
            hoistCapsuleDeclarations(dynamic_pointer_cast<CapsuleNode>(ast));
        }

        // Check node children first
        if (ast->getValue()) {
            bool childValid = checkAST(ast->getValue());

            if (!childValid) return false;
        } else if (ast->getLeft()) {
            bool lhsValid = checkAST(ast->getLeft());
            bool rhsValid = checkAST(ast->getRight());

            if (!lhsValid || !rhsValid) return false;
        } else if (ast->hasMany()) {
            shared_ptr<ASTNodeList> nodeList = dynamic_pointer_cast<ASTNodeList>(ast);

            vector<shared_ptr<ASTNode>> nodeElements = nodeList->getElements();

            for (int i = 0; i < nodeElements.size(); i++) {
                bool elValid = checkAST(nodeElements[i]);

                if (!elValid) return false;
            }
        }

        if (ast->hasOwnScope()) identifierTable.exitScope();

        return checkNode(ast);
    }

    bool TypeChecker::checkNode(shared_ptr<ASTNode> node) {
        if (node->getNodeType() == ASTNode::AST_NODE_LIST) {
            return true;
        } else if (node->getNodeType() == ASTNode::SOURCE) {
            node->setResolvedType(node->getValue()->getResolvedType());
            return true;
        } else if (node->getNodeType() == ASTNode::RETURN) {
            node->setResolvedType(node->getValue()->getResolvedType());
            return true;
        } else if (node->getNodeType() == ASTNode::NUMBER_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::NUMBER, node));
            return true;
        } else if (node->getNodeType() == ASTNode::STRING_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::STRING, node));
            return true;
        } else if (node->getNodeType() == ASTNode::BOOLEAN_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN, node));
            return true;
        } else if (node->getNodeType() == ASTNode::CAPSULE) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::CAPSULE, node));
            return true;
        } else if (node->getNodeType() == ASTNode::SYMBOL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::SYMBOL, node));
            return true;
        } else if (node->getNodeType() == ASTNode::TYPE_DECLARATION) {
            return checkTypeDeclarationNode(dynamic_pointer_cast<TypeDeclarationNode>(node));
        } else if (node->getNodeType() == ASTNode::ASSIGNMENT) {
            return checkAssignmentNode(dynamic_pointer_cast<AssignmentNode>(node));
        } else if (node->getNodeType() == ASTNode::IDENTIFIER) {
            return checkIdentifierNode(dynamic_pointer_cast<IdentifierNode>(node));
        } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
            return checkBinaryOperationNode(dynamic_pointer_cast<BinaryOperationNode>(node));
        } else if (node->getNodeType() == ASTNode::UNARY_OPERATION) {
            return checkUnaryOperationNode(dynamic_pointer_cast<UnaryOperationNode>(node));
        } else if (node->getNodeType() == ASTNode::BLOCK) {
            return checkBlockNode(dynamic_pointer_cast<BlockNode>(node));
        } else if (node->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            return checkFunctionDeclarationNode(dynamic_pointer_cast<FunctionDeclarationNode>(node));
        } else if (node->getNodeType() == ASTNode::FUNCTION_INVOCATION) {
            return checkFunctionInvocationNode(dynamic_pointer_cast<FunctionInvocationNode>(node));
        } else if (node->getNodeType() == ASTNode::CONTROL_FLOW) {
            return checkControlFlowNode(dynamic_pointer_cast<ControlFlowNode>(node));
        } else if (node->getNodeType() == ASTNode::LIST) {
            return checkListNode(dynamic_pointer_cast<ListNode>(node));
        } else if (node->getNodeType() == ASTNode::TUPLE) {
            return checkTupleNode(dynamic_pointer_cast<TupleNode>(node));
        } else if (node->getNodeType() == ASTNode::DICTIONARY) {
            return checkDictionaryNode(dynamic_pointer_cast<DictionaryNode>(node));
        } else if (node->getNodeType() == ASTNode::STRUCT_DEFINITION) {
            return checkStructDefinitionNode(dynamic_pointer_cast<StructDefinitionNode>(node));
        } else if (node->getNodeType() == ASTNode::STRUCT_DECLARATION) {
            return checkStructDeclarationNode(dynamic_pointer_cast<StructDeclarationNode>(node));
        }

        return false;
    }

    bool TypeChecker::checkTypeDeclarationNode(shared_ptr<TypeDeclarationNode> node) {
        if (isLanguageDataType(node->getType())) return true;

        shared_ptr<ASTNode> customDataTypeInScope = lookupInScope(node->getType());
        
        if (!customDataTypeInScope) {
            Compiler::getInstance().addException(make_shared<ReferenceError>(node->getType()));
            return false;
        }

        return true;
    }

    bool TypeChecker::checkAssignmentNode(shared_ptr<AssignmentNode> node) {
        bool typesMatch = isSameType(node->getLeft()->getValue(), node->getRight()->getResolvedType());
        
        shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(node->getLeft());

        if (!typesMatch) {
            string leftTypeString = dynamic_pointer_cast<TypeDeclarationNode>(node->getLeft()->getValue())->toString();
            string rightTypeString = dynamic_pointer_cast<TypeDeclarationNode>(node->getRight()->getResolvedType())->toString();

            Compiler::getInstance().addException(
                make_shared<TypeError>(
                    rightTypeString + " is not assignable to " + leftTypeString,
                    node->getLeft()->getValue(),
                    node->getRight()->getResolvedType()
                )
            );

            return false;
        }

        node->setResolvedType(node->getLeft()->getValue());

        string rhsType = dynamic_pointer_cast<TypeDeclarationNode>(node->getRight()->getResolvedType())->getType();
        
        // Function names can be overloaded, so functions don't need this check
        if (rhsType == DataTypes::FUNCTION) {
            string uniqueFuncIdentifier = Compiler::getQualifiedFunctionIdentifier(
                ident->getIdentifier(),
                (node->getRight()->getNodeType() == ASTNode::FUNCTION_DECLARATION 
                    ? node->getRight()
                    // We're assigning to the result of a function invocation, and that invocation returns a function
                    : node->getRight()->getResolvedType()
                )
            );
            
            auto existingFuncIdentifierInScope = identifierTable.lookup(uniqueFuncIdentifier);

            if (existingFuncIdentifierInScope.has_value()) {
                Compiler::getInstance().addException(make_shared<IllegalReassignmentError>(ident->getIdentifier()));
                return false;
            }

            identifierTable.insert(uniqueFuncIdentifier, node->getRight());
        } else {
            auto existingIdentifierInScope = identifierTable.lookup(ident->getIdentifier());

            if (existingIdentifierInScope.has_value()) {
                Compiler::getInstance().addException(make_shared<IllegalReassignmentError>(ident->getIdentifier()));
                return false;
            }

            identifierTable.insert(ident->getIdentifier(), node->getResolvedType());
        }

        return true;
    }

    bool TypeChecker::checkIdentifierNode(shared_ptr<IdentifierNode> node) {
        // Auto return if the identifier comes with its own type declaration. This is for assignment nodes lhs
        if (node->getValue()) return true;
        
        shared_ptr<ASTNode> foundReferencedIdentifier = lookupInScope(node->getIdentifier());

        if (!foundReferencedIdentifier) {
            Compiler::getInstance().addException(make_shared<ReferenceError>(node->getIdentifier()));
            return false;
        }

        node->setResolvedType(foundReferencedIdentifier);
        return true;
    }

    bool TypeChecker::checkBinaryOperationNode(shared_ptr<BinaryOperationNode> node) {
        bool typesMatch = isSameType(node->getLeft()->getResolvedType(), node->getRight()->getResolvedType());

        if (!typesMatch) {
            Compiler::getInstance().addException(
                make_shared<TypeError>(
                    "Binary Expression is not homogenous",
                    node->getLeft()->getResolvedType(),
                    node->getRight()->getResolvedType()
                )
            );

            return false;
        }

        if (isBooleanOperator(node->getOperator())) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN, node));
        } else {
            node->setResolvedType(node->getLeft()->getResolvedType());
        }

        return true;
    }

    bool TypeChecker::checkUnaryOperationNode(shared_ptr<UnaryOperationNode> node) {
        bool valid = checkAST(node->getValue());

        if (!valid) return false;

        shared_ptr<TypeDeclarationNode> boolType = make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN, nullptr);
        shared_ptr<TypeDeclarationNode> numType = make_shared<TypeDeclarationNode>(DataTypes::NUMBER, nullptr);

        if (isSameType(node->getValue()->getResolvedType(), boolType) && node->getOperator() != Lexemes::NOT) {
            Compiler::getInstance().addException(
                make_shared<TypeError>(
                    "Boolean expression may only have boolean unary operator",
                    node->getValue()->getResolvedType(),
                    numType
                )
            );
            
            return false;
        }

        if (isSameType(node->getValue()->getResolvedType(), numType) && node->getOperator() != Lexemes::MINUS) {
            Compiler::getInstance().addException(
                make_shared<TypeError>(
                    "Numerical expression may only have numerical unary operator",
                    node->getValue()->getResolvedType(),
                    boolType
                )
            );
            
            return false;
        }
        
        node->setResolvedType(node->getValue()->getResolvedType());
        return true;
    }

    bool TypeChecker::checkBlockNode(shared_ptr<BlockNode> node) {
        vector<shared_ptr<TypeDeclarationNode>> blockReturnTypes;

        vector<shared_ptr<ASTNode>> returns = Compiler::findAllInTree(node, ASTNode::RETURN);

        for (int i = 0; i < returns.size(); i++) {
            blockReturnTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(returns.at(i)->getResolvedType()));
        }

        // Add return type of last expression for implicit return
        if (node->getElements().size() > 0) {
            blockReturnTypes.push_back(
                dynamic_pointer_cast<TypeDeclarationNode>(node->getElements().at(node->getElements().size() - 1)->getResolvedType())
            );
        }

        if (blockReturnTypes.size() == 1) {
            node->setResolvedType(blockReturnTypes[0]);
        } else if (blockReturnTypes.size() > 1) {
            node->setResolvedType(makeVariadicType(blockReturnTypes, node));
        }

        return true;
    }

    bool TypeChecker::checkFunctionDeclarationNode(shared_ptr<FunctionDeclarationNode> node) {
        // "Typecheck" function params first to make them available within the scope of the definition
        vector<shared_ptr<ASTNode>> fnParams = dynamic_pointer_cast<ASTNodeList>(node->getParameters())->getElements();

        vector<pair<string, shared_ptr<ASTNode>>> paramScopeBindings;
        for (auto param : fnParams) {
            // TODO: Handle assignment nodes for when we set a default value for parameters
            if (param->getNodeType() == ASTNode::IDENTIFIER) {
                shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(param);

                paramScopeBindings.push_back(make_pair(ident->getIdentifier(), ident->getValue()));
            }
        }

        bool valid = checkAST(node->getDefinition(), paramScopeBindings);

        if (!valid) return false;

        shared_ptr<TypeDeclarationNode> funcType = make_shared<TypeDeclarationNode>(DataTypes::FUNCTION, node);
    
        vector<shared_ptr<ASTNode>> typeValues;
        for (auto param : node->getParameters()->getElements()) {
            typeValues.push_back(Compiler::deepCopyTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(param->getValue()), funcType));
        }

        typeValues.push_back(node->getDefinition()->getResolvedType());
        
        funcType->setElements(typeValues);

        // A function might already have a resolvedType if it was hoisted, we need to redefine it with the real return type
        if (node->getResolvedType()) {
            if (typeValues.size() == 1) {
                node->getResolvedType()->setValue(typeValues.at(0));
            } else {
                dynamic_pointer_cast<TypeDeclarationNode>(node->getResolvedType())->setElements(typeValues);
            }
        } else {
            node->setResolvedType(funcType); 
        }

        return valid;
    }

    bool TypeChecker::checkFunctionInvocationNode(shared_ptr<FunctionInvocationNode> node) {
        vector<shared_ptr<ASTNode>> params = dynamic_pointer_cast<ASTNodeList>(node->getParameters())->getElements();

        bool validParams = checkAST(node->getParameters());

        if (!validParams) return false;

        string funcIdentifier = dynamic_pointer_cast<IdentifierNode>(node->getIdentifier())->getIdentifier();
        string uniqueFuncIdentifier = Compiler::getQualifiedFunctionIdentifier(funcIdentifier, node);

        shared_ptr<ASTNode> referencedFunction = lookupInScope(uniqueFuncIdentifier);
        
        if (!referencedFunction) {
            string paramTypes = "(";

            for (int i = 0; i < node->getParameters()->getElements().size(); i++) {
                if (i > 0) paramTypes += ", ";
                paramTypes += dynamic_pointer_cast<TypeDeclarationNode>(
                    node->getParameters()->getElements().at(i)->getResolvedType()
                )->toString();  
            }

            paramTypes += ")";

            Compiler::getInstance().addException(make_shared<ReferenceError>(funcIdentifier + paramTypes));
            return false;
        }

        shared_ptr<TypeDeclarationNode> referencedFunctionType = dynamic_pointer_cast<TypeDeclarationNode>(referencedFunction->getResolvedType());

        // The function return type is the last element in the types list
        if (referencedFunctionType->getValue()) {
            node->setResolvedType(referencedFunctionType->getValue());
        } else {
            node->setResolvedType(referencedFunctionType->getElements().back());
        }

        return true;
    }

    bool TypeChecker::checkControlFlowNode(shared_ptr<ControlFlowNode> node) {
        vector<shared_ptr<TypeDeclarationNode>> returnTypes;
        bool hasElseBlock = false;

        for (int i = 0; i < node->getConditionExpressionPairs().size(); i++) {
            pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>> pair = node->getConditionExpressionPairs().at(i);

            // It might be a nullptr in the case of the else block
            if (pair.first) {
                bool validCondition = checkAST(pair.first);

                shared_ptr<TypeDeclarationNode> boolType = make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN, nullptr);

                vector<shared_ptr<ASTNode>> typesThatCanBeInterpretedAsBooleans = {
                    boolType,
                    make_shared<TypeDeclarationNode>(DataTypes::NUMBER, nullptr)
                };

                if (!validCondition || !isOneOfTypes(pair.first->getResolvedType(), typesThatCanBeInterpretedAsBooleans)) {
                    Compiler::getInstance().addException(
                        make_shared<TypeError>(
                            "Non-boolean expression in control flow condition",
                            pair.first->getResolvedType(),
                            boolType
                        )
                    );

                    return false;
                }
            } else {
                hasElseBlock = true;
            }

            bool validExpression = checkAST(pair.second);

            if (!validExpression) return false;

            returnTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(pair.second->getResolvedType()));
        }

        // If we have an if without an else, thats fine, but that means we have a potential hole if we try to use this as
        // a return value to something (like assigning a variable to the result of a control flow). We can return nil as part
        // of the resolved type of the node, which will cause assignments without an else to fail (as they should)
        if (!hasElseBlock) returnTypes.push_back(make_shared<TypeDeclarationNode>(DataTypes::NIL, node));

        if (returnTypes.size() == 1) {
            node->setResolvedType(returnTypes[0]);
        } else {
            node->setResolvedType(makeVariadicType(returnTypes, node));
        }

        return true;
    }

    bool TypeChecker::checkListNode(shared_ptr<ListNode> node) {
        vector<shared_ptr<TypeDeclarationNode>> returnTypes;

        shared_ptr<ASTNodeList> listNode = dynamic_pointer_cast<ASTNodeList>(node);

        for (int i = 0; i < listNode->getElements().size(); i++) {
            bool validElement = checkAST(listNode->getElements().at(i));

            if (!validElement) return false;
            
            returnTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(listNode->getElements().at(i)->getResolvedType()));
        }

        if (!isHomogenous(returnTypes)) {
            for (auto type : returnTypes) {
                if (!isSameType(type, returnTypes.at(0))) {
                    Compiler::getInstance().addException(
                        make_shared<TypeError>(
                            "Lists must be homogenous",
                            returnTypes.at(0),
                            type
                        )
                    );
                }
            }

            return false;
        }
    
        shared_ptr<TypeDeclarationNode> listType = make_shared<TypeDeclarationNode>(DataTypes::LIST, node);
        
        if (returnTypes.size() == 0) {
            listType->setValue(make_shared<TypeDeclarationNode>(DataTypes::UNKNOWN, listType));
        } else {
            listType->setValue(returnTypes.at(0));
        }

        node->setResolvedType(listType);

        return true;
    }

    bool TypeChecker::checkTupleNode(shared_ptr<TupleNode> node) {
        bool validLeft = checkAST(node->getLeft());
        bool validRight = checkAST(node->getRight());

        if (!validLeft || !validRight) return false;
        
        shared_ptr<TypeDeclarationNode> type = make_shared<TypeDeclarationNode>(DataTypes::TUPLE, node);

        type->setLeft(node->getLeft()->getResolvedType());
        type->setRight(node->getRight()->getResolvedType());

        node->setResolvedType(type);

        return true;
    }

    bool TypeChecker::checkDictionaryNode(shared_ptr<DictionaryNode> node) {
        vector<shared_ptr<TypeDeclarationNode>> keyTypes;
        vector<shared_ptr<TypeDeclarationNode>> valueTypes;

        shared_ptr<ASTNodeList> dictNode = dynamic_pointer_cast<ASTNodeList>(node);
        
        for (int i = 0; i < dictNode->getElements().size(); i++) {
            shared_ptr<ASTNode> kvTuple = dictNode->getElements().at(i);

            bool isKeyValid = checkAST(kvTuple->getLeft());
            bool isValValid = checkAST(kvTuple->getRight());

            if (!isKeyValid || !isValValid) return false;
        
            shared_ptr<TypeDeclarationNode> symbolType = make_shared<TypeDeclarationNode>(DataTypes::SYMBOL, nullptr);

            if (!isSameType(kvTuple->getLeft()->getResolvedType(), symbolType)) {
                Compiler::getInstance().addException(
                    make_shared<TypeError>(
                        "Dictionary key must be a <Symbol>",
                        kvTuple->getLeft()->getResolvedType(),
                        symbolType
                    )
                );
            }
        
            keyTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(kvTuple->getLeft()->getResolvedType()));
            valueTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(kvTuple->getRight()->getResolvedType()));
        }

        if (!isHomogenous(valueTypes)) {
            Compiler::getInstance().addException(
                make_shared<TypeError>(
                    "Dictionary values must be homogenous",
                    valueTypes.at(0),
                    valueTypes.at(1) // FIXME: This wont always be the incorrect type. Need to detect which values are mistyped
                )
            );

            return false;
        }

        shared_ptr<TypeDeclarationNode> dictType = make_shared<TypeDeclarationNode>(DataTypes::DICT, node);
    
        if (valueTypes.size() == 0) {
            dictType->setValue(make_shared<TypeDeclarationNode>(DataTypes::UNKNOWN, dictType));
        } else {
            dictType->setValue(valueTypes.at(0));
        }

        node->setResolvedType(dictType);
        
        return true;
    }

    bool TypeChecker::checkStructDefinitionNode(shared_ptr<StructDefinitionNode> node) {
        shared_ptr<ASTNodeList> structNode = dynamic_pointer_cast<ASTNodeList>(node);

        for (int i = 0; i < structNode->getElements().size(); i++) {
            bool valid = checkAST(structNode->getElements().at(i));

            if (!valid) return false;
        }

        structNode->setResolvedType(make_shared<TypeDeclarationNode>(node->getName(), structNode));
    
        auto existingIdentifierInScope = identifierTable.lookup(node->getName());

        if (existingIdentifierInScope.has_value()) {
            Compiler::getInstance().addException(make_shared<IllegalReassignmentError>(node->getName()));
            return false;
        }

        identifierTable.insert(node->getName(), node);

        return true;
    }

    bool TypeChecker::checkStructDeclarationNode(shared_ptr<StructDeclarationNode> node) {
        shared_ptr<ASTNode> foundDefinition = lookupInScope(node->getStructType());

        if (!foundDefinition) {
            Compiler::getInstance().addException(make_shared<ReferenceError>(node->getStructType()));
            return false;
        }

        shared_ptr<ASTNodeList> structDeclarationNode = dynamic_pointer_cast<ASTNodeList>(node->getValue());

        shared_ptr<ASTNodeList> structDefinition = dynamic_pointer_cast<ASTNodeList>(foundDefinition);

        map<string, shared_ptr<TypeDeclarationNode>> requiredStructFields;
        for (int i = 0; i < structDefinition->getElements().size(); i++) {
            shared_ptr<IdentifierNode> elem = dynamic_pointer_cast<IdentifierNode>(structDefinition->getElements().at(i));

            // Needs the : because struct declarations are lists of tuples
            requiredStructFields.insert(make_pair(":" + elem->getIdentifier(), dynamic_pointer_cast<TypeDeclarationNode>(elem->getValue())));
        }

        for (int i = 0; i < structDeclarationNode->getElements().size(); i++) {
            shared_ptr<SymbolNode> key = dynamic_pointer_cast<SymbolNode>(structDeclarationNode->getElements().at(i)->getLeft());
            
            auto it = requiredStructFields.find(key->getSymbol()); 

            if (it == requiredStructFields.end()) {
                Compiler::getInstance().addException(
                    make_shared<IntegrityError>(
                        "Struct declaration can't contain keys that are not in its definition. ",
                        key->getSymbol() + " is not defined in struct " + node->getStructType()
                    )
                );
                return false;
            }

            if (!isSameType(it->second, structDeclarationNode->getElements().at(i)->getRight()->getResolvedType())) {
                Compiler::getInstance().addException(
                    make_shared<TypeError>(
                        "Struct key type mismatch",
                        it->second,
                        structDeclarationNode->getElements().at(i)->getRight()->getResolvedType()
                    )
                );

                return false;
            }

            requiredStructFields.erase(key->getSymbol());
        }

        // Should be empty by the end. If its not, that means the declared struct doesnt match the definition
        if (requiredStructFields.size() > 0) {
            string missingFields = "";

            for (auto field : requiredStructFields) {
                if (missingFields.length() > 0) missingFields += ", ";
                missingFields += field.first;
            }

            Compiler::getInstance().addException(
                make_shared<IntegrityError>(
                    "Struct declaration is missing required field(s) from definition",
                    missingFields + " " + (requiredStructFields.size() == 1 ? "is" : "are") + " required in " + node->getStructType() + " but missing in declaration."
                )
            );

            return false;
        }

        node->setResolvedType(make_shared<TypeDeclarationNode>(node->getStructType(), node));

        return true;
    }

    void TypeChecker::hoistCapsuleDeclarations(shared_ptr<CapsuleNode> node) {
        vector<shared_ptr<ASTNode>> capsuleTopLevelElements = dynamic_pointer_cast<ASTNodeList>(node->getValue())->getElements();

        capsuleDeclarationsTable.enterScope();

        for (int i = 0; i < capsuleTopLevelElements.size(); i++) {
            ASTNode::Types nodeType = capsuleTopLevelElements.at(i)->getNodeType();

            if (nodeType == ASTNode::ASSIGNMENT) {
                if (capsuleTopLevelElements.at(i)->getRight()->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
                    hoistFunction(capsuleTopLevelElements.at(i));
                } else {
                    hoistIdentifier(capsuleTopLevelElements.at(i));
                }
            } else if (nodeType == ASTNode::STRUCT_DEFINITION) {
                hoistStructDefinition(capsuleTopLevelElements.at(i));
            }
        }
    }

    void TypeChecker::hoistFunction(shared_ptr<ASTNode> node) {
        shared_ptr<AssignmentNode> assignmentNode = dynamic_pointer_cast<AssignmentNode>(node); 
        shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(node->getLeft());

        string uniqueFuncIdentifier = Compiler::getQualifiedFunctionIdentifier(ident->getIdentifier(), node->getRight());

        auto existingFuncIdentifierInScope = capsuleDeclarationsTable.lookup(uniqueFuncIdentifier);

        if (existingFuncIdentifierInScope.has_value()) {
            Compiler::getInstance().addException(make_shared<IllegalReassignmentError>(ident->getIdentifier()));
            return;
        }

        // Initially set the function resolvedType to whatever the identifier type is specified. This will get
        // updated later when we actually typecheck the function definition to whatever types the function actually returns.
        // This way, we support recursive function type resolution and cyclic function type resolution
        node->getRight()->setResolvedType(Compiler::deepCopyTypeDeclaration(dynamic_pointer_cast<TypeDeclarationNode>(ident->getValue()), node));

        capsuleDeclarationsTable.insert(uniqueFuncIdentifier, node->getRight());
    }

    void TypeChecker::hoistStructDefinition(shared_ptr<ASTNode> node) {
        shared_ptr<StructDefinitionNode> structNode = dynamic_pointer_cast<StructDefinitionNode>(node);

        auto existingStructDefinitionInScope = capsuleDeclarationsTable.lookup(structNode->getName());
        
        if (existingStructDefinitionInScope.has_value()) {
            Compiler::getInstance().addException(make_shared<IllegalReassignmentError>(structNode->getName()));
            return;
        }

        structNode->setResolvedType(make_shared<TypeDeclarationNode>(structNode->getName(), structNode));

        capsuleDeclarationsTable.insert(structNode->getName(), node);
    }

    void TypeChecker::hoistIdentifier(shared_ptr<ASTNode> node) {
        shared_ptr<IdentifierNode> identNode = dynamic_pointer_cast<IdentifierNode>(node->getLeft());

        auto existingHoistedIdentifier = capsuleDeclarationsTable.lookup(identNode->getIdentifier());
    
        if (existingHoistedIdentifier.has_value()) {
            Compiler::getInstance().addException(make_shared<IllegalReassignmentError>(identNode->getIdentifier()));
            return;
        }

        identNode->setResolvedType(identNode->getValue());

        capsuleDeclarationsTable.insert(identNode->getIdentifier(), identNode->getResolvedType());
    }

    bool TypeChecker::isOneOfTypes(shared_ptr<ASTNode> type, vector<shared_ptr<ASTNode>> options) {
        for (auto option : options) {
            if (isSameType(type, option)) return true;
        }

        return false;
    }

    bool TypeChecker::isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2) {
        shared_ptr<TypeDeclarationNode> t1 = dynamic_pointer_cast<TypeDeclarationNode>(type1);
        shared_ptr<TypeDeclarationNode> t2 = dynamic_pointer_cast<TypeDeclarationNode>(type2);

        if (!t1 && !t2) return true;

        // For dicts or lists that are initialized to empty 
        if (
            t1 && t2 && 
            (
                (t1->getType() == DataTypes::DICT && t2->getType() == DataTypes::DICT) ||
                (t1->getType() == DataTypes::LIST && t2->getType() == DataTypes::LIST)
            ) && 
            t1->getValue() &&
            t2->getValue() && dynamic_pointer_cast<TypeDeclarationNode>(t2->getValue())->getType() == DataTypes::UNKNOWN
        ) return true;

        if (
            (!t1 || !t2) ||
            // Single child type
            (t1->getValue() && !t2->getValue()) ||
            (!t1->getValue() && t2->getValue()) ||
            (t1->getValue() && t2->getValue() && !isSameType(t1->getValue(), t2->getValue())) ||
            // Multi-child types
            (t1->getLeft() && !t2->getLeft()) ||
            (!t1->getLeft() && t2->getLeft()) ||
            (t1->getRight() && !t2->getRight()) ||
            (!t1->getRight() && t2->getRight()) ||
            (
                t1->getLeft() && t2->getLeft() && t1->getRight() && t2->getRight() &&
                (!isSameType(t1->getLeft(), t2->getLeft()) || !isSameType(t1->getRight(), t2->getRight()))
            ) ||
            // Variadic types
            (!t1->hasMany() && t2->hasMany())
        ) return false;

        // Only true if t1 contains all of the types in t2. t1 may have more types than t2. This is for situations like
        // x<Variadic<String, Number>> = 1, which should pass type validation.
        if (t1->hasMany() && !t2->hasMany()) {
            for (int i = 0; i < t1->getElements().size(); i++) {
                bool containsType = isSameType(t2, t1->getElements().at(i));

                if (!containsType) return false;
            }
        } else if (t1->hasMany() && t2->hasMany()) {
            for (int i = 0; i < t2->getElements().size(); i++) {
                bool containsType = false;

                for (int j = 0; j < t1->getElements().size(); j++) {
                    if (isSameType(t2->getElements().at(i), t1->getElements().at(j))) containsType = true;
                }

                if (!containsType) return false;
            }
        }

        return t1->getType() == t2->getType();
    }

    bool TypeChecker::isHomogenous(vector<shared_ptr<TypeDeclarationNode>> types) {
        if (types.size() == 0) return true;

        sort(types.begin(), types.end(), [](const shared_ptr<TypeDeclarationNode> &a, const shared_ptr<TypeDeclarationNode> &b) {
            if (a && b) return a->getType() < b->getType();

            return false;
        });
        
        auto ip = unique(types.begin(), types.end(), isSameType);
        types.resize(distance(types.begin(), ip));
    
        return types.size() == 1;
    }

    bool TypeChecker::isLanguageDataType(string type) {
        array<string, 9> LANGUAGE_DATATYPES = {
            DataTypes::NUMBER,
            DataTypes::STRING,
            DataTypes::BOOLEAN,
            DataTypes::DICT,
            DataTypes::LIST,
            DataTypes::TUPLE,
            DataTypes::VARIADIC,
            DataTypes::SYMBOL,
            DataTypes::FUNCTION
        };

        return find(LANGUAGE_DATATYPES.begin(), LANGUAGE_DATATYPES.end(), type) != LANGUAGE_DATATYPES.end();
    }

    bool TypeChecker::isBooleanOperator(string op) {
        array<string, 9> BOOLEAN_OPERATORS = {
            Lexemes::EQUALITY,
            Lexemes::INEQUALITY,
            Lexemes::LT,
            Lexemes::LTEQ,
            Lexemes::GT,
            Lexemes::GTEQ,
            Lexemes::AND,
            Lexemes::OR
        };

        return find(BOOLEAN_OPERATORS.begin(), BOOLEAN_OPERATORS.end(), op) != BOOLEAN_OPERATORS.end();
    }

    shared_ptr<TypeDeclarationNode> TypeChecker::makeVariadicType(vector<shared_ptr<TypeDeclarationNode>> types, shared_ptr<ASTNode> parent) {
        shared_ptr<TypeDeclarationNode> variadicTypeNode = make_shared<TypeDeclarationNode>(DataTypes::VARIADIC, parent);

        // The unique function requires a sorted vector
        sort(types.begin(), types.end(), [](const shared_ptr<TypeDeclarationNode>& a, const shared_ptr<TypeDeclarationNode>& b) {
            if (a && b) {
                return a->getType() < b->getType();
            }
            return false;
        });

        auto ip = unique(types.begin(), types.end(), isSameType);
        types.resize(distance(types.begin(), ip));

        // If theres only 1 unique type, this isn't variadic
        if (types.size() == 1) return types[0];

        vector<shared_ptr<ASTNode>> typesAsASTNode;
        for (int i = 0; i < types.size(); i++) {
            typesAsASTNode.push_back(dynamic_pointer_cast<ASTNode>(types.at(i)));
        }

        variadicTypeNode->setElements(typesAsASTNode);

        return variadicTypeNode;
    }

    shared_ptr<ASTNode> TypeChecker::lookupInScope(string identifierName) {
        auto foundInCapsule = capsuleDeclarationsTable.lookup(identifierName);
        auto foundInLocalScope = identifierTable.lookup(identifierName);

        // Local scope overrides capsule scope
        if (foundInLocalScope.has_value()) return foundInLocalScope.value();
    
        if (foundInCapsule.has_value()) return foundInCapsule.value();
    
        return nullptr;
    }

    shared_ptr<TypeDeclarationNode> TypeChecker::getFunctionReturnType(shared_ptr<FunctionDeclarationNode> fnDeclNode) {
        if (fnDeclNode->getResolvedType()->getValue()) return dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getResolvedType()->getValue());

        return dynamic_pointer_cast<TypeDeclarationNode>(dynamic_pointer_cast<TypeDeclarationNode>(fnDeclNode->getResolvedType())->getElements().back());
    }
}
