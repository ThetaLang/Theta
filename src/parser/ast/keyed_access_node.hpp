#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class KeyedAccessNode : public ASTNode {
    public:
        string nodeType;
        string identifier;

        KeyedAccessNode() : nodeType("KeyedAccess") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"left\": " << (left ? left->toJSON() : "null");
            oss << ", \"right\": " << (right ? right->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};