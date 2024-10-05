#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class SymbolNode : public ASTNode {
  public:
    string symbol;

    SymbolNode(string sym, shared_ptr<ASTNode> parent) : ASTNode(ASTNode::SYMBOL, parent), symbol(":" + sym) {};

    string getSymbol() { return symbol; }

    string toJSON() const override {
      ostringstream oss;

      oss << "{";
      oss << "\"type\": \"" << getNodeTypePretty() << "\"";
      oss << ", \"value\": \"" << symbol << "\"";
      oss << "}";

      return oss.str();
    }
  };
}
