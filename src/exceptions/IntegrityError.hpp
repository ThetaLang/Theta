#pragma once

#include <string>
#include <iostream>
#include "Error.hpp"

using namespace std;

namespace Theta {
  class IntegrityError : public Error {
  public:
    IntegrityError(string l1, string l2) : line1(l1), line2(l2) {}

    string line1;
    string line2;

    void display() override {
      cout << "  \033[1;31mIntegrityError\033[0m: " + line1 << endl;
      cout << "    " + line2 << endl;
    }
  };
}
