#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include <vector>

using namespace std;

namespace Theta {
    class ASTNodeList : public ASTNode {
        public:
            vector<shared_ptr<ASTNode>> elements;

            ASTNodeList(ASTNode::Types type = ASTNode::AST_NODE_LIST) : ASTNode(type) {};

            void setElements(vector<shared_ptr<ASTNode>> el) { elements = el; }

            vector<shared_ptr<ASTNode>>& getElements() { return elements; }

            bool hasMany() override { return true; }

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
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
