#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class DictionaryNode : public ASTNodeList {
    public:
        DictionaryNode() : ASTNodeList(ASTNode::Types::DICTIONARY){};
    };
}
