#pragma once

#include <sstream>
#include <string>

#include "../../lexer/Token.hpp"
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class LiteralNode : public ASTNode {
    public:
        string literalValue;

        LiteralNode(ASTNode::Types typ, string val) : ASTNode(typ), literalValue(val){};

        string getLiteralValue() { return literalValue; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
            oss << "\"value\": \"" << literalValue << "\" ";
            oss << "}";

            return oss.str();
        }
    };

}
