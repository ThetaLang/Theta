#include "TypeChecker.hpp"
#include "Compiler.hpp"
#include <memory>
#include <string>
#include "DataTypes.hpp"
#include "../util/Exceptions.hpp"
#include "../parser/ast/ASTNodeList.hpp"
#include "parser/ast/TupleNode.hpp"
#include "parser/ast/TypeDeclarationNode.hpp"

using namespace std;

namespace Theta {
    bool TypeChecker::checkAST(shared_ptr<ASTNode> ast) {
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

        return checkNode(ast);
    }

    bool TypeChecker::checkNode(shared_ptr<ASTNode> node) {
        if (node->getNodeType() == ASTNode::IDENTIFIER || node->getNodeType() == ASTNode::TYPE_DECLARATION) {
            return true;
        } else if (node->getNodeType() == ASTNode::SOURCE) {
            node->setResolvedType(node->getValue()->getResolvedType());
            return true;
        } else if (node->getNodeType() == ASTNode::RETURN) {
            node->setResolvedType(node->getValue()->getResolvedType());
            return true;
        } else if (node->getNodeType() == ASTNode::NUMBER_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::NUMBER));
            return true;
        } else if (node->getNodeType() == ASTNode::STRING_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::STRING));
            return true;
        } else if (node->getNodeType() == ASTNode::BOOLEAN_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN));
            return true;
        } else if (node->getNodeType() == ASTNode::CAPSULE) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::CAPSULE));
            return true;
        } else if (node->getNodeType() == ASTNode::SYMBOL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>(DataTypes::SYMBOL));
            return true;
        } else if (node->getNodeType() == ASTNode::ASSIGNMENT) {
            return checkAssignmentNode(dynamic_pointer_cast<AssignmentNode>(node));
        } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
            return checkBinaryOperationNode(dynamic_pointer_cast<BinaryOperationNode>(node));
        } else if (node->getNodeType() == ASTNode::UNARY_OPERATION) {
            return checkUnaryOperationNode(dynamic_pointer_cast<UnaryOperationNode>(node));
        } else if (node->getNodeType() == ASTNode::BLOCK) {
            return checkBlockNode(dynamic_pointer_cast<BlockNode>(node));
        } else if (node->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            return checkFunctionDeclarationNode(dynamic_pointer_cast<FunctionDeclarationNode>(node));
        } else if (node->getNodeType() == ASTNode::CONTROL_FLOW) {
            return checkControlFlowNode(dynamic_pointer_cast<ControlFlowNode>(node));
        } else if (node->getNodeType() == ASTNode::LIST) {
            return checkListNode(dynamic_pointer_cast<ListNode>(node));
        } else if (node->getNodeType() == ASTNode::TUPLE) {
            return checkTupleNode(dynamic_pointer_cast<TupleNode>(node));
        }
        // if (node->getValue()) {
        //     shared_ptr<ASTNode> childResolvedType = node->getValue()->getResolvedType();

        //     if (childResolvedType)
        // }

        return false;
    }

    bool TypeChecker::checkAssignmentNode(shared_ptr<AssignmentNode> node) {
        bool typesMatch = isSameType(node->getLeft()->getValue(), node->getRight()->getResolvedType());

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

        node->setResolvedType(node->getLeft()->getResolvedType());

        return true;
    }

    bool TypeChecker::checkUnaryOperationNode(shared_ptr<UnaryOperationNode> node) {
        bool valid = checkAST(node->getValue());

        if (!valid) return false;

        shared_ptr<TypeDeclarationNode> boolType = make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN);
        shared_ptr<TypeDeclarationNode> numType = make_shared<TypeDeclarationNode>(DataTypes::NUMBER);

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

        vector<shared_ptr<ASTNode>> returns = findAllInTree(node, ASTNode::RETURN);

        for (int i = 0; i < returns.size(); i++) {
            blockReturnTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(returns.at(i)->getResolvedType()));
        }

        // Add return type of last expression for implicit return
        blockReturnTypes.push_back(
            dynamic_pointer_cast<TypeDeclarationNode>(node->getElements().at(node->getElements().size() - 1)->getResolvedType())
        );

        if (blockReturnTypes.size() == 1) {
            node->setResolvedType(blockReturnTypes[0]);
        } else {
            node->setResolvedType(makeVariadicType(blockReturnTypes));
        }

        return true;
    }

    bool TypeChecker::checkFunctionDeclarationNode(shared_ptr<FunctionDeclarationNode> node) {
        bool valid = checkAST(node->getDefinition());

        if (valid) {
            node->setResolvedType(node->getDefinition()->getResolvedType());
        }

        return valid;
    }

    bool TypeChecker::checkControlFlowNode(shared_ptr<ControlFlowNode> node) {
        vector<shared_ptr<TypeDeclarationNode>> returnTypes;

        for (int i = 0; i < node->getConditionExpressionPairs().size(); i++) {
            pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>> pair = node->getConditionExpressionPairs().at(i);

            // It might be a nullptr in the case of the else block
            if (pair.first) {
                bool validCondition = checkAST(pair.first);

                shared_ptr<TypeDeclarationNode> boolType = make_shared<TypeDeclarationNode>(DataTypes::BOOLEAN);

                if (!validCondition || !isSameType(pair.first->getResolvedType(), boolType)) {
                    Compiler::getInstance().addException(
                        make_shared<TypeError>(
                            "Non-boolean expression in control flow condition",
                            pair.first->getResolvedType(),
                            boolType
                        )
                    );

                    return false;
                }
            }

            bool validExpression = checkAST(pair.second);

            if (!validExpression) return false;

            returnTypes.push_back(dynamic_pointer_cast<TypeDeclarationNode>(pair.second->getResolvedType()));
        }

        if (returnTypes.size() == 1) {
            node->setResolvedType(returnTypes[0]);
        } else {
            node->setResolvedType(makeVariadicType(returnTypes));
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
            // TODO: Loop through all the returnTypes and add an exception for each unmatching
            // type 
            Compiler::getInstance().addException(
                make_shared<TypeError>(
                    "Lists must be homogenous",
                    returnTypes.at(0),
                    returnTypes.at(1) // FIXME: This isn't always going to be the wrong type
                )
            );

            return false;
        }
    
        shared_ptr<TypeDeclarationNode> listType = make_shared<TypeDeclarationNode>(DataTypes::LIST);

        listType->setValue(returnTypes.at(0));

        node->setResolvedType(listType);

        return true;
    }

    bool TypeChecker::checkTupleNode(shared_ptr<TupleNode> node) {
        bool validLeft = checkAST(node->getLeft());
        bool validRight = checkAST(node->getRight());

        if (!validLeft || !validRight) return false;
        
        shared_ptr<TypeDeclarationNode> type = make_shared<TypeDeclarationNode>(DataTypes::TUPLE);

        type->setLeft(node->getLeft()->getResolvedType());
        type->setRight(node->getRight()->getResolvedType());

        node->setResolvedType(type);

        return true;
    }

    bool TypeChecker::isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2) {
        shared_ptr<TypeDeclarationNode> t1 = dynamic_pointer_cast<TypeDeclarationNode>(type1);
        shared_ptr<TypeDeclarationNode> t2 = dynamic_pointer_cast<TypeDeclarationNode>(type2);

        if (!t1 && !t2) return true;

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

    vector<shared_ptr<ASTNode>> TypeChecker::findAllInTree(shared_ptr<ASTNode> node, ASTNode::Types nodeType) {
        if (node->getNodeType() == nodeType) return { node };

        if (node->getNodeType() == ASTNode::CONTROL_FLOW) {
            vector<shared_ptr<ASTNode>> found;
            shared_ptr<ControlFlowNode> cfNode = dynamic_pointer_cast<ControlFlowNode>(node);

            for (int i = 0; i < cfNode->getConditionExpressionPairs().size(); i++) {
                vector<shared_ptr<ASTNode>> foundInElem = findAllInTree(cfNode->getConditionExpressionPairs().at(i).second, nodeType);

                found.insert(found.end(), foundInElem.begin(), foundInElem.end());
            }

            return found;
        }

        if (node->getValue()) return findAllInTree(node->getValue(), nodeType);

        if (node->getLeft()) {
            vector<shared_ptr<ASTNode>> found = findAllInTree(node->getLeft(), nodeType);
            vector<shared_ptr<ASTNode>> rightFound = findAllInTree(node->getRight(), nodeType);

            found.insert(found.end(), rightFound.begin(), rightFound.end());

            return found;
        }

        if (node->hasMany()) {
            vector<shared_ptr<ASTNode>> found;
            shared_ptr<ASTNodeList> nodeList = dynamic_pointer_cast<ASTNodeList>(node);

            for (int i = 0; i < nodeList->getElements().size(); i++) {
                vector<shared_ptr<ASTNode>> foundInElem = findAllInTree(nodeList->getElements().at(i), nodeType);
                found.insert(found.end(), foundInElem.begin(), foundInElem.end());
            }

            return found;
        }

        return {};
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

    shared_ptr<TypeDeclarationNode> TypeChecker::makeVariadicType(vector<shared_ptr<TypeDeclarationNode>> types) {
        shared_ptr<TypeDeclarationNode> variadicTypeNode = make_shared<TypeDeclarationNode>(DataTypes::VARIADIC);

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
}
