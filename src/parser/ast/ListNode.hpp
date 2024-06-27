#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

class ListNode : public ASTNodeList {
    public:
        ListNode() : ASTNodeList(ASTNode::Types::LIST) {};
};
