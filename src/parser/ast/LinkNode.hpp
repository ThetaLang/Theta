#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class LinkNode : public ASTNode {
        public:
            string capsule;

            LinkNode(string cap) : capsule(cap), ASTNode(ASTNode::Types::LINK) {};

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\"";
                oss << ", \"capsule\": \"" << capsule << "\"";
                oss << ", \"value\": " << (value ? value->toJSON() : "null");
                oss << "}";

                return oss.str();
            }
    };
}
