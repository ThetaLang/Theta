#pragma once

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class DictionaryNode : public ASTNodeList {
        public:
            DictionaryNode(shared_ptr<ASTNode> parent) : ASTNodeList(parent, ASTNode::DICTIONARY) {};
    };
}
