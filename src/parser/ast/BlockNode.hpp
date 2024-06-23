#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

class BlockNode : public ASTNode {
    public:
        vector<shared_ptr<ASTNode>> blockExpressions;

        BlockNode() : ASTNode(ASTNode::Types::BLOCK) {};

        void setBlockExpressions(vector<shared_ptr<ASTNode>> expr) { blockExpressions = expr; }

        vector<shared_ptr<ASTNode>> getBlockExpressions() { return blockExpressions; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
            oss << "\"blockExpressions\": [";

            for (int i = 0; i < blockExpressions.size(); i++) {
                if (i > 0) {
                    oss << ", ";
                }

                oss << (blockExpressions[i] ? blockExpressions[i]->toJSON() : "null");
            }

            oss << "] ";
            oss << "}";

            return oss.str();
        }
};
