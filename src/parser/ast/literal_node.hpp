#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class LiteralNode : public ASTNode {
    public:
        string nodeType;
        string type;
        string literalValue;

        LiteralNode(string typ, string val) : type(typ), literalValue(val), nodeType("Literal") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << type << nodeType << "\", ";
            oss << "\"value\": \"" << literalValue << "\" ";
            oss << "}";

            return oss.str();
        }
};