#pragma once

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <memory>
#include "Compiler.hpp"
#include "DataTypes.hpp"
#include "../util/Exceptions.hpp"
#include "../parser/ast/ASTNode.hpp"
#include "../parser/ast/ASTNodeList.hpp"
#include "../parser/ast/IdentifierNode.hpp"
#include "../parser/ast/AssignmentNode.hpp"
#include "../parser/ast/BinaryOperationNode.hpp"
#include "../parser/ast/BlockNode.hpp"
#include "../parser/ast/ControlFlowNode.hpp"
#include "../parser/ast/FunctionDeclarationNode.hpp"
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

            static bool checkUnaryOperatonNode(shared_ptr<UnaryOperationNode> node);

            static bool checkBlockNode(shared_ptr<BlockNode> node);

            static bool checkFunctionDeclarationNode(shared_ptr<FunctionDeclarationNode> node);

            static bool checkControlFlowNode(shared_ptr<ControlFlowNode> node);

            static bool isSameType(shared_ptr<ASTNode> type1, shared_ptr<ASTNode> type2);

            static vector<shared_ptr<ASTNode>> findAllInTree(shared_ptr<ASTNode> node, ASTNode::Types type);

            static shared_ptr<TypeDeclarationNode> makeVariadicType(vector<shared_ptr<TypeDeclarationNode>> types);
    };
}
