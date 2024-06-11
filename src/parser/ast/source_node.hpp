#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class SourceNode : public ASTNode {
    public:
        string nodeType;
        vector<shared_ptr<ASTNode>> imports;

        SourceNode(string ident) : identifier(ident), nodeType("Source") {};

        string getNodeType() const override { return nodeType; }

        void setImports(vector<shared_ptr<ASTNode>> imp) { imports = imp; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"imports\": \"" << identifier << "\"";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};