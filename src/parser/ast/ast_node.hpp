#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <memory>

using namespace std;

class ASTNode {
    public:
        virtual string getNodeType() const = 0;
        virtual string toJSON() const = 0;
        shared_ptr<ASTNode> value;
        shared_ptr<ASTNode> left;
        shared_ptr<ASTNode> right;

        ASTNode() : value(nullptr) {};

        virtual void setValue(shared_ptr<ASTNode> childNode) { value = childNode; }

        virtual shared_ptr<ASTNode> getValue() { return value; }

        virtual void setLeft(shared_ptr<ASTNode> childNode) { left = childNode; }

        virtual shared_ptr<ASTNode> getLeft() { return left; }

        virtual void setRight(shared_ptr<ASTNode> childNode) { right = childNode; }

        virtual shared_ptr<ASTNode> getRight() { return right; }

        virtual ~ASTNode() = default;
};

#endif