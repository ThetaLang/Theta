#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class LinkNode : public ASTNode {
    public:
        string nodeType;
        string capsule;

        LinkNode(string cap) : capsule(cap), nodeType("Link") {};

        string getNodeType() const override { return nodeType; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"capsule\": \"" << capsule << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};