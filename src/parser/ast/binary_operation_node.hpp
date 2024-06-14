#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class BinaryOperationNode : public ASTNode {
    public:
        string nodeType;
        string operatorSymbol;

        BinaryOperationNode(string op) : operatorSymbol(op), nodeType("BinaryOperation") {};

        string getNodeType() const override { return nodeType; }

        string getOperator() { return operatorSymbol; }

        string toJSON() const override {
            std::ostringstream oss;
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
            oss << "\"operator\": \"" << operatorSymbol << "\", ";
            oss << "\"left\": " << (left ? left->toJSON() : "null") << ", ";
            oss << "\"right\": " << (right ? right->toJSON() : "null");
            oss << "}";
            return oss.str();
        }
};