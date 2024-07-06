#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class ListNode : public ASTNodeList {
    public:
        ListNode() : ASTNodeList(ASTNode::Types::LIST){};
    };
}
