#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

class BlockNode : public ASTNodeList {
    public:
        BlockNode() : ASTNodeList(ASTNode::Types::BLOCK) {};
};
