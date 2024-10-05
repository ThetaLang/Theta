#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class UnaryOperationNode : public ASTNode {
  public:
    string operatorSymbol;

    UnaryOperationNode(string op, shared_ptr<ASTNode> parent) : ASTNode(ASTNode::UNARY_OPERATION, parent),  operatorSymbol(op) {};

    string getOperator() { return operatorSymbol; }

    string toJSON() const override {
      std::ostringstream oss;
      oss << "{";
      oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
      oss << "\"operator\": \"" << operatorSymbol << "\", ";
      oss << "\"value\": " << (value ? value->toJSON() : "null");
      oss << "}";
      return oss.str();
    }
  };
}
