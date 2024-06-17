#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class ListDefinitionNode : public ASTNode {
    public:
        string nodeType;
        vector<shared_ptr<ASTNode>> elements;

        ListDefinitionNode() : nodeType("ListDefinition") {};

        string getNodeType() const override { return nodeType; }

        void setElements(vector<shared_ptr<ASTNode>> el) { elements = el; }

        vector<shared_ptr<ASTNode>> getElements() { return elements; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
            oss << "\"elements\": [";
            
            for (int i = 0; i < elements.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (elements[i] ? elements[i]->toJSON() : "null");
            }

            oss << "] ";
            oss << "}";

            return oss.str();
        }
};