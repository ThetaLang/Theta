#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class LiteralNode : public ASTNode {
  public:
    string literalValue;

    LiteralNode(ASTNode::Types typ, string val, shared_ptr<ASTNode> parent) : ASTNode(typ, parent), literalValue(val) {};

    string getLiteralValue() { return literalValue; }
    void setLiteralValue(string val) { literalValue = val; }

    string toJSON() const override {
      ostringstream oss;

      oss << "{";
      oss << "\"type\": \"" << getNodeTypePretty() << "\", ";
      oss << "\"value\": \"" << literalValue << "\" ";
      oss << "}";

      return oss.str();
    }
  };
}
