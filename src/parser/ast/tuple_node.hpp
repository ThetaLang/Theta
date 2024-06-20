#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class TupleNode : public ASTNode {
    public:
        string nodeType;

        TupleNode() : nodeType("Tuple") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            std::ostringstream oss;
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
            oss << "\"first\": " << (left ? left->toJSON() : "null") << ", ";
            oss << "\"second\": " << (right ? right->toJSON() : "null");
            oss << "}";
            return oss.str();
        }
};
