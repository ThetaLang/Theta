#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class SourceNode : public ASTNode {
    public:
        string nodeType;
        vector<shared_ptr<ASTNode>> links;

        SourceNode() : nodeType("Source") {};

        string getNodeType() const override { return nodeType; }

        void setLinks(vector<shared_ptr<ASTNode>> ln) { links = ln; }

        vector<shared_ptr<ASTNode>> getLinks() { return links; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"links\": [";
            
            for (int i = 0; i < links.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (links[i] ? links[i]->toJSON() : "null");
            }

            oss << "] ";
            oss << ", \"value\": " << (value ? value->toJSON() : "null");
            oss << "}";

            return oss.str();
        }
};