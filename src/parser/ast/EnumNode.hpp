#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class EnumNode : public ASTNodeList {
        public:
            shared_ptr<ASTNode> identifier;

            EnumNode() : ASTNodeList(ASTNode::Types::ENUM) {};

            void setIdentifier(shared_ptr<ASTNode> ident) { identifier = ident; }

            shared_ptr<ASTNode> getIdentifier() { return identifier; }

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
                oss << "\"identifier\": " << (identifier ? identifier->toJSON() : "null") << ", ";
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
}
