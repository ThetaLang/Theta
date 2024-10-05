#pragma once

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
  class BlockNode : public ASTNodeList {
  public:
    BlockNode(shared_ptr<ASTNode> parent) : ASTNodeList(parent, ASTNode::BLOCK) {};
    
    bool hasOwnScope() override { return true; }
  };
}
