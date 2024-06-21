#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class CapsuleNode : public ASTNode {
    public:
        string nodeType;
        string name;

        CapsuleNode(string n) : name(n), nodeType("Capsule") {};

        string getNodeType() const override { return nodeType; }

        string getName() { return name; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"name\": \"" << name << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};
