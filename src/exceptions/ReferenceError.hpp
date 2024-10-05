#pragma once

#include <string>
#include <iostream>
#include "Error.hpp"

using namespace std;

namespace Theta {
  class ReferenceError : public Error {
  public:
    ReferenceError(string ident) : identifier(ident) {}

    string identifier;

    void display() override {
      cout << "  \033[1;31mReferenceError\033[0m: '" +
        identifier +
        "' does not exist in the scope where it was referenced." << endl;
    }
  };
}
