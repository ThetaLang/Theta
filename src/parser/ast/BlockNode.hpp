#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class BlockNode : public ASTNodeList {
    public:
        BlockNode() : ASTNodeList(ASTNode::Types::BLOCK){};
    };
}
