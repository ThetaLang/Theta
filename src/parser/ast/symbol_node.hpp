#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class SymbolNode : public ASTNode {
    public:
        string nodeType;
        string symbol;

        SymbolNode(string sym) : symbol(":" + sym), nodeType("Symbol") {};

        string getNodeType() const override { return nodeType; }

        string getSymbol() { return symbol; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"value\": \"" << symbol << "\"";
            oss << "}";

            return oss.str();
        }
};
