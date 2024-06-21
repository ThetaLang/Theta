#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class FunctionDeclarationNode : public ASTNode {
    public:
        string nodeType;
        vector<shared_ptr<ASTNode>> parameters;
        shared_ptr<ASTNode> definition;

        FunctionDeclarationNode() : nodeType("FunctionDeclaration") {};

        string getNodeType() const override { return nodeType; }

        void setParameters(vector<shared_ptr<ASTNode>> params) { parameters = params; }

        vector<shared_ptr<ASTNode>> getParameters() { return parameters; }

        void setDefinition(shared_ptr<ASTNode> def) { definition = def; }

        shared_ptr<ASTNode> getDefinition() { return definition; }

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

            oss << ", \"definition\": " + (definition ? definition->toJSON() : "null");

            oss << "}";

            return oss.str();
        }
};
