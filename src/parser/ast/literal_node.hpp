#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"
#include "../../util/tokens.hpp"

using namespace std;

class LiteralNode : public ASTNode {
    public:
        string nodeType;
        Tokens type;
        string literalValue;

        LiteralNode(Tokens typ, string val) : type(typ), literalValue(val), nodeType("Literal") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << tokenTypeToString(type) << nodeType << "\", ";
            oss << "\"value\": \"" << literalValue << "\" ";
            oss << "}";

            return oss.str();
        }
};