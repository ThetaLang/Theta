#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <memory>

using namespace std;

class ASTNode {
    public:
        enum Types {
            ASSIGNMENT,
            BINARY_OPERATION,
            BLOCK,
            CAPSULE,
            DICTIONARY,
            FUNCTION_DECLARATION,
            IDENTIFIER,
            KEYED_ACCESS,
            LINK,
            LIST,
            STRING_LITERAL,
            NUMBER_LITERAL,
            BOOLEAN_LITERAL,
            SOURCE,
            SYMBOL,
            TUPLE,
            TYPE_DECLARATION,
            UNARY_OPERATION
        };

        virtual ASTNode::Types getNodeType() { return nodeType; }
        virtual string getNodeTypePretty() const { return nodeTypeToString(nodeType); }
        virtual string toJSON() const = 0;
        ASTNode::Types nodeType;
        shared_ptr<ASTNode> value;
        shared_ptr<ASTNode> left;
        shared_ptr<ASTNode> right;

        ASTNode(ASTNode::Types type) : nodeType(type), value(nullptr) {};

        virtual void setValue(shared_ptr<ASTNode> childNode) { value = childNode; }

        virtual shared_ptr<ASTNode> getValue() { return value; }

        virtual void setLeft(shared_ptr<ASTNode> childNode) { left = childNode; }

        virtual shared_ptr<ASTNode> getLeft() { return left; }

        virtual void setRight(shared_ptr<ASTNode> childNode) { right = childNode; }

        virtual shared_ptr<ASTNode> getRight() { return right; }

        virtual ~ASTNode() = default;

        static string nodeTypeToString(ASTNode::Types nodeType) {
            static map<ASTNode::Types, string> typesMap = {
                { ASTNode::Types::ASSIGNMENT, "Asignment" },
                { ASTNode::Types::BINARY_OPERATION, "BinaryOperation" },
                { ASTNode::Types::BLOCK, "Block" },
                { ASTNode::Types::CAPSULE, "Capsule" },
                { ASTNode::Types::DICTIONARY, "Dictionary" },
                { ASTNode::Types::FUNCTION_DECLARATION, "FunctionDeclaration" },
                { ASTNode::Types::IDENTIFIER, "Identifier" },
                { ASTNode::Types::KEYED_ACCESS, "KeyedAccess" },
                { ASTNode::Types::LINK, "Link" },
                { ASTNode::Types::LIST, "List" },
                { ASTNode::Types::STRING_LITERAL, "StringLiteral" },
                { ASTNode::Types::NUMBER_LITERAL, "NumberLiteral" },
                { ASTNode::Types::BOOLEAN_LITERAL, "BooleanLiteral" },
                { ASTNode::Types::SOURCE, "Source" },
                { ASTNode::Types::SYMBOL, "Symbol" },
                { ASTNode::Types::TUPLE, "Tuple" },
                { ASTNode::Types::TYPE_DECLARATION, "TypeDeclaration" },
                { ASTNode::Types::UNARY_OPERATION, "UnaryOperation" }
            };

            auto it = typesMap.find(nodeType);
            if (it != typesMap.end()) {
                return it->second;
            } else {
                return "UNKNOWN";
            }
        }
};

#endif
