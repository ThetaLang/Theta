#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class TypeDeclarationNode : public ASTNode {
    public:
        string type;

        TypeDeclarationNode(string typ) : type(typ), ASTNode(ASTNode::Types::TYPE_DECLARATION){};

        string getType() { return type; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\"";
            oss << ",\"declaredType\": \"" << type << "\"";

            if (left) {
                oss << ",\"left\": " << left->toJSON();
                oss << ",\"right\": " << (right ? right->toJSON() : "null");
            } else {
                oss << ",\"value\": " << (value ? value->toJSON() : "null");
            }

            oss << "}";

            return oss.str();
        }
    };
}
