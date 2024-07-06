#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class UnaryOperationNode : public ASTNode {
    public:
        string operatorSymbol;

        UnaryOperationNode(string op) : ASTNode(ASTNode::Types::UNARY_OPERATION), operatorSymbol(op){};

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
}
