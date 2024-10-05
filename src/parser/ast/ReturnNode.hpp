#pragma once

#include <memory>
#include <string>
#include <sstream>
#include "ASTNode.hpp"

using namespace std;

namespace Theta {
  class ReturnNode : public ASTNode {
  public:
    ReturnNode(shared_ptr<ASTNode> parent) : ASTNode(ASTNode::RETURN, parent) {};

    string toJSON() const override {
      ostringstream oss;

      oss << "{";
      oss << "\"type\": \"" << getNodeTypePretty() << "\"";
      oss << ", \"value\": " << (value ? value->toJSON() : "null");
      oss << "}";

      return oss.str();
    }
  };
}
