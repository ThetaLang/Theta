#pragma once

#include <string>
#include <sstream>
#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

class StructDefinitionNode : public ASTNodeList {
    public:
        string name;

        StructDefinitionNode(string n) : ASTNodeList(ASTNode::Types::STRUCT_DEFINITION), name(n) {};
};
