#include "OptimizationPass.hpp"
#include "parser/ast/ControlFlowNode.hpp"
#include "parser/ast/FunctionDeclarationNode.hpp"
#include "parser/ast/FunctionInvocationNode.hpp"
#include <memory>
#include <iostream>
#include <utility>

using namespace Theta;

void OptimizationPass::optimize(shared_ptr<ASTNode> &ast, bool isCapsuleDirectChild) {
    if (ast->hasOwnScope()) localScope.enterScope();

    if (ast->getNodeType() == ASTNode::CAPSULE) {
        hoistNecessary(ast);

        shared_ptr<ASTNodeList> capsuleBlock = dynamic_pointer_cast<ASTNodeList>(ast->getValue());
        vector<shared_ptr<ASTNode>> elements = capsuleBlock->getElements();
        vector<shared_ptr<ASTNode>> newElements;

        for (int i = 0; i < elements.size(); i++) {
            optimize(elements.at(i), true);

            if (elements.at(i) != nullptr) {
                newElements.push_back(elements.at(i));
            }
        }

        capsuleBlock->setElements(newElements);

        return;
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
    } else if (ast->getNodeType() == ASTNode::CONTROL_FLOW) {
        shared_ptr<ControlFlowNode> cFlowNode = dynamic_pointer_cast<ControlFlowNode>(ast);
        vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> newPairs;

        for (auto conditionExpressionPair : cFlowNode->getConditionExpressionPairs()) {
            shared_ptr<ASTNode> condition = conditionExpressionPair.first;
            shared_ptr<ASTNode> expression = conditionExpressionPair.second;

            if (condition) optimize(condition);
            optimize(expression);

            newPairs.push_back(make_pair(condition, expression));
        }

        cFlowNode->setConditionExpressionPairs(newPairs);
    }

    if (ast->hasOwnScope()) localScope.exitScope();

    optimizeAST(ast, isCapsuleDirectChild);
}

shared_ptr<ASTNode> OptimizationPass::lookupInScope(string identifierName) {
    shared_ptr<ASTNode> foindHoisted = hoistedScope.lookup(identifierName);
    shared_ptr<ASTNode> foundInLocalScope = localScope.lookup(identifierName);

    // Local scope overrides capsule scope
    if (foundInLocalScope) return foundInLocalScope;

    return foindHoisted;
}
