#pragma once

#include "OptimizationPass.hpp"
#include "parser/ast/ASTNode.hpp"
#include "compiler/SymbolTableStack.hpp"
#include <memory>

using namespace std;

namespace Theta {
    class LiteralInlinerPass : public OptimizationPass {
    private:
        void optimizeAST(shared_ptr<ASTNode> &ast) override;

        void hoistNecessary(shared_ptr<ASTNode> &ast) override;

        void substituteIdentifiers(shared_ptr<ASTNode> &ast);

        void bindIdentifierToScope(shared_ptr<ASTNode> &ast, SymbolTableStack &scope);

        void unpackEnumElementsInScope(shared_ptr<ASTNode> ast, SymbolTableStack &scope);

        void remapEnumTypeReferences(shared_ptr<ASTNode> &ast);
    };
}
