#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class BlockNode : public ASTNodeList {
        public:
            BlockNode() : ASTNodeList(ASTNode::BLOCK) {};
            
            bool hasOwnScope() override { return true; }
    };
}
