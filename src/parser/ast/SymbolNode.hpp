#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class SymbolNode : public ASTNode {
        public:
            string symbol;

            SymbolNode(string sym) : symbol(":" + sym), ASTNode(ASTNode::SYMBOL) {};

            string getSymbol() { return symbol; }

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\"";
                oss << ", \"value\": \"" << symbol << "\"";
                oss << "}";

                return oss.str();
            }
    };
}
