#pragma once

#include <memory>
#include <map>
using namespace std;

namespace Theta {
    template<typename T>
    class SymbolTable {
    public:
        void insert(const string &name, T value) {
            table[name] = value;
        }

        optional<T> lookup(const string &name) {
            auto it = table.find(name);

            if (it != table.end()) return it->second;

            return nullopt;
        }

    private:
        map<string, T> table;
    };
}
