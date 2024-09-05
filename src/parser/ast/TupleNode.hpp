#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class TupleNode : public ASTNode {
        public:
            TupleNode(shared_ptr<ASTNode> parent) : ASTNode(ASTNode::TUPLE, parent) {};

            string toJSON() const override {
                std::ostringstream oss;
                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
                oss << "\"first\": " << (left ? left->toJSON() : "null") << ", ";
                oss << "\"second\": " << (right ? right->toJSON() : "null");
                oss << "}";
                return oss.str();
            }
    };
}
