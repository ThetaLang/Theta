#pragma once

#include <memory>
#include <string>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
  class StructDefinitionNode : public ASTNodeList {
  public:
    string name;

    StructDefinitionNode(string n, shared_ptr<ASTNode> parent) : ASTNodeList(parent, ASTNode::STRUCT_DEFINITION), name(n) {};

    string getName() { return name; }
  };
}
