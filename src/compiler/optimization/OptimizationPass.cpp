#include "OptimizationPass.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"

using namespace Theta;

void OptimizationPass::optimize(shared_ptr<ASTNode> &ast) {
    if (ast->hasOwnScope()) localScope.enterScope();

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

    if (ast->hasOwnScope()) localScope.exitScope();

    optimizeAST(ast);
}

shared_ptr<ASTNode> OptimizationPass::lookupInScope(string identifierName) {
    shared_ptr<ASTNode> foindHoisted = hoistedScope.lookup(identifierName);
    shared_ptr<ASTNode> foundInLocalScope = localScope.lookup(identifierName);

    // Local scope overrides capsule scope
    if (foundInLocalScope) return foundInLocalScope;

    return foindHoisted;
}
