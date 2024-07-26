#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class FunctionInvocationNode : public ASTNode {
        public:
            FunctionInvocationNode() : ASTNode(ASTNode::FUNCTION_INVOCATION) {};

            shared_ptr<ASTNode> identifier;
            shared_ptr<ASTNodeList> arguments;

            void setIdentifier(shared_ptr<ASTNode> ident) { identifier = ident; }

            shared_ptr<ASTNode> getIdentifier() { return identifier; }

            void setParameters(shared_ptr<ASTNodeList> params) { arguments = params; }

            shared_ptr<ASTNodeList> getParameters() { return arguments; }

            string toJSON() const override {
                std::ostringstream oss;
                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
                oss << "\"function\": " << (identifier ? identifier->toJSON() : "null") << ", ";
                oss << "\"arguments\": [";
                for (int i = 0; i < arguments->getElements().size(); i++) {
                    if (i > 0) {
                        oss << ", ";
                    }

                    oss << (arguments->getElements()[i] ? arguments->getElements()[i]->toJSON() : "null");
                }

                oss << "] ";
                oss << "}";
                return oss.str();
            }
    };
}
