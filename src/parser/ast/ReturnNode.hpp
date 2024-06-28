#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class ReturnNode : public ASTNode {
    public:
        ReturnNode() : ASTNode(ASTNode::Types::RETURN) {};

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};
