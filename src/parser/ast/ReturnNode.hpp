#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class ReturnNode : public ASTNode {
        public:
            ReturnNode() : ASTNode(ASTNode::RETURN) {};

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\"";
                oss << ", \"value\": " << (value ? value->toJSON() : "null");
                oss << "}";

                return oss.str();
            }
    };
}
