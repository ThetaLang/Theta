#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class AssignmentNode : public ASTNode {
    public:
        AssignmentNode() : ASTNode(ASTNode::Types::ASSIGNMENT){};

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

}
