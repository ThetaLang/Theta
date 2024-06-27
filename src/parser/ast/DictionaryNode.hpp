#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

class DictionaryNode : public ASTNodeList {
    public:
        DictionaryNode() : ASTNodeList(ASTNode::Types::DICTIONARY) {};
};
