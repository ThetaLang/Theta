#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class ASTNodeList : public ASTNode {
    public:
        vector<shared_ptr<ASTNode>> expressions;

        ASTNodeList() : ASTNode(ASTNode::Types::AST_NODE_LIST) {};

        void setExpressions(vector<shared_ptr<ASTNode>> expr) { expressions = expr; }

        vector<shared_ptr<ASTNode>> getExpressions() { return expressions; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
            oss << "\"expressions\": [";

            for (int i = 0; i < expressions.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (expressions[i] ? expressions[i]->toJSON() : "null");
            }

            oss << "] ";
            oss << "}";

            return oss.str();
        }
};
