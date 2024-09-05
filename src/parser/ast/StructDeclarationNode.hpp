#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class StructDeclarationNode : public ASTNode {
        public:
            string structType;

            StructDeclarationNode(string type, shared_ptr<ASTNode> parent) : ASTNode(ASTNode::STRUCT_DECLARATION, parent), structType(type) {};

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
