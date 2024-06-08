#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class AssignmentNode : public ASTNode {
    public:
        string nodeType;
        string identifier;
        string type;

        AssignmentNode(string ident, string typ) : identifier(ident), type(typ), nodeType("Assignment") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
            oss << "\"identifier\": \"" << identifier << "\", ";
            oss << "\"variableType\": \"" << type << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};