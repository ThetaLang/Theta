#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class IdentifierNode : public ASTNode {
  public:
    string identifier;

    IdentifierNode(string ident, shared_ptr<ASTNode> parent) : ASTNode(ASTNode::IDENTIFIER, parent), identifier(ident) {};

    string getIdentifier() { return identifier; }

    string toJSON() const override {
      ostringstream oss;

      oss << "{";
      oss << "\"type\": \"" << getNodeTypePretty() << "\"";
      oss << ", \"value\": \"" << identifier << "\"";
      oss << ", \"variableType\": " << (value ? value->toJSON() : "null");
      oss << "}";

      return oss.str();
    }
  };
}
