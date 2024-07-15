#include "parser/ast/ASTNode.hpp"
#include "SymbolTableStack.hpp"
#include <memory>

using namespace std;

namespace Theta {
    class ASTPreprocessor {
    public:
        void optimize(shared_ptr<ASTNode> ast);

    private:
        SymbolTableStack scopedIdentifierTable;
        SymbolTableStack hoistedIdentifierTable;
        
        void substituteEnumValues(shared_ptr<ASTNode> &ast);

        void hoistNecessary(shared_ptr<ASTNode> ast);
    };
}
