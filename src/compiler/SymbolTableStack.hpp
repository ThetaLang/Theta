#include "parser/ast/ASTNode.hpp"
#include "SymbolTable.hpp"
#include <stack>

using namespace std;

namespace Theta {
    class SymbolTableStack {
        public:
            void enterScope() {
                scopes.push(make_shared<SymbolTable>());
            }

            void exitScope() {
                if (!scopes.empty()) scopes.pop();
            }

            void insert(const string &name, shared_ptr<ASTNode> type) {
                if (!scopes.empty()) scopes.top()->insert(name, type);
            }

            shared_ptr<ASTNode> lookup(const string &name) {
                stack<shared_ptr<SymbolTable>> tmpScopes = scopes;

                while(!tmpScopes.empty()) {
                    auto type = tmpScopes.top()->lookup(name);
                    
                    if (type) return type;

                    tmpScopes.pop();
                }

                return nullptr;
            }

        private:
            stack<shared_ptr<SymbolTable>> scopes;
    };
}
