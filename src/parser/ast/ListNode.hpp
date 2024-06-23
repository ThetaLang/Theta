#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class ListNode : public ASTNode {
    public:
        vector<shared_ptr<ASTNode>> elements;

        ListNode() : ASTNode(ASTNode::Types::LIST) {};

        void setElements(vector<shared_ptr<ASTNode>> el) { elements = el; }

        vector<shared_ptr<ASTNode>> getElements() { return elements; }

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
