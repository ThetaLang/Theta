#pragma once

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/ASTNodeList.hpp"
#include "../parser/ast/IdentifierNode.hpp"
#include "../parser/ast/AssignmentNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/TypeDeclarationNode.hpp"


using namespace std;

namespace Theta {
    class TypeChecker {
        public:
            static bool checkAST(shared_ptr<ASTNode> ast) {
                // Check node children first
                if (ast->getValue()) {
                    bool childValid = checkAST(ast->getValue());

                    if (!childValid) return false;
                } else if (ast->getLeft()) {
                    bool lhsValid = checkAST(ast->getLeft());
                    bool rhsValid = checkAST(ast->getRight());

                    cout << "LHS: " + to_string(lhsValid) + ", RHS: " + to_string(rhsValid) << endl;

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

        private:
            static bool checkNode(shared_ptr<ASTNode> node) {
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
                } else if (node->getNodeType() == ASTNode::SOURCE) {
                    node->setResolvedType(node->getValue()->getResolvedType());
                    return true;
                } else if (node->getNodeType() == ASTNode::BINARY_OPERATION) {
                    return checkBinaryOperationNode(dynamic_pointer_cast<BinaryOperationNode>(node));
                }
                // if (node->getValue()) {
                //     shared_ptr<ASTNode> childResolvedType = node->getValue()->getResolvedType();

                //     if (childResolvedType)
                // }

                return false;
            }

            static bool checkAssignmentNode(shared_ptr<AssignmentNode> node) {
                bool typesMatch = isSameType(node->getLeft()->getValue(), node->getRight()->getResolvedType() );

                if (!typesMatch) return false;

                node->setResolvedType(node->getLeft()->getValue());

                return true;
            }

            static bool checkBinaryOperationNode(shared_ptr<BinaryOperationNode> node) {
                bool typesMatch = isSameType(node->getLeft()->getResolvedType(), node->getRight()->getResolvedType());

                if (!typesMatch) return false;

                node->setResolvedType(node->getLeft()->getResolvedType());

                return true;
            }

            static bool isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2) {
                shared_ptr<TypeDeclarationNode> t1 = dynamic_pointer_cast<TypeDeclarationNode>(type1);
                shared_ptr<TypeDeclarationNode> t2 = dynamic_pointer_cast<TypeDeclarationNode>(type2);

                if (
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
    };
}
