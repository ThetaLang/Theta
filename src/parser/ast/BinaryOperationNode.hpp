#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class BinaryOperationNode : public ASTNode {
    public:
      string operatorSymbol;

      BinaryOperationNode(string op, shared_ptr<ASTNode> parent) : ASTNode(ASTNode::BINARY_OPERATION, parent), operatorSymbol(op) {};

      string getOperator() { return operatorSymbol; }

      string toJSON() const override {
        std::ostringstream oss;
        oss << "{";
        oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
        oss << "\"operator\": \"" << operatorSymbol << "\", ";
        oss << "\"left\": " << (left ? left->toJSON() : "null") << ", ";
        oss << "\"right\": " << (right ? right->toJSON() : "null");
        oss << "}";
        return oss.str();
      }
  };
}
