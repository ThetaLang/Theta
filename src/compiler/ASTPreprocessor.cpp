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
        auto value = ast->getValue();
        optimize(value);
        ast->setValue(value);
    } else if (ast->getLeft()) {
        auto left = ast->getLeft();
        optimize(left);
        ast->setLeft(left);

        auto right = ast->getRight();
        optimize(right);
        ast->setRight(right);
    } else if (ast->hasMany()) {
        shared_ptr<ASTNodeList> nodeList = dynamic_pointer_cast<ASTNodeList>(ast);
        auto elements = nodeList->getElements();
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

        auto params = dynamic_pointer_cast<ASTNode>(funcDecNode->getParameters());
        optimize(params);
        funcDecNode->setParameters(dynamic_pointer_cast<ASTNodeList>(params));

        auto def = funcDecNode->getDefinition();
        optimize(def);
        funcDecNode->setDefinition(def);
    }

    if (ast->hasOwnScope()) scopedIdentifierTable.exitScope();

    substituteEnumValues(ast);
}

void ASTPreprocessor::substituteEnumValues(shared_ptr<ASTNode> &ast) {
    if (ast->getNodeType() == ASTNode::IDENTIFIER) {
        shared_ptr<IdentifierNode> ident = dynamic_pointer_cast<IdentifierNode>(ast);

//        hoistedIdentifierTable.listKeys();

        shared_ptr<ASTNode> foundIdentifier = hoistedIdentifierTable.lookup(ident->getIdentifier());

        shared_ptr<ASTNode> foundInScope = scopedIdentifierTable.lookup(ident->getIdentifier());
        if (foundInScope) {
            foundIdentifier = foundInScope;
        }

        // Only optimize if we found the literal value we need to replace with
        if (!foundIdentifier || foundIdentifier->getNodeType() != ASTNode::NUMBER_LITERAL) return;

        cout << "FOUND IDENTIFIER: " + ident->getIdentifier() << endl;

        shared_ptr<LiteralNode> enumValue = dynamic_pointer_cast<LiteralNode>(foundIdentifier);

        ast = make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, enumValue->getLiteralValue());

        cout << ast->toJSON() << endl;
    } else if (ast->getNodeType() == ASTNode::ENUM) {
        shared_ptr<EnumNode> node = dynamic_pointer_cast<EnumNode>(ast);

        unpackEnumElementsInScope(
            dynamic_pointer_cast<IdentifierNode>(node->getIdentifier())->getIdentifier(),
            dynamic_pointer_cast<ASTNodeList>(node)->getElements(),
            scopedIdentifierTable
        );

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
 
            cout << "HOISTING " << node->toJSON() << endl;

            unpackEnumElementsInScope(
                dynamic_pointer_cast<IdentifierNode>(node->getIdentifier())->getIdentifier(),
                dynamic_pointer_cast<ASTNodeList>(node)->getElements(),
                hoistedIdentifierTable
            );
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
        
        cout << "unpacking as " << enumElIdentifier << endl;
        
        shared_ptr<ASTNode> foundScopeIdentifier = scope.lookup(enumElIdentifier);
        if (foundScopeIdentifier) {
            // TODO: Throw error
            cout << "CANT REDEFINE EXISTING IDENTIFIER" << endl;
            return;
        }

        scope.insert(enumElIdentifier, make_shared<LiteralNode>(ASTNode::NUMBER_LITERAL, to_string(i)));
    }
}
