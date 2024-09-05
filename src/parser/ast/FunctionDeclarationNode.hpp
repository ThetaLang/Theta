#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class FunctionDeclarationNode : public ASTNode {
        public:
            shared_ptr<ASTNodeList> parameters;
            shared_ptr<ASTNode> definition;

            FunctionDeclarationNode(shared_ptr<ASTNode> parent) : ASTNode(ASTNode::FUNCTION_DECLARATION, parent) {};

            void setParameters(shared_ptr<ASTNodeList> params) { parameters = params; }

            shared_ptr<ASTNodeList>& getParameters() { return parameters; }

            void setDefinition(shared_ptr<ASTNode> def) { definition = def; }

            shared_ptr<ASTNode>& getDefinition() { return definition; }

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\"";
                oss << ", \"parameters\": [";

                for (int i = 0; i < parameters->getElements().size(); i++) {
                    if (i > 0) {
                        oss << ", ";
                    }

                    oss << (parameters->getElements()[i] ? parameters->getElements()[i]->toJSON() : "null");
                }

                oss << "] ";

                oss << ", \"definition\": " + (definition ? definition->toJSON() : "null");

                oss << "}";

                return oss.str();
            }
    };
}
