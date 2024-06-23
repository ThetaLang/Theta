#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class KeyedAccessNode : public ASTNode {
    public:
        string identifier;

        KeyedAccessNode() : ASTNode(ASTNode::Types::KEYED_ACCESS) {};

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\"";
            oss << ", \"left\": " << (left ? left->toJSON() : "null");
            oss << ", \"right\": " << (right ? right->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};
