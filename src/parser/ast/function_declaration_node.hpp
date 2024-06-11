#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class FunctionDeclarationNode : public ASTNode {
    public:
        string nodeType;
        vector<shared_ptr<ASTNode>> parameters;
        vector<shared_ptr<ASTNode>> definition;

        FunctionDeclarationNode() : nodeType("FunctionDeclaration") {};

        string getNodeType() const override { return nodeType; }

        void setParameters(vector<shared_ptr<ASTNode>> params) { parameters = params; }

        void setDefinition(vector<shared_ptr<ASTNode>> def) { definition = def; }

        string toJSON() const override {
            ostringstream oss;
            
            oss << "{";
            oss << "\"type\": \"" << nodeType << "\"";
            oss << ", \"parameters\": [";
            
            for (int i = 0; i < parameters.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (parameters[i] ? parameters[i]->toJSON() : "null");
            }

            oss << "] ";

            oss << ", \"definition\": [";
            
            for (int i = 0; i < definition.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (definition[i] ? definition[i]->toJSON() : "null");
            }

            oss << "] ";

            oss << "}";

            return oss.str();
        }
};