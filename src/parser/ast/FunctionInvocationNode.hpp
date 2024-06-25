#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class FunctionInvocationNode : public ASTNode {
    public:
        FunctionInvocationNode() : ASTNode(ASTNode::Types::FUNCTION_INVOCATION) {};

        shared_ptr<ASTNode> identifier;
        vector<shared_ptr<ASTNode>> parameters;

        void setIdentifier(shared_ptr<ASTNode> ident) { identifier = ident; }

        shared_ptr<ASTNode> getIdentifier() { return identifier; }

        void setParameters(vector<shared_ptr<ASTNode>> params) { parameters = params; }

        vector<shared_ptr<ASTNode>> getParameters() { return parameters; }

        string toJSON() const override {
            std::ostringstream oss;
            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
            oss << "\"function\": " << (identifier ? identifier->toJSON() : "null") << ", ";
            oss << "\"parameters\": [";
            for (int i = 0; i < parameters.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (parameters[i] ? parameters[i]->toJSON() : "null");
            }

            oss << "] ";
            oss << "}";
            return oss.str();
        }
};
