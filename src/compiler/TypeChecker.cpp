#include "TypeChecker.hpp"
#include <memory>

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
        if (node->getNodeType() == ASTNode::ASSIGNMENT) {
            return checkAssignmentNode(dynamic_pointer_cast<AssignmentNode>(node));
        } else if (node->getNodeType() == ASTNode::IDENTIFIER) {
            return true;
        } else if (node->getNodeType() == ASTNode::TYPE_DECLARATION) {
            return true;
        } else if (node->getNodeType() == ASTNode::NUMBER_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>("Number"));
            return true;
        } else if (node->getNodeType() == ASTNode::STRING_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>("String"));
            return true;
        } else if (node->getNodeType() == ASTNode::BOOLEAN_LITERAL) {
            node->setResolvedType(make_shared<TypeDeclarationNode>("Boolean"));
            return true;
        } else if (node->getNodeType() == ASTNode::CAPSULE) {
            node->setResolvedType(make_shared<TypeDeclarationNode>("Capsule"));
            return true;
        } else if (node->getNodeType() == ASTNode::SOURCE) {
            node->setResolvedType(node->getValue()->getResolvedType());
            return true;
        } else if (node->getNodeType() == ASTNode::RETURN) {
            node->setResolvedType(node->getValue()->getResolvedType());
            return true;
        } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
            return checkBinaryOperationNode(dynamic_pointer_cast<BinaryOperationNode>(node));
        } else if (node->getNodeType() == ASTNode::BLOCK) {
            return checkBlockNode(dynamic_pointer_cast<BlockNode>(node));
        } else if (node->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
            return checkFunctionDeclarationNode(dynamic_pointer_cast<FunctionDeclarationNode>(node));
        } else if (node->getNodeType() == ASTNode::CONTROL_FLOW) {
            return checkControlFlowNode(dynamic_pointer_cast<ControlFlowNode>(node));
        }
        // if (node->getValue()) {
        //     shared_ptr<ASTNode> childResolvedType = node->getValue()->getResolvedType();

        //     if (childResolvedType)
        // }

        return true;
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

                if (!validCondition || !isSameType(pair.first->getResolvedType(), make_shared<TypeDeclarationNode>("Boolean"))) {
                    Compiler::getInstance().addException(
                        make_shared<TypeError>(
                            "Non-boolean expression in control flow condition",
                            pair.first->getResolvedType(),
                            make_shared<TypeDeclarationNode>("Boolean")
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
            )
        ) return false;

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

    shared_ptr<TypeDeclarationNode> TypeChecker::makeVariadicType(vector<shared_ptr<TypeDeclarationNode>> types) {
        shared_ptr<TypeDeclarationNode> variadicTypeNode = make_shared<TypeDeclarationNode>("Variadic");

        auto ip = unique(types.begin(), types.end(), isSameType);
        types.resize(distance(types.begin(), ip));

        // If theres only 1 unique type, this isn't variadic
        if (types.size() == 1) return types[0];

        cout << "MAKING VARIADIC TYPE" << endl;

        for (int i = 0; i < types.size(); i++) {
            cout << types.at(i)->toJSON() << endl;
        }

        return variadicTypeNode;
    }
}
