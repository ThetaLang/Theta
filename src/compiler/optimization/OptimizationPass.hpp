#pragma once

#include "parser/ast/ASTNode.hpp"
#include "compiler/SymbolTableStack.hpp"

namespace Theta {
    class OptimizationPass {
        public:
            void optimize(shared_ptr<ASTNode> &ast);

        protected:
            SymbolTableStack localScope;
            SymbolTableStack hoistedScope;

        private:
            virtual void optimizeAST(shared_ptr<ASTNode> &ast) = 0;
            
            virtual void hoistNecessary(shared_ptr<ASTNode> &ast) {}
    };
}
