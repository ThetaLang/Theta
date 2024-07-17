#include "parser/ast/ASTNode.hpp"
#include "SymbolTableStack.hpp"
#include <memory>
#include <vector>

using namespace std;

namespace Theta {
    class ASTPreprocessor {
    public:
        void optimize(shared_ptr<ASTNode> &ast);

    private:
        SymbolTableStack scopedIdentifierTable;
        SymbolTableStack hoistedIdentifierTable;
        
        void substituteLiterals(shared_ptr<ASTNode> &ast);

        void hoistNecessary(shared_ptr<ASTNode> ast);

        void unpackEnumElementsInScope(string baseIdentifier, vector<shared_ptr<ASTNode>> enumElements, SymbolTableStack &scope);
    };
}
