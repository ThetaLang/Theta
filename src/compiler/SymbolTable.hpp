#pragma once

#include <memory>
#include "../parser/ast/TypeDeclarationNode.hpp"
#include "parser/ast/ASTNode.hpp"
using namespace std;

namespace Theta {
    class SymbolTable {
    public:
        void insert(const string &name, shared_ptr<ASTNode> type) {
            table[name] = type;
        }

        shared_ptr<ASTNode> lookup(const string &name) {
            auto it = table.find(name);

            if (it != table.end()) return it->second;

            return nullptr;
        }

    private:
        map<string, shared_ptr<ASTNode>> table;
    };
}
