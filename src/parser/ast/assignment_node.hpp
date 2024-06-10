#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"
#include "identifier_node.hpp"

using namespace std;

class AssignmentNode : public ASTNode {
    public:
        string nodeType;

        AssignmentNode() : nodeType("Assignment") {};

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