#pragma once

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"
#include <memory>

using namespace std;

namespace Theta {
  class ListNode : public ASTNodeList {
  public:
    ListNode(shared_ptr<ASTNode> parent) : ASTNodeList(parent, ASTNode::LIST) {};
  };
}
