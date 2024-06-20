#pragma once

#include <string>
#include <sstream>
#include "ast_node.hpp"

using namespace std;

class BlockNode : public ASTNode {
    public:
        string nodeType;
        vector<shared_ptr<ASTNode>> blockExpressions;

        BlockNode() : nodeType("Block") {};

        string getNodeType() const override { return nodeType; }

        void setBlockExpressions(vector<shared_ptr<ASTNode>> expr) { blockExpressions = expr; }

        vector<shared_ptr<ASTNode>> getBlockExpressions() { return blockExpressions; }

        string toJSON() const override {
            ostringstream oss;

            oss << "{";
            oss << "\"type\": \"" << nodeType << "\", ";
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
