#pragma once

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include "Compiler.hpp"
#include "../util/Exceptions.hpp"
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/ASTNodeList.hpp"
#include "../parser/ast/IdentifierNode.hpp"
#include "../parser/ast/AssignmentNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/TypeDeclarationNode.hpp"


using namespace std;

namespace Theta {
    class Compiler;

    class TypeChecker {
        public:
            static bool checkAST(shared_ptr<ASTNode> ast);

        private:
            static bool checkNode(shared_ptr<ASTNode> node);

            static bool checkAssignmentNode(shared_ptr<AssignmentNode> node);

            static bool checkBinaryOperationNode(shared_ptr<BinaryOperationNode> node);

            static bool isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2);
    };
}
