#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class StructDeclarationNode : public ASTNode {
    public:
        string structType;

        StructDeclarationNode(string type) : structType(type), ASTNode(ASTNode::Types::STRUCT_DECLARATION){};

        string getStructType() { return structType; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\"";
            oss << ", \"struct\": \"" << structType << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
    };
}
