#pragma once

#include <sstream>
#include <string>

#include "ASTNode.hpp"
#include "ASTNodeList.hpp"

using namespace std;

namespace Theta {
    class StructDefinitionNode : public ASTNodeList {
    public:
        string name;

        StructDefinitionNode(string n) : ASTNodeList(ASTNode::Types::STRUCT_DEFINITION), name(n){};
    };
}
