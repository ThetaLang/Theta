#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class BinaryOperationNode : public ASTNode {
    public:
        string operatorSymbol;

        BinaryOperationNode(string op) : operatorSymbol(op), ASTNode(ASTNode::Types::BINARY_OPERATION) {};

        string getOperator() { return operatorSymbol; }

        string toJSON() const override {
            std::ostringstream oss;
            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
            oss << "\"operator\": \"" << operatorSymbol << "\", ";
            oss << "\"left\": " << (left ? left->toJSON() : "null") << ", ";
            oss << "\"right\": " << (right ? right->toJSON() : "null");
            oss << "}";
            return oss.str();
        }
};
