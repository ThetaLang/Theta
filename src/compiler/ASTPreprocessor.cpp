#include "ASTPreprocessor.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include "parser/ast/EnumNode.hpp"
#include "parser/ast/IdentifierNode.hpp"
#include "parser/ast/LiteralNode.hpp"
#include "parser/ast/SymbolNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include <memory>
#include <iostream>

using namespace Theta;

void ASTPreprocessor::optimize(shared_ptr<ASTNode> &ast) {
    if (ast->hasOwnScope()) scopedIdentifierTable.enterScope();

    if (ast->getNodeType() == ASTNode::CAPSULE) {
        hoistNecessary(ast);
    }

    if (ast->getValue()) {
        optimize(ast->getValue());
    } else if (ast->getLeft()) {
        optimize(ast->getLeft());
        optimize(ast->getRight());
    } else if (ast->hasMany()) {
        shared_ptr<ASTNodeList> nodeList = dynamic_pointer_cast<ASTNodeList>(ast);
        vector<shared_ptr<ASTNode>> elements = nodeList->getElements();
        vector<shared_ptr<ASTNode>> newElements;

        for (int i = 0; i < elements.size(); i++) {
            optimize(elements.at(i));

            if (elements.at(i) != nullptr) {
                newElements.push_back(elements.at(i));
            }
        }

        nodeList->setElements(newElements);
    } else if (ast->getNodeType() == ASTNode::FUNCTION_DECLARATION) {
        shared_ptr<FunctionDeclarationNode> funcDecNode = dynamic_pointer_cast<FunctionDeclarationNode>(ast);

        shared_ptr<ASTNode> params = dynamic_pointer_cast<ASTNode>(funcDecNode->getParameters());
        optimize(params);

        optimize(funcDecNode->getDefinition());
    }

    if (ast->hasOwnScope()) scopedIdentifierTable.exitScope();

    substituteLiterals(ast);
}

// Finds any number literals that are stored in variables / enums and substitutes them with their literal value. This speeds up typechecking
void ASTPreprocessor::substituteLiterals(shared_ptr<ASTNode> &ast) {
    if (ast->getNodeType() == ASTNode::IDENTIFIER) {
        shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(ast);

        shared_ptr<ASTNode> foundIdentifier = hoistedIdentifierTable.lookup(ident->getIdentifier());

        shared_ptr<ASTNode> foundInScope = scopedIdentifierTable.lookup(ident->getIdentifier());
        if (foundInScope) {
            foundIdentifier = foundInScope;
        }

        // Only optimize if we found the literal value we need to replace with
        if (
            !foundIdentifier ||
            !(
                foundIdentifier->getNodeType() == ASTNode::NUMBER_LITERAL ||
                foundIdentifier->getNodeType() == ASTNode::STRING_LITERAL ||
                foundIdentifier->getNodeType() == ASTNode::BOOLEAN_LITERAL
            )
        ) return;

        shared_ptr<LiteralNode> enumValue = dynamic_pointer_cast<LiteralNode>(foundIdentifier);

        ast = make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, enumValue->getLiteralValue());
    } else if (ast->getNodeType() == ASTNode::ENUM) {
        shared_ptr<EnumNode> node = dynamic_pointer_cast<EnumNode>(ast);

        unpackEnumElementsInScope(
            dynamic_pointer_cast<IdentifierNode>(node->getIdentifier())->getIdentifier(),
            dynamic_pointer_cast<ASTNodeList>(node)->getElements(),
            scopedIdentifierTable
        );

        ast = nullptr;
    } else if (
        ast->getNodeType() == ASTNode::ASSIGNMENT &&
        (
            ast->getRight()->getNodeType() == ASTNode::BOOLEAN_LITERAL ||
            ast->getRight()->getNodeType() == ASTNode::STRING_LITERAL ||
            ast->getRight()->getNodeType() == ASTNode::NUMBER_LITERAL
        )
    ) {
        string identifier = dynamic_pointer_cast<IdentifierNode>(ast->getLeft())->getIdentifier();

        shared_ptr<ASTNode> foundIdentInScope = scopedIdentifierTable.lookup(identifier);

        if (foundIdentInScope) {
            // TODO: Reassignment error
            cout << "CAN NOT REASSIGN IDENTIFIER IN SCOPE" << endl;
            return;
        }

        scopedIdentifierTable.insert(identifier, ast->getRight());

        ast = nullptr;
    }
}

void ASTPreprocessor::hoistNecessary(shared_ptr<ASTNode> ast) {
    shared_ptr<ASTNodeList> nodeList = dynamic_pointer_cast<ASTNodeList>(ast->getValue());

    vector<shared_ptr<ASTNode>> topLevelElements = nodeList->getElements();
    vector<int> removeAtIndices;

    hoistedIdentifierTable.enterScope();

    for (int i = 0; i < topLevelElements.size(); i++) {
        if (topLevelElements.at(i)->getNodeType() == ASTNode::ENUM) {
            shared_ptr<EnumNode> node = dynamic_pointer_cast<EnumNode>(topLevelElements.at(i));
 
            unpackEnumElementsInScope(
                dynamic_pointer_cast<IdentifierNode>(node->getIdentifier())->getIdentifier(),
                dynamic_pointer_cast<ASTNodeList>(node)->getElements(),
                hoistedIdentifierTable
            );
            removeAtIndices.push_back(i);
        } else if (
            ast->getNodeType() == ASTNode::ASSIGNMENT &&
            (
                ast->getRight()->getNodeType() == ASTNode::BOOLEAN_LITERAL ||
                ast->getRight()->getNodeType() == ASTNode::STRING_LITERAL ||
                ast->getRight()->getNodeType() == ASTNode::NUMBER_LITERAL
            )
        ) {
            string identifier = dynamic_pointer_cast<IdentifierNode>(ast->getLeft())->getIdentifier();

            shared_ptr<ASTNode> foundIdent = hoistedIdentifierTable.lookup(identifier);

            if (foundIdent) {
                // TODO: Reassignment error
                cout << "CAN NOT REASSIGN IDENTIFIER IN SCOPE" << endl;
                return;
            }

            hoistedIdentifierTable.insert(identifier, ast->getRight());

            removeAtIndices.push_back(i);
        }
    }

    // Remove the enum node from the ast, it no longer exists since we've processed it
    for (int i = removeAtIndices.size() - 1; i >= 0; i--) {
        topLevelElements.erase(topLevelElements.begin() + removeAtIndices.at(i));
    }

    // Set the modified vector back
    nodeList->setElements(topLevelElements);
}

void ASTPreprocessor::unpackEnumElementsInScope(string baseIdentifier, vector<shared_ptr<ASTNode>> enumElements, SymbolTableStack &scope) { 
    for (int i = 0; i < enumElements.size(); i++) {
        shared_ptr<SymbolNode> elSymbol = dynamic_pointer_cast<SymbolNode>(enumElements.at(i));

        string enumElIdentifier = baseIdentifier + "." + elSymbol->getSymbol().substr(1);
        
        shared_ptr<ASTNode> foundScopeIdentifier = scope.lookup(enumElIdentifier);
        if (foundScopeIdentifier) {
            // TODO: Throw error
            cout << "CANT REDEFINE EXISTING IDENTIFIER" << endl;
            return;
        }

        scope.insert(enumElIdentifier, make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, to_string(i)));
    }
}
