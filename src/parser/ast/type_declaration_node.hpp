#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class TypeDeclarationNode : public ASTNode {
    public:
        string nodeType;
        string type;

        TypeDeclarationNode(string typ) : type(typ), nodeType("TypeDeclaration") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ",\"declaredType\": \"" << type << "\"";
            oss << ",\"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};