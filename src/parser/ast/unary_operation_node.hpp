#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class UnaryOperationNode : public ASTNode {
    public:
        string nodeType;
        string operatorSymbol;

        UnaryOperationNode(string op) : operatorSymbol(op), nodeType("UnaryOperation") {};

        string getNodeType() const override { return nodeType; }

        string getOperator() { return operatorSymbol; }

        string toJSON() const override {
            std::ostringstream oss;
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
            oss << "\"operator\": \"" << operatorSymbol << "\", ";
            oss << "\"value\": " << (value ? value->toJSON() : "null");
            oss << "}";
            return oss.str();
        }
};