#include "parser/ast/ASTNode.hpp"
#include "SymbolTableStack.hpp"
#include "parser/ast/EnumNode.hpp"
#include <memory>
#include <vector>
#include <iostream>

using namespace std;

namespace Theta {
    class ASTPreprocessor {
    public:
        void optimize(shared_ptr<ASTNode> &ast);

    private:
        SymbolTableStack scopedIdentifierTable;
        SymbolTableStack hoistedIdentifierTable;
        
        void substituteEnumValues(shared_ptr<ASTNode> &ast);

        void hoistNecessary(shared_ptr<ASTNode> ast);

        void unpackEnumElementsInScope(string baseIdentifier, vector<shared_ptr<ASTNode>> enumElements, SymbolTableStack &scope);
    };
}
