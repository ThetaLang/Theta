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

        virtual void setValue(string side, shared_ptr<ASTNode> childNode) {
            if (side == "left") {
                left = childNode;
            } else {
                right = childNode;
            }
        }

        virtual ~ASTNode() = default;
};

#endif