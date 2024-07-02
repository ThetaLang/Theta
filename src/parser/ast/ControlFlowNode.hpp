#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
    class ControlFlowNode : public ASTNode {
        public:
            vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> conditionExpressionPairs;

            ControlFlowNode() : ASTNode(ASTNode::Types::CONTROL_FLOW) {};

            void setConditionExpressionPairs(vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> cnd) {
                conditionExpressionPairs = cnd;
            }

            vector<pair<shared_ptr<ASTNode>, shared_ptr<ASTNode>>> getConditionExpressionPairs() {
                return conditionExpressionPairs;
            }

            string toJSON() const override {
                ostringstream oss;

                oss << "{";
                oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
                oss << "\"conditionExpressionPairs\": [";

                for (int i = 0; i < conditionExpressionPairs.size(); i++) {
                    if (i > 0) {
                        oss << ", ";
                    }

                    oss << "[ ";
                    oss << (conditionExpressionPairs[i].first ? conditionExpressionPairs[i].first->toJSON() : "null");
                    oss << ", ";
                    oss << (conditionExpressionPairs[i].second ? conditionExpressionPairs[i].second->toJSON() : "null");
                    oss << "] ";
                }

                oss << "] ";
                oss << "}";

                return oss.str();
            }
    };
}
