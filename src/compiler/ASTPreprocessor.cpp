#include "ASTPreprocessor.hpp"
#include "parser/ast/ASTNodeList.hpp"
#include <memory>

using namespace Theta;

void ASTPreprocessor::optimize(shared_ptr<ASTNode> ast) {
    if (ast->getValue()) {
        optimize(ast->getValue());
    } else if (ast->getLeft()) {
        optimize(ast->getLeft());
        optimize(ast->getRight());
    } else if (ast->hasMany()) {
        shared_ptr<ASTNodeList> nodeList = dynamic_pointer_cast<ASTNodeList>(ast);

        for (int i = 0; i < nodeList->getElements().size(); i++) {
            optimize(nodeList->getElements().at(i));
        }
    }

    substituteEnumValues(ast);
}

void ASTPreprocessor::substituteEnumValues(shared_ptr<ASTNode> &ast) {

}
