#pragma once

#include <string>
#include <iostream>
#include "Error.hpp"
#include "compiler/TypeChecker.hpp"

using namespace std;

namespace Theta {
  class TypeError : public Error {
  public:
    TypeError(string msg, shared_ptr<ASTNode> t1, shared_ptr<ASTNode> t2) : message(msg), type1(t1), type2(t2) {}

    string message;
    shared_ptr<ASTNode> type1;
    shared_ptr<ASTNode> type2;

    void display() override {
      string errText = "  \033[1;31mTypeError\033[0m: " + message + ": "; 

      pair<string, string> typeDiff = getTypeDiff(
        dynamic_pointer_cast<TypeDeclarationNode>(type1),
        dynamic_pointer_cast<TypeDeclarationNode>(type2)
      );

      if (typeDiff.first == "VARIADIC_MISMATCH") {
        errText += "Variadic right hand side must satisfy at least one left hand side type, and may not include extra types";
      } else {
        errText += typeDiff.first + " is not equivalent to " + typeDiff.second;
      }

      cout << errText << endl;
    }
  
  private:
    static pair<string, string> getTypeDiff(shared_ptr<TypeDeclarationNode> t1, shared_ptr<TypeDeclarationNode> t2) {
      if (!t1 || !t2) {
        return make_pair((t1 ? t1->getType() : "Nothing"), (t2 ? t2->getType() : "Nothing"));
      } else if (t1->getType() != t2->getType()) {
        return make_pair(
          (t1->getType() != "" ? t1->getType() : "Nothing"), 
          (t2->getType() != "" ? t2->getType() : "Nothing")
        );
      } else if (t1->getValue() || t2->getValue()) {
        return getTypeDiff(
          dynamic_pointer_cast<TypeDeclarationNode>(t1->getValue()), 
          dynamic_pointer_cast<TypeDeclarationNode>(t2->getValue())
        );
      } else if (t1->getLeft() || t2->getLeft()) {
        if (!TypeChecker::isSameType(t1->getLeft(), t2->getLeft())) {
          return getTypeDiff(
            dynamic_pointer_cast<TypeDeclarationNode>(t1->getLeft()),
            dynamic_pointer_cast<TypeDeclarationNode>(t2->getLeft())
          );
        }
        
        return getTypeDiff(
          dynamic_pointer_cast<TypeDeclarationNode>(t1->getRight()),
          dynamic_pointer_cast<TypeDeclarationNode>(t2->getRight())
        );
      } 

      return make_pair("VARIADIC_MISMATCH", "VARIADIC_MISMATCH");
    }
  };
}
