#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class CapsuleNode : public ASTNode {
    public:
        string name;

        CapsuleNode(string n) : name(n), ASTNode(ASTNode::Types::CAPSULE){};

        string getName() { return name; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\"";
            oss << ", \"name\": \"" << name << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
    };

}
