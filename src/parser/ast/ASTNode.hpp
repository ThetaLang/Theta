#pragma once

#include <string>
#include <memory>
#include <map>

using namespace std;

#ifdef RETURN
#undef RETURN
#endif

#ifdef NEWLINE
#undef NEWLINE
#endif

namespace Theta {
  class ASTNode {
  public:
    enum Types {
      ASSIGNMENT,
      AST_NODE_LIST,
      BINARY_OPERATION,
      BLOCK,
      BOOLEAN_LITERAL,
      CAPSULE,
      CONTROL_FLOW,
      DICTIONARY,
      ENUM,
      FUNCTION_DECLARATION,
      FUNCTION_INVOCATION,
      IDENTIFIER,
      LINK,
      LIST,
      NUMBER_LITERAL,
      RETURN,
      SOURCE,
      STRING_LITERAL,
      STRUCT_DECLARATION,
      STRUCT_DEFINITION,
      SYMBOL,
      TUPLE,
      TYPE_DECLARATION,
      UNARY_OPERATION
    };

    static int nextId;
    virtual ASTNode::Types getNodeType() { return nodeType; }
    virtual string getNodeTypePretty() const { return nodeTypeToString(nodeType); }
    virtual string toJSON() const = 0;
    int id;
    ASTNode::Types nodeType;
    shared_ptr<ASTNode> value;
    shared_ptr<ASTNode> left;
    shared_ptr<ASTNode> right;
    shared_ptr<ASTNode> resolvedType;
    shared_ptr<ASTNode> parent;
    int mappedBinaryenIndex;

    ASTNode(ASTNode::Types type, shared_ptr<ASTNode> par) : nodeType(type), parent(par), value(nullptr) {
      id = nextId;
      nextId++;
    };

    virtual int getId() { return id; }

    virtual void setValue(shared_ptr<ASTNode> childNode) { value = childNode; }
    virtual shared_ptr<ASTNode>& getValue() { return value; }

    virtual void setLeft(shared_ptr<ASTNode> childNode) { left = childNode; }
    virtual shared_ptr<ASTNode>& getLeft() { return left; }

    virtual void setRight(shared_ptr<ASTNode> childNode) { right = childNode; }
    virtual shared_ptr<ASTNode>& getRight() { return right; }

    virtual int getMappedBinaryenIndex() { return mappedBinaryenIndex; }
    virtual void setMappedBinaryenIndex(int idx) { mappedBinaryenIndex = idx; }

    virtual void setParent(shared_ptr<ASTNode> parentNode) { parent = parentNode; }
    virtual shared_ptr<ASTNode>& getParent() { return parent; }

    void setResolvedType(shared_ptr<ASTNode> typeNode) { resolvedType = typeNode; }
    shared_ptr<ASTNode> getResolvedType() { return resolvedType; }

    virtual bool hasMany() { return false; }

    virtual bool hasOwnScope() { return false; }

    virtual ~ASTNode() = default;

    static string nodeTypeToString(ASTNode::Types nodeType) {
      static map<ASTNode::Types, string> typesMap = {
        { ASTNode::ASSIGNMENT, "Assignment" },
        { ASTNode::AST_NODE_LIST, "ASTNodeList" },
        { ASTNode::BINARY_OPERATION, "BinaryOperation" },
        { ASTNode::BLOCK, "Block" },
        { ASTNode::BOOLEAN_LITERAL, "BooleanLiteral" },
        { ASTNode::CAPSULE, "Capsule" },
        { ASTNode::CONTROL_FLOW, "ControlFlow" },
        { ASTNode::DICTIONARY, "Dictionary" },
        { ASTNode::ENUM, "Enum" },
        { ASTNode::FUNCTION_DECLARATION, "FunctionDeclaration" },
        { ASTNode::FUNCTION_INVOCATION, "FunctionInvocation" },
        { ASTNode::IDENTIFIER, "Identifier" },
        { ASTNode::LINK, "Link" },
        { ASTNode::LIST, "List" },
        { ASTNode::NUMBER_LITERAL, "NumberLiteral" },
        { ASTNode::RETURN, "Return" },
        { ASTNode::SOURCE, "Source" },
        { ASTNode::STRING_LITERAL, "StringLiteral" },
        { ASTNode::STRUCT_DECLARATION, "StructDeclaration" },
        { ASTNode::STRUCT_DEFINITION, "StructDefinition" },
        { ASTNode::SYMBOL, "Symbol" },
        { ASTNode::TUPLE, "Tuple" },
        { ASTNode::TYPE_DECLARATION, "TypeDeclaration" },
        { ASTNode::UNARY_OPERATION, "UnaryOperation" }
      };

      auto it = typesMap.find(nodeType);
      if (it != typesMap.end()) {
        return it->second;
      } else {
        return "UNKNOWN";
      }
    }
  };
}
