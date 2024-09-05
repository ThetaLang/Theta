#pragma once

#include "SymbolTable.hpp"
#include <stack>

using namespace std;

namespace Theta {
    template<typename T>
    class SymbolTableStack {
        public:
            void enterScope() {
                scopes.push(make_shared<SymbolTable<T>>());
            }

            void exitScope() {
                if (!scopes.empty()) scopes.pop();
            }

            void insert(const string &name, T value) {
                if (!scopes.empty()) scopes.top()->insert(name, value);
            }

            optional<T> lookup(const string &name) {
                stack<shared_ptr<SymbolTable<T>>> tmpScopes = scopes;

                while(!tmpScopes.empty()) {
                    auto result = tmpScopes.top()->lookup(name);
                    
                    if (result.has_value()) return result.value();

                    tmpScopes.pop();
                }

                return nullopt;
            }

        private:
            stack<shared_ptr<SymbolTable<T>>> scopes;
    };
}
