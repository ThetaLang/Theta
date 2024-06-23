#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class UnaryOperationNode : public ASTNode {
    public:
        string operatorSymbol;

        UnaryOperationNode(string op) : ASTNode(ASTNode::Types::UNARY_OPERATION),  operatorSymbol(op) {};

        string getOperator() { return operatorSymbol; }

        string toJSON() const override {
            std::ostringstream oss;
            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
            oss << "\"operator\": \"" << operatorSymbol << "\", ";
            oss << "\"value\": " << (value ? value->toJSON() : "null");
            oss << "}";
            return oss.str();
        }
};
