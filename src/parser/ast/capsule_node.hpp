#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class CapsuleNode : public ASTNode {
    public:
        string nodeType;
        string name;
        vector<shared_ptr<ASTNode>> definitions;

        CapsuleNode(string n) : name(n), nodeType("Capsule") {};

        string getNodeType() const override { return nodeType; }

        void setDefinitions(vector<shared_ptr<ASTNode>> def) { definitions = def; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"name\": \"" << name << "\"";
            oss << ", \"definitions\": [";
            
            for (int i = 0; i < definitions.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (definitions[i] ? definitions[i]->toJSON() : "null");
            }

            oss << "] ";
            oss << "}";

            return oss.str();
        }
};